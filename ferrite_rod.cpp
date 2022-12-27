/* ferrite_rod.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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


#include "ferrite_rod.h"
#include "ui_ferrite_rod.h"

Ferrite_Rod::Ferrite_Rod(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Ferrite_Rod)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    ui->label_Dr->setText(tr("Rod diameter") + " Dr:");
    ui->label_Lr->setText(tr("Rod length") + " Lr:");
    ui->label_mu->setText(tr("Magnetic permeability") + " µ:");
    ui->label_dc->setText(tr("Former diameter") + "  dc:");
    ui->label_s->setText(tr("Shift") + "  s:");
    ui->label_dw->setText(tr("Wire diameter") + " dw:");
    ui->label_p->setText(tr("Winding pitch") + " p:");

    dv = new QDoubleValidator(0.0, MAX_DOUBLE, 380);
    awgV = new QRegExpValidator(QRegExp(AWG_REG_EX));
    ui->lineEdit_ind->setValidator(dv);
    ui->lineEdit_Dr->setValidator(dv);
    ui->lineEdit_Lr->setValidator(dv);
    ui->lineEdit_mu->setValidator(dv);
    ui->lineEdit_dc->setValidator(dv);
    ui->lineEdit_s->setValidator(dv);
    ui->lineEdit_dw->setValidator(dv);
    ui->lineEdit_p->setValidator(dv);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ferrite_Rod::~Ferrite_Rod()
{
    double I = loc.toDouble(ui->lineEdit_ind->text())*fOpt->dwInductanceMultiplier;
    double Dr = loc.toDouble(ui->lineEdit_Dr->text())*fOpt->dwLengthMultiplier;
    double Lr = loc.toDouble(ui->lineEdit_Lr->text())*fOpt->dwLengthMultiplier;
    double mu = loc.toDouble(ui->lineEdit_mu->text());
    double dc = loc.toDouble(ui->lineEdit_dc->text())*fOpt->dwLengthMultiplier;
    double s = loc.toDouble(ui->lineEdit_s->text())*fOpt->dwLengthMultiplier;
    double dw = 0.0;
    if (fOpt->isAWG){
        dw = convertfromAWG(ui->lineEdit_dw->text());
    } else {
        dw = loc.toDouble(ui->lineEdit_dw->text())*fOpt->dwLengthMultiplier;
    }
    double p = loc.toDouble(ui->lineEdit_p->text())*fOpt->dwLengthMultiplier;
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "FerriteRod" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", size());
    settings->setValue("L", I);
    settings->setValue("Dr", Dr);
    settings->setValue("Lr", Lr);
    settings->setValue("mu", mu);
    settings->setValue("dc", dc);
    settings->setValue("s", s);
    settings->setValue("dw", dw);
    settings->setValue("p", p);
    settings->endGroup();
    delete settings;
    delete fOpt;
    delete awgV;
    delete dv;
    delete ui;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Ferrite_Rod::getOpt(_OptionStruct gOpt){
    *fOpt = gOpt;
    ui->label_ind_m->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_Dr_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_Lr_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_dc_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_s_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_dw_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_p_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "FerriteRod" );
    double I = settings->value("L", 0).toDouble();
    double Dr = settings->value("Dr", 0).toDouble();
    double mu = settings->value("mu", 0).toDouble();
    double Lr = settings->value("Lr", 0).toDouble();
    double dc = settings->value("dc", 0).toDouble();
    double s = settings->value("s", 0).toDouble();
    double dw = settings->value("dw", 0).toDouble();
    double p = settings->value("p", 0).toDouble();
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    settings->endGroup();
    ui->lineEdit_ind->setText(roundTo(I / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_Dr->setText(roundTo(Dr / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_Lr->setText(roundTo(Lr / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_mu->setText(roundTo(mu, loc, fOpt->dwAccuracy));
    ui->lineEdit_dc->setText(roundTo(dc / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_s->setText(roundTo(s / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    if (fOpt->isAWG){
        ui->label_dw_m->setText(tr("AWG"));
        ui->lineEdit_dw->setValidator(awgV);
        if (dw > 0){
            ui->lineEdit_dw->setText(converttoAWG(dw));
        } else
            ui->lineEdit_dw->setText("");
    } else
        ui->lineEdit_dw->setText(roundTo(dw / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_p->setText(roundTo(p / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_ind->setFocus();
    ui->lineEdit_ind->selectAll();
    resize(size);
    move(pos);
    delete settings;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Ferrite_Rod::getCurrentLocale(QLocale locale){
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Ferrite_Rod::on_pushButton_close_clicked()
{
    this->close();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Ferrite_Rod::on_pushButton_calculate_clicked()
{
    if ((ui->lineEdit_ind->text().isEmpty())||(ui->lineEdit_Dr->text().isEmpty())||(ui->lineEdit_Lr->text().isEmpty())||(ui->lineEdit_mu->text().isEmpty())
            ||(ui->lineEdit_dc->text().isEmpty())||(ui->lineEdit_s->text().isEmpty())||(ui->lineEdit_dw->text().isEmpty())||(ui->lineEdit_p->text().isEmpty())){
        showWarning(tr("Warning"), tr("One or more inputs are empty!"));
        return;
    }
    bool ok1,ok2, ok3, ok4, ok5, ok6, ok7, ok8;
    double I = loc.toDouble(ui->lineEdit_ind->text(), &ok1)*fOpt->dwInductanceMultiplier;
    double Dr = loc.toDouble(ui->lineEdit_Dr->text(), &ok2)*fOpt->dwLengthMultiplier;
    double Lr = loc.toDouble(ui->lineEdit_Lr->text(), &ok3)*fOpt->dwLengthMultiplier;
    double mu = loc.toDouble(ui->lineEdit_mu->text(), &ok4);
    double dc = loc.toDouble(ui->lineEdit_dc->text(), &ok5)*fOpt->dwLengthMultiplier;
    double s = loc.toDouble(ui->lineEdit_s->text(), &ok6)*fOpt->dwLengthMultiplier;
    double dw = 0.0;
    if (fOpt->isAWG){
        dw = convertfromAWG(ui->lineEdit_dw->text(), &ok7);
    } else {
        dw = loc.toDouble(ui->lineEdit_dw->text(), &ok7)*fOpt->dwLengthMultiplier;
    }
    double p = loc.toDouble(ui->lineEdit_p->text(), &ok8)*fOpt->dwLengthMultiplier;
    if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok7)||(!ok8)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    if ((I == 0)||(Dr == 0)||(Lr == 0)||(mu == 0)||(dc == 0)||(dw == 0)||(p == 0)){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        return;
    }
    if (dw > p){
        showWarning(tr("Warning"), "dw > p");
        return;
    }
    if (Dr > dc){
        showWarning(tr("Warning"), "Dr > dc");
        return;
    }
    if (mu < 100){
        showWarning(tr("Warning"), "mu < 100");
        return;
    }
    if (p > 2*dw){
        showWarning(tr("Warning"), "p > 2*dw");
        return;
    }
    if (dc > 2*Dr){
        showWarning(tr("Warning"), "dc > 2*Dr");
        return;
    }
    _CoilResult result;
    findFerriteRodN(I, Lr, Dr, mu, dc, s, dw, p, &result);
    if (result.thd > (3*Lr/4)){
        showWarning(tr("Warning"), "lc > ¾Lr");
        return;
    }
    if ((0.5*(Lr-result.thd)-s) < (0.125*Lr)){
        showWarning(tr("Warning"), tr("Distance from coil edge to core edge less than 1/8 Lr"));
        return;
    }
    QString sResult = "<hr>";
    if (fOpt->isShowTitle){
        sResult = "<h2>" +QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - "
                + windowTitle() + "</h2><br/>";
    }
    if (fOpt->isInsertImage){
        sResult += "<img src=\":/images/res/Ferrite-rod.png\">";
    }
    sResult += "<p><u>" + tr("Input data") + ":</u><br/>";
    sResult += ui->label_ind->text() + " = " + ui->lineEdit_ind->text() + " " + ui->label_ind_m->text() + "<br/>";
    sResult += "<p><u>" + ui->groupBox_core->title() + ":</u><br/>";
    sResult += ui->label_Dr->text() + " = " + ui->lineEdit_Dr->text() + " " + ui->label_Dr_m->text() + "<br/>";
    sResult += ui->label_Lr->text() + " = " + ui->lineEdit_Lr->text() + " " + ui->label_Lr_m->text() + "<br/>";
    sResult += ui->label_mu->text() + ui->lineEdit_mu->text() + "<br/>";
    sResult += "<p><u>" + ui->groupBox_coil->title() + ":</u><br/>";
    sResult += ui->label_dc->text() + " = " + ui->lineEdit_dc->text() + " " + ui->label_dc_m->text() + "<br/>";
    sResult += ui->label_s->text() + " = " + ui->lineEdit_s->text() + " " + ui->label_s_m->text() + "<br/>";
    sResult += ui->label_dw->text() + " = " + ui->lineEdit_dw->text() + " " + ui->label_dw_m->text() + "<br/>";
    sResult += ui->label_p->text() + " = " + ui->lineEdit_p->text() + " " + ui->label_p_m->text() + "</p>";
    sResult += "<hr>";
    sResult += "<p><u>" + tr("Result") + ":</u><br/>";
    sResult += tr("Number of turns of the coil") + " N = " + QString::number(result.N) + "<br/>";
    sResult += tr("Length of winding") + " lc = " + roundTo(result.thd, loc, fOpt->dwAccuracy) + "<br/>";
    sResult += tr("Effective magnetic permeability of the core") + " μ<sub>e</sub> = " + roundTo(result.sec, loc, 0);
    sResult += "</p><hr>";
    emit sendResult(sResult);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Ferrite_Rod::on_pushButton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/ferrite-rod-core-coil.html"));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Ferrite_Rod::on_lineEdit_dw_editingFinished()
{
    bool ok;
    double d = 0;
    if (fOpt->isAWG){
        d = convertfromAWG(ui->lineEdit_dw->text(), &ok);
    } else {
        d = loc.toDouble(ui->lineEdit_dw->text(), &ok)*fOpt->dwLengthMultiplier;
    }
    if (!ok){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    double k_m = odCalc(d);
    if (d > 0){
        ui->lineEdit_p->setText( roundTo(k_m / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    }
}
