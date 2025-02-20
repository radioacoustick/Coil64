/* loop.h - header text to Coil64 - Radio frequency inductor and choke calculator
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

#ifndef LOOP_H
#define LOOP_H

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
class Loop;
}

class Loop : public QDialog
{
    Q_OBJECT

public:
    explicit Loop(QWidget *parent = 0);
    ~Loop();
signals:
    void sendResult(QString);

private slots:
    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);

    void on_radioButton_round_clicked();
    void on_radioButton_triangle_clicked();
    void on_radioButton_rectangle_clicked();
    void on_checkBox_isReverce_clicked();

    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

private:
    Ui::Loop *ui;

    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QRegExpValidator *awgV;
    QLocale loc;

    int loopKind;
    double ind, a, b, dw, lw, mu;
};

#endif // LOOP_H
