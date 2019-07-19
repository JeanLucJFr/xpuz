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
   
#ifndef _global_h
#   include "global.h"
#endif

#include <X11/Xmd.h>


#ifndef _objects_h
#   include "objects.h"
#endif
#ifndef _ximage_h
#   include "ximage.h"
#endif
#ifndef _imgbuff_h
#   include "imgbuff.h"
#endif
#ifndef _vec2_h
#   include "vec2.h"
#endif
#ifndef _vec2list_h
#   include "vec2list.h"
#endif
#ifndef _mat2_h
#   include "mat2.h"
#endif
#ifndef _color_mapper_h
#   include "color_mapper.h"
#endif
#ifndef _puzzle_h
#   include "puzzle.h"
#endif


/******************************************************************************

PieceFrame    - corner, edge and pin-data of the piece including polygon-spline
      Vec2List *vl
RotatedFrame  - position-data including
                window-position, upward-angle and rotated polygin spline
      Vec2     winpos
      Real     windir
      Vec2List *tvl
BitmapPiece   - pixmap with mask of the rotated Piece
        Pixmap   tilemask
PixmapPiece   - pixmap with Puzzle-Image of the rotated Piece
        Pixmap   tilemap
ShadowPiece   - pixmap with Puzzle-Image and ShadowFrame of the rotated Piece
        Pixmap   shadowmask
        Pixmap   shadowmap
PieceObject   - object subclass to be stack-controlled
DBPieceObject - double buffered moves and turns

 ******************************************************************************/

//  . . . . + . . . . + . . . . + . . . .
//  . . . . . . . . . . . . . . . . . . .
//  . * . . . . . . . . . . . . . . . . .
//  . . . . . . . . . . . . . . . . . . .
//  . . . . . . . . . . . . . . . . . . .
//  . . . . + . . . . + . . . . + . . . .
//  . . . . . . . . . . . . . . . . . . .
//  . * . . . . . . . . . . . . . . . . .
//  . . . . . . . . . * . . * . . . . . .
//  . . * . . . . . . . . . . . . . . . .
//  . . . . * . * . . + . . . . + * . . .
//  . . . . . . . . . . . . . . . . . . .
//  . . . . . . . . . . . . . . . . . . .
//  . . . . . . . . . . . . . . . . . . .
//  . . . . . . . . . . . . . . . . . * .
//  . . . . + . . . . + . . . . + . . . .
//  . . . . . . . . . . . . . . . . . * .



PieceFrameSetup::PieceFrameSetup() {

}

PieceFrameSetup::~PieceFrameSetup() {


}


void PieceFrameSetup::Init( const Vec2 &tl, const Vec2 &tr, const Vec2 &br, const Vec2 &bl ) {

 

    center = (tl+tr+br+bl)/4;
    v[0] = tl-center;
    v[1] = tr-center;
    v[2] = br-center;
    v[3] = bl-center;
    pin[0] = pin[1] = pin[2] = pin[3] = 0;


#if (0)
    printf( "%g %g             %g %g\n", (double)tl.X(), (double)tl.Y(), (double)tr.X(), (double)tr.Y() );
    printf( "center: %g %g\n", (double)center.X(), (double)center.Y() );
    printf( "%g %g             %g %g\n", (double)bl.X(), (double)bl.Y(), (double)br.X(), (double)br.Y() );
#endif
}

void PieceFrame::PositionChanged()  {}
void PieceFrame::DirectionChanged() {}

PieceFrame::PieceFrame() {
    vl = 0;
    join_count=1;
}

PieceFrame::~PieceFrame() {
    if (vl)     delete vl;
}

const Vec2List &PieceFrame::Init( const PieceFrameSetup &pfs ) {




    center=pfs.center;
    vl = new Vec2List(MaxSplineLen);

    for (int i=0;i<4;i++) {
        Vec2    s(pfs.v[i]);

        if (pfs.pin[i]) {
            int j;
            Vec2    e(pfs.v[(i+1)%4]);      // end of segment
            Vec2    d=e-s;              // edge vector

            Vec2    m1=s+pfs.pin[i]*d;      // medium point for pin
            Vec2    bw=(s-m1)/24.0; // backstep vector to startpoint
            Vec2    fw=(e-m1)/24.0; // forward vector to endpoint

            Vec2    n=d/44;
            Vec2    o=(pfs.left[i]?n.TurnLeft():n.TurnRight());

            *vl |= s;
            for (j=1;j<=HalfSplineLen;j++) {
                *vl |= m1 + spx[HalfSplineLen-j]*bw + spy[HalfSplineLen-j]*o;
            }
            for (j=0;j<HalfSplineLen;j++) {
                *vl |= m1 + spx[j]*fw + spy[j]*o;
            }
        }
        else {
            *vl |= s;
        }
    }
    // *vl |= pfs.v[0];
    return *vl;
}

