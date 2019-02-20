/* bessel.h - header text to Coil64 - Radio frequency inductor and choke calculator
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

#ifndef BESSEL_H
#define BESSEL_H

double bessi0(double x);
double bessi1(double x);
double bessj0(double x);
double bessj1(double x);
double bessy0(double x);
double bessy1(double x);
double bessk0(double x);
double bessk1( double x );
double bessi(int n, double x);
double bessjn(int n, double x);
double bessyn(int n, double x);

#endif // BESSEL_H
