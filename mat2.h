
#ifndef _mat2_h
#define _mat2_h

// #include <math.h>
#ifndef _real_h
#	include "real.h"
#endif
#ifndef _vec2_h
#	include "vec2.h"
#endif

// 
// Member:  r11 r12 tx
//          r21 r22 ty
//           0   0   1
// 
// Move:    1  0 tx   Scale:  sx  0  0   Rotate:  cosa -sina  0
//          0  1 ty            0 sy  0            sina  cosa  0
//          0  0  1            0  0  1             0     0    1
//
// ShearX:  1  a  0   ShearY:  1  0  0
//          0  1  0            b  1  0
//          0  0  1            0  0  1
// 
// Multiplikation:   C = A * B
//          cr11 = ar11*br11 + ar12*br21;
//          cr21 = ar21*br11 + ar22*br21;
//          cr12 = ar11*br12 + ar12*br22;
//          cr22 = ar21*br12 + ar22*br22;
//          ctx  = ar11*btx  + ar12*bty  + atx;
//          cty  = ar21*btx  + ar22*bty  + aty;
// 
// inverse Matrix:
//          detA = r11*r22 - r12*r21
// 
//           -1    r22/detA  -r12/detA 
//          A  =  -r21/detA   r11/detA
//          tx =  (r12*ty-r22*tx)/detA
//          ty =  (r21*tx-r11*ty)/detA
// 
// Punktumrechnung:
//                   p2 = A * p1
//          x2 = x1*r11 + y1*r12 + tx;
//          y2 = x1*r21 + y1*r22 + ty;
// 
//                         -1
//                   p1 = A  * p2;
//          x1 = ( x2*r22 - y2*r21)/detA - tx;
//          y1 = (-x2*r12 + y2*r11)/detA - ty;
// 
// -------------------------------------------------------------------------

class Mat2
{
   
   public:
		//
		// Konstruktoren
		//
      Mat2()
			{ r11=r22=1.0; r12=r21= tx=ty= 0.0; }
      Mat2( const Mat2 &m )
			{ r11=m.r11; r12=m.r12; r21=m.r21; r22=m.r22; tx=m.tx; ty=m.ty; }
      Mat2(const Real &ir11, const Real &ir12, const Real &itx,
					const Real &ir21, const Real &ir22, const Real &ity )
			{ r11=ir11; r12=ir12; r21=ir21; r22=ir22; tx=itx; ty=ity; }
		//
		// Methoden zum Anwenden einer Transformation.
		// Die der Transformation entsprechende Matrix wird von links!!
		// in die aktuelle Matrix hineinmultipliziert.
		//
      Mat2 &Reset();

      Mat2 &Move(const Real &dx, const Real &dy);
      Mat2 &Move(const Vec2 &p);
      Mat2 &MoveX(const Real &dx);
      Mat2 &MoveY(const Real &dy);

      Mat2 &Scale(const Real &sx, const Real &sy);
      Mat2 &Scale(const Real &s);
      Mat2 &ScaleX(const Real &sx);
      Mat2 &ScaleY(const Real &sy);
      Mat2 &ScaleAt(const Real &px, const Real &py,const Real &sx, const Real &sy);
      Mat2 &ScaleAt(const Vec2 &p,const Real &sx, const Real &sy);
      Mat2 &ScaleAt(const Real &px, const Real &py,const Real &s);
      Mat2 &ScaleAt(const Vec2 &p,const Real &s);

      Mat2 &Rotate(const Real &a);
      Mat2 &RotateAt(const Real &px, const Real &py,const Real &a);
      Mat2 &RotateAt(const Vec2 &p,const Real &a);

      Mat2 &RotateDeg(const Real &a);
      Mat2 &RotateDegAt(const Real &px, const Real &py,const Real &a);
      Mat2 &RotateDegAt(const Vec2 &p,const Real &a);

      Mat2 &ShearX(const Real& a);
      Mat2 &ShearY(const Real& b);
      Mat2 &Shear(const Real& a, const Real& b);

      int IsRotated() const { return r12 || r21; }

		//
		// Zuweisungsoperator
		//
      const Mat2& operator=(const Mat2 &m)
			{	r11=m.r11; r12=m.r12; r21=m.r21; r22=m.r22; tx=m.tx; ty=m.ty;
				return *this; }