// ===========================================================================

TwinPieceFrame::TwinPieceFrame() {
    page=0;
}

TwinPieceFrame::~TwinPieceFrame() {
}

void TwinPieceFrame::FlipPage() {
    page=1-page;
    DirectionChanged();
}

// ===========================================================================

RotatedFrame::RotatedFrame() {
    tvl = 0;
}
RotatedFrame::~RotatedFrame() {
    if (tvl)        delete tvl;
}

void RotatedFrame::Init( const PieceFrameSetup &pfs ) {
    PieceFrame::Init( pfs );
    windir = 0;
    winpos = Vec2Zero;
}

int RotatedFrame::CheckForJoin( class RotatedFrame *obj ) {
Real    windir_delta = windir - obj->windir;

    // check for correct page
    if (page!=obj->page)                                        return 0;

    // check for an angle between -10 and +10 degrees
    if (fmod(windir_delta+380.0,360.0)>40)              return 0;


Vec2    v1(center - obj->center);
Vec2    v2(winpos - obj->winpos);
Real    help=fabs(windir-obj->windir);
Real    mid_angle;
        if (help>180.0) mid_angle=fmod((windir+obj->windir+360.0)/2,360.0);
        else                    mid_angle=fmod((windir+obj->windir)/2,360.0);
Vec2    erg;
        v2=v2.TurnAngleDeg( -mid_angle );

        if (page) { erg=Vec2(v1.X()+v2.X(),v1.Y()-v2.Y());
        }
        else            erg=Vec2(v1.X()-v2.X(),v1.Y()-v2.Y());


#ifdef DEBUG
    v2=v2.TurnAngleDeg( mid_angle );

    printf( "ImageA: %6.2f %6.2f  WindowA: %6.2f %6.2f  at angle: %6.2f\n",
        (double)center.X(), (double)center.Y(), (double)winpos.X(), (double)winpos.Y(), (double)windir );
    printf( "ImageB: %6.2f %6.2f  WindowB: %6.2f %6.2f  at angle: %6.2f\n",
        (double)obj->center.X(), (double)obj->center.Y(), (double)obj->winpos.X(), (double)obj->winpos.Y(), (double)obj->windir );
    printf( "        %6.2f %6.2f          %6.2f %6.2f\n",
        (double)v1.X(), (double)v1.Y(), (double)v2.X(), (double)v2.Y() );

    v2=v2.TurnAngleDeg( -mid_angle );

    printf( "        angle: %6.2f          %6.2f %6.2f\n",
            mid_angle, (double)v2.X(), (double)v2.Y() );
    printf( "                               %6.2f %6.2f\n",
            (double)erg.X(), (double)erg.Y() );
#endif


    if (erg.Norm()<tile_size/3) {
            DBG0( "***** HIT *****\n" );
            return 1;
    }
    return 0;
}

void RotatedFrame::PositionChanged() {
}
void RotatedFrame::DirectionChanged() {
    if (tvl)        delete tvl;
    if (!page) {
        tvl = new Vec2List(GetPolyLine());
    }
    else {
        tvl = new Vec2List(vl->Len());
        for (int i=0;i<vl->Len();i++) {
            tvl->AddAt(i,Vec2( -(*vl)[i].X(), (*vl)[i].Y() ));
        }
    }
    tvl->TurnAngleDeg(windir);
}

// DoJoin:
// A Joined segment in both polylines has to be found. When found
// the points out of that segment are added from the second to the first
// polyline, which will result into the merge of the two outlines.
//
// vl:       +----+----+----+----+----+----+----+----
//           1    2    3    4    s j  e    7    8
// obj->vl:       +----+----+----+----
//                1'   e  i s    4'
// result:   +----+----+----+----+----+----+----+----+----+----
//           1    2    3    4    s    4'   1'   e    7    8
//
// special cases:
// obj->vl:       +----+----+----+----
//                1'   2' i s    4'
//                          e

int RotatedFrame::FindStartForJoin( class PieceFrame *obj ) {
int i,j;

    for (i=0;i<obj->vl->Len();i++) {
        for (j=0;j<vl->Len();j++) {
            if (same_point(j,obj,i))        return i;
        }
    }
    return -1;
}

