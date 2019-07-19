
// This include is meant to be the body of the function ResetXX,
// which is implemented exactly the same for different resolutions. For
// compatibility reasons this is controlled by Defines instead of templates.
// Therefore the following type definitions are set to the required values,
// while compiling that function:
//
// #define DATA_TYPE unsigned char
// #define DATA_PAD  1


DATA_TYPE	*xdata;

// create buffer for Image-Data
// there is an optimization in the rotation-routine, which sometimes tries
// to access data beyond the allocated image, that might lead to a segmentation
// violation. Therefore, it might be good to allocated some additional
// rows of data for the image.
	offset_bytes=xwidth*offset_rows*sizeof(DATA_TYPE);
	xdata=new DATA_TYPE[xwidth*(xheight+2*offset_rows)];
	{	DATA_TYPE	*xdata_run=xdata;
		unsigned long blk_pixel=BlackPixel(dpy,scr);
		for (int i=xwidth*(xheight+2*offset_rows);i>0;i--) {
			*xdata_run++=(DATA_TYPE)blk_pixel;
		}
	}
	xdata+=(offset_bytes/sizeof(DATA_TYPE));

	if (!xdata) {
		fprintf(stderr,"not enough memory for XImage-data");
		exit(-1);
	}

// create the XImage
	ximage = XCreateImage(dpy, DefaultVisual(dpy,scr),
				DefaultDepth(dpy,scr), ZPixmap, 0,
				(char*)xdata, xwidth, xheight, 8*DATA_PAD, xwidth*sizeof(DATA_TYPE));

	if (!ximage) {
		fprintf(stderr,"\n*** can't allocate ximage.\n" );
		exit(0);
	}

// copy data from original image and inserting pixel values on the fly
	if (Width()==xwidth&&Height()==xheight) {
		register DATA_TYPE	*copy = xdata;
		register const byte	*org  = Data();
		register int	j,i;

		if(DataIsRGB()) {
			for (i=0; i<Height(); i++) 
				for (j=0; j<Width(); j++, copy++) {
#if DATA_PAD == 4
					*copy = (*org++);
					*copy = (*copy << 8) | (*org++);
					*copy = (*copy << 8) | (*org++);
#else
					*copy = ((*org++) >> 3);
					*copy = (*copy << 6) | ((*org++) >> 2);
					*copy = (*copy << 5) | ((*org++) >> 3);
#endif
				}
		} else {
			for (i=0; i<Height(); i++) 
				for (j=0; j<Width(); j++, copy++)
					*copy = (DATA_TYPE)gif_cols[*org++];
		}
	}
	else {
		int size = DataIsRGB()?3:1;
		for (int y=0;y<xheight;y++) {
			register const byte	*org  = Data() + (y*Height()/xheight) * Width()*size;
			register DATA_TYPE *copy = xdata + y * xwidth;
	
			if (xwidth<Width()) {
				register int x;
				register int delta = Width()/2;
	
				for (x=Width();x>0;x--) {
					delta-=xwidth;
					if (delta<0) {
						delta+=Width();
						if(size==3) {
#if DATA_PAD == 4
							*copy = (*org);
							*copy = (*copy << 8) | (org[1]);
							*copy = (*copy << 8) | (org[2]);
#elif DATA_PAD == 2
							*copy = ((*org) >> 3);
							*copy = (*copy << 6) | ((org[1]) >> 2);
							*copy = (*copy << 5) | ((org[2]) >> 3);
#endif
							copy++;
						} else 
							*copy++ = (DATA_TYPE)gif_cols[*org];
					}
					org+=size;
				}
			}
			else {
				register int x;
				register int delta = xwidth/2;
	
				for (x=xwidth;x>0;x--) {
					delta-=Width();
					if(size==3) {
#if DATA_PAD == 4
						*copy = (*org);
						*copy = (*copy << 8) | (org[1]);
						*copy = (*copy << 8) | (org[2]);
#elif DATA_PAD == 2
						*copy = ((*org) >> 3);
						*copy = (*copy << 6) | ((org[1]) >> 2);
						*copy = (*copy << 5) | ((org[2]) >> 3);
#endif
						copy++;
					} else 
						*copy++ = (DATA_TYPE)gif_cols[*org];
					if (delta<0) {
						delta+=xwidth;
						org+=size;
					}
				}
			}
		}
	}

	ximage->data = (char*)xdata;

//	XPutImage(dpy,pixmap,p->gc_all,ximage,0,0,0,0,xwidth,xheight);
