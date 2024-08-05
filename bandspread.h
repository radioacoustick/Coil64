/* bandspread.h - header text to Coil64 - Radio frequency inductor and choke calculator
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
#ifndef BANDSPREAD_H
#define BANDSPREAD_H

#include <QtWidgets>
#include <QDialog>
#include <QScreen>
#include <QDoubleValidator>
#include <QString>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QtMath>

#include "system_functions.h"
#include "resolves.h"
#include "bandspread_utils.h"

namespace Ui {
class Bandspread;
}

class Bandspread : public QDialog
{
    Q_OBJECT

public:
    explicit Bandspread(QWidget *parent = 0);
    ~Bandspread();

signals:
    void sendResult(QString);

private slots:
    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);
    void on_tabWidget_currentChanged(int index);
    void on_pushButton_close_clicked();
    void on_pushButton_help_clicked();
    void on_pushButton_calculate_clicked();
    void on_pushButton_export_clicked();
    void on_lineEdit_flo_editingFinished();
    void on_lineEdit_fhi_editingFinished();
    void on_lineEdit_cvmin_editingFinished();
    void on_lineEdit_cvmax_editingFinished();
    void on_lineEdit_cs_editingFinished();
    void on_lineEdit_ind_editingFinished();
    void on_lineEdit_flo_t_editingFinished();
    void on_lineEdit_fhi_t_editingFinished();
    void on_lineEdit_IF_editingFinished();
    void on_lineEdit_cvmin_t_editingFinished();
    void on_lineEdit_cvmax_t_editingFinished();
    void on_lineEdit_cs_t_editingFinished();
    void on_lineEdit_Le_editingFinished();
    void on_horizontalSlider_L_valueChanged(int value);
    void on_horizontalSlider_Le_valueChanged(int value);
    void showInductanceRange();
    void on_toolButton_plus_clicked();
    void on_toolButton_minus_clicked();
    void on_toolButton_plus_L_clicked();
    void on_toolButton_minus_L_clicked();
    void on_checkBox_toggled(bool checked);

private:
    Ui::Bandspread *ui;
    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QLocale loc;
    int L_value_pos;
    double ind, f_low, f_high, f_i, Cv_min, Cv_max, Cs, Ct = 0, Cp = 0;
};

#endif // BANDSPREAD_H
