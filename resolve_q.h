/* resolve_q.h - header text to Coil64 - Radio frequency inductor and choke calculator
Copyright (C) 2019 Kustarev V.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses
*/

#ifndef RESOLVE_Q_H
#define RESOLVE_Q_H

#include <QtMath>
#include "resolves.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// aluminium, copper, silver, tin -> (Rho, Chi, Alpha, Dencity) array of material parameters
/// from tables http://www.g3ynh.info/zdocs/comps/part_1.html
/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


unsigned long int solve_Qr(double I, double Df, double pm, double Dw, double fa, double N, double Cs, Material mt, _CoilResult *result);
unsigned long int solve_Qc(double I, double Df, double pm, double _w, double _t, double fa,  double N, double Cs, Material mt, _CoilResult *result);
double solve_Qpcb(long N, double _I, double _D, double _d, double _W, double _t, double _s,  double _f, int layout);

#endif // RESOLVE_Q_H
