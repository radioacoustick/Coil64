/* rf_toroid.h - header text to Coil64 - Radio frequency inductor and choke calculator
Copyright (C) 2021 Kustarev V.

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

#ifndef RF_TOROID_H
#define RF_TOROID_H

#include <QDialog>
#include <QFile>
#include <QTextStream>
#include <QScreen>
#include <QDoubleValidator>
#include <complex>

#include "system_functions.h"
#include "resolves.h"
#include "spline.h"
#include "amidon.h"

using std::complex;

namespace Ui {
class RF_Toroid;
}

class RF_Toroid : public QDialog
{
    Q_OBJECT

public:
    explicit RF_Toroid(QWidget *parent = 0);
    ~RF_Toroid();
    std::vector<double> readMaterialData(QString fileName, QStringList *lDataList, int column);

signals:
    void sendResult(QString);

private slots:
    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);
    double getDCresistance(double N, double OD, double ID, double H, double dw);
    void on_pushButton_close_clicked();
    void on_comboBox_currentIndexChanged(int index);
    void on_comboBox_2_currentIndexChanged(int index);
    void on_lineEdit_N_editingFinished();
    void on_lineEdit_f_editingFinished();
    void on_lineEdit_cs_editingFinished();
    void on_label_cs_toggled(bool checked);
    void on_lineEdit_od_editingFinished();
    void on_lineEdit_id_editingFinished();
    void on_lineEdit_h_editingFinished();
    void on_lineEdit_c_editingFinished();
    void on_lineEdit_d_editingFinished();
    void on_lineEdit_mu1_editingFinished();
    void on_lineEdit_mu2_editingFinished();
    void on_pushButton_calculate_clicked();
    void on_pushButton_export_clicked();
    void on_pushButton_help_clicked();

private:
    Ui::RF_Toroid *ui;
    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QLocale loc;
    QString styleInfoColor;
    double N, f, Cs, od, id, h, c, d, mu1, mu2;
    bool isCsAuto = false;
};

#endif // RF_TOROID_H
