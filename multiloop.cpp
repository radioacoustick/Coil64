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
    dv = new QDoubleValidator(0.0, DBL_MAX, 380);
    awgV = new QRegExpValidator(QRegExp(AWG_REG_EX));
    ui->lineEdit_Di->setValidator(dv);
    ui->lineEdit_Pin->setValidator(dv);
    ui->lineEdit_d->setValidator(dv);
    ui->lineEdit_do->setValidator(dv);
    ui->lineEdit_dw->setValidator(dv);
    ui->lineEdit_dt->setValidator(dv);
    ui->lineEdit_dw2->setValidator(dv);
    ui->lineEdit_dt2->setValidator(dv);
    ui->lineEdit_N->setValidator(dv);
    ui->lineEdit_Ftx->setValidator(dv);
    ui->lineEdit_deltaF->setValidator(dv);
    ui->lineEdit_Ctx->setValidator(dv);
    ui->lineEdit_Crx->setValidator(dv);
    ui->lineEdit_Rtx->setValidator(dv);
    thread = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Multiloop::~Multiloop()
{
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Multi_loop" );
    double Pin = loc.toDouble(ui->lineEdit_Pin->text())*fOpt->dwLengthMultiplier;
    int tab = ui->tabWidget->currentIndex();
    if(tab == 0){
        dt = loc.toDouble(ui->lineEdit_do->text())*fOpt->dwLengthMultiplier;
    } else if(tab == 1){
        dt = loc.toDouble(ui->lineEdit_dt->text())*fOpt->dwLengthMultiplier;
    }
    double f_tx = loc.toDouble(ui->lineEdit_Ftx->text())*fOpt->dwFrequencyMultiplier;
    double deltaF = loc.toDouble(ui->lineEdit_deltaF->text())*fOpt->dwFrequencyMultiplier;
    double Ctx = loc.toDouble(ui->lineEdit_Ctx->text())*fOpt->dwCapacityMultiplier;
    double Crx = loc.toDouble(ui->lineEdit_Crx->text())*fOpt->dwCapacityMultiplier;
    double Rtx = loc.toDouble(ui->lineEdit_Rtx->text());
    double dw2 = loc.toDouble(ui->lineEdit_dw2->text())*fOpt->dwLengthMultiplier;
    double dt2 = loc.toDouble(ui->lineEdit_dt2->text())*fOpt->dwLengthMultiplier;
    int wire_material = 0;
    if (ui->radioButton_aluminum->isChecked())
        wire_material = 1;
    settings->setValue("pos", this->pos());
    settings->setValue("size", this->size());
    settings->setValue("N", nTurns);
    settings->setValue("ind", ind);
    settings->setValue("Di", Di);
    settings->setValue("d", dw);
    settings->setValue("dt", dt);
    settings->setValue("d2", dw2);
    settings->setValue("dt2", dt2);
    settings->setValue("ftx", f_tx);
    settings->setValue("deltaF", deltaF);
    settings->setValue("ctx", Ctx);
    settings->setValue("crx", Crx);
    settings->setValue("rtx", Rtx);
    settings->setValue("Pin", Pin);
    settings->setValue("wire_material", wire_material);
    settings->setValue("isReverse", ui->checkBox_isReverce->isChecked());
    settings->setValue("coilForm", ui->tabWidget->currentIndex());
    settings->setValue("isPlus", isPlus);
    settings->setValue("isIdentical", isIdentical);
    settings->endGroup();
    delete settings;
    delete fOpt;
    delete awgV;
    delete dv;
    delete ui;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::getOpt(_OptionStruct gOpt){
    *fOpt = gOpt;

    ui->tabWidget->setTabText(0, "O-" + tr("Coil"));
    ui->tabWidget->setTabText(1, "DD-" + tr("Coil"));
    ui->label_N->setText(tr("Number of turns") + " N:");
    ui->label_1->setText(tr("Inside diameter") + " Di:");
    ui->label_2->setText(tr("Wire diameter") + " d:");
    ui->label_3->setText(tr("Wire diameter with insulation") + " do:");

    ui->label_Ftx->setText(tr("Frequency") + " Ftx:");
    ui->label_deltaF->setText("TX-RX " + tr("frequency offset") + " ΔF:");
    ui->label_dw->setText(tr("Wire diameter") + " dt:");
    ui->label_dt->setText(tr("Wire diameter with insulation") + " dto:");
    ui->label_dw2->setText(tr("Wire diameter") + " dr:");
    ui->label_dt2->setText(tr("Wire diameter with insulation") + " dro:");
    ui->label_Pin->setText(tr("Inner bundle perimeter") + " Pin:");
    ui->label_Ctx->setText(tr("Capacitance") + " Ctx:");
    ui->label_Crx->setText(tr("Capacitance") + " Crx:");
    ui->label_Rtx->setText(tr("Resistance") + " Rtx:");

    ui->label_N_m->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_01->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_02->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_03->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_Pin_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_dw_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_dt2_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_dw2_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_dt_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_Ftx_m->setText(qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
    ui->label_deltaF_m->setText(qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
    ui->label_Ctx_m->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_Crx_m->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Multi_loop" );
    nTurns = settings->value("N", 0).toULongLong();
    ind = settings->value("ind", 0).toDouble();
    Di = settings->value("Di", 0).toDouble();
    dw = settings->value("d", 0).toDouble();
    dt = settings->value("dt", 0).toDouble();
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    bool isReverse = settings->value("isReverse", false).toBool();
    int coilForm = settings->value("coilForm", false).toInt();
    int wire_material = settings->value( "wire_material", 0 ).toInt();
    double f_tx = settings->value("ftx", 0).toDouble();
    double deltaF = settings->value("deltaF", 0).toDouble();
    double ctx = settings->value("ctx", 0).toDouble();
    double crx = settings->value("crx", 0).toDouble();
    double rtx = settings->value("rtx", 0).toDouble();
    double Pin = settings->value("Pin", 0).toDouble();
    double dw2 = settings->value("d2", 0).toDouble();
    double dt2 = settings->value("dt2", 0).toDouble();
    isPlus = settings->value("isPlus", false).toBool();
    isIdentical = settings->value("isIdentical", false).toBool();
    settings->endGroup();
    if (wire_material == 0){
        ui->radioButton_copper->setChecked(true);
    } else if (wire_material == 1){
        ui->radioButton_aluminum->setChecked(true);
    }
    ui->checkBox_isReverce->setChecked(isReverse);
    ui->lineEdit_Ftx->setText(roundTo(f_tx / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_deltaF->setText(roundTo(deltaF / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_Ctx->setText(roundTo(ctx / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_Crx->setText(roundTo(crx / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_Rtx->setText(roundTo(rtx, loc, fOpt->dwAccuracy));
    ui->lineEdit_Pin->setText(roundTo(Pin / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->toolButton->setChecked(isPlus);
    on_toolButton_toggled(isPlus);
    on_checkBox_isReverce_clicked();
    ui->checkBox_identical->setChecked(isIdentical);
    ui->tabWidget->setCurrentIndex(coilForm);
    on_tabWidget_currentChanged(coilForm);
    if (!ui->checkBox_isReverce->isChecked())
        ui->lineEdit_N->setText(roundTo(nTurns, loc, fOpt->dwAccuracy));
    else
        ui->lineEdit_N->setText(roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_Di->setText(roundTo(Di / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    if (fOpt->isAWG){
        ui->label_02->setText(tr("AWG"));
        ui->label_dw->setText(tr("AWG"));
        ui->label_dw2->setText(tr("AWG"));
        ui->lineEdit_d->setValidator(awgV);
        ui->lineEdit_dw->setValidator(awgV);
        ui->lineEdit_dw2->setValidator(awgV);
        if (dw > 0){
            ui->lineEdit_d->setText(converttoAWG(dw));
            ui->lineEdit_dw->setText(converttoAWG(dw));
            ui->lineEdit_dw2->setText(converttoAWG(dw2));
        } else {
            ui->lineEdit_d->setText("");
            ui->lineEdit_dw->setText("");
            ui->lineEdit_dw2->setText("");
        }
    } else {
        ui->lineEdit_d->setText(roundTo(dw / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_dw->setText(roundTo(dw / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_dw2->setText(roundTo(dw2 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    }
    if (ui->lineEdit_d->text().isEmpty() || (ui->lineEdit_d->text() == "0") || (dt > 0))
        ui->lineEdit_do->setText(roundTo(dt / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    else
        on_lineEdit_2_editingFinished();
    if (ui->lineEdit_dw->text().isEmpty() || (ui->lineEdit_dw->text() == "0") || (dt > 0))
        ui->lineEdit_dt->setText(roundTo(dt / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    else
        on_lineEdit_dw_editingFinished();
    if (ui->lineEdit_dw2->text().isEmpty() || (ui->lineEdit_dw2->text() == "0") || (dt2 > 0))
        ui->lineEdit_dt2->setText(roundTo(dt2 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    else
        on_lineEdit_dw2_editingFinished();
    resize(size);
    move(pos);
    delete settings;
    if (fOpt->styleGUI == _DarkStyle){
        ui->pushButton_calculate->setIcon(reverseIconColors(ui->pushButton_calculate->icon()));
        ui->pushButton_close->setIcon(reverseIconColors(ui->pushButton_close->icon()));
        ui->pushButton_help->setIcon(reverseIconColors(ui->pushButton_help->icon()));
        ui->pushButton_export->setIcon(reverseIconColors(ui->pushButton_export->icon()));
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::getCurrentLocale(QLocale locale){
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_pushButton_calculate_clicked()
{
    int tab = ui->tabWidget->currentIndex();
    if(tab == 0){
        if ((ui->lineEdit_N->text().isEmpty())||(ui->lineEdit_Di->text().isEmpty())||(ui->lineEdit_d->text().isEmpty())||(ui->lineEdit_do->text().isEmpty())){
            showWarning(tr("Warning"), tr("One or more inputs are empty!"));
            return;
        }
        bool ok1, ok2, ok3, ok4;
        if (!ui->checkBox_isReverce->isChecked()){
            nTurns = loc.toInt(ui->lineEdit_N->text(), &ok1);
            if (nTurns == 0){
                showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                return;
            }
        } else {
            ind = loc.toDouble(ui->lineEdit_N->text(), &ok1)*fOpt->dwInductanceMultiplier;
            if (ind == 0){
                showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                return;
            }
        }
        double Di = loc.toDouble(ui->lineEdit_Di->text(), &ok2)*fOpt->dwLengthMultiplier;
        if (fOpt->isAWG){
            dw = convertfromAWG(ui->lineEdit_d->text(), &ok3);
        } else {
            dw = loc.toDouble(ui->lineEdit_d->text(), &ok3)*fOpt->dwLengthMultiplier;
        }
        double dt = loc.toDouble(ui->lineEdit_do->text(), &ok4)*fOpt->dwLengthMultiplier;
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
        if (!ui->checkBox_isReverce->isChecked())
            thread = new MThread_calculate( _Multiloop, -1, nTurns, Di, 0, dt, 0, 0, 0, 0);
        else
            thread = new MThread_calculate( _Multiloop, -1, ind, Di, 0, dt, 1, 0, 0, 0);
    } else if(tab == 1){
        if ((ui->lineEdit_Ftx->text().isEmpty())||(ui->lineEdit_deltaF->text().isEmpty())||(ui->lineEdit_Ctx->text().isEmpty())||(ui->lineEdit_Rtx->text().isEmpty())
                ||(ui->lineEdit_Pin->text().isEmpty())||(ui->lineEdit_dw->text().isEmpty())||(ui->lineEdit_dt->text().isEmpty())){
            showWarning(tr("Warning"), tr("One or more inputs are empty!"));
            return;
        }
        bool ok1, ok2, ok3, ok4, ok5, ok6, ok7, ok8, ok9, ok10;
        double f_tx = loc.toDouble(ui->lineEdit_Ftx->text(), &ok1)*fOpt->dwFrequencyMultiplier;
        double Ctx = loc.toDouble(ui->lineEdit_Ctx->text(), &ok2)*fOpt->dwCapacityMultiplier;
        double Pin = loc.toDouble(ui->lineEdit_Pin->text(), &ok3)*fOpt->dwLengthMultiplier;
        if (fOpt->isAWG){
            dw = convertfromAWG(ui->lineEdit_dw->text(), &ok4);
        } else {
            dw = loc.toDouble(ui->lineEdit_dw->text(), &ok4)*fOpt->dwLengthMultiplier;
        }
        double dt = loc.toDouble(ui->lineEdit_dt->text(), &ok5)*fOpt->dwLengthMultiplier;
        double deltaF = loc.toDouble(ui->lineEdit_deltaF->text(), &ok6)*fOpt->dwFrequencyMultiplier;
        loc.toDouble(ui->lineEdit_Rtx->text(), &ok7);
        if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok7)){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if ((f_tx == 0)||(Ctx == 0)||(dw == 0)||(dt == 0)||(Pin == 0)){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        if (dt < dw){
            showWarning(tr("Warning"), "dto < dt");
            return;
        }
        double dw2 = 0.0;
        double dt2 = 0.0;
        double Crx = Ctx;
        if(!isIdentical){
            if (fOpt->isAWG){
                dw2 = convertfromAWG(ui->lineEdit_dw2->text(), &ok8);
            } else {
                dw2 = loc.toDouble(ui->lineEdit_dw2->text(), &ok8)*fOpt->dwLengthMultiplier;
            }
            Crx = loc.toDouble(ui->lineEdit_Crx->text(), &ok9)*fOpt->dwCapacityMultiplier;
            dt2 = loc.toDouble(ui->lineEdit_dt2->text(), &ok10)*fOpt->dwLengthMultiplier;
            if((!ok8)||(!ok9)||(!ok10)){
                showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                return;
            }
            if ((Crx == 0)||(dw2 == 0)||(dt2 == 0)){
                showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                return;
            }
            if (dt2 < dw2){
                showWarning(tr("Warning"), "dro < dr");
                return;
            }
        }
        double indTx = CalcLC1(Ctx, f_tx);
        double f_rx = f_tx - deltaF;
        if(isPlus)
            f_rx = f_tx + deltaF;
        double indRx = CalcLC1(Crx, f_rx);
        thread = new MThread_calculate( _Multiloop, -1, Pin, indTx, indRx, 0, dt, 0, dt2, 1);
    }
    connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_Multiloop_Result(_CoilResult)));
    connect(thread, SIGNAL(finished()), this, SLOT(on_calculation_finished()));
    thread->start();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::get_Multiloop_Result(_CoilResult result)
{
    int tab = ui->tabWidget->currentIndex();
    QString sResult = "";
    Material mt = Cu;
    if(ui->radioButton_aluminum->isChecked())
        mt = Al;
    if(tab == 0){
        if (ui->checkBox_isReverce->isChecked()){
            nTurns = (double)result.six;
            if(!((unsigned long)nTurns < ULONG_MAX)){
                ui->label_result->setText("<span style=\"color:red;\">" + tr("Coil can not be realized") + "! </span>");
                return;
            } else {
                sResult += formattedOutput(fOpt, tr("Number of turns of the coil") + " N = ", roundTo(nTurns, loc, fOpt->dwAccuracy)) + "<br/>";
            }
        } else {
            ind = result.seven;
            sResult += formattedOutput(fOpt, tr("Inductance") + " L = ", roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                       qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
        }
        sResult += formattedOutput(fOpt, tr("Mean diameter") + " Dm = ", roundTo( (result.N)/fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy ),
                                   qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
        sResult += showCoilParameters(result.sec, dw, result.thd, mt);
    } else if(tab == 1) {
        if((result.N < 0) || (result.seven < 0)){
            sResult += "<span style=\"color:red;\">" + tr("Calculation was aborted") + "</span>";
            ui->label_result->setText(sResult);
            return;
        }
        double Pin = loc.toDouble(ui->lineEdit_Pin->text())*fOpt->dwLengthMultiplier;
        double f_tx = loc.toDouble(ui->lineEdit_Ftx->text())*fOpt->dwFrequencyMultiplier;
        double Ctx = loc.toDouble(ui->lineEdit_Ctx->text())*fOpt->dwCapacityMultiplier;
        double Rtx = loc.toDouble(ui->lineEdit_Rtx->text());
        double ro = 1000 * sqrt(ind / Ctx);
        double Resistivity = MTRL[mt][Rho]*1e2;
        double resistance = (Resistivity * result.thd * 100 * 4) / (M_PI * dw * dw * 0.01); //Resistance to DC (Ohm)
        double Qtx = ro / (Rtx + resistance);
        if((result.N == 10000) || (result.N < 2)){
            sResult += "<span style=\"color:red;\">TX " + tr("Coil can not be realized") + "! </span>";
        } else {
            ind = CalcLC1(Ctx, f_tx);
            nTurns = result.N;
            Di = Pin / M_PI;
            sResult += formattedOutput(fOpt, tr("Equivalent diameter") + " Deq = ", roundTo( Di/fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy ),
                                       qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
            QString p = "TX ";
            QString o = "";
            if(isIdentical){
                p = "";
                o = "<br/>";
            } else {
                sResult += "<br/>";
            }
            sResult += formattedOutput(fOpt, p + tr("Inductance") + " L = ", roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                       qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>" + o;
            sResult += formattedOutput(fOpt, p +  tr("Number of turns of the coil") + " N = ", roundTo(nTurns, loc, fOpt->dwAccuracy)) + "<br/>";
            sResult += showCoilParameters(result.sec, dw, result.thd, mt, p);
            sResult += formattedOutput(fOpt, "TX " + tr("Characteristic impedance") + " ρ = ", roundTo(ro, loc, fOpt->dwAccuracy), tr("Ohm")) + "<br/>";
            sResult += formattedOutput(fOpt, "TX " + tr("LC-circut quality factor") + " Qt = ", roundTo(Qtx, loc, 0)) + "<br/>";
        }
        sResult += "<br/>";
        double deltaF = loc.toDouble(ui->lineEdit_deltaF->text())*fOpt->dwFrequencyMultiplier;
        double f_rx = f_tx - deltaF;
        if(isPlus)
            f_rx = f_tx + deltaF;
        sResult += formattedOutput(fOpt, "RX " + tr("frequency") + " Frx:", roundTo( f_rx /fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy),
                                   qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8())) + "<br/>";
        if(isIdentical){
            double Crx = CalcLC2(ind, f_rx);
            double ro2 = 1000 * sqrt(ind / Crx);
            double Qrx = ro2 / resistance;
            sResult += formattedOutput(fOpt, "RX " + tr("capacitance") + " Crx:", roundTo( Crx /fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy),
                                       qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
            sResult += formattedOutput(fOpt, "RX " + tr("Characteristic impedance") + " ρ = ", roundTo(ro2, loc, fOpt->dwAccuracy), tr("Ohm")) + "<br/>";
            sResult += formattedOutput(fOpt, "RX " + tr("LC-circut quality factor") + " Qr = ", roundTo(Qrx, loc, 0)) + "<br/>";
        } else {
            double Crx = loc.toDouble(ui->lineEdit_Crx->text())*fOpt->dwCapacityMultiplier;
            double dw2 = 0.0;
            if (fOpt->isAWG){
                dw2 = convertfromAWG(ui->lineEdit_dw2->text());
            } else {
                dw2 = loc.toDouble(ui->lineEdit_dw2->text())*fOpt->dwLengthMultiplier;
            }
            double indRX = CalcLC1(Crx, f_rx);
            double nTurnsRX = result.seven;
            double ro = 1000 * sqrt(indRX / Crx);
            double Resistivity = MTRL[mt][Rho]*1e2;
            double resistance = (Resistivity * result.five * 100 * 4) / (M_PI * dw2 * dw2 * 0.01); //Resistance to DC (Ohm)
            double Qrx = ro / resistance;
            if((result.seven == 10000) || (result.seven < 2)){
                sResult += "<span style=\"color:red;\">RX " + tr("Coil can not be realized") + "! </span>";
            } else {
                sResult += formattedOutput(fOpt, "RX " + tr("Inductance") + " L = ", roundTo(indRX / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                           qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
                sResult += formattedOutput(fOpt, "RX " +  tr("Number of turns of the coil") + " N = ", roundTo(nTurnsRX, loc, fOpt->dwAccuracy)) + "<br/>";
                sResult += showCoilParameters(result.fourth, dw2, result.five, mt, "RX ");
                sResult += formattedOutput(fOpt, "RX " + tr("Characteristic impedance") + " ρ = ", roundTo(ro, loc, fOpt->dwAccuracy), tr("Ohm")) + "<br/>";
                sResult += formattedOutput(fOpt, "RX " + tr("LC-circut quality factor") + " Qr = ", roundTo(Qrx, loc, 0)) + "<br/>";
            }
        }
    }
    ui->label_result->setText(sResult);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString Multiloop::showCoilParameters(double T, double dw, double wlength, Material mt, QString head)
{
    QString result = head  + formattedOutput(fOpt, tr("Thickness of the coil") + " T = ", roundTo( (4.0 / M_PI * T)/fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy),
                               qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    double Resistivity = MTRL[mt][Rho]*1e2;
    double resistance = (Resistivity * wlength * 100 * 4) / (M_PI * dw * dw * 0.01); //Resistance to DC (Ohm)
    result += head  + formattedOutput(fOpt, tr("Resistance of the coil") + " R = ", roundTo(resistance, loc, fOpt->dwAccuracy), tr("Ohm")) + "<br/>";
    QString _wire_length = formatLength(wlength, fOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(" ", skip_empty_parts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    result += head  + formattedOutput(fOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, fOpt->dwAccuracy),
                               qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
    double dencity = MTRL[mt][Dencity];
    double mass = 0.25 * dencity * M_PI * dw * dw * wlength;
    result += head  + formattedOutput(fOpt, tr("Weight of wire") + " m = ", roundTo(mass, loc, fOpt->dwAccuracy), tr("g")) + "<br/>";
    return result;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_pushButton_export_clicked()
{
    int tab = ui->tabWidget->currentIndex();
    QString sCaption = QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle();
    sCaption += "<br/>" + ui->tabWidget->tabText(ui->tabWidget->currentIndex());
    QString sImage = "<img src=\":/images/res/multi_loop.png\">";
    QString sInput = "<p><u>" + tr("Input data") + ":</u><br/>";
    if(tab == 1){
        sImage = "<img src=\":/images/res/d-coil.png\">";
    }
    if(tab == 0){
        QString measureUnit = "";
        if (!ui->checkBox_isReverce->isChecked())
            measureUnit = "";
        else
            measureUnit = ui->label_N_m->text();
        sInput += formattedOutput(fOpt, ui->label_N->text(), ui->lineEdit_N->text(), measureUnit) + "<br/>";
        sInput += "<u>" + tr("Dimensions") + ":</u><br/>";
        sInput += formattedOutput(fOpt, ui->label_1->text(), ui->lineEdit_Di->text(), ui->label_01->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_2->text(), ui->lineEdit_d->text(), ui->label_02->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_3->text(), ui->lineEdit_do->text(), ui->label_03->text()) + "</p>";
    } else if (tab == 1) {
        sInput += formattedOutput(fOpt, ui->label_Ftx->text(), ui->lineEdit_Ftx->text(), ui->label_Ftx_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_deltaF->text(), ui->lineEdit_deltaF->text(), ui->label_deltaF_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_Pin->text(), ui->lineEdit_Pin->text(), ui->label_Pin_m->text()) + "<br/>";
        sInput += "<u>" + ui->groupBox_TX->title() + ":</u><br/>";
        sInput += formattedOutput(fOpt, ui->label_Ctx->text(), ui->lineEdit_Ctx->text(), ui->label_Ctx_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_Rtx->text(), ui->lineEdit_Rtx->text(), ui->label_Rtx_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_dw->text(), ui->lineEdit_dw->text(), ui->label_dw_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_dt->text(), ui->lineEdit_dt->text(), ui->label_dt_m->text()) + "<br/>";
        if(!isIdentical){
            sInput += "<u>" + ui->groupBox_RX->title() + ":</u><br/>";
            sInput += formattedOutput(fOpt, ui->label_Crx->text(), ui->lineEdit_Crx->text(), ui->label_Crx_m->text()) + "<br/>";
            sInput += formattedOutput(fOpt, ui->label_dw2->text(), ui->lineEdit_dw2->text(), ui->label_dw2_m->text()) + "<br/>";
            sInput += formattedOutput(fOpt, ui->label_dt2->text(), ui->lineEdit_dt2->text(), ui->label_dt2_m->text()) + "<br/>";
        }
        sInput += "<br/>";
        QString material_name = getCheckedRadioButton(ui->groupBox_Material)->text();
        sInput += formattedOutput(fOpt, ui->groupBox_Material->title().mid(0, ui->groupBox_Material->title().length() - 1) + " Mt:", material_name) + "</p>";
    }
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    sResult += ui->label_result->text();
    sResult += "</p>";
    if(ui->label_result->text().length() > 0)
        emit sendResult(sCaption + LIST_SEPARATOR + sImage + LIST_SEPARATOR + sInput + LIST_SEPARATOR + sResult);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_calculation_finished()
{
    thread->deleteLater();
    thread = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_pushButton_close_clicked()
{
    this->close();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_pushButton_help_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/metal-detector-search-coil.html"));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_lineEdit_2_editingFinished()
{
    bool ok;
    if (fOpt->isAWG){
        dw = convertfromAWG(ui->lineEdit_d->text(), &ok);
    } else {
        dw = loc.toDouble(ui->lineEdit_d->text(), &ok)*fOpt->dwLengthMultiplier;
    }
    if (!ok){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    dt = odCalc(dw);
    if (dw > 0){
        ui->lineEdit_do->setText( roundTo(dt / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_lineEdit_dw_editingFinished()
{
    bool ok;
    if (fOpt->isAWG){
        dw = convertfromAWG(ui->lineEdit_dw->text(), &ok);
    } else {
        dw = loc.toDouble(ui->lineEdit_dw->text(), &ok)*fOpt->dwLengthMultiplier;
    }
    if (!ok){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    dt = odCalc(dw);
    if (dw > 0){
        ui->lineEdit_dt->setText( roundTo(dt / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_lineEdit_dw2_editingFinished()
{
    bool ok;
    double dw2 = 0.0;
    if (fOpt->isAWG){
        dw2 = convertfromAWG(ui->lineEdit_dw2->text(), &ok);
    } else {
        dw2 = loc.toDouble(ui->lineEdit_dw2->text(), &ok)*fOpt->dwLengthMultiplier;
    }
    if (!ok){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    double dt2 = odCalc(dw2);
    if (dw2 > 0){
        ui->lineEdit_dt2->setText( roundTo(dt2 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_lineEdit_Di_editingFinished()
{
    bool ok;
    Di = convertfromAWG(ui->lineEdit_Di->text(), &ok);
    if (!ok){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_checkBox_isReverce_clicked()
{
    QString tmp_txt;
    if (!ui->checkBox_isReverce->isChecked()){
        tmp_txt = tr("Number of turns") + " N:";
        ui->label_N->setText(tmp_txt);
        ui->lineEdit_N->setText(roundTo(nTurns, loc, fOpt->dwAccuracy));
        ui->label_N_m->setVisible(false);
    } else {
        tmp_txt = tr("Inductance") + " L:";
        ui->label_N->setText(tmp_txt);
        ui->lineEdit_N->setText(roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
        ui->label_N_m->setVisible(true);
    }
    ui->lineEdit_N->setFocus();
    ui->lineEdit_N->selectAll();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_tabWidget_currentChanged(int index)
{
    if (index == 0){
        ui->image->setPixmap(QPixmap(":/images/res/multi_loop.png"));
        ui->lineEdit_Di->setText(roundTo(Di / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
        if (!ui->checkBox_isReverce->isChecked())
            ui->lineEdit_N->setText(roundTo(nTurns, loc, fOpt->dwAccuracy));
        else
            ui->lineEdit_N->setText(roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
        if (dw > 0){
            if (fOpt->isAWG){
                ui->lineEdit_d->setText(converttoAWG(dw));
            } else {
                ui->lineEdit_d->setText(roundTo(dw / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
            }
            ui->lineEdit_do->setText(roundTo(dt / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
        }
        ui->lineEdit_N->setFocus();
        ui->lineEdit_N->selectAll();
    } else if (index == 1){
        ui->image->setPixmap(QPixmap(":/images/res/d-coil.png"));
        if (dw > 0){
            if (fOpt->isAWG){
                ui->lineEdit_dw->setText(converttoAWG(dw));
            } else {
                ui->lineEdit_dw->setText(roundTo(dw / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
            }
            ui->lineEdit_dt->setText(roundTo(dt / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
        }
        ui->lineEdit_Ftx->setFocus();
        ui->lineEdit_Ftx->selectAll();
    }
    ui->label_result->setText("");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_toolButton_toggled(bool checked)
{
   isPlus = checked;
   if(isPlus)
       ui->toolButton->setText("+");
   else
       ui->toolButton->setText("-");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multiloop::on_checkBox_identical_toggled(bool checked)
{
    isIdentical = checked;
    ui->groupBox_RX->setVisible(!checked);
}

