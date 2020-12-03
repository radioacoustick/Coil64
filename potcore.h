/* potcore.h - header text to Coil64 - Radio frequency inductor and choke calculator
Copyright (C) 2020 Kustarev V.

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

#ifndef POTCORE_H
#define POTCORE_H

#include <QDialog>
#include <QScreen>
#include <QSettings>
#include <QDoubleValidator>
#include <QDesktopServices>
#include <QUrl>

#include "system_functions.h"
#include "resolves.h"


namespace Ui {
class PotCore;
}

class PotCore : public QDialog
{
    Q_OBJECT

public:
    explicit PotCore(QWidget *parent = 0);
    ~PotCore();

signals:
    void sendResult(QString);

private slots:
    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);
    QString getPotCoreSize(QString sizes);
    void on_pushButton_close_clicked();
    void on_checkBox_isReverce_clicked();
    void on_comboBox_currentIndexChanged(int index);
    void on_pushButton_help_clicked();
    void on_pushButton_calculate_clicked();

private:
    Ui::PotCore *ui;
    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QLocale loc;
    double N, ind, d1, d2, d3, d4, h1, h2, b, g, mu;
    int currStdCore;
};

#endif // POTCORE_H
