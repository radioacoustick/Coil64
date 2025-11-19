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

#include "mthread_calculate.h"
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
    void on_pushButton_calculate_clicked();
    void get_Multiloop_Result(_CoilResult result);
    QString showCoilParameters(double T, double dw, double wlength, Material mt, QString head = "");
    void on_pushButton_export_clicked();
    void on_calculation_finished();
    void on_pushButton_close_clicked();
    void on_pushButton_help_clicked();
    void on_lineEdit_2_editingFinished();
    void on_lineEdit_dw_editingFinished();
    void on_lineEdit_dw2_editingFinished();
    void on_lineEdit_Di_editingFinished();
    void on_checkBox_isReverce_clicked();
    void on_tabWidget_currentChanged(int index);
    void on_toolButton_toggled(bool checked);
    void on_checkBox_identical_toggled(bool checked);

private:
    Ui::Multiloop *ui;
    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QRegExpValidator *awgV;
    QLocale loc;
    double ind;
    double dw, dt;
    double nTurns;
    double Di;
    bool isPlus, isIdentical;
    MThread_calculate *thread;
};

#endif // MULTILOOP_H
