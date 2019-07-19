#ifndef _puzzle_h
#define _puzzle_h

#ifndef _vec2_h
#	include "vec2.h"
#endif
#ifndef _object_h
	// to get Piece
#	include "objects.h"
#endif

//
// Class to create a grid, which should be the frame for the puzzle-pieces
//

class Grid {
	public:
		Grid(int w,int h);
		~Grid();

		void Init(int maxx, int maxy);
		void Reset( int x, int y );
		void Randomize(int percent);

		Vec2 &P(int x,int y)				{ return p[x+width*y]; }
		const Real &X(int x,int y)		{ return p[x+width*y].X(); }
		const Real &Y(int x,int y)		{ return p[x+width*y].Y(); }

		int	Width()						{ return width; }
		int	Height()						{ return height; }

	private:
		int	width, height;
		int	max_width, max_height;
		Vec2	*p;
};

class Puzzle {
	public:
		Puzzle();
		~Puzzle();

		Piece &P(int x,int y)			{ return *(p[x+width*y]); }

		void Init(int img_width, int img_height, int dx, int dy, const char *sfx );
		int CheckForJoin( Piece *pi, int depth=0 );
		void DropTile( int x, int y ) {
			DropTile(x+y*width);
		}
		void DropTile( int i ) {
			delete p[i];
			p[i]=0;
			tiles_left--;

		}
		void Redraw();
		void Rotation();
		int Finished()		{ return (tiles_left<=1); }
		int PiecesLeft()	{ return (tiles_left); }

	private:
		int	width,height;
		int	tiles_left;
		Piece			**p;
		Grid			*g;
};

#endif