int RotatedFrame::DoJoin( class RotatedFrame *obj, int i, int swap ) {
int j,k;

    for (j=0;j<vl->Len();j++) {
        if (same_point(j,obj,i)) {
        // found intersecting point: now find start and end of intersection
            int s,e;
            int sj;
            int ej;
            int c=1;

            for (s=1;s<obj->vl->Len();s++) {
                int si=(i+s)%obj->vl->Len();
                sj=((j-s)+vl->Len())%vl->Len();
                if (!same_point(sj,obj,si)) break;
                c++;
            }
            s--;
            sj=((j-s)+vl->Len())%vl->Len();
            for (e=1;e<obj->vl->Len();e++) {
                int ei=((i-e)+obj->vl->Len())%obj->vl->Len();
                ej=(j+e)%vl->Len();
                if (!same_point(ej,obj,ei)) break;
                c++;
            }
            e--;
            ej=(j+e)%vl->Len();

        // found start and end of intersection as in shown in diagramm
        // now the points have to be moved to the destination polygon
            DBG2( "*** Range: %d %d\n", sj, ej );
            if ((sj==ej)&&(c>1)) {
                //
                // when a surrounding tile is moved onto of the inner tile, the
                // problem arise that the whole inner tile and the partial surrounding
                // tile has to be deleted. Since this usually happens the other way round:
                // "The hole is filled with the inner tile.", this special case is reversed
                // by swapping both tiles and redoing the join, in which the partial list
                // of the surrounding tile will be found in sj - ej
                //
                DBG0( "*** special inner case: do reverse join\n" );
                Vec2    help_center=center;
                center=obj->center;
                obj->center=help_center;

                Vec2List *help_list=vl;
                vl=obj->vl;
                obj->vl=help_list;

                return DoJoin(obj,j,1);
            }
            join_count+=obj->join_count;

            vl->DelRange(sj,ej,&sj);

            k=(-e+obj->vl->Len())%obj->vl->Len();
            if ( k!=s || (sj==ej&&c==1) ) {
                DBG1( "*** Insert at: %d -", sj );
                do {
                    DBG1( " %d", (i+k)%obj->vl->Len() );
                    vl->AddAt(sj+1,(*obj->vl)(i+k)+obj->center-center);
                    k=(k-1+obj->vl->Len())%obj->vl->Len();
                }
                while( k!=s );
                DBG0( "\n" );
            }
            else {
                // an inner tile was removed from the polyline and therefore
                // some kind of one way trail to the inner tile remained in the
                // polyline which should be removed with a tiny "garbage" collector.
                DBG1( "*** Garbage Deleted (from: %d)", vl->Len() );
                for (j=0;j<vl->Len()-2;j++) {
                    while (same_point(j,this,j+2)) {
                        DBG1( "%d ", j );
                        vl->DelRange(j,j+2,&j);
                        if (j>0)    j--;
                    }
                }
                DBG1( " (to: %d)\n", vl->Len() );
            }

        // the new extend has to be queried to setup the center
            Vec2    tl, br;
            vl->GetExtent( &tl, &br );
            Vec2    new_center( center+((tl+br)/2) );

#if (0)
    printf( "    center: %g %g\n", center.X(), center.Y() );
    printf( "      from: %g %g\n", tl.X(), tl.Y() );
    printf( "        to: %g %g\n", br.X(), br.Y() );
    printf( "new center: %g %g\n", new_center.X(), new_center.Y() );
#endif

            if (swap) {
                // this time the second objects position should be remained
                    winpos = obj->winpos-(obj->center-new_center).ScaleX(page?-1:1).TurnAngleDeg(obj->windir);
            }
            else    winpos -= (center-new_center).ScaleX(page?-1:1).TurnAngleDeg(windir);

            *vl += (center-new_center);
            center=new_center;

            PositionChanged();
            DirectionChanged();
            return 1;
        }
    }
    return 0;
}

// ===========================================================================

FlipFrame::FlipFrame() {
    ftvl=0;
    itm=0;
}

FlipFrame::~FlipFrame() {
    if (ftvl)   delete ftvl;
    if (itm)        delete itm;
}

void FlipFrame::StartFlip( const Real &angle ) {
    mangle = angle;
}
void FlipFrame::StopFlip() {
    if (ftvl)   delete ftvl;
    ftvl=0;
    if (itm)            delete itm;
    itm=0;
}

void FlipFrame::SetFlip( const Real &current ) {
    if (ftvl)       delete ftvl;
    if (itm)            delete itm;

    itm = new Mat2();                   // set up transformation
    if (page)   *itm = (*itm).ScaleX( -1 );
    *itm = (*itm).RotateDeg( -windir )
                    .Rotate( mangle )
                    .ScaleX( current )
                    .Rotate( -mangle );


    ftvl = new Vec2List(RotatedFrame::GetPolyLine(),*itm);
    *itm = !*itm;                       // reverse transformation for texture-mapping
    DirectionChanged();
}

Vec2List &FlipFrame::GetTPolyLine() {
    if (!ftvl)          return *tvl;
    else                    return *ftvl;
}

// ===========================================================================

GC BitmapPiece::gcb = 0;

