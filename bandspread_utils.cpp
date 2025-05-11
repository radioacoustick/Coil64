/* bandspread_utils.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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
#include "bandspread_utils.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double lsModel(double P[], double f_int, double Cm, double Cv, double fEntr, int tosc, double *freq){
    //calculation of inductance and capacitance of local oscillator band spread capacitors using the frequency grid fitting method
    //without model conversion
    //Cv: capacitance value of the variable condenser at i-th mesh point
    //Cm: circuit stray capacitance
    double r1 = P[1] + Cv;
    double r2 = P[0] + r1;
    double a = P[2] * (Cm + P[0] * r1 / r2);
    *freq = 1e3 / (2.0 * M_PI * sqrt(a)) - tosc * f_int;
    return fEntr - *freq;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double qDer(double p[],double Cm, int j, double Ct){
    double r1 = p[1] + Ct;
    double r2 = p[0] + r1;
    double r3 = -1e3 / (4.0 * M_PI) * pow((p[2] * (Cm + p[0] * r1 / r2)), -1.5);
    double q = 0.0;
    if (j == 0)
        q = r3 * p[2] * pow((r1 / r2), 2);
    if (j == 1)
        q = r3 * p[2] * pow((p[0] / r2), 2);
    if (j == 2)
        q = r3 * (Cm + p[0] * r1 / r2);
    return q;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void inverseMatrix(double **matrA, int N)
//Inverse of a Matrix matrA using Gauss-Jordan Method
{
    double c;
    double **tmpMatrE = new double *[N];
    for (int i = 0; i < N; i++)
        tmpMatrE[i] = new double [N];
    //Create the Identity matrix
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
        {
            tmpMatrE[i][j] = 0.0;

            if (i == j)
                tmpMatrE[i][j] = 1.0;
        }
    for (int k = 0; k < N; k++)
    {
        c = matrA[k][k];
        for (int j = 0; j < N; j++)
        {
            matrA[k][j] /= c;
            tmpMatrE[k][j] /= c;
        }
        for (int i = k + 1; i < N; i++)
        {
            c = matrA[i][k];
            for (int j = 0; j < N; j++)
            {
                matrA[i][j] -= matrA[k][j] * c;
                tmpMatrE[i][j] -= tmpMatrE[k][j] * c;
            }
        }
    }
    for (int k = N - 1; k > 0; k--)
    {
        for (int i = k - 1; i >= 0; i--)
        {
            c = matrA[i][k];
            for (int j = 0; j < N; j++)
            {
                matrA[i][j] -= matrA[k][j] * c;
                tmpMatrE[i][j] -= tmpMatrE[k][j] * c;
            }
        }
    }
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            matrA[i][j] = tmpMatrE[i][j];

    for (int i = 0; i < N; i++)
        delete [] tmpMatrE[i];
    delete [] tmpMatrE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int nlsm(double f_low, double f_high, double f_i, double Cv_low, double Cv_high, double Cstray, double ind, double Cpad_entr, double Ctrim_entr,
                             QVector<double> *data, QVector<double> *conFreq, int accuracy, bool isHighSideInjection, double *maxDelta)
{
    data->clear();
    conFreq->clear();
    const int MESH = 100;
    int tosc = 1;
    int result = 0;
    if (!isHighSideInjection)
        tosc = -1;
    double step = (f_high - f_low) / (MESH - 1);
    QVector<double> *fEntr = new QVector<double>();
    QVector<double> *capCv = new QVector<double>();
    QVector<double> *capTmp = new QVector<double>();
    QVector<double> *trueFreq = new QVector<double>();
    QVector<double> *indRange = new QVector<double>();
    calcCapacitors(f_low, f_high, Cv_low, Cv_high, Cstray, ind, capTmp, accuracy);
    data->push_back(Cpad_entr);
    data->push_back(Ctrim_entr);
    for (int i = 0; i < MESH; i++){
        double fentr = f_low + i * step;
        fEntr->push_back(fentr);
        //Calculating Cv capacitance array;
        double C0entr = CalcLC2(ind, fentr);
        double cv = ((C0entr - Cstray) * (Ctrim_entr + Cpad_entr) - Ctrim_entr * Cpad_entr) / (Ctrim_entr - C0entr + Cstray);
        capCv->push_back(cv);
    }
    double f_osc_max = f_high + tosc * f_i; //Local oscillator max frequency;
    double f_osc_min = f_low + tosc * f_i;  //Local oscillator min frequency;
    //initial Lo estimate by Foscmax (p3):
    double Losc = CalcLC1(capTmp->at(2), f_osc_max); //Local oscillator inductance [uH] (the initial estimate);
    //the initial estimates of C1o, C2o:
    calcCapacitors(f_osc_min, f_osc_max, Cv_low, Cv_high, Cstray, Losc, capTmp, accuracy);
    if (!capTmp->isEmpty()){
        const int ITERATION_LIMIT = 100; //maximum iteration
        const double E1 = 0.001; //accuracy
        const int N = 3; // matrix dimension
        double frequency = 0.0;
        double frac = 1.0;
        double g = 0.0;
        double s1 = 0.0;
        double s2 = 0.0;
        double delta = 0.0;
        double hand = 1.0;
        double weight = 0.0;
        double q = 0.0;
        double Cpad_osc = capTmp->at(0); //Local oscillator padder capacitance [pF] (initial value);
        double Ctrim_osc = capTmp->at(1); //Local oscillator trimmer capacitance [pF] (initial value);
        double **matrA = new double *[N]; //Create Matrix of normal equations
        for (int i = 0; i < N; i++)
            matrA[i] = new double [N];
        double b[3] = {0.0,0.0,0.0}; //auxiliary array
        double v[3] = {0.0,0.0,0.0}; //auxiliary array
        double p[] = {Ctrim_osc, Cpad_osc, Losc}; //initial local oscillator marameters array
        for (int iter = 0; iter <= ITERATION_LIMIT; iter++) {
            //Fill matrix and arrays with zeros
            for(int i = 0; i < N; i++){
                for (int j = 0; j < N; j++){
                    matrA[i][j] = 0.0;
                }
            }
            for(int i = 0; i < N; i++){
                b[i] = 0.0;
            }
            for(int i = 0; i < N; i++){
                v[i] = 0.0;
            }
            s1 = 0.0;
            hand = 1.0;
            for(int i = 0; i < MESH; i++){
                delta = lsModel(p, f_i, Cstray, capCv->at(i), fEntr->at(i), tosc, &frequency);
                weight = 1.0 / fEntr->at(i);
                s1 = s1 + delta * delta * weight;
                for(int k = 0; k < N; k++){
                    q = qDer(p, Cstray, k, capCv->at(i));
                    double c = weight * q;
                    b[k] = b[k] + c * delta;
                    for(int l = k; l < N; l++){
                        q = qDer(p, Cstray, l, capCv->at(i));
                        matrA[k][l] = matrA[k][l] + c * q;
                        matrA[l][k] = matrA[k][l];
                    }
                }
            }
            inverseMatrix(matrA, N);
            //parameter estimates
            for (int j = 0; j < N; j++){
                v[j] = 0;
                for (int k = 0; k < N; k++){
                    v[j] = v[j] + matrA[j][k] * b[k];
                }
            }
            do{
                for (int j = 0; j < N; j++){
                    p[j] = p[j] + hand * frac * v[j];
                }
                s2 = 0.0;
                //weighted least squares
                for (int i = 0; i < MESH; i++){
                    delta = lsModel(p, f_i, Cstray, capCv->at(i), fEntr->at(i), tosc, &frequency);
                    weight = 1.0 / fEntr->at(i);
                    s2 = s2 + delta * delta * weight;
                }
                if (fabs(s1 - s2) < (E1 * s2))
                    break;
                hand = -1.0;
                g = 0.0;
                frac = 0.5 * frac;
            } while (s2 > s1);
            if (fabs(s1 - s2) < (E1 * s2))
                break;
            g++;
            if (g >= 2){
                if (frac <= 0.5)
                    frac = 2 * frac;
            }
            if(iter == ITERATION_LIMIT)
                result = 2;
        }
        if (result != 2){
            if ((std::isnormal(p[0])) && (std::isnormal(p[1])) && (std::isnormal(p[2]))){
                for(int i = 0; i < MESH; i++){
                    lsModel(p, f_i, Cstray, capCv->at(i), fEntr->at(i), tosc, &frequency);
                    trueFreq->push_back(frequency);
                }
                //find zero error tracking points
                *maxDelta = 0.0;
                for (int k = 0; k < MESH - 1; k++){
                    double f1 = fEntr->at(k);
                    double f2 = trueFreq->at(k);
                    double del1 = fEntr->at(k) - trueFreq->at(k);
                    if (fabs(del1) > *maxDelta)
                        *maxDelta = fabs(del1);
                    double del2 = fEntr->at(k + 1) - trueFreq->at(k + 1);
                    if (fabs(del2) > *maxDelta)
                        *maxDelta = fabs(del2);
                    if (!(std::copysign(1, del1) == std::copysign(1, del2))){
                        double f0 = (f2 * fabs(del1) + f1 * fabs(del2)) / (fabs(del1) + fabs(del2));
                        conFreq->push_back(f0);
                    }
                }
                data->push_back(p[0]);
                data->push_back(p[1]);
                data->push_back(p[2]);
            } else {
                result = 3;
            }
        }
        for (int i = 0; i < N; i++)
            delete [] matrA[i];
        delete [] matrA;
    } else {
        result = 1;
    }
    delete fEntr;
    delete capCv;
    delete capTmp;
    delete trueFreq;
    delete indRange;
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int trackingLocalOscillator(double f_low, double f_high, double f_i, double Cv_low, double Cv_high, double Cstray, double *ind,
                            QVector<double> *data, QVector<double> *conFreq, int accuracy, bool isHighSideInjection, bool isAutomaticLe, double *maxDelta){
    QVector<double> *range = new QVector<double>();
    int result = 0;
    calcInductanceRange(f_low, f_high, Cv_low, Cv_high, Cstray, range, accuracy);
    if (!range->isEmpty()){
        if (((*ind < range->at(0)) || (*ind > range->at(1))) && (!isAutomaticLe)){
            result = -2;
        } else {
            QVector<double> *cap = new QVector<double>();
            if (isAutomaticLe){
                /*Sequentially go over the discrete mesh of inductances in the permissible range of values ​​and
              select the value with the minimum frequency discrepancy.*/
                const int MESH = 200;
                double step = (range->at(1) - range->at(0)) / (MESH - 1);
                double delta = DBL_MAX;
                double Cpad_entr = 0.0;
                double Ctrim_entr = 0.0;
                for (int i = 0; i < MESH; i++){
                    double tmpL =  range->at(0) + i * step;
                    calcCapacitors(f_low, f_high, Cv_low, Cv_high, Cstray, tmpL, cap, accuracy);
                    if (!cap->isEmpty()){
                        double tmpCpad_entr = cap->at(0);
                        double tmpCtrim_entr = cap->at(1);
                        int r =  nlsm(f_low, f_high, f_i, Cv_low, Cv_high, Cstray, tmpL, tmpCpad_entr, tmpCtrim_entr, data, conFreq,
                                      accuracy, isHighSideInjection, maxDelta);
                        if (r == 0){
                            if (delta > *maxDelta){
                                delta = *maxDelta;
                                Cpad_entr = tmpCpad_entr;
                                Ctrim_entr = tmpCtrim_entr;
                                *ind = tmpL;
                            }
                        }
                    }
                }
                if (delta != DBL_MAX)
                    result = nlsm(f_low, f_high, f_i, Cv_low, Cv_high, Cstray, *ind, Cpad_entr, Ctrim_entr, data, conFreq, accuracy, isHighSideInjection, maxDelta);
                else
                    result = 1;
            } else {
                calcCapacitors(f_low, f_high, Cv_low, Cv_high, Cstray, *ind, cap, accuracy);
                if (!cap->isEmpty()){
                    double Cpad_entr = cap->at(0);
                    double Ctrim_entr = cap->at(1);
                    result =  nlsm(f_low, f_high, f_i, Cv_low, Cv_high, Cstray, *ind, Cpad_entr, Ctrim_entr, data, conFreq, accuracy, isHighSideInjection, maxDelta);
                } else {
                    result = 1;
                }
            }
            delete cap;
        }
    } else {
        result = -1;
    }
    delete range;
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findPadderCapacitance(double Ct,double Cv_low,double Cv_high,double Cstray,double cap_ratio) {
    double chp = Cv_high + Ct;
    double clp = Cv_low + Ct;
    double beta = (cap_ratio - 1) * Cstray;
    double a = cap_ratio * clp - chp + beta;
    double b = (cap_ratio - 1) * clp * chp + beta * (clp + chp);
    double c = beta * clp * chp;
    return (-b - sqrt(b * b - 4 * a * c)) / (2.0 * a);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double findTrimmerCapacitance(double Cp,double Cv_low,double Cv_high,double Cstray,double cap_ratio) {
    double Ct = 0.0;
    if (Cp < 0.00001) {
        Ct = (Cv_high + Cstray - cap_ratio * (Cv_low + Cstray)) / (cap_ratio - 1);
    } else {
        double k0 = (1 - cap_ratio) * Cstray;
        double k1 = k0 * Cp * Cp;
        double k2 = k0 * Cp + Cp * Cp;
        double k3 = k0 * Cp - cap_ratio * Cp * Cp;
        double k4 = k0 + (1 - cap_ratio) * Cp;
        double a = k4;
        double b = k2 + k3 + k4 * (Cv_low + Cv_high);
        double c = k1 + k2 * Cv_high + k3 * Cv_low + k4 * Cv_low * Cv_high;
        Ct = (-b - sqrt(b * b - 4 * a * c)) / (2.0 * a);
    }
    return Ct;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void calcInductanceRange(double f_low, double f_high, double Cv_low, double Cv_high, double Cstray, QVector<double> *range, int accuracy) {
    int appr = (int) pow(10, accuracy);
    double ratioFreq = f_high / f_low;
    double ratioCap = ratioFreq * ratioFreq;
    double cpmin = findPadderCapacitance(0, Cv_low, Cv_high, Cstray, ratioCap);
    double ctmax = findTrimmerCapacitance(0,Cv_low, Cv_high, Cstray, ratioCap);
    double w = 1e3/(2 * M_PI * f_low);
    double chpppm = Cv_high * cpmin / (Cv_high + cpmin) + Cstray;
    double Lmax = w * w / chpppm;
    double chppp = Cv_high + Cstray + ctmax;
    double Lmin = w * w / chppp;
    range->clear();
    if ((Lmax > 0) && (Lmin > 0) && (ctmax > 0) && (cpmin > 0)){
        range->push_front(floor(Lmax * appr) / appr);
        range->push_front(ceil(Lmin * appr) / appr);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void calcCapacitors(double f_low, double f_high, double Cv_low, double Cv_high, double Cstray, double ind, QVector<double> *cap, int accuracy){
    int appr = (int) pow(10, accuracy);
    double w = 1e3/(2 * M_PI * f_low);
    double chppp = w * w / ind;
    w = 1e3/(2 * M_PI * f_high);
    double clppp = w * w / ind;
    double alpha = Cstray - chppp;
    double beta = Cstray - clppp;
    double k1 = Cv_high - chppp - Cv_low + clppp;
    double k2 = alpha - beta;
    double k3 = alpha * Cv_high - beta * Cv_low;
    double k4 = alpha * Cv_high;
    double k5 = Cv_high + alpha;
    double a = -k2 / k1;
    double b = alpha - (k3 + k2 * k5) / k1;
    double c = k4 - (k3 * k5) / k1;
    double Ct = (-b + sqrt(b * b - 4 * a * c)) / (2.0 * a);
    double Cp = -(k2 * Ct + k3) / k1;
    double netCmin = CalcLC2(ind, f_high);
    double netCmax = CalcLC2(ind, f_low);
    cap->clear();
    if ((std::isnormal(Ct)) && (std::isnormal(Cp)) && (Ct > 0) && (Cp > 0)){
        cap->push_front(floor(netCmax * appr) / appr);
        cap->push_front(ceil(netCmin * appr) / appr);
        cap->push_front(round(Cp * appr) / appr);
        cap->push_front(round(Ct * appr) / appr);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void calcFrequencyRange(double Ct, double Cp, double Cv_low, double Cv_high, double Cstray, double ind, QVector<double> *freq, int accuracy){
    int appr = (int) pow(10, accuracy);
    double chppp = (Cp * (Cv_high + Ct) / (Cv_high + Ct + Cp)) + Cstray;
    double clppp = (Cp * (Cv_low + Ct) / (Cv_low + Ct + Cp)) + Cstray;
    double low_freq = CalcLC0(ind, chppp);
    double high_freq = CalcLC0(ind, clppp);
    double netCmin = CalcLC2(ind, high_freq);
    double netCmax = CalcLC2(ind, low_freq);
    freq->clear();
    if ((low_freq > 0) && (high_freq > 0)){
        freq->push_front(round(netCmax * appr) / appr);
        freq->push_front(ceil(netCmin * appr) / appr);
        freq->push_front(round(high_freq * appr) / appr);
        freq->push_front(ceil(low_freq * appr) / appr);
    }
}
