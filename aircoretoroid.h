/* aircoretoroid.h - header text to Coil64 - Radio frequency inductor and choke calculator
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

#ifndef AIRCORETOROID_H
#define AIRCORETOROID_H

#include <QDialog>
#include <QScreen>
#include <QDoubleValidator>
#include <QString>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>

#include "system_functions.h"
#include "resolves.h"

namespace Ui {
class AirCoreToroid;
}

class AirCoreToroid : public QDialog
{
    Q_OBJECT

public:
    explicit AirCoreToroid(QWidget *parent = 0);
    ~AirCoreToroid();
signals:
    void sendResult(QString);

private slots:
    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);
    void on_pushButton_close_clicked();
    void on_pushButton_help_clicked();
    void on_radioButton_round_clicked();
    void on_radioButton_rect_clicked();
    void on_checkBox_isReverce_clicked();
    void on_pushButton_calculate_clicked();

private:
    Ui::AirCoreToroid *ui;
    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QLocale loc;

    int windingKind;
    double N, ind, D1, D2, h, dw;
};

#endif // AIRCORETOROID_H
