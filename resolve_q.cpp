/* resolve_q.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "resolve_q.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// LINEAR INTERPOLATION FUNCTION FROM ARNOLD'S TABLES///
/// https://ieeexplore.ieee.org/document/5240803
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double LinearInterpolation_Table7_Phi(double z){
    double za[] = {0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9,
                   3.0, 3.2, 3.4, 3.6, 3.8, 4.0, 4.2, 4.4, 4.6, 4.8, 5.0, 5.2, 5.4, 5.6, 5.8, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0};
    double Fi[] = {0.001, 0.003, 0.005, 0.008, 0.013, 0.02, 0.029, 0.041, 0.055, 0.072, 0.092, 0.114, 0.139, 0.167, 0.196, 0.226, 0.257,
                   0.288, 0.319, 0.349, 0.378, 0.406, 0.432, 0.457, 0.48, 0.501, 0.539, 0.571, 0.599, 0.622, 0.643, 0.661, 0.677, 0.691,
                   0.704, 0.716, 0.727, 0.737, 0.746, 0.755, 0.763, 0.782, 0.797, 0.811, 0.823, 0.833, 0.843, 0.851, 0.858};
    unsigned int zi;
    double result = 0;
    if (z > 10) {
        result = 1 - (sqrt(2) / z);
    } else {
        zi = 0;
        while ((z >= za[zi]) && (zi < sizeof(za)/sizeof(za[0]) - 1)) {
            zi++;
            if (zi > 0) {
                result = ((Fi[zi] - Fi[zi - 1]) / (za[zi] - za[zi - 1])) * (z - za[zi - 1]) + Fi[zi - 1];
            }
        }
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double LinearInterpolation_Table7_Chi(double z){
    double za[] = {0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9,
                   3.0, 3.2, 3.4, 3.6, 3.8, 4.0, 4.2, 4.4, 4.6, 4.8, 5.0, 5.2, 5.4, 5.6, 5.8, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0};
    double Ci[] = {0.001, 0.002, 0.004, 0.006, 0.010, 0.015, 0.022, 0.03, 0.041, 0.053, 0.067, 0.083, 0.101, 0.12, 0.14, 0.16, 0.18, 0.199,
                   0.218, 0.235, 0.251, 0.265, 0.278, 0.289, 0.299, 0.307, 0.32, 0.33, 0.337, 0.343, 0.348, 0.353, 0.357, 0.361, 0.365, 0.37,
                   0.374, 0.378, 0.382, 0.386, 0.389, 0.397, 0.404, 0.41, 0.416, 0.42, 0.425, 0.428, 0.432};
    unsigned int zi;
    double result = 0;
    if (z > 10) {
        result = 0.5 - (1 / (z * sqrt(2)));
    } else {
        zi = 0;
        while ((z >= za[zi]) && (zi < sizeof(za)/sizeof(za[0]) - 1)) {
            zi++;
            if (zi > 0) {
                result = ((Ci[zi] - Ci[zi - 1]) / (za[zi] - za[zi - 1])) * (z - za[zi - 1]) + Ci[zi - 1];
            }
        }
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double LinearInterpolation_Table8_v(double N){
    double Na[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 25, 30, 35, 40, 45, 50, 67, 100, 125, 167, 250, 500};
    double v[] ={-1, -0.25, 0.21, 0.52, 0.76, 0.94, 1.09, 1.22, 1.33, 1.5, 1.63, 1.73, 1.82, 1.91, 2.06, 2.17, 2.26, 2.33, 2.39, 2.44,
                 2.56, 2.71, 2.77, 2.85, 2.94, 3.05};
    unsigned int ni;
    double result = 0;
    if (N > 500) {
        result = 3.29;
    } else {
        ni = 0;
        while ((N >= Na[ni]) && (ni < sizeof(Na)/sizeof(Na[0]) - 1)) {
            ni++;
            if (ni > 0) {
                result = ((v[ni] - v[ni - 1]) / (Na[ni] - Na[ni - 1])) * (N - Na[ni - 1]) + v[ni - 1];
            }
        }
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double LinearInterpolation_Table8_w(double N){
    double Na[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 25, 30, 35, 40, 45, 50, 67, 100, 125, 167, 250, 500};
    double w[] = {1, 2.4, 3.5, 4.3, 4.9, 5.4, 5.9, 6.3, 6.6, 7.0, 7.4, 7.8, 8.0, 8.2, 8.6, 8.9, 9.1, 9.3, 9.4, 9.5, 9.7, 10.0, 10.2, 10.3, 10.4, 10.6};
    unsigned int ni;
    double result = 0;
    if (N > 500) {
        result = 10.8;
    } else {
        ni = 0;
        while ((N >= Na[ni]) && (ni < sizeof(Na)/sizeof(Na[0]) - 1)) {
            ni++;
            if (ni > 0) {
                result = ((w[ni] - w[ni - 1]) / (Na[ni] - Na[ni - 1])) * (N - Na[ni - 1]) + w[ni - 1];
            }
        }
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double LinearInterpolation_Table3(double N){
    double Na[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 25, 34, 50, 100};
    double p[] = {2.29, 1.79, 1.48, 1.28, 1.12, 1.01, 0.91, 0.84, 0.78, 0.68, 0.60, 0.54, 0.50, 0.46, 0.39, 0.31, 0.22, 0.12};
    unsigned int ni;
    double result = 0;
    if (N > 100) {
        result = 1e-6;
    } else {
        ni = 0;
        while ((N >= Na[ni]) && (ni < sizeof(Na)/sizeof(Na[0]) - 1)) {
            ni++;
            if (ni > 0) {
                result = ((p[ni] - p[ni - 1]) / (Na[ni] - Na[ni - 1])) * (N - Na[ni - 1]) + p[ni - 1];
            }
        }
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double LinearInterpolation_Table5(double ro){
    double roa[] = {0.1, 0.2, 0.3, 0.4, 0.6, 0.8, 1, 1.2, 1.4, 1.6, 1.8, 2, 2.2, 2.4, 2.6, 2.8, 3, 3.2, 3.4, 3.6, 3.8, 4, 4.2, 4.4, 4.6, 4.8, 5.0,
                    5.6, 6.25, 7.14, 7.7, 8.33, 9.1, 10, 11.1, 12.5, 14.28, 16.66, 20, 25, 33.33, 50, 100};
    double g[] = {0.06, 0.18, 0.32, 0.49, 0.85, 1.23, 1.61, 1.99, 2.36, 2.71, 3.04, 3.35, 3.63, 3.90, 4.16, 4.39, 4.61, 4.82, 5.01, 5.19,
                  5.36, 5.51, 5.66, 5.80, 5.93, 6.06, 6.18, 6.46, 6.76, 7.09, 7.26, 7.43, 7.61, 7.79, 7.98, 8.17, 8.37, 8.57, 8.78, 9.00, 9.21, 9.43, 9.65};
    unsigned int ni;
    double result = 0;
    if (ro > 100) {
        result = 9.87;
    } else {
        ni = 0;
        while ((ro >= roa[ni]) && (ni < sizeof(roa)/sizeof(roa[0]) - 1)) {
            ni++;
            if (ni > 0) {
                result = ((g[ni] - g[ni - 1]) / (roa[ni] - roa[ni - 1])) * (ro - roa[ni - 1]) + g[ni - 1];
            }
        }
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double LinearInterpolation_Table4_a(double r){
    double ra[] = {0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0};
    double a[] = {3.29, 3.24, 3.13, 2.99, 2.83, 2.67, 2.51, 2.36, 2.21, 2.07, 1.95, 1.83, 1.72, 1.63, 1.54, 1.46, 1.28, 1.14, 1.02, 0.93,
                  0.85, 0.78, 0.72, 0.67, 0.63, 0.59};
    unsigned int ri;
    double result = 0;
    if (r > 8.0) {
        result = 19 / (4 * r);
    } else {
        ri = 0;
        while ((r >= ra[ri]) && (ri < sizeof(ra)/sizeof(ra[0]) - 1)) {
            ri++;
            if (ri > 0) {
                result = ((a[ri] - a[ri - 1]) / (ra[ri] - ra[ri - 1])) * (r - ra[ri - 1]) + a[ri - 1];
            }
        }
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double LinearInterpolation_Table4_b(double r){
    double ra[] = {0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0};
    double b[] = {0, -0.03, -0.04, -0.04, -0.03, -0.02, -0.01, 0, 0.01, 0.02, 0.03, 0.04, 0.04, 0.05, 0.06, 0.06, 0.08, 0.09, 0.1, 0.11, 0.12,
                  0.12, 0.13, 0.13, 0.13, 0.14};
    unsigned int ri;
    double result = 0;
    if (r > 8.0) {
        result = 4 / 19 - 3 / (5 * r);
    } else {
        ri = 0;
        while ((r >= ra[ri]) && (ri < sizeof(ra)/sizeof(ra[0]) - 1)) {
            ri++;
            if (ri > 0) {
                result = ((b[ri] - b[ri - 1]) / (ra[ri] - ra[ri - 1])) * (r - ra[ri - 1]) + b[ri - 1];
            }
        }
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///CALCULATING PROXIMITY EFFECT FACTOR BY ARNOLD RESEARCH
///https://ieeexplore.ieee.org/document/5240803
/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double lookup_Psi(double Df, double dw, double pt, double N, double fm, Material mt){
    //el->winding length [mm], Df->coilformer diameter [mm], pt->winding pitch [mm], Dw->wire diameter [mm], fm -> frequency [kHz]
    double d, P, rc, f, si, zeta, eta, ro, z, Phi, Chi, v, w, alpha, betta, gamma, pm, a, b, u1, u2, g;

    d = dw / 10;
    P = pt / 10;
    rc = Df / 20;
    f = fm * 1e3;
    si = 1e-11 / mtrl[mt][Rho];
    eta = d / P;
    zeta = P / rc;
    ro = (N - 1) * zeta;
    z = M_PI * d * sqrt(2 * f * si);
    Phi = LinearInterpolation_Table7_Phi(z);
    Chi = LinearInterpolation_Table7_Chi(z);
    v = LinearInterpolation_Table8_v(N);
    w = LinearInterpolation_Table8_w(N);
    alpha = (1 + 0.209 * w * pow(Phi, 2) * pow(eta, 4)) / (1 + 0.084 * w * pow(Phi, 2) * pow(eta, 4));
    betta = 1 / (pow((1 - 0.278 * v * Phi * eta * eta), 1.8));
    gamma = 1 / (pow((1 + 0.385 * v * Phi * eta * eta), 1.3));
    pm = LinearInterpolation_Table3(N);
    a = LinearInterpolation_Table4_a(ro);
    b = LinearInterpolation_Table4_b(ro);
    u1 = a - (1 / (1 / pm + ro * b));
    g = LinearInterpolation_Table5(ro);
    u2 = g / (1 + (0.42 + (0.3 / ro)) * sqrt(zeta) + (1.35 + (2.8 / sqrt(ro))) * pow(zeta, 1.5));
    double result = alpha + eta * eta * Chi * (betta * u1 + gamma * u2);
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CALCULATING SKIN-EFFECT FACTOR FOR ROUND WIRE BY D.KNIGHT RESEARCH
///http://www.g3ynh.info/zdocs/comps/Zint.pdf
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double get_Xir(Material mt, double fm, double dw){
    //AC resistance factor. TED-MLD formula (skin effect factor)
    const double mu0 = 12.56637e-7; // absolutly permeability
    double y, z, f, r, delta_i, delta_i_prim;

    f = fm * 1e3;
    r = dw / 2000;
    delta_i = sqrt(mtrl[mt][Rho] / (f * M_PI * mu0 * (1 + mtrl[mt][Chi])));
    delta_i_prim = delta_i * (1 - exp(-r / delta_i));
    z = 0.62006 * r / delta_i;
    y = 0.189774 / pow((1 + 0.272481 * pow((pow(z, 1.82938) - pow(z, -0.99457)), 2)), 1.0941);
    double result = r * r / (2 * r * delta_i_prim - delta_i_prim * delta_i_prim) * (1 - y);
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CALCULATING SKIN-EFFECT FACTOR FOR RECTANGULAR CONDUCTOR BY ALAN PAYNE RESEARCH
/// http://g3rbj.co.uk/wp-content/uploads/2017/06/The-ac-Resistance-of-Rectangular-Conductors-Payne-Issue-3.pdf
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double get_Xic(Material mt, double f, double w, double t){
    //AC resistance factor formula 4.8.1
    const double mu0 = 12.56637e-7; // absolutly permeability
    double delta = sqrt(mtrl[mt][Rho] / (f * M_PI * mu0 * (1 + mtrl[mt][Chi])));
    double p = sqrt(w * t)/(1.26 * delta);
    double Ff = 1 - exp(-0.026 * p);
    double x = ((2 * delta / t) * (1 + t / w) + 8 * pow(delta / t, 3)/(w / t)) / (pow(w / t, 0.33)*exp(-3.5*(t / delta)) + 1);
    double Kc = 1 + Ff * (1.2 / exp(2.1 * (t / w)) + 1.2 / exp(2.1 * (w / t)));
    double result  = Kc / (1 - exp(-x));
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PUBLIC FUNCTIONS REALIZATION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Q-FACTOR OF THE ONE-LAYER COIL WITH ROUND WIRE (WITH PROXIMITY EFFECT)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long int solve_Qr(double I, double Df, double pm, double dw, double fa, double N, double Cs, Material mt){
    //I->inductance µH
    //l->winding length mm, Df->coilwinding diameter mm, pm->winding pitch mm, dw->wire diameter mm
    //fm->frequency MHz, N->number of turns, mt->material of wire
    double Induct, fm, f, D, r, p, WireLength, Rdc, Rac0, Rac, Xi, Psi, kQ, R_ind, Rl, Rc;

    Induct = I * 1.0e-6;
    fm = fa * 1e3;
    f = fm * 1e3;
    D = Df / 1000;
    r = dw / 2000;
    p = pm / 1000;
    WireLength = M_PI * N * sqrt(D * D + p * p / 4);
    Rdc = mtrl[mt][Rho] * WireLength / (M_PI * r * r);
    Xi = get_Xir(mt, fm, dw);
    Psi = lookup_Psi(Df, dw, pm, N, fm, mt);
    Rac0 = Rdc * (1 + ((Xi - 1) * Psi * (N - 1 + 1 / Psi)) / N);
    kQ = pow((1 - (fm * I * Cs) / 2.53e7), 2);
    Rac = Rac0 / kQ;
    Rl = 2 * M_PI * f * Induct;
    Rc = 1 / (2 * M_PI * f * Cs * 1e-12);
    R_ind = 1 / (1 / Rl + 1 / Rc);
    return round(R_ind / Rac);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Q-FACTOR OF THE ONE-LAYER COIL WITH RECTANGULAR WIRE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long int solve_Qc(double I, double Df, double pm, double _w, double _t, double fa,  double N, double Cs, Material mt){
    //I->inductance µH
    //l->winding length mm, Df->coilwinding diameter mm, pm->winding pitch mm, _w->wire width mm, _t->wire thickness mm
    //fm->frequency MHz, N->number of turns, mt->material of wire
    double Induct, fm, f, D, w, t, p, WireLength, Rdc, Rac0, Rac, Xi, kQ, R_ind, Rl, Rc, Psi;

    Induct = I * 1e-6;
    fm = fa * 1e3;
    f = fm * 1e3;
    D = Df / 1000;
    w = _w / 1000;
    t = _t / 1000;
    p = pm / 1000;
    WireLength = M_PI * N * sqrt(D * D + p * p / 4);
    Rdc = mtrl[mt][Rho] * WireLength / (w * t);
    Xi = get_Xic(mt, f, w, t);
    Psi = lookup_Psi(Df, _w, pm, N, fm, mt);
    Rac0 = Rdc * (1 + ((Xi - 1) * Psi * (N - 1 + 1 / Psi)) / N);
    kQ = pow((1 - (fm * I * Cs) / 2.53e7), 2);
    Rac = Rac0 / kQ;
    Rl = 2 * M_PI * f * Induct;
    Rc = 1 / (2 * M_PI * f * Cs * 1e-12);
    R_ind = 1 / (1 / Rl + 1 / Rc);
    return round(R_ind / Rac);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Q-FACTOR OF THE PCB SPIRAL COIL
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double solve_Qpcb(long N, double _I, double _D, double _d, double _W, double _t, double _s,  double _f, int layout){
    //_I->inductance µH, N->number of turns, _D->outer winding diameter mm, _d->inner winding diameter mm
    // _f->frequency MHz, _t-> thickness of the trace, _W->width of the trace, _s->winding pitch
    const double mu0 = 12.56637e-7;
    double f = _f * 1e6;
    double I = _I *  1e-6;
    double D = _D *  1e-3;
    double d = _d *  1e-3;
    double W = _W *  1e-3;
    double t = _t *  1e-3;
    double s = _s *  1e-3;
    double StripLength = 0;
    switch (layout) {
    case 0:{
        StripLength +=  d / 2;
        for (int i = 0; i < N; i++){
            StripLength += 2 * (2 * d + (4 * i + 1) * s);
        }
        StripLength -= D / 2;
        break;
    }
    case 1:{
        StripLength = find_actual_spiral_length(N, d, s);
        break;
    }
    default:
        break;
    }
    double Rdc = mtrl[1][Rho] * StripLength / (W * t);
    double delta = sqrt(mtrl[1][Rho] / (f * M_PI * mu0 * (1 + mtrl[1][Chi])));
    double Xi  = t / (delta*(1 - exp(-t/delta)));
    double Psi = (Xi / 3) * pow(3 * W / (2 * s), 4);
    double Rac = Rdc * (Xi + Psi);
    double Xl = 2 * M_PI * f * I;
    double q = Xl / Rac;
    return q;
}
