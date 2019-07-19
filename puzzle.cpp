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
#	include "global.h"
#endif
#ifndef _puzzle_h
#	include "puzzle.h"
#endif

Grid::Grid(int w, int h) {
	width  = w;
	height = h;

	p = new Vec2[width*height];
}

Grid::~Grid() {
	delete [] p;
}

void Grid::Reset(int x, int y) {
		P(x,y) = Vec2( x*max_width/(width-1), y*max_height/(height-1) );
}

void Grid::Init(int maxx, int maxy) {
int	x,y;

	max_width  = maxx;
	max_height = maxy;
	for (x=0;x<width;x++) {
		for (y=0;y<height;y++) {
			Reset(x,y);
		}
	}
}

void Grid::Randomize(int percent) {
int	x,y;

	for (x=1;x<width-1;x++) {
		for (y=1;y<height-1;y++) {
			Vec2	off( ((max_width/(width-1))/10000.0*percent) * (my_rand()%100)
			, ((max_height/(height-1))/10000.0*percent) * (my_rand()%100) );
			P(x,y) = Vec2( x*max_width/(width-1), y*max_height/(height-1) ) + off;
		}
	}

}

Puzzle::Puzzle() {
	g=0;
	p=0;
	tiles_left=0;
}

Puzzle::~Puzzle() {
	if (g)		delete g;
	if (p) {
		for (int i=0;i<width*height;i++)
			if (p[i])	delete p[i];
		delete [] p;
	}
}

void Puzzle::Init(int img_width, int img_height, int dx, int dy, const char *sfx ) {
int x,y;

PieceFrameSetup	*pfs;
Vec2					*pos;
#define	PFS(x,y)	(pfs[(x)+width*(y)])

	width  = dx;
	height = dy;
	tiles_left=width*height;

	// create and initialize the grid
	// check for special effect extension and reset the desired Dots
	g = new Grid(width+1,height+1);
	g->Init(img_width,img_height);
	g->Randomize(distortion);
	if (sfx) {
		int dir,len;
		const char *sfx_p=sfx;
		while( *sfx_p ) {
			if (sscanf( sfx_p, "%02x%02x%01x%01x", &x, &y, &len, &dir )!=4) {
				fprintf( stderr, "*** image error in extension 0x12\n" );
				exit(0);
			}
			g->Reset(x,y);
			while( len-- ) {
				switch(dir) {
				case 0:	x++; break;
				case 1:	y++; break;
				case 2:	x--; break;
				case 3:	y--; break;
				}
				g->Reset(x,y);
			}
			sfx_p+=6;
                       
		}
	}

	// create and initialize setup-structures for all pieces
	pfs = new PieceFrameSetup[width*height];
	pos = new Vec2[width*height];


	// setup corner information
	for (x=0;x<width;x++) {
		for (y=0;y<height;y++) {
			PFS(x,y).Init(g->P(x,y),g->P(x+1,y),g->P(x+1,y+1),g->P(x,y+1));
			pos[x+width*y]=g->P(x,y)+Vec2(offx*(x+1)+img_width/dx/2,offy*(y+1)+img_height/dy/2);
		}
	}

	// setup pin information
	for (x=0;x<width-1;x++) {
		for (y=0;y<height;y++) {
			// Real	off = (my_rand()%100)/500.0+0.4;
			Real	off = (my_rand()%100)/250.0+0.3;
			int	left=my_rand()%2;

			PFS(x,y).SetPin(1,left,off);
			PFS(x+1,y).SetPin(3,1-left,1.0-off);
		}
	}
	for (y=0;y<height-1;y++) {
		for (x=0;x<width;x++) {
			Real	off = (my_rand()%100)/500.0+0.4;
			int	left=my_rand()%2;

			PFS(x,y).SetPin(2,left,off);
			PFS(x,y+1).SetPin(0,1-left,1.0-off);
		}
	}

	// override Pin-Information for special effects
	if (sfx) {
		int dir,len;
		const char *sfx_p=sfx;
		while( *sfx_p ) {
			if (sscanf( sfx_p, "%02x%02x%01x%01x", &x, &y, &len, &dir )!=4) {
				fprintf( stderr, "*** image error in extension 0x12\n" );
				exit(0);
			}
			while( len-- ) {
				switch(dir) {
				case 0:	PFS(x,y-1).SetPin(2,0,0); PFS(x,y).SetPin(0,0,0); x++; break;
				case 1:	PFS(x-1,y).SetPin(1,0,0); PFS(x,y).SetPin(3,0,0); y++; break;
				case 2:	x--; PFS(x,y-1).SetPin(2,0,0); PFS(x,y).SetPin(0,0,0); break;
				case 3:	y--; PFS(x-1,y).SetPin(1,0,0); PFS(x,y).SetPin(3,0,0); break;
				}
			}
			sfx_p+=6;
		}
	}

	if (shuffle&2) {
		for (x=0;x<width*height;x++) {
			int	i=rand()%(width*height);
			Vec2	swap(pos[i]);
			pos[i]=pos[x];
			pos[x]=swap;
		}
	}
	if (shuffle&4) {
		for (x=0;x<width*height;x++) {
			pos[x]=Vec2(offx,offy)+Vec2(rand()%(img_width+offx*width-offx),rand()%(img_height+offy*height-offy));
		}
	}

	// create pieces and let them be initialized with the setup structure
	p = new Piece*[width*height];
	for (x=0;x<width;x++) {
		for (y=0;y<height;y++) {
			p[x+width*y]=new Piece;
			P(x,y).Init(PFS(x,y));
			if (straight_setup>=0) {
				int	row=y;
				int	col=x;

					// query the offset of the center from the grid edge in the original piece
				Vec2	org_offset(P(x,y).Center()-Vec2(x*img_width/width,y*img_height/height));
					// compute the piece edge in the 'straight' position
				Vec2	new_pos( col*img_width/width+col*straight_setup, row*img_height/height+row*straight_setup );
				 
                                 P(x,y).SetPos( new_pos+org_offset );
                                
			}
			else {
                                int	row=y;
				int	col=x;
                                

                                if (sortedges==1 &&(col==width-1 || col==0 || row==0 || row==height-1)) {
				    Vec2 new_pos( 700,rand()%(img_height+offy*height-offy));
                                   //Vec2 new_pos( rand()%(dx*img_width/width), rand()%(img_height+offy*height-offy) );
				    P(x,y).SetPos( new_pos );
                                }else{
                                   P(x,y).SetPos(pos[x+y*width]);
                                }
			}

			if (shuffle&1)	{	
                             if (fix_angle) {  
                                 P(x,y).SetDir( ((rand()%(int)(2))-1) + (rand()%4)*90 );
                           } else {
                                P(x,y).SetDir( ((rand()%(int)(maxang*2))-maxang) + (rand()%4)*90 );
			    }
                         } 
                         else {				
                              P(x,y).SetDir(angle*(x+y));
                         }

			if (side_lock<0) {
				if (rand()&2)		P(x,y).FlipPage();
			}
			else {
				if (side_lock==1)	P(x,y).FlipPage();
			}
			stk->Append( &P(x,y) );
		}
	}

	delete [] pos;
	delete [] pfs;
#undef PFS
}

