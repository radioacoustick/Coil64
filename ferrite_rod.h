/* ferrite_rod.h - header text to Coil64 - Radio frequency inductor and choke calculator
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

#ifndef FERRITE_ROD_H
#define FERRITE_ROD_H

#include <QDialog>
#include <QScreen>
#include <QSettings>
#include <QDoubleValidator>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>

#include "mthread_calculate.h"
#include "system_functions.h"
#include "resolves.h"

namespace Ui {
class Ferrite_Rod;
}

class Ferrite_Rod : public QDialog
{
    Q_OBJECT

public:
    explicit Ferrite_Rod(QWidget *parent = 0);
    ~Ferrite_Rod();

signals:
    void sendResult(QString);

private slots:
    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);
    void on_pushButton_close_clicked();
    void on_pushButton_calculate_clicked();
    void get_ferriteRod_Result(_CoilResult result);
    void on_pushButton_clicked();
    void on_lineEdit_dw_editingFinished();
    void on_timer();
    void on_calculation_started();
    void on_calculation_finished();

private:
    Ui::Ferrite_Rod *ui;
    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QRegExpValidator *awgV;
    QLocale loc;
    MThread_calculate *thread;
    QTimer *timer;
};

#endif // FERRITE_ROD_H
