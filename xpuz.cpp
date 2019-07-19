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
    
#include "global.h"

#include <time.h> // for time()
#include <sys/types.h> // for getpid()
#include <unistd.h> // for getpid()

#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include <X11/Xos.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>

#ifndef X_GETTIMEOFDAY
    /* define X_GETTIMEOFDAY macro, a portable gettimeofday() */
    /* copied from Xos.h for pre-X11R6 releases               */
#   if defined(SVR4) || defined(VMS) || defined(WIN32)
#       define X_GETTIMEOFDAY(t) gettimeofday(t)
#   else
#       define X_GETTIMEOFDAY(t) gettimeofday(t, (struct timezone*)0)
#   endif
#endif
#ifndef FDS_TYPE
//
// The problem occured, that on HP-UX, the type used in the masks of the
// select() system call are not the usual 'fd_set'. Instead they are just
// integer pointers, which leads to an compile-error, when not casted.
// Therefore it actually will get casted by the following Macro FDS_TYPE,
// which should usually be defined to an empty string.
//
#  ifdef __hpux
#     define   FDS_TYPE (int*)
#  else
#     define   FDS_TYPE
#  endif
#endif

#ifndef _objects_h
#   include "objects.h"
#endif

#include "ximage.h"
#include "color_mapper.h"
#include "imgbuff.h"
#include "puzzle.h"

#include "cursor.h"

#define _DEBUG
#include "global.h"


Display *dpy;
int     scr;
Window  win;
GC          gc;
const char *xpuzver = "2.6.4";
int texture_mode=0;             // mode for texture mapping depending on depth

int stylecount =11;
Cursor  normal_cursor, move_cursor, pull_cursor, idle_cursor, no_cursor;
  // 0=normal,1=squareish,2=jagged,3=scrap,4=square,5=spiky,6=basic,7=peggy,8=strips,9=wavy,10=saw,11=stub
double spx_style[20] [10]= {{ 1, 2.5, 5, 7, 7, 5, 5, 6, 8,8 },     // default
                           { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 },       // squareish  
                           { 1, 1, 1, 1, 1, 13, 13, 13, 13, 13 },  // jagged
                           { 0, 0, 4, 8, 16, 16, 8, 4, 0,0 },      // scrap
                           { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },       // square
                           { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},        // spiky
                           { 1, 2.5, 5, 7, 7, 5, 5, 6, 8,8 },      // basic
                           { 1, 2.5, 5, 7, 7, 5, 5, 6, 8,8 },      // peggy  
                           { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },       // strips
                           { 0,2,6,8,10,12,14,16,18,20 },          // wavy
                           {0,3.5,7.5,11.5,15.5,19.5,20,20,20,20 },  // saw
                           { 0,1,2,3,4,5,6,8,10,13 }};             // stub
                              
double spy_style[20] [10]= {{ 13,12.5,11, 8, 5, 2, 0,-2,-3,-3 },   // default
                           { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 },       // squareish 
                           { 13, 13,13, 13, 13, 1, 1, 1, 1, 1 },   // jagged
                           { 0, 0, 4, 8, 16, 16, 8, 4, 0,0 },      // scrap
                           { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },       // square
                           { 13,13,13,13,13,13,13,13,13,13 },      // spiky
                           {13,12.5,11, 8, 5, 2, 0,0,0,0 },        // basic
                           {13,13,13, 13, 13, 13, 0,0,0,0 },       // peggy  
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },        // strips
                           {6,6,5.5,5,4.5,4,3,2,1,1 },             // wavy
                           {5.5,0,5.5,0,5.5,0,0,0,0,0 },           // saw
                           {7.5,7.25,7,6.5,6,5,4,0,0,0 }};         // stub
                                 
double spx[] = { 1, 2.5, 5, 7, 7, 5, 5, 6, 8,13 };  
double spy[]= { 13,12.5,11, 8, 5, 2, 0,-2,-3,-3 };

int     zoom_factor=20;
int     win_size_x=100;
int     win_size_y=100;

int     offx=0;                 // half tilesize as offset to frames
int     offy=0;
int     width =0;               // width of image
int     height=0;               // height of image
int     dx = 0;                 // number of tiles in x-direction
int     dy = 0;                 // number of tiles in y-direction
int     tile_size=80;            // average tile size
int     shuffle=3;              // shuffle tile as default
int     side_lock=-1;           // which side (of TwinPixmap) as default
int		tile_solid_back=0;			// tile has solid back color instead of image
int		tile_back_color=0x2b8475;	// color of the back when solid
int     btn_swap=1;              // swap left and right mouse buttons
int     fix_angle=0;             // fix the angle of piece rotation to 90 deg
int     piecestyle = 0;           // set the style to normal
int     mcs=200;                  // mouse click speed
int     no_rotation = 0;          // no rotation  
int     sortedges=0;              // sort edges
int     any_angle=0;              // use any angle when not fixed rotation
int		no_qk=0;		          // dont show quick keys
int 	loadgame=0;
int 	bestfit=0;
#ifdef __hpux
int     shared=0;               // dont use extension as default
#else
int     shared=1;               // use MIT-SHM extension as default
#endif


int     quit=0;                 // global flag to initiate quitting
int     random_seed=0;
int     distortion=20;          // factor to control distortion of the tiles
double  maxang=45;              // maximum rotation angle (off from 90degrees)
int     shadow_size=1;          // pixels in shadow frame
int     straight_setup=-1;      // offset for straight debugging setup
int     no_flip=1;              // suppress automatic flip of landscape images
int     autocrop=0;             // try to cut the edges away

double  fliptimebase=0.07;  // base time for flipping
double  fliptimedelta=0.02; // added to base for each tile
int     maxfliptiles=8;     // max. number of tiles for automated flip
int     minadjustcount=4;       // number of tiles to start 90 degrees autoadjust
double  flipsave=0.2;           // dont let the tile come close to a vertical
                              // position during the flip ...

double  turntimebase=0.15;  // base time for 90 degree rotation
double  turntimedelta=0.02; // added to base for each additional tile
int     maxturntiles=8;     // max. number of tiles for rotation animation

int     maxsnapretries=1;       // max. no. of tiles that could recursively snap together

int     warp_center;
int     warp_lock_x=WARP_NO_LOCK;
int     warp_lock_y=WARP_NO_LOCK;

const char        *dispname="";
const char        *filename="";
const char        *bgcol="";
const char        *playername="";
const char        *gamename="";
const char        *xpuzdir=".xpuz";

 char        tpc[8];
 char        btc[8];
 char        bgc[8];
//const char        *homefolder=getenv ("HOME");
int     verbose=0;
int     rotate=0;               // rotation demo for debugging
int     angle=0;                    // preset angles for debugging

