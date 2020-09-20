/* amidon.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "amidon.h"
#include "ui_amidon.h"


enum _TypeCore
{
    _TToroid = 0, //iron powder toroid
    _FToroid,   //ferrite toroid
    _PotCore,   //pot core
    _ECore,   //E core
    _MCore, //Multi-aperture core
};

QStringList TToroidMaterial = (QStringList()<<"0"<<"1"<<"2"<<"3"<<"6"<<"7"<<"8"<<"10"<<"12"<<"15"<<"17"<<"18"<<"26"<<"30"<<"40"<<"52");
QString TToroidSize[] = {
    "5,0.050,0.025,0.025", "7,0.07,0.035,0.03", "10,0.100,0.044,0.030", "12,0.125,0.062,0.050",
    "12A,0.125,0.062,0.042", "16,0.160,0.078,0.060", "20,0.200,0.088,0.070", "22,0.223,0.097,0.143",
    "25,0.250,0.120,0.096", "27,0.28,0.151,0.128", "30,0.307,0.151,0.128", "37,0.370,0.205,0.128",
    "38,0.375,0.175,0.19", "44,0.440,0.229,0.159", "50,0.500,0.300,0.190", "50B,0.5,0.303,0.25",
    "60,0.600,0.336,0.234", "68,0.690,0.370,0.190", "68A,0.69,0.37,0.25", "72,0.72,0.28,0.26",
    "80,0.795,0.495,0.250", "80A,0.795,0.495,0.375", "90,0.90,0.55,0.375", "94,0.942,0.560,0.312",
    "106,1.060,0.570,0.437", "106A,1.06,0.57,0.312", "106B,1.06,0.57,0.575", "130,1.300,0.780,0.437",
    "157,1.57,0.95,0.57", "184,1.84,0.95,0.71", "200,2.00,1.25,0.55", "200A,2.00,1.25,1.00",
    "225,2.25,1.4,0.55", "225A,2.25,1.4,1", "300,3.04,1.93,0.5", "300A,3.04,1.93,1", "400,4.0,2.5,0.65",
    "400A,4.0,2.25,1.3", "520,5.2,3.08,0.8"
};
double TToroid_AL[][39] = {
    {1.6, 0, 2.4, 3, 0, 3, 3.5, 0, 4.5, 0, 6, 4.9, 0, 6.5, 6.4, 0, 0, 7.5, 0, 0, 8.5, 0, 0, 10.6, 19, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 48, 0, 44, 52, 0, 70, 0, 85, 85, 0, 105, 100, 0, 0, 115, 0, 0, 115, 0, 0, 160, 280, 0, 0, 200, 320, 500, 250, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 20, 0, 22, 25, 0, 34, 0, 43, 40, 0, 52, 49, 0, 65, 57, 0, 0, 55, 0, 0, 84, 135, 0, 0, 110, 140, 240, 120, 218, 120, 215, 114, 228, 180, 360, 207},
    {0, 0, 0, 60, 0, 61, 76, 0, 100, 0, 140, 120, 0, 180, 175, 0, 0, 195, 0, 0, 180, 0, 0, 248, 450, 0, 0, 350, 420, 0, 425, 0, 425, 0, 0, 0, 0, 0, 0},
    {10, 13, 11.5, 17, 18.5, 19, 22, 45, 27, 27, 36, 30, 0, 42, 46, 0, 0, 47, 0, 0, 45, 0, 0, 70, 116, 0, 0, 96, 115, 195, 104, 180, 105, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 23, 0, 29, 0, 0, 32, 0, 46, 43, 0, 0, 52, 0, 0, 0, 0, 0, 0, 0, 0, 0, 103, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 170, 0, 100, 0, 140, 120, 0, 180, 175, 0, 190, 0, 260, 360, 180, 0, 300, 0, 450, 0, 0, 350, 420, 720, 425, 0, 425, 0, 0, 0, 600, 0, 0},
    {0, 9, 8, 12, 0, 13, 16, 32, 19, 22, 25, 25, 0, 33, 31, 0, 0, 32, 0, 0, 32, 0, 0, 58, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 6, 5, 7.5, 0, 8, 10, 0, 12, 0, 16, 15, 0, 18.5, 18, 0, 0, 21, 0, 0, 22, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 50, 0, 55, 65, 0, 85, 0, 93, 90, 0, 160, 135, 0, 0, 180, 0, 0, 170, 0, 0, 200, 345, 0, 0, 250, 360, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 5, 7.5, 0, 8, 10, 0, 12, 0, 16, 15, 0, 18.5, 18, 0, 0, 21, 0, 0, 22, 0, 0, 29, 0, 0, 0, 40, 53, 87, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 95, 0, 0, 0, 0, 220, 190, 0, 225, 240, 320, 345, 0, 0, 600, 310, 0, 470, 420, 0, 490, 0, 580, 730, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 145, 185, 0, 245, 0, 335, 0, 0, 370, 330, 720, 500, 435, 580, 0, 460, 0, 700, 0, 930, 670, 1240, 810, 1000, 1690, 920, 1600, 980, 1600, 800, 1600, 1310, 2620, 1490},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 205, 0, 280, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 570, 490, 810, 0, 0, 690, 0, 0, 790, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 170, 0, 230, 0, 305, 260, 490, 350, 330, 435, 470, 400, 540, 820, 420, 290, 640, 570, 670, 0, 0, 890, 990, 0, 920, 0, 0, 0, 800, 0, 1310, 0, 0}
};
QString TToroid_Features[] = {
    "1,100 MHz - 300 MHz,Tan/Tan,Phenolic",
    "20,0.5 MHz - 5 MHz,Blue/Clear,Carbonyl C",
    "10,2 MHz - 30 MHz,Red/Clear,Carbonyl E",
    "35,0.05 MHz - 0.5 MHz,Gray/Clear,Carbonyl HP",
    "8,10 MHz - 50 MHz,Yellow/Clear,Carbonyl SF",
    "9,3 MHz - 35 MHz,White/Clear,Carbonyl TH",
    "35,N/A,Orange/Clear,Carbonyl GQ4",
    "6,30 MHz - 100 MHz,Black/Clear,Carbonyl W",
    "4,50 MHz - 200 MHz,Green/White,Synthetic Oxide",
    "25,0.1 MHz - 2.0 MHz,Red/White,Carbonyl GS6",
    "4,20 MHz - 200 MHz,Blue/Yellow,Carbonyl",
    "55,N/A,Green/Red,N/A",
    "75,LF filters/chokes,Yellow/White,Special",
    "22,N/A,Green/Clear,N/A",
    "60,N/A,Green/Yellow,N/A",
    "75,N/A,Green/Blue,N/A"
};
QStringList FToroidMaterial = (QStringList()<<"43"<<"52"<<"61"<<"67"<<"68"<<"75"<<"77"<<"F"<<"H"<<"J"<<"K"<<"W");
QString FToroidSize[] = {
    "16,0.155,0.088,0.05", "23,0.230,0.120,0.060", "23A,0.230,0.120,0.120", "37,0.375,0.187,0.125",
    "50,0.500,0.281,0.188", "50A,0.500,0.312,0.250", "50B,0.500,0.312,0.500", "63A,0.630,0.378,0.250",
    "82,0.825,0.520,0.250", "82A,0.825,0.520,0.468", "87,0.870,0.540,0.250", "87A,0.870,0.540,0.500",
    "100,1.000,0.610,0.250", "100A,1.000,0.610,0.320", "100B,1.000,0.610,0.500", "114,1.142,0.748,0.295",
    "114A,1.142,0.748,0.545", "125,1.250,0.750,0.375", "140,1.400,0.900,0.500", "140A,1.417,0.906,0.590",
    "150,1.500,0.750,0.250", "150A,1.500,0.750,0.500", "193,1.932,1.252,0.625", "193A,1.932,1.252,0.850",
    "240,2.400,1.400,0.500", "290,2.900,1.530,0.500", "337,3.375,2.187,0.500", "350,3.500,2.000,0.500"
};
double FToroid_AL[][28] = {
    {0, 158, 0, 350, 440, 480, 965, 520, 470, 885, 485, 970, 500, 645, 1000, 510, 950, 775, 885, 0, 0, 0, 0, 1075, 1300, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 325, 0, 0, 0},
    {15, 25, 0, 55, 69, 75, 150, 80, 75, 135, 75, 0, 0, 0, 0, 80, 145, 120, 140, 0, 0, 0, 0, 0, 170, 0, 0, 178},
    {0, 6, 0, 18, 22, 24, 48, 0, 24, 0, 25, 0, 0, 32, 0, 25, 47, 39, 45, 0, 0, 0, 0, 0, 55, 0, 0, 0},
    {0, 0, 0, 0, 6.6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 585, 0, 2200, 2725, 3000, 6000, 0, 2950, 0, 0, 6100, 0, 4000, 0, 0, 0, 0, 5500, 0, 0, 0, 0, 0, 0, 8100, 0, 0},
    {0, 420, 850, 945, 1180, 1300, 2595, 1400, 1270, 2375, 0, 0, 0, 1730, 0, 1365, 2520, 2090, 2545, 0, 0, 0, 0, 0, 3155, 3740, 0, 0},
    {0, 585, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3624, 0, 0, 0, 1902, 0, 0, 0, 4040, 0, 5020, 0, 4460, 4107, 4880, 0, 0},
    {0, 0, 0, 6590, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 980, 0, 2196, 2715, 2968, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3170, 0, 0, 0, 6736, 4400, 8365, 6065, 7435, 6845, 8140, 5520, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 530, 0, 0, 458, 0, 0, 0},
    {0, 0, 0, 4392, 5936, 0, 0, 0, 0, 0, 6040, 12080, 0, 0, 0, 0, 0, 0, 0, 13400, 0, 16700, 0, 0, 13690, 16820, 11040, 0}
};
QString FToroid_Features[] = {
    "850,2950,1310,0.45,135,0.01-1,1-50,30-600",        //43
    "250,4200,3300,0.60,250,0.5-20,20-200,200-1000",   //52
    "125,2400,1000,1.9,350,0.2-10,10-200,200-1000",     //61
    "40,2300,800,4,450,10-80,50-500,350-1500",          //67
    "20,2500,700,7,500,80-180,200-1000,1000-5000",      //68
    "5000,4700,1200,0.1,175,0.001-1,0.2-10,0.5-15",     //75
    "2000,4900,1500,0.2,190,0.001-2,0.5-30,1-50",       //77
    "3000,4700,N/A,0.2,210,0.001-1,0.5-30,1-20",        //F
    "15000,4200,N/A,0.04,120,0.001-0.2,0.001-1,0.01-0.5",//H
    "5000,4300,N/A,0.1,145,0.001-1,0.1-10,0.5-15",      //J
    "290,4600,N/A,0.2,230,0.5-30,20-200,200-5000",      //K
    "10000,4300,N/A,0.04,125,0.001-0.3,0.001-1,0.1-1"   //W
};
QString PotCoreSize[] = {
    "10.90-11.30,6.40-6.60,9.00-9.40,4.50-4.70,2.00-2.10,4.40-4.70,6.50-6.95,2.65-3.05",
    "13.80-14.30,8.20-8.50,11.60-12.00,5.80-6.00,3.00-3.10,5.60-6.00,8.70-9.20,3.60-4.10",
    "17.60-18.40,10.40-10.70,14.90-15.40,7.30-7.60,3.00-3.20,7.20-7.60,11.30-12.30,3.90-4.40",
    "21.20-22.00,13.20-13.60,17.90-18.50,9.10-9.40,4.40-4.60,9.20-9.60,13.30-13.90,3.90-4.50",
    "25.00-26.00,15.90-16.30,21.20-22.00,11.10-11.50,5.40-5.60,11.00-11.40,17.00-17.70,3.90-4.50",
    "29.50-30.50,18.60-19.00,25.00-25.80,13.10-13.50,5.40-5.60,13.00-13.40,20.00-20.90,4.70-5.30",
    "35.00-36.00,21.40-22.00,29.90-30.70,15.80-16.20,5.40-5.70,14.60-15.00,25.00-26.00,4.30-4.70"
};
QString PotCore_Features[] = {
    "1420,15.90,15.90,252",
    "1960,19.80,25.10,495",
    "2880,25.80,43.30,1120",
    "3660,31.50,63.40,2000",
    "4700,37.50,94,3520",
    "5900,45,136,6120",
    "7680,53,202,10700"
};
QString ECoreSize[] = {
    "0.760,0.318,0.187,0.225,0.093,0.192,0.187",
    "1.000,0.380,0.250,0.255,0.125,0.250,0.250",
    "1.375,0.562,0.375,0.375,0.187,0.312,0.375",
    "1.625,0.650,0.500,0.405,0.250,0.312,0.500",
    "1.680,0.825,0.605,0.593,0.234,0.375,0.468"
};
QString ECore_Features[] = {
    "22.5,40.1,900,10,1060",
    "40.4,48.0,1930,20,1660",
    "90.3,68.8,6240,70,2760",
    "160,76.7,12300,100,4470",
    "184,98,18000,200,5300"
};
QStringList MultiapertureMaterial = (QStringList()<<"43"<<"61"<<"67"<<"73");
QString MultiapertureSize[]= {
    "102;13.3±0.30;13.4±0.30;7.5±0.35;5.7±0.25;3.80±0.25;120;170;2700;1",//BN43-102
    "1502;13.30±0.30;6.60±0.30;7.50±0.35;5.7±0.25;3.80±0.25;70;106;1300;1",//BN43-1502
    "202;13.30±0.30;14.35±0.50;7.50±0.35;5.7±0.25;3.80±0.25;120;180;2800;1",//BN43-202
    "2302;3.45±0.25;2.35±0.25;2±0.2;1.45±0.15;0.75±0.25;35;40;600;1",//BN43-2302
    "2402;7±0.25;6.2±0.25;4.2±0.35;2.9±0.1;1.7±0.2;74;100;1400;1",//BN43-2402
    "2702;7±0.25;6.2±0.25;4.2±0.35;2.9±0.1;1.7±0.2;35;45;700;1",//BN43-2702
    "302;13.3±0.60;10.3±0.30;7.50±0.35;5.5±0.3;3.8±0.25;100;125;1900;1",//BN43-302
    "3312;19.4±0.45;25±0.70;9.50±0.3;10±0.3;4.75±0.20;300;400;5000;3",//BN43-3312
    "402;19.4±0.45;12.7±0.50;9.50±0.3;10±0.3;4.75±0.20;150;200;2500;3",//BN43-402
    "6802;13.3±0.65;27±0.75;7.50±0.35;5.7±0.3;3.8±0.25;215;295;5000;1",//BN43-6802
    "7051;29±0.65;29±0.75;14.2±0.35;14.00±0.30;6.35±0.2;375;480;6700;1",//BN43-7051

    "002;30±0.65;29±0.75;15±0.35;14.5±0.45;6.80±0.3;500;600;1100;3",//BN61-002
    "102;13.3±0.3;13.4±0.3;7.5±0.35;5.7±0.25;3.80±0.25;155;220;360;1",//BN61-102
    "1502;13.3±0.3;6.6±0.3;7.5±0.35;5.7±0.25;3.80±0.25;90;110;170;1",//BN61-1502
    "1702;6.35±0.25;12±0.35;0;2.75±0.25;1.1±0.3;200;270;500;2",//BN61-1702
    "1802;6.35±0.25;6.15±0.25;0;2.75±0.25;1.1±0.3;115;150;310;2",//BN61-1802
    "202;13.30±0.30;14.35±0.50;7.50±0.35;5.7±0.25;3.80±0.25;150;185;380;1",//BN61-202
    "2302;3.45±0.25;2.35±0.25;2±0.2;1.45±0.15;0.75±0.25;35;50;60;1",//BN61-2302
    "2402;7±0.25;6.2±0.25;4.2±0.25;2.9±0.1;1.7±0.2;80;115;200;1",//BN61-2402
    "2702;7±0.25;6.2±0.25;4.2±0.25;2.9±0.1;1.7±0.2;40;60;100;1",//BN61-2702
    "302;13.30±0.60;10.30±0.30;7.50±0.35;5.5±0.30;3.80±0.25;150;200;300;1",//BN61-302
    "6802;13.30±0.65;27±0.75;7.50±0.35;5.7±0.30;3.80±0.25;300;400;720;1",//BN61-6802

    "102;13.30±0.60;13.40±0.30;7.50±0.35;5.71±0.25;3.80±0.25;0;0;68;1",//BN67-102
    "1502;13.30±0.60;6.60±0.25;7.50±0.35;5.71±0.25;3.80±0.25;0;0;44;1",//BN67-1502
    "2302;3.5±0.25;2.4±0.25;2.00±0.15;1.5±0.15;0.9±0.25;0;0;18;1",//BN67-2302
    "2402;7.1±0.25;6.1±0.25;4.1±0.25;2.9±0.25;1.8±0.20;0;0;48;1",//BN67-2402
    "2702;7.1±0.25;3.1±0.25;4.1±0.25;2.9±0.25;1.8±0.20;0;0;24;1",//BN67-2702

    "1502;13.34±0.60;6.6±0.35;7.50±0.35;5.71±0.35;3.81±0.25;0;0;0;1",//BN73-1502
    "202;13.30±0.60;14.35±0.50;7.50±0.35;5.71±0.35;3.80±0.25;125;100;8500;1",//BN73-202
    "2302;3.45±0.25;2.35±0.25;2.00±0.15;1.45±0.15;0.75±0.25;35;44;0;1",//BN73-2302
    "2402;7.00±0.25;6.20±0.25;4.20±0.25;2.9±0.20;1.70±0.20;80;75;3750;1",//BN73-2402
    "2702;7.00±0.25;6.20±0.25;4.20±0.25;2.9±0.20;1.70±0.20;25;35;2100;1",//BN73-2702
    "302;13.30±0.60;10.30±0.30;7.50±0.35;5.72±0.25;3.80±0.25;94;75;0;1",//BN73-302
    "6802;13.30±0.60;27.00±0.60;7.50±0.35;5.72±0.25;3.80±0.25;195;180;0;1"//BN73-6802
};
QString MultiapertureFeatures[] = {
    "850,3000,2750,1200,130,0.01-1,1-50,30-600",        //43
    "125,450,2150,1200,350,0.2-10,10-200,200-1000",     //61
    "40,125,2300,800,450,10-80,25-500,350-1500",          //67
    "2500,4000,4000,1250,160,0.001-1,0.5-30,10-50",     //73
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Amidon::Amidon(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Amidon)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator;
    ui->lineEdit_ind->setValidator(dv);
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(0);
    effect->setColor(palette().color(QPalette::Shadow));
    effect->setOffset(1,1);
    ui->label->setGraphicsEffect(effect);
    for(int i = 0; i < TToroidMaterial.length(); i++){
        ui->comboBox_tm->addItem(TToroidMaterial.at(i));
    }
    for(int i = 0; i < FToroidMaterial.length(); i++){
        ui->comboBox_fm->addItem(FToroidMaterial.at(i));
    }
    for(int i = 0; i < MultiapertureMaterial.length(); i++){
        ui->comboBox_bn_m->addItem(MultiapertureMaterial.at(i));
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Amidon::~Amidon()
{
    double I = loc.toDouble(ui->lineEdit_ind->text())*fOpt->dwInductanceMultiplier;
    int tab = ui->tabWidget->currentIndex();
    int t_material = ui->comboBox_tm->currentIndex();
    int t_size = ui->comboBox_td->currentIndex();
    int f_material = ui->comboBox_fm->currentIndex();
    int f_size = ui->comboBox_fd->currentIndex();
    int bn_material = ui->comboBox_bn_m->currentIndex();
    int bn_size = ui->comboBox_bn_t->currentIndex();
    int p_index;
    if (ui->radioButton_p01->isChecked())
        p_index = 0;
    if (ui->radioButton_p02->isChecked())
        p_index = 1;
    if (ui->radioButton_p03->isChecked())
        p_index = 2;
    if (ui->radioButton_p04->isChecked())
        p_index = 3;
    if (ui->radioButton_p05->isChecked())
        p_index = 4;
    if (ui->radioButton_p06->isChecked())
        p_index = 5;
    if (ui->radioButton_p07->isChecked())
        p_index = 6;
    int e_index;
    if (ui->radioButton_e01->isChecked())
        e_index = 0;
    if (ui->radioButton_e02->isChecked())
        e_index = 1;
    if (ui->radioButton_e03->isChecked())
        e_index = 2;
    if (ui->radioButton_e04->isChecked())
        e_index = 3;
    if (ui->radioButton_e05->isChecked())
        e_index = 4;
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Amidon" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", this->size());
    settings->setValue("L", I);
    settings->setValue("tab", tab);
    settings->setValue("t_material", t_material);
    settings->setValue("t_size", t_size);
    settings->setValue("f_material", f_material);
    settings->setValue("f_size", f_size);
    settings->setValue("p_index", p_index);
    settings->setValue("e_index", e_index);
    settings->setValue("bn_material", bn_material);
    settings->setValue("bn_size", bn_size);
    settings->endGroup();
    delete settings;
    delete fOpt;
    delete dv;
    delete ui;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_pushButton_close_clicked()
{
    this->close();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::getOpt(_OptionStruct gOpt)
{
    *fOpt = gOpt;
    ui->label_2->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    QFont f1 = this->font();
    f1.setFamily(fOpt->mainFontFamily);
    f1.setPixelSize(fOpt->mainFontSize);
    this->setFont(f1);
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup("Amidon");
    QSize size = settings->value("size", this->minimumSize()).toSize();
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    double I = settings->value("L", 0).toDouble();
    int tab = settings->value("tab", 0).toInt();
    int t_material = settings->value("t_material", 0).toInt();
    int f_material = settings->value("f_material", 0).toInt();
    int bn_material = settings->value("bn_material", 0).toInt();
    int t_size = settings->value("t_size", 0).toInt();
    int f_size = settings->value("f_size", 0).toInt();
    int bn_size = settings->value("bn_size", 0).toInt();
    int p_index = settings->value("p_index", 0).toInt();
    int e_index = settings->value("e_index", 0).toInt();
    settings->endGroup();
    ui->lineEdit_ind->setText(loc.toString(I / fOpt->dwInductanceMultiplier));
    ui->comboBox_tm->setCurrentIndex(t_material);
    ui->comboBox_fm->setCurrentIndex(f_material);
    ui->comboBox_bn_m->setCurrentIndex(bn_material);
    if (p_index == 0){
        ui->radioButton_p01->setChecked(true);
        on_radioButton_p01_clicked(true);
    }
    if (p_index == 1){
        ui->radioButton_p02->setChecked(true);
        on_radioButton_p02_clicked(true);
    }
    if (p_index == 2){
        ui->radioButton_p03->setChecked(true);
        on_radioButton_p03_clicked(true);
    }
    if (p_index == 3){
        ui->radioButton_p04->setChecked(true);
        on_radioButton_p04_clicked(true);
    }
    if (p_index == 4){
        ui->radioButton_p05->setChecked(true);
        on_radioButton_p05_clicked(true);
    }
    if (p_index == 5){
        ui->radioButton_p06->setChecked(true);
        on_radioButton_p06_clicked(true);
    }
    if (p_index == 6){
        ui->radioButton_p07->setChecked(true);
        on_radioButton_p07_clicked(true);
    }
    if (e_index == 0){
        ui->radioButton_e01->setChecked(true);
        on_radioButton_e01_clicked(true);
    }
    if (e_index == 1){
        ui->radioButton_e02->setChecked(true);
        on_radioButton_e02_clicked(true);
    }
    if (e_index == 2){
        ui->radioButton_e03->setChecked(true);
        on_radioButton_e03_clicked(true);
    }
    if (e_index == 3){
        ui->radioButton_e04->setChecked(true);
        on_radioButton_e04_clicked(true);
    }
    if (e_index == 4){
        ui->radioButton_e05->setChecked(true);
        on_radioButton_e05_clicked(true);
    }
    ui->comboBox_td->setCurrentIndex(t_size);
    ui->comboBox_fd->setCurrentIndex(f_size);
    ui->comboBox_bn_t->setCurrentIndex(bn_size);
    ui->tabWidget->setCurrentIndex(tab);
    on_tabWidget_currentChanged(tab);
    resize(size);
    move(pos);
    ui->lineEdit_ind->selectAll();
    delete settings;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_tabWidget_currentChanged(int index)
{
    if (fOpt->mainFontSize > 0){
        switch (index) {
        case _TToroid:{
            ui->label_desc->setPixmap(QPixmap(":/images/res/T-description.png"));
            QString tfeatures_str = TToroid_Features[ui->comboBox_tm->currentIndex()];
            QStringList tfeatures = tfeatures_str.split(",");
            QString tcolors = tfeatures.at(2);
            QStringList colors = tcolors.split("/");
            drawToroid(decodeColor(colors.at(0)),decodeColor(colors.at(1)));
            on_comboBox_td_currentTextChanged(ui->comboBox_td->currentText());
        }
            break;
        case _FToroid:{
            ui->label_desc->setPixmap(QPixmap(":/images/res/F-description.png"));
            drawToroid(Qt::black,0x2F4F4F);
            on_comboBox_fd_currentTextChanged(ui->comboBox_fd->currentText());
        }
            break;
        case _PotCore:{
            ui->label_desc->setPixmap(QPixmap(":/images/res/E-description.png"));
            drawImage(QPixmap(":/images/res/e-core.jpg"));
            if (ui->radioButton_e01->isChecked())
                on_radioButton_e01_clicked(true);
            if (ui->radioButton_e02->isChecked())
                on_radioButton_e02_clicked(true);
            if (ui->radioButton_e03->isChecked())
                on_radioButton_e03_clicked(true);
            if (ui->radioButton_e04->isChecked())
                on_radioButton_e04_clicked(true);
            if (ui->radioButton_e05->isChecked())
                on_radioButton_e05_clicked(true);
        }
            break;
        case _ECore:{
            ui->label_desc->setPixmap(QPixmap(":/images/res/P-description.png"));
            drawImage(QPixmap(":/images/res/pot-core.jpg"));
            if (ui->radioButton_p01->isChecked())
                on_radioButton_p01_clicked(true);
            if (ui->radioButton_p02->isChecked())
                on_radioButton_p02_clicked(true);
            if (ui->radioButton_p03->isChecked())
                on_radioButton_p03_clicked(true);
            if (ui->radioButton_p04->isChecked())
                on_radioButton_p04_clicked(true);
            if (ui->radioButton_p05->isChecked())
                on_radioButton_p05_clicked(true);
            if (ui->radioButton_p06->isChecked())
                on_radioButton_p06_clicked(true);
            if (ui->radioButton_p07->isChecked())
                on_radioButton_p07_clicked(true);

        }
            break;
        case _MCore:{
            on_comboBox_bn_t_currentIndexChanged(ui->comboBox_bn_t->currentIndex());
        }
            break;
        default:
            break;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_comboBox_tm_currentIndexChanged(int index)
{
    QString td_curr_size = ui->comboBox_td->currentText().mid(1);
    int sel_index = 0;
    int j = 0;
    ui->comboBox_td->clear();
    for (int i = 0; i < 39; i++) {
        if (TToroid_AL[index][i] > 0) {
            QString tsise_str = TToroidSize[i];
            QStringList t_size_val = tsise_str.split(",");
            ui->comboBox_td->addItem("T" + t_size_val.at(0));
            j++;
            if (t_size_val.at(0) == td_curr_size) {
                sel_index = j - 1;
            }
        }
    }
    ui->comboBox_td->setCurrentIndex(sel_index);
    on_tabWidget_currentChanged(ui->tabWidget->currentIndex());
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_comboBox_fm_currentIndexChanged(int index)
{
    QString fd_curr_size = ui->comboBox_fd->currentText().mid(3);
    int sel_index = 0;
    int j = 0;
    ui->comboBox_fd->clear();
    for (int i = 0; i < 28; i++) {
        if (FToroid_AL[index][i] > 0) {
            QString fsise_str = FToroidSize[i];
            QStringList f_size_val = fsise_str.split(",");
            ui->comboBox_fd->addItem("FT-" + f_size_val.at(0));
            j++;
            if (f_size_val.at(0) == fd_curr_size) {
                sel_index = j - 1;
            }
        }
    }
    ui->comboBox_fd->setCurrentIndex(sel_index);
    on_tabWidget_currentChanged(ui->tabWidget->currentIndex());
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_comboBox_bn_m_currentIndexChanged(int index)
{
    ui->comboBox_bn_t->clear();
    switch (index) {
    case 0:{
        for (int i = 0; i < 11; i++) {
            QString bn_size_str = MultiapertureSize[i];
            QStringList bn_size_val = bn_size_str.split(";");
            ui->comboBox_bn_t->addItem("BN-43-" + bn_size_val.at(0));
        }
    }
        break;
    case 1:{
        for (int i = 11; i < 22; i++) {
            QString bn_size_str = MultiapertureSize[i];
            QStringList bn_size_val = bn_size_str.split(";");
            ui->comboBox_bn_t->addItem("BN-61-" + bn_size_val.at(0));
        }
    }
        break;
    case 2:{
        for (int i = 22; i < 27; i++) {
            QString bn_size_str = MultiapertureSize[i];
            QStringList bn_size_val = bn_size_str.split(";");
            ui->comboBox_bn_t->addItem("BN-67-" + bn_size_val.at(0));
        }
    }
        break;
    case 3:{
        for (int i = 27; i < 34; i++) {
            QString bn_size_str = MultiapertureSize[i];
            QStringList bn_size_val = bn_size_str.split(";");
            ui->comboBox_bn_t->addItem("BN-73-" + bn_size_val.at(0));
        }
    }
        break;
    default:
        break;
    }
    on_comboBox_bn_t_currentIndexChanged(0);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_comboBox_bn_t_currentIndexChanged(int index){

    if ((ui->tabWidget->currentIndex() == _MCore) && (fOpt->mainFontSize > 0) && !(index < 0)){
        int bn_material_index = ui->comboBox_bn_m->currentIndex();
        QString bn_size_str = "";
        if (bn_material_index == 0)
            bn_size_str = MultiapertureSize[index];
        if (bn_material_index == 1)
            bn_size_str = MultiapertureSize[index + 11];
        if (bn_material_index == 2)
            bn_size_str = MultiapertureSize[index + 22];
        if (bn_material_index == 3)
            bn_size_str = MultiapertureSize[index + 27];
        QStringList bn_size_val = bn_size_str.split(";");
        QString bn_type = bn_size_val.at(9);
        int i_type = bn_type.toInt();
        if (i_type == 1){
            ui->label_desc->setPixmap(QPixmap(":/images/res/BN1-description.png"));
            drawImage(QPixmap(":/images/res/bn1.jpg"));
        }
        if (i_type == 2){
            ui->label_desc->setPixmap(QPixmap(":/images/res/BN2-description.png"));
            drawImage(QPixmap(":/images/res/bn2.jpg"));
        }
        if (i_type == 3){
            ui->label_desc->setPixmap(QPixmap(":/images/res/BN3-description.png"));
            drawImage(QPixmap(":/images/res/bn3.jpg"));
        }
        QString sC = bn_size_val.at(3);
        QString info = "<p>" + tr("Dimensions") + ":</span><br/>";
        info += "A = <span style=\"color:blue;\">" + getMultiapertureCoreSize(bn_size_val.at(1)) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;";
        info += "B = <span style=\"color:blue;\">" + getMultiapertureCoreSize(bn_size_val.at(2)) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;<br/>";
        if (!(sC == "0"))
            info += "C = <span style=\"color:blue;\">" + getMultiapertureCoreSize(bn_size_val.at(3)) + "</span>&nbsp;"
                    + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;";
        info += "E = <span style=\"color:blue;\">" + getMultiapertureCoreSize(bn_size_val.at(4)) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;";
        info += "H = <span style=\"color:blue;\">" + getMultiapertureCoreSize(bn_size_val.at(5)) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "&nbsp;<hr/>";
        QString lowImp = bn_size_val.at(6);
        QString highImp = bn_size_val.at(7);
        QString al_s = bn_size_val.at(8);
        al = al_s.toDouble();
        QString sH = bn_size_val.at(5);
        QStringList lH = sH.split("±");
        QString H = lH.at(0);
        double hole_diam = H.toDouble()/fOpt->dwLengthMultiplier;
        hole_area = M_PI * hole_diam / 4;
        QString sLowFreq = "";
        QString sHighFreq = "";
        switch (bn_material_index) {
        case 0:{
            sLowFreq = "25";
            sHighFreq = "100";
        }
            break;
        case 1:{
            sLowFreq = "100";
            sHighFreq = "250";
        }
            break;
        case 3:{
            sLowFreq = "10";
            sHighFreq = "25";
        }
            break;
        default:
            break;
        }
        if (!(lowImp == "0"))
            info += tr("Impedance at") + " " + sLowFreq + qApp->translate("Context", "MHz") +
                    ":&nbsp;<span style=\"color:blue;\">" + lowImp +
                    "</span>&nbsp;"+ qApp->translate("Context", "Ohm") +"<br/>";
        if (!(highImp == "0"))
            info += tr("Impedance at") + " " + sHighFreq + qApp->translate("Context", "MHz") +
                    ":&nbsp;<span style=\"color:blue;\">" + highImp +
                    "</span>&nbsp;"+ qApp->translate("Context", "Ohm") +"<br/>";
        QString bnfeatures_str = MultiapertureFeatures[bn_material_index];
        QStringList pfeatures = bnfeatures_str.split(",");

        info += tr("Initial magnetic permeability") + " (μ<sub>i</sub>): <span style=\"color:blue;\">"
                + pfeatures.at(0) + "</span><br/>";
        info += tr("Maximum magnetic permeability") + " (μ<sub>max</sub>): <span style=\"color:blue;\">"
                + pfeatures.at(1) + "</span><br/>";
        info += tr("Saturation flux density") + " (Bs): <span style=\"color:blue;\">" + pfeatures.at(2)
                + "</span>&nbsp;Gs<br/>";
        info += tr("Residual flux density") + " (Br): <span style=\"color:blue;\">" + pfeatures.at(3)
                + "</span>&nbsp;Gs<br/>";
        info += tr("Curie Temperature") + ": <span style=\"color:blue;\">" + pfeatures.at(4)
                + "</span>&nbsp;°C<br/>";

        info += "<u>" + tr("Working frequency") + "</u>:<br/>";
        info += tr("Resonant circuit coils") + " = <span style=\"color:blue;\">" + pfeatures.at(5)
                + "</span>&nbsp;" + qApp->translate("Context", "MHz") + "<br/>";
        info += tr("Wideband transformers") + "&nbsp;(TLT) = <span style=\"color:blue;\">" + pfeatures.at(6)
                + "</span>&nbsp;" + qApp->translate("Context", "MHz") + "<br/>";
        info += tr("Chokes") + " = <span style=\"color:blue;\">" + pfeatures.at(7)
                + "</span>&nbsp;" + qApp->translate("Context", "MHz") + "<hr/>";
        info += "A<sub>L</sub>&nbsp;" + tr("factor") + ": <span style=\"color:blue;\">"
                + QString("%1").arg(al) + "</span>&nbsp;nH/N<sup>2</sup></p>";
        ui->label_info->setText(info);
        ui->label->setText(ui->comboBox_bn_t->currentText());
        onCalculate();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_comboBox_td_currentTextChanged(const QString &arg1){

    if ((ui->tabWidget->currentIndex() == _TToroid) && (fOpt->mainFontSize > 0)){
        QString td_curr_size = arg1.mid(1);
        int tm_index = ui->comboBox_tm->currentIndex();
        QString tfeatures_str = TToroid_Features[tm_index];
        QStringList tfeatures = tfeatures_str.split(",");
        QString info = "<p>" + tr("Color code") + ": <span style=\"color:blue;\">" + tfeatures.at(2) + "</span><br/>";
        info += tr("Material") + ": <span style=\"color:blue;\">" + tfeatures.at(3) + "</span><br/>";
        info += tr("Resonant circuit frequency range") + ": <br/><span style=\"color:blue;\">" + tfeatures.at(1)
                + "</span><br/>";
        info += tr("Initial magnetic permeability") + " (μ<sub>i</sub>): <span style=\"color:blue;\">" + tfeatures.at(0)
                + "</span><br/>";
        for (int i = 0; i < 39; i++) {
            QString tsise_str = TToroidSize[i];
            QStringList t_size_val = tsise_str.split(",");
            if (t_size_val[0] == td_curr_size) {
                QString ID = t_size_val[1];
                QString OD = t_size_val[2];
                QString H = t_size_val[3];
                double id = ID.toDouble()*25.4/fOpt->dwLengthMultiplier;
                double od = OD.toDouble()*25.4/fOpt->dwLengthMultiplier;
                double h = H.toDouble()*25.4/fOpt->dwLengthMultiplier;
                hole_area = M_PI * od / 4;
                int accurasy = fOpt->indexLengthMultiplier + 1;
                if (accurasy == 4)
                    accurasy = 0;
                info += tr("Dimensions") + " (OD x ID x H): <span style=\"color:blue;\"><br/>"
                        + QString::number(id, 'f', accurasy) + " x "
                        + QString::number(od, 'f', accurasy) + " x "
                        + QString::number(h, 'f', accurasy) +
                        + "</span>&nbsp;" + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
                al = TToroid_AL[tm_index][i];
                info += "A<sub>L</sub>&nbsp;" + tr("factor") + ": <span style=\"color:blue;\">"
                        + QString("%1").arg(al) + "</span>&nbsp;µH/(N/100)<sup>2</sup></p>";
            }
        }
        ui->label_info->setText(info);
        ui->label->setText(arg1 + "-" + ui->comboBox_tm->currentText());
        onCalculate();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_comboBox_fd_currentTextChanged(const QString &arg1)
{
    if ((ui->tabWidget->currentIndex() == _FToroid) && (fOpt->mainFontSize > 0)){
        QString fd_curr_size = arg1.mid(3);
        int fm_index = ui->comboBox_fm->currentIndex();
        QString ffeatures_str = FToroid_Features[fm_index];
        QStringList ffeatures = ffeatures_str.split(",");
        QString info = "<p>" + tr("Initial magnetic permeability") + " (μ<sub>i</sub>): <span style=\"color:blue;\">" + ffeatures.at(0)
                + "</span><br/>";
        info += tr("Saturation flux density") + " (Bs): <span style=\"color:blue;\">" + ffeatures.at(1)
                + "</span>&nbsp;Gs<br/>";
        info += tr("Residual flux density") + " (Br): <span style=\"color:blue;\">" + ffeatures.at(2)
                + "</span>&nbsp;Gs<br/>";
        info += tr("Coercive Force") + " (Hc): <span style=\"color:blue;\">" + ffeatures.at(3)
                + "</span>&nbsp;Oe<br/>";
        info += tr("Curie Temperature") + ": <span style=\"color:blue;\">" + ffeatures.at(4)
                + "</span>&nbsp;°C<br/>";
        for (int i = 0; i < 28; i++) {
            QString fsise_str = FToroidSize[i];
            QStringList f_size_val = fsise_str.split(",");
            if (f_size_val[0] == fd_curr_size) {
                QString ID = f_size_val[1];
                QString OD = f_size_val[2];
                QString H = f_size_val[3];
                double id = ID.toDouble()*25.4/fOpt->dwLengthMultiplier;
                double od = OD.toDouble()*25.4/fOpt->dwLengthMultiplier;
                double h = H.toDouble()*25.4/fOpt->dwLengthMultiplier;
                hole_area = M_PI * od / 4;
                int accurasy = fOpt->indexLengthMultiplier + 1;
                if (accurasy == 4)
                    accurasy = 0;
                info += tr("Dimensions") + " (OD x ID x H): <span style=\"color:blue;\"><br/>"
                        + QString::number(id, 'f', accurasy) + " x "
                        + QString::number(od, 'f', accurasy) + " x "
                        + QString::number(h, 'f', accurasy) +
                        + "</span>&nbsp;" + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
                al = FToroid_AL[fm_index][i];
                info += "A<sub>L</sub>&nbsp;" + tr("factor") + ": <span style=\"color:blue;\">"
                        + QString("%1").arg(al) + "</span>&nbsp;nH/N<sup>2</sup></p>";
            }
        }
        info += "<p><u>" + tr("Working frequency") + "</u>:<br/>";
        info += tr("Resonant circuit coils") + " = <span style=\"color:blue;\">" + ffeatures.at(5)
                + "</span>&nbsp;" + qApp->translate("Context", "MHz") + "<br/>";
        info += tr("Wideband transformers") + "&nbsp;(TLT) = <span style=\"color:blue;\">" + ffeatures.at(6)
                + "</span>&nbsp;" + qApp->translate("Context", "MHz") + "<br/>";
        info += tr("Chokes") + " = <span style=\"color:blue;\">" + ffeatures.at(7)
                + "</span>&nbsp;" + qApp->translate("Context", "MHz") + "<br/>";
        ui->label_info->setText(info);
        ui->label->setText(arg1 + "-" + ui->comboBox_fm->currentText());
        onCalculate();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_lineEdit_ind_textChanged()
{
    onCalculate();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int Amidon::decodeColor(QString scolour)
{
    if (scolour == "Clear")
        return 0x2F4F4F;
    if (scolour == "Tan")
        return 0xD2B48C;
    if (scolour == "Blue")
        return 0x0000FF;
    if (scolour == "Red")
        return 0xFF0000;
    if (scolour == "Yellow")
        return 0xFFFF00;
    if (scolour == "White")
        return 0xFFFFFF;
    if (scolour == "Orange")
        return 0xFFA500;
    if (scolour == "Black")
        return 0x000000;
    if (scolour == "Green")
        return 0x008000;
    if (scolour == "Gray")
        return 0x808080;
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::drawToroid(QColor colour1, QColor colour2)
{
    QGraphicsScene *scene = new QGraphicsScene();
    ui->graphArea->setScene(scene);
    QLinearGradient *gradient = new QLinearGradient(QPointF(-100, ui->graphArea->height()/2),
                                                    QPointF(ui->graphArea->width()+100, ui->graphArea->height()/2));
    gradient->setColorAt(0, Qt::white);
    gradient->setColorAt(0.8, colour1);

    QGraphicsEllipseItem *ellipseItem = new QGraphicsEllipseItem();
    ellipseItem->setRect(QRect( 0, 100, 250, 75 ));
    ellipseItem->setPen( QPen(Qt::black) );
    ellipseItem->setBrush( *gradient );
    scene->addItem(ellipseItem);

    QGraphicsRectItem *rectItem = new QGraphicsRectItem();
    rectItem->setRect(0, 100, 250, 40);
    rectItem->setPen( QPen( Qt::NoPen) );
    rectItem->setBrush( *gradient );
    scene->addItem(rectItem);

    QGraphicsLineItem *lineItem = new QGraphicsLineItem(0,100,0,140);
    scene->addItem(lineItem);
    QGraphicsLineItem *lineItem2 = new QGraphicsLineItem(250,100,250,140);
    scene->addItem(lineItem2);

    QGraphicsEllipseItem *ellipseItem2 = new QGraphicsEllipseItem();
    ellipseItem2->setRect(QRect( 0, 60, 250, 75 ));
    ellipseItem2->setPen( QPen(Qt::black) );
    ellipseItem2->setBrush(colour2);
    scene->addItem(ellipseItem2);

    QLinearGradient *gradient2 = new QLinearGradient(QPointF(0, ui->graphArea->height()/2),
                                                    QPointF(ui->graphArea->width(), ui->graphArea->height()/2));
    gradient2->setColorAt(0.1, colour1);
    gradient2->setColorAt(1, Qt::white);

    QGraphicsEllipseItem *ellipseItem3 = new QGraphicsEllipseItem();
    ellipseItem3->setRect(QRect(50, 80, 150, 30));
    ellipseItem3->setPen( QPen(Qt::black) );
    ellipseItem3->setBrush(*gradient2);
    scene->addItem(ellipseItem3);

    QGraphicsEllipseItem *ellipseItem4 = new QGraphicsEllipseItem();
    ellipseItem4->setRect(QRect(50, 105, 150, 40));
    ellipseItem4->setPen( QPen(Qt::NoPen) );
    ellipseItem4->setBrush(Qt::NoBrush);
    scene->addItem(ellipseItem4);

    QPainterPath intersectedPath = ellipseItem3->shape().intersected(ellipseItem4->shape());
    scene->addPath(intersectedPath,QPen(Qt::black),Qt::white);

    QGraphicsDropShadowEffect *shadow1 = new QGraphicsDropShadowEffect();
    shadow1->setColor(Qt::gray);
    shadow1->setOffset(QPointF(10, 5));
    shadow1->setBlurRadius(25);
    ellipseItem->setGraphicsEffect(shadow1);

    ui->graphArea->show();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::drawImage(QPixmap image)
{
    QGraphicsScene *scene = new QGraphicsScene();
    ui->graphArea->setScene(scene);
    scene->addPixmap(image);
    ui->graphArea->show();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::onCalculate()
{
    double ind = loc.toDouble(ui->lineEdit_ind->text())*fOpt->dwInductanceMultiplier;
    ui->label_result->clear();
    QString Result = "";
    int N = 0;
    switch (ui->tabWidget->currentIndex()) {
    case _TToroid:
        N = round(100 * sqrt(ind / al));
        break;
    case _FToroid:
        N = round(sqrt(1000 * ind / al));
        break;
    case _PotCore:
        N = round(1000 * sqrt(ind / al / 1000));
        break;
    case _ECore:
        N = round(1000 * sqrt(ind / al / 1000));
        break;
    case _MCore:
        N = round(1000 * sqrt(ind / al / 1000));
        break;
    default:
        break;
    }
    if (N  > 0){
        double max_dw = 2 * sqrt(0.7 * hole_area / N / M_PI);
        Result += tr("Number of turns of the coil") + " N = " + QString::number(N) + "<br/>";
        Result += tr("Maximum wire diameter") + " dw_max = " + QString::number(max_dw / fOpt->dwLengthMultiplier, 'f', fOpt->dwAccuracy)
                + "&nbsp;" + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8());
        QString awg = converttoAWG(max_dw);
        if (!awg.isEmpty())
            Result += "&nbsp;(" + awg +"&nbsp;AWG)";
    } else {
        Result += tr("Not available");
    }
    ui->label_result->setText(Result);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString Amidon::getPotCoreSize(QString sizes, double *average_size)
{
    QStringList size_list = sizes.split("-");
    QString s_size_min = size_list.at(0);
    QString s_size_max = size_list.at(1);
    double dw_size_min = s_size_min.toDouble();
    double dw_size_max = s_size_max.toDouble();
    double dw_size_average = ((dw_size_max + dw_size_min)/2);
    double deviation = (dw_size_max - dw_size_average);
    int accurasy = fOpt->indexLengthMultiplier + 1;
    if (accurasy == 4)
        accurasy = 0;
    QString sResult = QString::number(dw_size_average/fOpt->dwLengthMultiplier, 'f', accurasy) + "±"
            + QString::number(deviation/fOpt->dwLengthMultiplier, 'f', accurasy);
    *average_size = dw_size_average;
    return sResult;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString Amidon::getMultiapertureCoreSize(QString sizes)
{
    QStringList size_list = sizes.split("±");
    QString s_size = size_list.at(0);
    QString s_deviation = size_list.at(1);
    double dw_size = s_size.toDouble();
    double dw_deviation = s_deviation.toDouble();
    int accurasy = fOpt->indexLengthMultiplier + 1;
    if (accurasy == 4)
        accurasy = 0;
    QString sResult = QString::number(dw_size/fOpt->dwLengthMultiplier, 'f', accurasy) + "±"
            + QString::number(dw_deviation/fOpt->dwLengthMultiplier, 'f', accurasy);
    return sResult;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::resolvePotCore(int index)
{
    if ((ui->tabWidget->currentIndex() == 3) && (fOpt->mainFontSize > 0)){
        double A, B, C, D, E, F, G, H;
        QString psize_str = PotCoreSize[index];
        QStringList psize = psize_str.split(",");
        QString info = "<p>" + tr("Dimensions") + ":</span><br/>";
        info += "A = <span style=\"color:blue;\">" + getPotCoreSize(psize.at(0),&A) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;";
        info += "B = <span style=\"color:blue;\">" + getPotCoreSize(psize.at(1),&B) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;<br/>";
        info += "C = <span style=\"color:blue;\">" + getPotCoreSize(psize.at(2),&C) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;";
        info += "D = <span style=\"color:blue;\">" + getPotCoreSize(psize.at(3),&D) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;<br/>";
        info += "E = <span style=\"color:blue;\">" + getPotCoreSize(psize.at(4),&E) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;";
        info += "F = <span style=\"color:blue;\">" + getPotCoreSize(psize.at(5),&F) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;<br/>";
        info += "G = <span style=\"color:blue;\">" + getPotCoreSize(psize.at(6),&G) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;";
        info += "H = <span style=\"color:blue;\">" + getPotCoreSize(psize.at(7),&H) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "&nbsp;<br/>";
        QString pfeatures_str = PotCore_Features[index];
        QStringList pfeatures = pfeatures_str.split(",");
        QString al_s = pfeatures.at(0);
        QString le_s = pfeatures.at(1);
        QString ae_s = pfeatures.at(2);
        QString ve_s = pfeatures.at(3);
        al = al_s.toDouble();
        hole_area = (C - D) * F;
        double le = le_s.toDouble();
        double ae = ae_s.toDouble();
        double ve = ve_s.toDouble();
        int accurasy = fOpt->indexLengthMultiplier + 1;
        if (accurasy == 4)
            accurasy = 0;
        info += tr("Effective magnetic path length") + " (l<sub>e</sub>): <span style=\"color:blue;\">"
                + QString::number(le/fOpt->dwLengthMultiplier, 'f', accurasy)
                + "</span>&nbsp;" + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
        info += tr("Effective area of magnetic path") + " (A<sub>e</sub>): <span style=\"color:blue;\">"
                + QString::number(ae/(fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier), 'f', accurasy)
                + "</span>&nbsp;" + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<sup>2</sup><br/>";
        info += tr("Effective volume") + " (V<sub>e</sub>): <span style=\"color:blue;\">"
                + QString::number(ve/(fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier), 'f', accurasy)
                + "</span>&nbsp;" + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<sup>3</sup><br/>";
        info += "A<sub>L</sub>&nbsp;" + tr("factor") + ": <span style=\"color:blue;\">"
                + al_s + "</span>&nbsp;µH/(N/1000)<sup>2</sup></p>";
        ui->label_info->setText(info);
        onCalculate();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::resolveECore(int index)
{
    if ((ui->tabWidget->currentIndex() == 2) && (fOpt->mainFontSize > 0)){
        QString esize_str = ECoreSize[index];
        QStringList esize = esize_str.split(",");
        QString sA = esize.at(0);
        QString sB = esize.at(1);
        QString sC = esize.at(2);
        QString sD = esize.at(3);
        QString sE = esize.at(4);
        QString sF = esize.at(5);
        QString sG = esize.at(6);
        double A = sA.toDouble() * 25.4;
        double B = sB.toDouble() * 25.4;
        double C = sC.toDouble() * 25.4;
        double D = sD.toDouble() * 25.4;
        double E = sE.toDouble() * 25.4;
        double F = sF.toDouble() * 25.4;
        double G = sG.toDouble() * 25.4;
        int accurasy = fOpt->indexLengthMultiplier + 1;
        if (accurasy == 4)
            accurasy = 0;
        QString info = "<p>" + tr("Dimensions") + ":</span><br/>";
        info += "A = <span style=\"color:blue;\">"
                + QString::number(A/fOpt->dwLengthMultiplier, 'f', accurasy) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;";
        info += "B = <span style=\"color:blue;\">"
                + QString::number(B/fOpt->dwLengthMultiplier, 'f', accurasy) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;<br/>";
        info += "C = <span style=\"color:blue;\">"
                + QString::number(C/fOpt->dwLengthMultiplier, 'f', accurasy) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;";
        info += "D = <span style=\"color:blue;\">"
                + QString::number(D/fOpt->dwLengthMultiplier, 'f', accurasy) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;<br/>";
        info += "E = <span style=\"color:blue;\">"
                + QString::number(E/fOpt->dwLengthMultiplier, 'f', accurasy) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;";
        info += "F = <span style=\"color:blue;\">"
                + QString::number(F/fOpt->dwLengthMultiplier, 'f', accurasy) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + ",&nbsp;<br/>";
        info += "G = <span style=\"color:blue;\">"
                + QString::number(G/fOpt->dwLengthMultiplier, 'f', accurasy) + "</span>&nbsp;"
                + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "&nbsp;<br/>";
        QString efeatures_str = ECore_Features[index];
        QStringList efeatures = efeatures_str.split(",");
        QString ae_s = efeatures.at(0);
        QString le_s = efeatures.at(1);
        QString ve_s = efeatures.at(2);
        QString W_s = efeatures.at(3);
        QString al_s = efeatures.at(4);
        al = al_s.toDouble();
        hole_area = 2 * D * F;
        double le = le_s.toDouble();
        double ae = ae_s.toDouble();
        double ve = ve_s.toDouble();
        double W = W_s.toDouble();
        info += tr("Effective magnetic path length") + " (l<sub>e</sub>): <span style=\"color:blue;\">"
                + QString::number(le/fOpt->dwLengthMultiplier, 'f', accurasy)
                + "</span>&nbsp;" + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
        info += tr("Effective area of magnetic path") + " (A<sub>e</sub>): <span style=\"color:blue;\">"
                + QString::number(ae/(fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier), 'f', accurasy)
                + "</span>&nbsp;" + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<sup>2</sup><br/>";
        info += tr("Effective volume") + " (V<sub>e</sub>): <span style=\"color:blue;\">"
                + QString::number(ve/(fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier), 'f', accurasy)
                + "</span>&nbsp;" + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<sup>3</sup><br/>";
        info += tr("Power") + " (W): <span style=\"color:blue;\">" + QString::number(W, 'f', accurasy) + "</span>&nbsp;W<br/>";
        info += "A<sub>L</sub>&nbsp;" + tr("factor") + ": <span style=\"color:blue;\">"
                + al_s + "</span>&nbsp;µH/(N/1000)<sup>2</sup></p>";
        ui->label_info->setText(info);
        onCalculate();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_pushButton_export_clicked()
{
    double I = loc.toDouble(ui->lineEdit_ind->text())*fOpt->dwInductanceMultiplier;
    if (I == 0){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        return;
    }
    QString sResult = "<hr><h2>" +QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle() + "</h2><br/>";
    sResult += "<p><u>" + tr("Selected core") + ":</u><br/>";
    sResult += "<b>" + ui->label->text() + "</b></p><p>";
    if (fOpt->isInsertImage){
        QPixmap image = ui->graphArea->grab();
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        image.save(&buffer, "PNG");
        sResult += QString("<img src=\"data:image/png;base64,") + byteArray.toBase64() + "\"/></p><p>";
        QByteArray byteArray2;
        QBuffer buffer2(&byteArray2);
        ui->label_desc->pixmap()->save(&buffer2, "PNG");
        sResult += QString("<img src=\"data:image/png;base64,") + byteArray2.toBase64() + "\"/></p>";
    }
    sResult += "<p><u>" + tr("Input data") + ":</u><br/>";
    sResult += ui->groupBox_ind->title() + " = " + ui->lineEdit_ind->text() + " " + ui->label_2->text() + "</p>";
    sResult += "<p><u>" + ui->groupBox_info->title() + ":</u>";
    sResult += ui->label_info->text() + "</p>";
    sResult += "<p><u>" + ui->groupBox_result->title() + ":</u><br/>";
    sResult += ui->label_result->text() + "</p><hr>";
    emit sendResult(sResult);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_pushButton_help_clicked()
{
    int tab = ui->tabWidget->currentIndex();
    switch (tab) {
    case 0:
        QDesktopServices::openUrl(QUrl("http://www.amidoncorp.com/iron-powder-toroids"));
        break;
    case 1:
        QDesktopServices::openUrl(QUrl("http://www.amidoncorp.com/ferrite-toroids"));
        break;
    case 2:
        QDesktopServices::openUrl(QUrl("http://www.amidoncorp.com/e-core-sets"));
        break;
    case 3:
        QDesktopServices::openUrl(QUrl("http://www.amidoncorp.com/pot-core-sets"));
        break;
    default:
        break;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_radioButton_p01_clicked(bool checked)
{
    if (checked){
        ui->label->setText(ui->radioButton_p01->text());
        resolvePotCore(0);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_radioButton_p02_clicked(bool checked)
{
    if (checked){
        ui->label->setText(ui->radioButton_p02->text());
        resolvePotCore(1);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_radioButton_p03_clicked(bool checked)
{
    if (checked){
        ui->label->setText(ui->radioButton_p03->text());
        resolvePotCore(2);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_radioButton_p04_clicked(bool checked)
{
    if (checked){
        ui->label->setText(ui->radioButton_p04->text());
        resolvePotCore(3);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_radioButton_p05_clicked(bool checked)
{
    if (checked){
        ui->label->setText(ui->radioButton_p05->text());
        resolvePotCore(4);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_radioButton_p06_clicked(bool checked)
{
    if (checked){
        ui->label->setText(ui->radioButton_p06->text());
        resolvePotCore(5);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_radioButton_p07_clicked(bool checked)
{
    if (checked){
        ui->label->setText(ui->radioButton_p07->text());
        resolvePotCore(6);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_radioButton_e01_clicked(bool checked)
{
    if (checked){
        ui->label->setText(ui->radioButton_e01->text());
        resolveECore(0);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_radioButton_e02_clicked(bool checked)
{
    if (checked){
        ui->label->setText(ui->radioButton_e02->text());
        resolveECore(1);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_radioButton_e03_clicked(bool checked)
{
    if (checked){
        ui->label->setText(ui->radioButton_e03->text());
        resolveECore(2);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_radioButton_e04_clicked(bool checked)
{
    if (checked){
        ui->label->setText(ui->radioButton_e04->text());
        resolveECore(3);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Amidon::on_radioButton_e05_clicked(bool checked)
{
    if (checked){
        ui->label->setText(ui->radioButton_e05->text());
        resolveECore(4);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
