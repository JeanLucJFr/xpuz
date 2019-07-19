#ifndef _global_h
#	include "global.h"
#endif

#ifndef _vec2_h
#	include "vec2.h"
#endif

Vec2 Vec2Zero( RealZero, RealZero );

//
// Splitting the vector into 2 vectors parallel and vertical to the
// Direction vector d given by the following equation system is solved:
//
//	   vx        dx        -dy
//	  (  ) =	l *(  ) + u *(   )
// 	vy        dy         dx
//
//     V()  =   x     +   y
//
void Vec2::Split( const Vec2 &d, Vec2 *vx, Vec2 *vy ) const
{
Real	l,u;

	if (d.Y()!=RealZero) {
		l  = (   Y() + (   X() * d.X() / d.Y() ) )
			/ ( d.Y() + ( d.X() * d.X() / d.Y() ) );
		u  = ( l * d.X() - X() ) / d.Y();
		*vx = Vec2( l*  d.X() , l*d.Y() );
		*vy = Vec2( u*(-d.Y()), u*d.X() );
	}
	else {
		if (d.X()!=RealZero) {
			*vx = Vec2( X(), RealZero );		// parallel zur X-Achse
			*vy = Vec2(RealZero, Y() );
		}
		else {
			*vx = *this;					// keine Richtung -> gesamter Vektor ist x
			*vy = Vec2Zero;
		}
	}
}

//
//Analogous to the complete splitting is used in the following
// Version of split returned only the parallel component.
//
void Vec2::Split( const Vec2 &d, Vec2 *vx ) const
{
Real	l;

	if (d.Y()!=RealZero) {
		l  = (   Y() + (   X() * d.X() / d.Y() ) )
			/ ( d.Y() + ( d.X() * d.X() / d.Y() ) );
		*vx = Vec2( l*  d.X() , l*d.Y() );
	}
	else {
		if (d.X()!=RealZero) {
			*vx = Vec2( X(), RealZero );		// parallel zur X-Achse
		}
		else {
			*vx = *this;					// keine Richtung -> gesamter Vektor ist x
		}
	}
}


//
// Calculate the angle that the specified point to the current Korrdinate
// Has. Result is between 0 and 2 * M_PI
//
Real Vec2::AngleRadial( const Vec2 &d ) const
{
Real	erg;
Real	dx=d.X()-X();
Real	dy=d.Y()-Y();
Real	fdx=fabs(dx);
Real	fdy=fabs(dy);

	if (fdx>fdy) {
		if (fdx>1e-10)		erg = atan( -dy/dx );
		else					erg = (dy<0)?M_PI_2:3*M_PI_2;		// Fehler behoben ???

		if (dx<0)			erg+= M_PI;
	}
	else {
		if (fdy>1e-10)		erg = atan( dx/dy );
		else					erg = (dx>0)?M_PI_2:3*M_PI_2;		// Fehler behoben ???

		if (dy<0)			erg+= M_PI;
		erg-=M_PI_2;
	}
	if (erg<RealZero)		erg+= 2*M_PI;
	return erg;
}

Vec2 Vec2::TurnAngleRad( const Real &angle ) const
{
	if (!IsZero()) {
			Real	len = Norm();
			Real	ang = Vec2Zero.AngleRadial(*this) + angle;

			return Vec2( len*cos(ang), -len*sin(ang) );
	}
	else	return *this;
}

//
//Solution of the equation system:	p1+t1*d1 = p2+t2*d2
// after the two "times" t1 und t2
//
int Vec2::Solve(	const Vec2 &p1, const Vec2 &d1,
							const Vec2 &p2, const Vec2 &d2, Real *t1 )
{
		if (d1.X()!=RealZero) {
			Real div = d2.Y()-d2.X()/d1.X()*d1.Y();
			if (div==RealZero)		{ *t1=RealZero; return 1; }		// parallel
			*t1 = ( p1.Y()-p2.Y()+
					(p2.X()-p1.X())/d1.X()*d1.Y() )
					/ div;
		}
		else {
			Real div = d2.X()	/* -d2.Y()/d1.Y()*d1.X() */;
			if (div==RealZero)		{ *t1=RealZero; return 1; }		// parallel
			*t1 = ( p1.X()-p2.X()
					/* + (p2.Y()-p1.Y())/d1.Y()*d1.X() */ )
					/ div;
		}
		return 0;											// Ergebnis ok.
}

#if (0)
int Vec2::Project( const Vec2 &p1, const Vec2 &d1,
							const Vec2 &p2, Real *t1 )
{
	return Solve(p1,d1,p2,d1.TurnLeft(),t1);
}
#endif
