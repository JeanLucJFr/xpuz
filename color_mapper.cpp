/*This file is part of xpuz.

    xpuz is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xpuz is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xpuz.  If not, see <http://www.gnu.org/licenses/>.
*/
   
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __color_mapper_h
#	include "color_mapper.h"
#endif

ColorMapper::ColorMapper(Display *dpy_in)
: dpy(dpy_in)
{
Screen	*screen = DefaultScreenOfDisplay(dpy);

	mymap=DefaultColormapOfScreen(screen);
	cells=0;
	colors=0;
	if (DefaultVisualOfScreen(screen)->c_class!=PseudoColor)		return;

	cells=CellsOfScreen(screen);
	colors      = new XColor[cells];
	setup_usage();
}

ColorMapper::~ColorMapper() {
	free_usage();
	if (colors)			delete [] colors;
}

void ColorMapper::free_usage() {
int i;

	for (i=0;i<cells;i++) {
		if (colors[i].flags) {
   		XFreeColors(dpy,mymap,&colors[i].pixel,1,0L);
   	}
	}
}

void ColorMapper::setup_usage() {
int i;
unsigned long	*help;

	for (i=0;i<cells;i++)	colors[i].pixel = i;
	XQueryColors(dpy,mymap,colors,cells);
	for (i=0;i<cells;i++)	colors[i].flags = DoRed | DoGreen | DoBlue;

	help = new unsigned long[cells];
/*
 * allocate unused cells of the colormap for read/write
 */
   i = 0;
   while(XAllocColorCells( dpy, mymap, False, 0L, 0, &help[i], 1 )) {
   	colors[help[i]].flags = 0;		// mark unusable for shared color
   	i++;
   }

/*
 * free the pixels again
 */
   XFreeColors(dpy,mymap,help,i,0L);
   delete [] help;
   XSync( dpy, 0 );

   for (i=0;i<cells;i++) {
   	if (colors[i].flags) {		// still not invalidated
   	// all shareable colors are again allocated !!
   		if (XAllocColor(dpy,mymap,&colors[i])) {
   			if ((unsigned)i!=colors[i].pixel) {
   				XFreeColors(dpy,mymap,&colors[i].pixel,1,0L);
   				colors[i].flags=0;		// different pixel -> not usefull
   			}
   		}
   		else {
   				colors[i].flags=0;		// not allocatable -> not usefull
   		}
   	}
   }
   return;
}

unsigned long ColorMapper::alloc_color(XColor *def) {
int	i;
long	min_dist;
int	min_i;

	if (!colors) {
		def->flags=DoRed | DoGreen | DoBlue;
		if (!XAllocColor(dpy,mymap,def)) {
			fprintf( stderr, "\n*** failed to allocated color on '%s'\n\n", DisplayString(dpy) );
			exit(0);
		}
		return def->pixel;
	}

	min_i=-1;
	min_dist=0;
	for (i=0;i<cells;i++) {
		if (colors[i].flags) {
			long rd = ((long)colors[i].red   - (long)def->red)/4;
			long gd = ((long)colors[i].green - (long)def->green)/4;
			long bd = ((long)colors[i].blue  - (long)def->blue)/4;
			long dist=rd*rd+gd*gd+bd*bd;

			if (min_i<0 || dist<min_dist) {
				min_dist = dist;
				min_i    = i;
			}
		}
	}

// reuse already allocated color, when possible
	if (min_i>=0 && min_dist<10000) {
		return colors[min_i].pixel;
	}

// allocate additional entry for that pixel
	def->flags=DoRed | DoGreen | DoBlue;
	if (XAllocColor(dpy,mymap,def)) {
		colors[def->pixel] = *def;
		return def->pixel;
	}

// allocate the closest entry
	if (min_i>=0) {
		return colors[min_i].pixel;
	}

// everything else failed ...
	fprintf( stderr, "can't handle colormap overflow ...\n" );
	exit(0);
	return 0;
}

unsigned long ColorMapper::alloc_named_color( const char *name ) {
XColor   def;
	if (!XLookupColor(dpy,mymap,name,&def,&def )) {
		fprintf( stderr, "\n*** failed to query color '%s'\n\n", name );
		exit(0);
	}
	def.flags = DoRed | DoGreen | DoBlue;
	return alloc_color(&def);
}

// ============================================================================

Port::Port(Display *dpy_in) {
	dpy = dpy_in;
	mapper = new ColorMapper(dpy);
}

Port::~Port() {
	delete mapper;
}
