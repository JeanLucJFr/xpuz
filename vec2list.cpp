#ifndef _global_h
#	include "global.h"
#endif

#ifndef _vec2list_h
#	include "vec2list.h"
#endif
#ifndef _mat2_h
#	include "mat2.h"
#endif

Vec2List::Vec2List( int len_in ) {
	alloc_len = len_in;
	len = 0;
	v = new Vec2[alloc_len];
}

Vec2List::Vec2List( const Vec2List &vl ) {
	v = 0;
	*this = vl;
}

Vec2List::Vec2List( const Vec2List &vl, const Mat2 &m ) {
	alloc_len = len = vl.Len();
	v = new Vec2[alloc_len];
	for (int i=0;i<len;i++) {
		v[i] = m*vl.v[i];
	}
}

void Vec2List::GetExtent( Vec2 *tl, Vec2 *br ) const {
	if (len) {
		Real	min_x(v[0].X());
		Real	max_x(v[0].X());
		Real	min_y(v[0].Y());
		Real	max_y(v[0].Y());

		for (int i=1;i<len;i++) {
			if (v[i].X()<min_x)		min_x=v[i].X();
			if (v[i].X()>max_x)		max_x=v[i].X();
			if (v[i].Y()<min_y)		min_y=v[i].Y();
			if (v[i].Y()>max_y)		max_y=v[i].Y();
		}
		*tl = Vec2(min_x,min_y);
		*br = Vec2(max_x,max_y);
	}
	else {
		*tl = Vec2Zero;
		*br = Vec2Zero;
	}
}

const Vec2List& Vec2List::SetAt(int id,const Vec2 &z) {
	if (id>=0&&id<len) {
		v[id] = z;
	}
	return *this;
}

const Vec2List& Vec2List::AddAt(int id,const Vec2 &z) {
int i;
	if (len>=alloc_len) {
		alloc_len += 4;
		Vec2	*new_v = new Vec2[alloc_len];
		for (i=0;i<len;i++)		new_v[i] = v[i];
		delete v;
		v = new_v;
	}
	for (i=len-1;i>=id;i--)	v[i+1]=v[i];		// shift to the end
	len++;
	v[id] = z;
	return *this;
}

const Vec2List& Vec2List::Del(int id) {
int i;

	for (i=id;i<len-1;i++)	v[i]=v[i+1];		// shift to front
	len--;
	return *this;
}

// deletes range of the polyline, include <from> but not include <to>
const Vec2List& Vec2List::DelRange( int from, int to, int *start ) {
int i;

	if (from<to) {
		for (i=to+1;i<len;i++)	v[from+i-to]=v[i];
		*start=from;
		len-=to-from;
		return *this;
	}
	else if (from>to) {
		// cut to end
		for (i=to+1;i<=from;i++)	v[i-to-1]=v[i];
		*start=from-to-1;
		len=from-to;
		return *this;
	}
	else {	/* from == to */
		*start=from;
		return *this;
	}
}

const Vec2List& Vec2List::operator=(const Vec2List &vl) {
	if (v)	delete v;
	alloc_len = len = vl.Len();
	v = new Vec2[alloc_len];
	for (int i=0;i<len;i++) {
		v[i] = vl.v[i];
	}
	return *this;
}

const Vec2List& Vec2List::operator|=(const Vec2& z)
{
	if (len>=alloc_len) {
		alloc_len += 4;
		Vec2	*new_v = new Vec2[alloc_len];
		for (int i=0;i<len;i++)		new_v[i] = v[i];
		delete v;
		v = new_v;
	}
	v[len++] = z;
	return *this;
}

const Vec2List& Vec2List::operator+=(const Vec2& z2)
{
	for (int i=0;i<len;i++)		v[i]+=z2;
	return *this;
}

const Vec2List& Vec2List::operator-=(const Vec2& z2)
{
	for (int i=0;i<len;i++)		v[i]-=z2;
	return *this;
}

const Vec2List& Vec2List::operator*=(const Real& val)
{
	for (int i=0;i<len;i++)		v[i]*=val;
	return *this;
}

const Vec2List& Vec2List::operator/=(const Real& val)
{
	for (int i=0;i<len;i++)		v[i]/=val;
	return *this;
}


const Vec2List& Vec2List::operator*=(const Mat2& m)
{
	for (int i=0;i<len;i++)		v[i]=m*v[i];
	return *this;
}

int operator==(const Vec2List& z1, const Vec2List& z2)
{
int erg;
	if (z1.Len()!=z2.Len())		return 0;
	for (int i=0;i<z1.Len();i++) {
		erg = (z1.v[i]==z2.v[i]);
		if (!erg)			return erg;
	}
	return 1;
}

void Vec2List::TurnAngleRad( const Real &angle )
{
	for (int i=0;i<len;i++) {
		v[i]=v[i].TurnAngleRad(angle);
	}
}
void Vec2List::TurnRight()
{
	for (int i=0;i<len;i++) {
		v[i]=v[i].TurnRight();
	}
}
void Vec2List::TurnLeft()
{
	for (int i=0;i<len;i++) {
		v[i]=v[i].TurnLeft();
	}
}
