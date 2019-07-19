#ifndef	_global_h
#define	_global_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

//
// The basic floating point class can be exchanged between doubles and
// floats. The latter one is faster on my 386.
#ifndef __TURBOC__
#	define	REAL_IS_FLOAT	0
#else
#	define	REAL_IS_FLOAT	1
#endif

//
// A real C++-Class can be used for real arithmetic. Unfortunately
// that really slows the calculation down, even though the whole class
// is defined inline.
#define	REAL_IS_CLASS	0

//
// There are some specialized vector classes for 2 and 3 dimensionaL
// vectors, which can also be realized by inheriting from an universaL
// vector-class (but again, that's expensive)
#define	Vec2IsVector	0
#define	Vec3IsVector	0

//
// constants to overcome the problem with unprecise real-arithmetics
#if (REAL_IS_FLOAT)
#	define	EPS		 	1e-4
#else
#	define	EPS		 	1e-10
#endif

#ifdef DEBUG

#define  DBG0(f)         printf( f )
#define  DBG1(f,a)       printf( f,a )
#define  DBG2(f,a,b)     printf( f,a,b )
#define  DBG3(f,a,b,c)   printf( f,a,b,c )
#define  DBG4(f,a,b,c,d) printf( f,a,b,c,d )

#else

#define  DBG0(f)
#define  DBG1(f,a)
#define  DBG2(f,a,b)
#define  DBG3(f,a,b,c)
#define  DBG4(f,a,b,c,d)

#endif


//
// here come something very unlike OO, but its just easier ...
// all common variables are defined in the xpuz.cpp file
//
extern Display	*dpy;					// the display connection
extern int		scr;					// the current screen
extern Window	win;					// the main window (can be root in shape-mode)
extern GC		gc;					// the main graphic context

extern int verbose;
extern int texture_mode;				// mode for texture mapping depending on depth

extern Cursor	normal_cursor, move_cursor, pull_cursor, idle_cursor, no_cursor;

extern int	zoom_factor;				// current zooming stage (default: 20)
extern int	win_size_x;
extern int	win_size_y;

extern int	offx;							// half tilesize as offset to frames
extern int	offy;
extern int	width;						// height of image
extern int	height;						// width of image
extern int	dx;							// number of tiles in x-direction
extern int	dy;							// number of tiles in y-direction
extern int	tile_size;					// average tile size
extern int	tile_solid_back;				// tile has solid back color instead of image
extern int	tile_back_color;				// color of the back when solid
extern int  any_angle;                                      // use any angle when not fixed rotation
extern int	shared;						// flag about usage of MIT-SHM


extern int	shadow_size;				// pixels in shadow frame

extern double	fliptimebase;			// base time for flipping
extern double	fliptimedelta;			// added to base for each tile
extern int		maxfliptiles;			// max. number of tiles for automated flip
extern int		minadjustcount;		// number of tiles to start 90 degrees autoadjust
extern double	flipsave;				// dont let the tile come close to a vertical
      		                        // position during the flip ...

extern double	turntimebase;			// base time for 90 degree rotation
extern double	turntimedelta;			// added to base for each additional tile
extern int		maxturntiles;			// max. number of tiles for rotation animation

extern int		maxsnapretries;		// max. possible retries to snap the snapped

extern class Puzzle			*p;		// Collection of all puzzle pieces
extern class XjigPixmap		*pm;		// Original pixmap for the puzzle tiles
extern class Port			*port;	// Port (Display synonym) for color mapping
extern class ObjectStack	*stk;		// administrator object for all viewable objects
extern class ImageBuffer	*img_buf;//	memory for rotating image (probably shared)


extern double spx[10];	// Spline-Positions for any Pin
extern double spy[10];

#define WARP_NO_LOCK -2000
extern int	warp_center;				// help information to safely warp the pointer
extern int	warp_lock_x;
extern int	warp_lock_y;

extern int	side_lock;                      // which side (of TwinPixmap) as default
extern int btn_swap;                       // swap the left and right mouse buttons
extern int fix_angle;                      // fix the rotation angle to 90 deg     
extern int piecestyle;			//sets the style of the pieces 0 to 8		
extern int mcs;                            //set the mouse click speed
extern int no_rotation;                             // no rotation

extern int	distortion;					// factor to control distortion of the tiles
extern double maxang;					// maxmum offset angle at startup
extern int	shuffle;						// shuffle tile as default
extern int	straight_setup;			// offset for straight debugging setup
extern int	angle;						// preset angles for debugging
extern int sortedges;                          // sort edges
extern int	quit;				// global flag to initiate quitting

extern double GetCurrentTime(int busy=0);		// to query current time
extern int my_rand(void);					// private randomizer

#define XPix(x)   ((int)(x))
#define YPix(y)   ((int)(y))
#define AnyButtonMask (Button1Mask|Button2Mask|Button3Mask|Button4Mask|Button5Mask)

#endif
