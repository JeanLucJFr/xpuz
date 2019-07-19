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
#ifndef _real_h
#	include "real.h"
#endif

#ifndef RealZero
const Real RealZero = 0.0;
#endif


FunTab::FunTab( double (*fkt) (double), double from_in, double to_in, int step_in ) {

	from		= from_in;
	to   		= to_in;
	step		= step_in;
	interval = (to-from)/Real((double)step);

	val = new Real [step+1];

	for (int i=0;i<=step;i++) {
		val[i] = fkt( from+interval*(double)i );
	}
}


FunTab::~FunTab() {
	if (val)		delete [] val;
}

const Real &FunTab::GetVal( const Real &in ) const  {
	double	m = fmod( in, 2.0*M_PI );
	while( m<0 )	m+= M_PI*2.0;
	int ind = (int)((m-from)/interval+0.5);
	return val[ind];
}

Real FunTab::GetRezVal( const Real &in ) const {
int	hi = step;
int	lo = 0;

int	mid = (hi+lo)/2;

	while( mid!=lo ) {
		if (val[mid]>in)	hi=mid;
		else					lo=mid;
		mid = (hi+lo)/2;
	}
	return (double)mid*interval+from;
}
