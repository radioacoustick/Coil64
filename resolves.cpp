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

#pragma pack(push,1)
struct _MagCoreConst{
    double C1;
    double C2;
};
#pragma pack(pop)

const double mu0 = 4e-7 * M_PI;
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
double convertfromAWG(QString AWG, bool *isOK){
    QRegularExpression awgrule(AWG_REG_EX);
    double result = 0;
    if (awgrule.match(AWG).hasMatch()){
        QStringList captured = awgrule.match(AWG).capturedTexts();
        QString cap = captured[0];
        if (AWG == cap){
            if (AWG == "0000") result = 11.684;
            else if (AWG == "000") result = 10.40384;
            else if (AWG == "00") result = 9.26592;
            else {
                double awg = AWG.toDouble();
                result = 0.127 * pow(92, (36 - awg) / 39);
            }
            if (isOK != NULL) *isOK = true;
        } else if (isOK != NULL) *isOK = false;
    } else if (isOK != NULL) *isOK = false;
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString converttoAWG(double d, bool *isOK){
    QString result ="";
    if (!(d > 12)){
        if ((d <= 12) && (d >= 11)) result = "0000";
        else if ((d < 11) && (d >= 10)) result = "000";
        else if ((d < 10) && (d >= 8.6)) result = "00";
        else if ((d < 8.6) && (d >= 0.00785)){
            double AWG = round(-39 * log(d / 0.127) / log(92) + 36);
            result = QString::number(AWG);
        }
        if (isOK != NULL) *isOK = true;
    } else if (isOK != NULL) *isOK = false;
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double find_Archimedean_spiral_length(int n, double a) {
  //function to calculate the Archimedean spiral length
  double phi = 2 * n * M_PI;
  double l = (a / (4 * M_PI)) * (phi * sqrt(1 + phi * phi) + log(phi + sqrt(1 + phi * phi)));
  return l;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double find_actual_spiral_length(int N, double Din, double k) {
    //Find the spiral length
    int ni = ceil(Din / (2 * k));
    double Lin = find_Archimedean_spiral_length(ni, k);
    int n = ni + N;
    double Lt = find_Archimedean_spiral_length(n, k);
    return (Lt - Lin);
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
    //the complete elliptic integrals of the first and second kind
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
    //Mutual inductance of two coaxial circular filaments
    //r1,r2 - radii of the two circular filaments
    //x - distance between the centres of the circular filaments
    //g - Geometric Mean Distance
    double l = sqrt(pow(r2 - r1, 2) + pow(x, 2));
    double c = 2 * sqrt(r1 * r2) / sqrt(pow(r1 + r2, 2) + pow(l - g, 2));
    _Elliptic Ec;
    EF(c, &Ec);
    double result = -0.004 * M_PI * sqrt(r1 * r2) * ((c - 2 / c) * Ec.Fk + (2 / c) * Ec.Ek);
    return (result);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double SelfInductanceStraightWire(double l, double dw){
    double r = 0.5 * dw * exp(0.25);
    double result = 0.002 * (l * log((l + sqrt(l * l + r * r)) / r) - sqrt(l * l + r * r) + l / 4 + r);
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double MutInductanceStraightWire(double L1, double l2, double D){
    double tmpLength = 0;
    L1 = L1 / 2;
    l2 = l2 / 2;
    if (L1 < l2){
        tmpLength = L1;
        L1 = l2;
        l2 = tmpLength;
    }
    double result = 0.002 * (2 * L1 * log((L1 + l2 + sqrt((L1 + l2) * (L1 + l2) + D * D)) / D) + (L1 - l2) *
                             log((L1 + l2 + sqrt((L1 + l2) * (L1 + l2) + D * D)) / (L1 - l2 + sqrt((L1 - l2) * (L1 - l2) + D * D))) +
                             sqrt((L1 - l2) * (L1 - l2) + D * D) - sqrt((L1 + l2) * (L1 + l2) + D * D));
    return result;
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
double HeliCoilS(double Lw, double psi, double r, double dw, double w, double t, bool isRoundWire, unsigned int accuracy){
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
    int err = (int) round(accuracy / 2);
    MaxErr = pow(10,-err);
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
        while ((CurrentErr > MaxErr) || (m < 1024)) {
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
double solveHelicalInductance(double N, double p, double Dk, double dw, double w, double t, double *lw, bool isRoundWire, unsigned int accuracy){
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
        Result = HeliCoilS(lW, psi, Dk / 2, dw, 0, 0, isRoundWire, accuracy);
    } else {
        Result = HeliCoilS(lW, psi, Dk / 2, 0, w, t, isRoundWire, accuracy);
    }
    *lw = lW;
    return (Result);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double deriveOneLayerPoligonalN(double Dk, double dw, double p, double n, double I, double *lw, unsigned int accuracy){
  double Ind, N_max, N_min, N, k, iDk, Kw, rA, rP;
  k = 2;
  N_min = 0;
  rA = sqrt((1 / M_PI) * (0.5 * n * pow(0.5 * Dk,2) * sin(2 * M_PI / n)));
  rP = (0.5 / M_PI) * (Dk * n * sin(M_PI / n));
  Kw = sqrt(1 / 369);
  iDk = 2 * (((Kw * pow(rP,2)) + ((2 - Kw) * pow(rA,2))) / (2 * rA));
  N = sqrt(I / (0.0002 * M_PI * iDk * (log(1 + M_PI / (2 * k)) + 1 / (2.3004 + 3.437 * k + 1.763 * k * k - 0.47 /
    pow((0.755 + 1 / k), 1.44)))));
  _CoilResult res;
  getOneLayerI_Poligonal(Dk, dw, p, N, n, &res, accuracy);
  Ind = res.sec;
  while (Ind < I){
    N_min = N;
    N_max = 2 * N;
    N = (N_max + N_min) / 2;
    getOneLayerI_Poligonal(Dk, dw, p, N, n, &res, accuracy);
    Ind = res.sec;
  }
  N_max = N;
  while (fabs(1 - (Ind / I)) > 0.001){
    N = (N_min + N_max) / 2;
    getOneLayerI_Poligonal(Dk, dw, p, N, n, &res, accuracy);
    Ind = res.sec;
    if (Ind > I)
      N_max = N;
    else
      N_min = N;
  }
  *lw = res.thd;
  return N;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getFerriteCoreMagConst(double l1, double l2, double l3, double l4, double l5,
                            double A1, double A2, double A3, double A4, double A5, _MagCoreConst *c)
{
    //auxiliary function to get the constants C1 & C2 of a ferrite core with the close magnetic circuit
    double sum11 = l1 / A1;
    double sum21 = l1 / (A1 * A1);
    double sum12 = l2 / A2;
    double sum22 = l2 / (A2 * A2);
    double sum13 = l3 / A3;
    double sum23 = l3 / (A3 * A3);
    double sum14 = l4 / A4;
    double sum24 = l4 / (A4 * A4);
    double sum15 = l5 / A5;
    double sum25 = l5 / (A5 * A5);
    c->C1 = sum11 + sum12 + sum13 + sum14 + sum15;
    c->C2 = sum21 + sum22 + sum23 + sum24 + sum25;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PUBLIC FUNCTIONS REALIZATION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double getOneLayerN_withRoundWire(double Dk, double dw, double p, double I, double *lw, unsigned int accuracy){

    double ind, N_max, N_min, N, k;
    k = 2;
    N_min = 0;

    N = sqrt(I / (0.0002 * M_PI * Dk * (log(1 + M_PI / (2 * k)) + 1 / (2.3004 + 3.437 * k + 1.763 * k * k - 0.47 / pow((0.755 + 1 / k), 1.44)))));
    ind = solveHelicalInductance(N, p, Dk, dw, 0, 0, lw, true, accuracy);
    while (ind < I) {
        N_min = N;
        N_max = 2 * N;
        N = (N_max + N_min) / 2;
        ind = solveHelicalInductance(N, p, Dk, dw, 0, 0, lw, true, accuracy);
    }
    N_max = N;
    while (fabs(1 - (ind / I)) > 0.001) {
        N = (N_min + N_max) / 2;
        ind = solveHelicalInductance(N, p, Dk, dw, 0, 0, lw, true, accuracy);
        if (ind > I) {
            N_max = N;
        } else {
            N_min = N;
        }
    }
    return N;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double getOneLayerI_withRoundWire(double Dk, double dw, double p, double N, double *lw, unsigned int accuracy){
    return solveHelicalInductance(N, p, Dk, dw, 0, 0, lw, true, accuracy);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double getOneLayerN_withRectWire(double Dk, double w, double t, double p, double I, double *lw, unsigned int accuracy){

    double ind, N_max, N_min, N, k;
    k = 2;
    N_min = 0;

    N = sqrt(I / (0.0002 * M_PI * Dk * (log(1 + M_PI / (2 * k)) + 1 / (2.3004 + 3.437 * k + 1.763 * k * k - 0.47 / pow((0.755 + 1 / k), 1.44)))));
    ind = solveHelicalInductance(N, p, Dk, 0, w, t, lw, false, accuracy);
    while (ind < I) {
        N_min = N;
        N_max = 2 * N;
        N = (N_max + N_min) / 2;
        ind = solveHelicalInductance(N, p, Dk, 0, w, t, lw, false, accuracy);
    }
    N_max = N;
    while (fabs(1 - (ind / I)) > 0.001) {
        N = (N_min + N_max) / 2;
        ind = solveHelicalInductance(N, p, Dk, 0, w, t, lw, false, accuracy);
        if (ind > I) {
            N_max = N;
        } else {
            N_min = N;
        }
    }
    return N;

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double getOneLayerI_withRectWire(double Dk, double w, double t, double p, double N, double *lw, unsigned int accuracy){
    return solveHelicalInductance(N, p, Dk, 0, w, t, lw, false, accuracy);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double getOneLayerN_Poligonal(double I, double Dk, double dw, double  h, double n, _CoilResult *result, unsigned int accuracy){

  double N, lw;

  N = deriveOneLayerPoligonalN(Dk, dw, h, n, I, &lw, accuracy);
  result->sec = h * N;
  result->thd = lw;
  return N;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getOneLayerI_Poligonal(double Dk, double dw, double h, double N, double n, _CoilResult *result, unsigned int accuracy){

  double Kw, rA, rP, L, iDk, lw;

  L = N * h;
  rA = sqrt((1 / M_PI) * (0.5 * n * pow(0.5 * Dk,2) * sin(2 * M_PI / n)));
  rP = (0.5 / M_PI) * (Dk * n * sin(M_PI / n));
  Kw = sqrt(1 / (1 + 368 * (L / Dk)));
  iDk = 2 * (((Kw * pow(rP,2)) + ((2 - Kw) * pow(rA,2))) / (2 * rA));
  result->sec = solveHelicalInductance(N, h, iDk, dw, 0, 0, &lw, true, accuracy);
  result->thd = lw;
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getMultiLayerN_rectFormer(double Ind, double a, double b, double l, double dw, double k, _CoilResult *result){
    //Calculation formulas of multilayer inductor with rectangular former https://coil32.net/multilayer-rectangular.html

    double a0, b0, D, Db, Da, nx, ny, jx, jy, lengthNa, lengthNb, lengthJa, lengthJb, Ltotal, Ladd, Lsub, Madd, Msub, lw, Km, Ks, Lcor, Rdc;
    int n, Nc, Nl, Jc, nLayer, jLayer;

    a = a / 10;
    b = b / 10;
    l = l / 10;
    dw = dw / 10;
    k = k / 10;
    n = 0;
    Ltotal = 0;
    a0 = a + k;
    b0 = b + k;
    lw = 0;
    nLayer = 0;
    Nl = floor(l / k); // Number of turns in layer
    while (Ltotal < Ind){
        n++;
        Nc = (n - 1) % Nl; // Position of the turn on x
        nLayer = floor((n - 1) / Nl); // Position of the turn on y
        nx = Nc * k; // x-offset of current turn
        ny = nLayer * k; // y-offset of current turn
        lengthNa = a0 + 2 * k * (nLayer); Rdc = (0.0175 * lw * 1E-4 * 4) / (M_PI * dw * dw);// lenght of straight conductor of current turn (side a)
        lengthNb = b0 + 2 * k * (nLayer); // lenght of straight conductor of current turn (side b)
        lw += 2 * (a0 + b0 + 2 * k * (nLayer));
        Ladd = SelfInductanceStraightWire(lengthNa, dw) + SelfInductanceStraightWire(lengthNb, dw); // half of self-inductance of the current turn
        Db = 2 * ny + a0; // distance to opposite cunductor of the same turn (side b)
        Da = 2 * ny + b0; // distance to opposite cunductor of the same turn (side a)
        Lsub = 2 * MutInductanceStraightWire(lengthNa, lengthNa, Da) + 2 * MutInductanceStraightWire(lengthNb, lengthNb, Db);
        // half mutual inductance with opposite conductor of current turn
        Madd = 0;
        Msub = 0;
        if (n > 1){
            for (int j = n; j >= 2; j--){
                Jc = (j - 2) % Nl; // position of previous turn on x
                jx = Jc * k; // x-offset of previous turn
                jLayer = floor((j - 2) / Nl); // Position of the turn on y
                jy = k * jLayer; // y-offset of previous turn
                lengthJa = a0 + 2 * k * (nLayer + 1); // lenght of straight conductor of previous turn (side a)
                lengthJb = b0 + 2 * k * (nLayer + 1); // lenght of straight conductor of previous turn (side b)
                D = sqrt(pow(nx - jx, 2) + pow(ny - jy, 2)); // distance to in-phase straight conductor of previous turn
                Madd = Madd + 2 * MutInductanceStraightWire(lengthNa, lengthJa, D) + 2 * MutInductanceStraightWire(lengthNb, lengthJb, D);
                // half mutual inductance with in-phase conductor in previous turn
                Db = sqrt(pow(nx - jx, 2) + pow(ny + jy + a0, 2));
                // distance to opposite cunductor between the current turn and previous (side b)
                Da = sqrt(pow(nx - jx, 2) + pow(ny + jy + b0, 2));
                // distance to opposite cunductor between the current turn and previous (side a)
                Msub = Msub + 2 * MutInductanceStraightWire(lengthNa, lengthJa, Da) + 2 * MutInductanceStraightWire(lengthNb, lengthJb, Db);
                // half mutual inductance with opposite conductor in previous turn
            }
        }
        Ltotal += 2 * (Ladd - Lsub + Madd - Msub);
        Ks = rosaKs(k / dw);
        Km = rosaKm(n);
        Lcor = 0.0002 * M_PI * (a + b) * n * (Ks + Km);
        Ltotal = Ltotal - Lcor;
    }
    Rdc = (0.0175 * lw * 1E-4 * 4) / (M_PI * dw * dw);
    result->N = n; //number of turns
    result->sec = nLayer + 1; //number of layers
    result->thd = lw * 0.01; //length of wire
    result->fourth = Rdc; //resistance to DC
    result->five = (nLayer + 1) * k * 10; //coil thickness
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getMultiLayerI_byN(double D, double lk, double dw, double k, double N, _CoilResult *result)
{
    D = D / 10;
    lk = lk / 10;
    dw = dw / 10;
    k = k / 10;
    int Nl, Nc, Jc, nLayer = 0, jLayer;
    double nx, ny, jx, jy, Lns, M;
    double Ltotal = 0; // initialize variable of total self-inductance
    double lw = 0;
    double r0 = (D + k) / 2;
    Nl = (int) floor(lk / k);
    double g = exp(-0.25) * dw / 2;
    for (int w = 1; w < N + 1; w++){
        Nc = (w - 1) % Nl;
        nLayer = (int) floor((w - 1) / Nl);
        nx = Nc * k;
        ny = r0 + k * nLayer;
        Lns = Mut(ny, ny, g, 0);
        // self inductance of current turn
        lw = lw + 2 * M_PI * ny;
        M = 0;
        if (w > 1) {
            for (int j = w; j >= 2; j--) {
                Jc = (j - 2) % Nl;
                jx = Jc * k;
                jLayer = (int) floor((j - 2) / Nl);
                jy = r0 + k * jLayer;
                M = M + 2 * Mut(ny, jy, nx - jx, g);
            }
        }
        Ltotal = Ltotal + Lns + M;
    }
    double Rdc = (0.0175 * lw * 1E-4 * 4) / (M_PI * dw * dw);
    result->N = Ltotal; //inductance value
    result->sec = nLayer + 1; //number of layers
    result->thd = lw * 0.01; //length of wire
    result->fourth = Rdc; //resistance to DC
    result->five = (nLayer + 1) * k * 10; //coil thickness
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getMultiLayerI(double D, double lk, double dw, double k, double c, double gap, long Ng, _CoilResult *result){
    double bTmp, nTmp, lw, Lns, Ltotal, r0, M, g, nx, ny, jx, jy, n_g = 0, jg = 0, ind1, ind2, N1, N2;
    int n, Nl, j, Nc, Jc, nLayer, jLayer;
    ind1 = 0;
    D = D / 10;
    lk = lk / 10;
    c = c / 10;
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
    while (bTmp < (c + k)) {
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
        if (nTmp < c) {
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
void getMultiLayerI_rectFormer(double a, double b, double l, double c, double dw, double k, _CoilResult *result){

    double a0, b0, D, Db, Da, nx, ny, jx, jy, lengthNa, lengthNb, lengthJa, lengthJb, Ltotal, Ladd, Lsub, Madd,
            Msub, lw, Km, Ks, Lcor, cTmp = 0, nTmp = 0, ind1 = 0, ind2, N1, N2;
    int n, Nc, Nl, Jc, nLayer, jLayer;

    a = a / 10;
    b = b / 10;
    l = l / 10;
    c = c / 10;
    dw = dw / 10;
    k = k / 10;
    n = 0;
    Ltotal = 0;
    a0 = a + k;
    b0 = b + k;
    lw = 0;
    nLayer = 0;
    Nl = floor(l / k); // Number of turns in layer
    while (cTmp < (c + k)) {
        n++;
        Nc = (n - 1) % Nl; // Position of the turn on x
        nLayer = floor((n - 1) / Nl); // Position of the turn on y
        nx = Nc * k; // x-offset of current turn
        ny = nLayer * k; // y-offset of current turn
        lengthNa = a0 + 2 * k * (nLayer); // lenght of straight conductor of current turn (side a)
        lengthNb = b0 + 2 * k * (nLayer); // lenght of straight conductor of current turn (side b)
        lw = lw + 2 * (a0 + b0 + 2 * k * (nLayer));
        Ladd = SelfInductanceStraightWire(lengthNa, dw) + SelfInductanceStraightWire(lengthNb, dw); // half of self-inductance of the current turn
        Db = 2 * ny + a0; // distance to opposite cunductor of the same turn (side b)
        Da = 2 * ny + b0; // distance to opposite cunductor of the same turn (side a)
        Lsub = 2 * MutInductanceStraightWire(lengthNa, lengthNa, Da) + 2 * MutInductanceStraightWire(lengthNb, lengthNb, Db);
        // half mutual inductance with opposite conductor of current turn
        Madd = 0;
        Msub = 0;
        if (n > 1){
            for (int j = n; j >= 2; j--){
                Jc = (j - 2) % Nl; // position of previous turn on x
                jx = Jc * k; // x-offset of previous turn
                jLayer = floor((j - 2) / Nl); // Position of the turn on y
                jy = k * jLayer; // y-offset of previous turn
                lengthJa = a0 + 2 * k * (nLayer + 1); // lenght of straight conductor of previous turn (side a)
                lengthJb = b0 + 2 * k * (nLayer + 1); // lenght of straight conductor of previous turn (side b)
                D = sqrt(pow(nx - jx, 2) + pow(ny - jy, 2)); // distance to in-phase straight conductor of previous turn
                Madd = Madd + 2 * MutInductanceStraightWire(lengthNa, lengthJa, D) + 2 * MutInductanceStraightWire(lengthNb, lengthJb, D);
                // half mutual inductance with in-phase conductor in previous turn
                Db = sqrt(pow(nx - jx, 2) + pow(ny + jy + a0, 2));
                // distance to opposite cunductor between the current turn and previous (side b)
                Da = sqrt(pow(nx - jx, 2) + pow(ny + jy + b0, 2));
                // distance to opposite cunductor between the current turn and previous (side a)
                Msub = Msub + 2 * MutInductanceStraightWire(lengthNa, lengthJa, Da) + 2 * MutInductanceStraightWire(lengthNb, lengthJb, Db);
                // half mutual inductance with opposite conductor in previous turn
            }
        }
        Ltotal = Ltotal + 2 * (Ladd - Lsub + Madd - Msub);
        Ks = rosaKs(k / dw);
        Km = rosaKm(n);
        Lcor = 0.0002 * M_PI * (a + b) * n * (Ks + Km);
        Ltotal = Ltotal - Lcor;
        if (nTmp < c) {
            nTmp = (nLayer + 1) * k;
            ind1 = Ltotal;
        }
        cTmp = (nLayer + 1) * k;
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
void  getMultiLayerI_fromResistance (double D, double lk, double c, double k, double Rm, _CoilResult *result){

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
    c = c /10;
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
        if (bTmp > c)
            break;
        if (nTmp < c){
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
void getMultilayerN_Foil(double D, double w, double t, double ins, double I, _CoilResult *result){
    D = D / 10;
    w = w / 10;
    t = t /10;
    ins = ins / 10;
    double g = exp(-1.5) * w;
    //double g = 0.2235*(w + t);
    double k = ins + t;
    int N = 0;
    double r0 = (D + t) / 2;
    double Ltotal = 0;
    while (Ltotal <= I) {
        N++;
        double ny = r0 + k * (N - 1);
        double Lns = Mut(ny, ny, g, 0);
        double M = 0;
        if (N > 1) {
            for (double j = N; j >= 2; j--) {
                double jy = r0 + k * (j - 2);
                double r = ny - jy;
                double gmd = exp(((r * r) / (w * w)) * log(r) + 0.5 * (1 - ((r * r) / (w * w))) * log(w * w + r * r) + (2 * r / w) * atan(w / r) - 1.5);
                double gmr = sqrt(ny*jy);
                double ra = (gmd + sqrt(gmd * gmd + 4 * gmr * gmr)) / 2;
                double rb = ra - gmd;
                M = M + 2 * Mut(ra, rb, 0, gmd);
            }
        }
        Ltotal = Ltotal + Lns + M;
    }
    double th = k * (N - 1);
    double Do = (D + 2 * th) *10;
    double Length_spiral = find_actual_spiral_length(N, D, k) * 10;
    double Rdcc = (0.0175 * Length_spiral * 1e-4) / (w * t); // resistance of the copper foil
    double Rdca = (0.028 * Length_spiral * 1e-4) / (w * t); // resistance of the aliminum foil
    result->N = N;
    result->sec = Length_spiral/1000;
    result->thd = Do;
    result->fourth = Rdcc;
    result->five = Rdca;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getMultilayerI_Foil(double D, double w, double t, double ins, int _N, _CoilResult *result) {
    D = D / 10;
    w = w / 10;
    t = t / 10;
    ins = ins / 10;
    double g = exp(-1.5) * w;
    double k = ins + t;
    double r0 = (D + t) / 2;
    double Ltotal = 0;
    for (int N = 1; N <= _N; N++) {
        double ny = r0 + k * (N - 1);
        double Lns = Mut(ny, ny, g, 0);
        double M = 0;
        if (N > 1) {
            for (double j = N; j >= 2; j--) {
                double jy = r0 + k * (j - 2);
                double r = ny - jy;
                double gmd = exp(((r * r) / (w * w)) * log(r) + 0.5 * (1 - ((r * r) / (w * w))) * log(w * w + r * r) + (2 * r / w) * atan(w / r) - 1.5);
                double gmr = sqrt(ny*jy);
                double ra = (gmd + sqrt(gmd * gmd + 4 * gmr * gmr)) / 2;
                double rb = ra - gmd;
                M = M + 2 * Mut(ra, rb, 0, gmd);
            }
        }
        Ltotal = Ltotal + Lns + M;
    }
    double th = k * (_N - 1);
    double Do = (D + 2 * th) *10;
    double Length_spiral = find_actual_spiral_length(_N, D, k) * 10;
    double Rdcc = (0.0175 * Length_spiral * 1e-4) / (w * t); // resistance of the copper foil
    double Rdca = (0.028 * Length_spiral * 1e-4) / (w * t); // resistance of the aliminum foil
    result->N = Ltotal;
    result->sec = Length_spiral / 1000;
    result->thd = Do;
    result->fourth = Rdcc;
    result->five = Rdca;
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
void getPCB_N (double I, double D, double d, double ratio, int layout, _CoilResult *result) {

    double N = 0.5, s = 0, W = 0, iTmp = 0;
    while (iTmp < I) {
        N = N + 0.01;
        s = (D - d) / (2 * N);
        W = s * ratio;
        iTmp = getPCB_I(N, d, s, layout, result);
    }
    if (s < 0) {
        N = 0;
    }
    result->N = N;
    result->sec = s;
    result->thd = W;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double getPCB_I(double N, double _d, double _s, int layout, _CoilResult *result) {

    double c1 = 0, c2 = 0, c3 = 0, c4 = 0;

    switch (layout) {
    case 0:{
        c1 = 1.27;
        c2 = 2.07;
        c3 = 0.18;
        c4 = 0.13;
        break;
    }
    case 1:{
        c1 = 1.0;
        c2 = 2.46;
        c3 = 0.0;
        c4 = 0.2;
        break;
    }
    default:
        break;
    }
    double d = _d * 1e3;
    double s = _s * 1e3;
    double D = d + 2 * s * N;
    double Davg = (D + d) / 2;
    double fi = (D - d) / (D + d);
    double I = mu0 * N * N * Davg * c1 * 0.5 * (log(c2 / fi) + c3 * fi + c4 * fi * fi);

    result->five = D / 1e3;
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
    double Length_spiral = find_actual_spiral_length(N, Di, k) * 10;
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
    double Length_spiral = find_actual_spiral_length(_N, Di, k) * 10;
    result->N = Ltotal;
    result->sec = Length_spiral / 1000;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CalcLC0(double L, double C) {
    double f = 1e3 / (2 * M_PI * sqrt(L * C));
    return f;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CalcLC1(double C, double f) {
    double L1 = 1e3 / (2 * M_PI * f);
    double L = pow(L1, 2) / C;
    return L;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CalcLC2(double L, double f) {
    double C1 = 1e3 / (2 * M_PI * f);
    double C = pow(C1, 2) / L;
    return C;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CalcLC3(double Zo, double f, _CoilResult *result)
{
    result->N = 1e6 / (2 * M_PI * f * Zo);
    result->sec = Zo / (2 * M_PI * f);
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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void findMeadrPCB_I(double a, double d, double h, double W, int N, _CoilResult *result){
    //http://www.journal.ftn.kg.ac.rs/Vol_1-3/08-Stojanovic-Zivanov-Damnjanovic.pdf (The monomial equation [11])

    result->N = 0.00266 * pow(a, 0.0603) * pow(h, 0.4429) * pow(N, 0.954) * pow(d, 0.606) * pow(W, -0.173);
    result->sec = 2 * N * d + 2 * a;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findMultiloop_I(double N, double Di, double dw, double dt, _CoilResult *result){
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
    result->fourth = (0.0175 * result->thd * 100 * 1E-4 * 4) / (M_PI * dw * dw * 0.01); //Resistance to DC (Ohm)
    return ind;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long findMultiloop_N(double I, double Di, double dw, double dt, _CoilResult *result){
    double tmpI = 0;
    unsigned long int N = 0;
    while (tmpI <= I){
        N++;
        tmpI = findMultiloop_I(N, Di, dw, dt, result);
        if ((N > 1e7) || ((N == 1) && (tmpI > I)))
            return -1;
    }
    return N;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findRoundLoop_I(double D, double dw){
    //

    return 0.0002 * M_PI * D * (log(8 * D / dw) - 2);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findRoundLoop_D(double Ind, double dw){
    double tmpI = 0;
    double D = 2 * dw;
    unsigned long int i = 0;
    while (tmpI <= Ind){
        i++;
        tmpI = findRoundLoop_I(D, dw);
        D += 0.01;
        if ((D > 2e4) || ((i == 1) && (tmpI > Ind)))
            return -1;
    }
    return D;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findIsoIsoscelesTriangleLoop_I(double _a, double _b, double dw){
    //

    double c = _a / 1000;
    double b = _b / 1000;
    double r = dw / 2000;

    double a1 = 2 * c * log(2 * c / r);
    double a2 = b * log(2 * c / r);
    double a3 = 2 * (b + c) * asinh(b * b / (sqrt(4 * b * b * c * c - pow(b, 4))));
    double a4 = 2 * c * asinh((2 * c * c - b * b) / (sqrt(4 * b * b * c * c - pow(b, 4))));
    double a5 = 2 * c + b;
    return 0.2 * (a1 + a2 - a3 - a4 - a5);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findIsoIsoscelesTriangleLoop_a(double Ind, double dw){
    double tmpI = 0;
    double  a = 2 * dw;
    unsigned long int i = 0;
    while (tmpI <= Ind){
        i++;
        tmpI = findIsoIsoscelesTriangleLoop_I(a, a, dw);
        a += 0.01;
        if ((a > 2e4) || ((i == 1) && (tmpI > Ind)))
            return -1;
    }
    return a;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findRectangleLoop_I(double _a, double _b, double dw){
    //

    double a = _a / 1000;
    double b = _b / 1000;
    double r = dw / 2000;
    double d = sqrt(a * a  + b * b);

    double a1 = -2 * (a + b);
    double a2 = 2 * d;
    double a3 = b * log((b + d) / a);
    double a4 = a * log((a + d) / b);
    double a5 = b * log(2 * b / r);
    double a6 = a * log(2 * a / r);
    return 0.4 * (a1 + a2 - a3 - a4 + a5 + a6);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findRectangleLoop_a(double Ind, double dw){
    double tmpI = 0;
    double  a = 2 * dw;
    unsigned long int i = 0;
    while (tmpI <= Ind){
        i++;
        tmpI = findRectangleLoop_I(a, a, dw);
        a += 0.01;
        if ((a > 2e4) || ((i == 1) && (tmpI > Ind)))
            return -1;
    }
    return a;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findSheildedInductance(double I, double D, double Ds, double l, double Hs){
  double result= I * (1 - (D / Ds) * (D / Ds) * (D / Ds)) * (1 - (l / (2 * Hs)) * (l / (2 * Hs)));
  return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findAirCoreRoundToroid_I(double N, double D1, double D2, double dw)
{
  double R = 0.1 * (D1 + D2) / 4;
  double a = 0.05 * (((D1 - D2) / 2) + dw);
  double ind = 0.01257 * N * N * ( R - sqrt(R * R - a * a));
  return ind;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findAirCoreRoundToroid_N(double Ind, double D1, double D2, double dw)
{
    double R = 0.1 * (D1 + D2) / 4;
    double a = 0.05 * (((D1 - D2) / 2) + dw);
    double N = sqrt(Ind / (0.01257 * ( R - sqrt(R * R - a * a))));
    return N;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findPotCore_I(double N, double D1, double D2, double D3, double D4, double h1, double h2, double g, double b, double mu, _CoilResult *result)
{
    double r1 = 0.5 * D4;
    double r2 = 0.5 * D3;
    double r3 = 0.5 * D2;
    double r4 = 0.5 * D1;
    double h = 0.5 * (h1 - h2);
    double l1 = h2;
    double l3 = h2;
    double k1 = 2 * b * (r4 - r3);
    double A1 = M_PI * (r4 - r3) * (r4 + r3) - k1;
    double A3 = M_PI * (r2 - r1) * (r2 + r1);
    double sum11 = l1 / A1;
    double sum21 = l1 / (A1 * A1);
    double k2 = 1 / (1 - (2 * b / (2 * M_PI * r3)));
    double sum12 = k2 * log(r3 / r2) / (M_PI * h);
    double sum22 = k2 * (r3 - r2) / (2 * r3 * r2 * M_PI * M_PI * h * h);
    double sum13 = l3 / A3;
    double sum23 = l3 / (A3 * A3);
    double s1 = r2 - sqrt(0.5 * (r2 * r2 + r1 * r1));
    double s2 = sqrt(0.5 * (r3 * r3 + r3 * r3)) - r3;
    double l4 = 0.25 * M_PI * (2 * s2 + h);
    double l5 = 0.25 * M_PI * (2 * s1 + h);
    double k4 = 1 - (2 * b / (M_PI * (r3 + r4)));
    double A4 = 0.5 * k4 * M_PI * (r4 *r4 - r3 * r3 + 2 * r3 * h);
    double A5 = 0.5 * M_PI * (r2 *r2 - r1 * r1 + 2 * r2 * h);
    double sum14 = l4 / A4;
    double sum24 = l4 / (A4 * A4);
    double sum15 = l5 / A5;
    double sum25 = l5 / (A5 * A5);
    double C1 = sum11 + sum12 + sum13 + sum14 + sum15;
    double C2 = sum21 + sum22 + sum23 + sum24 + sum25;
    double le = C1 * C1 / C2;
    double Ae = C1 / C2;
    double mu_e = mu / (1 + g * mu / le);
    double ind = 1000 * N * N * mu0 * mu_e / C1;
    result->N = le;
    result->sec = Ae;
    result->thd = mu_e;
    return ind;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long findPotCore_N(double Ind, double D1, double D2, double D3, double D4, double h1, double h2, double g, double b, double mu, _CoilResult *result)
{
    double tmpI = 0;
    unsigned long int N = 0;
    while (tmpI <= Ind){
        N++;
        tmpI = findPotCore_I(N,D1,D2,D3,D4,h1,h2,g,b,mu, result);
    }
    return N;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findECore_I(double N, double A, double B, double C, double D, double E, double F, double g, double b, double mu,
                   bool isEI, bool isRound, _CoilResult *result)
{
    double l1;
    double l3;
    double k = 1;
    if (isRound)
        k = 1.1918;
    if (isEI){
        l1 = D;
        l3 = D;
    } else {
        l1 = 2 * D;
        l3 = 2 * D;
    }
    double h = B - D;
    double s = 0.5 * F;
    double p = (A - E) / 2;
    double q = C;
    double l2 = E - F;
    double l4 = M_PI * (p + h) / 4;
    double l5 = M_PI * (k * s + h) / 4;
    double A1 = 2 * q * p - 0.5 * M_PI * b * b;
    double A2 = 2 * q * h;
    double A3;
    if (isRound)
        A3 = M_PI * s * s;
    else
        A3 = 2 * s * q;
    double A4 = 0.5 * (A1 + A2);
    double A5 = 0.5 * (A2 + A3);
    _MagCoreConst c;
    getFerriteCoreMagConst(l1,l2,l3,l4,l5,A1,A2,A3,A4,A5,&c);
    double le = c.C1 * c.C1 / c.C2;
    double Ae = c.C1 / c.C2;
    double mu_e = mu / (1 + g * mu / le);
    double ind = 1000 * N * N * mu0 * mu_e / c.C1;
    result->N = le;
    result->sec = Ae;
    result->thd = mu_e;
    return ind;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long findECore_N(double Ind, double A, double B, double C, double D, double E, double F, double g, double b, double mu,
                 bool isEI, bool isRound, _CoilResult *result)
{
    double tmpI = 0;
    unsigned long int N = 0;
    while (tmpI <= Ind){
        N++;
        tmpI = findECore_I(N,A,B,C,D,E,F,g,b,mu,isEI,isRound,result);
    }
    return N;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findUCore_I(double N, double A, double B, double C, double D, double E, double F, double s, double mu, _CoilResult *result)
{
    double l1 = 2 * D;
    double l3 = 2 * D;
    double h = B - D;
    double q = C;
    double l2 = 2 * E;
    double p = 0;
    double y = 0;
    double A1 = 0;
    double A3 = 0;
    if (F <= 0){
        p = (A - E) / 2;
        y = p;
    } else {
        p = A - E - F;
        y = F;
    }
    double l4 = M_PI * (p + h) / 4;
    double l5 = M_PI * (y + h) / 4;
    if (F == 0)
        A1 = q * p;
    else if (F < 0)
        A1 = 0.25 * M_PI * q * q - 0.25 * M_PI * s * s;
    else if (F > 0)
        A1 = 0.25 * M_PI * q * q;
    double A2 = q * h;
    if (F == 0)
        A3 = q * p;
    else if (F < 0)
        A3 = 0.25 * M_PI * q * q - 0.25 * M_PI * s * s;
    else if (F > 0)
        A3 = q * y - 0.25 * M_PI * s * s;
    double A4 = 0.5 * (A1 + A2);
    double A5 = 0.5 * (A2 + A3);
    _MagCoreConst c;
    getFerriteCoreMagConst(l1,l2,l3,l4,l5,A1,A2,A3,A4,A5,&c);
    double le = c.C1 * c.C1 / c.C2;
    double Ae = c.C1 / c.C2;
    double ind = 1000 * N * N * mu0 * mu / c.C1;
    result->N = le;
    result->sec = Ae;
    return ind;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long findUCore_N(double Ind, double A, double B, double C, double D, double E, double F, double s, double mu, _CoilResult *result)
{
    double tmpI = 0;
    unsigned long int N = 0;
    while (tmpI <= Ind){
        N++;
        tmpI = findUCore_I(N,A,B,C,D,E,F,s,mu,result);
    }
    return N;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void findBrooksCoil(double I, double d, double pa, double pr,
                    double &N, double &nLayer, double &Nc, double &c, double &lengthWire, double &massWire, double &DCR ){
    const double a = 0.0025491;
    double _I = 0;
    double ha = pa * d;
    double hr = pr * d;
    N = 0;
    do{
        N += 0.01;
        double discr = pow(ha + d, 2) - 4 * (ha * d - hr * ha * N);
        c = 0.5 * (ha + d + sqrt(discr));
        _I = a * c * N * N;
    } while (I > _I);
    Nc = (c / (ha * d) - 1);    //number of turns in the layer
    nLayer = ceil(N / Nc);      //number of layers
    // Calculation of the wire length in all layers except the last
    if (nLayer > 1) {
        for (int j = 0;j <= nLayer - 2; j++) {
            lengthWire += sqrt( ha * ha + pow( M_PI * (2 * c + d + 2 * hr * j), 2));
        }
    } else
        lengthWire = 0;
    lengthWire = Nc * lengthWire;
    // Calculating the wire length in the last layer
    double lengthWireLastLayer = (sqrt(ha * ha +pow( M_PI * (2 * c + d + 2 * hr * (nLayer - 1)), 2))) * (N - Nc * (nLayer - 1));
    lengthWire += lengthWireLastLayer;
    lengthWire = lengthWire / 1000;
    DCR = (175 * lengthWire * 1E-4 * 4) / (M_PI * d * d);   //Resistance of the wire to DC [Ohm]
    massWire = 2.225 * M_PI * d * d * lengthWire;           //Weight of the wire [g]
}
