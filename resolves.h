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

#include <QString>
#include <QtMath>

enum _FormCoil
{
    _Onelayer = 0,
    _Onelayer_p,
    _Multilayer,
    _Multilayer_p,
    _FerrToroid,
    _PCB_square,
    _Flat_Spiral
};

#pragma pack(push,1)
    struct _CoilResult{
      double N;
      double sec;
      double thd;
      double fourth;
      double five;
      unsigned long int six;
    };
#pragma pack(pop)
#pragma pack(push,1)
    struct _Data{
      double N;
      double inductance;
      double capacitance;
      double frequency;
      double D;
      double d;
      double k;
      double p;
      double w;
      double t;
      double isol;
      double l;
      double c;
      double g;
      double Ng;
      double Do;
      double Di;
      double h;
      double mu;
      double ratio;
      double s;
      double Rdc;
    };
#pragma pack(pop)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    double getOneLayerN_withRoundWire(double Dk, double dw, double p, double I, double *lw); //get Number of turns for One-layer coil with round wire
    double getOneLayerN_withRectWire(double Dk, double w, double t, double p, double I, double *lw); //get Number of turns for One-layer coil with round wire
    void getMultiLayerN(double I, double D, double dw, double k, double lk, double gap, long Ng, _CoilResult *result); //get Number of turns for Multi-layer coil
    void getFerriteN(double L, double Do, double Di, double h, double dw, double mu, _CoilResult *result); //get Number of turns for Ferrite toroid coil
    void getPCB_N (double I, double D, double d, double ratio, _CoilResult *result);
    void getSpiralN(double I, double Di, double dw, double s, _CoilResult *result);

    double getOneLayerI_withRoundWire(double Dk, double dw, double p, double N, double *lw); //get Inductance for One-layer coil with round wire
    double getOneLayerI_withRectWire(double Dk, double w, double t, double p, double N, double *lw); //get Inductance for One-layer coil with round wire
    void getMultiLayerI(double D, double lk, double dw, double k, double b, double gap, long Ng, _CoilResult *result); //get Inductance for Multi-layer coil
    void  getMultiLayerI_fromResistance (double D, double lk, double b, double k, double Rm, _CoilResult *result);
    double getFerriteI(double N, double Do, double Di, double h, double mu); //get Inductance for Ferrite toroid coil
    double getPCB_I(double N, double D, double d, double s, double W);
    void getSpiralI(double Do, double Di, double dw, int _N, _CoilResult *result);

    void findToroidPemeability(double N, double I, double Do, double Di, double h, _CoilResult *result);
    void findFerriteRodN(double I, double Lr, double Dr, double mu, double dc, double s, double dw, double p, _CoilResult *result);


    double CalcCONTUR0(double L, double C);
    double CalcCONTUR1(double C, double f);
    double CalcCONTUR2(double L, double f);

    double odCalc(double id);

#endif // RESOLVES_H