		//
		// Operatoren
		//
      const Mat2 &operator*=(const Mat2 &m);
      Mat2 operator*(const Mat2 &m) const
			{
				Mat2 help(*this);
				return help*=m;
			}
      void Invert(Mat2 *m) const;
      Mat2 operator!() const
			{	Mat2	m;
				Invert( &m );
				return m;
			}

      Vec2 operator*(const Vec2 &p) const {
			return Vec2( r11*p.X() + r12*p.Y() + tx, r21*p.X() + r22*p.Y() + ty );
		}

		//
		// Zugriff auf Koeffizienten
		//
      void Split( Real *sh, Real *sx, Real *sy, Real *angle, Real *mx, Real *my ) const;
      
   protected:
      
      Real	r11, r12, tx;
      Real	r21, r22, ty;
      
};


inline Mat2 &Mat2::Reset()
			{ r11=r22=1.0; r12=r21= tx=ty= 0.0; return *this; }
inline Mat2 &Mat2::Move(const Real &dx, const Real &dy)
			{ tx+=dx; ty+=dy; return *this; }
inline Mat2 &Mat2::Move(const Vec2 &p)
			{ tx+=p.X(); ty+=p.Y(); return *this; }
inline Mat2 &Mat2::MoveX(const Real &dx)
			{ tx+=dx; return *this; }
inline Mat2 &Mat2::MoveY(const Real &dy)
			{ ty+=dy; return *this; }
inline Mat2 &Mat2::Scale(const Real &sx, const Real &sy)
			{ r11*=sx; r21*=sy; r12*=sx; r22*=sy; tx*=sx; ty*=sy; return *this; }
inline Mat2 &Mat2::Scale(const Real &s)
			{ return Scale(s,s); }
inline Mat2 &Mat2::ScaleX(const Real &sx)
			{ r11*=sx; r12*=sx; tx*=sx; return *this; }
inline Mat2 &Mat2::ScaleY(const Real &sy)
			{ r21*=sy; r22*=sy; ty*=sy; return *this; }
inline Mat2 &Mat2::ScaleAt(const Real &px, const Real &py,const Real &sx, const Real &sy)
			{ return Move(-px,-py).Scale(sx,sy).Move(px,py); }
inline Mat2 &Mat2::ScaleAt(const Vec2 &p,const Real &sx, const Real &sy)
			{ return Move(-p).Scale(sx,sy).Move(p); }
inline Mat2 &Mat2::ScaleAt(const Real &px, const Real &py,const Real &s)
			{ return Move(-px,-py).Scale(s).Move(px,py); }
inline Mat2 &Mat2::ScaleAt(const Vec2 &p,const Real &s)
			{ return Move(-p).Scale(s).Move(p); }
inline Mat2 &Mat2::Rotate(const Real &a)
			{	Mat2 help(*this);
				r11 =    r22 = cos(a);
				r12 = -( r21 = sin(a) );
				tx  = ty = 0.0;
				operator*=(help);
				return *this;
			}
inline Mat2 &Mat2::RotateDeg(const Real &a)
			{ return Rotate(a/Real(180/M_PI)); }
inline Mat2 &Mat2::RotateAt(const Real &px, const Real &py,const Real &a)
			{ return Move(-px,-py).Rotate(a).Move(px,py); }
inline Mat2 &Mat2::RotateDegAt(const Real &px, const Real &py,const Real &a)
			{ return Move(-px,-py).Rotate(a/Real(180/M_PI)).Move(px,py); }
inline Mat2 &Mat2::RotateAt(const Vec2 &p,const Real &a)
			{ return Move(-p).Rotate(a).Move(p); }
inline Mat2 &Mat2::RotateDegAt(const Vec2 &p,const Real &a)
			{ return Move(-p).Rotate(a/Real(180/M_PI)).Move(p); }
inline Mat2 &Mat2::ShearX(const Real& a)
			{ r11+=a*r21; r12+=a*r22; tx+=a*ty; return *this; }
inline Mat2 &Mat2::ShearY(const Real& b)
			{ r21+=b*r11; r22+=b*r12; ty+=b*tx; return *this; }
inline Mat2 &Mat2::Shear(const Real& a, const Real& b)
			{ return ShearX(a).ShearY(b); }

#endif    /* __Mat2_h */

