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
                                     double _arg5, double _arg6, double _arg7, double _arg8, Material _mt)
{
    this->coilForm = _coilForm;
    this->tab = _tab;
    this->arg1 = _arg1;
    this->arg2 = _arg2;
    this->arg3 = _arg3;
    this->arg4 = _arg4;
    this->arg5 = _arg5;
    this->arg6 = _arg6;
    this->arg7 = _arg7;
    this->arg8 = _arg8;
    this->mt = _mt;
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
                    result.N = getOneLayerN_withRoundWire( arg1, arg2, arg3, arg4, &result.sec, arg8 );//number of turns
                    Dk = arg1;
                    dw = arg2;
                } else {
                    result.N = getOneLayerN_byWindingLength(arg1, arg2, arg4, &result, arg8);
                    dw = result.five;
                    arg3 = odCalc(dw);
                    Dk = arg1 + arg3;
                }
                result.thd = find_Cs(arg3, Dk, arg3 * result.N); //self-capacitance
                result.fourth = findSRF(arg3 * result.N, Dk, result.sec);//self-resonance frequency
                result.six = solve_Qr(arg4, Dk, arg3, dw, arg5, result.fourth, result.N, result.thd, mt, &result);//Q-factor
                break;
            }
            case _Onelayer_p:{
                //arg: Dk, w, t, p, I, f, 0, mt
                result.N = getOneLayerN_withRectWire( arg1, arg2, arg3, arg4, arg5, &result.sec, arg8 );//number of turns
                result.thd = find_Cs(arg4, arg1, arg4 * result.N); //self-capacitance
                result.fourth = findSRF(arg4 * result.N, arg1, result.sec);//self-resonance frequency
                result.six = solve_Qc(arg5,arg1,arg4,arg2, arg3, arg6, result.fourth, result.N, result.thd, mt, &result);//Q-factor
                break;
            }
            case _Onelayer_q:{
                //arg: I, Dk, d, p, f, _n, 0, mt
                //double I, double Dk, double dw, double  p, double n, _CoilResult *result, unsigned int accuracy
                result.N = getOneLayerN_Poligonal( arg1, arg2, arg3, arg4, arg6, &result, arg8 );//number of turns
                result.fourth = find_Cs(arg4, result.seven, arg4 * result.N); //self-capacitance
                result.five = findSRF(arg4 * result.N, result.seven, result.thd);//self-resonance frequency
                result.six = solve_Qr(arg1, result.seven, arg4, arg3, arg5, result.five, result.N, result.fourth, mt, &result);//Q-factor
                break;
            }
            case _Multilayer:{
                //arg: I, D, d, k, lk
                bool isOrthocyclic = !!arg8;
                getMultiLayerN(arg1, arg2, arg3, arg4, arg5, 0, -1, &result, isOrthocyclic);
                break;
            }
            case _Multilayer_p:{
                //arg: I, D, d, k, lk, gap, ng
                getMultiLayerN(arg1, arg2, arg3, arg4, arg5, arg6, arg7, &result, false);
                break;
            }
            case _Multilayer_r:{
                //arg: I, a, b, l, d, k
                getMultiLayerN_rectFormer(arg1, arg2, arg3, arg4, arg5, arg6, &result);
                break;
            }
            case _Multilayer_f:{
                //arg: Dk, w, t, ins, I, 0, 0, 0
                getMultilayerN_Foil( arg1, arg2, arg3, arg4, arg5, &result );//number of turns
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
                    getPCB_N(arg1, arg2, arg3, arg4, layoutPCB, &result);
                else
                    getPCB_RectN(arg1, arg2, arg3, arg8, arg6, arg4, &result);
                if ((result.sec != 0) && (result.thd != 0))
                    result.fourth = solve_Qpcb(result.N, arg1, arg2, arg3, result.thd, arg6, result.sec, arg7, layoutPCB);
                break;
            }
            case _Flat_Spiral:{
                //arg:I, Di, d, s
                getSpiralN(arg1, arg2, arg3, arg4, &result);
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
                result.N = getOneLayerI_withRoundWire( arg1, arg2, arg3, arg4, &result.sec, arg8 );//number of turns
                result.thd = find_Cs(arg3, arg1, arg3 * arg4); //self-capacitance
                result.fourth = findSRF(arg3 * arg4, arg1, result.sec);//self-resonance frequency
                result.six = solve_Qr(result.N,arg1, arg3, arg2, arg5, result.fourth, arg4, result.thd, mt, &result);//Q-factor
                break;
            }
            case _Onelayer_p:{
                //arg:Dk, w, t, p, N, f, 0, mt
                result.N = getOneLayerI_withRectWire( arg1, arg2, arg3, arg4, arg5, &result.sec, arg8 );//number of turns
                result.thd = find_Cs(arg4, arg1, arg4 * arg5); //self-capacitance
                result.fourth = findSRF(arg4 * arg5, arg1, result.sec);//self-resonance frequency
                result.six = solve_Qc(result.N, arg1, arg4, arg2, arg3, arg6, result.fourth, arg5, result.thd, mt, &result);//Q-factor
                break;
            }
            case _Onelayer_q:{
                //arg: Dk, d, p, N, f, _n, 0, mt
                //double Dk, double dw, double p, double N, double n, _CoilResult *result, unsigned int accuracy
                getOneLayerI_Poligonal( arg1, arg2, arg3, arg4, arg6, &result, arg8);
                result.fourth = find_Cs(arg3, result.seven, arg3 * arg4); //self-capacitance
                result.five = findSRF(arg4 * arg3, arg1, result.thd);//self-resonance frequency
                result.six = solve_Qr(result.sec, result.seven, arg3, arg2, arg5, result.five, arg4, result.fourth, mt, &result);//Q-factor
                break;
            }
            case _Multilayer:{
                //arg: D, l, c, d, k, 0, 0; D, l, c, Rm, k, 0, 1;
                bool isOrthocyclic = !!arg8;
                if (arg7 == 0){
                    getMultiLayerI_byN(arg1, arg2, arg4, arg5, arg3, &result, isOrthocyclic);
                } else if (arg7 == 1){
                    getMultiLayerI(arg1, arg2, arg4, arg5, arg3, 0, -1, &result, isOrthocyclic);
                } else if (arg7 == 2){
                    getMultiLayerI_fromResistance(arg1, arg2, arg3, arg5, arg4, &result, isOrthocyclic);
                }
                break;
            }
            case _Multilayer_p:{
                //arg: D, l, c, d, k, g, Ng
                //double D, double lk, double dw, double k, double b, double gap, long Ng, _CoilResult *result
                getMultiLayerI(arg1, arg2, arg4, arg5, arg3, arg6, arg7, &result, false);
                break;
            }
            case _Multilayer_r:{
                //arg: a, b, l, c, d, k
                //double a, double b, double l, double c, double dw, double k, _CoilResult *result
                if (arg7 == 0){
                    getMultiLayerI_rectFormer_byN(arg4, arg1, arg2, arg3, arg5, arg6, &result);
                } else if (arg7 == 1){
                    getMultiLayerI_rectFormer(arg1, arg2, arg3, arg4, arg5, arg6, &result);
                }
                break;
            }
            case _Multilayer_f:{
                //arg: D, w, t, ins, N
                getMultilayerI_Foil(arg1, arg2, arg3, arg4, arg5, &result);
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
                result.fourth = solve_Qpcb(arg1, result.N, arg2, arg3, arg5, arg7, arg4, arg8, layoutPCB);
                break;
            }
            case _Flat_Spiral:{
                //arg: N, D1, D2, d, 0, 0, 0
                getSpiralI(arg2, arg3, arg4, arg1, &result);
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
