#ifndef _vec2_h
#define _vec2_h

#ifndef _real_h
#	include "real.h"
#endif

#if (Vec2IsVector)

#ifndef _vector_h
#	include "vector.h"
#endif

class Vec2 : public Vector {
	public:
		const Real &X() const 							{ return data[0]; }
		const Real &Y() const 							{ return data[1]; }

		Vec2( double x, double y ) : Vector( 2, x, y )		{ }
		Vec2( const Vector &v ) : Vector( v )					{ Resize(2); }
		Vec2()															{ }

		const Vec2& operator=(const Vector &v)
		{ Vector::operator=(v); return *this; }

		Vec2 TurnLeft() const					{ return Vec2( -Y(), X() ); }
		Vec2 TurnRight() const					{ return Vec2( Y(), -X() ); }
		Vec2 TurnAngleRad( const Real &angle ) const;
		Vec2 TurnAngleDeg( const Real &angle ) const
			{ return TurnAngleRad(angle/Real(180/M_PI)); }

		void Split( const Vec2 &d, Vec2 *vx, Vec2 *vy ) const;
		void Split( const Vec2 &d, Vec2 *vx ) const;

		Real AngleRadial( const Vec2 &d ) const;
		Real AngleDeg( const Vec2 &d ) const		{ return AngleRadial(d)*Real(180/M_PI); }

		static int Solve(	const Vec2 &p1, const Vec2 &d1,
						const Vec2 &p2, const Vec2 &d2, Real *t1 );
};
#else
//
// -------------------------------------------------------------------------
// class Vec2 : Vektorklasse, die einfach die komplexen Zahlen erweitert
// -------------------------------------------------------------------------
//

class Vec2 {
	private:
		Real	x_val;
		Real	y_val;

	public:
		const Real &X() const 							{ return x_val; }
		const Real &Y() const 							{ return y_val; }

		int	IsZero() const								{ return X()==0.0&&Y()==0.0; }

		Vec2( const Real &x, const Real &y )		{ x_val=x; y_val=y; }
		Vec2( const Vec2 &v )						{ x_val=v.X(); y_val=v.Y(); }
		Vec2()												{ }

		Real SqrNorm() const;
		Real Norm() const;
		Vec2 Norm1() const;

		Vec2 TurnLeft() const				{ return Vec2( -Y(), X() ); }
		Vec2 TurnRight() const			{ return Vec2( Y(), -X() );	}
		Vec2 TurnAngleRad( const Real &angle ) const;
		Vec2 TurnAngleDeg( const Real &angle ) const
			{ return TurnAngleRad(angle/Real(180/M_PI)); }

		void Split( const Vec2 &d, Vec2 *vx, Vec2 *vy ) const;
		void Split( const Vec2 &d, Vec2 *vx ) const;

		Real AngleRadial( const Vec2 &d ) const;
		Real AngleDeg( const Vec2 &d ) const		{ return AngleRadial(d)*Real(180/M_PI); }

		static int Solve(	const Vec2 &p1, const Vec2 &d1,
						const Vec2 &p2, const Vec2 &d2, Real *t1 );

	Vec2 ScaleX(const Real &sx)	{ return Vec2(x_val*sx,y_val); }
	Vec2 ScaleY(const Real &sy)	{ return Vec2(x_val,y_val*sy); }

	inline const Vec2& operator=(const Vec2 &v);

	// Binary Operator Functions

	inline Vec2 operator+(const Vec2&) const;
	inline Vec2 operator-(const Vec2&) const;

#ifndef __TURBOC__
	friend inline Real operator*(const Vec2&, const Vec2&);
	friend inline Vec2 operator*(const Real&, const Vec2&);
	friend inline int operator==(const Vec2&, const Vec2&);
	friend inline int operator!=(const Vec2&, const Vec2&);
#else
	friend Real operator*(const Vec2&, const Vec2&);
	friend Vec2 operator*(const Real&, const Vec2&);
	friend int operator==(const Vec2&, const Vec2&);
	friend int operator!=(const Vec2&, const Vec2&);
#endif

	inline Vec2 operator*(const Real&) const;
	inline Vec2 operator/(const Real&) const;

	inline const Vec2& operator+=(const Vec2&);
	inline const Vec2& operator-=(const Vec2&);
	inline const Vec2& operator*=(const Real&);
	inline const Vec2& operator/=(const Real&);
	inline Vec2 operator+() const;
	inline Vec2 operator-() const;

};

inline const Vec2& Vec2::operator=(const Vec2 &v) {
	x_val = v.x_val;
	y_val = v.y_val;
	return *this;
}

inline Vec2 Vec2::operator+() const
{
	return *this;
}

inline Vec2 Vec2::operator-() const
{
	return Vec2(-x_val, -y_val);
}


// Definitions of compound-assignment operator member functions

inline const Vec2& Vec2::operator+=(const Vec2& z2)
{
	x_val += z2.x_val;
	y_val += z2.y_val;
	return *this;
}

inline const Vec2& Vec2::operator-=(const Vec2& z2)
{
	x_val -= z2.x_val;
	y_val -= z2.y_val;
	return *this;
}

inline const Vec2& Vec2::operator*=(const Real& val)
{
	x_val *= val;
	y_val *= val;
	return *this;
}

inline const Vec2& Vec2::operator/=(const Real& val)
{
	x_val /= val;
	y_val /= val;
	return *this;
}


// Definitions of non-member binary operator functions

inline Vec2 Vec2::operator+(const Vec2& z2) const
{
		  return Vec2(x_val + z2.x_val, y_val + z2.y_val);
}
inline Vec2 Vec2::operator-(const Vec2& z2) const
{
		  return Vec2(x_val - z2.x_val, y_val - z2.y_val);
}


inline Real operator*(const Vec2& z1, const Vec2& z2)
{
		  return z1.x_val*z2.x_val + z1.y_val*z2.y_val;
}
inline Vec2 Vec2::operator*(const Real& val2) const
{
		  return Vec2(x_val*val2, y_val*val2);
}
inline Vec2 operator*(const Real& val, const Vec2& z2)
{
		  return Vec2(z2.x_val*val, z2.y_val*val);
}

inline Vec2 Vec2::operator/(const Real& val) const
{
		  return Vec2(x_val/val, y_val/val);
}

inline int operator==(const Vec2& z1, const Vec2& z2)
{
		  return z1.x_val == z2.x_val && z1.y_val == z2.y_val;
}

inline int operator!=(const Vec2& z1, const Vec2& z2)
{
		  return z1.x_val != z2.x_val || z1.y_val != z2.y_val;
}

inline Real Vec2::SqrNorm() const				{ return X()*X()+Y()*Y(); }
inline Real Vec2::Norm() const					{ return sqrt(SqrNorm()); }
inline Vec2 Vec2::Norm1() const				{ return *this / Norm(); }

#endif

extern Vec2 Vec2Zero;

#endif
