#ifndef __gifx_image_h
#define __gifx_image_h

#include "gif_image.h"

class GifXImage : public GifImage {
	public:
		GifXImage( class Port *port, const char *filename, int autocrop=0);
		~GifXImage();

	// Size Information
		int   XWidth()							{  return xwidth; }
		int   XHeight()						{  return xheight; }
		void GetXSize( int *w, int *h )	{ *w=xwidth; *h=xheight; }

		void CreateData( int w, int h );
		void SetupMapper();
		void SetupTrueMapper();
		void TraceMapper();

		XImage *GetImage() {
			if (!ximage)		CreateData(xwidth,xheight);
			return ximage;
		}
		unsigned long GetPixel( int x, int y );

	public:
		void DropData();
		void Reset8();
		void Reset16();
		void Reset32();

		Display	*dpy;
		int		scr;

		XImage	*ximage;
		int		xwidth, xheight;
		int		offset_rows;		// rows befor and ahead of the image
		int		offset_bytes;		// to prevent overwrite when optimized

		unsigned long		*gif_cols;
		class Port			*p;
};

class GifPixmap : public GifXImage {
	public:
		GifPixmap( class Port *port, const char *filename, int autocrop=0);
		~GifPixmap();

		Pixmap	GetPixmap();
		void CreateData( int w, int h );

		int		Width()	{ return GifXImage::Width()/xmult; }
		int		Height()	{ return GifXImage::Height()/ymult; }
		int		XWidth()	{ return GifXImage::XWidth()/xmult; }
		int		XHeight()	{ return GifXImage::XHeight()/ymult; }

		int		IsTwinPixmap()		{ return ymult==2; }

	private:
		Pixmap	pixmap;

		int		xmult,ymult;
};

#endif