BitmapPiece::BitmapPiece() {
    tilemask = 0;
}
BitmapPiece::~BitmapPiece() {
    DropBitmap();
}
void BitmapPiece::DropBitmap() {
    if (tilemask) {
        XFreePixmap(dpy,tilemask);
        tilemask=0;
    }
}

void BitmapPiece::PositionChanged() {
    winx   = XPix(winpos.X())-offx;
    winy   = YPix(winpos.Y())-offy;
}

void BitmapPiece::DirectionChanged() {

    DropBitmap();
    FlipFrame::DirectionChanged();

Vec2    tl,br;
int x1,y1,x2,y2;
const Vec2List &poly( GetTPolyLine() );

    poly.GetExtent( &tl, &br );
    x1 = XPix(tl.X());
    y1 = YPix(tl.Y());
    x2 = XPix(br.X());
    y2 = YPix(br.Y());

    offx   = -x1;
    offy   = -y1;
    winx   = XPix(winpos.X())-offx;
    winy   = YPix(winpos.Y())-offy;
    width  = x2-x1+1;
    height = y2-y1+1;
#if (0)
    printf( "%g %g  (%d %d)\n", (double)tl.X(), (double)tl.Y(), offx, offy );
    printf( "center: %g %g\n", (double)center.X(), (double)center.Y() );
    printf( "    %g %g\n", (double)br.X(), (double)br.Y() );
#endif

    tilemask=XCreatePixmap(dpy,RootWindow(dpy,scr),width,height,1);
    if (!gcb)   gcb=XCreateGC(dpy,tilemask,0,0);

    XSetForeground(dpy,gcb,0);                              // clear the new bitmap
    XFillRectangle(dpy,tilemask,gcb,0,0,width,height);

    XSetForeground(dpy,gcb,1);                              // draw the polyline
XPoint  xpts[MaxSplineLen];
XPoint  *xpts_p;
    if ((unsigned)poly.Len()>(sizeof(xpts)/sizeof(XPoint))) {
        xpts_p=new XPoint[poly.Len()];                  // create buffer when necessary
    }
    else xpts_p=xpts;

    for (int i=0;i<poly.Len();i++) {                            // create the pointlist
        Vec2    help( poly[i]-tl );
        xpts_p[i].x = XPix(help.X());
        xpts_p[i].y = YPix(help.Y());
    }
#if (0)
    XDrawLines( dpy, tilemask, gcb, xpts_p, poly.Len(), 0 );
#else
    XFillPolygon(dpy,tilemask, gcb, xpts_p, poly.Len(), 0, 0 );
#endif

#if (0)
    XSetFunction(dpy,gcb,GXxor);                            // draw a test-frame
    XDrawRectangle(dpy,tilemask,gcb,0,0,width-1,height-1);
    XSetFunction(dpy,gcb,GXcopy);
#endif

    if (xpts_p!=xpts)       delete [] xpts_p;
}

void BitmapPiece::Redraw() {
    XSetClipMask(dpy,gc,GetBitmap());
    XSetClipOrigin(dpy,gc,winx,winy);

// printf( "winpos: %g %g\n", (double)winpos.X(), (double)winpos.Y() );
// printf( "offx: %d %d\n", offx, offy );
    XFillRectangle(dpy,win,gc, winx, winy, width, height );
}

// ===========================================================================

GC PixmapPiece::gcp=0;

PixmapPiece::PixmapPiece() {
    tilemap = 0;
    if (!gcp)   gcp=XCreateGC(dpy,RootWindow(dpy,scr),0,0);

}
PixmapPiece::~PixmapPiece() {
    DropPixmap();
}
void PixmapPiece::DropPixmap() {
    if (tilemap) {
        XFreePixmap(dpy,tilemap);
        tilemap=0;
    }
}

#define DATA_TYPE   CARD32
#define DATA_PAD    4
void PixmapPiece::CreateTilemap32() {
#   include "rotate.h"
}
#undef DATA_TYPE
#undef DATA_PAD

#define DATA_TYPE   CARD16
#define DATA_PAD    2
void PixmapPiece::CreateTilemap16() {
#   include "rotate.h"
}
#undef DATA_TYPE
#undef DATA_PAD

#define DATA_TYPE   CARD8
#define DATA_PAD    1
void PixmapPiece::CreateTilemap8() {
#   include "rotate.h"
}
#undef DATA_TYPE
#undef DATA_PAD


