/* options.h - header text to Coil64 - Radio frequency inductor and choke calculator
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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <QDialog>
#include <QMetaType>
#include <QAbstractButton>
#include <QScreen>

#include "system_functions.h"
#include "mainwindow.h"

namespace Ui {
class Options;
}

class Options : public QDialog
{
    Q_OBJECT

public:
    explicit Options(QWidget *parent = 0);
    ~Options();

signals:
    void sendOpt(_OptionStruct);


private slots:
    void getOpt(_OptionStruct gOpt);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();

private:
    Ui::Options *ui;
    _OptionStruct *oOpt;
};

Q_DECLARE_METATYPE(_OptionStruct)
#endif // OPTIONS_H
