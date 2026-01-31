/* coupledcoil.h - header text to Coil64 - Radio frequency inductor and choke calculator
Copyright (C) 2025 Kustarev V.

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
#ifndef COUPLEDCOIL_H
#define COUPLEDCOIL_H

#include <QDialog>
#include <QScreen>
#include <QDoubleValidator>
#include <QString>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>

#include "mthread_calculate.h"
#include "system_functions.h"
#include "resolves.h"

namespace Ui {
class CoupledCoil;
}

class CoupledCoil : public QDialog
{
    Q_OBJECT

public:
    explicit CoupledCoil(QWidget *parent = nullptr);
    ~CoupledCoil();

signals:
    void sendResult(QString);
    void showLtspice(bool);

private slots:
    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);
    void on_pushButton_close_clicked();
    void on_pushButton_help_clicked();
    void on_pushButton_calculate_clicked();
    void get_CoupledCoils_Result(_CoilResult result);
    void on_timer();
    void on_calculation_started();
    void on_calculation_finished();
    void on_tabWidget_currentChanged(int index);
    void on_toolButton_spice_toggled(bool checked);

private:
    Ui::CoupledCoil *ui;

    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QIntValidator *iv;
    QRegExpValidator *awgV;
    QLocale loc;
    MThread_calculate *thread;
    QTimer *timer;
    double D1, D2, dw, l1, l2, xs, D, p, N, Nt, f, Rload;
    int N1, N2;
};

#endif // COUPLEDCOIL_H
