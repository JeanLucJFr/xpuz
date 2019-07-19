#ifndef __base_image_h
#define __base_image_h

typedef unsigned char	byte;
typedef int					boolean;

#define	OPTION_EXTENSION		(0x10)
#define	REMOVETILE_EXTENSION	(0x11)
#define	FLATTILE_EXTENSION	(0x12)
#define	SUBSIZE_EXTENSION		(0x13)
#define	SCALEINFO_START		(0x20)
#define	COMMENT_EXTENSION		(0xFE)

class Image {
	public:
		Image( const char *filename, int autocrop=1 );
		Image();
		virtual ~Image();

		virtual const char *GetExtensionData( unsigned char code ) { return 0; }

		const char *Name()	{ return name; }
		int Width()				{ return width; }
		int Height()			{ return height; }
		const byte* Data()	{ return data; }
		boolean DataIsRGB()	{ return pixel_size==3; }

		int CropImage(int x1,int y1, int x2, int y2);
		int CropImage();

		void Rotate90();

	protected:
		virtual int LoadImageFile(const char *fname);
		virtual int LoadImageFile(FILE *fp) {
			/* This can only be reached if a child class fails to provide either
			 * one of the LoadImageFile() methods. I can't figure a way of
			 * making that a compile-time error. Think of these 2 methods as
			 * "jointly pure virtual". */
			fprintf(stderr,
					  "internal error: LoadImageFile() methods missing\n");
			exit(1);
		}
		void CloseImage();
		void DefaultBackground();

	protected:
		char	*name;
		int	width, height;								// image dimensions
		int 	pixel_size;
		byte	*data;										// image data
		byte	Red[256], Green[256], Blue[256];		// Colormap-Data
		boolean	HasColormap;
		int		ColorMapSize;						/* number of colors */
		int		Background;							/* background color */

	public:
		int	GetNCols()	{ return ColorMapSize; }
		int	GetColor(int id, unsigned short *red, unsigned short *green, unsigned short *blue);
};

#endif
