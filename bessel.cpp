/* bessel.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "bessel.h"
#include <math.h>
#include <iostream>

const double ACC = 4.0;
const double BIGNO = 1.0e10;
const double BIGNI = 1.0e-10;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double bessi0(double x){
    double answer;
    double ax = fabs(x);
    if (ax < 3.75) { // polynomial fit
        double y = x / 3.75;
        y *= y;
        answer = 1.0 + y * (3.5156229 + y * (3.0899424 + y * (1.2067492 + y * (0.2659732 + y * (0.360768e-1 + y * 0.45813e-2)))));
    } else {
        double y = 3.75 / ax;
        answer = 0.39894228 + y * (0.1328592e-1 + y * (0.225319e-2 + y * (-0.157565e-2 + y * (0.916281e-2 + y * (-0.2057706e-1 + y * (0.2635537e-1 + y * (-0.1647633e-1 + y * 0.392377e-2)))))));
        answer *= (exp(ax) / sqrt(ax));
    }
    return answer;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double bessi1(double x){
    double answer;
    double ax = fabs(x);
    if (ax < 3.75) { // polynomial fit
        double y = x / 3.75;
        y *= y;
        answer = ax * (0.5 + y * (0.87890594 + y * (0.51498869 + y * (0.15084934 + y * (0.2658733e-1 + y * (0.301532e-2 + y * 0.32411e-3))))));
    } else {
        double y = 3.75 / ax;
        answer = 0.2282967e-1 + y * (-0.2895312e-1 + y * (0.1787654e-1 - y * 0.420059e-2));
        answer = 0.39894228 + y * (-0.3988024e-1 + y * (-0.362018e-2 + y * (0.163801e-2 + y * (-0.1031555e-1 + y * answer))));
        answer *= (exp(ax) / sqrt(ax));
    }
    return answer;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double bessi(int n, double x){
    if (n < 2)
        throw ("Function order must be greater than 1");
    if (x == 0.0) {
        return 0.0;
    } else {
        double tox = 2.0/fabs(x);
        double ans = 0.0;
        double bip = 0.0;
        double bi  = 1.0;
        for (int j = 2*(n + (int)sqrt(ACC*n)); j > 0; --j) {
            double bim = bip + j*tox*bi;
            bip = bi;
            bi = bim;
            if (fabs(bi) > BIGNO) {
                ans *= BIGNI;
                bi *= BIGNI;
                bip *= BIGNI;
            }
            if (j == n) {
                ans = bip;
            }
        }
        ans *= bessi0(x)/bi;
        return (((x < 0.0) && ((n % 2) == 0)) ? -ans : ans);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @param x a double value
 * @return the Bessel function of order 0 of the argument.
 */
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double bessj0(double x){
    try {
        double ax;

        if( (ax=fabs(x)) < 8.0 ) {
            double y=x*x;
            double ans1=57568490574.0+y*(-13362590354.0+y*(651619640.7
                                                           +y*(-11214424.18+y*(77392.33017+y*(-184.9052456)))));
            double ans2=57568490411.0+y*(1029532985.0+y*(9494680.718
                                                         +y*(59272.64853+y*(267.8532712+y*1.0))));

            return ans1/ans2;

        } else {
            double z=8.0/ax;
            double y=z*z;
            double xx=ax-0.785398164;
            double ans1=1.0+y*(-0.1098628627e-2+y*(0.2734510407e-4
                                                   +y*(-0.2073370639e-5+y*0.2093887211e-6)));
            double ans2 = -0.1562499995e-1+y*(0.1430488765e-3
                                              +y*(-0.6911147651e-5+y*(0.7621095161e-6
                                                                      -y*0.934935152e-7)));

            return sqrt(0.636619772/ax)*
                    (cos(xx)*ans1-z*sin(xx)*ans2);
        }
    } catch (const char* msg) {
        std::cout << msg;
        return 0;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @param x a double value
 * @return the Bessel function of order 1 of the argument.
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double bessj1(double x){
    try {
        double ax;
        double y;
        double ans1, ans2;

        if ((ax=fabs(x)) < 8.0) {
            y=x*x;
            ans1=x*(72362614232.0+y*(-7895059235.0+y*(242396853.1
                                                      +y*(-2972611.439+y*(15704.48260+y*(-30.16036606))))));
            ans2=144725228442.0+y*(2300535178.0+y*(18583304.74
                                                   +y*(99447.43394+y*(376.9991397+y*1.0))));
            return ans1/ans2;
        } else {
            double z=8.0/ax;
            double xx=ax-2.356194491;
            y=z*z;

            ans1=1.0+y*(0.183105e-2+y*(-0.3516396496e-4
                                       +y*(0.2457520174e-5+y*(-0.240337019e-6))));
            ans2=0.04687499995+y*(-0.2002690873e-3
                                  +y*(0.8449199096e-5+y*(-0.88228987e-6
                                                         +y*0.105787412e-6)));
            double ans=sqrt(0.636619772/ax)*
                    (cos(xx)*ans1-z*sin(xx)*ans2);
            if (x < 0.0) ans = -ans;
            return ans;
        }
    } catch (const char* msg) {
        std::cout << msg;
        return 0;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @param n integer order
 * @param x a double value
 * @return the Bessel function of order n of the argument.
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double bessjn(int n, double x){
    try {
        int j,m;
        double ax,bj,bjm,bjp,sum,tox,ans;
        bool jsum;

        double ACC   = 40.0;
        double BIGNO = 1.0e+10;
        double BIGNI = 1.0e-10;

        if(n == 0) return bessj0(x);
        if(n == 1) return bessj1(x);

        ax=fabs(x);
        if(ax == 0.0)  return 0.0;
        else
            if (ax > (double)n) {
                tox=2.0/ax;
                bjm=bessj0(ax);
                bj=bessj1(ax);
                for (j=1;j<n;j++) {
                    bjp=j*tox*bj-bjm;
                    bjm=bj;
                    bj=bjp;
                }
                ans=bj;
            } else {
                tox=2.0/ax;
                m=2*((n+(int) sqrt(ACC*n))/2);
                jsum=false;
                bjp=ans=sum=0.0;
                bj=1.0;
                for (j=m;j>0;j--) {
                    bjm=j*tox*bj-bjp;
                    bjp=bj;
                    bj=bjm;
                    if (fabs(bj) > BIGNO) {
                        bj *= BIGNI;
                        bjp *= BIGNI;
                        ans *= BIGNI;
                        sum *= BIGNI;
                    }
                    if (jsum) sum += bj;
                    jsum=!jsum;
                    if (j == n) ans=bjp;
                }
                sum=2.0*sum-bj;
                ans /= sum;
            }
        return  x < 0.0 && n%2 == 1 ? -ans : ans;
    } catch (const char* msg) {
        std::cout << msg;
        return 0;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @param x a double value
 * @return the Bessel function of the second kind,
 *          of order 0 of the argument.
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double bessy0(double x){
    try {
        if (x < 8.0) {
            double y=x*x;

            double ans1 = -2957821389.0+y*(7062834065.0+y*(-512359803.6
                                                           +y*(10879881.29+y*(-86327.92757+y*228.4622733))));
            double ans2=40076544269.0+y*(745249964.8+y*(7189466.438
                                                        +y*(47447.26470+y*(226.1030244+y*1.0))));

            return (ans1/ans2)+0.636619772*bessj0(x)*log(x);
        } else {
            double z=8.0/x;
            double y=z*z;
            double xx=x-0.785398164;

            double ans1=1.0+y*(-0.1098628627e-2+y*(0.2734510407e-4
                                                   +y*(-0.2073370639e-5+y*0.2093887211e-6)));
            double ans2 = -0.1562499995e-1+y*(0.1430488765e-3
                                              +y*(-0.6911147651e-5+y*(0.7621095161e-6
                                                                      +y*(-0.934945152e-7))));
            return sqrt(0.636619772/x)*
                    (sin(xx)*ans1+z*cos(xx)*ans2);
        }
    } catch (const char* msg) {
        std::cout << msg;
        return 0;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @param x a double value
 * @return the Bessel function of the second kind,
 *  of order 1 of the argument.
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double bessy1(double x){
    try {
        if (x < 8.0) {
            double y=x*x;
            double ans1=x*(-0.4900604943e13+y*(0.1275274390e13
                                               +y*(-0.5153438139e11+y*(0.7349264551e9
                                                                       +y*(-0.4237922726e7+y*0.8511937935e4)))));
            double ans2=0.2499580570e14+y*(0.4244419664e12
                                           +y*(0.3733650367e10+y*(0.2245904002e8
                                                                  +y*(0.1020426050e6+y*(0.3549632885e3+y)))));
            return (ans1/ans2)+0.636619772*(bessj1(x)*log(x)-1.0/x);
        } else {
            double z=8.0/x;
            double y=z*z;
            double xx=x-2.356194491;
            double ans1=1.0+y*(0.183105e-2+y*(-0.3516396496e-4
                                              +y*(0.2457520174e-5+y*(-0.240337019e-6))));
            double ans2=0.04687499995+y*(-0.2002690873e-3
                                         +y*(0.8449199096e-5+y*(-0.88228987e-6
                                                                +y*0.105787412e-6)));
            return sqrt(0.636619772/x)*
                    (sin(xx)*ans1+z*cos(xx)*ans2);
        }
    } catch (const char* msg) {
        std::cout << msg;
        return 0;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @param n integer order
 * @param x a double value
 * @return the Bessel function of the second kind,
 *    of order n of the argument.
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double bessyn(int n, double x){
    try {
        double by,bym,byp,tox;

        if(n == 0) return bessy0(x);
        if(n == 1) return bessy1(x);

        tox=2.0/x;
        by=bessy1(x);
        bym=bessy0(x);
        for (int j=1;j<n;j++) {
            byp=j*tox*by-bym;
            bym=by;
            by=byp;
        }
        return by;
    } catch (const char* msg) {
        std::cout << msg;
        return 0;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//*------------------------------------------------------------*/
//* PURPOSE: Evaluate modified Bessel function Kn(x) and n=0.  */
//*------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double bessk0(double x){
    try {
        double y,ans;

        if (x <= 2.0) {
            y=x*x/4.0;
            ans=(-log(x/2.0)*bessi0(x))+(-0.57721566+y*(0.42278420
                                                        +y*(0.23069756+y*(0.3488590e-1+y*(0.262698e-2
                                                                                          +y*(0.10750e-3+y*0.74e-5))))));
        } else {
            y=2.0/x;
            ans=(exp(-x)/sqrt(x))*(1.25331414+y*(-0.7832358e-1
                                                 +y*(0.2189568e-1+y*(-0.1062446e-1+y*(0.587872e-2
                                                                                      +y*(-0.251540e-2+y*0.53208e-3))))));
        }
        return ans;
    } catch (const char* msg) {
        std::cout << msg;
        return 0;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//*------------------------------------------------------------*/
//* PURPOSE: Evaluate modified Bessel function Kn(x) and n=1.  */
//*------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double bessk1(double x){
    try {
        double y,ans;

        if (x <= 2.0) {
            y=x*x/4.0;
            ans=(log(x/2.0)*bessi1(x))+(1.0/x)*(1.0+y*(0.15443144
                                                       +y*(-0.67278579+y*(-0.18156897+y*(-0.1919402e-1
                                                                                         +y*(-0.110404e-2+y*(-0.4686e-4)))))));
        } else {
            y=2.0/x;
            ans=(exp(-x)/sqrt(x))*(1.25331414+y*(0.23498619
                                                 +y*(-0.3655620e-1+y*(0.1504268e-1+y*(-0.780353e-2
                                                                                      +y*(0.325614e-2+y*(-0.68245e-3)))))));
        }
        return ans;
    } catch (const char* msg) {
        std::cout << msg;
        return 0;
    }
}
