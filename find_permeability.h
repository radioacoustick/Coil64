/* find_permeability.h - header text to Coil64 - Radio frequency inductor and choke calculator
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

#ifndef FIND_PERMEABILITY_H
#define FIND_PERMEABILITY_H

#include <QDialog>
#include <QAbstractButton>
#include <QString>
#include <QSettings>


#include "system_functions.h"
#include "resolves.h"

namespace Ui {
class Find_Permeability;
}

class Find_Permeability : public QDialog
{
    Q_OBJECT

public:
    explicit Find_Permeability(QWidget *parent = 0);
    ~Find_Permeability();

signals:
    void sendResult(QString);

private slots:
    void getOpt(_OptionStruct gOpt);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::Find_Permeability *ui;
    _OptionStruct *fOpt;
};

#endif // FIND_PERMEABILITY_H
