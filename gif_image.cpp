//
// The loading routine for gif images was extracted from the gif-loader
// of the xv-package. It was enclosed into a C++-class and enhanced for some
// features to query possible extension data, which can be used to store
// the additional setup informations and copyright notes.
//
// Here is the original note, that once was contained in the loader
//

/*
 * xvgif.cpp  -  GIF loading code for 'xv'.  Based strongly on...
 *
 * gif2ras.cpp - Converts from a Compuserve GIF (tm) image to a Sun Raster image.
 *
 * Copyright (c) 1988, 1989 by Patrick J. Naughton
 *
 * Author: Patrick J. Naughton
 * naughton@wind.sun.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gif_image.h"

#define	False	0
#define	True	1
#define	DEBUG	0

#define	quiet				1
#define	fastinfo_flag	0

void Extension::AddData( char *ndata, int nlen ) {
	char *hdata=new char[len+nlen];
	memcpy(hdata,data,len);
	memcpy(hdata+len,ndata,nlen);
	delete data;
	len+=nlen;
	data=hdata;
}

GifImage::GifImage(const char *filename, int autocrop )
	: Image(filename, autocrop)
{
	first=0;
	LoadImageFile( filename );

	if (autocrop&&!fastinfo_flag)	CropImage();
}

GifImage::GifImage() {
	first=0;
}

GifImage::~GifImage() {
	if (first)	delete first;
}

const char *GifImage::GetExtensionData( unsigned char code ) {
Extension	*current;

	for (current=first;current;current=current->next) {
		if (current->code==code)		return current->data;
	}
	return 0;
}

#define IMAGESEP 0x2c
#define EXTENSION 0x21
#define INTERLACEMASK 0x40
#define COLORMAPMASK 0x80
#define ADDTOPIXEL(a) if (Quick) data[Offset++]=a; else AddToPixel(a)

#define ALLOCATED 3

static int BitOffset,	/* Bit Offset of next code */
	XC, YC,					/* Output X and Y coords of current pixel */
	Offset,					/* Offset in output array */
	Pass,						/* Used by output routine if interlaced pic */
	BytesPerScanline,		/* bytes per scanline in output raster */
	NumUsed,					/* Number of colors really used */
	CodeSize,				/* Code size, read from GIF header */
	ReadMask;				/* Code AND mask for current code size */

static boolean Interlace;

static byte *Raster;		/* The raster data stream, unblocked */

	/* The GIF headers found in the file */
static byte gifheader[13];
static byte imageheader[9];
static byte colormap[3*256];

	/* The hash table used by the decompressor */
static int  Prefix[4096];
static int  Suffix[4096];

	/* An output array used by the decompressor */
static byte OutCode[1025];

	/* The color map, read from the GIF header */
static byte used[256];

static char id[] = "GIF";


/*****************************/
int GifImage::LoadImageFile(const char *fname)
/*****************************/
{
	register byte ch;
	FILE *fp;

	BitOffset = 0,				/* Bit Offset of next code */
	XC = 0, YC = 0,				/* Output X and Y coords of current pixel */
	Offset = 0,				 /* Offset in output array */
	Pass = 0;						/* Used by output routine if interlaced pic */
	ColorMapSize = 0;
	data = NULL;

	fp = fopen(fname,"rb");

	if (!fp) {
			fprintf(stderr,"'%s': File not found\n", fname);
			exit(0);
	}

	if ( (fread(gifheader, sizeof(gifheader), 1, fp)!=1)
	  || ( (strncmp((char*)gifheader, id, 3)!=0)
	    && (strncmp((char*)gifheader, "FIG", 3)!=0) ) )
	  		{		fprintf(stderr,"'%s' not a GIF or JPG file\n", fname );
	  				exit(1);
	  		}

	if (strncmp((char*)gifheader+3, "87a", 3) && strncmp((char*)gifheader+3,"89a",3))
		fprintf(stderr,"Warning: %s contains unknown version %c%c%c",fname,
		   gifheader[3],gifheader[4],gifheader[5]);
	HasColormap = ((gifheader[10] & COLORMAPMASK) ? True : False);
	ColorMapSize = 1 << (gifheader[10]&7)+1;

	Background = gifheader[11];				/* background color... not used. */

/* Read in global colormap. */
	if (HasColormap) ReadColormap(fp);

/* Check for image extension */
	while ((ch=getc(fp)) == EXTENSION)
	{
		char	buffer[256];

		first=new Extension(getc(fp),first);
		while ((ch=getc(fp))>0) {
			fread(buffer,ch,1,fp);
			first->AddData(buffer,ch);
		}
	}
		
	if (ch != IMAGESEP) {
		fprintf(stderr,"'%s': corrupt GIF file (no image separator) '%x'\n", fname, ch);
		return 1;
	}

	fread(imageheader,sizeof(imageheader),1,fp);

	width		= imageheader[4] + 0x100 * imageheader[5];
	height	= imageheader[6] + 0x100 * imageheader[7];

	if (!quiet || fastinfo_flag) {
		printf("%s: %d x %d x %d\n", fname, Width(), Height(), ColorMapSize);
		fclose(fp);
		return 1;
	}

	Interlace = ((imageheader[8] & INTERLACEMASK) ? True : False);

	if (imageheader[8] & COLORMAPMASK) 
	{
		HasColormap = True;
		ColorMapSize = 1 << (imageheader[8]&7)+1;
		ReadColormap(fp);
	}
	CodeSize = getc(fp); 
	ReadImageData(fp);
	fclose(fp);
	DecodeImage();
	return 0;
}


