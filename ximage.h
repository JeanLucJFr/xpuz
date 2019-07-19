#ifndef __ximage_h
#define __ximage_h

#include "base_image.h"

class XjigXImage {
	public:
		XjigXImage( class Port *port, const char *filename, int autocrop=0);
		~XjigXImage();

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

		/* Can't inherit Image because it has a pure virtual function and
		 * XjigXImage needs to be instantiated. So instead, let it contain a
		 * pointer to one and forward all the necessary methods. */
		Image *img;
		int Width()				{ return img->Width(); }
		int Height()			{ return img->Height(); }
		const char *GetExtensionData( unsigned char code )
			{ return img->GetExtensionData(code); }
		int CropImage(int x1,int y1, int x2, int y2)
			{ return img->CropImage(x1,y1,x2,y2); }
		int CropImage() { return img->CropImage(); }
		void Rotate90() { img->Rotate90(); }
		int	GetNCols()	{ return img->GetNCols(); }
		int	GetColor(int id, unsigned short *red,
							unsigned short *green, unsigned short *blue)
			{ return img->GetColor(id, red, green, blue); }
		const byte* Data()	{ return img->Data(); }
		boolean DataIsRGB() { return img->DataIsRGB(); }
};

class XjigPixmap : public XjigXImage {
	public:
		XjigPixmap( class Port *port, const char *filename, int autocrop=0);
		~XjigPixmap();

		Pixmap	GetPixmap();
		void CreateData( int w, int h );

		int		Width()	{ return XjigXImage::Width()/xmult; }
		int		Height()	{ return XjigXImage::Height()/ymult; }
		int		XWidth()	{ return XjigXImage::XWidth()/xmult; }
		int		XHeight()	{ return XjigXImage::XHeight()/ymult; }

		int		IsTwinPixmap()		{ return ymult==2; }

	private:
		Pixmap	pixmap;

		int		xmult,ymult;
};

#endif
