/* resolve_srf_cs.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "resolve_srf_cs.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// SELF-CAPACITANCE & SELF-RESONANCE OF THE ONE-LAYER COIL
/// http://www.g3ynh.info/zdocs/magnetics/appendix/self_res/self-res.pdf
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const double e0 = 8.854187818;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double R0tan(double x){
    // Calculates R0 Tan(psi) for the sheet helix. D W Knight. Version 1.00, 2016-04-04
    double I0 = bessi0(x);
    double K0 = bessk0(x);
    double I1 = bessi1(x);
    double K1 = bessk1(x);
    double Result = 59.9584916 * sqrt(2 * I0 * K0 * I1 * K1);
    return Result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double OLLENDF(double x){
    // Calculates Ollendorff's function. D W Knight. Version 1.00, 2016-03-16
    double I0 = bessi0(x);
    double K0 = bessk0(x);
    double I1 = bessi1(x);
    double K1 = bessk1(x);
    double Result = sqrt(I0 * K0 / (I1 * K1));
    return Result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double W82W(double x){
    // calculates Nagaoka's coeff. using Wheeler's 1982 eqn (7) as modified by Bob Weaver88.
    // Max error is +/- 21ppM. x = Diam/length. D W Knight, v1.00, June 2012.
    double Result;
    if (x == 0) {
        Result = 1;
    } else {
        double zk = 2 / (M_PI * x);
        double K0 = 1 / (log(8 / M_PI) - 0.5);
        double k2 = 24 / (3 * M_PI * M_PI - 16);
        double w = -0.47 / pow((0.755 + x), 1.44);
        double p = K0 + 3.437 / x + k2 / (x * x) + w;
        Result = zk * (log(1 + 1 / zk) + 1 / p);
    }
    return Result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double VFnom(double lod, double ei, double ex){
    // Calculates nominal helical velocity factor for a free coil at its first SRF.
    // D W Knight, v1.00, 2016-04-14
    // Calls functions W82W(), Ollendf(), R0tan()
    double diff = 1;
    double y = 1;

    double x = M_PI / (2 * lod);
    x = x + 0.117 * exp(-22 * exp(-0.75 * lod));
    double kL = W82W(1 / lod);
    double erad = (ex / 2) * (1 + kL + (ei / ex) * (1 - kL));
    double vf0 = 1 / sqrt(erad);
    double vfh = vf0 * OLLENDF(x);
    double Cff = erad * 2 * e0 / (1 + log(1 + lod));
    double arg = 1.015 + lod * lod;
    double Caf = e0 * (ei + ex) / log(arg + sqrt(arg * arg - 1));
    double Rotn = vf0 * R0tan(x);
    double b = 1000000 * lod / (299.792458 * M_PI * (Cff + Caf) * Rotn);
    double a = M_PI / (2 * vfh);
    // solve for VFnom
    double z = vfh;
    int n = 0;
    while ((fabs(diff) > 1E-9) || (n < 255)) {
        y = (1 / a) * atan(b / z);
        diff = z - y;
        double der = -(b / a) / (z * z + b * b);
        double deltaz = diff / (der - 1);
        z = z + deltaz;
        n++;
    }
    return y;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findSRF(double L, double D, double lw){
    double Vhx= VFnom(L / D, 1, 1);
    double Result =0.5 * Vhx * 299.792458 / (2 * lw);
    return Result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double find_Cs(double p, double D, double l){
    p = p / 1000;
    D = D / 1000;
    l = l / 1000;
    double sinpsi = p / (M_PI * D);
    double cospsi = sqrt(1 - (sinpsi * sinpsi));
    double dl = D / l;
    double kc = (0.717439 * dl) + (0.933048 * pow(dl, 1.5)) + 0.106 * pow(dl, 2);
    double result = (4 * e0 / M_PI) * l * (1 + kc) / (cospsi * cospsi);
    return result;
}
