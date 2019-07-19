
// This include is meant to be the body of the function CreateTilemapXX,
// which is implemented exactly the same for different resolutions. For
// compatibility reasons this is controlled by Defines instead of templates.
// Therefor the following type definitions are set to the required values,
// while compiling that function:
//
// #define DATA_TYPE unsigned char
// #define DATA_PAD  1

#define	IMAX	1000

XImage		*ximage;
Vec2			wcenter;
Vec2			dirx,diry,edge;

	if (page) {
			if (pm->IsTwinPixmap())
					wcenter=Vec2( center.X(), pm->XHeight()+center.Y() );
			else	wcenter=Vec2( center.X(), center.Y() );
	}
	else		wcenter=center;

	ximage = img_buf->Init(width,height,DATA_PAD);
if(page && tile_solid_back) {
	for(int y=0; y<height; y++) {
		register DATA_TYPE	*dest = (DATA_TYPE*)(ximage->data + y*ximage->bytes_per_line);
		for(int x=0; x<width; x++) {
			*dest++ = tile_back_color;
		}
	}
} else {
	if (!itm) {
		if (page) {
			dirx=Vec2(-1,0).TurnAngleDeg(windir);
			diry=Vec2(1,0).TurnLeft().TurnAngleDeg(windir);
		}
		else {
			dirx=Vec2(1,0).TurnAngleDeg(-windir);
			diry=Vec2(1,0).TurnLeft().TurnAngleDeg(-windir);
		}
		edge=wcenter-offx*dirx-offy*diry;
	}
	else {
		dirx=(*itm)*Vec2(1,0);
		diry=(*itm)*Vec2(0,1);
		edge=wcenter+(*itm)*Vec2(-offx,-offy);
	}

#if (0)

//
// the traditional routine to copy each pixel from one image to the other
// with the generic routines. range checking is done in pm's GetPixel() and
// there's nothing to worry about (except the time...)
//

	pm->GetImage();		// to make sure the pixmap is valid

	for (int y=0;y<height;y++) {
		Vec2	pt=edge+y*diry;
		/* x,y, dx,dy bestimmen ... */
		register DATA_TYPE	*dest = (DATA_TYPE*)(ximage->data + y * ximage->bytes_per_line);
		for (int x=0;x<width;x++) {
			*dest++ = pm->GetPixel( XPix(pt.X()), YPix(pt.Y()) );
			pt+=dirx;
		}
	}

#else

//
// optimized mapping
// in a loop in brezenham fashion, each pixel is copied from one buffer to the
// other with direct access to the data structure. Since it would be inefficient
// to an access outside of the source image on every pixel, only the 4 edges are
// check, if the boundary is crossed, a traditional copy with range checking is
// done. This happens very seldom, since the buffer for the picture is
// large than it has to be, so that only a few flip's will really fall
// outside ...

XImage *src_image=pm->GetImage();

#ifndef RANGE_CHECK
{
char *src;
char	*pic_addr=src_image->data;					// image start
char	*min_addr=pic_addr-pm->offset_bytes;	// memory start (including offset)
char	*max_addr=pic_addr+src_image->bytes_per_line*src_image->height
						+pm->offset_bytes;				// memory end (behind offset)
	int	count=0;

	Vec2	t=edge;
		src = (char*)((DATA_TYPE*)(pic_addr+YPix(t.Y())*src_image->bytes_per_line)+XPix(t.X()));
		if (src<min_addr||src>=max_addr)		count++;
		t=edge+height*diry;
		src = (char*)((DATA_TYPE*)(pic_addr+YPix(t.Y())*src_image->bytes_per_line)+XPix(t.X()));
		if (src<min_addr||src>=max_addr)		count++;
		t=edge+width*dirx;
		src = (char*)((DATA_TYPE*)(pic_addr+YPix(t.Y())*src_image->bytes_per_line)+XPix(t.X()));
		if (src<min_addr||src>=max_addr)		count++;
		t=edge+height*diry+width*dirx;
		src = (char*)((DATA_TYPE*)(pic_addr+YPix(t.Y())*src_image->bytes_per_line)+XPix(t.X()));
		if (src<min_addr||src>=max_addr)		count++;

	if (count) {
		DBG0( "*** range overflow -> trying slow mapping\n" );

		for (int y=0;y<height;y++) {
			Vec2	pt=edge+y*diry;
			/* x,y, dx,dy bestimmen ... */
			register DATA_TYPE	*dest = (DATA_TYPE*)(ximage->data + y * ximage->bytes_per_line);
			for (int x=0;x<width;x++) {
				*dest++ = (DATA_TYPE)pm->GetPixel( XPix(pt.X()), YPix(pt.Y()) );
				pt+=dirx;
			}
		}
		img_buf->PutImage(dpy,tilemap,DefaultGC(dpy,scr),0,0,0,0,width,height);
		return;
	}
}
#endif

int pixels_per_line=src_image->bytes_per_line/sizeof(DATA_TYPE);

	register int dx=(int)(dirx.X()*IMAX);
	register int dy=(int)(dirx.Y()*IMAX);

	for (int y=0;y<height;y++) {
		Vec2	pt=edge+y*diry;
		/* x,y, dx,dy bestimmen ... */
		register int	dx_c  = ((dx>0)?dx:-dx)/2;
		register int	dy_c  = ((dy>0)?dy:-dy)/2;
#ifdef RANGE_CHECK
		register int	src_x = XPix(pt.X());
		register int	src_y = YPix(pt.Y());
#	define	INC_X	src_x++
#	define	DEC_X	src_x--
#	define	INC_Y	src_y++
#	define	DEC_Y	src_y--
#else
#	define	INC_X
#	define	DEC_X
#	define	INC_Y
#	define	DEC_Y
#endif

		register DATA_TYPE *src = (DATA_TYPE*)(src_image->data + YPix(pt.Y()) * src_image->bytes_per_line)+XPix(pt.X());
		register DATA_TYPE *dest= (DATA_TYPE*)(ximage->data + y * ximage->bytes_per_line);
		if (dx>=0) {
			if (dy>=0) {
				for (int x=0;x<width;x++) {
#ifdef RANGE_CHECK
					if (src_x<0||src_x>=src_image->width||src_y<0||src_y>=src_image->height) {
						*dest++ = 0;
					}
					else
#endif
						*dest++ = *src;

					dx_c-=dx;
					while (dx_c<0) { dx_c+=IMAX; INC_X; src++; }
					dy_c-=dy;
					while (dy_c<0) { dy_c+=IMAX; INC_Y; src+=pixels_per_line; }
				}
			}
			else {
				for (int x=0;x<width;x++) {
#ifdef RANGE_CHECK
					if (src_x<0||src_x>=src_image->width||src_y<0||src_y>=src_image->height) {
						*dest++ = 0;
					}
					else
#endif
						*dest++ = *src;

					dx_c-=dx;
					while (dx_c<0) { dx_c+=IMAX; INC_X; src++; }
					dy_c+=dy;
					while (dy_c<0) { dy_c+=IMAX; DEC_Y; src-=pixels_per_line; }
				}
			}
		}
		else {
			if (dy>=0) {
				for (int x=0;x<width;x++) {
#ifdef RANGE_CHECK
					if (src_x<0||src_x>=src_image->width||src_y<0||src_y>=src_image->height) {
						*dest++ = 0;
					}
					else
#endif
						*dest++ = *src;

					dx_c+=dx;
					while (dx_c<0) { dx_c+=IMAX; DEC_X; src--; }
					dy_c-=dy;
					while (dy_c<0) { dy_c+=IMAX; INC_Y; src+=pixels_per_line; }
				}
			}
			else {
				for (int x=0;x<width;x++) {
#ifdef RANGE_CHECK
					if (src_x<0||src_x>=src_image->width||src_y<0||src_y>=src_image->height) {
						*dest++ = 0;
					}
					else
#endif
						*dest++ = *src;

					dx_c+=dx;
					while (dx_c<0) { dx_c+=IMAX; DEC_X; src--; }
					dy_c+=dy;
					while (dy_c<0) { dy_c+=IMAX; DEC_Y; src-=pixels_per_line; }
				}
			}
		}
	}
#endif
}
	img_buf->PutImage(dpy,tilemap,DefaultGC(dpy,scr),0,0,0,0,width,height);

#undef IMAX
