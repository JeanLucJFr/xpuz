
#include "global.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xmd.h>

#include "ximage.h"
#include "color_mapper.h"
#include "gif_image.h"
#include "jpeg_image.h"

XjigXImage::XjigXImage(class Port *port,const char *filename,int autocrop)
: p(port) {
	const char *suffix = strrchr(filename, '.');
	if(suffix && (!strcasecmp(suffix, ".jpg") ||
					  !strcasecmp(suffix, ".jpeg") ||
					  !strcasecmp(suffix, ".jpe")))
		img=new jpegImage(filename, autocrop);

	else
		img=new GifImage(filename, autocrop);
	xwidth  = Width();
	xheight = Height();
	dpy=p->GetDisplay();
	scr=DefaultScreen(dpy);
	ximage = 0;
	gif_cols=0;
	if(!DataIsRGB()) SetupMapper();
#ifndef RANGE_CHECK
	offset_rows=(int)(sqrt((double)xwidth*(double)xwidth+(double)xheight*(double)xheight)/2.0)+1;
#else
	offset_rows=0;
#endif
}

XjigXImage::~XjigXImage() {
	DropData();
	if (gif_cols)		delete [] gif_cols;
}

void XjigXImage::SetupMapper() {
	if (gif_cols)		delete [] gif_cols;
	SetupTrueMapper();
}

void XjigXImage::SetupTrueMapper() {
	gif_cols=new unsigned long[GetNCols()];
	for (int i=0;i<GetNCols();i++) {
		XColor	def;
		GetColor(i,&def.red,&def.green,&def.blue);
		gif_cols[i]=p->GetMapper()->alloc_color(&def);
	}
}

void XjigXImage::TraceMapper() {
	for (int i=0;i<GetNCols();i++) {
		XColor   def;
		GetColor(i,&def.red,&def.green,&def.blue);
		printf( "%3d: %02x %02x %02x - %ld\n", i, def.red>>8, def.green>>8, def.blue>>8, gif_cols[i] );
	}
}

void XjigXImage::CreateData( int w, int h ) {
	xwidth  = w;
	xheight = h;
#ifndef RANGE_CHECK
	offset_rows=(int)(sqrt((double)w*(double)w+(double)h*(double)h)/2.0)+1;
#else
	offset_rows=0;
#endif
	DropData();
	switch(texture_mode) {
	case 1:		Reset8();		break;
	case 2:		Reset16();		break;
	case 3:		Reset32();		break;
	default:		fprintf( stderr, "depth not supported\n" );
					exit(0);
	}
}

void XjigXImage::DropData() {
	if (ximage) {
		delete [] (ximage->data-offset_bytes);
		ximage->data = 0L;
		XDestroyImage(ximage);
		ximage=0;
	}
}

unsigned long XjigXImage::GetPixel(int x, int y) {
	if (x<0||x>=xwidth||y<0||y>=xheight) {
		return 0;
	}
	else {
		return XGetPixel(ximage,x,y);
	}
}

/*----------------------------------------------------------------------------*/
#define	DATA_TYPE	CARD32
#define	DATA_PAD		4
void XjigXImage::Reset32() {
#	include "reset_image.h"
}
#undef DATA_TYPE
#undef DATA_PAD
/*----------------------------------------------------------------------------*/
#define	DATA_TYPE	CARD16
#define	DATA_PAD		2
void XjigXImage::Reset16() {
#	include "reset_image.h"
}
#undef DATA_TYPE
#undef DATA_PAD
/*----------------------------------------------------------------------------*/
#define	DATA_TYPE	CARD8
#define	DATA_PAD		1
void XjigXImage::Reset8() {
#	include "reset_image.h"
}
#undef DATA_TYPE
#undef DATA_PAD

// ========================================================================

XjigPixmap::XjigPixmap( Port *port, const char *filename, int autocrop )
: XjigXImage(port,filename,autocrop) {
int w, h;

	pixmap=0;

	const char	*ext=GetExtensionData( SUBSIZE_EXTENSION );

	if (ext&&sscanf( ext, "%dx%d", &w, &h)==2) {
		xmult=XjigXImage::Width()/w;
		ymult=XjigXImage::Height()/h;
	}
	else {
		xmult=1;
		ymult=1;
	}
}

XjigPixmap::~XjigPixmap() {
	if (pixmap)		XFreePixmap(dpy,pixmap);
}

Pixmap XjigPixmap::GetPixmap() {
	if (!pixmap) {
		pixmap=XCreatePixmap(dpy,RootWindow(dpy,scr),xwidth,xheight,DefaultDepth(dpy,scr));
		XPutImage(dpy,pixmap,DefaultGC(dpy,scr),GetImage(),0,0,0,0,xwidth,xheight);
	}
	return pixmap;
}

void XjigPixmap::CreateData(int w,int h) {
	w*=xmult;
	h*=ymult;
	if (pixmap)		{ XFreePixmap(dpy,pixmap); pixmap=0; }
	XjigXImage::CreateData(w,h);
}
