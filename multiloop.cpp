/* multiloop.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "multiloop.h"
#include "ui_multiloop.h"

Multiloop::Multiloop(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Multiloop)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator;
    ui->lineEdit_1->setValidator(dv);
    ui->lineEdit_2->setValidator(dv);
    ui->lineEdit_3->setValidator(dv);
    ui->lineEdit_N->setValidator(dv);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Multiloop::~Multiloop()
{
    double Di = loc.toDouble(ui->lineEdit_1->text())*fOpt->dwLengthMultiplier;
    double dt = loc.toDouble(ui->lineEdit_3->text())*fOpt->dwLengthMultiplier;
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Multi_loop" );
    settings->setValue("pos", this->pos());
    settings->setValue("N", nTurns);
    settings->setValue("ind", ind * fOpt->dwInductanceMultiplier);
    settings->setValue("Di", Di);
    settings->setValue("d", dw);
    settings->setValue("dt", dt);
    settings->setValue("isReverse", ui->checkBox_isReverce->isChecked());
    settings->endGroup();
    delete settings;
    delete fOpt;
    delete dv;
    delete ui;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::getOpt(_OptionStruct gOpt){
    *fOpt = gOpt;
    QString tmp_txt = tr("Number of turns") + " N:";
    ui->label_N->setText(tmp_txt);
    tmp_txt = tr("Inside diameter") + " Di:";
    ui->label_1->setText(tmp_txt);
    tmp_txt = tr("Wire diameter") + " d:";
    ui->label_2->setText(tmp_txt);
    tmp_txt = tr("Wire diameter with insulation") + " do:";
    ui->label_3->setText(tmp_txt);
    ui->label_N_m->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_01->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_02->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_03->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    QFont f1 = this->font();
    f1.setFamily(fOpt->mainFontFamily);
    f1.setPixelSize(fOpt->mainFontSize);
    this->setFont(f1);
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Multi_loop" );
    nTurns = settings->value("N", 0).toULongLong();
    ind = settings->value("ind", 0).toDouble();
    double Di = settings->value("Di", 0).toDouble();
    dw = settings->value("d", 0).toDouble();
    double dt = settings->value("dt", 0).toDouble();
    QPoint pos = settings->value("pos", QPoint(300, 300)).toPoint();
    bool isReverse = settings->value("isReverse", false).toBool();
    settings->endGroup();
    ui->checkBox_isReverce->setChecked(isReverse);
    on_checkBox_isReverce_clicked();
    if (!ui->checkBox_isReverce->isChecked())
        ui->lineEdit_N->setText(loc.toString(nTurns));
    else
        ui->lineEdit_N->setText(loc.toString(ind / fOpt->dwInductanceMultiplier, 'f', fOpt->dwAccuracy));
    ui->lineEdit_1->setText(loc.toString(Di / fOpt->dwLengthMultiplier));
    if (fOpt->isAWG){
        ui->label_02->setText(tr("AWG"));
        if (dw > 0){
            ui->lineEdit_2->setText(converttoAWG(dw));
        } else
            ui->lineEdit_2->setText("");
    } else
        ui->lineEdit_2->setText(loc.toString(dw / fOpt->dwLengthMultiplier));
    if (ui->lineEdit_2->text().isEmpty() || (ui->lineEdit_2->text() == "0")|| (dt > 0))
        ui->lineEdit_3->setText(loc.toString(dt / fOpt->dwLengthMultiplier));
    else
        on_lineEdit_2_editingFinished();
    ui->lineEdit_N->setFocus();
    ui->lineEdit_N->selectAll();
    move(pos);
    delete settings;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::getCurrentLocale(QLocale locale){
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_pushButton_clicked()
{
    if ((ui->lineEdit_N->text().isEmpty())||(ui->lineEdit_1->text().isEmpty())||(ui->lineEdit_2->text().isEmpty())||(ui->lineEdit_3->text().isEmpty())){
        showWarning(tr("Warning"), tr("One or more inputs are empty!"));
        return;
    }
    bool ok1, ok2, ok3, ok4;
    if (!ui->checkBox_isReverce->isChecked())
        nTurns = loc.toInt(ui->lineEdit_N->text(), &ok1);
    else
        ind = loc.toDouble(ui->lineEdit_N->text(), &ok1);
    double Di = loc.toDouble(ui->lineEdit_1->text(), &ok2)*fOpt->dwLengthMultiplier;
    if (fOpt->isAWG){
        dw = convertfromAWG(ui->lineEdit_2->text(), &ok3);
    } else {
        dw = loc.toDouble(ui->lineEdit_2->text(), &ok3)*fOpt->dwLengthMultiplier;
    }
    double dt = loc.toDouble(ui->lineEdit_3->text(), &ok4)*fOpt->dwLengthMultiplier;
    if((!ok1)||(!ok2)||(!ok3)||(!ok4)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    if ((Di == 0)||(dw == 0)||(dt == 0)){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        return;
    }
    if (dt < dw){
        showWarning(tr("Warning"), "do < d");
        return;
    }

    _CoilResult result;
    if (!ui->checkBox_isReverce->isChecked()) ind = findMultiloop_I(nTurns, Di, dw, dt, &result);
    else nTurns = findMultiloop_N(ind, Di, dw, dt, &result);
    if (nTurns == -1){
        nTurns = 0;
        showWarning(tr("Warning"),tr("Coil can not be realized") + "!");
        return;
    }
    QString sResult = "<hr><h2>" +QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle() + "</h2><br/>";
    if (fOpt->isInsertImage){
        sResult += "<img src=\":/images/res/multi_loop.png\">";
    }
    sResult += "<p><u>" + tr("Input data") + ":</u><br/>";
    QString measureUnit = "";
    if (!ui->checkBox_isReverce->isChecked())
        measureUnit = "";
    else
        measureUnit = ui->label_N_m->text();
    sResult += ui->label_N->text() + " " + ui->lineEdit_N->text() + " " + measureUnit + "<br/>";
    sResult += "<u>" + tr("Dimensions") + ":</u><br/>";
    sResult += ui->label_1->text() + " " + ui->lineEdit_1->text() + " " + ui->label_01->text() + "<br/>";
    sResult += ui->label_2->text() + " " + ui->lineEdit_2->text() + " " + ui->label_02->text() + "<br/>";
    sResult += ui->label_3->text() + " " + ui->lineEdit_3->text() + " " + ui->label_03->text() + "</p>";
    sResult += "<hr>";
    sResult += "<p><u>" + tr("Result") + ":</u><br/>";
    if (!ui->checkBox_isReverce->isChecked()){
        sResult += tr("Inductance") + " L = " + loc.toString(ind / fOpt->dwInductanceMultiplier, 'f', fOpt->dwAccuracy) + " "
                + qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()) + "<br/>";
    } else {
        sResult += tr("Number of turns of the coil") + " N = " + loc.toString(nTurns) + "<br/>";
    }
    sResult += tr("Mean diameter") + " Dm = " + loc.toString( (result.N)/fOpt->dwLengthMultiplier, 'f', fOpt->dwAccuracy ) + " " +
            qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    sResult += tr("Thickness of the coil") + " T = " + loc.toString( (result.sec)/fOpt->dwLengthMultiplier, 'f', fOpt->dwAccuracy ) + " " +
            qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    sResult += tr("Resistance of the coil") + " R = " + loc.toString(result.fourth, 'f', fOpt->dwAccuracy) + " " + tr("Ohm") + "<br/>";
    QString _wire_length = formatLength(result.thd, fOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    sResult += tr("Length of wire without leads") + " lw = " + loc.toString(d_wire_length.toDouble(), 'f', fOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double mass = 2.225 * M_PI * dw * dw * result.thd;
    sResult += tr("Weight of wire") + " m = " + loc.toString(mass) + " " + tr("g") + "<br/>";
    sResult += "</p><hr>";
    emit sendResult(sResult);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_pushButton_2_clicked()
{
    this->close();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_pushButton_3_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/metal-detector-search-coil.html"));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_lineEdit_2_editingFinished()
{
    bool ok;
    if (fOpt->isAWG){
        dw = convertfromAWG(ui->lineEdit_2->text(), &ok);
    } else {
        dw = loc.toDouble(ui->lineEdit_2->text(), &ok)*fOpt->dwLengthMultiplier;
    }
    if (!ok){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    double k_m = odCalc(dw);
    if (dw > 0){
        ui->lineEdit_3->setText( loc.toString(k_m / fOpt->dwLengthMultiplier));
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_checkBox_isReverce_clicked()
{
    QString tmp_txt;
    if (!ui->checkBox_isReverce->isChecked()){
        tmp_txt = tr("Number of turns") + " N:";
        ui->label_N->setText(tmp_txt);
        ui->lineEdit_N->setText(loc.toString(nTurns));
        ui->label_N_m->setVisible(false);
    } else {
        tmp_txt = tr("Inductance") + " L:";
        ui->label_N->setText(tmp_txt);
        ui->lineEdit_N->setText(loc.toString(ind / fOpt->dwInductanceMultiplier, 'f', fOpt->dwAccuracy));
        ui->label_N_m->setVisible(true);
    }
}