int    gamepaused=0;
long   pausetime=0;
int    gamepieces=0;
long   gameseconds;
const char  *gamestart="";
char  *homefolder=getenv ("HOME");
const char  *scorefilename="";

class PieceFrameSetup *pfs;
class Puzzle        *p;         // Collection of all puzzle pieces
class XjigPixmap    *pm;            // Original pixmap for the puzzle tiles
class Port          *port;      // Port (Display synonym) for color mapping
class ObjectStack   *stk;           // administrator object for all viewable objects
class ImageBuffer   *img_buf;   //  memory for rotating image (probably shared)


// ===========================================================================

static void my_srand( unsigned seed ) {
    srand(seed);
}

int my_rand(void) {
    return rand();
}

// ===========================================================================

static void local_usleep( long time )
{
struct timeval  timeout;
int    nfound;

   timeout.tv_sec  = (long)0;
   timeout.tv_usec = (long)time;

   nfound=select(0,0,0,0,&timeout);
}

/*static*/ void do_sound( char *str ) {
XKeyboardState      old_keyboard_values;
XKeyboardControl    values;
int     pitch, percent,duration,pause;
char        *str_p=str;

    XGetKeyboardControl(dpy,&old_keyboard_values);
    while( sscanf(str_p,"%03d%02d%02d%02d;",&pitch,&percent,&duration,&pause)==4 ) {
        values.bell_pitch=pitch;
        values.bell_percent=percent;
        values.bell_duration=duration;
        XChangeKeyboardControl(dpy,KBBellPercent|KBBellPitch|KBBellDuration,&values);
        XBell(dpy,values.bell_percent);
        XFlush(dpy);
        local_usleep(pause*10000);
        str_p+=10;
    }
#ifdef __hpux
    values.bell_pitch    = old_keyboard_values.bell_pitch;
    values.bell_percent  = old_keyboard_values.bell_percent;
    values.bell_duration = old_keyboard_values.bell_duration;
#else
    values.bell_pitch    = 440;
    values.bell_percent  =  50;
    values.bell_duration = 100;
#endif
    XChangeKeyboardControl(dpy,KBBellPercent|KBBellPitch|KBBellDuration,&values);
    XFlush(dpy);
}

// ===========================================================================
//
// some help routines for easy drawing ...
//


void DrawLine( const Real& x1, const Real& y1, const Real& x2, const Real& y2 ) {
int   px1 = XPix(x1);
int   py1 = YPix(y1);
int   px2 = XPix(x2);
int   py2 = YPix(y2);
    XDrawLine( dpy, win, gc, px1, py1, px2, py2 );
}
inline void DrawLine( const Vec2 &p1, const Vec2 &p2 ) {
   DrawLine( p1.X(), p1.Y(), p2.X(), p2.Y() );
}

// ===========================================================================

static unsigned long start_seconds;

static void InitTime() {
struct timeval start;

    X_GETTIMEOFDAY( &start );
    start_seconds = start.tv_sec;
    //gamestart=start_seconds;
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    gamestart=asctime (timeinfo);
}

double GetCurrentTime(int busy) {
struct timeval current;

     X_GETTIMEOFDAY( &current );
#ifdef PINUP_DEFAULT
        static unsigned long last=0;
        static unsigned long idle_start=0;

        unsigned long   val=(current.tv_sec%86400uL);
        if (busy)   idle_start=val+15;
        if (last!=val/5&&val<idle_start) {
            last=val/5;
            unsigned long   minute=val/60+120;
            if (minute<540||(minute>=570&&minute<765)||(minute>=810&&minute<1020)) {
                do_sound( "440995005;370995005;440995005;" );
            }
        }
#else
    (void)busy;
#endif
     return( ((double)(current.tv_sec-start_seconds))+(current.tv_usec/1000000.0) );
}





// =================================================================

class BackDrop : public Object {
    public:
        BackDrop();
        virtual ~BackDrop();

        virtual void ExposeRegion( int x, int y, int width, int height );
        virtual void ZoomView( int midx, int midy, int chg );

    protected:
        void InitFillStyle();

        unsigned long   toppixel;
        unsigned long   bgpixel;
        unsigned long   botpixel;
        Pixmap  tilemap;
        GC          gc_tile;
};

BackDrop::BackDrop() {

    bgpixel =port->AllocNamedColor( bgc );
    toppixel=port->AllocNamedColor( tpc );
    botpixel=port->AllocNamedColor( btc );

    gc_tile=XCreateGC(dpy,RootWindow(dpy,scr),0,0);
    InitFillStyle(); 
}

BackDrop::~BackDrop() {
    XFreeGC(dpy,gc_tile);
    XFreePixmap(dpy,tilemap);
}

void BackDrop::InitFillStyle() {
    tilemap=XCreatePixmap(dpy,RootWindow(dpy,scr),
                                zoom_factor,zoom_factor,DefaultDepth(dpy,scr));
    XSetForeground(dpy,DefaultGC(dpy,scr),bgpixel);
    XFillRectangle(dpy,tilemap,DefaultGC(dpy,scr),0,0,zoom_factor,zoom_factor);
    XSetForeground(dpy,DefaultGC(dpy,scr),toppixel);
    XDrawLine(dpy,tilemap,DefaultGC(dpy,scr),0,0,zoom_factor-2,0);
    XDrawLine(dpy,tilemap,DefaultGC(dpy,scr),0,1,0,zoom_factor-2);
    XSetForeground(dpy,DefaultGC(dpy,scr),botpixel);
    XDrawLine(dpy,tilemap,DefaultGC(dpy,scr),zoom_factor-1,0,zoom_factor-1,zoom_factor-1);
    XDrawLine(dpy,tilemap,DefaultGC(dpy,scr),0,zoom_factor-1,zoom_factor-2,zoom_factor-1);
    XSetTile(dpy,gc_tile,tilemap);
    XSetFillStyle(dpy,gc_tile,FillTiled);
}

void BackDrop::ExposeRegion( int x, int y, int width, int height ) {
    XSetTSOrigin(dpy,gc_tile,-x,-y);
    XFillRectangle(dpy,mystack->dbmap,gc_tile,0,0,width,height);
}

void BackDrop::ZoomView( int /*midx*/, int /*midy*/, int chg ) {
    zoom_factor+=chg;
    XFreePixmap(dpy,tilemap);
    InitFillStyle();
    zoom_factor-=chg;
}

// ===========================================================================

