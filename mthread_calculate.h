/* mthread_calculate.h - header text to Coil64 - Radio frequency inductor and choke calculator
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

#ifndef MTHREAD_CALCULATE_H
#define MTHREAD_CALCULATE_H

#include <QThread>
#include <QMetaType>

#include "resolves.h"
#include "resolve_q.h"
#include "resolve_srf_cs.h"
#include "multiloop_utils.h"


class MThread_calculate : public QThread
{
    Q_OBJECT
public:
    explicit MThread_calculate(int _coilForm, int _tab, double _arg1, double _arg2, double _arg3, double _arg4,
                               double _arg5, double _arg6, double _arg7, double _arg8, Material _mt = Cu, double _tand = 0,
                               double _arg9 = 0, double _arg10 = 0, double _arg11 = 0, double _arg12 = 0, bool st1 = false, bool st2 = false, int _index = 0);
    void run();
signals:
    void sendResult(_CoilResult);
public slots:
    void abort();
private:
    bool isAbort;
    bool st1;
    bool st2;
    int coilForm;
    int tab;
    int index;
    double arg1;
    double arg2;
    double arg3;
    double arg4;
    double arg5;
    double arg6;
    double arg7;
    double arg8;
    double arg9;
    double arg10;
    double arg11;
    double arg12;
    double tand;
    Material mt;
};

Q_DECLARE_METATYPE(_CoilResult)

#endif // MTHREAD_CALCULATE_H