int GifImage::ReadImageData(FILE *fp)
{
/* Read the raster data.  Here we just transpose it from the GIF array
 * to the Raster array, turning it from a series of blocks into one long
 * data stream, which makes life much easier for ReadCode().
 */
	long filesize, filepos;
	int ch;
	byte *ptr1;

	/* find the size of the file */
	filepos = ftell(fp);
	fseek(fp, 0L, 2);
	filesize = ftell(fp)-filepos;
	fseek(fp, filepos, 0);

	if (!(Raster = (byte *) malloc((unsigned)filesize))) {
			fprintf(stderr,"Not enough memory to store gif data");
			exit(-1);
	}

	ptr1 = Raster;
	while ((ch = getc(fp))>0)
	{
		if (fread(ptr1, 1, ch, fp)<(unsigned)ch)
				fprintf(stderr,"corrupt GIF file (unblock)\n");
		ptr1 += ch;
	}
	return 0;
}

int  GifImage::DecodeImage()
{
/* Start reading the raster data. First we get the intial code size
 * and compute decompressor constant values, based on this code size.
 */
int Quick,						/* True, when not interlaced and local Cmap */
	InitCodeSize,				/* Starting code size, used during Clear */
	InCode,						/* Value returned by ReadCode */
	MaxCode,						/* limiting value for current code size */
	ClearCode,					/* GIF clear code */
	EOFCode,						/* GIF end-of-information code */
	CurCode, OldCode = 0,	/* Decompressor variables */
	FreeCode,					/* Decompressor, next free slot in hashtable */
	OutCount = 0,				/* Decompressor output 'stack count' */
	FinChar = 0,				/* Decompressor variable */
	BitMask;						/* AND mask for data size */

	BitMask = ColorMapSize - 1;

	ClearCode = (1 << CodeSize);
	EOFCode = ClearCode + 1;
	FreeCode = ClearCode + 2;

/* The GIF spec has it that the code size is the code size used to
 * compute the above values is the code size given in the file, but the
 * code size used in compression/decompression is the code size given in
 * the file plus one. (thus the ++).
 */

	CodeSize++;
	InitCodeSize = CodeSize;
	MaxCode = (1 << CodeSize);
	ReadMask = MaxCode - 1;

/* Allocate the X Image */
	if (!(data = (byte *) malloc(Width()*Height()))) {
		fprintf(stderr,"not enough memory for image");
		exit(-1);
	}

#if (0)
	if (!(theImage = XCreateImage(theDisp, theVisual, 8, ZPixmap, 0, (char*)Image,
							 Width(), Height(), 8, Width()))) {
		fprintf(stderr,"unable to create XImage");
		return -1;
	}
#endif

	BytesPerScanline = Width();

/* Decompress the file, continuing until you see the GIF EOF code.
 * One obvious enhancement is to add checking for corrupt files here.
 */
	Quick = !Interlace;
	Offset = 0; 
	if (DEBUG) fprintf(stderr,"Decoding...\n");
	InCode = ReadCode();
	while (InCode != EOFCode) {

/* Clear code sets everything back to its initial value, then reads the
 * immediately subsequent code as uncompressed data.
 */

		if (InCode == ClearCode) {
			CodeSize = InitCodeSize;
			MaxCode = (1 << CodeSize);
			ReadMask = MaxCode - 1;
			FreeCode = ClearCode + 2;
			CurCode = OldCode = InCode = ReadCode();
			FinChar = CurCode & BitMask;
			ADDTOPIXEL(FinChar);
		}
		else {

/* If not a clear code, then must be data: save same as CurCode */

			CurCode = InCode;

/* If greater or equal to FreeCode, not in the hash table yet;
 * repeat the last character decoded
 */

			if (CurCode >= FreeCode) {
				CurCode = OldCode;
				OutCode[OutCount++] = FinChar;
			}

/* Unless this code is raw data, pursue the chain pointed to by CurCode
 * through the hash table to its end; each code in the chain puts its
 * associated output code on the output queue.
 */

			while (CurCode > BitMask) {
				if (OutCount >= 1024) {
					fprintf(stderr,"\nCorrupt GIF file (OutCount)!\n");
					exit(1);  
				}
				OutCode[OutCount++] = Suffix[CurCode];
				CurCode = Prefix[CurCode];
			}

/* The last code in the chain is treated as raw data. */

			/* OutCode[OutCount++] = FinChar = CurCode &BitMask*/;
			FinChar = CurCode & BitMask;
			ADDTOPIXEL(FinChar);

/* Now we put the data out to the Output routine.
 * It's been stacked LIFO, so deal with it that way...  */
			while (OutCount>0)
				ADDTOPIXEL(OutCode[--OutCount]);

/* Build the hash table on-the-fly. No table is stored in the file. */

			Prefix[FreeCode] = OldCode;
			Suffix[FreeCode] = FinChar;
			OldCode = InCode;

/* Point to the next slot in the table.  If we exceed the current
 * MaxCode value, increment the code size unless it's already 12.  If it
 * is, do nothing: the next code decompressed better be CLEAR
 */

			FreeCode++;
			if (FreeCode >= MaxCode) {
				if (CodeSize < 12) {
					CodeSize++;
					MaxCode *= 2;
					ReadMask = (1 << CodeSize) - 1;
				}
			}
		}
		InCode = ReadCode();
	}
	free(Raster);
	return 0;
}