void usage() {
using namespace std;
	cout << "\n\n xpuz " << xpuzver << "\n-----------------------\n";
	cout << "usage  : xpuz <filepath> \n" ;
    printf( "Use a filepath with no options to create a default puzzle from the image in <filepath>. The puzzle will have approx 40 pieces. NOTE: The filepath should always be in quotes.\n\n" );
    
	printf( "Or use options: xpuz <options>\n") ;
   
    printf( "\nOptions available:-\n" );
	printf( "	-play# <filepath>  : combines -file and -skill to create a puzzle from the image in <filepath>.( # = skill level 0 to 9). example: xpuz play3 \"/path/to/myimage.jpg\"\nNOTE: <filepath> should always be in quotes.\n\n" );
    printf( "	-file <filepath>: use image in <filepath> for puzzle\n" );
	printf( "	-skill <n>  : level of skill 0 to 9. example:- xpuz -skill 3 -file \"myimage.jpg\"\n" );	
    printf( "	-pw <n>     : number of pieces wide\n" );
    printf( "	-ph <n>     : number of pieces high\n" );
    printf( "	-ps <n>     : (-piece_size <n>) set average piece size (default %d)\n", tile_size );
    printf( "	-width <n>  : width of image\n" );
    printf( "	-height <n> : height of image\n" );
    printf( "	-side <n>   : single sided puzzle (0 or 1). side 0 is top side.\n" );
	printf( "	-single     : single sided puzzle. \n" );
	printf( "	-double     : double sided puzzle.\n" );
	printf( "	-sb         : use solid color for back of pieces\n" );
	printf( "	-sb_col <n> : color number for back\n" );
    printf( "	-ap         : (-auto_portrait) automatically rotate landscape images to portrait\n" );
    printf( "	-ac         :  (-auto_crop) automatically crop extra space from around images\n" );
    printf( "	-bs         : (-btn_swap) swap the left and right mouse buttons\n" );
    printf( "	-fa         : (-fix_angle) fix angle of pieces to 90 deg\n" );
    printf( "	-p_style    : set style of pieces 0 to %d (default %d)\n",stylecount, piecestyle );
    printf( "	-bgcol      : set the puzzleboard background colour. 18 digit hex\nExample: -bgcol CCCCCC999999FAFAFA \n" );
    printf( "	-bf         : (-best_fit) best fit. Attempt to fit all puzzle pieces into the screen area.\n" );
    printf( "	-mcs <n>    : set mouse click speed 150 to 350 (default %d)\n", mcs );
    printf( "	-se         : (-sort_edges) sort edges. Finds all edge pieces and places them in a seperate pile.\n" );
    printf( "	-nr         : (-no_rotation) no rotation (disable all rotation of pieces)\n" );
    printf( "	-any_angle  : any angle (any angle when rotating)\n" );
	printf( "	-no_anim    : don't animate rotation and flipping of tiles\n" );
    printf( "	-setup <n>  :  don't shuffle pieces use <n> spaces between pieces\n" );
    printf( "	-dist <n>   : distortion percentage for pieces. 0 to 35 is good \n" );
    printf( "	-shadow <n> : pixels of shadowed border, 1=default ,2=thick or 3=very thick\n" );
    printf( "	-v          : show xpuz version \n" );
    printf( "	-file <file>: path to image. (in quotes) \n" );
    printf( "	-nqk        : don't show quick key information at top of screen .\n" );
	printf( "	-pn <player name> : (-player_name) current players name. no spaces allowed. (used by scores file).\n" );
	printf( "	-gn <game name> : (-game_name) name for current puzzle. no spaces allowed.  (used by scores file).\n" );
	printf( "	-xf <xpuz folder name> : name of the xpuz folder (in 'users home') for scores file. no spaces allowed.  (default is '.xpuz') .\n" );
	


#ifdef USE_MIT_SHM
    printf( "	-shm        : use MIT-SHM %s\n", (shared)?"(default)":"" );
    printf( "	-no_shm     : don't use MIT-SHM extension %s\n", (shared)?"":"(default)" );
#endif
 

    if (verbose) {
    printf( "additional options for debugging:\n" );
    printf( "         -a  <n>    : startup angle\n" );
    printf( "         -s         : shuffle tiles\n" );
    printf( "         -sf        : full shuffle\n" );
    printf( "         -sa        : shuffle angles\n" );
    printf( "         -sp        : shuffle positions\n" );
    printf( "         -r         : rotation demo\n" );
    printf( "         -8 -16 -32 : manually select optimized texture mapping routine\n" );
    
    printf( "         -maxang <n>: maximum rotation angle at startup\n" );
    printf( "         -rand <n>  : seed for random generator\n" );
    
    printf( "         -ftb <t>   : set flip time base                        (%g)\n", fliptimebase );
    printf( "         -ftd <t>   : set flip time delta (for additional tiles) (%g)\n", fliptimedelta );
    printf( "         -mft <t>   : maximum number of flip tiles              (%d)\n", maxfliptiles );
    printf( "         -fs <t>    : set value, when to stop the flip          (%g)\n", flipsave );
    printf( "         -ttb <t>   : set turn time base                        (%g)\n", turntimebase );
    printf( "         -ttd <t>   : set turn time delta (for aditional tiles) (%g)\n", turntimedelta );
    printf( "         -mtt <t>   : maximun number of flip tiles              (%d)\n", maxturntiles );
    printf( "         -msr <t>   : maximun number snap retries               (%d)\n", maxsnapretries );
    printf( "         -mac <t>   : minimum number of tiles for adjustment    (%d)\n", minadjustcount );
    }
    printf( "\ncontrols:\n" );
    printf( "-------------\n" );
	printf( "mouse-left drag     : straight drag\n" );
    printf( "mouse-right drag    : drag with automatic rotation\n" );
    printf( "click mouse-left    : rotate 90 degrees anti-clockwise\n" );
    printf( "click mouse-right   : rotate 90 degrees clockwise\n" );
    printf( "CTRL + click left   : flip piece over (double sided puzzles)\n" );
    printf( "F                   : Fix or UnFix drag\n" );
    printf( "M                   : Mouse button swap\n" );
    printf( "S                   : Save current puzzle (for future use)\n" );
	printf( "Q (x2)              : Press Q twice to Quit\n" );
    
  
    exit(0);
}

