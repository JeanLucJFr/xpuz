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
   

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "base_image.h"

#define	False	0
#define	True	1
#define	DEBUG	0

#define	minpix			10

Image::Image(const char *filename, int autocrop )
{
const char *name_start;

	pixel_size=1;
	name_start = strrchr(filename,'/');
	if (name_start)	name=strdup(name_start+1);
	else			name=strdup(filename);

   /*can't call virtual from constructor, so child class will have to do this:*/
	/*LoadImageFile( filename );*/

	/*this depends on the image being loaded, so child class has to do it too:*/
	/*if (autocrop&&!fastinfo_flag)	CropImage();*/
}

Image::Image() {
	name=strdup("dummy");
	ColorMapSize=128;
	for (int i=0;i<128;i++) {
		Red[i]   = 255 * ((i>>5)&0x03) / 3;
		Green[i] = 255 * ((i>>2)&0x07) / 7;
		Blue[i]  = 255 *      (i&0x03) / 3;
	}
	width =1;
	height=1;
	pixel_size=1;
	data  =(byte*)malloc(1);
}

Image::~Image() {
	free(name);
	CloseImage();
}

int Image::GetColor(int id, unsigned short *red, unsigned short *green, unsigned short *blue) {
	*red  =Red[id]   | Red[id]<<8;
	*green=Green[id] | Green[id]<<8;
	*blue =Blue[id]  | Blue[id]<<8;
	return 0;
}

void Image::Rotate90() {
byte	*ndata;
int	help;
int	size = pixel_size;

	if (!(ndata = (byte *)malloc(width*size*height))) {
		fprintf(stderr,"not enough memory to flip image");
		exit(-1);
	}

	for (int y=0;y<height;y++) {
		for (int x=0;x<width;x++) {
			ndata[((height-y-1)+x*height)*size  ] = data[(x+y*width)*size  ];
			if(size>1) {
				ndata[((height-y-1)+x*height)*size+1] = data[(x+y*width)*size+1];
				ndata[((height-y-1)+x*height)*size+2] = data[(x+y*width)*size+2];
			}
		}
	}

	help=width; width=height; height=help;
	free(data);
	data=ndata;
}

int Image::LoadImageFile(const char *fname)
{
	int ret;
	FILE *fp;

	fp = fopen(fname,"rb");

	if (!fp) {
			fprintf(stderr,"'%s': File not found\n", fname);
			exit(0);
	}

	ret = LoadImageFile(fp);

	fclose(fp);
	return ret;
}

int Image::CropImage(int x1,int y1, int x2, int y2) {
int	w = x2-x1;
int	h = y2-y1;

	if (x1<0 || x2>width  || w<0 || y1<0 || y2>height || h<0) {
		fprintf(stderr,"unable to crop (%d,%d)-(%d,%d)\n", x1,y1,x2,y2);
		fprintf(stderr,"image size: %dx%d\n", width, height );
		exit(-1);
	}

	int size = pixel_size;
	for (int i=0;i<h;i++) {
			memmove(data+i*w*size,data+(i+y1)*Width()*size+x1,w*size);
	}
	width  = w;
	height = h;
	return 0;
}

int Image::CropImage() {
	// don't autocrop rgb images
	if(DataIsRGB()) return 0;

int		i,j;
int		b;
int		x1,x2,y1,y2;
byte		*ptr;
long		d;

	b=*data;
	d=0x7fffffff;
	for (i=0;i<256;i++) {
			if (Red[i]+Green[i]+Blue[i]<d) {
					b=i;
					d=Red[i]+Green[i]+Blue[i];
			}
	}
	x1=-1; x2=Width();
	y1=-1; y2=Height();
	for (i=0; i<Height(); i++) {
			int		flag[256];
			int		count=0;
			ptr = data + (i*Width());
			for (j=0;j<256;j++)		flag[j]=0;
		for (j=0; j<Width(); j++,ptr++) {
				if (!flag[*ptr]) {
						flag[*ptr]++;
						if (++count>=minpix)		break;
				}
		}
		if (count>=minpix)		break;
		y1=i;
	}
	for (i=Height()-1;i>=0; i--) {
			int		flag[256];
			int		count=0;
			ptr = data + (i*Width());
			for (j=0;j<256;j++)		flag[j]=0;
		for (j=0; j<Width(); j++,ptr++) {
				if (!flag[*ptr]) {
						flag[*ptr]++;
						if (++count>=minpix)		break;
				}
		}
		if (count>=minpix)		break;
		y2=i;
	}

	for (i=0; i<Width(); i++) {
			int		flag[256];
			int		count=0;
			ptr = data + i;
			for (j=0;j<256;j++)		flag[j]=0;
		for (j=0; j<Height(); j++,ptr+=Width()) {
				if (!flag[*ptr]) {
						flag[*ptr]++;
						if (++count>=minpix)		break;
				}
		}
		if (count>=minpix)		break;
		x1=i;
	}
	for (i=Width()-1;i>=0;i--) {
			int		flag[256];
			int		count=0;
			ptr = data + i;
			for (j=0;j<256;j++)		flag[j]=0;
		for (j=0; j<Height(); j++,ptr+=Width()) {
				if (!flag[*ptr]) {
						flag[*ptr]++;
						if (++count>=minpix)		break;
				}
		}
		if (count>=minpix)		break;
		x2=i;
	}

	x1++; x2--; y1++; y2--;
	if (x2<=x1||y2<=y1)				return 1;

	CropImage(x1,y1,x2,y2);
	return 0;
}


void Image::CloseImage()
{
#if (0)
	if (LocalCmap)
	{
		XFreeColormap(theDisp, LocalCmap);
		LocalCmap=0;
	}  
	else
	{
		int i,j;
		unsigned long pixels[256];
		 
		for (j=i=0;i<ColorMapSize;i++)
			if (used[i]==ALLOCATED)
				pixels[j++]=cols[i];
		XFreeColors(theDisp, theCmap, pixels, j, 0L);
	}
#endif
	if (data)
	{
		free(data);
		data=NULL;
	}
}

void Image::DefaultBackground()
{
	int darkest=255*255*3+1;
	int i;
	for(i=0;i<ColorMapSize;++i) {
		int s;
		s=Red[i]*Red[i]+Green[i]*Green[i]+Blue[i]*Blue[i];
		if(s<darkest) {
			darkest=s;
			Background=i;
		}
	}

	/* Add a black pixel for background, if there was no black in the image
	 * and there's a free slot in the palette. */
	if(darkest>0 && i<256) {
		Red[i]=Green[i]=Blue[i]=0;
		Background=i;
		++ColorMapSize;
	}
}