/* Fetch the next code from the raster data stream.  The codes can be
 * any length from 3 to 12 bits, packed into 8-bit bytes, so we have to
 * maintain our location in the Raster array as a BIT Offset.  We compute
 * the byte Offset into the raster array by dividing this by 8, pick up
 * three bytes, compute the bit Offset into our 24-bit chunk, shift to
 * bring the desired code to the bottom, then mask it off and return it. 
 */
int GifImage::ReadCode()
{
	int RawCode, ByteOffset, BitShift;

	ByteOffset = BitOffset / 8;
	BitShift = BitOffset % 8;
	BitOffset += CodeSize;
	if (BitShift+CodeSize<8)
		return (Raster[ByteOffset]>>BitShift) & ReadMask;
	else
	{
		RawCode = Raster[ByteOffset] + (0x100 * Raster[ByteOffset + 1]);
		if (BitShift+CodeSize >= 16)
			RawCode += (0x10000 * Raster[ByteOffset + 2]);
		return((RawCode>>BitShift) & ReadMask);
	}
}


void GifImage::AddToPixel(byte Index)
{
	if (YC<Height()) /* Might be of importance when reading interlaced gifs */
		data[YC*BytesPerScanline+XC] = Index;
	if (!used[Index]) { used[Index]=True; NumUsed++; }
	if (++XC == Width())
	{
		XC = 0;
		if (Interlace)
		{
			switch (Pass) 
			{
			case 0: YC += 8; if (YC >= Height()) { Pass++; YC = 4; } break;
			case 1: YC += 8; if (YC >= Height()) { Pass++; YC = 2; } break;
			case 2: YC += 4; if (YC >= Height()) { Pass++; YC = 1; } break;
			case 3: YC += 2; break;
			default: break;
			}
		}
		else
			YC++;
	}
}



void GifImage::ReadColormap(FILE *fp)
{
	byte *ptr=colormap;
	int i;

	if (DEBUG) fprintf(stderr,"Reading Color map...\n");
	fread(colormap, ColorMapSize, 3, fp);
	for (i = 0; i < ColorMapSize; i++) {
		Red[i] = (*ptr++);
		Green[i] = (*ptr++);
		Blue[i] = (*ptr++);
		used[i] = 0;
	}
	NumUsed=0;
}