//=================== options list =================================
void options() {
    printf( "-op: (or -options) show these options \n" );
	printf( "-display: [-display <xdisplay>] x11 display to use. (hostname:number.screen_number) \n" );
	printf( "-v: show xpuz version \n" );
    printf( "-file: [-file <file>] path to image. (in quotes) \n" );
    printf( "-nqk: no quick key information at top of screen .\n" );
	printf( "-pn: [-pn <player name>] current players name. in quotes if it contains spaces. (used by scores file).\n" );
	printf( "-gn: [-gn <game name>] name for current puzzle. in quotes if it contains spaces.  (used by scores file).\n" );
	printf( "-xf: [-xf <xpuz folder name>] name of the xpuz folder (in 'users home') for scores file. in quotes.  (default is '.xpuz') .\n" );
	
	printf( "-skill: [-skill <n>] level of skill 0 to 9. for quick launch. example:- xpuz -skill 3 -file \"myimage.jpg\"\n" );
printf( "-play#  : [-play# <filepath>] . create a puzzle from the image in <filepath>.( # = skill level 0 to 9). example: xpuz play3 \"/path/to/myimage.jpg\". NOTE: <filepath> should always be in quotes.\n\n" );
    printf( "-pw: [-pw <n>] number of pieces in x direction\n" );
    printf( "-ph: [-ph <n>] number of pieces in y direction\n" );
    printf( "-ps: [-ps <n>] set average piece size (default %d)\n", tile_size );
    printf( "-ww: [-ww <n>] width of image\n" );
    printf( "-wh: [-wh <n>] height of image\n" );
    printf( "-side: [-side <n>] make single sided puzzle <n> (0 or 1). (0 is top side) \n" );
    printf( "-single: make single sided puzzle  \n" );
    printf( "-double: make double sided puzzle  \n" );
	printf( "-sb: use solid color for back of pieces\n" );
	printf( "-sb_col: [-sb_col <n>] color number for back\n" );
    printf( "-ap: automatically rotate landscape images to portrait\n" );
    printf( "-ac: automatically crop extra space from around images\n" );
    printf( "-bs: swap the left and right mouse buttons\n" );
    printf( "-fa: fix angle of pieces to 90 deg (always) \n" );
	printf( "-aa: any angle (any angle when dragging and rotating)\n" );
    printf( "-p_style: [-p_style <n>] set style of pieces 0 to %d (default %d)\n",stylecount, piecestyle );
    printf( "-bgcol: [-bgcol <18digit hex>] set the puzzleboard background colour. Example: -bgcol CCCCCC999999FAFAFA \n" );
    printf( "-bf: best fit. Attempt to fit all puzzle pieces into the screen area.\n" );
    printf( "-mcs: [-mcs <n>] set mouse click speed 150 to 350 (default %d)\n", mcs );
    printf( "-se: sort edges. Finds all edge pieces and places them in a seperate pile.\n" );
    printf( "-nr:no rotation of pieces (disable all rotation)\n" );
    printf( "-no_anim: don't animate rotation and flipping of piecess\n" );
    printf( "-setup: [-setup <n>] don't shuffle pieces use <n> spaces between pieces\n" );
    printf( "-dist: [-dist <n>] distortion percentage 0 to 35 is good \n" );
    printf( "-shadow: [-shadow <n>] pixels of shadowed border, 1=default ,2=thick or 3=very thick\n" );

#ifdef USE_MIT_SHM
    printf( "-shm: use MIT-SHM %s\n", (shared)?"(default)":"" );
    printf( "-no_shm: don't use MIT-SHM extension %s\n", (shared)?"":"(default)" );
#endif
  
	printf( "-a: [-a <n>] startup angle\n" );
    printf( "-s: shuffle pieces.\n" );
    printf( "-sf: full shuffle\n" );
    printf( "-sa: shuffle angles\n" );
    printf( "-sp: shuffle positions\n" );
    printf( "-rp: rotation pieces after creating puzzle\n" );
    printf( "-maxang: [-maxang <n>] maximum rotation angle at startup\n" );
    printf( "-rand: [-rand <n>] seed for random generator\n" );
    
    printf( "-ftb: [-ftb <t>] set flip time base                        (%g)\n", fliptimebase );
    printf( "-ftd: [-ftd <t>] set flip time delta (for additional tiles) (%g)\n", fliptimedelta );
    printf( "-mft: [-mft <t>] maximum number of flip tiles              (%d)\n", maxfliptiles );
    printf( "-fs:  [-fs <t>] set value, when to stop the flip          (%g)\n", flipsave );
    printf( "-ttb: [-ttb <t>] set turn time base                        (%g)\n", turntimebase );
    printf( "-ttd: [-ttd <t>] set turn time delta (for aditional tiles) (%g)\n", turntimedelta );
    printf( "-mtt: [-mtt <t>] maximun number of flip tiles              (%d)\n", maxturntiles );
    printf( "-msr: [-msr <t>] maximun number snap retries               (%d)\n", maxsnapretries );
    printf( "-mac: [-mac <t>] minimum number of tiles for adjustment    (%d)\n", minadjustcount );
    
    exit(0);
}

