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
                                     double _arg5, double _arg6, double _arg7, Material _mt)
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
    this->mt = _mt;
    qRegisterMetaType<_CoilResult>();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MThread_calculate::run(){
    _CoilResult result;
    try{
        switch (this->tab) {
        case 0:{
            switch (this->coilForm) {
            case _Onelayer_cw:
            case _Onelayer:{
                //arg: Dk, d, p, I, f, 0, 0, mt
                result.N = getOneLayerN_withRoundWire( arg1, arg2, arg3, arg4, &result.sec, arg7 );//number of turns
                result.thd = find_Cs(arg3, arg1, arg3 * result.N); //self-capacitance
                result.six = solve_Qr(arg4,arg1,arg3,arg2,arg5, result.N, result.thd, mt);//Q-factor
                result.fourth = findSRF(arg3 * result.N, arg1, result.sec);//self-resonance frequency
                break;
            }
            case _Onelayer_p:{
                //arg: Dk, w, t, p, I, f, 0, mt
                result.N = getOneLayerN_withRectWire( arg1, arg2, arg3, arg4, arg5, &result.sec, arg7 );//number of turns
                result.thd = find_Cs(arg4, arg1, arg4 * result.N); //self-capacitance
                result.six = solve_Qc(arg5,arg1,arg4,arg2, arg3, arg6, result.N, result.thd, mt);//Q-factor
                result.fourth = findSRF(arg4 * result.N, arg1, result.sec);//self-resonance frequency
                break;
            }
            case _Onelayer_q:{
                //arg: I, Dk, d, p, f, _n, 0, mt
                result.N = getOneLayerN_Poligonal( arg1, arg2, arg3, arg4, arg6, &result, arg7 );//number of turns
                result.fourth = find_Cs(arg4, arg2, arg4 * result.N); //self-capacitance
                result.six = solve_Qr(arg1, arg2, arg4, arg3, arg5, result.N, result.fourth, mt);//Q-factor
                result.five = findSRF(arg4 * result.N, arg2, result.thd);//self-resonance frequency
                break;
            }
            case _Multilayer:{
                //arg: I, D, d, k, lk
                getMultiLayerN(arg1, arg2, arg3, arg4, arg5, 0, -1, &result);
                break;
            }
            case _Multilayer_p:{
                //arg: I, D, d, k, lk, gap, ng
                getMultiLayerN(arg1, arg2, arg3, arg4, arg5, arg6, arg7, &result);
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
                //arg:I, D1, D2, h, d, mu
                getFerriteN(arg1, arg2, arg3, arg4, arg5, arg6, &result);
                break;
            }
            case _PCB_coil:{
                //arg:I, D1, D2, ratio, isPCBSquare
                if (arg5 == 0)
                    getPCB_N(arg1, arg2, arg3, arg4, &result);
                else
                    getSpiralPCB_N(arg2, arg3, arg4, arg1, &result);
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
                result.N = getOneLayerI_withRoundWire( arg1, arg2, arg3, arg4, &result.sec, arg7 );//number of turns
                result.thd = find_Cs(arg3, arg1, arg3 * arg4); //self-capacitance
                result.six = solve_Qr(result.N,arg1, arg3, arg2, arg5, arg4, result.thd, mt);//Q-factor
                result.fourth = findSRF(arg3 * arg4, arg1, result.sec);//self-resonance frequency
                break;
            }
            case _Onelayer_p:{
                //arg:Dk, w, t, p, N, f, 0, mt
                result.N = getOneLayerI_withRectWire( arg1, arg2, arg3, arg4, arg5, &result.sec, arg7 );//number of turns
                result.thd = find_Cs(arg4, arg1, arg4 * arg5); //self-capacitance
                result.six = solve_Qc(result.N, arg1, arg4, arg2, arg3, arg6, arg5, result.thd, mt);//Q-factor
                result.fourth = findSRF(arg4 * arg5, arg1, result.sec);//self-resonance frequency
                break;
            }
            case _Onelayer_q:{
                //arg: Dk, d, p, N, f, _n, 0, mt
                getOneLayerI_Poligonal( arg1, arg2, arg3, arg4, arg6, &result, arg7);
                result.fourth = find_Cs(arg3, arg1, arg3 * arg4); //self-capacitance
                result.six = solve_Qr(result.sec, arg1, arg3, arg2, arg5, arg4, result.fourth, mt);//Q-factor
                result.five = findSRF(arg3 * arg4, arg1, result.thd);//self-resonance frequency
                break;
            }
            case _Multilayer:{
                //arg: D, l, c, d, k, 0, 0; D, l, c, Rm, k, 0, 1;
                if (arg7 == 0){
                    getMultiLayerI_byN(arg1, arg2, arg4, arg5, arg3, &result);
                } else if (arg7 == 1){
                    getMultiLayerI(arg1, arg2, arg4, arg5, arg3, 0, -1, &result);
                } else if (arg7 == 2){
                    getMultiLayerI_fromResistance(arg1, arg2, arg3, arg5, arg4, &result);
                }
                break;
            }
            case _Multilayer_p:{
                //arg: D, l, c, d, k, g, Ng
                //double D, double lk, double dw, double k, double b, double gap, long Ng, _CoilResult *result
                getMultiLayerI(arg1, arg2, arg4, arg5, arg3, arg6, arg7, &result);
                break;
            }
            case _Multilayer_r:{
                //arg: a, b, l, c, d, k
                //double a, double b, double l, double c, double dw, double k, _CoilResult *result
                getMultiLayerI_rectFormer(arg1, arg2, arg3, arg4, arg5, arg6, &result);
                break;
            }
            case _Multilayer_f:{
                //arg: D, w, t, ins, N
                getMultilayerI_Foil(arg1, arg2, arg3, arg4, arg5, &result);
                break;
            }
            case _FerrToroid:{
                //arg: N, D1, D2, h, mu, 0, 0
                result.N = getFerriteI(arg1, arg2, arg3, arg4, arg5);
                break;
            }
            case _PCB_coil:{
                //arg: N, D1, D2, s, W, isPCBSquare, 0
                if (arg6 == 0)
                    result.N = getPCB_I(arg1, arg2, arg3, arg4, arg5);
                else
                    result.N = getSpiralPCB_I(arg2, arg3, arg1);
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
