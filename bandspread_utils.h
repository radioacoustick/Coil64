/* bandspread_utils.h - header text to Coil64 - Radio frequency inductor and choke calculator
Copyright (C) 2019-2024 Kustarev V.

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
#ifndef BANDSPREAD_UTILS_H
#define BANDSPREAD_UTILS_H

#include <QtMath>
#include <QVector>
#include "resolves.h"

double findPadderCapacitance(double Ct,double Cv_low,double Cv_high,double Cstray,double cap_ratio);
double findTrimmerCapacitance(double Cp,double Cv_low,double Cv_high,double Cstray,double cap_ratio);
void calcInductanceRange(double f_low, double f_high, double Cv_low, double Cv_high, double Cstray, QVector<double> *range, int accuracy);
void calcCapacitors(double f_low, double f_high, double Cv_low, double Cv_high, double Cstray, double ind, QVector<double> *cap, int accuracy);
void calcFrequencyRange(double Ct, double Cp, double Cv_low, double Cv_high, double Cstray, double ind, QVector<double> *freq, int accuracy);
int trackingLocalOscillator(double f_low, double f_high, double f_i, double Cv_low, double Cv_high, double Cstray, double *ind,
                             QVector<double> *result, QVector<double> *conFreq, int accuracy, bool isHighSideInjection, bool isAutomaticLe, double *maxDelta);

#endif // BANDSPREAD_UTILS_H