void scan_args( int argc, char **argv ) {
    for (int i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-display"))            dispname=argv[++i];
        else if (!strcmp(argv[i],"-lg")||!strcmp(argv[i],"-loadgame"))  loadgame=1;
		else if (!strcmp(argv[i],"-op") || !strcmp(argv[i],"-options"))            options();
		else if (!strcmp(argv[i],"-width")&&i<argc-1 || !strcmp(argv[i],"-ww")&&i<argc-1)  width=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-wh")||!strcmp(argv[i],"-height")||!strcmp(argv[i],"-hh"))    height=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-w")&&i<argc-1 || !strcmp(argv[i],"-pw")&&i<argc-1)   dx=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-h")&&i<argc-1|| !strcmp(argv[i],"-ph")&&i<argc-1)   dy=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-ts")||!strcmp(argv[i],"-ps")||!strcmp(argv[i],"piece_size"))  tile_size=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-r") || !strcmp(argv[i],"-rp"))  rotate=1;
        else if (!strcmp(argv[i],"-a")&&i<argc-1) {
                angle=atoi(argv[++i]);
                shuffle&=~1;
        }
         
        else if (!strcmp(argv[i],"-s"))         shuffle=0;

        else if (!strcmp(argv[i],"-sf"))            shuffle=4;
        else if (!strcmp(argv[i],"-sa"))            shuffle=1;
        else if (!strcmp(argv[i],"-sp"))            shuffle=2;
        else if (!strcmp(argv[i],"-no_flip"))  no_flip=1;
		else if (!strcmp(argv[i],"-ap") || !strcmp(argv[i],"-auto_portrait"))  no_flip=0;
		else if (!strcmp(argv[i],"-nqk"))  no_qk=1;
		else if (!strcmp(argv[i],"-fa") || !strcmp(argv[i],"-fix_angle"))  fix_angle=1;
		else if (!strcmp(argv[i],"-bs") || !strcmp(argv[i],"-btn_swap"))  btn_swap=0;
		else if (!strcmp(argv[i],"-se") || !strcmp(argv[i],"-sort_edges"))  sortedges=1;
		else if (!strcmp(argv[i],"-bf") || !strcmp(argv[i],"-best_fit"))  bestfit=1;
        else if (!strcmp(argv[i],"-nr") || !strcmp(argv[i],"-no_rotate")) {
                                           no_rotation=1;
                                           angle=0;
                                           shuffle&=~1;
                                         } 

        else if (!strcmp(argv[i],"-p_style"))   piecestyle=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-mcs"))   mcs=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-no_crop"))  autocrop=0;
		else if (!strcmp(argv[i],"-ac") || !strcmp(argv[i],"-auto_crop"))  autocrop=1;
		
		else if (!strcmp(argv[i],"-sb"))  tile_solid_back=1;
		else if (!strcmp(argv[i],"-sb_col")&&i<argc-1)	tile_back_color=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-aa") || !strcmp(argv[i],"-any_angle"))  any_angle=1;
        else if (!strcmp(argv[i],"-side")&&i<argc-1)        side_lock=atoi(argv[++i]);
		else if (!strcmp(argv[i],"-single")&&i<argc-1)        side_lock=0;
		else if (!strcmp(argv[i],"-double")&&i<argc-1)        side_lock=-1;
        else if (!strcmp(argv[i],"-rand")&&i<argc-1)        random_seed=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-dist")&&i<argc-1)        distortion=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-maxang")&&i<argc-1)  maxang=atof(argv[++i]);
        else if (!strcmp(argv[i],"-shadow")&&i<argc-1)  shadow_size=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-setup")&&i<argc-1) {
                straight_setup=atoi(argv[++i]);
                shuffle=0;
        }

		else if (!strcmp(argv[i],"-v")) { std::cout << "xpuz " << xpuzver << "\n"; exit(0);  }
		else if (!strcmp(argv[i],"-pn")&&i<argc-1)	playername=argv[++i];
		else if (!strcmp(argv[i],"-player_name")&&i<argc-1)	playername=argv[++i];

		else if (!strcmp(argv[i],"-gn")&&i<argc-1)	gamename=argv[++i];
		else if (!strcmp(argv[i],"-game_name")&&i<argc-1)	gamename=argv[++i];

		else if (!strcmp(argv[i],"-xf")&&i<argc-1)	xpuzdir=argv[++i];
        else if (!strcmp(argv[i],"-file")&&i<argc-1) filename=argv[++i];

        else if (!strcmp(argv[i],"-8"))             texture_mode=1;
        else if (!strcmp(argv[i],"-16"))            texture_mode=2;
        else if (!strcmp(argv[i],"-32"))            texture_mode=3;
        else if (!strcmp(argv[i],"-shm"))           shared=1;
        else if (!strcmp(argv[i],"-no_shm"))        shared=0;
        
        else if (!strncmp(argv[i],"-verbose",2))    verbose=1;
        else if (!strcmp(argv[i],"-ftb")&&i<argc-1)         fliptimebase=atof(argv[++i]);
        else if (!strcmp(argv[i],"-ftd")&&i<argc-1)         fliptimedelta=atof(argv[++i]);
        else if (!strcmp(argv[i],"-mft")&&i<argc-1)         maxfliptiles=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-fs")&&i<argc-1)          flipsave=atof(argv[++i]);
        else if (!strcmp(argv[i],"-ttb")&&i<argc-1 || !strcmp(argv[i],"-anim_speed")&&i<argc-1)   turntimebase=atof(argv[++i]);
        else if (!strcmp(argv[i],"-ttd")&&i<argc-1)         turntimedelta=atof(argv[++i]);
        else if (!strcmp(argv[i],"-mtt")&&i<argc-1)         maxturntiles=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-msr")&&i<argc-1)         maxsnapretries=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-mac")&&i<argc-1)         minadjustcount=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-bgcol")&&i<argc-1)       bgcol=argv[++i];
        else if (!strcmp(argv[i],"-no_anim")) { maxturntiles = maxfliptiles = 0; }
		
		else if (!strcmp(argv[i],"-play0")&&i<argc-1)
		{
 		  filename=argv[++i]; width= 800; dx=3; side_lock=0; distortion= 5; fix_angle=1; bestfit=1; btn_swap=1;
			return;
		}
		
		else if (!strcmp(argv[i],"-play1")&&i<argc-1)
		{
 		  filename=argv[++i]; width= 800; dx=5; side_lock=0; sortedges=1; bestfit=1; btn_swap=1;
			return;
		}
		else if (!strcmp(argv[i],"-play2")&&i<argc-1)
		{
 		  filename=argv[++i]; width= 800; dx=8; side_lock=0; sortedges=1; bestfit=1; btn_swap=1;
			return;
		}
		else if (!strcmp(argv[i],"-play3")&&i<argc-1)
		{
 		  filename=argv[++i]; width= 800; dx=12; side_lock=0; sortedges=1; bestfit=1; btn_swap=1;
			return;
		}
		else if (!strcmp(argv[i],"-play4")&&i<argc-1)
		{
 			filename=argv[++i]; width= 800; dx=16; side_lock=0; bestfit=1; btn_swap=1;
			return;
		}
		else if (!strcmp(argv[i],"-play5")&&i<argc-1)
		{
 			filename=argv[++i]; dx=18; side_lock=0; bestfit=1; btn_swap=1;
			return;
		}
		else if (!strcmp(argv[i],"-play6")&&i<argc-1)
		{
 			filename=argv[++i]; dx=24; bestfit=1; btn_swap=1;
			return;
		}
		else if (!strcmp(argv[i],"-play7")&&i<argc-1)
		{
 			filename=argv[++i]; dx=28; bestfit=1; btn_swap=1;
			return;
		}
		else if (!strcmp(argv[i],"-play8")&&i<argc-1)
		{
 			filename=argv[++i]; dx=32; bestfit=1; btn_swap=1;
			return;
		}
		else if (!strcmp(argv[i],"-play9")&&i<argc-1)
		{
 			filename=argv[++i]; dx=36; btn_swap=1;
			return;
		}






		else if (!strcmp(argv[i],"-skill")&&i<argc-1) 
		{
            int skill=atoi(argv[++i]);
			btn_swap=1;
			bestfit=1;
			
               	
			

			     if(skill==0){width= 800; dx=3; fix_angle=1; side_lock=0; }
            else if(skill==1){width= 800; dx=5; side_lock=0; sortedges=1;}
			else if(skill==2){width= 800; dx=8; side_lock=0; sortedges=1;}
			else if(skill==3){width= 800; dx=12; side_lock=0; sortedges=1;}
			else if(skill==4){dx=16; side_lock=0; sortedges=1;}
			else if(skill==5){dx=18; side_lock=0; sortedges=1;}
			else if(skill==6){dx=24; }
			else if(skill==7){dx=28; }
			else if(skill==8){dx=32; }
			else if(skill==9){dx=36; }
			
        }

		else if (!strcmp(argv[i],"-h")) {  usage(); return; }

        
		else if (i==1)
		{
			filename=argv[i]; width= 800; dx=8; side_lock=0; sortedges=1; bestfit=1; btn_swap=1;
			return;
		}
    }