void Puzzle::Redraw() {
	for (int i=0;i<width*height;i++)
		if (p[i])	p[i]->Redraw();

}

int Puzzle::CheckForJoin( Piece *pi, int depth ) {
int i,j,s;

	for (i=0;i<width*height;i++)
		if (p[i]&&p[i]!=pi) {
		DBG1("=== %d\n",i);
			if (p[i]->CheckForJoin(pi)) {
				if ((s=p[i]->FindStartForJoin(pi))>=0) {
					// start double buffering

					p[i]->StoreExtent();
					pi->JoinExtent();

					p[i]->DoJoin(pi,s,(depth>0)?((p[i]->join_count>=pi->join_count)?0:1):0);
					// The part was join, so it can be deleted ...
					for (j=width*height-1;j>=0;j--) {
						if (p[j]==pi)	break;
					}
					if (j>=0)	DropTile(j);

					p[i]->UpdateExtent();
					if (depth<maxsnapretries)
						CheckForJoin(p[i],depth+1);	// recursive call to connect parts
					return 1;
				}
				else {
				}
			}
		}
	return 0;
}

void Puzzle::Rotation() {
#if (0)
Real	tangle;

	for (tangle=0;tangle<360;tangle+=10) {
		for (int x=0;x<width;x++) {
			for (int y=0;y<height;y++) {
				P(x,y).Turn(tangle+angle*(x+y));
			}
		}
	}
#else
	for (int i=0;i<32;i++) {
		int x=rand()%width;
		int y=rand()%height;
		stk->Raise(&P(x,y));
		Vec2	dir((rand()%7)-3,(rand()%7)-3);
		for (int j=0;j<18;j++) {
				P(x,y).MoveTurn(P(x,y).GetPos()+dir,P(x,y).GetDir()+5.0);
				if (quit)	return;
		}
	}
#endif
}

