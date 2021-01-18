/* multiloop.h - header text to Coil64 - Radio frequency inductor and choke calculator
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

#ifndef MULTILOOP_H
#define MULTILOOP_H

#include <QDialog>
#include <QScreen>
#include <QThread>
#include <QDoubleValidator>
#include <QString>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>

#include "system_functions.h"
#include "resolves.h"

namespace Ui {
class Multiloop;
}

class Multiloop : public QDialog
{
    Q_OBJECT

public:
    explicit Multiloop(QWidget *parent = 0);
    ~Multiloop();

signals:
    void sendResult(QString);

private slots:

    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_lineEdit_2_editingFinished();
    void on_checkBox_isReverce_clicked();

private:
    Ui::Multiloop *ui;
    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QRegExpValidator *awgV;
    QLocale loc;
    double ind;
    double dw;
    int nTurns;
};

#endif // MULTILOOP_H
