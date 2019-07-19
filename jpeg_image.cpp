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
#include <jpeglib.h>

#include "jpeg_image.h"
#include "global.h"

#undef XJIG_JPEG_DEBUG

jpegImage::jpegImage(const char *filename, int autocrop )
	: Image(filename, autocrop)
{
	LoadImageFile( filename );
	if (autocrop)	CropImage();
}

/*****************************/
int jpegImage::LoadImageFile(FILE *fp)
/*****************************/
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int i, j, k, copied_lines, bytes_written;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);

	/* Select palettized or RGB output */
	if(texture_mode==1) {
		HasColormap = TRUE;
		pixel_size = 1;
	} else {
		HasColormap = FALSE;
		pixel_size = 3;
	}
	cinfo.out_color_space = JCS_RGB;
	cinfo.quantize_colors = HasColormap;

	jpeg_start_decompress(&cinfo);

	width=cinfo.output_width;
	height=cinfo.output_height;

#ifdef XJIG_JPEG_DEBUG
	fprintf(stderr, "jpeg width=%d height=%d\n", width, height);
	if(cinfo.output_components!=pixel_size) {
		fprintf(stderr, "Not getting correctly formatted output from libjpeg!\n");
		exit(1);
	}
#endif

	data=(byte *)malloc(width*pixel_size*height*sizeof *data);
	if(!data) {
		fprintf(stderr, "Not enough memory to store jpeg data");
		exit(1);
	}

	JSAMPARRAY lines=(JSAMPARRAY)malloc(cinfo.rec_outbuf_height*sizeof *lines);
	if(!lines) {
		fprintf(stderr, "Not enough memory to for temporary jpeg buffer");
		exit(1);
	}
	for(i=0 ; i<cinfo.rec_outbuf_height ; ++i) {
		lines[i]=(JSAMPROW)malloc(width*pixel_size*sizeof *lines[i]);
		if(!lines[i]) {
			fprintf(stderr, "Not enough memory to for temporary jpeg buffer");
			exit(1);
		}
	}

	copied_lines=0;
	bytes_written=0;
	while(cinfo.output_scanline < cinfo.output_height) {
		int gotlines=jpeg_read_scanlines(&cinfo, lines, cinfo.rec_outbuf_height);
#ifdef XJIG_JPEG_DEBUG
		fprintf(stderr, "got %d lines from jpeg\n", gotlines);
#endif
		for(i=0;i<gotlines;++i)
			for(j=0;j<width;++j)
				for(k=0;k<pixel_size;k++)
					data[bytes_written++]=lines[i][j*pixel_size+k];
		copied_lines+=i;
#ifdef XJIG_JPEG_DEBUG
		fprintf(stderr, "have now filled in %d lines of data\n", copied_lines);
#endif
	}

	for(i=0 ; i<cinfo.rec_outbuf_height ; ++i)
		free(lines[i]);
	free(lines);

	if(HasColormap) {
		ColorMapSize=cinfo.actual_number_of_colors;
		for(i=0;i<ColorMapSize;++i) {
			Red[i]   = cinfo.colormap[0][i];
			Green[i] = cinfo.colormap[1][i];
			Blue[i]  = cinfo.colormap[2][i];
		}
	}
	Background=-1;

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return 0;
}
