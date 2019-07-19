#ifndef _real_h
#define _real_h

// copied from math.h
// #ifndef M_PI
// #	define M_PI    3.14159265358979323846
// #endif

#if (!REAL_IS_CLASS)
#	if (REAL_IS_FLOAT)
		typedef float Real;
#	else
		typedef double Real;
#	endif
#	define	_RealZero		0.0
#else

class Real {
	public:
		Real(const Real &r)           { d=r.d; }
		Real(int l_val)              { d = (double)l_val; }
		Real(double d_val)            { d = d_val; }
		Real()                        { }
		operator double() const       { return (double)d; }
//		operator int() const          { return (int)d; }

	const Real& operator=(const Real &z);

	inline const Real& operator+=(const Real &z);
	inline const Real& operator+=(double d);
	inline Real Real::operator+(const Real&) const;
	friend inline Real operator+(double, const Real&);
	friend inline Real operator+(const Real&, double);

	inline const Real& operator-=(const Real &z);
	inline const Real& operator-=(double d);
	inline Real Real::operator-(const Real&) const;
	friend inline Real operator-(double, const Real&);
	friend inline Real operator-(const Real&, double);

	inline const Real& operator*=(const Real &z);
	inline const Real& operator*=(double d);
	inline Real Real::operator*(const Real&) const;
	friend inline Real operator*(double, const Real&);
	friend inline Real operator*(const Real&, double);

	inline const Real& operator/=(const Real &z);
	inline const Real& operator/=(double d);
	inline Real Real::operator/(const Real&) const;
	friend inline Real operator/(double, const Real&);
	friend inline Real operator/(const Real&, double);

	friend inline int operator==(const Real&, const Real&);
	friend inline int operator!=(const Real&, const Real&);
	friend inline int operator<=(const Real&, const Real&);
	friend inline int operator>=(const Real&, const Real&);
	friend inline int operator< (const Real&, const Real&);
	friend inline int operator> (const Real&, const Real&);

	friend inline int operator==(const Real&, double d);
	friend inline int operator!=(const Real&, double d);
	friend inline int operator<=(const Real&, double d);
	friend inline int operator>=(const Real&, double d);
	friend inline int operator< (const Real&, double d);
	friend inline int operator> (const Real&, double d);

	inline int  operator!() const	{ return (d)?0:1; }
	inline Real operator+() const  { return *this; }
	inline Real operator-() const  { return Real(-d); }

	private:
//		Real(	long )		{}
//		Real( int )		{}

#if (REAL_IS_FLOAT)
		float		d;
#else
		double	d;
#endif
};

//--------------------------------------------------------------------------

inline const Real& Real::operator=(const Real &z)
{
			d = z.d;
			return *this;
}

inline const Real& Real::operator+=(const Real &z)
{
			d += z.d;
			return *this;
}
inline const Real& Real::operator-=(const Real &z)
{
			d -= z.d;
			return *this;
}
inline const Real& Real::operator*=(const Real &z)
{
			d *= z.d;
			return *this;
}
inline const Real& Real::operator/=(const Real &z)
{
			d /= z.d;
			return *this;
}

//--------------------------------------------------------------------------

inline int operator==(const Real& z1, const Real& z2)
{       return z1.d == z2.d;     }
inline int operator!=(const Real& z1, const Real& z2)
{       return z1.d != z2.d;     }
inline int operator<(const Real& z1, const Real& z2)
{       return z1.d < z2.d;      }
inline int operator>(const Real& z1, const Real& z2)
{       return z1.d > z2.d;      }
inline int operator<=(const Real& z1, const Real& z2)
{       return z1.d <= z2.d;     }
inline int operator>=(const Real& z1, const Real& z2)
{       return z1.d >= z2.d;     }

inline int operator==(const Real& z1, double d)
{       return z1.d == d;     }
inline int operator!=(const Real& z1, double d)
{       return z1.d != d;     }
inline int operator<(const Real& z1, double d)
{       return z1.d < d;      }
inline int operator>(const Real& z1, double d)
{       return z1.d > d;      }
inline int operator<=(const Real& z1, double d)
{       return z1.d <= d;     }
inline int operator>=(const Real& z1, double d)
{       return z1.d >= d;     }

//--------------------------------------------------------------------------
inline Real Real::operator+( const Real &z ) const	{ Real erg(*this); erg+=z; return erg; }
inline const Real& Real::operator+=(double d)      { return *this+=Real(d); }
inline Real operator+( double d, const Real &z2 )  { return Real(d)+z2; }
inline Real operator+( const Real &z1, double d )  { return z1+Real(d); }

inline Real Real::operator-( const Real &z ) const	{ Real erg(*this); erg-=z; return erg; }
inline const Real& Real::operator-=(double d)      { return *this-=Real(d); }
inline Real operator-( double d, const Real &z2 )  { return Real(d)-z2; }
inline Real operator-( const Real &z1, double d )  { return z1-Real(d); }

inline Real Real::operator*( const Real &z ) const	{ Real erg(*this); erg*=z; return erg; }
inline const Real& Real::operator*=(double d)      { return *this*=Real(d); }
inline Real operator*( double d, const Real &z2 )  { return Real(d)*z2; }
inline Real operator*( const Real &z1, double d )  { return z1*Real(d); }

inline Real Real::operator/( const Real &z ) const	{ Real erg(*this); erg/=z; return erg; }
inline const Real& Real::operator/=(double d)      { return *this/=Real(d); }
inline Real operator/( double d, const Real &z2 )  { return Real(d)/z2; }
inline Real operator/( const Real &z1, double d )  { return z1/Real(d); }
//--------------------------------------------------------------------------



#endif

#ifndef RealZero
	extern const Real RealZero;	// Null als Konstante der Klasse
#endif


class FunTab {
	public:
		FunTab( double (*fkt)(double), double from=0.0, double to=360.0, int step=360 );
		~FunTab();

		const Real &GetVal( const Real &in ) const;
		Real GetRezVal( const Real &in ) const;

	private:
		Real *val;

		Real 		from;
		Real		to;
		int		step;
		Real		interval;
};

extern "C" {
double floor( double x );			// aus math.h
}
inline int rtoi(const Real &r) 	{ return (int)floor((double)r+0.5); }

#endif
