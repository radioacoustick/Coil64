/* resolves.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "resolves.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///ROUTINE FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma pack(push,1)
struct _Elliptic{
    double Fk;
    double Ek;
};
#pragma pack(pop)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double odCalc(double id){
    //Calculating the outer diameter (od) of the wire with insulation from the internal diameter (id) without insulation
    const double  M = 0.96344;
    const double  b = -0.19861;
    double od1 = exp(M * log(id) + b);
    double od2 = 1.09 * id;
    double od = qMax(od1, od2);
    return od;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double find_spiral_length(int N, double Din, double k) {
    //Find the spiral length
    double Ltotal = 0;
    double length_i, Dik;
    for (int i = 1; i < N + 1; i++) {
        Dik = Din + 2 * (i - 1) * k;
        length_i = sqrt(pow((M_PI * Dik), 2) + k * k);
        Ltotal += length_i;
    }
    return Ltotal;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double rosaKm(double n){
    //Rosa's round wire mutual inductance correction
    double n2 = n * n;
    double n3 = n2 * n;
    double n5 = n3 * n2;
    double n7 = n5 * n2;
    double n9 = n7 * n2;
    return (log(2 * M_PI) - 1.5 - log(n) / (6 * n) - 0.33084236 / n - 1 / (120 * n3) + 1 / (504 * n5) - 0.0011923 / n7 + 0.0005068 / n9);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double rosaKs(double x){
    //Rosa's round wire self inductance correction
    return (1.25 - log(2 * x));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EF(double c, _Elliptic *result){
    double a = 1;
    double b = sqrt(1 - pow(c, 2));
    double E = 1 - pow(c, 2) / 2;
    double i = 1;
    while (fabs(a - b) > 1e-15) {
        double a1 = (a + b) / 2;
        double b1 = sqrt(a * b);
        E = E - i * pow((a - b) / 2, 2);
        i = 2 * i;
        a = a1;
        b = b1;
    }
    double Fk = M_PI / (2 * a);
    double Ek = E * Fk;
    result->Ek = Ek;
    result->Fk = Fk;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double Mut(double r1, double r2, double x, double g){
    double l = sqrt(pow(r2 - r1, 2) + pow(x, 2));
    double c = 2 * sqrt(r1 * r2) / sqrt(pow(r1 + r2, 2) + pow(l - g, 2));
    _Elliptic Ec;
    EF(c, &Ec);
    double result = -0.004 * M_PI * sqrt(r1 * r2) * ((c - 2 / c) * Ec.Fk + (2 / c) * Ec.Ek);
    return (result);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double Ingrnd(double phi, double kphitheta, double sinpsi, double cos2psi, double rr, double y){
    // by Robert Weaver from http://electronbunker.ca/eb/CalcMethods2d.html
    //Integrand function called by HeliCoilS()
    double result = (1 + cos2psi * (cos(kphitheta) - 1)) / sqrt(2 * rr * (1 - cos(kphitheta)) + (sinpsi * phi - y) *
                                                                (sinpsi * phi - y));
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double HeliCoilS(double Lw, double psi, double r, double dw, double w, double t, bool isRoundWire){
    // by Robert Weaver from http://electronbunker.ca/eb/CalcMethods2d.html (Version 1.0, 2011-03-25)
    // edited by Valery Kustarev 2018-12-16

    // Uses helical filament mutual inductance formula
    // evaluated using Simpson's rule, and conductor gmd
    // Lw = total length of wire
    // psi = pitch angle of winding
    // r = radius of winding
    // dw = wire diameter
    // MaxErr = max allowable error (set to 10000)
    //
    //px = p/(2pi) = lCOIL/(2piN)
    //psi = Arctan(px/(piD)) = Arctan(p/(2pi^2*D))
    //lW = piND/cos psi
    //where:
    //D is coil diameter = 2r;
    //p is centre to centre turns spacing;
    //lCOIL is length of coil.
    // If Lw>2*pi*r, check that pitch angle >= psi-c (close wound pitch)

    double sinpsic, psic, g, rr, psio, ThetaO, Y0, cosThetaO, k1, k2, t1, t0, c2s, ss, k, a, b, grandtotal;
    double dx, MaxErr, CurrentErr, kat, kbt, Sum2, LastIntg, Sum, phi, kpt, Integral, aaa, bbb, ccc, ddd;
    int i, m;

    MaxErr = 10000;
    Integral = 0;
    //  grandtotal = 0;
    if (Lw > 2 * M_PI * r) {
        if (isRoundWire){
            sinpsic = dw / (2 * M_PI * r);
        } else {
            sinpsic = w / (2 * M_PI * r);
        }
        psic = atan(sinpsic / sqrt(1 - sinpsic * sinpsic));
        if (psi < psic) {
            //     pitch angle is too small, //     so set value of function to an illegal value and exit
            return -1;
        }
    }
    // gmd of solid round conductor. Other values may be substituted
    // for different conductor geometries
    if (isRoundWire){
        g = exp(-0.25) * dw / 2;
    } else {
        g = 0.2235*(w + t);
    }
    rr = r * r;
    psio = 0.5 * M_PI - psi;
    // Calculate Filament 2 offset angle
    // Trap for psi=0 condition in which case ThetaO=0 and Y0=g
    // Trap for psio=0 condition in which case use simplified formula for ThetaO and Y0=0
    // which happens with circular (non-helical) filament
    if (psi == 0) {
        ThetaO = 0;
        Y0 = g;
    } else if (psio == 0) {
        cosThetaO = 1 - (g * g / (2 * rr));
        ThetaO = -fabs(atan(sqrt(1 - cosThetaO * cosThetaO) / cosThetaO));
        Y0 = 0;
    } else {
        //  Use Newton-Raphson method
        k1 = (g * g) / (2 * r * r) - 1;
        k2 = tan(psio);
        k2 = 0.5 * k2 * k2;
        t1 = g / r * sin(psi);
        do {
            t0 = t1;
            t1 = t0 - (k1 + cos(t0) - k2 * t0 * t0) / (-sin(t0) - 2 * k2 * t0);
        } while (fabs(t1 - t0) > 1e-12);
        ThetaO = -fabs(t1);
        //   Calculate Filament 2 Y-offset, using formula (29)
        Y0 = sqrt(g * g - 2 * rr * (1 - cos(ThetaO)));
    }
    // Psi constants
    c2s = cos(psi) * cos(psi);
    ss = sin(psi);
    k = cos(psi) / r;
    // Start of Simpson's rule code
    a = 0;
    b = Lw / 32768;
    if (b > Lw) {
        b = Lw;
    }
    grandtotal = 0;
    while (a < Lw) {
        dx = b - a;
        m = 1;
        CurrentErr = 2 * MaxErr;
        kat = k * a;
        kbt = k * b;
        aaa = (Lw - a) * (Ingrnd(-a, -kat - ThetaO, ss, c2s, rr, Y0));
        bbb = Ingrnd(a, kat - ThetaO, ss, c2s, rr, Y0);
        ccc = (Lw - b) * (Ingrnd(-b, -kbt - ThetaO, ss, c2s, rr, Y0));
        ddd = Ingrnd(b, kbt - ThetaO, ss, c2s, rr, Y0);
        Sum2 = aaa + bbb + ccc + ddd;

        //   Initialize LastResult to trapezoidal area for test purposes

        LastIntg = Sum2 / 2 * dx;
        while ((CurrentErr > MaxErr) || (m < 512)) {
            m = 2 * m;
            dx = dx / 2;
            Sum = 0;
            int max= round(m/2);
            for (i = 1; i <= max; i++){
                phi = 2 * i * dx + a;
                kpt = k * phi;
                Sum = Sum + (Lw - phi) * (Ingrnd(-phi, -kpt - ThetaO, ss, c2s, rr, Y0) + Ingrnd(phi, kpt - ThetaO, ss, c2s, rr, Y0));
            }
            Integral = (4 * (Sum) + Sum2) * dx / 3;
            CurrentErr = fabs((Integral) / (LastIntg) - 1);
            LastIntg = Integral;
            Sum2 = Sum2 + Sum * 2;
        }
        grandtotal = grandtotal + Integral;
        a = b;
        b = b * 2;
        if (b > Lw) {
            b = Lw;
        }
    }
    double result = 1e-1 * grandtotal;
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double solveHelicalInductance(double N, double p, double Dk, double dw, double w, double t, double *lw, bool isRoundWire){
    double lW;
    double psi;
    double sinpsi;
    p = p / 1000;
    Dk = Dk / 1000;
    double Result;
    if (isRoundWire){
        dw = dw / 1000;
    } else {
        w = w / 1000;
        t = t / 1000;
    }
    //psi= atan(p/(2*pi*pi*Dk));
    sinpsi = p / (M_PI * Dk);
    psi = atan(sinpsi / sqrt(1 - sinpsi * sinpsi));
    lW = M_PI * N * Dk / cos(psi);
    if (isRoundWire){
        Result = HeliCoilS(lW, psi, Dk / 2, dw, 0, 0, isRoundWire);
    } else {
        Result = HeliCoilS(lW, psi, Dk / 2, 0, w, t, isRoundWire);
    }
    *lw = lW;
    return (Result);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PUBLIC FUNCTIONS REALIZATION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double getOneLayerN_withRoundWire(double Dk, double dw, double p, double I, double *lw){

    double ind, N_max, N_min, N, k;
    k = 2;
    N_min = 0;

    N = sqrt(I / (0.0002 * M_PI * Dk * (log(1 + M_PI / (2 * k)) + 1 / (2.3004 + 3.437 * k + 1.763 * k * k - 0.47 / pow((0.755 + 1 / k), 1.44)))));
    ind = solveHelicalInductance(N, p, Dk, dw, 0, 0, lw, true);
    while (ind < I) {
        N_min = N;
        N_max = 2 * N;
        N = (N_max + N_min) / 2;
        ind = solveHelicalInductance(N, p, Dk, dw, 0, 0, lw, true);
    }
    N_max = N;
    while (fabs(1 - (ind / I)) > 0.001) {
        N = (N_min + N_max) / 2;
        ind = solveHelicalInductance(N, p, Dk, dw, 0, 0, lw, true);
        if (ind > I) {
            N_max = N;
        } else {
            N_min = N;
        }
    }
    return N;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double getOneLayerI_withRoundWire(double Dk, double dw, double p, double N, double *lw){
    return solveHelicalInductance(N, p, Dk, dw, 0, 0, lw, true);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double getOneLayerN_withRectWire(double Dk, double w, double t, double p, double I, double *lw){

    double ind, N_max, N_min, N, k;
    k = 2;
    N_min = 0;

    N = sqrt(I / (0.0002 * M_PI * Dk * (log(1 + M_PI / (2 * k)) + 1 / (2.3004 + 3.437 * k + 1.763 * k * k - 0.47 / pow((0.755 + 1 / k), 1.44)))));
    ind = solveHelicalInductance(N, p, Dk, 0, w, t, lw, false);
    while (ind < I) {
        N_min = N;
        N_max = 2 * N;
        N = (N_max + N_min) / 2;
        ind = solveHelicalInductance(N, p, Dk, 0, w, t, lw, false);
    }
    N_max = N;
    while (fabs(1 - (ind / I)) > 0.001) {
        N = (N_min + N_max) / 2;
        ind = solveHelicalInductance(N, p, Dk, 0, w, t, lw, false);
        if (ind > I) {
            N_max = N;
        } else {
            N_min = N;
        }
    }
    return N;

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double getOneLayerI_withRectWire(double Dk, double w, double t, double p, double N, double *lw){
    return solveHelicalInductance(N, p, Dk, 0, w, t, lw, false);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getMultiLayerN(double I, double D, double dw, double k, double lk, double gap, long Ng, _CoilResult *result){
    double n_g = 0;
    double jg = 0;
    D = D / 10;
    lk = lk / 10;
    dw = dw / 10;
    k = k / 10;
    gap = gap / 10;
    if (Ng == -1) {
        gap = 0;
    }
    double Ltotal = 0; // initialize variable of total self-inductance
    int nLayer = 1;
    double lw = 0;
    double r0 = (D + k) / 2;
    unsigned long int N = 0;
    int Nl = (int) floor(lk / k); // number of turns in layer
    double g = exp(-0.25) * dw / 2;
    while (Ltotal < I) // start calculation loop increasing N-turns to reach requiring inductance (I)
    {
        N++;
        int Nc = (N - 1) % Nl; // position of N-turn in layer
        nLayer = (int) floor((N - 1) / Nl); // current layer for N-turn
        if (((nLayer % Ng) == 0) && (nLayer > 0)) {
            n_g = gap;
        } else {
            n_g = 0;
        }
        double nx = Nc * k; // x-offset of turn
        double ny = r0 + k * nLayer + n_g; // y-offset of turn
        double Lns = Mut(ny, ny, g, 0); // self inductance of current turn
        lw = lw + 2 * M_PI * ny; // length of wire with the current turn
        double M = 0; // start calculation loop of the mutual inductance - current turn (N) + all another turns (j)
        if (N > 1) {
            int j;
            for (j = N; j >= 2; j--) {
                double Jc = (j - 2) % Nl;
                double jx = Jc * k;
                int jLayer = (int) floor((j - 2) / Nl);
                if (((jLayer % Ng) == 0) && (jLayer > 0)) {
                    jg = gap;
                } else {
                    jg = 0;
                }
                double jy = r0 + k * jLayer + jg;
                M = M + 2 * Mut(ny, jy, nx - jx, g); // mutual inductance
                // between current
                // N-turn and j-turn
            }
        }
        Ltotal = Ltotal + Lns + M; // total summary inductance (adding self-inductance and mutual inductance of current N-turn)
    }
    double R = (0.0175 * lw * 1e-4 * 4) / (M_PI * dw * dw); // resistance of the wire
    double lw0 = lw / 100;
    double NLayer = nLayer + 1;
    int NumberInterLayer = (int) floor(nLayer / Ng);
    double c = NLayer * k * 10 + NumberInterLayer * gap * 10;
    result->N = R;
    result->sec = lw0;
    result->thd = NLayer;
    result->fourth = c;
    result->five = NumberInterLayer;
    result->six = N;
    // return: N(turns); R(resistance) Ohm; lw0(length of wire) m;
    // NLayer (Number of layer); c (Winding thickness) mm
    // NumberInterLayer (Number of inter-layers);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getMultiLayerI(double D, double lk, double dw, double k, double b, double gap, long Ng, _CoilResult *result){
    double bTmp, nTmp, lw, Lns, Ltotal, r0, M, g, nx, ny, jx, jy, n_g = 0, jg = 0, ind1, ind2, N1, N2;
    int n, Nl, j, Nc, Jc, nLayer, jLayer;
    ind1 = 0;
    D = D / 10;
    lk = lk / 10;
    b = b / 10;
    nTmp = 0;
    bTmp = 0;
    dw = dw / 10;
    k = k / 10;
    gap = gap / 10;
    if (Ng == -1) {
        gap = 0;
    }
    Ltotal = 0; // initialize variable of total self-inductance
    lw = 0;
    r0 = (D + k) / 2;
    n = 0;
    Nl = (int) floor(lk / k);
    g = exp(-0.25) * dw / 2;
    while (bTmp < (b + k)) {
        n++;
        Nc = (n - 1) % Nl;
        nLayer = (int) floor((n - 1) / Nl);
        if (((nLayer % Ng) == 0) && (nLayer > 0)) {
            n_g = gap;
        } else {
            n_g = 0;
        }
        nx = Nc * k;
        ny = r0 + k * nLayer + n_g;
        Lns = Mut(ny, ny, g, 0);
        // self inductance of current turn
        lw = lw + 2 * M_PI * ny;
        M = 0;
        if (n > 1) {
            for (j = n; j >= 2; j--) {
                Jc = (j - 2) % Nl;
                jx = Jc * k;
                jLayer = (int) floor((j - 2) / Nl);
                if (((jLayer % Ng) == 0) && (jLayer > 0)) {
                    jg = gap;
                } else {
                    jg = 0;
                }
                jy = r0 + k * jLayer + jg;
                M = M + 2 * Mut(ny, jy, nx - jx, g);
            }
        }
        Ltotal = Ltotal + Lns + M;
        if (nTmp < b) {
            nTmp = (nLayer + 1) * k;
            ind1 = Ltotal;
        }
        bTmp = (nLayer + 1) * k;
    }
    N1 = n - Nl;
    N2 = n + Nl;
    ind2 = Ltotal;
    result->N = ind1;
    result->sec = ind2;
    result->thd = N1;
    result->fourth = N2;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  getMultiLayerI_fromResistance (double D, double lk, double b, double k, double Rm, _CoilResult *result){

  double dw, bTmp, nTmp, tmpR, lw, Lns, Ltotal, r0, M, g, nx, ny, jx, jy, N1, N2;
  int n, Nl, j, Nc, Jc, nLayer = 0, jLayer;

  double aWire[67][5] = {{0.06, 0.075, 0.09, 0.085, 0.09},
                         {0.063, 0.078, 0.09, 0.085, 0.09}, {0.07, 0.084, 0.092, 0.092, 0.1}, {0.071, 0.088, 0.095, 0.095, 0.1},
                         {0.08, 0.095, 0.105, 0.105, 0.11}, {0.09, 0.105, 0.12, 0.115, 0.12}, {0.1, 0.122, 0.13, 0.125, 0.13},
                         {0.112, 0.134, 0.14, 0.125, 0.14}, {0.12, 0.144, 0.15, 0.145, 0.15}, {0.125, 0.149, 0.155, 0.15, 0.155},
                         {0.13, 0.155, 0.16, 0.155, 0.16}, {0.14, 0.165, 0.17, 0.165, 0.17}, {0.15, 0.176, 0.19, 0.18, 0.19},
                         {0.16, 0.187, 0.2, 0.19, 0.2}, {0.17, 0.197, 0.21, 0.2, 0.21}, {0.18, 0.21, 0.22, 0.21, 0.22},
                         {0.19, 0.22, 0.23, 0.22, 0.23}, {0.2, 0.23, 0.24, 0.23, 0.24}, {0.21, 0.24, 0.25, 0.25, 0.25},
                         {0.224, 0.256, 0.27, 0.26, 0.27}, {0.236, 0.26, 0.285, 0.27, 0.28}, {0.25, 0.284, 0.3, 0.275, 0.3},
                         {0.265, 0.305, 0.315, 0.305, 0.31}, {0.28, 0.315, 0.33, 0.315, 0.33}, {0.3, 0.34, 0.35, 0.34, 0.34},
                         {0.315, 0.35, 0.365, 0.352, 0.36}, {0.335, 0.375, 0.385, 0.375, 0.38}, {0.355, 0.395, 0.414, 0.395, 0.41},
                         {0.38, 0.42, 0.44, 0.42, 0.44}, {0.4, 0.44, 0.46, 0.442, 0.46}, {0.425, 0.465, 0.485, 0.47, 0.47},
                         {0.45, 0.49, 0.51, 0.495, 0.5}, {0.475, 0.525, 0.545, 0.495, 0.53}, {0.5, 0.55, 0.57, 0.55, 0.55},
                         {0.53, 0.58, 0.6, 0.578, 0.6}, {0.56, 0.61, 0.63, 0.61, 0.62}, {0.6, 0.65, 0.67, 0.65, 0.66},
                         {0.63, 0.68, 0.7, 0.68, 0.69}, {0.67, 0.72, 0.75, 0.72, 0.75}, {0.71, 0.76, 0.79, 0.77, 0.78},
                         {0.75, 0.81, 0.84, 0.81, 0.83}, {0.8, 0.86, 0.89, 0.86, 0.89}, {0.85, 0.91, 0.94, 0.91, 0.94},
                         {0.9, 0.96, 0.99, 0.96, 0.99}, {0.93, 0.99, 1.02, 0.99, 1.02}, {0.95, 1.01, 1.04, 1.02, 1.04},
                         {1.0, 1.07, 1.1, 1.07, 1.11}, {1.06, 1.13, 1.16, 1.14, 1.16}, {1.08, 1.16, 1.19, 1.16, 1.19},
                         {1.12, 1.19, 1.22, 1.2, 1.23}, {1.18, 1.26, 1.28, 1.26, 1.26}, {1.25, 1.33, 1.35, 1.33, 1.36},
                         {1.32, 1.4, 1.42, 1.4, 1.42}, {1.4, 1.48, 1.51, 1.48, 1.51}, {1.45, 1.53, 1.56, 1.53, 1.56},
                         {1.5, 1.58, 1.61, 1.58, 1.61}, {1.56, 1.63, 1.67, 1.64, 1.67}, {1.6, 1.68, 1.71, 1.68, 1.71},
                         {1.7, 1.78, 1.81, 1.78, 1.81}, {1.74, 1.82, 1.85, 1.82, 1.85}, {1.8, 1.89, 1.92, 1.89, 1.92},
                         {1.9, 1.99, 2.02, 1.99, 2.02}, {2.0, 2.1, 2.12, 2.1, 2.12}, {2.12, 2.21, 2.24, 2.22, 2.24},
                         {2.24, 2.34, 2.46, 2.34, 2.46}, {2.36, 2.46, 2.48, 2.36, 2.48}, {2.5, 2.6, 2.63, 2.6, 2.62}};

  D = D / 10;
  lk = lk / 10;
  k = k / 10;
  b = b /10;
  bTmp = 0;
  nTmp = 0;
  for (int z = 0; z < 67; ++z){
    dw = aWire[z][0] / 10;
    Ltotal = 0;
    // initialize variable of total self-inductance
    lw = 0;
    r0 = (D + k) / 2;
    n = 0;
    Nl = (int) floor(lk / k);
    g = exp(-0.25) * dw / 2;
    tmpR = 0;
    while (tmpR < Rm){
      n++;
      Nc = (n - 1) %  Nl;
      nLayer = (int) floor((n - 1) / Nl);
      nx = Nc * k;
      ny = r0 + k * nLayer;
      Lns = Mut(ny, ny, g, 0);
      // self inductance of current turn
      lw = lw + 2 * M_PI * ny;
      M = 0;
      if (n > 1){
        for (j = n; j >= 2; j--){
          Jc = (j - 2) % Nl;
          jx = Jc * k;
          jLayer = (int) floor((j - 2) / Nl);
          jy = r0 + k * jLayer;
          M = M + 2 * Mut(ny, jy, nx - jx, g);
        }
      }
      Ltotal = Ltotal + Lns + M;
      tmpR = (0.0175 * lw * 1e-4 * 4) / (M_PI * dw * dw);
    }
    bTmp = (nLayer + 1) * k;
    if (bTmp > b)
      break;
    if (nTmp < b){
      nTmp = (nLayer + 2) * k;
      result->N = Ltotal;
    }
  }
  N1 = n - Nl;
  N2 = n + Nl;
  result->sec = Ltotal;
  result->thd = N1;
  result->fourth = N2;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getFerriteN(double L, double Do, double Di, double h, double dw, double mu, _CoilResult *result){
    double w = 0, wt;

    if ((Do / Di) < 1.75) {
        w = 100 * sqrt((L * (Do + Di)) / (4 * h * mu * (Do - Di)));
    } else {
        w = 100 * sqrt(L / (2 * h * mu * log(Do / Di)));
    }
    double D1t = Do;
    double D2t = Di;
    double ht = h;
    double Lt = 0;
    double w1 = 0;
    do {
        wt = M_PI * D2t / dw;
        // Number of turns that fit into internal diameter
        w1 = w1 + wt; // Increasing Number of turns counter
        if (w1 >= w) {
            Lt = Lt + (w - (w1 - wt)) * (D1t - D2t + ht * 2);
            // Wire length of this layer with previos counter
            break;
        }
        Lt = Lt + wt * (D1t - D2t + ht * 2); // Wire length of this layer
        Lt = Lt + Lt * 0.03;
        D1t = D1t + dw; // Increasing coil size
        D2t = D2t - dw;
        ht = ht + 2 * dw;
        if (D2t <= dw) {
            Lt = -100;
            break;
        }
    } while (w1 < w);

    double lw = 0.001 * Lt;
    result->N = w;
    result->sec = lw;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double getFerriteI(double N, double Do, double Di, double h, double mu){
    double I;
    if (Do / Di < 1.75) {
        I = 4E-04 * mu * h * N * N * (Do - Di) / (Do + Di);
    } else {
        I = 2E-04 * mu * h * N * N * log(Do / Di);
    }
    return I;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getPCB_N (double I, double D, double d, double ratio, _CoilResult *result) {
    double betta = 1.62E-3;
    double alpha1 = -1.12;
    double alpha2 = -0.147;
    double alpha3 = 2.40;
    double alpha4 = 1.78;
    double alpha5 = -0.03;
    double N, t = 0, W = 0, Davg, iTmp = 0;

    Davg = (d + D) / 2;
    N = 0.5;
    while (iTmp < I) {
        N = N + 0.01;
        t = (D - d) / (N);
        W = t * ratio;
        iTmp = betta * pow(D, alpha1) * pow(W, alpha2) * pow(Davg, alpha3) * pow(N, alpha4) * pow(t - W, alpha5);
    }
    if (t < 0) {
        N = 0;
    }
    result->N = N;
    result->sec = t;
    result->thd = W;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double getPCB_I(double N, double D, double d, double s, double W) {
    double betta = 1.62E-3;
    double alpha1 = -1.12;
    double alpha2 = -0.147;
    double alpha3 = 2.40;
    double alpha4 = 1.78;
    double alpha5 = -0.03;

    double Davg = (D + d) / 2;
    double I = betta * pow(D, alpha1) * pow(W, alpha2) * pow(Davg, alpha3) * pow(N, alpha4) * pow(s - W, alpha5);
    return (I);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getSpiralN(double I, double Di, double dw, double s, _CoilResult *result) {
    Di = Di / 10;
    dw = dw / 10;
    s = s / 10;
    double g = exp(-0.25) * dw / 2;
    double k = s + dw;
    int N = 0;
    double r0 = (Di + dw) / 2;
    double w = 0;
    double Ltotal = 0;
    while (Ltotal < I) {
        N++;
        double ny = r0 + k * (N - 1);
        double Lns = Mut(ny, ny, g, 0);
        double M = 0;
        if (N > 1) {
            for (double j = N; j >= 2; j--) {
                double jy = r0 + k * (j - 2);
                M = M + 2 * Mut(ny, jy, 0, g);
            }
        }
        Ltotal = Ltotal + Lns + M;
    }
    w = k * (N - 1);
    double Do = (Di + 2 * w) * 10;
    double Length_spiral = find_spiral_length(N, Di, k) * 10;
    result->N = N;
    result->sec = Length_spiral/1000;
    result->thd = Do;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getSpiralI(double Do, double Di, double dw, int _N, _CoilResult *result) {

    Di = Di / 10;
    Do = Do / 10;
    dw = dw / 10;
    double g = exp(-0.25) * dw / 2;
    double w = (Do - Di) / 2;
    double k = w / (_N - 1);
    double r0 = (Di + dw) / 2;
    double Ltotal = 0;
    for (int N = 1; N < _N; N++) {
        double ny = r0 + k * (N - 1);
        double Lns = Mut(ny, ny, g, 0);
        double M = 0;
        if (N > 1) {
            for (double j = N; j >= 2; j--) {
                double jy = r0 + k * (j - 2);
                M = M + 2 * Mut(ny, jy, 0, g);
            }
        }
        Ltotal = Ltotal + Lns + M;
    }
    double Length_spiral = find_spiral_length(_N, Di, k) * 10;
    result->N = Ltotal;
    result->sec = Length_spiral / 1000;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CalcCONTUR0(double L, double C) {
    double f = 1e3 / (2 * M_PI * sqrt(L * C));
    return f;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CalcCONTUR1(double C, double f) {
    double L1 = 1e3 / (2 * M_PI * f);
    double L = pow(L1, 2) / C;
    return L;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CalcCONTUR2(double L, double f) {
    double C1 = 1e3 / (2 * M_PI * f);
    double C = pow(C1, 2) / L;
    return C;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void findToroidPemeability(double N, double I, double Do, double Di, double h, _CoilResult *result) {
    double m;
    if ((Do / Di) < 1.75) {
        m = ceil((10000 * I * (Do + Di)) / (4 * N * N * h * (Do - Di)));
    } else {
        m = ceil(10000 * I / (2 * N * N * h * log(Do / Di)));
    }
    double al = 1000 * I / (N * N);
    result->N = m;
    result->sec = al;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void findFerriteRodN(double I, double Lr, double Dr, double mu, double dc, double s, double dw, double p, _CoilResult *result) {
    //Based on "The Inductance of Ferrite Rod Antennas Issue" by Alan Payne
    //[10.1][10.2] http://g3rbj.co.uk/wp-content/uploads/2014/06/Web-The-Inductance-of-Ferrite-Rod-Antennas-issue-3.pdf
    double x2 = 2 * s / Lr;
    double N = 0;
    double Dk = dc + dw;
    double iTmp = 0;
    double Lf_Lair = 0;
    double lc = 0;
    //A.V. Cainov's regression analysis algorithm calculates correction factor on shifting the coil from the center of the rod
    double kx = -440.9943706 * pow(x2, 8) + 1318.707293 * pow(x2, 7) - 1604.5491034 * pow(x2, 6) + 1021.078226 * pow(x2, 5) -
            363.8218957 * pow(x2, 4) + 71.6178135 * pow(x2, 3) - 7.6027344 * pow(x2, 2) + 0.3013663 * x2 + 0.995;
    if ((kx < 0) || (kx > 1)) {
        return;
    }
    double e0 = 8.8542e-12;
    double mufe = (mu - 1) * pow((Dr / Dk), 2) + 1;
    double dLp = -1e-4 * Dk * ((p / dw) - 1) * ((12 - (p / dw)) / 4);
    while (iTmp < I) {
        N++;
        lc = N * p;
        double k = lc / Dk;
        //The optimized version of Wheeler's Continuous Inductance formula for the one-layer coil corrected by Robert Weaver
        //[34] (http://electronbunker.ca/eb/CalcMethods3b.html)
        double i = 0.0002 * M_PI * Dk * N * N * (log(1 + M_PI / (2 * k)) + 1 / (2.3004 + 3.437 * k + 1.763 *
                k * k - 0.47 / pow((0.755 + 1 / k), 1.44)));

        double Ks = rosaKs(p / dw);
        double Km = rosaKm(N);
        double Lcor = 0.0002 * M_PI * Dk * N * (Ks + Km);
        double Lc = i - Lcor;
        if (N > 4) {
            Lc = Lc + dLp * N;
        }
        if (mu > 1) {
            double Canf = 5e-4 * M_PI * e0 * (Lr - lc) / (log(2 * (Lr + Dr) / Dr) - 1);
            double phi_phimax = 1 / (1 + (pow((Lr - lc) / Dr, 1.4) / (5 * mu)));
            double _k = ((phi_phimax * Canf / e0) + 2e-3 * Dr) / (2e-3 * dc);
            double l_prim = lc + 0.45 * dc;
            double _x = 5.1 * (l_prim / dc) / (1 + 2.8 * (dc / l_prim));
            mufe = (mu - 1) * pow(Dr / dc, 2) + 1;
            Lf_Lair = (1 + _x) / (1 / _k + _x / mufe);
            iTmp = Lc * Lf_Lair * kx;
        } else {
            Lf_Lair = 1;
            iTmp = Lc;
        }
    }
    result->N = N;
    result->sec = Lf_Lair;
    result->thd = lc;
}

