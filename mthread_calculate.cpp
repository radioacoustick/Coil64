/* mthread_calculate.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "mthread_calculate.h"

MThread_calculate::MThread_calculate(int _coilForm, int _tab, double _arg1, double _arg2, double _arg3, double _arg4,
                                     double _arg5, double _arg6, double _arg7, double _arg8, Material _mt, double _tand,
                                     double _arg9, double _arg10, double _arg11, double _arg12, bool st1, bool st2, int _index)
{
    this->coilForm = _coilForm;
    this->tab = _tab;
    this->index = _index;
    this->arg1 = _arg1;
    this->arg2 = _arg2;
    this->arg3 = _arg3;
    this->arg4 = _arg4;
    this->arg5 = _arg5;
    this->arg6 = _arg6;
    this->arg7 = _arg7;
    this->arg8 = _arg8;
    this->arg9 = _arg9;
    this->arg10 = _arg10;
    this->arg11 = _arg11;
    this->arg12 = _arg12;
    this->tand = _tand;
    this->mt = _mt;
    this->isAbort = false;
    this->st1 = st1;
    this->st2 = st2;
    qRegisterMetaType<_CoilResult>();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MThread_calculate::run(){
    _CoilResult result = {0,0,0,0,0,0,0};
    try{
        switch (this->tab) {
        case 0:{
            switch (this->coilForm) {
            case _Onelayer_cw:
            case _Onelayer:{
                //arg: Dk, d, k, I, f, 0, 0, myOpt->dwAccuracy, mt
                double Dk = 0;
                double dw = 0;
                if (arg3 > 0){ //when winding length option is activated, k=0
                    result.N = getOneLayerN_withRoundWire( arg1, arg2, arg3, arg4, &result.sec, arg8, &isAbort);//number of turns
                    Dk = arg1;
                    dw = arg2;
                } else {
                    result.N = getOneLayerN_byWindingLength(arg1, arg2, arg4, &result, arg8, &isAbort);
                    dw = result.five;
                    arg3 = odCalc(dw);
                    Dk = arg1 + arg3;
                }
                if(result.N > 0){
                    result.thd = find_Cs(arg3, Dk, arg3 * result.N); //self-capacitance
                    result.fourth = findSRF(arg3 * result.N, Dk, result.sec);//self-resonance frequency
                    result.six = solve_Qr(arg4, Dk, arg3, dw, arg5, result.fourth, result.N, result.thd, mt, &result);//Q-factor
                }
                break;
            }
            case _Onelayer_p:{
                //arg: Dk, w, t, p, I, f, 0, mt
                result.N = getOneLayerN_withRectWire( arg1, arg2, arg3, arg4, arg5, &result.sec, arg8, &isAbort);//number of turns
                if(result.N > 0){
                    result.thd = find_Cs(arg4, arg1, arg4 * result.N); //self-capacitance
                    result.fourth = findSRF(arg4 * result.N, arg1, result.sec);//self-resonance frequency
                    result.six = solve_Qc(arg5,arg1,arg4,arg2, arg3, arg6, result.fourth, result.N, result.thd, mt, &result);//Q-factor
                }
                break;
            }
            case _Onelayer_q:{
                //arg: I, Dk, d, p, f, _n, 0, mt
                //double I, double Dk, double dw, double  p, double n, _CoilResult *result, unsigned int accuracy
                result.N = getOneLayerN_Poligonal( arg1, arg2, arg3, arg4, arg6, &result, arg8, &isAbort);//number of turns
                if(result.N > 0){
                    result.fourth = find_Cs(arg4, result.seven, arg4 * result.N); //self-capacitance
                    result.five = findSRF(arg4 * result.N, result.seven, result.thd);//self-resonance frequency
                    result.six = solve_Qr(arg1, result.seven, arg4, arg3, arg5, result.five, result.N, result.fourth, mt, &result);//Q-factor
                }
                break;
            }
            case _Multilayer:{
                //arg: I, D, d, k, lk
                bool isOrthocyclic = !!arg8;
                getMultiLayerN(arg1, arg2, arg3, arg4, arg5, 0, -1, &result, isOrthocyclic, &isAbort);
                break;
            }
            case _Multilayer_p:{
                //arg: I, D, d, k, lk, gap, ng
                getMultiLayerN(arg1, arg2, arg3, arg4, arg5, arg6, arg7, &result, false, &isAbort);
                break;
            }
            case _Multilayer_r:{
                //arg: I, a, b, l, d, k
                getMultiLayerN_rectFormer(arg1, arg2, arg3, arg4, arg5, arg6, &result, &isAbort);
                break;
            }
            case _Multilayer_f:{
                //arg: Dk, w, t, ins, I, 0, 0, 0
                getMultilayerN_Foil( arg1, arg2, arg3, arg4, arg5, &result, &isAbort);//number of turns
                break;
            }
            case _FerrToroid:{
                //arg:I, D1, D2, h, d, mu, Ch
                getFerriteN(arg1, arg2, arg3, arg4, arg5, arg6, arg7, &result);
                break;
            }
            case _PCB_coil:{
                //arg: I, D1, D2, ratio, layoutPCB, th, f, a
                int layoutPCB = round(arg5);
                if (layoutPCB != 2)
                    getPCB_N(arg1, arg2, arg3, arg4, layoutPCB, &result, &isAbort);
                else
                    getPCB_RectN(arg1, arg2, arg3, arg8, arg6, arg4, &result, &isAbort);
                if ((result.sec != 0) && (result.thd != 0))
                    result.fourth = solve_Qpcb(result.N, arg1, arg2, arg3, result.thd, arg6, result.sec, arg7, tand, layoutPCB);
                break;
            }
            case _Flat_Spiral:{
                //arg:I, Di, d, s
                getSpiralN(arg1, arg2, arg3, arg4, &result, &isAbort);
                break;
            }
            default:
                break;
            }
            break;
        }
        case 1:{
            switch (this->coilForm) {
            case _Onelayer_cw:
            case _Onelayer:{
                //arg: Dk, d, p, N, f, 0, 0, mt
                result.N = getOneLayerI_withRoundWire( arg1, arg2, arg3, arg4, &result.sec, arg8, &isAbort);//inductance
                if(result.N > 0){
                    result.thd = find_Cs(arg3, arg1, arg3 * arg4); //self-capacitance
                    result.fourth = findSRF(arg3 * arg4, arg1, result.sec);//self-resonance frequency
                    result.six = solve_Qr(result.N,arg1, arg3, arg2, arg5, result.fourth, arg4, result.thd, mt, &result);//Q-factor
                }
                break;
            }
            case _Onelayer_p:{
                //arg:Dk, w, t, p, N, f, 0, mt
                result.N = getOneLayerI_withRectWire( arg1, arg2, arg3, arg4, arg5, &result.sec, arg8, &isAbort);//number of turns
                if(result.N > 0){
                    result.thd = find_Cs(arg4, arg1, arg4 * arg5); //self-capacitance
                    result.fourth = findSRF(arg4 * arg5, arg1, result.sec);//self-resonance frequency
                    result.six = solve_Qc(result.N, arg1, arg4, arg2, arg3, arg6, result.fourth, arg5, result.thd, mt, &result);//Q-factor
                }
                break;
            }
            case _Onelayer_q:{
                //arg: Dk, d, p, N, f, _n, 0, mt
                //double Dk, double dw, double p, double N, double n, _CoilResult *result, unsigned int accuracy
                getOneLayerI_Poligonal( arg1, arg2, arg3, arg4, arg6, &result, arg8, &isAbort);
                if(result.sec > 0){
                    result.fourth = find_Cs(arg3, result.seven, arg3 * arg4); //self-capacitance
                    result.five = findSRF(arg4 * arg3, arg1, result.thd);//self-resonance frequency
                    result.six = solve_Qr(result.sec, result.seven, arg3, arg2, arg5, result.five, arg4, result.fourth, mt, &result);//Q-factor
                }
                break;
            }
            case _Multilayer:{
                //arg: D, l, c, d, k, 0, 0; D, l, c, Rm, k, 0, 1;
                bool isOrthocyclic = !!arg8;
                if (arg7 == 0){
                    getMultiLayerI_byN(arg1, arg2, arg4, arg5, arg3, &result, isOrthocyclic, &isAbort);
                } else if (arg7 == 1){
                    getMultiLayerI(arg1, arg2, arg4, arg5, arg3, 0, -1, &result, isOrthocyclic, &isAbort);
                } else if (arg7 == 2){
                    getMultiLayerI_fromResistance(arg1, arg2, arg3, arg5, arg4, &result, isOrthocyclic, &isAbort);
                }
                break;
            }
            case _Multilayer_p:{
                //arg: D, l, c, d, k, g, Ng
                //double D, double lk, double dw, double k, double b, double gap, long Ng, _CoilResult *result
                getMultiLayerI(arg1, arg2, arg4, arg5, arg3, arg6, arg7, &result, false, &isAbort);
                break;
            }
            case _Multilayer_r:{
                //arg: a, b, l, c, d, k
                //double a, double b, double l, double c, double dw, double k, _CoilResult *result
                if (arg7 == 0){
                    getMultiLayerI_rectFormer_byN(arg4, arg1, arg2, arg3, arg5, arg6, &result, &isAbort);
                } else if (arg7 == 1){
                    getMultiLayerI_rectFormer(arg1, arg2, arg3, arg4, arg5, arg6, &result, &isAbort);
                }
                break;
            }
            case _Multilayer_f:{
                //arg: D, w, t, ins, N
                getMultilayerI_Foil(arg1, arg2, arg3, arg4, arg5, &result, &isAbort);
                break;
            }
            case _FerrToroid:{
                //arg: N, D1, D2, h, mu, Ch, dw
                result.N = getFerriteI(arg1, arg2, arg3, arg4, arg5, arg6, arg7, &result);
                break;
            }
            case _PCB_coil:{
                //arg: N, 0, D2, s, W, layoutPCB, th, f
                int layoutPCB = round(arg6);
                if (layoutPCB != 2)
                //double N, double _d, double _s, int layout
                    result.N = getPCB_I(arg1, arg3, arg4, layoutPCB, &result);
                else
                    result.N = getPCB_RectI(arg1,arg2,arg3,arg4,arg5,arg7, &result);
                //long N, double _I, double _D, double _d, double _W, double _t, double _s,  double _f, int layout
                result.fourth = solve_Qpcb(arg1, result.N, arg2, arg3, arg5, arg7, arg4, arg8, tand, layoutPCB);
                break;
            }
            case _Flat_Spiral:{
                //arg: N, D1, D2, d, 0, 0, 0
                getSpiralI(arg2, arg3, arg4, arg1, &result, &isAbort);
                break;
            }
            default:
                break;
            }
            break;
        }
        case -1:{
            switch (this->coilForm) {
            case _SingleLoop:{
                if(this->st1){
                    switch (this->index) {
                    case 0:{
                        result.N = findRoundLoop_D(arg1,arg2,arg3);
                        break;
                    }
                    case 1:{
                        result.N = findIsoIsoscelesTriangleLoop_a(arg1,arg2,arg3);
                        break;
                    }
                    case 2:{
                        result.N = findRectangleLoop_a(arg1,arg2,arg3);
                        break;
                    }
                    default:
                        break;
                    }
                } else {
                    switch (this->index) {
                    case 0:{
                        result.N = findRoundLoop_I(arg1,arg3,arg4);
                        break;
                    }
                    case 1:{
                        result.N = findIsoIsoscelesTriangleLoop_I(arg1, arg2, arg3, arg4);
                        break;
                    }
                    case 2:{
                        result.N = findRectangleLoop_I(arg1, arg2, arg3, arg4);
                        break;
                    }
                    default:
                        break;
                    }
                }
                break;
            }
            case _Multiloop:{
                if(arg8 == 0){
                    if(this->arg5 == 0){
                        result.seven = findMultiloopO_I(arg1, arg2, arg4, &result);
                    } else {
                        result.six = findMultiloopO_N(arg1, arg2, arg4, &result);
                    }
                } else if(arg8 == 1){
                     //thread = new MThread_calculate( _Multiloop, -1, Pin, indTx, indRx, 0, dt, 0, dt2, 1);
                    //findMultiLoopD_N(double Ind, double Pin, double dt, _CoilResult *result, bool *isStop)
                    result.N = findMultiLoopD_N(arg2, arg1, arg5, &result, &isAbort);
                    double wlength1 = result.thd;
                    double T1 = result.sec;
                    result.seven = findMultiLoopD_N(arg3, arg1, arg7, &result, &isAbort);
                    double wlength2 = result.thd;
                    double T2 = result.sec;

                    result.sec = T1;
                    result.thd = wlength1;
                    result.fourth = T2;
                    result.five = wlength2;
                }
                break;
            }
            case _Multisection:{
                bool isOrthocyclic = !!arg7;
                getMultiLayerI(arg1, arg3, arg5, arg6, arg2, 0, -1, &result, isOrthocyclic, &isAbort, index, arg4);
                break;
            }
            case _CoupledCoils:{
                if(tand > 0){
                    //D1, D2, dw, l1, l2, N1, N2, fOpt->dwAccuracy
                    //getOneLayerI_withRoundWire(double Dk, double dw, double p, double N, double *lw, unsigned int accuracy, bool *isStop)
                    //double findOneLayerMutualInductance(double D1, double D2, double l1, double l2, double x, double dw, int N1, int N2, bool *isStop)
                    double p1 = arg4 / arg6;
                    double p2 = arg5 / arg7;
                    result.N = getOneLayerI_withRoundWire( arg1, arg3, p1, arg6, &result.seven, arg8, &isAbort);//inductance1
                    result.sec = getOneLayerI_withRoundWire( arg2, arg3, p2, arg7, &result.five, arg8, &isAbort);//inductance2
                    result.thd = findOneLayerMutualInductance(arg1, arg2, arg4, arg5, tand, arg3, arg6, arg7, &isAbort);//mutual inductance
                } else {
                    //MThread_calculate( _CoupledCoils, -1, D, 0, dw, p, 0, N, Nt, fOpt->dwAccuracy, Cu, 0);
                    double lw = 0.0;
                    result.N = getOneLayerI_withRoundWire( arg1, arg3, arg4, arg6, &result.sec, arg8, &isAbort);//total inductance
                    result.thd = getOneLayerI_withRoundWire( arg1, arg3, arg4, arg7, &lw, arg8, &isAbort);//inductance1
                    result.fourth = getOneLayerI_withRoundWire( arg1, arg3, arg4, (arg6 - arg7), &lw, arg8, &isAbort);//inductance2
                }
                break;
            }
            default:
                break;
            }
            break;
        }
        case -2:{
            switch (this->coilForm) {
            case _Rod_Core:{
                findFerriteRodN(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8, &result, &isAbort);
                break;
            }
            case _Pot_Core:{
                if(this->tand == 0){
                    result.six = findPotCore_N(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10, &result);
                } else {
                    result.seven = findPotCore_I(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10, &result);
                }
                break;
            }
            case _E_Core:{
                if(this->tand == 0){
                    result.six = findECore_N(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,st1,st2, &result);
                } else {
                    result.seven = findECore_I(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,st1,st2, &result);
                }
                break;
            }
            case _U_Core:{
                if(this->tand == 0){
                    result.six = findUCore_N(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9, &result);
                } else {
                    result.seven = findUCore_I(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9, &result);
                }
                break;
            }
            case _RM_Core:{
                if(this->tand == 0){
                    result.six = findRMCore_N(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12,index, &result);
                } else {
                    result.seven = findRMCore_I(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12,index, &result);
                }
                break;
            }
            default:
                break;
            }
            break;
        }
        default:
            break;
        }
        emit sendResult(result);
    } catch (...) {
        this->quit();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MThread_calculate::abort()
{
    isAbort = true;
}
