/* al.h - header text to Coil64 - Radio frequency inductor and choke calculator
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
#ifndef AL_H
#define AL_H

#include <QDialog>
#include <QScreen>
#include <QSettings>
#include <QDoubleValidator>
#include <QDesktopServices>
#include <QUrl>

#include "system_functions.h"
#include "math.h"

namespace Ui {
class AL;
}

class AL : public QDialog
{
    Q_OBJECT

public:
    explicit AL(QWidget *parent = 0);
    ~AL();

private slots:
    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);
    void on_pushButton_close_clicked();
    void on_tabWidget_currentChanged(int index);
    void on_pushButton_calculate_clicked();

private:
    Ui::AL *ui;
    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QLocale loc;
    int tabIndex;
    int al, N_m, N_r;
    double L_m, L_r;
};

#endif // AL_H