void PixmapPiece::DirectionChanged() {
    DropPixmap();
    BitmapPiece::DirectionChanged();

    tilemap=XCreatePixmap(dpy,RootWindow(dpy,scr),width,height,DefaultDepth(dpy,scr));

    if (windir==0&&!itm&&!page) {
        Vec2            wcenter;
        if (page&&pm->IsTwinPixmap())
                    wcenter=Vec2( center.X(), pm->XHeight()+center.Y() );
        else        wcenter=center;

        XCopyArea(dpy,pm->GetPixmap(),tilemap,gcp,
            XPix(wcenter.X())-offx, YPix(wcenter.Y())-offy, width, height, 0, 0 );
    }
    else {
        switch(texture_mode) {
        case 1:     CreateTilemap8();       break;
        case 2:     CreateTilemap16();  break;
        case 3:     CreateTilemap32();  break;
        }
    }
}

void PixmapPiece::Redraw() {
 //printf( "winpos: %g %g\n", (double)winpos.X(), (double)winpos.Y() );// debug
 //printf( "offx: %d %d\n", offx, offy );  //debug

    XSetClipMask(dpy,gc,GetBitmap());
    XSetClipOrigin(dpy,gc,winx,winy);

    GetPixmap();
    XCopyArea(dpy,tilemap,win,gc,
        0, 0, width, height, winx, winy );
}

// ===========================================================================

ShadowedPiece::ShadowedPiece() {
    shadowmap = 0;
    if (!gcp)   gcp=XCreateGC(dpy,RootWindow(dpy,scr),0,0);

}
ShadowedPiece::~ShadowedPiece() {
    DropPixmap();
}
void ShadowedPiece::DropPixmap() {
    if (shadowmap) {
        XFreePixmap(dpy,shadowmap);
        shadowmap=0;
        XFreePixmap(dpy,shadowmask);
    }
}

void ShadowedPiece::DirectionChanged() {
int x;

    DropPixmap();
    PixmapPiece::DirectionChanged();

    swidth = width   + 2*ShadowSize();
    sheight = height + 2*ShadowSize();

// create mask of the shadowed tile
    shadowmask=XCreatePixmap(dpy,RootWindow(dpy,scr),swidth,sheight,1);

    XSetForeground(dpy,gcb,0);
    XFillRectangle(dpy,shadowmask,gcb,0,0,swidth,sheight);

    XSetFunction(dpy,gcb,GXor);
    for (x=0;x<=2*ShadowSize();x++) {
        XCopyArea(dpy,tilemask,shadowmask,gcb,0,0,width,height,x,x);
    }
    XSetFunction(dpy,gcb,GXcopy);

// create the tile
    shadowmap=XCreatePixmap(dpy,RootWindow(dpy,scr),swidth,sheight,DefaultDepth(dpy,scr));

    XSetForeground(dpy,gc,WhitePixel(dpy,scr));
    XSetClipMask(dpy,gc,tilemask);
    for (x=0;x<ShadowSize();x++) {
        XSetClipOrigin(dpy,gc,x,x);
        XFillRectangle(dpy,shadowmap,gc,x,x,width,height);
    }
    XSetForeground(dpy,gc,BlackPixel(dpy,scr));
    for (x=ShadowSize()+1;x<=2*ShadowSize();x++) {
        XSetClipOrigin(dpy,gc,x,x);
        XFillRectangle(dpy,shadowmap,gc,x,x,width,height);
    }

    XSetClipOrigin(dpy,gc,ShadowSize(),ShadowSize());
    XCopyArea(dpy,tilemap,shadowmap,gc,
        0, 0, width, height, ShadowSize(),ShadowSize() );
}

void ShadowedPiece::Redraw() {
    XSetClipMask(dpy,gc,shadowmask);
    XSetClipOrigin(dpy,gc,winx,winy);

    XCopyArea(dpy,shadowmap,win,gc,
        0, 0, swidth, sheight, winx, winy );
}

int ShadowedPiece::IsInside( int x, int y ) {
XImage  *help_image;

    x -= winx;
    y -= winy;
    if (x<0 || y<0 || x>=swidth || y >=sheight) return 0;

    help_image=XGetImage(dpy,shadowmask,x,y,1,1,1,XYPixmap);
    unsigned long erg=XGetPixel(help_image,0,0);
    XDestroyImage(help_image);
    return (int)erg;
}

// ===========================================================================

WindowPiece::WindowPiece() {
    swin=0;

}
WindowPiece::~WindowPiece() {
    if (swin)       XDestroyWindow( dpy, swin );
}

