/* aircoretoroid.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "aircoretoroid.h"
#include "ui_aircoretoroid.h"

AirCoreToroid::AirCoreToroid(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AirCoreToroid)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, MAX_DOUBLE, 380);
    awgV = new QRegExpValidator(QRegExp(AWG_REG_EX));
    ui->lineEdit_1->setValidator(dv);
    ui->lineEdit_2->setValidator(dv);
    ui->lineEdit_3->setValidator(dv);
    ui->lineEdit_4->setValidator(dv);
    ui->lineEdit_N->setValidator(dv);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AirCoreToroid::~AirCoreToroid()
{
    bool isReverse = ui->checkBox_isReverce->isChecked();
    if (isReverse)
        N = loc.toDouble(ui->lineEdit_N->text());
    else
        ind = loc.toDouble(ui->lineEdit_N->text())*fOpt->dwInductanceMultiplier;
    D1 = loc.toDouble(ui->lineEdit_1->text())*fOpt->dwLengthMultiplier;
    D2 = loc.toDouble(ui->lineEdit_2->text())*fOpt->dwLengthMultiplier;
    h = loc.toDouble(ui->lineEdit_3->text())*fOpt->dwLengthMultiplier;
    if (fOpt->isAWG){
        dw = convertfromAWG(ui->lineEdit_4->text());
    } else {
        dw = loc.toDouble(ui->lineEdit_4->text())*fOpt->dwLengthMultiplier;
    }
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "AirCoreToroid" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", size());
    settings->setValue("ind", ind);
    settings->setValue("N", N);
    settings->setValue("D1", D1);
    settings->setValue("D2", D2);
    settings->setValue("h", h);
    settings->setValue("dw", dw);
    settings->setValue("isReverse", ui->checkBox_isReverce->isChecked());
    settings->setValue("windingKind", windingKind);
    settings->endGroup();
    delete settings;
    delete fOpt;
    delete dv;
    delete awgV;
    delete ui;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AirCoreToroid::getOpt(_OptionStruct gOpt)
{
    *fOpt = gOpt;
    ui->label_N_m->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_01->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_02->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_03->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_04->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "AirCoreToroid" );
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    bool isReverse = settings->value("isReverse", false).toBool();
    windingKind = settings->value("windingKind", 0).toInt();
    N = settings->value("N", 0).toDouble();
    ind = settings->value("ind", 0).toDouble();
    D1 = settings->value("D1", 0).toDouble();
    D2 = settings->value("D2", 0).toDouble();
    h = settings->value("h", 0).toDouble();
    dw = settings->value("dw", 0).toDouble();
    settings->endGroup();
    if (isReverse)
        ui->lineEdit_N->setText(loc.toString(N));
    else
        ui->lineEdit_N->setText(loc.toString(ind / fOpt->dwInductanceMultiplier));
    ui->lineEdit_1->setText(loc.toString(D1 / fOpt->dwLengthMultiplier));
    ui->lineEdit_2->setText(loc.toString(D2 / fOpt->dwLengthMultiplier));
    ui->lineEdit_3->setText(loc.toString(h / fOpt->dwLengthMultiplier));
    if (fOpt->isAWG){
        ui->label_04->setText(tr("AWG"));
        ui->lineEdit_4->setValidator(awgV);
        if (dw > 0){
            ui->lineEdit_4->setText(converttoAWG(dw));
        } else
            ui->lineEdit_4->setText("");
    } else
        ui->lineEdit_4->setText(loc.toString(dw / fOpt->dwLengthMultiplier));
    ui->label_1->setText(tr("Outside diameter")+" D1:");
    ui->label_2->setText(tr("Inside diameter")+" D2:");
    ui->label_3->setText(tr("Height") + " h:");
    ui->label_4->setText(tr("Wire diameter") + " dw:");
    resize(size);
    move(pos);
    ui->checkBox_isReverce->setChecked(isReverse);
    on_checkBox_isReverce_clicked();
    delete settings;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AirCoreToroid::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AirCoreToroid::on_pushButton_close_clicked()
{
    this->close();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AirCoreToroid::on_pushButton_help_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/toroid-air-core-coil.html"));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AirCoreToroid::on_radioButton_round_clicked()
{
    windingKind = 0;
    ui->image->setPixmap(QPixmap(":/images/res/toroid-round.png"));
    ui->label_03->setVisible(false);
    ui->label_3->setVisible(false);
    ui->lineEdit_3->setVisible(false);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AirCoreToroid::on_radioButton_rect_clicked()
{
    windingKind = 1;
    ui->image->setPixmap(QPixmap(":/images/res/toroid-square.png"));
    ui->label_03->setVisible(true);
    ui->label_3->setVisible(true);
    ui->lineEdit_3->setVisible(true);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AirCoreToroid::on_checkBox_isReverce_clicked()
{
    if (ui->checkBox_isReverce->isChecked()){
        ui->lineEdit_N->setText(loc.toString(N, 'f', fOpt->dwAccuracy));
        QString tmpTxt = tr("Number of turns") + " N:";
        ui->label_N->setText(tmpTxt);
        ui->label_N_m->setVisible(false);
    } else {
        ui->lineEdit_N->setText(loc.toString(ind / fOpt->dwInductanceMultiplier, 'f', fOpt->dwAccuracy));
        QString tmpTxt = tr("Inductance") + " L:";
        ui->label_N->setText(tmpTxt);
        ui->label_N_m->setVisible(true);
    }
    switch (windingKind) {
    case 0:{
        ui->radioButton_round->setChecked(true);
        on_radioButton_round_clicked();
        break;
    }
    case 1:{
        ui->radioButton_rect->setChecked(true);
        on_radioButton_rect_clicked();
        break;
    }
    default:
        break;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AirCoreToroid::on_pushButton_calculate_clicked()
{
    QString sResult = "<hr><h2>" +QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() +
            " - " + windowTitle();
    if ((ui->lineEdit_N->text().isEmpty())||(ui->lineEdit_1->text().isEmpty())||(ui->lineEdit_2->text().isEmpty())||(ui->lineEdit_4->text().isEmpty())){
        showWarning(tr("Warning"), tr("One or more inputs are empty!"));
        return;
    }
    bool ok1, ok2, ok3, ok4, ok5;
    if (fOpt->isAWG){
        dw = convertfromAWG(ui->lineEdit_4->text(), &ok2);
    } else {
        dw = loc.toDouble(ui->lineEdit_4->text(), &ok2)*fOpt->dwLengthMultiplier;
    }
    D1 = loc.toDouble(ui->lineEdit_1->text(), &ok3)*fOpt->dwLengthMultiplier;
    D2 = loc.toDouble(ui->lineEdit_2->text(), &ok4)*fOpt->dwLengthMultiplier;
    if((!ok2)||(!ok3)||(!ok4)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    if(windingKind == 0){
        sResult += " (" + ui->radioButton_round->text() +  + ")</h2><br/>";
        if (fOpt->isInsertImage)
            sResult += "<img src=\":/images/res/toroid-round.png\">";
    } else if (windingKind == 1){
        if(ui->lineEdit_3->text().isEmpty()){
            showWarning(tr("Warning"), tr("One or more inputs are empty!"));
            return;
        }
        h = loc.toDouble(ui->lineEdit_3->text(), &ok5)*fOpt->dwLengthMultiplier;
        if (!ok5){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        sResult += " (" + ui->radioButton_rect->text() +  + ")</h2><br/>";
        if (fOpt->isInsertImage)
            sResult += "<img src=\":/images/res/toroid-square.png\">";
    }
    _CoilResult result;
    if (ui->checkBox_isReverce->isChecked()){
        N = loc.toDouble(ui->lineEdit_N->text(), &ok1);
        if (!ok1){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if (windingKind == 0)
            ind = findAirCoreRoundToroid_I(N,D1,D2,dw);
        else
            ind = getFerriteI(N, D1, D2, h, 1);
    } else {
        ind = loc.toDouble(ui->lineEdit_N->text(), &ok1)*fOpt->dwInductanceMultiplier;
        if (!ok1){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if (windingKind == 0)
            N = findAirCoreRoundToroid_N(ind, D1, D2, dw);
        else {
            getFerriteN(ind, D1, D2, h, dw, 1, &result);
            N = result.N;
        }
    }
    sResult += "<p><u>" + tr("Input data") + ":</u><br/>";
    if (ui->checkBox_isReverce->isChecked())
        sResult += ui->label_N->text() + " " + ui->lineEdit_N->text() + "<br/>";
    else
        sResult += ui->label_N->text() + " " + ui->lineEdit_N->text() + " " + ui->label_N_m->text() + "<br/>";
    sResult += ui->label_1->text() + " " + ui->lineEdit_1->text() + " " + ui->label_01->text() + "<br/>";
    sResult += ui->label_2->text() + " " + ui->lineEdit_2->text() + " " + ui->label_02->text() + "<br/>";
    if (windingKind == 1){
        sResult += ui->label_3->text() + " " + ui->lineEdit_3->text() + " " + ui->label_03->text() + "<br/>";
    }
    sResult += ui->label_4->text() + " " + ui->lineEdit_4->text() + " " + ui->label_04->text() + "</p>";
    sResult += "<hr>";
    sResult += "<p><u>" + tr("Result") + ":</u><br/>";
    if (ui->checkBox_isReverce->isChecked()){
        sResult += tr("Inductance") + " L = " + loc.toString(ind / fOpt->dwInductanceMultiplier, 'f', fOpt->dwAccuracy) + " "
                + qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8());
    } else {
        sResult += tr("Number of turns of the coil") + " N = " + loc.toString(N, 'f', fOpt->dwAccuracy);
    }
    sResult += "</p><hr>";
    emit sendResult(sResult);
}
