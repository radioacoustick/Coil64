/* amidon.h - header text to Coil64 - Radio frequency inductor and choke calculator
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
#ifndef AMIDON_H
#define AMIDON_H

#include <QDialog>
#include <QScreen>
#include <QPainter>
#include <QSettings>
#include <QDoubleValidator>
#include <QDesktopServices>
#include <QUrl>
#include <QBuffer>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsDropShadowEffect>

#include <math.h>
#include "system_functions.h"
#include "resolves.h"

namespace Ui {
class Amidon;
}

class Amidon : public QDialog
{
    Q_OBJECT

public:
    explicit Amidon(QWidget *parent = 0);
    ~Amidon();

signals:
    void sendResult(QString);

private slots:
    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);

    void on_pushButton_close_clicked();
    void on_pushButton_export_clicked();
    void on_pushButton_help_clicked();
    void on_comboBox_tm_currentIndexChanged(int index);
    void on_comboBox_fm_currentIndexChanged(int index);
    void on_comboBox_bn_m_currentIndexChanged(int index);
    void on_comboBox_bn_t_currentIndexChanged(int index);
    void on_tabWidget_currentChanged(int index);
    void on_comboBox_fd_currentTextChanged(const QString &arg1);
    void on_comboBox_td_currentTextChanged(const QString &arg1);
    void on_lineEdit_ind_textChanged();
    void on_radioButton_p01_clicked(bool checked);
    void on_radioButton_p02_clicked(bool checked);
    void on_radioButton_p03_clicked(bool checked);
    void on_radioButton_p04_clicked(bool checked);
    void on_radioButton_p05_clicked(bool checked);
    void on_radioButton_p06_clicked(bool checked);
    void on_radioButton_p07_clicked(bool checked);
    void on_radioButton_e01_clicked(bool checked);
    void on_radioButton_e02_clicked(bool checked);
    void on_radioButton_e03_clicked(bool checked);
    void on_radioButton_e04_clicked(bool checked);
    void on_radioButton_e05_clicked(bool checked);

    void drawToroid(QColor colour1, QColor colour2);
    void drawImage(QPixmap image);
    void onCalculate();
    unsigned int decodeColor(QString scolour);
    QString getPotCoreSize(QString sizes, double *average_size = NULL);
    QString getMultiapertureCoreSize(QString sizes);
    void resolvePotCore(int index);
    void resolveECore(int index);

private:
    Ui::Amidon *ui;
    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QLocale loc;
    QString styleInfoColor;
    double al;
    double hole_area;
};
extern QString FToroidSize[];

#endif // AMIDON_H
