#ifndef _objects_h
#define _objects_h

/******************************************************************************

  The functionality of the puzzle tiles was created and tested step by step by
  implementing a linear class with each subclass adding a few special features
  to the full object.

PieceFrame    - corner, edge and pin-data of the piece including polygon-spline
      Vec2List *vl
TwinPieceFrame - intermediate class to store the current page of the tile
RotatedFrame  - position-data including
                window-position, upward-angle and rotated polygin spline
      Vec2     winpos
      Real     windir
      Vec2List *tvl
FlipFrame     - intermediate class to control the flip animation with the help
                of a transformation matrix
      Mat2		*itm
      Vec2List *ftvl
BitmapPiece   - pixmap with mask of the rotated Piece
		Pixmap   tilemask
PixmapPiece   - pixmap with Puzzle-Image of the rotated Piece
		Pixmap   tilemap
ShadowPiece   - pixmap with Puzzle-Image and ShadowFrame of the rotated Piece
		Pixmap   shadowmask
		Pixmap   shadowmap
WindowPiece   - class to control creation of local windows for each tile
		Window	swin
PieceObject   - object subclass to be stack-controlled
DBPieceObject - double buffered moves and turns

 ******************************************************************************/

#ifndef _stack_h
#	include "stack.h"
#endif
#ifndef _vec2_h
#	include "vec2.h"
#endif
#ifndef _vec2list_h
#	include "vec2list.h"
#endif
#ifndef _mat2_h
#	include "mat2.h"
#endif

// ===========================================================================

// **************
// * PieceFrame *
// **************
//
// The PieceFrame class containes the information about the boundary corners of
// an image in the original puzzle picture. Therefore the 4 corners are stored
// in that class (measured relative to the center of the object).
// Additionally the information, how the splines of the 4 pins are located,
// is stored in that class.
//
// The data is set up by Puzzle::Init
//
// All the edge information is finally combined into a point-list of a
// polygon, that surrounds the whole tile.

class PieceFrameSetup {
	public:
		PieceFrameSetup();
		virtual ~PieceFrameSetup();
                
		void Init( const Vec2 &tl, const Vec2 &tr, const Vec2 &br, const Vec2 &bl );
		void SetPin( int i, int l, const Real &pd )	{ left[i]=l; pin[i]=pd; }
              
               
	protected:

#define	HalfSplineLen	10
#define	MaxSplineLen	(8*HalfSplineLen+5)

	protected:
		Vec2	v[4];				// coordinates of corners (offset to center)
		char	left[4];			// flag, wether the pin is left ro right
		Real	pin[4];			// offset of pin (-1 .. 1)
		Vec2	center;			// center of piece in origin

		//static double spx[HalfSplineLen];	// Spline-Position for any Pin
		//static double spy[HalfSplineLen];


friend class PieceFrame;
};

// ===========================================================================

class PieceFrame{
	public:
		PieceFrame();
		virtual ~PieceFrame();

		const Vec2List &Init( const PieceFrameSetup &pfs );

		const Vec2List &GetPolyLine()		{ return *vl; }
		const Vec2 &Center()					{ return center; }

		virtual void PositionChanged();
		virtual void DirectionChanged();

	protected:
		Vec2		center;
		Vec2List	*vl;			// non-rotated polyline with pins and corners

		int same_point(int j,class PieceFrame *obj,int i) {
			Vec2	dist( ((*obj->vl)[i]+obj->center) - ((*vl)[j]+center) );
			return (fabs(dist.X())<EPS && fabs(dist.Y())<EPS);
		}
		int	join_count;

	friend class RotatedFrame;
	friend class Puzzle;
};

// ===========================================================================

class TwinPieceFrame : public PieceFrame {
	public:
		TwinPieceFrame();
		virtual ~TwinPieceFrame();

		virtual void FlipPage();

	protected:
		int	page;
};

// ===========================================================================

// ****************
// * RotatedFrame *
// ****************
//
// The PieceFrame gets extended by a window position and and upward angle,
// through which it is possible to compute a rotated list of the polygon points
//
// The public routines SetDir and SetPos can be used for moving the tile to
// a new position, in which case the virtual routines lead to an update of
// the subclasses.

class RotatedFrame : public TwinPieceFrame {
	public:
		RotatedFrame();
		virtual ~RotatedFrame();

		void Init( const PieceFrameSetup &pfs );

		void SetPos( const Vec2 &p )	{ winpos=p; PositionChanged(); }
		const Vec2 &GetPos()				{ return winpos; }
		void SetDir( const Real &d )	{
			windir=fmod(d+360.0,360.0);
			DirectionChanged();
		}
		int AdjustDir() {
			Real	help=fmod(windir+20.0,90);
			if (	help>17 && help<23
				||(join_count>minadjustcount && help<40 && help!=20.0)) {
						SetDir(windir+20-help);
						return 1;
			}
			else	return 0;
		}
		Real GetDir()						{ return windir; }
		void Redraw();

		virtual void PositionChanged();
		virtual void DirectionChanged();
		Vec2List &GetTPolyLine()		{ return *tvl; }