//std::cout << xpuzdir << std::endl;//========== debugonly =========
}




void (*old_sighandler)(int);
void sig_quit( int ) {
    // handler that raises the quit flag on interrupts
    quit=1;
}

static XErrorHandler old_handler;
int error_handler( Display * dpy_in, XErrorEvent *xerror ) {
    if (xerror->error_code==BadWindow) {
        // just ignore BadWindow failures, since they easily might occur
        // due to the destruction of windows.
        return 0;
    }
    else return old_handler( dpy_in, xerror );
}


// =================================================================
// load last game saved
// =================================================================
static void loadlastgame() {
char     buffer[100];
// this is not yet available
// put the code here when you figure it out

std::cout << "This is not available yet. (debug only)\n";

// exit(0);
}

// =================================================================
// save the current game
// =================================================================
static void savegame() {
char     buffer[40];
// this is not yet available
// put the code here when you figure it out

sprintf( buffer, "Game Saved (debug only)" );
XStoreName(dpy,win,buffer);

}

// =================================================================
// save scores 
// =================================================================
static void savefinish() {
long ns=350;
using namespace std;
int   wmf=0;
string pln (getenv("USER"));
if(playername=="") {playername= pln.c_str();}


string gametime;
//string plevel="1-Junior";
char playtime[10]; 
string filepath(filename);
string homefolder (getenv ("HOME"));
long   gamevalue;
char strgamevalue[10]; 

string gamefilename(homefolder + "/" + xpuzdir +"/puzzle.name");
//cout << xpuzdir << std::endl;//========== debugonly =========
const char *gfn;
gfn=gamefilename.c_str();

//string scorefilename("/var/games/xjig.scores");
string scorefilename(homefolder + "/" + xpuzdir +"/scores");

sprintf( playtime, "%02ld:%02ld", gameseconds / 60, gameseconds % 60 );
string game_name;

 if(gamename == "")
{ 
       ifstream gfile;

       gfile.open (gfn);
 
       if (gfile.is_open()) 
		{ 
          while (! gfile.eof() )
           {
        		getline(gfile,game_name); 
           }
      
          gfile.close();
		  gamename=game_name.c_str(); 
		}
}
// write to the file
 
const char *sfn;
sfn=scorefilename.c_str(); 	
//FILE *scorefile;
//scorefile = fopen(sfn,"a+"); 
//fprintf(scorefile,"%s",scoretext); 
//fclose(scorefile); 
 

 ofstream scorefile;
  scorefile.open (sfn,ios::app);

   if (scorefile.is_open()) { 
     scorefile  << gamename << "|" << gamepieces << "|" << playername <<  "|" << gameseconds << "\n";
     scorefile.close();
   } 
 
}