void WindowPiece::CreateWindow() {
    XSetWindowAttributes    attrib;

    swin=XCreateSimpleWindow(dpy,RootWindow(dpy,scr),
            winx,winy,swidth,sheight,0,WhitePixel(dpy,scr), port->AllocNamedColor( "grey50" ) );
    attrib.bit_gravity=StaticGravity;
    attrib.override_redirect = True;
    XChangeWindowAttributes( dpy, swin, CWBitGravity|CWOverrideRedirect, &attrib );
    XSelectInput(dpy,swin, ExposureMask|KeyPressMask|EnterWindowMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
}

void WindowPiece::PositionChanged() {
    ShadowedPiece::PositionChanged();

    
}

void WindowPiece::DirectionChanged() {

    ShadowedPiece::DirectionChanged();

    
}

void WindowPiece::Redraw() {
    XSetClipMask(dpy,gc,shadowmask);
    XSetClipOrigin(dpy,gc,0,0);

    XCopyArea(dpy,shadowmap,swin,gc, 0, 0, swidth, sheight, 0, 0 );
}

// ===========================================================================

PieceObject::PieceObject() {
}

PieceObject::~PieceObject() {
}

int PieceObject::Intersects(int x,int y,int w,int h) {
    if (    (x>winx+swidth) || (winx>x+w)
        ||  (y>winy+sheight) || (winy>y+h) )        return 0;
    else                                                    return 1;
}

int PieceObject::IsInside(int x,int y) {
    if ((x>=winx)&&(x<winx+swidth)&&(y>=winy)&&(y<winy+sheight)) {
        if (ShadowedPiece::IsInside(x,y))       return 2;   // exact hit
        else                                                return 1;   // somewhere near
    }
    else                                                    return 0;   // far away
}

void PieceObject::ExposeRegion(int x,int y,int /*width*/,int /*height*/) {
    XSetClipMask(dpy,gc,shadowmask);
    XSetClipOrigin(dpy,gc,winx-x, winy-y);
    XCopyArea(dpy,shadowmap,mystack->dbmap,gc,
                0, 0, swidth, sheight, winx-x, winy-y );
}

void PieceObject::ExposeWindowRegion(Window w,int /*x*/,int /*y*/,int /*width*/,int /*height*/) {
    if (w==swin) {
        XSetClipMask(dpy,gc,shadowmask);
        XSetClipOrigin(dpy,gc,0,0);

        XCopyArea(dpy,shadowmap,swin,gc, 0, 0, swidth, sheight, 0, 0 );
    }
}

void PieceObject::PanView( int ox, int oy ) {
    winpos-=Vec2(ox,oy);
    PositionChanged();
}

void PieceObject::ZoomView( int midx, int midy, int chg ) {
double  factor=((double)(zoom_factor+chg))/zoom_factor;

    center = center * factor;
    *vl = *vl * factor;
    winpos = (winpos-Vec2(midx,midy)) * factor + Vec2(midx,midy);
    PositionChanged();
    DirectionChanged();
}

// ===========================================================================

int DBPieceObject::x1;
int DBPieceObject::y1;
int DBPieceObject::x2;
int DBPieceObject::y2;

DBPieceObject::DBPieceObject() {
}

DBPieceObject::~DBPieceObject() {
}

void DBPieceObject::TurnOver( const Real &d ) {
        XDefineCursor( dpy, win, no_cursor );
        XFlush(dpy);

Real a;
Real start_windir=windir;

    a=fmod(d-start_windir+360,360);
    if (a>180.0)    a=a-360.0;

    if (join_count<=maxturntiles) {
        double turnstart=GetCurrentTime()-0.1;
        double turntime=turntimebase+join_count*turntimedelta;
        double loop;
        for ( loop=0.1; loop<0.9; loop=(GetCurrentTime()-turnstart)/turntime) {
            StoreExtent();
            SetDir( start_windir+loop*a );
            UpdateExtent();
            XSync(dpy,0);
        }
    }
    StoreExtent();
    SetDir(d);
    UpdateExtent();
}

void DBPieceObject::FlipOver( const Vec2 &pos ) {
        XDefineCursor( dpy, win, no_cursor );
        XFlush(dpy);

Vec2  wpos=pos;                     // button position after probable adjustment
Real    wa=winpos.AngleDeg(pos);    // angle to button position

    if (fmod(windir+5,90)<10) {     // close to straight -> adjusted flip

// Probably round position for exact flips
// int  quart=(int)(fmod(wa+22,360)/45)&3;
int quart=(int)(fmod(wa+45,360)/45)&2;

        switch(quart) {
        case 0: {
                wpos=Vec2( pos.X(), winpos.Y() );           // horizontal flip
                break;
            }
        case 1: {
                Vec2    erg;                                            // diagonal flip 1
                (pos-winpos).Split( Vec2(1,-1), &erg ); // project to diagonal
                wpos=winpos+erg;
                break;
            }
        case 2: {
                wpos=Vec2( winpos.X(), pos.Y() );           // vertival flip
                break;
            }
        case 3: {
                Vec2    erg;                                            // diagonal flip 2
                (pos-winpos).Split( Vec2(1,1), &erg );      // project to diagonal
                wpos=winpos+erg;
                break;
            }
        }
        wa=winpos.AngleDeg(wpos);       // angle to (adjusted position)
    }


Vec2    mdir=wpos-winpos;                   // move distance to center of flip
Vec2    newpos=mdir+wpos;                   // destination on other side of the flip
Real    a=wa+90;                                // angle of mirror
Real    newdir=a-(windir-a)+180;                // new windir after the flip

    DBG1( "*** Mirror Angle: %g\n", (double)a );
    DBG2( "*** Windir: %g -> New Angle: %g\n", (double)wa, (double)newdir );

    if (join_count<=maxfliptiles) {
    // do tiny animation, if there are less than 5 tiles combined

        Real    flipstart=GetCurrentTime();
        Real    loop;               // current state of the flip

        double fliptime=fliptimebase+join_count*fliptimedelta;
        StartFlip( Vec2Zero.AngleRadial(mdir) );// start flipping up
        for (loop=0.9;loop>=flipsave;loop=1.0-(GetCurrentTime()-flipstart)/fliptime) {
            StoreExtent();
            SetPos( wpos-loop*mdir );
            SetFlip( loop );
            UpdateExtent();
            XSync(dpy,0);
        }

        StoreExtent();                                  // swap to other side
        FlipPage();
        SetDir(newdir);

        loop=flipsave;                                  // first frame on back side
        flipstart=GetCurrentTime();
        StartFlip( Vec2Zero.AngleRadial(wpos-newpos) );
        SetPos( wpos+loop*mdir );
        SetFlip(loop);
        UpdateExtent();
        XSync(dpy,0);
        loop=flipsave+(GetCurrentTime()-flipstart)/fliptime;

        for (;loop<1.0-0.1;loop=flipsave+(GetCurrentTime()-flipstart)/fliptime) {
            StoreExtent();
            SetPos( wpos+loop*mdir );
            SetFlip( loop );
            UpdateExtent();
            XSync(dpy,0);
        }

        StoreExtent();                                  // last frame on back side
        SetPos(newpos);
        StopFlip();
        DirectionChanged();                         // trigger for a last update
        UpdateExtent();
    }
    else {
        StoreExtent();                                  // just flip and show
        FlipPage();
        SetPos(newpos);
        SetDir(newdir);
        UpdateExtent();
    }
}

void DBPieceObject::StoreExtent() {
    x1 = winx;                  // store old frame
    y1 = winy;
    x2 = x1+swidth;
    y2 = y1+sheight;
}

int DBPieceObject::JoinExtent( int *xx1, int *yy1, int *xx2, int *yy2 ) {
    if (winx<*xx1)              *xx1 = winx;
    if (winy<*yy1)              *yy1 = winy;
    if (winx+swidth>*xx2)   *xx2 = winx+swidth;
    if (winy+sheight>*yy2)  *yy2 = winy+sheight;
    return 1;
}

int DBPieceObject::GetExtent( int *xx1, int *yy1, int *xx2, int *yy2 ) {
    *xx1 = winx;
    *yy1 = winy;
    *xx2 = winx+swidth;
    *yy2 = winy+sheight;
    return 1;
}

void DBPieceObject::JoinExtent() {
    if (winx<x1)            x1=winx;
    if (winx+swidth>x2) x2=winx+swidth;
    if (winy<y1)            y1=winy;
    if (winy+sheight>y2)    y2=winy+sheight;
}

void DBPieceObject::UpdateExtent() {
    JoinExtent();
    mystack->ExposeRegion(x1,y1,x2-x1,y2-y1);
}

// ===========================================================================

int  MoveablePiece::turnflag;           // flag about direction of turn
Time MoveablePiece::start_time;            // event time of button press
Real MoveablePiece::start_angle;        // angle at the start of turn
Vec2 MoveablePiece::start;
Vec2 MoveablePiece::poffset;            // offset of pointer from center
Real MoveablePiece::poffset_len;        // length of offset of pointer from center

MoveablePiece::MoveablePiece() {
}

MoveablePiece::~MoveablePiece() {
}

void MoveablePiece::DispatchPress( XButtonEvent * xbutton ) {
    if (side_lock<0 && xbutton->state&ControlMask) {
        // printf( "doing the flip ...\n" );
        mystack->Raise(this);
        FlipOver( Vec2(xbutton->x,xbutton->y) );
        start_time=0;
    }
    else {
        start_time=xbutton->time;
    }

    start_angle=GetDir();
    if (xbutton->state&AnyButtonMask ) {
        // there is already a button pressed ...
        if (xbutton->button==Button1) {
            // second button in addition to middle button -> turn around center
            turnflag=2*(xbutton->button-Button1-1);
        }
        else {
            // start non-rotating motion on multiple press
            turnflag=0;
        }
    }
    else if (!fix_angle && !no_rotation){
        // first button - start normal drag
        mystack->Raise(this);
        start=Vec2( xbutton->x, xbutton->y );
        poffset=(start-winpos);
        poffset_len=poffset.Norm();
        

        // when too close to the center of the tile - move it off a bit ...
        while (poffset_len<2) {
            // printf( "*** very close hit - moving start\n" );
            start+=Vec2(1,1);
            poffset=(start-winpos);
            poffset_len=poffset.Norm();
        }

               if (btn_swap)    {
        turnflag=xbutton->button-Button3-1;
              } else {
                 turnflag=xbutton->button-Button1-1;
                }

        if (poffset_len<warp_center) {
            poffset_len=warp_center;
            poffset=poffset_len*poffset.Norm1();
            start=winpos+poffset;
            warp_lock_x = XPix(start.X());
            warp_lock_y = YPix(start.Y());
            XWarpPointer(dpy,None,win,0,0,0,0,warp_lock_x,warp_lock_y);
            XSync(dpy,0);       // Flush and drop the Warp-Event
        }

        if (!turnflag)      Move( start-poffset );
    }else {
        double w=90;
        mystack->Raise(this);
        start=Vec2( xbutton->x, xbutton->y );
        poffset=(start-winpos);
        poffset_len=poffset.Norm();
        if (fmod(start_angle,w ) > 1) { 
         start_angle=start_angle-fmod(start_angle,w ); // rotate to the nearest 90deg
         }

    }

    XDefineCursor( dpy, win, (turnflag<0)?pull_cursor:move_cursor );
}

void MoveablePiece::DispatchMotion( XMotionEvent * xmotion ) {

Vec2    newpos(xmotion->x,xmotion->y);
Vec2    new_offset=newpos-(start-poffset);

    if (fabs(new_offset.X())+fabs(new_offset.Y())<2) {
        DBG0( "#### skipping event near center of tile\n" );
        return;
    }

    switch (turnflag) {
    case 2:
    case -2: {
        // new style for turning around center
            Real    a1=Vec2Zero.AngleDeg(poffset);
            Real    a2=Vec2Zero.AngleDeg(new_offset);
            poffset=new_offset;
            MoveTurn(newpos-poffset,GetDir()-a1+a2);
            break;
    }
    case -1: {
        // new style for pulling corners
            new_offset=(new_offset).Norm1()*poffset_len;
            Real    a1=Vec2Zero.AngleDeg(poffset);
            Real    a2=Vec2Zero.AngleDeg(new_offset);
            poffset=new_offset;
            MoveTurn(newpos-poffset,GetDir()-a1+a2);
            break;
    }
    case 1:
    default:
            Move( newpos-poffset );
            break;
    }
    start=newpos;
}

void MoveablePiece::DispatchRelease( XButtonEvent * xbutton ) {
    if (xbutton->button!=Button2&&(xbutton->button==Button1||!(xbutton->state&Button1Mask))) {
            if (xbutton->time-start_time<mcs) {
                switch(xbutton->button) {
                case Button1:
                 if (join_count<=maxturntiles  && !no_rotation && !any_angle) {
                     if (fmod(start_angle,90.0 ) > 1) { 
                      start_angle=start_angle-fmod(start_angle,90.0 );  // rotate clockwise to the nearest 90deg
                     }
                     TurnOver(start_angle+90.0);
                 }else{
                    if (join_count<=maxturntiles && !no_rotation ) {TurnOver(start_angle+90.0);}
                   }


                 break;
                case Button3:   if ( !no_rotation && !any_angle) {
                     if (fmod(start_angle,90.0 ) > 1) { 
                      start_angle=start_angle-fmod(start_angle,90.0 );  // rotate clockwise to the nearest 90deg
                      TurnOver(fabs(start_angle)); 
                     }else{
                      TurnOver(fabs(start_angle)-90.0); 
                     }
                   }else{
                  if (!no_rotation) {  TurnOver(start_angle-90.0); }
                   }
                break;
               
                }
            }
            turnflag=0;
    }
    else if (side_lock<0 && xbutton->time-start_time<mcs&&xbutton->button==Button2&&!(xbutton->state&~Button2Mask)) {
                FlipOver(Vec2(xbutton->x,xbutton->y));
    }
    else if (xbutton->state&Button1Mask)
            turnflag=-1;    // restart rotatable drag

    if (!((xbutton->state&AnyButtonMask)&~(Button1Mask<<(xbutton->button-1)))) {
        XDefineCursor( dpy, win, idle_cursor );
        XFlush(dpy);
        // no more buttons pressed -> check against other pieces
        if (p->CheckForJoin( (Piece*)this )) {
            /* don't use THIS in this case since it's already deleted !!! */
        }
        else {
            AdjustDirection();
        }
    }
    else XDefineCursor( dpy, win, (turnflag)?pull_cursor:move_cursor );
}

