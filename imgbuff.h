
#ifndef _imgbuff_h
#define _imgbuff_h

#ifdef USE_MIT_SHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

class ImageBuffer {
	public:
		ImageBuffer();
		~ImageBuffer();

		XImage *Init(int w, int h, int bpp8);
		XImage *GetXImage()		{ return ximage; }

		void PutImage(Display *dpy,Drawable d,GC gc,int src_x, int src_y, int dest_x, int dest_y, unsigned int w, unsigned int h) {
#ifdef USE_MIT_SHM
			if (shm) {
					XShmPutImage(dpy,d,gc,ximage,src_x,src_y,dest_x,dest_y,w,h,False);
					XSync(dpy,0);
			}
			else
#endif
					XPutImage(dpy,d,gc,ximage,src_x,src_y,dest_x,dest_y,w,h); }

	private:
		void AllocData(int w,int h,int bpp8);
		void FreeData();

		XImage				*ximage;
		int					width, height;

#ifdef USE_MIT_SHM
		int					shm;				// flag, if shared memory is in use
		XShmSegmentInfo	shminfo;			// shm information
#endif
};

#endif
