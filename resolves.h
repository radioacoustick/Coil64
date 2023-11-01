/* resolves.h - header text to Coil64 - Radio frequency inductor and choke calculator
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

#ifndef RESOLVES_H
#define RESOLVES_H

#include <QObject>
#include <QString>
#include <QtMath>
#include <QVector>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "definitions.h"

enum _FormCoil
{
    _Onelayer_cw = 0, //close-winding one-layer coil with round wire on a round former
    _Onelayer, //pitch-winding one-layer coil with round wire on a round former
    _Onelayer_p,   //one-layer coil with rect wire on a round former
    _Onelayer_q,   //one-layer coil with round wire on a poligonal former
    _Multilayer,   //multilayer coil with round wire on a round former
    _Multilayer_p, //multilayer coil with round wire on a round former with insulating pads
    _Multilayer_r, //multilayer coil with round wire on a rect former
    _Multilayer_f,  //multilayer coil with foil winding
    _FerrToroid,   //ferrite toroid coil
    _PCB_coil,   //PCB coil with spiral round and square turns
    _Flat_Spiral   //Flat spiral Tesla coil
};

#pragma pack(push,1)
    struct _CoilResult{
      double N;
      double sec;
      double thd;
      double fourth;
      double five;
      unsigned long int six;
      double seven;
    };
#pragma pack(pop)
#pragma pack(push,1)
    struct _Data{
      double N;
      double inductance;
      double capacitance;
      double frequency;
      double B;
      double D;
      double a;
      double b;
      double d;
      double k;
      double p;
      double w;
      double t;
      double isol;
      double l;
      double c;
      double Ch;
      double g;
      double Ng;
      double Do;
      double Di;
      double th;
      double h;
      double mu;
      double ratio;
      double zo;
      double s;
      double Rdc;
      int ns;
    };
#pragma pack(pop)


    enum Material {Al, Cu, Ag, Ti};
    enum _Mprop {Rho,Chi,Alpha,Dencity};

    const double mu0 = 4e-7 * M_PI;
    const double dens = 0.5 * sqrt(3.0);
    double const mtrl[4][4] = {{ 2.824e-8, 2.21e-5, 0.0039, 2.69808 }, { 1.7241e-8, - 9.56e-6, 0.00393, 8.96 },
                               { 1.59e-8, - 2.63e-5, 0.0038, 10.5 }, {1.15e-7, 2.4e-6, 0.0042, 7.29}};
    double const E24[25] = {1.0,1.1,1.2,1.3,1.5,1.6,1.8,2.0,2.2,2.4,2.7,3.0,3.3,3.6,3.9,4.3,4.7,5.1,5.6,6.2,6.8,7.5,8.2,9.1,10.0};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    double getOneLayerN_withRoundWire(double Dk, double dw, double p, double I, double *lw, unsigned int accuracy); //get Number of turns for One-layer coil with round wire
    double getOneLayerN_byWindingLength( double D, double L, double I, _CoilResult *result, unsigned int accuracy); //get Inductance and wire diameter for One-layer coil by winding length
    double getOneLayerN_withRectWire(double Dk, double w, double t, double p, double I, double *lw, unsigned int accuracy); //get Number of turns for One-layer coil with round wire
    double getOneLayerN_Poligonal(double I, double D, double dw, double  h, double n, _CoilResult *result, unsigned int accuracy);
    void getMultiLayerN(double I, double D, double dw, double k, double lk, double gap, long Ng, _CoilResult *result, bool isOrthocyclic); //get Number of turns for Multi-layer coil
    void getMultiLayerN_rectFormer(double Ind, double a, double b, double l, double dw, double k, _CoilResult *result);
    void getMultilayerN_Foil(double Dk, double w, double t, double ins, double I, _CoilResult *result);
    void getFerriteN(double L, double Do, double Di, double h, double dw, double mu, double Ch, _CoilResult *result); //get Number of turns for Ferrite toroid coil
    void getPCB_N (double I, double D, double d, double ratio, int layout, _CoilResult *result);
    void getPCB_RectN (double I, double A, double B, double a, double th, double ratio, _CoilResult *result);
    void getSpiralN(double I, double Di, double dw, double s, _CoilResult *result);

    double getOneLayerI_withRoundWire(double Dk, double dw, double p, double N, double *lw, unsigned int accuracy); //get Inductance for One-layer coil with round wire
    double getOneLayerI_withRectWire(double Dk, double w, double t, double p, double N, double *lw, unsigned int accuracy); //get Inductance for One-layer coil with round wire
    void getOneLayerI_Poligonal(double Dk, double dw, double h, double N, double n, _CoilResult *result, unsigned int accuracy);
    void getMultiLayerI_byN(double D, double lk, double dw, double k, double N, _CoilResult *result, bool isOrthocyclic); //get Inductance for Multi-layer coil
    void getMultiLayerI(double D, double lk, double dw, double k, double c, double gap, long Ng, _CoilResult *result, bool isOrthocyclic); //get Inductance for Multi-layer coil
    void  getMultiLayerI_fromResistance (double D, double lk, double c, double k, double Rm, _CoilResult *result, bool isOrthocyclic);
    void getMultiLayerI_rectFormer(double a, double b, double l, double c, double dw, double k, _CoilResult *result);
    void getMultiLayerI_rectFormer_byN(double N, double a, double b, double l, double dw, double k, _CoilResult *result);
    void getMultilayerI_Foil(double D, double w, double t, double ins, int _N, _CoilResult *result);
    double getFerriteI(double N, double Do, double Di, double h, double mu, double Ch, _CoilResult *result); //get Inductance for Ferrite toroid coil
    double getPCB_I(double N, double _d, double _s, int layout, _CoilResult *result);
    double getPCB_RectI(int N, double A, double B, double s, double w, double th, _CoilResult *result);
    void getSpiralI(double Do, double Di, double dw, int _N, _CoilResult *result);

    void findToroidPemeability(double N, double I, double Do, double Di, double h, double Ch, _CoilResult *result);
    void findFerriteRodN(double I, double Lr, double Dr, double mu, double dc, double s, double dw, double p, _CoilResult *result);
    void findMeadrPCB_I(double a, double d, double h, double W, int N, _CoilResult *result);

    double findMultiloop_I(double N, double Di, double dw, double dt, _CoilResult *result);
    double findMultiloop_N(double I, double Di, double dw, double dt, _CoilResult *result);

    double findRoundLoop_I(double D, double dw);
    double findAirCoreRoundToroid_I(double N, double D1, double D2, double dw);
    double findRoundLoop_D(double Ind, double dw);
    double findIsoIsoscelesTriangleLoop_I(double _a, double _b, double dw);
    double findIsoIsoscelesTriangleLoop_a(double Ind, double dw);
    double findRectangleLoop_I(double _a, double _b, double dw);
    double findRectangleLoop_a(double Ind, double dw);
    double findSheildedInductance(double I, double D, double Ds, double l, double Hs);
    double findAirCoreRoundToroid_I(double N, double D1, double D2, double dw);
    double findAirCoreRoundToroid_N(double Ind, double D1, double D2, double dw);
    double findPotCore_I(double N, double D1, double D2, double D3, double D4, double h1, double h2, double g, double b, double mu, _CoilResult *result);
    long findPotCore_N(double Ind, double D1, double D2, double D3, double D4, double h1, double h2, double g, double b, double mu, _CoilResult *result);
    double findECore_I(double N, double A, double B, double C, double D, double E, double F, double g, double b, double mu, bool isEI, bool isRound, _CoilResult *result);
    long findECore_N(double Ind, double A, double B, double C, double D, double E, double F, double g, double b, double mu, bool isEI, bool isRound, _CoilResult *result);
    double findUCore_I(double N, double A, double B, double C, double D, double E, double F, double s, double mu, _CoilResult *result);
    long findUCore_N(double Ind, double A, double B, double C, double D, double E, double F, double s, double mu, _CoilResult *result);
    void findBrooksCoil(double I, double d, double pa, double pr, double &N, double &nLayer, double &Nc, double &c, double &lengthWire, double &massWire, double &DCR );
    double findRMCore_I (double N, double a, double b, double c, double e, double d2, double d3, double d4, double h1,  double h2, double g, double mu, int type, _CoilResult *result);
    long findRMCore_N (double Ind, double a1, double a, double B, double C, double D1, double D2, double D3, double H1,  double H2, double g, double mu, int type, _CoilResult *result);

    double CalcLC0(double L, double C);
    double CalcLC1(double C, double f);
    double CalcLC2(double L, double f);
    void CalcLC3(double Zo, double f, _CoilResult *result);

    double odCalc(double id);
    double toNearestE24(double val, int accurasy);
    double find_actual_spiral_length(int N, double Din, double k);
    double convertfromAWG (QString AWG, bool *isOK = NULL);
    QString converttoAWG (double d, bool *isOK = NULL);
    double findPadderCapacitance(double Ct,double Cv_low,double Cv_high,double Cstray,double cap_ratio);
    double findTrimmerCapacitance(double Cp,double Cv_low,double Cv_high,double Cstray,double cap_ratio);


#endif // RESOLVES_H
