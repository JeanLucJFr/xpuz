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
   
#ifndef _vec2list_h
#define _vec2list_h

#ifndef _vec2_h
#	include "vec2.h"
#endif

//
// -------------------------------------------------------------------------
// class Vec2 : Vektorklasse, die einfach die komplexen Zahlen erweitert
// -------------------------------------------------------------------------
//

class Vec2List {
	private:
		Vec2	*v;
		int	len;
		int	alloc_len;

	public:
		Vec2List( int len=2 );
		Vec2List( const Vec2List &vl );
		Vec2List( const Vec2List &vl, const class Mat2 &m );
		~Vec2List()											{ if (v)		delete [] v; }

		void TurnLeft();
		void TurnRight();
		void TurnAngleRad( const Real &angle );
		void TurnAngleDeg( const Real &angle ) { TurnAngleRad(angle/Real(180/M_PI)); }

		const Vec2List& operator=(const Vec2List &v);

		int Len() const									{ return len; }
		const Vec2 &operator[](int i) const			{ return v[i]; }
		const Vec2 &operator()(int i) const			{ return v[(i+len)%len]; }

		const Vec2List &SetAt( int i, const Vec2 &z );
		const Vec2List &AddAt( int i, const Vec2 &z );
		const Vec2List &Del( int i );
		const Vec2List &DelRange( int i, int j, int *erg );

		void GetExtent( Vec2 *tl, Vec2 *br ) const;

	// Binary Operator Functions

		inline Vec2List operator+(const Vec2&) const;
		inline Vec2List operator-(const Vec2&) const;

#ifndef __TURBOC__
		friend inline Vec2List operator*(const Real&, const Vec2List&);
		friend int operator==(const Vec2List&, const Vec2List&);
		friend inline int operator!=(const Vec2List&, const Vec2List&);
#else
		friend Vec2List operator*(const Real&, const Vec2List&);
		friend extern int operator==(const Vec2List&, const Vec2List&);
		friend int operator!=(const Vec2List&, const Vec2List&);
#endif

		inline Vec2List operator*(const Real&) const;
		inline Vec2List operator/(const Real&) const;

		const Vec2List& operator+=(const Vec2&);
		const Vec2List& operator-=(const Vec2&);
		const Vec2List& operator*=(const Real&);
		const Vec2List& operator/=(const Real&);
		const Vec2List& operator|=(const Vec2&);

		const Vec2List& operator*=(const class Mat2&);

		inline Vec2List operator+() const;
		inline Vec2List operator-() const;
};

inline Vec2List Vec2List::operator+() const
{
	return *this;
}

inline Vec2List Vec2List::operator-() const
{
	Vec2List help(*this);
	return help*=-1;
}


// Definitions of compound-assignment operator member functions

inline Vec2List Vec2List::operator+(const Vec2 &z) const
{
		Vec2List help(*this);
		return help-=z;
}
inline Vec2List Vec2List::operator-(const Vec2 &z) const
{
		Vec2List help(*this);
		return help-=z;
}
inline Vec2List Vec2List::operator*(const Real& val2) const
{
		Vec2List help(*this);
		return help*=val2;
}
inline Vec2List operator*(const Real& val, const Vec2List& z2)
{
		Vec2List	help(z2);
		return help*=val;
}

inline Vec2List Vec2List::operator/(const Real& val) const
{
		Vec2List	help(*this);
		return help/=val;
}

extern int operator==(const Vec2List& z1, const Vec2List& z2);

inline int operator!=(const Vec2List& z1, const Vec2List& z2)
{
		  return !(z1==z2);
}

#endif