		int CheckForJoin( class RotatedFrame *obj );
		int FindStartForJoin( class PieceFrame *obj );
		int DoJoin( class RotatedFrame *obj, int i, int swap=0 );

	protected:
		Vec2		winpos;			// Window-Position
		Real		windir;			// upward angle of tile;
		Vec2List	*tvl;				// pointlist of turned points

};

// ===========================================================================

class FlipFrame : public RotatedFrame {
	public:
		FlipFrame();
		virtual ~FlipFrame();

		Vec2List &GetTPolyLine();

		void StartFlip( const Real &angle );
		void SetFlip( const Real &current );
		void StopFlip();

	protected:
		Real		mangle;			// flip angle
		Vec2List	*ftvl;
		Mat2		*itm;
};

// ===========================================================================

class BitmapPiece : public FlipFrame {
	public:
		BitmapPiece();
		virtual ~BitmapPiece();

		void DropBitmap();
		virtual void PositionChanged();
		virtual void DirectionChanged();
		Pixmap GetBitmap()				{ return tilemask; }
		void Redraw();

	protected:
		int		winx, winy;			// TopLeft in window
		int		offx, offy;			// TopLeft corner-offset from center
		int		width, height;		// size of bitmap
		Pixmap	tilemask;			// the bitmap itself

		static GC gcb;					// GC to draw in Bitmaps

	friend class DBPieceObject;
};

// ===========================================================================

class PixmapPiece : public BitmapPiece {
	public:
		PixmapPiece();
		virtual ~PixmapPiece();

		void DropPixmap();
		virtual void DirectionChanged();
		Pixmap GetPixmap()				{ return tilemap; }
		void Redraw();

	protected:
		void CreateTilemap8();
		void CreateTilemap16();
		void CreateTilemap32();

		Pixmap	tilemap;
		static GC	gcp;
};

// ===========================================================================

class ShadowedPiece : public PixmapPiece {
	public:
		ShadowedPiece();
		virtual ~ShadowedPiece();

		void DropPixmap();
		virtual void DirectionChanged();
		Pixmap GetPixmap()				{ return shadowmap; }
		void Redraw();

		int ShadowSize()					{ return shadow_size; }

		int IsInside( int x, int y );

	protected:
		Pixmap	shadowmask;
		Pixmap	shadowmap;
		int		swidth,sheight;

	friend class DBPieceObject;
};

// ===========================================================================

class WindowPiece : public ShadowedPiece {
	public:
		WindowPiece();
		virtual ~WindowPiece();

		virtual void PositionChanged();
		virtual void DirectionChanged();
		void Redraw();

	protected:
		void CreateWindow();
		Window	swin;

	friend class WindowObjectStack;
};

// ===========================================================================

class PieceObject : public WindowPiece, public Object {
	public:
		PieceObject();
		virtual ~PieceObject();

		virtual int Intersects(int x,int y,int width,int height);
		virtual int IsInside(int x,int y);
		virtual void ExposeRegion(int x,int y,int width,int height);
		virtual void ExposeWindowRegion(Window w, int x,int y,int width,int height);

		virtual void PanView( int offx, int offy );
		virtual void ZoomView( int midx, int midy, int chg );

	private:
};

// ===========================================================================

class DBPieceObject : public PieceObject {
	public:
		DBPieceObject();
		virtual ~DBPieceObject();

		void Move( const Vec2 &pos )
			{ StoreExtent(); SetPos(pos);  UpdateExtent(); };
		void MoveTurn( const Vec2 &pos, const Real &d )
			{ StoreExtent(); SetPos(pos); SetDir(d); UpdateExtent(); };
		void Turn( const Real &d )
			{ StoreExtent(); SetDir(d); UpdateExtent(); };
		void FlipOver( const Vec2 &pos );		// animated flip
		void TurnOver( const Real &d );			// animated turn
		void AdjustDirection()
			{ StoreExtent(); if (PieceObject::AdjustDir()) UpdateExtent(); };

		int JoinExtent( int *x1, int *y1, int *x2, int *y2 );
		int GetExtent( int *x1, int *y1, int *x2, int *y2 );
		void StoreExtent();
		void JoinExtent();
		void UpdateExtent();

	private:
		static int x1,y1,x2,y2;
};

// ===========================================================================

class MoveablePiece : public DBPieceObject {
	public:
		MoveablePiece();
		virtual ~MoveablePiece();

		virtual void DispatchPress( XButtonEvent * /*xbutton*/ );
		virtual void DispatchRelease( XButtonEvent * /*xbutton*/ );
		virtual void DispatchMotion( XMotionEvent * /*xmotion*/ );

	private:
		static int	turnflag;			//   -1 = left, 0 = no turn, 1 = right
												// -2/2 = around center
		static Time	start_time;
		static Real	start_angle;
		static Vec2	start;
		static Vec2 poffset;
		static Real poffset_len;
};

// ===========================================================================

class Piece : public MoveablePiece {
};

#endif
