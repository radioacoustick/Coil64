/* find_permeability.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "find_permeability.h"
#include "ui_find_permeability.h"



Find_Permeability::Find_Permeability(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Find_Permeability)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator;
    ui->lineEdit_ind->setValidator(dv);
    ui->lineEdit_N->setValidator(dv);
    ui->lineEdit_1->setValidator(dv);
    ui->lineEdit_2->setValidator(dv);
    ui->lineEdit_3->setValidator(dv);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Find_Permeability::~Find_Permeability()
{
    double I = loc.toDouble(ui->lineEdit_ind->text())*fOpt->dwInductanceMultiplier;
    double N = loc.toDouble(ui->lineEdit_N->text());
    double D1 = loc.toDouble(ui->lineEdit_1->text())*fOpt->dwLengthMultiplier;
    double D2 = loc.toDouble(ui->lineEdit_2->text())*fOpt->dwLengthMultiplier;
    double h = loc.toDouble(ui->lineEdit_3->text())*fOpt->dwLengthMultiplier;
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "FindPermeability" );
    settings->setValue("pos", this->pos());
    settings->setValue("L", I);
    settings->setValue("N", N);
    settings->setValue("D1", D1);
    settings->setValue("D2", D2);
    settings->setValue("h", h);
    settings->endGroup();
    delete settings;
    delete dv;
    delete fOpt;
    delete ui;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Find_Permeability::getOpt(_OptionStruct gOpt){
    *fOpt = gOpt;
    ui->label_ind_m->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_01->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_02->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_03->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "FindPermeability" );
    double I = settings->value("L", 0).toDouble();
    double N = settings->value("N", 0).toDouble();
    double D1 = settings->value("D1", 0).toDouble();
    double D2 = settings->value("D2", 0).toDouble();
    double h = settings->value("h", 0).toDouble();
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    settings->endGroup();
    ui->lineEdit_ind->setText(loc.toString(I / fOpt->dwInductanceMultiplier));
    ui->lineEdit_N->setText(loc.toString(N));
    ui->lineEdit_1->setText(loc.toString(D1 / fOpt->dwLengthMultiplier));
    ui->lineEdit_2->setText(loc.toString(D2 / fOpt->dwLengthMultiplier));
    ui->lineEdit_3->setText(loc.toString(h / fOpt->dwLengthMultiplier));
    ui->lineEdit_N->setFocus();
    ui->lineEdit_N->selectAll();
    move(pos);
    delete settings;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Find_Permeability::getCurrentLocale(QLocale locale){
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Find_Permeability::on_pushButton_clicked()
{
    if ((ui->lineEdit_ind->text().isEmpty())||(ui->lineEdit_N->text().isEmpty())||(ui->lineEdit_1->text().isEmpty())||(ui->lineEdit_2->text().isEmpty())||(ui->lineEdit_3->text().isEmpty())){
        showWarning(tr("Warning"), tr("One or more inputs are empty!"));
        return;
    }
    bool ok1,ok2, ok3, ok4, ok5;
    double I = loc.toDouble(ui->lineEdit_ind->text(), &ok1)*fOpt->dwInductanceMultiplier;
    double N = loc.toDouble(ui->lineEdit_N->text(), &ok2);
    double D1 = loc.toDouble(ui->lineEdit_1->text(), &ok3)*fOpt->dwLengthMultiplier;
    double D2 = loc.toDouble(ui->lineEdit_2->text(), &ok4)*fOpt->dwLengthMultiplier;
    double h = loc.toDouble(ui->lineEdit_3->text(), &ok5)*fOpt->dwLengthMultiplier;
    if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    if ((I == 0)||(N == 0)||(D1 == 0)||(D2 == 0)||(h == 0)){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        return;
    }
    if (D1 < D2){
        showWarning(tr("Warning"), "D1 < D2");
        return;
    }
    _CoilResult result;
    findToroidPemeability(N, I, D1, D2, h, &result);

    QString sResult = "<hr><h2>" +QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle() + "</h2><br/>";
    if (fOpt->isInsertImage){
        sResult += "<img src=\":/images/res/Coil6.png\">";
    }
    sResult += "<p><u>" + tr("Input data") + ":</u><br/>";
    sResult += ui->label_ind->text() + ": L = " + ui->lineEdit_ind->text() + " " + ui->label_ind_m->text() + "<br/>";
    sResult += ui->label_N->text() + ": N = " + ui->lineEdit_N->text() + "<br/>";
    sResult += "<u>" + tr("Dimensions") + ":</u><br/>";
    sResult += ui->label_1->text() + " = " + ui->lineEdit_1->text() + " " + ui->label_01->text() + "<br/>";
    sResult += ui->label_2->text() + " = " + ui->lineEdit_2->text() + " " + ui->label_02->text() + "<br/>";
    sResult += ui->label_3->text() + " = " + ui->lineEdit_3->text() + " " + ui->label_03->text() + "</p>";
    sResult += "<hr>";
    sResult += "<p><u>" + tr("Result") + ":</u><br/>";
    sResult += tr("Relative magnetic permeability of the toroid") + " μ = " + loc.toString(result.N) + "<br/>";
    sResult += tr("Magnetic factor of the core") + " AL = " + loc.toString(result.sec, 'f', 0);
    sResult += "</p><hr>";
    emit sendResult(sResult);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Find_Permeability::on_pushButton_2_clicked()
{
    this->close();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Find_Permeability::on_pushButton_3_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/ferrite-toroid-core.html"));
}
