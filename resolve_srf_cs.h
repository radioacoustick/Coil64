/* resolve_srf_cs.h - header text to Coil64 - Radio frequency inductor and choke calculator
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

#ifndef RESOLVE_SRF_CS_H
#define RESOLVE_SRF_CS_H

#include <QtMath>
#include "bessel.h"

double findSRF(double L, double D, double lw);
double find_Cs(double p, double D, double l);

#endif // RESOLVE_SRF_CS_H
