#ifndef __jpeg_image_h
#define __jpeg_image_h

#include "base_image.h"

class jpegImage : public Image {
	public:
		jpegImage( const char *filename, int autocrop=1 );

	protected:
		int LoadImageFile(FILE *fp);
		int LoadImageFile(const char *fname) {return Image::LoadImageFile(fname);}
};

#endif
