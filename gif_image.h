#ifndef __gif_image_h
#define __gif_image_h

#include "base_image.h"

class Extension {
	public:
		Extension(unsigned char code_in,class Extension *n)
			{ code=code_in; data=new char[1]; len=0; next=n; }
		~Extension()		{
				if (next)	delete next;
				if (data)	delete data;
		}
		void AddData( char *ndata, int nlen );

	private:
		unsigned char		code;
		char					*data;
		int					len;
		class Extension	*next;

	friend class GifImage;
};

class GifImage : public Image {
	public:
		GifImage( const char *filename, int autocrop=1 );
		GifImage();
		~GifImage();

		const char *GetExtensionData( unsigned char code );

	protected:
		int LoadImageFile(const char *fname);
		int ReadImageData(FILE *fp);			// read raw raster data into buffer
		int DecodeImage();						// decode data into image
		static int ReadCode();
		void AddToPixel(byte Index);
#if (0)
		int ColorDicking();
#endif
		void ReadColormap(FILE *fp);

	protected:
		Extension	*first;
};

#endif
