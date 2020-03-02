/* shield.h - header text to Coil64 - Radio frequency inductor and choke calculator
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

#ifndef SHIELD_H
#define SHIELD_H

#include <QDialog>
#include <QScreen>
#include <QThread>
#include <QDoubleValidator>
#include <QString>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <math.h>

#include "system_functions.h"
#include "resolves.h"

namespace Ui {
class Shield;
}

class Shield : public QDialog
{
    Q_OBJECT

public:
    explicit Shield(QWidget *parent = 0);
    ~Shield();

signals:
    void sendResult(QString);

private slots:
    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_radioButton_2_clicked();

    void on_radioButton_clicked();

    void on_pushButton_clicked();

private:
    Ui::Shield *ui;
    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QLocale loc;
};

#endif // SHIELD_H
