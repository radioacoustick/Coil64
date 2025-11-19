/* multiloop_utils.h - source text to Coil64 - Radio frequency inductor and choke calculator
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
#ifndef MULTILOOPUTILS_H
#define MULTILOOPUTILS_H

#include <QtMath>
#include <QVector3D>
#include "resolves.h"

double findMultiloopO_I(double N, double Di, double dt, _CoilResult *result);
unsigned long findMultiloopO_N(double I, double Di, double dt, _CoilResult *result);
int findMultiLoopD_N(double Ind, double Pin, double dt, _CoilResult *result, bool *isStop);

#endif // MULTILOOPUTILS_H