//========================================
//============ main ======================
//========================================
int main(int argc, char **argv)
{
XEvent  event;
unsigned long next_sec=0;
const char *options;
	

    if (argc<2) {
        usage();
    }

    scan_args( argc, argv );

    quit=0;
    old_sighandler=signal( SIGINT, sig_quit );

	if(loadgame==1)
	{ 
	    loadlastgame();  
		exit(0); 
	}


   //=========================================================
   // Set background colours

   char wmbgc[8];
   char wmtpc[8];
   char wmbtc[8];

    if (strlen(bgcol) >=6){

      
    strcpy(wmbgc, "#");
    strncat(wmbgc, bgcol, 6);
    strcpy(bgc, wmbgc);
  
// printf("bgc: %s\n", bgc);  
        if (strlen(bgcol) >= 18) {
          strcpy(wmtpc, "#");
          strncat(wmtpc, bgcol+6, 6);
          strcpy(tpc, wmtpc);

          strcpy(wmbtc, "#");
          strncat(wmbtc, bgcol+12, 6);
          strcpy(btc, wmbtc);
         }else{
        
          strcpy(tpc,wmbgc);
          strcpy(btc,wmbgc);
         }
    }else{
        
        strcpy(bgc,"#7F7F7F");
        strcpy(tpc,"#999999");
        strcpy(btc,"#666666");
    }

    //printf( "bgcol: %d, bgc: %s.\n\n", bgcol, bgc );


   //
    // open the display and a port object as a color manager
    //
    
	
	dpy = XOpenDisplay(dispname);
    if (!dpy) {
        fprintf(stderr,"can't open display '%s'\n", dispname);
        exit(-1);
    }
    scr = DefaultScreen(dpy);
    port = new Port(dpy);

    std::cout << "\nxpuz " << xpuzver << "\n";
 
    if (!texture_mode) {
        //
        // check screen depth to select function for texture mappings
        //
        switch(DefaultDepth(dpy,scr)) {
        case 8:     texture_mode=1; break;
        case 16:        texture_mode=2; break;
        case 24:
        case 32:        texture_mode=3; break;
        }
    }
    if (!texture_mode) {
        fprintf( stderr, "*** Unable to select texture mode for Depth %d\n", DefaultDepth(dpy,scr) );
        fprintf( stderr, "    You can manually select one by trying either -8, -16 or -32\n" );
        fprintf( stderr, "    Good Luck.\n" );
        exit(0);
    }

    if (verbose) {
        switch( texture_mode ) {
        case 1: printf( "texture mode 1: 1 byte\n" ); break;
        case 2: printf( "texture mode 2: 2 byte\n" ); break;
        case 3: printf( "texture mode 3: 4 byte\n" ); break;
        }
    }

    old_handler=XSetErrorHandler( error_handler );


//============ setup the piece type ==============

//piecestyle = 3; // for debug only



 if (piecestyle > stylecount || piecestyle <0){ 
  piecestyle=0; 
  }

 
switch (piecestyle ) {

  case 3 : 
    // Scrap
    distortion += 20;
    break;

  case 4 : 
    // Square (no distortion)
    distortion = 0;
    break;

  case 8 : 
    // Strips (no distortion)
    distortion = 0;
    dx=width/(tile_size*2);
    dy= dx*5;
   
    break;
  
}

int i;

 for(i=0; i<10; i++) {
     spx[i]=spx_style[piecestyle] [i];
     spy[i]=spy_style[piecestyle] [i];
 }

//==========================================================

    //
    // load image and scale it according to the input options
    // or set original image size, when no size selected.
    //
    pm=new XjigPixmap(port,filename);
    options=pm->GetExtensionData( OPTION_EXTENSION );
    if (options) {
        char    opt_buffer[512];
        int argc_opt=0;
        char    *argv_opt[30];
        char    *cptr=opt_buffer;
        strcpy(opt_buffer,options);

        argv_opt[argc_opt++]=argv[0];
        argv_opt[argc_opt++]=cptr;
        while( *cptr ) {
            if ( *cptr==' ' ) {
                *cptr++=0;
                argv_opt[argc_opt++]=cptr;
            }
            else cptr++;
        }
        scan_args( argc_opt, argv_opt );
    }
    const char *comment=pm->GetExtensionData( COMMENT_EXTENSION );
    if (comment)    { printf( "%s\n", comment ); }

    if (autocrop)       pm->CropImage();
    if (pm->Width()>pm->Height()&&!no_flip)     pm->Rotate90();
    if (verbose)
        printf("original image size: %d %d\n", pm->Width(), pm->Height() );

    if (!width&&!height) {
        width = pm->Width();
        height= pm->Height();
    }
    else {
        // scale to desired size, (keep aspect when only one param selected)
        if (!width)     width=height*pm->Width()/pm->Height();
        if (!height)    height=width*pm->Height()/pm->Width();
    }
    pm->CreateData( width, height );

    if (!dx||!dy) {
        if (dy)         dx=width*dy/height;
        else if (dx)    dy=height*dx/width;
        else if (tile_size) {
                            dx=width/tile_size;
                            dy=height/tile_size;
        }
    }
    if (dx<=0||dy<=0)    { dx=4; dy=6; }

    if (verbose)
        printf( "number of pieces: %d\n", dx * dy );

    offy = height/dy/2;
    if ( (height+(dy+1)*offy) > DisplayHeight(dpy,scr)-20 ) {
        offy=(DisplayHeight(dpy,scr)-height-20)/(dy+1);
    }
    offx = width /dx/2;
    win_size_x=2*(width+dx*offx)+offx;
    if ( win_size_x > DisplayWidth(dpy,scr)-8 ) {
        win_size_x=DisplayWidth(dpy,scr)-8;
        if ( 2*width+dx*offx+offx > win_size_x )    offx=(win_size_x-2*width)/(dx+1);
    }
    tile_size=width/dx;

   
    win_size_y=height+(dy+1)*offy;

    win=XCreateSimpleWindow(dpy,RootWindow(dpy,scr),
                 0,0,win_size_x,win_size_y,2,WhitePixel(dpy,scr),
                 port->AllocNamedColor( "#7f7f7f" ) );
    XStoreName(dpy,win,argv[0]);
    
    gc =XCreateGC(dpy,win,0,0);
    XSetForeground(dpy,gc,WhitePixel(dpy,scr));



    // prepare some cursors
XColor  white_col, black_col;
Pixmap  pixmap;

    XParseColor(dpy,DefaultColormap(dpy,scr), "white", &white_col );
    XParseColor(dpy,DefaultColormap(dpy,scr), "black", &black_col );

    normal_cursor = XCreateFontCursor( dpy, XC_top_left_arrow );
    move_cursor   = XCreateFontCursor( dpy, XC_fleur );
    pull_cursor   = XCreateFontCursor( dpy, XC_hand2 );
    idle_cursor   = XCreateFontCursor( dpy, XC_watch );

    pixmap = XCreateBitmapFromData(dpy,RootWindow(dpy,scr),
                    cursor_bits, cursor_width, cursor_height );
    no_cursor = XCreatePixmapCursor( dpy, pixmap, pixmap,
                    &white_col, &black_col, cursor_x_hot, cursor_y_hot );
    XFreePixmap( dpy, pixmap );



    InitTime();

    // create Object-Stack with Background
    
        stk = new DBObjectStack();
        stk->Append( new BackDrop() );
    

    // create buffer for faster image rotation
    img_buf = new ImageBuffer();

    // initialize puzzle game
    if(random_seed>0)
        my_srand( random_seed );
    else
        my_srand(time(NULL)+getpid());

   

 
    p = new Puzzle();
   p->Init(width,height,dx,dy, pm->GetExtensionData( FLATTILE_EXTENSION ) );
       gamepieces=(dx*dy);
    // check for hidden pieces
    options=pm->GetExtensionData( REMOVETILE_EXTENSION );
    if (options) {
        const char  *cptr=options;
        while(*cptr) {
            int x,y;
            sscanf(cptr,"%02x%02x",&x,&y);
            cptr+=4;
            p->DropTile(x,y);
        }
    }

    
        XSelectInput(dpy,win, ExposureMask|StructureNotifyMask|KeyPressMask|
                ButtonPressMask|ButtonReleaseMask|PointerMotionMask);
        XMapRaised(dpy,win);
  
if(bestfit==1)
      {
		int x1,y1,x2,y2;
        stk->GetExtent(&x1,&y1,&x2,&y2);
        stk->PanView( x1-win_size_x/2+(x2-x1)/2, y1-win_size_y/2+(y2-y1)/2 );
        int zf1=(int)(win_size_x*zoom_factor/(x2-x1));
        int zf2=(int)(win_size_y*zoom_factor/(y2-y1));
        if (zf2<zf1)    zf1=zf2;
        if (zf1>3) {
        stk->ZoomView(win_size_x/2,win_size_y/2,zf1-zoom_factor);
        stk->ExposeRegion(0,0,win_size_x,win_size_y);
      }


}

  

    while(quit!=2) {

        if (!XPending(dpy)) {
            XSync(dpy,0);
            while (!XPending(dpy)) {
                struct timeval timeout;
                /*struct*/ fd_set readfds;
                int     nfds;

                FD_ZERO( &readfds );
                FD_SET(  ConnectionNumber(dpy), &readfds  );
                nfds = ConnectionNumber(dpy)+1;

                timeout.tv_sec  = 0;
                double current_time=GetCurrentTime();
                timeout.tv_usec = (long)(1000000 * (1.0-(current_time-floor(current_time))));
                select( nfds, FDS_TYPE &readfds, 0, 0, &timeout );

                current_time = GetCurrentTime();
                //  printf( "%g\n", current_time );

               if ((unsigned long)current_time>next_sec) {
                    char     buffer[20];
                 
                  if (gamepaused) {pausetime= (unsigned long)current_time-next_sec;}

                  next_sec = (unsigned long)current_time-pausetime;

                    if (!p->Finished()) {
                       if (gamepaused) {
                          sprintf( buffer, "Game paused" );
                          XStoreName(dpy,win,buffer);
                       }
                       else{
							int pl=p->PiecesLeft();
							int pt=dx*dy;
							if(pl<pt) pl=pl-1;
			if(no_qk==0)
			{
                         sprintf( buffer, "Pieces:%d Time:%02ld:%02ld  Keys: [P=Pause] [PgUp=ZoomIn] [PgDown=ZoomOut] [Home=Original] [End=BestFit] [Cursorkeys=Pan] ",pl ,next_sec / 60, next_sec % 60 );
			} 
			else 
			{
				sprintf( buffer, "Pieces:%d  Time:%02ld:%02ld ",pl , next_sec / 60, next_sec % 60 );

			}
                          XStoreName(dpy,win,buffer);
                       }
                    }
                    else {
                        sprintf( buffer, "Puzzle complete   %02ld:%02ld",next_sec / 60, next_sec % 60 );
                            gameseconds=next_sec;
                            savefinish();
                         XStoreName(dpy,win,buffer);
                        XBell(dpy,100);
                        XFlush(dpy);
                        next_sec=1000000;
                    }
                }
            }
        }


        XNextEvent(dpy,&event);
        switch(event.type) {
        case KeyPress: {
            char                buffer=0;
            XComposeStatus  compose;
            KeySym          keysym;
            int             mult=(event.xkey.state&ShiftMask)?2:1;

            XDefineCursor(dpy,win,idle_cursor);
            XFlush(dpy);
            XLookupString( (XKeyEvent*)&event, &buffer, 1, &keysym, &compose );
            switch( keysym ) {
            case XK_plus:
            case XK_KP_Add:
            case XK_Page_Up:
            case XK_KP_Page_Up:
                    stk->ZoomView(win_size_x/2,win_size_y/2,2*mult);
                    stk->ExposeRegion(0,0,win_size_x,win_size_y);
                    break;
            case XK_minus:
            case XK_KP_Subtract:
            case XK_Page_Down:
            case XK_KP_Page_Down:
                    if (zoom_factor>5)  stk->ZoomView(win_size_x/2,win_size_y/2,-2*mult);
                    stk->ExposeRegion(0,0,win_size_x,win_size_y);
                    break;
            case XK_Right:
            case XK_KP_Right:
                    stk->PanView( 10*mult, 0 );
                    stk->ExposeRegion(0,0,win_size_x,win_size_y);
                    break;
            case XK_Down:
            case XK_KP_Down:
                    stk->PanView( 0, 10*mult );
                    stk->ExposeRegion(0,0,win_size_x,win_size_y);
                    break;
            case XK_Left:
            case XK_KP_Left:
                    stk->PanView( -10*mult, 0 );
                    stk->ExposeRegion(0,0,win_size_x,win_size_y);
                    break;
            case XK_Up:
            case XK_KP_Up:
                    stk->PanView( 0, -10*mult );
                    stk->ExposeRegion(0,0,win_size_x,win_size_y);
                    break;
            case XK_End:
            case XK_KP_End: {
                    int x1,y1,x2,y2;
                    stk->GetExtent(&x1,&y1,&x2,&y2);
                    stk->PanView( x1-win_size_x/2+(x2-x1)/2, y1-win_size_y/2+(y2-y1)/2 );
                    int zf1=(int)(win_size_x*zoom_factor/(x2-x1));
                    int zf2=(int)(win_size_y*zoom_factor/(y2-y1));
                    if (zf2<zf1)    zf1=zf2;
                    if (zf1>3) {
                        stk->ZoomView(win_size_x/2,win_size_y/2,zf1-zoom_factor);
                        stk->ExposeRegion(0,0,win_size_x,win_size_y);
                    }
                    break;
            }
            case XK_Home:
            case XK_KP_Home: {
                    int x1,y1,x2,y2;
                    stk->GetExtent(&x1,&y1,&x2,&y2);
                    stk->PanView( x1-10, y1-win_size_y/2+(y2-y1)/2 );
                    stk->ZoomView(10,win_size_y/2,20-zoom_factor);
                    stk->ExposeRegion(0,0,win_size_x,win_size_y);
                    break;
            }
          //  case XK_Escape:
            case XK_Q:
            case XK_q:
                    quit++;
                    break;

            case XK_f:
            case XK_F:
                   if(fix_angle==0) {
                      fix_angle=1;
                     }else{
                      fix_angle=0; }
                    break;


            case XK_m:
            case XK_M:
                   if(btn_swap==0) {
                      btn_swap=1;
                     }else{
                      btn_swap=0; }
                    break;

            case XK_p:
            case XK_P:
                   if(gamepaused==0) {
                      gamepaused=1;
                      
                     }else{
                      gamepaused=0;
                      
                     }
                    break;

            case XK_s:
            case XK_S:
                   savegame();
                  
                   break;

	    case XK_k:
            case XK_K:
                   if(no_qk==0) { no_qk=1; }else{ no_qk=0; }
                    break;

            

            default:
                    break;
            }
            XDefineCursor(dpy,win,normal_cursor);
            XSync(dpy,0);
            warp_lock_x=WARP_NO_LOCK;   // just as a way tp unlock
            warp_lock_y=WARP_NO_LOCK;
            break;
        }

        case ButtonPress:
            gamepaused=0;
			quit=0;
            GetCurrentTime(1);  // Reset idle start
            stk->DispatchPress( &event.xbutton );
            break;
        case ButtonRelease:
            stk->DispatchRelease( &event.xbutton );
            break;
        case MotionNotify:
            if (warp_lock_x!=WARP_NO_LOCK) {
                do {
                    if (warp_lock_x==event.xmotion.x&&warp_lock_y==event.xmotion.y) 
			{
                        warp_lock_x=WARP_NO_LOCK;
                        warp_lock_y=WARP_NO_LOCK;
                        break;
                    }
                    printf( "#### motion event skipped due to warp lock.\n" );
                }
                while( XCheckMaskEvent(dpy,PointerMotionMask,&event) );
            }
            else {
                while( XCheckMaskEvent(dpy,PointerMotionMask,&event) );
                if (event.xmotion.state)    stk->DispatchMotion( &event.xmotion );
            }
            break;

        case EnterNotify:
            XSetInputFocus( dpy, event.xcrossing.window, (int)None, CurrentTime );
            break;

        case Expose:
            
                stk->ExposeRegion(event.xexpose.x, event.xexpose.y,
                        event.xexpose.width, event.xexpose.height );
            
                if (rotate) {
                    XSync(dpy,0);
                    double  t1=GetCurrentTime();
                    p->Rotation();
                    double  t2=GetCurrentTime();
                    printf( "Rotation Time: %.4g secs.\n", t2 - t1 );
                    rotate=0;
                }
            break;

        case ConfigureNotify:
            if (win_size_x!=event.xconfigure.width||win_size_y!=event.xconfigure.height) {
                win_size_x=event.xconfigure.width;
                win_size_y=event.xconfigure.height;
            }
            break;
        }
    }

    delete img_buf;

    printf( "terminated\n" );
    return 0;
}
