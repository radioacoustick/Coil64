/* al.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "al.h"
#include "ui_al.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AL::AL(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AL)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, MAX_DOUBLE, 380);
    ui->lineEdit_L_1->setValidator(dv);
    ui->lineEdit_L_2->setValidator(dv);
    ui->lineEdit_L_3->setValidator(dv);
    ui->lineEdit_N_1->setValidator(dv);
    ui->lineEdit_N_2->setValidator(dv);
    ui->lineEdit_N_3->setValidator(dv);
    ui->lineEdit_AL_1->setValidator(dv);
    ui->lineEdit_AL_2->setValidator(dv);
    ui->lineEdit_AL_3->setValidator(dv);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AL::~AL()
{
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "AL" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", this->size());
    settings->setValue("tabIndex", tabIndex);
    settings->setValue("al", al);
    settings->setValue("N_m", N_m);
    settings->setValue("N_r", N_r);
    settings->setValue("L_m", L_m);
    settings->setValue("L_r", L_r);
    settings->endGroup();
    delete settings;
    delete fOpt;
    delete dv;
    delete ui;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AL::getOpt(_OptionStruct gOpt)
{
    *fOpt = gOpt;
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "AL" );
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    tabIndex = settings->value("tabIndex", 0).toInt();
    al = settings->value("al", 0).toDouble();
    N_m = settings->value("N_m", 0).toDouble();
    N_r = settings->value("N_r", 0).toDouble();
    L_m = settings->value("L_m", 0).toDouble();
    L_r = settings->value("L_r", 0).toDouble();
    settings->endGroup();
    QString nanoH = qApp->translate("Context","nH");
    ui->label_AL_01->setText(nanoH + "/" + tr("turn") + "<sup>2</sup>");
    ui->label_AL_02->setText(nanoH + "/" + tr("turn") + "<sup>2</sup>");
    ui->label_AL_03->setText(nanoH + "/" + tr("turn") + "<sup>2</sup>");
    ui->label_AL_1->setText(tr("Magnetic factor of the core") + " A<sub>L</sub>:");
    ui->label_AL_2->setText(tr("Magnetic factor of the core") + " A<sub>L</sub>:");
    ui->label_AL_3->setText(tr("Magnetic factor of the core") + " A<sub>L</sub>:");
    ui->label_L_01->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_L_02->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_L_03->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_L_1->setText(tr("Inductance") + " L:");
    ui->label_L_2->setText(tr("Inductance") + " L:");
    ui->label_L_3->setText(tr("Inductance") + " L:");
    ui->label_N_1->setText(tr("Number of turns") + " N:");
    ui->label_N_2->setText(tr("Number of turns") + " N:");
    ui->label_N_3->setText(tr("Number of turns") + " N:");
    ui->tabWidget->setCurrentIndex(tabIndex);
    on_tabWidget_currentChanged(tabIndex);
    resize(size);
    move(pos);
    delete settings;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AL::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AL::on_pushButton_close_clicked()
{
    this->close();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AL::on_tabWidget_currentChanged(int index)
{
    tabIndex = index;
    switch (index) {
    case 0:
        ui->lineEdit_L_1->setText(loc.toString(L_m / fOpt->dwInductanceMultiplier));
        ui->lineEdit_N_1->setText(loc.toString(N_m));
        break;
    case 1:
        ui->lineEdit_AL_2->setText(loc.toString(al));
        ui->lineEdit_L_2->setText(loc.toString(L_r / fOpt->dwInductanceMultiplier));
        break;
    case 2:
        ui->lineEdit_AL_3->setText(loc.toString(al));
        ui->lineEdit_N_3->setText(loc.toString(N_r));
        break;
    default:
        break;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AL::on_pushButton_calculate_clicked()
{
    bool ok1 = false;
    bool ok2 = false;
    switch (tabIndex) {
    case 0:
        if ((ui->lineEdit_L_1->text().isEmpty())||(ui->lineEdit_N_1->text().isEmpty())){
            showWarning(tr("Warning"), tr("One or more inputs are empty!"));
            return;
        }
        L_m = loc.toDouble(ui->lineEdit_L_1->text(), &ok1) * fOpt->dwInductanceMultiplier;
        N_m = loc.toInt(ui->lineEdit_N_1->text(), &ok2);
        if((!ok1)||(!ok2)){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if ((L_m == 0)||(N_m == 0)){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        al = L_m * 1000 / (N_m * N_m);
        ui->lineEdit_AL_1->setText(loc.toString(al));
        break;
    case 1:
        if ((ui->lineEdit_L_2->text().isEmpty())||(ui->lineEdit_AL_2->text().isEmpty())){
            showWarning(tr("Warning"), tr("One or more inputs are empty!"));
            return;
        }
        L_r = loc.toDouble(ui->lineEdit_L_2->text(), &ok1) * fOpt->dwInductanceMultiplier;
        al = loc.toInt(ui->lineEdit_AL_2->text(), &ok2);
        if((!ok1)||(!ok2)){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if ((L_r == 0)||(al == 0)){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        N_r = sqrt(L_r * 1000 / al);
        ui->lineEdit_N_2->setText(loc.toString(N_r));
        break;
    case 2:
        if ((ui->lineEdit_N_3->text().isEmpty())||(ui->lineEdit_AL_3->text().isEmpty())){
            showWarning(tr("Warning"), tr("One or more inputs are empty!"));
            return;
        }
        N_r = loc.toInt(ui->lineEdit_N_3->text(), &ok1);
        al = loc.toInt(ui->lineEdit_AL_3->text(), &ok2);
        if((!ok1)||(!ok2)){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if ((N_r == 0)||(al == 0)){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        L_r = (double)N_r * N_r * al / 1000;
        ui->lineEdit_L_3->setText(roundTo(L_r / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
        break;
    default:
        break;
    }
}
