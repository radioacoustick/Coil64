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
    OD = loc.toDouble(ui->lineEdit_1->text())*fOpt->dwLengthMultiplier;
    ID = loc.toDouble(ui->lineEdit_2->text())*fOpt->dwLengthMultiplier;
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
    settings->setValue("OD", OD);
    settings->setValue("ID", ID);
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
    OD = settings->value("OD", 0).toDouble();
    ID = settings->value("ID", 0).toDouble();
    h = settings->value("h", 0).toDouble();
    dw = settings->value("dw", 0).toDouble();
    settings->endGroup();
    if (isReverse)
        ui->lineEdit_N->setText(roundTo(N, loc, fOpt->dwAccuracy));
    else
        ui->lineEdit_N->setText(roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_1->setText(roundTo(OD / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_2->setText(roundTo(ID / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_3->setText(roundTo(h / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    if (fOpt->isAWG){
        ui->label_04->setText(tr("AWG"));
        ui->lineEdit_4->setValidator(awgV);
        if (dw > 0){
            ui->lineEdit_4->setText(converttoAWG(dw));
        } else
            ui->lineEdit_4->setText("");
    } else
        ui->lineEdit_4->setText(roundTo(dw / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->label_1->setText(tr("Outside diameter")+" OD:");
    ui->label_2->setText(tr("Inside diameter")+" ID:");
    ui->label_3->setText(tr("Height") + " h:");
    ui->label_4->setText(tr("Wire diameter") + " dw:");
    resize(size);
    move(pos);
    ui->checkBox_isReverce->setChecked(isReverse);
    on_checkBox_isReverce_clicked();
    delete settings;
    if (fOpt->styleGUI == _DarkStyle){
        ui->pushButton_calculate->setIcon(reverseIconColors(ui->pushButton_calculate->icon()));
        ui->pushButton_close->setIcon(reverseIconColors(ui->pushButton_close->icon()));
        ui->pushButton_help->setIcon(reverseIconColors(ui->pushButton_help->icon()));
    }
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
        ui->lineEdit_N->setText(roundTo(N, loc, fOpt->dwAccuracy));
        QString tmpTxt = tr("Number of turns") + " N:";
        ui->label_N->setText(tmpTxt);
        ui->label_N_m->setVisible(false);
    } else {
        ui->lineEdit_N->setText(roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
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
    ui->lineEdit_N->setFocus();
    ui->lineEdit_N->selectAll();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AirCoreToroid::on_pushButton_calculate_clicked()
{
    QString sCaption = QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle();
    QString sImage = "";
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
    OD = loc.toDouble(ui->lineEdit_1->text(), &ok3)*fOpt->dwLengthMultiplier;
    ID = loc.toDouble(ui->lineEdit_2->text(), &ok4)*fOpt->dwLengthMultiplier;
    if((!ok2)||(!ok3)||(!ok4)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    if ((OD == 0)||(ID == 0)||(dw == 0)){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        return;
    }
    if (OD < ID){
        showWarning(tr("Warning"), "OD < ID");
        return;
    }
    if(windingKind == 0){
        sCaption += " (" + ui->radioButton_round->text() +  + ")";
        sImage = "<img src=\":/images/res/toroid-round.png\">";
    } else if (windingKind == 1){
        if(ui->lineEdit_3->text().isEmpty()){
            showWarning(tr("Warning"), tr("One or more inputs are empty!"));
            return;
        }
        h = loc.toDouble(ui->lineEdit_3->text(), &ok5)*fOpt->dwLengthMultiplier;
        if (h == 0){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        if (!ok5){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        sCaption += " (" + ui->radioButton_rect->text() +  + ")";
        sImage = "<img src=\":/images/res/toroid-square.png\">";
    }
    _CoilResult result;
    if (ui->checkBox_isReverce->isChecked()){
        N = loc.toDouble(ui->lineEdit_N->text(), &ok1);
        if (N == 0){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        if (!ok1){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if (windingKind == 0){
            ind = findAirCoreRoundToroid_I(N,OD,ID,dw);
            lw = getToroidWireLength(OD, ID, 0, dw , N, NULL, true);
        } else {
            ind = getFerriteI(N, OD, ID, h, 1, dw / 2, 0, &result);
            lw = getToroidWireLength(OD, ID, h, dw, N);
        }
    } else {
        ind = loc.toDouble(ui->lineEdit_N->text(), &ok1)*fOpt->dwInductanceMultiplier;
        if (ind == 0){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        if (!ok1){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if (windingKind == 0){
            N = findAirCoreRoundToroid_N(ind, OD, ID, dw);
            lw = getToroidWireLength(OD, ID, 0, dw , N, NULL, true);
        } else {
            getFerriteN(ind, OD, ID, h, dw, 1, dw / 2, &result);
            N = result.N;
            lw = getToroidWireLength(OD, ID, h, dw, N);
        }
    }
    QString sInput = "<p><u>" + tr("Input data") + ":</u><br/>";
    if (ui->checkBox_isReverce->isChecked())
        sInput += formattedOutput(fOpt, ui->label_N->text(), ui->lineEdit_N->text()) + "<br/>";
    else
        sInput += formattedOutput(fOpt, ui->label_N->text(), ui->lineEdit_N->text(), ui->label_N_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_1->text(), ui->lineEdit_1->text(), ui->label_01->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_2->text(), ui->lineEdit_2->text(), ui->label_02->text()) + "<br/>";
    if (windingKind == 1){
        sInput += formattedOutput(fOpt, ui->label_3->text(), ui->lineEdit_3->text(), ui->label_03->text()) + "<br/>";
    }
    sInput += formattedOutput(fOpt, ui->label_4->text(), ui->lineEdit_4->text(), ui->label_04->text()) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    if (ui->checkBox_isReverce->isChecked()){
        sResult += formattedOutput(fOpt, tr("Inductance") + " L = ", roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                   qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    } else {
        sResult += formattedOutput(fOpt, tr("Number of turns of the coil") + " N = ", roundTo(N, loc, fOpt->dwAccuracy));
    }
    if (lw > 0){
        QString _wire_length = formatLength(lw, fOpt->dwLengthMultiplier);
        QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
        QString d_wire_length = list[0];
        QString _ssLengthMeasureUnit = list[1];
        if (d_wire_length != "-100"){
            sResult += "<br/>" + formattedOutput(fOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, fOpt->dwAccuracy),
                                      qApp->translate("Context", _ssLengthMeasureUnit.toUtf8()));
        }
    }
    sResult += "</p>";
    emit sendResult(sCaption + LIST_SEPARATOR + sImage + LIST_SEPARATOR + sInput + LIST_SEPARATOR + sResult);
}
