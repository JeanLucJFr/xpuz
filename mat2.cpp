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
#ifndef _mat2_h
#	include "mat2.h"
#endif


const Mat2 &Mat2::operator*=(const Mat2 &m)
{
   Real h11=r11;
   Real h12=r12;
   Real h21=r21;
   Real h22=r22;
   r11 = h11*m.r11 + h12*m.r21;
   r21 = h21*m.r11 + h22*m.r21;
   r12 = h11*m.r12 + h12*m.r22;
   r22 = h21*m.r12 + h22*m.r22;
   tx += h11*m.tx  + h12*m.ty;
   ty += h21*m.tx  + h22*m.ty;
   return *this;
}

void Mat2::Invert(Mat2 *m) const
{
   Real detA = r11*r22-r12*r21;

   m->r11 =  r22/detA;
   m->r12 = -r12/detA;
   m->r21 = -r21/detA;
   m->r22 =  r11/detA;
   m->tx  = -m->r11*tx -m->r12*ty;
   m->ty  = -m->r21*tx -m->r22*ty;
}

void Mat2::Split(Real *sh_p, Real *sx_p, Real *sy_p, Real *angle_p, Real *tx_p, Real *ty_p) const
{
   Real cosa;		// Rotation

	if (r21) {
	   Real r1121 = r11/r21;
			cosa = r1121 / sqrt(1+r1121*r1121);
			*angle_p = acos(cosa);
			*sx_p = r11/cosa;

			*sy_p = (r11*r22/r21-r12)/(sin(*angle_p/180*M_PI)+r11*cosa/r21);
			*sh_p  = (r22-cosa**sy_p)/r21;
	}
	else {
		if (r11) {
			*sx_p		= r11;
			*angle_p = 0.0;
			cosa	= 1.0;
			*sh_p   = r12/r11;
			*sy_p		= r22;
		}
		else {
			*sx_p = 0;
			*sh_p  = 0;	// sowieso egal

			Real r2212 = r22/r12;
			cosa = -r2212 / sqrt(1+r2212*r2212);
			*angle_p = acos(cosa);
			*sy_p = r22/cosa;
		}
	}

	// wenn beide Skalierungen negativ: Punktspiegelung
	if (*sx_p<0 && *sy_p<0) {
		*sx_p = *sx_p * -1;
		*sy_p = *sy_p * -1;
		if (*angle_p>0)	*angle_p-=180.0;
		else				*angle_p+=180.0;
	}
	if (fabs(*sx_p-1)<EPS)	*sx_p=1.0;
	if (fabs(*sy_p-1)<EPS)	*sy_p=1.0;
	if (fabs(*sx_p-*sy_p)<EPS)	*sy_p=*sx_p;
	if (fabs(*sh_p)<EPS)		*sh_p=0.0;
	if (fabs(*angle_p)<EPS)	*angle_p=0.0;
	*tx_p = (fabs(tx)>EPS)?tx:0;
	*ty_p = (fabs(ty)>EPS)?ty:0;
}


// void Mat2::Print() const
// {
//    cout <<  "Matrix  r11, r12, tx  " <<r11<<"  "<<r12<<"  "<<tx<< endl;
//    cout <<  "        r21, r22, ty  " <<r21<<"  "<<r22<<"  "<<ty<< endl;
// }
