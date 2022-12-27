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
    dv = new QDoubleValidator(0.0, MAX_DOUBLE, 380);
    ui->lineEdit_ind->setValidator(dv);
    ui->lineEdit_N->setValidator(dv);
    ui->lineEdit_1->setValidator(dv);
    ui->lineEdit_2->setValidator(dv);
    ui->lineEdit_3->setValidator(dv);
    ui->lineEdit_4->setValidator(dv);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Find_Permeability::~Find_Permeability()
{
    double I = loc.toDouble(ui->lineEdit_ind->text())*fOpt->dwInductanceMultiplier;
    double N = loc.toDouble(ui->lineEdit_N->text());
    double OD = loc.toDouble(ui->lineEdit_1->text())*fOpt->dwLengthMultiplier;
    double ID = loc.toDouble(ui->lineEdit_2->text())*fOpt->dwLengthMultiplier;
    double h = loc.toDouble(ui->lineEdit_3->text())*fOpt->dwLengthMultiplier;
    double Ch = loc.toDouble(ui->lineEdit_4->text())*fOpt->dwLengthMultiplier;
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "FindPermeability" );
    settings->setValue("pos", this->pos());
    settings->setValue("L", I);
    settings->setValue("N", N);
    settings->setValue("OD", OD);
    settings->setValue("ID", ID);
    settings->setValue("h", h);
    settings->setValue("Ch", Ch);
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
    double OD = settings->value("OD", 0).toDouble();
    double ID = settings->value("ID", 0).toDouble();
    double h = settings->value("h", 0).toDouble();
    double Ch = settings->value("Ch", 0).toDouble();
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    settings->endGroup();
    ui->lineEdit_ind->setText(roundTo(I / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_N->setText(roundTo(N, loc, fOpt->dwAccuracy));
    ui->lineEdit_1->setText(roundTo(OD / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_2->setText(roundTo(ID / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_3->setText(roundTo(h / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_4->setText(roundTo(Ch / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
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
    bool ok1,ok2, ok3, ok4, ok5, ok6;
    double I = loc.toDouble(ui->lineEdit_ind->text(), &ok1)*fOpt->dwInductanceMultiplier;
    double N = loc.toDouble(ui->lineEdit_N->text(), &ok2);
    double OD = loc.toDouble(ui->lineEdit_1->text(), &ok3)*fOpt->dwLengthMultiplier;
    double ID = loc.toDouble(ui->lineEdit_2->text(), &ok4)*fOpt->dwLengthMultiplier;
    double h = loc.toDouble(ui->lineEdit_3->text(), &ok5)*fOpt->dwLengthMultiplier;
    double Ch = loc.toDouble(ui->lineEdit_4->text(), &ok6)*fOpt->dwLengthMultiplier;
    if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    if ((I == 0)||(N == 0)||(OD == 0)||(ID == 0)||(h == 0)){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        return;
    }
    if ((OD < ID) || (OD <= Ch) || (ID <= Ch) || (h <= Ch)){
        showWarning(tr("Warning"), "(OD < ID) | (OD <= C) | (ID <= C) | (h <= C)");
        return;
    }
    _CoilResult result;
    findToroidPemeability(N, I, OD, ID, h, Ch, &result);
    QString sResult = "<hr>";
    if (fOpt->isShowTitle){
        sResult = "<h2>" +QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - "
                + windowTitle() + "</h2><br/>";
    }
    if (fOpt->isInsertImage){
        sResult += "<img src=\":/images/res/T-core.png\">";
    }
    sResult += "<p><u>" + tr("Input data") + ":</u><br/>";
    sResult += ui->label_ind->text() + ": L = " + ui->lineEdit_ind->text() + " " + ui->label_ind_m->text() + "<br/>";
    sResult += ui->label_N->text() + ": N = " + ui->lineEdit_N->text() + "<br/>";
    sResult += "<u>" + tr("Dimensions") + ":</u><br/>";
    sResult += ui->label_1->text() + " = " + ui->lineEdit_1->text() + " " + ui->label_01->text() + "<br/>";
    sResult += ui->label_2->text() + " = " + ui->lineEdit_2->text() + " " + ui->label_02->text() + "<br/>";
    sResult += ui->label_3->text() + " = " + ui->lineEdit_3->text() + " " + ui->label_03->text() + "<br/>";
    if (Ch > 0)
        sResult += ui->label_4->text() + " = " + ui->lineEdit_4->text() + " " + ui->label_04->text();
    sResult += "</p><hr>";
    sResult += "<p><u>" + tr("Result") + ":</u><br/>";
    sResult += tr("Relative magnetic permeability of the toroid") + " μ<sub>r</sub> = " + loc.toString(result.N) + "<br/>";
    sResult += tr("Magnetic factor of the core") + " A<sub>L</sub> = " + loc.toString(result.sec, 'f', 0)+ "&nbsp;"
            +  qApp->translate("Context","nH") + "/N<sup>2</sup>";
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
