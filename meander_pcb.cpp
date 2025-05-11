/* meander_pcb.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "meander_pcb.h"
#include "ui_meander_pcb.h"

Meander_pcb::Meander_pcb(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Meander_pcb)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, DBL_MAX, 380);
    ui->lineEdit_1->setValidator(dv);
    ui->lineEdit_2->setValidator(dv);
    ui->lineEdit_3->setValidator(dv);
    ui->lineEdit_4->setValidator(dv);
    ui->lineEdit_N->setValidator(dv);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Meander_pcb::~Meander_pcb()
{
    double N = loc.toDouble(ui->lineEdit_N->text());
    double a = loc.toDouble(ui->lineEdit_1->text())*fOpt->dwLengthMultiplier;
    double d = loc.toDouble(ui->lineEdit_2->text())*fOpt->dwLengthMultiplier;
    double h = loc.toDouble(ui->lineEdit_3->text())*fOpt->dwLengthMultiplier;
    double W = loc.toDouble(ui->lineEdit_4->text())*fOpt->dwLengthMultiplier;
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Meander_PCB" );
    settings->setValue("pos", this->pos());
    settings->setValue("N", N);
    settings->setValue("a", a);
    settings->setValue("d", d);
    settings->setValue("h", h);
    settings->setValue("W", W);
    settings->endGroup();
    delete settings;
    delete fOpt;
    delete dv;
    delete ui;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Meander_pcb::getOpt(_OptionStruct gOpt){
    *fOpt = gOpt;
    ui->label_N->setText(tr("Number of turns") + " N:");
    ui->label_01->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_02->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_03->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_04->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Meander_PCB" );
    double N = settings->value("N", 0).toDouble();
    double a = settings->value("a", 0).toDouble();
    double d = settings->value("d", 0).toDouble();
    double h = settings->value("h", 0).toDouble();
    double W = settings->value("W", 0).toDouble();
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    settings->endGroup();
    ui->lineEdit_N->setText(roundTo(N, loc, fOpt->dwAccuracy));
    ui->lineEdit_1->setText(roundTo(a / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_2->setText(roundTo(d / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_3->setText(roundTo(h / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_4->setText(roundTo(W / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_N->setFocus();
    ui->lineEdit_N->selectAll();
    move(pos);
    delete settings;
    if (fOpt->styleGUI == _DarkStyle){
        ui->pushButton->setIcon(reverseIconColors(ui->pushButton->icon()));
        ui->pushButton_2->setIcon(reverseIconColors(ui->pushButton_2->icon()));
        ui->pushButton_3->setIcon(reverseIconColors(ui->pushButton_3->icon()));
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Meander_pcb::getCurrentLocale(QLocale locale){
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Meander_pcb::on_pushButton_clicked()
{
    if ((ui->lineEdit_N->text().isEmpty())||(ui->lineEdit_1->text().isEmpty())||(ui->lineEdit_2->text().isEmpty())||(ui->lineEdit_3->text().isEmpty())
            ||(ui->lineEdit_4->text().isEmpty())){
        showWarning(tr("Warning"), tr("One or more inputs are empty!"));
        return;
    }
    bool ok1,ok2, ok3, ok4, ok5;
    double N = loc.toDouble(ui->lineEdit_N->text(), &ok1);
    double a = loc.toDouble(ui->lineEdit_1->text(), &ok2)*fOpt->dwLengthMultiplier;
    double d = loc.toDouble(ui->lineEdit_2->text(), &ok3)*fOpt->dwLengthMultiplier;
    double h = loc.toDouble(ui->lineEdit_3->text(), &ok4)*fOpt->dwLengthMultiplier;
    double W = loc.toDouble(ui->lineEdit_4->text(), &ok5)*fOpt->dwLengthMultiplier;
    if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    if ((N == 0)||(a == 0)||(d == 0)||(h == 0) || (W == 0)){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        return;
    }
    if (d <= W){
        showWarning(tr("Warning"), "d <= W");
        return;
    }
    _CoilResult result;
    findMeadrPCB_I(a, d, h, W, N, &result);
    QString sCaption = QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle();
    QString sImage = "<img src=\":/images/res/meandr_pcb.png\">";
    QString sInput = "<p><u>" + tr("Input data") + ":</u><br/>";
    sInput += formattedOutput(fOpt, ui->label_N->text(), ui->lineEdit_N->text()) + "<br/>";
    sInput += "<u>" + tr("Dimensions") + ":</u><br/>";
    sInput += formattedOutput(fOpt, ui->label_1->text(), ui->lineEdit_1->text(), ui->label_01->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_2->text(), ui->lineEdit_2->text(), ui->label_02->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_3->text(), ui->lineEdit_3->text(), ui->label_03->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_4->text(), ui->lineEdit_4->text(), ui->label_04->text()) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    sResult += formattedOutput(fOpt, tr("Inductance") + " L = ", roundTo(result.N / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                               qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
    sResult += formattedOutput(fOpt, tr("Length of winding") + " l = ", roundTo( (result.sec)/fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy),
                               qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    sResult += "</p>";
    emit sendResult(sCaption + LIST_SEPARATOR + sImage + LIST_SEPARATOR + sInput + LIST_SEPARATOR + sResult);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Meander_pcb::on_pushButton_2_clicked()
{
    this->close();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Meander_pcb::on_pushButton_3_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/meandr-pcb-coil.html"));
}

