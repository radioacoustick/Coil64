/* multiloop_utils.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "multiloop_utils.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findMultiloopO_I(double N, double Di, double dt, _CoilResult *result){
    //The author of the source code of this function is George Overton.
    //The source code is used as an open with the consent of the author.
    //The code is from the author's book "Inside the METAL DETECTOR" Appendix A
    //https://www.facebook.com/Inside-the-Metal-Detector-222330481232886/
    //https://www.geotech1.com

    double c = sqrt(N) * dt;
    double a = (Di + c) / 2;
    double x = pow(c / 2 / a, 2);
    double s1 = 0.0004 * M_PI * a;
    double s2 = pow(N, 2);
    double s3 = s1 * s2;
    double s4 = 0.5 + x/12;
    double s5 = log(8/x);
    double s6 = (s4 * s5) - 0.85 + (0.2 * x);
    double ind = s3 * s6; //Inductance (microH)
    result->N = 2 * a; //Mean coil diameter (mm)
    result->sec = c; //coil thickness (mm)
    result->thd = 2e-3 * M_PI * a * N; //length of the wire (m)
    return ind;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long findMultiloopO_N(double I, double Di, double dt, _CoilResult *result){
    double tmpI = 0;
    unsigned long N_max = ULONG_MAX;
    unsigned long N_min = 0;
    unsigned long N = (N_max - N_min) / 2;
    if(findMultiloopO_I(N, Di, dt, result) <= I)
        return N_max;
    while ((N_max - N_min) > 1){
        N = (N_max + N_min) / 2;
        tmpI = findMultiloopO_I(N, Di, dt, result);
        if(tmpI > I){
            N_max = N;
        } else {
            N_min = N;
        }
    }
    return N;
}

double computeInductanceD(double R, double c) {
    R /= 1000.0;
    c /= 1000.0;
    // Inductance of a semicircle arc
    double x = pow(c / 2 / R, 2);
    double L_arc = MU0 * R / M_PI * ((0.5 + x / 12.0) * log(8.0 / x) - 0.85 + 0.2 * x); //complex formula
    //double L_arc = (MU0 * R / M_PI) * (log(8.0 * R / (c / 2.0)) - 2.0); // simple formula

    // Chord length = 2R
    double chordLength = 2.0 * R;

    // Chord inductance (straight conductor)
    double L_chord = (MU0 * chordLength / (2.0 * M_PI)) * (log(chordLength / c) - 0.75);

    // Mutual inductance between an arc and a chord
    double M = 1.003 * MU0 * R / M_PI;

    // Total inductance
    return 1e6 * (L_arc + L_chord + 2.0 * M);
}

int findMultiLoopD_N(double Ind, double Pin, double dt, _CoilResult *result, bool *isStop)
{
    int N = 0;
    for (N = 1; N < 10000; N++){
        if(*isStop){
            return -1;
        }
        double c = sqrt(N) * dt; //coil thickness (mm)
        double Ri = Pin / (M_PI + 2.0); //Inner radius of the semicircle arc
        double Rm = Ri + c / 2.0; //Mean radius of the semicircle arc
        double L = N * N * computeInductanceD(Rm, c);
        if (L > Ind){
            double Pm = Rm * (M_PI + 2.0); //Mean D-coil perimeter
            result->thd = 1e-3 * Pm * N; //length of the wire (m)
            result->sec = c; //coil thickness (mm)
            break;
        }
    }
    return N;
}


