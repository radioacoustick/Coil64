/* crossover.h - header text to Coil64 - Radio frequency inductor and choke calculator
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

#ifndef CROSSOVER_H
#define CROSSOVER_H

#include <QDialog>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QScreen>
#include <QSettings>
#include <QDoubleValidator>
#include <QDesktopServices>
#include <QUrl>
#include <QMenu>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <bitset>

#include "system_functions.h"
#include "resolves.h"

namespace Ui {
class Crossover;
}

class Crossover : public QDialog
{
    Q_OBJECT

public:
    explicit Crossover(QWidget *parent = 0);
    ~Crossover();

private slots:
    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);
    void switchColumnVisible();
    void on_removeRow();
    void fillTable(QStandardItem *item, int count, double wire_d);

    void on_comboBox_currentIndexChanged(int index);
    void on_lineEdit_ind_editingFinished();
    void on_comboBox_N_m_currentIndexChanged(int index);
    void on_tableView_customContextMenuRequested(const QPoint &pos);
    void tableHeaderCustomContextMenuRequested(const QPoint &pos);

    void on_pushButton_calculate_clicked();
    void on_toolButton_Help_clicked();
    void on_toolButton_Save_clicked();
    void on_toolButton_Clear_clicked();
    void on_pushButton_close_clicked();


private:
    Ui::Crossover *ui;
    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QLocale loc;
    QStandardItemModel *model;
    QStandardItem *item;
    QMenu *tablepopupmenu;
    QMenu *headerpopupmenu;
    std::bitset<8> header_bits;
    double ind;
};

#endif // CROSSOVER_H
