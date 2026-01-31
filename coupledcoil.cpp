/* coupledcoil.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
Copyright (C) 2025 Kustarev V.

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
#include "coupledcoil.h"
#include "ui_coupledcoil.h"

CoupledCoil::CoupledCoil(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CoupledCoil)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, DBL_MAX, 380);
    iv = new QIntValidator(1, INT16_MAX, this);
    awgV = new QRegExpValidator(QRegExp(AWG_REG_EX));
    ui->lineEdit_D1->setValidator(dv);
    ui->lineEdit_D2->setValidator(dv);
    ui->lineEdit_d->setValidator(dv);
    ui->lineEdit_l1->setValidator(dv);
    ui->lineEdit_l2->setValidator(dv);
    ui->lineEdit_x->setValidator(dv);
    ui->lineEdit_N1->setValidator(iv);
    ui->lineEdit_N2->setValidator(iv);
    ui->lineEdit_D->setValidator(dv);
    ui->lineEdit_dw->setValidator(dv);
    ui->lineEdit_p->setValidator(dv);
    ui->lineEdit_N->setValidator(dv);
    ui->lineEdit_Nt->setValidator(dv);
    ui->lineEdit_F->setValidator(dv);
    ui->lineEdit_load->setValidator(dv);
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(on_timer()));
    thread = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CoupledCoil::~CoupledCoil()
{
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Coupled" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", size());
    settings->setValue("D1", D1);
    settings->setValue("D2", D2);
    settings->setValue("dw", dw);
    settings->setValue("l1", l1);
    settings->setValue("l2", l2);
    settings->setValue("x", xs);
    settings->setValue("N1", N1);
    settings->setValue("N2", N2);
    settings->setValue("D", D);
    settings->setValue("p", p);
    settings->setValue("N", N);
    settings->setValue("Nt", Nt);
    settings->setValue("F", f);
    settings->setValue("Rload", Rload);
    settings->setValue("coilForm", ui->tabWidget->currentIndex());
    settings->endGroup();
    delete settings;
    if(thread != nullptr){
        if(thread->isRunning())
            thread->abort();
    }
    delete fOpt;
    delete awgV;
    delete iv;
    delete dv;
    delete timer;
    delete ui;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::getOpt(_OptionStruct gOpt)
{
    *fOpt = gOpt;
    ui->label_D1_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_D2_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_d_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_l1_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_l2_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_x_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_D_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_dw_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_p_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_F_m->setText(qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Coupled" );
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    int coilForm = settings->value("coilForm", false).toInt();
    D1 = settings->value("D1", 0).toDouble();
    D2 = settings->value("D2", 0).toDouble();
    dw = settings->value("dw", 0).toDouble();
    l1 = settings->value("l1", 0).toDouble();
    l2 = settings->value("l2", 0).toDouble();
    xs = settings->value("x", 0).toDouble();
    N1 = settings->value("N1", 0).toInt();
    N2 = settings->value("N2", 0).toInt();
    D = settings->value("D", 0).toDouble();
    p = settings->value("p", 0).toDouble();
    N = settings->value("N", 0).toDouble();
    Nt = settings->value("Nt", 0).toDouble();
    f = settings->value("F", 0).toDouble();
    Rload = settings->value("Rload", 50).toDouble();
    settings->endGroup();
    resize(size);
    move(pos);
    ui->tabWidget->setCurrentIndex(coilForm);
    on_tabWidget_currentChanged(coilForm);
    if (fOpt->isAWG){
        ui->label_d_m->setText(tr("AWG"));
        ui->lineEdit_d->setValidator(awgV);
        ui->label_dw_m->setText(tr("AWG"));
        ui->lineEdit_dw->setValidator(awgV);
        if (dw > 0){
            ui->lineEdit_d->setText(converttoAWG(dw));
            ui->lineEdit_dw->setText(converttoAWG(dw));
        } else {
            ui->lineEdit_d->setText("");
            ui->lineEdit_dw->setText("");
        }
    } else {
        ui->lineEdit_d->setText(roundTo(dw / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_dw->setText(roundTo(dw / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    }
    ui->lineEdit_D1->setText(roundTo(D1 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_D2->setText(roundTo(D2 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_l1->setText(roundTo(l1 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_l2->setText(roundTo(l2 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_x->setText(roundTo(xs / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_N1->setText(QString::number(N1));
    ui->lineEdit_N2->setText(QString::number(N2));
    ui->lineEdit_D->setText(roundTo(D / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_p->setText(roundTo(p / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_N->setText(roundTo(N, loc, fOpt->dwAccuracy));
    ui->lineEdit_Nt->setText(roundTo(Nt, loc, fOpt->dwAccuracy));
    ui->lineEdit_F->setText(roundTo(f / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_load->setText(roundTo(Rload, loc, fOpt->dwAccuracy));
    ui->label_D->setText(tr("Winding diameter") + " D:");
    ui->label_D1->setText(tr("Winding diameter") + " D1:");
    ui->label_D2->setText(tr("Winding diameter") + " D2:");
    ui->label_d->setText(tr("Wire diameter") + " d:");
    ui->label_dw->setText(tr("Wire diameter") + " d:");
    ui->label_l1->setText(tr("Winding length") + " l1:");
    ui->label_l2->setText(tr("Winding length") + " l2:");
    ui->label_x->setText(tr("Coils offset") + " x:");
    ui->label_p->setText(tr("Winding pitch") + " p:");
    ui->label_N1->setText(tr("Number of turns") + " N1:");
    ui->label_N2->setText(tr("Number of turns") + " N2:");
    ui->label_N->setText(tr("Number of turns") + " N:");
    ui->label_Nt->setText(tr("Number of turns") + " nt:");
    ui->label_F->setText(tr("Frequency") + " f:");
    ui->label_load->setText(tr("Load resistance") + " Rload:");
    ui->toolButton_spice->setChecked(fOpt->isShowLTSpice);
    delete settings;
    if (fOpt->styleGUI == _DarkStyle){
        ui->pushButton_calculate->setIcon(reverseIconColors(ui->pushButton_calculate->icon()));
        ui->pushButton_close->setIcon(reverseIconColors(ui->pushButton_close->icon()));
        ui->pushButton_help->setIcon(reverseIconColors(ui->pushButton_help->icon()));
        ui->toolButton_spice->setIcon(reverseIconColors(ui->toolButton_spice->icon()));
    }
    ui->lineEdit_F->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_pushButton_close_clicked()
{
    this->close();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_pushButton_help_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/coupled-coils.html"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_pushButton_calculate_clicked()
{
    if(thread != nullptr){
        if(thread->isRunning()){
            ui->pushButton_calculate->setEnabled(false);
            thread->abort();
            return;
        }
    }
    int tab = ui->tabWidget->currentIndex();
    if(tab == 0){
        bool ok1, ok2, ok3, ok4, ok5, ok6, ok7, ok8, ok9;
        D1 = loc.toDouble(ui->lineEdit_D1->text(), &ok1)*fOpt->dwLengthMultiplier;
        D2 = loc.toDouble(ui->lineEdit_D2->text(), &ok2)*fOpt->dwLengthMultiplier;
        if (fOpt->isAWG){
            dw = convertfromAWG(ui->lineEdit_d->text(), &ok3);
        } else {
            dw = loc.toDouble(ui->lineEdit_d->text(), &ok3)*fOpt->dwLengthMultiplier;
        }
        l1 = loc.toDouble(ui->lineEdit_l1->text(), &ok4)*fOpt->dwLengthMultiplier;
        l2 = loc.toDouble(ui->lineEdit_l2->text(), &ok5)*fOpt->dwLengthMultiplier;
        xs = loc.toDouble(ui->lineEdit_x->text(), &ok6)*fOpt->dwLengthMultiplier;
        N1 = loc.toInt(ui->lineEdit_N1->text(), &ok7);
        N2 = loc.toInt(ui->lineEdit_N2->text(), &ok8);
        f = loc.toDouble(ui->lineEdit_F->text(), &ok9)*fOpt->dwFrequencyMultiplier;
        if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok7)||(!ok8)||(!ok9)){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if ((D1 == 0)||(D2 == 0)||(dw == 0)||(l1 == 0)||(l2 == 0)||(N1 == 0)||(N2 == 0)||(f == 0)){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        double p1 = l1 / N1;
        double p2 = l2 / N2;
        if (p1 < dw){
            showWarning(tr("Warning"), "p1 < d");
            return;
        }
        if (p2 < dw){
            showWarning(tr("Warning"), "p2 < d");
            return;
        }
        thread = new MThread_calculate( _CoupledCoils, -1, D1, D2, dw, l1, l2, N1, N2, fOpt->dwAccuracy, Cu, xs);
        connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_CoupledCoils_Result(_CoilResult)));
    } else {
        bool ok1, ok2, ok3, ok4, ok5, ok6, ok7;
        D = loc.toDouble(ui->lineEdit_D->text(), &ok1)*fOpt->dwLengthMultiplier;
        if (fOpt->isAWG){
            dw = convertfromAWG(ui->lineEdit_dw->text(), &ok2);
        } else {
            dw = loc.toDouble(ui->lineEdit_dw->text(), &ok2)*fOpt->dwLengthMultiplier;
        }
        p = loc.toDouble(ui->lineEdit_p->text(), &ok3)*fOpt->dwLengthMultiplier;
        if (p < dw){
            showWarning(tr("Warning"), "p < d");
            return;
        }
        N = loc.toDouble(ui->lineEdit_N->text(), &ok4);
        Nt = loc.toDouble(ui->lineEdit_Nt->text(), &ok5);
        if (N < Nt){
            showWarning(tr("Warning"), "N < nt");
            return;
        }
        f = loc.toDouble(ui->lineEdit_F->text(), &ok6)*fOpt->dwFrequencyMultiplier;
        if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        Rload = loc.toDouble(ui->lineEdit_load->text(), &ok7);
        if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok7)){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if ((D == 0)||(p == 0)||(dw == 0)||(N == 0)||(Nt == 0)||(f == 0)||(Rload == 0)){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        thread = new MThread_calculate( _CoupledCoils, -1, D, 0, dw, p, 0, N, Nt, fOpt->dwAccuracy, Cu, 0);
        connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_CoupledCoils_Result(_CoilResult)));
    }
    connect(thread, SIGNAL(started()), this, SLOT(on_calculation_started()));
    connect(thread, SIGNAL(finished()), this, SLOT(on_calculation_finished()));
    thread->start();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::get_CoupledCoils_Result(_CoilResult result)
{
    int tab = ui->tabWidget->currentIndex();
    QString sCaption = QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle();
    QString sImage = "";
    QString sInput = "<p><u>" + tr("Input data") + ":</u><br/>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    if(tab == 0){
        double L1 = result.N;
        double L2 = result.sec;
        double M = result.thd;
        double k = M / sqrt(L1 * L2);
        double lw1 = result.seven;
        double lw2 = result.five;
        sImage = "<img src=\":/images/res/Coupled-coils.png\">";
        sInput += formattedOutput(fOpt, ui->label_D1->text(), ui->lineEdit_D1->text(), ui->label_D1_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_D2->text(), ui->lineEdit_D2->text(), ui->label_D2_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_l1->text(), ui->lineEdit_l1->text(), ui->label_l1_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_l2->text(), ui->lineEdit_l2->text(), ui->label_l2_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_d->text(), ui->lineEdit_d->text(), ui->label_d_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_x->text(), ui->lineEdit_x->text(), ui->label_x_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_N1->text(), ui->lineEdit_N1->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_N2->text(), ui->lineEdit_N2->text()) + "</p>";
        if ((M > 0.0) && (L1 > 0.0) && (L2 > 0.0) && ( k <= 1.0)){
            Material mt = Cu;
            double srF1 = findSRF(l1, D1, lw1);
            double srF2 = findSRF(l2, D2, lw2);
            if ((f < 0.7 * srF1) || (f < 0.7 * srF2)){
                sResult += formattedOutput(fOpt, tr("Inductance") + " L<sub>1</sub> = ", roundTo(L1 / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                           qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
                sResult += formattedOutput(fOpt, tr("Inductance") + " L<sub>2</sub> = ", roundTo(L2 / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                           qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
                sResult += formattedOutput(fOpt, tr("Mutual Inductance") + " M<sub>12</sub> = ", roundTo(M / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                           qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
                sResult += formattedOutput(fOpt, tr("Coupling coefficient") + " k = ", roundTo(k, loc, fOpt->dwAccuracy)) + "<br/>";
                if(fOpt->isShowLTSpice){
                    double Resistivity = MTRL[mt][Rho]*1e6;
                    double R1 = (Resistivity * lw1 * 4) / (M_PI * dw * dw);
                    double R2 = (Resistivity * lw2 * 4) / (M_PI * dw * dw);
                    double Cs1 = find_Cs(l1 / N1, D1, l1);
                    double Cs2 = find_Cs(l2 / N2, D2, l2);
                    double Qr1 = solve_Qr(L1, D1, l1 / N1, dw, f, srF1, N1, Cs1, mt, &result);
                    double ESR1 = result.seven;
                    double Qr2 = solve_Qr(L2, D2, l2 / N2, dw, f, srF2, N2, Cs2, mt, &result);
                    double ESR2 = result.seven;
                    sResult += "<br/><u>" + tr("Input data for LTSpice") + ":</u><br/>";
                    sResult += "<u>L1</u><br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Inductance: ", QString::number(L1, 'f', fOpt->dwAccuracy) + "μ") + "<br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Series resistance: ", QString::number(R1 * 1000, 'f', fOpt->dwAccuracy) + "m") + "<br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Parallel resistance: ", QString::number((ESR1 * (1 + Qr1 * Qr1)) / 1000, 'f', fOpt->dwAccuracy) + "k") + "<br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Parallel capacitance: ", QString::number(Cs1, 'f', fOpt->dwAccuracy) + "p") + "<br/>";
                    sResult += "<u>L2</u><br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Inductance: ", QString::number(L2, 'f', fOpt->dwAccuracy) + "μ") + "<br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Series resistance: ", QString::number(R2 * 1000, 'f', fOpt->dwAccuracy) + "m") + "<br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Parallel resistance: ", QString::number((ESR2 * (1 + Qr2 * Qr2)) / 1000, 'f', fOpt->dwAccuracy) + "k") + "<br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Parallel capacitance: ", QString::number(Cs2, 'f', fOpt->dwAccuracy) + "p") + "<br/>";
                    sResult += "<u>SPICE directive</u><br/>";
                    sResult += "→ " + formattedOutput(fOpt, "", "K1 L1 L2 " + QString::number(k, 'f', fOpt->dwAccuracy)) + "<br/>";
                }
            } else {
                QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
                sResult += "<br/>" + formattedOutput(fOpt, tr("Coil self-resonance frequency") + " Fsr = ", roundTo(result.fourth/fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy),
                                                     qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8())) + "<br/>";
                sResult += "<span style=\"color:blue;\">" + message + "</span>";
            }
        } else if ((M == 0.0) || (L1 == 0.0) || (L2 == 0.0) || (k > 1.0)){
            sResult += "<span style=\"color:red;\">" + tr("Calculation was failed") + "</span>";
        } else {
            sResult += "<span style=\"color:red;\">" + tr("Calculation was aborted") + "</span>";
        }
    } else {
        double L = result.N;
        double L2 = result.thd;
        double L1 = result.fourth;
        double M = 0.5 * (L - L1 - L2);
        double k = M / sqrt(L1 * L2);
        double Tr = (L2 + M) / L;
        sImage = "<img src=\":/images/res/Tapped-coil.png\">";
        sInput += formattedOutput(fOpt, ui->label_D->text(), ui->lineEdit_D->text(), ui->label_D_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_dw->text(), ui->lineEdit_dw->text(), ui->label_dw_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_p->text(), ui->lineEdit_p->text(), ui->label_p_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_N->text(), ui->lineEdit_N->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_Nt->text(), ui->lineEdit_Nt->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_F->text(), ui->lineEdit_F->text(), ui->label_F_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_load->text(), ui->lineEdit_load->text(), ui->label_load_m->text()) + "</p>";
        if ((M > 0.0) && (L1 > 0.0) && (L2 > 0.0) && ( k <= 1.0)){
            Material mt = Cu;
            double srF = findSRF(p * N, D, result.sec);//self-resonance frequency
            if (f < 0.7 * srF){
                double Cs = find_Cs(p, D, p * N); //self-capacitance
                double Qr = solve_Qr(L, D, p, dw, f, srF, N, Cs, mt, &result);//Q-factor

                sResult += formattedOutput(fOpt, tr("Inductance") + " L = ", roundTo(L / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                           qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
                double reactance = 2 * M_PI * L * f;
                sResult += formattedOutput(fOpt, tr("Reactance of the coil") + " X = ", roundTo(reactance, loc, fOpt->dwAccuracy), tr("Ohm")) + "<br/>";
                sResult += formattedOutput(fOpt, tr("Self capacitance") + " Cs = ", roundTo(Cs / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy),
                                           qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
                sResult += formattedOutput(fOpt, tr("Coil self-resonance frequency") + " Fsr = ",
                                           roundTo(srF / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy),
                                           qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8())) + "<br/>";
                sResult += formattedOutput(fOpt, tr("Coil constructive Q-factor") + " Q<sub>0</sub> = ", QString::number(Qr)) + "<br/>";
                double ESR = result.seven;
                sResult += formattedOutput(fOpt, tr("Loss resistance") + " ESR = ", roundTo(ESR, loc, fOpt->dwAccuracy), tr("Ohm")) + "<br/><br/>";
                sResult += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
                double  C = CalcLC2(L, f);
                sResult += "→ " + formattedOutput(fOpt, tr("Circuit capacitance") + " Ck = ", roundTo((C - Cs)/fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy),
                                                     qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
                double ro = 1000 * sqrt(L / C);
                sResult += "→ " + formattedOutput(fOpt, tr("Characteristic impedance") + ": ρ = ",
                                                     roundTo(ro, loc, fOpt->dwAccuracy), tr("Ohm")) + "<br/>";
                double Qs= 1 / (0.001 + 1 / Qr);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
                double Re = ro * Qs;
                sResult += "→ " + formattedOutput(fOpt, tr("LC-circuit Q-factor at Q<sub>C</sub>=1000") + ": Q<sub>U</sub> = ", QString::number((int) Qs)) + "<br/>";
                auto res = getResistancePair(Re, loc, fOpt->dwAccuracy);
                sResult += "→ " + formattedOutput(fOpt, tr("Equivalent resistance") + ": Re = ", res.first, tr(res.second.toLatin1())) + "<br/>";
                double deltaf = 1000 * f / Qs;
                sResult += "→ " + formattedOutput(fOpt, tr("Bandwidth") + ": 3dBΔf = ", roundTo(deltaf, loc, fOpt->dwAccuracy)) + tr("kHz") + "<br/><br/>";
                sResult += "→ " + formattedOutput(fOpt, tr("Effective transformation ratio") + " T = ",
                                                     roundTo(Tr / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy)) + "<br/>";
                double Rout = Tr * Tr * Re;
                double Qex = Rload / (Tr * Tr * reactance);
                double Qd = 1 / (1 / Qs + 1 / Qex);
                res = getResistancePair(Rout, loc, fOpt->dwAccuracy);
                sResult += "→ " + formattedOutput(fOpt, tr("Output resistance") + " Rout = ", res.first, tr(res.second.toLatin1())) + "<br/>";
                sResult += "→ " + formattedOutput(fOpt, tr("Loaded LC-circuit Q-factor") + ": Q<sub>L</sub> = ", QString::number((int) Qd)) + "<br/>";
                sResult += "→ " + formattedOutput(fOpt, tr("Loaded LC-circuit Bandwidth") + ": 3dBΔf = ", roundTo(1000 * f / Qd, loc, fOpt->dwAccuracy)) + tr("kHz") + "<br/><br/>";
                if(fOpt->isShowLTSpice){
                    double Resistivity = MTRL[mt][Rho]*1e6;
                    double R = (Resistivity * result.sec * 4) / (M_PI * dw * dw);
                    double R2 = Nt / N * R;
                    double R1 = R - R2;
                    double lw2 = Nt / N * result.sec;
                    double lw1 = result.sec - lw2;
                    double srF1 = findSRF(p * (N - Nt), D, lw1);
                    double srF2 = findSRF(p * Nt, D, lw2);
                    double Cs2 = find_Cs(p, D, p * Nt);
                    double Cs1 = find_Cs(p, D, p * (N - Nt));
                    double Qr1 = solve_Qr(L1, D, p, dw, f, srF1, (N - Nt), Cs1, mt, &result);
                    double ESR1 = result.seven;
                    double Qr2 = solve_Qr(L2, D, p, dw, f, srF2, Nt, Cs2, mt, &result);
                    double ESR2 = result.seven;
                    sResult += "<br/><u>" + tr("Input data for LTSpice") + ":</u><br/>";
                    sResult += "<u>Lo</u><br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Inductance: ", QString::number(L1, 'f', fOpt->dwAccuracy) + "μ") + "<br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Series resistance: ", QString::number(R1 * 1000, 'f', fOpt->dwAccuracy) + "m") + "<br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Parallel resistance: ", QString::number((ESR1 * (1 + Qr1 * Qr1)) / 1000, 'f', fOpt->dwAccuracy) + "k") + "<br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Parallel capacitance: ", QString::number(Cs1, 'f', fOpt->dwAccuracy) + "p") + "<br/>";
                    sResult += "<u>Lc</u><br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Inductance: ", QString::number(L2, 'f', fOpt->dwAccuracy) + "μ") + "<br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Series resistance: ", QString::number(R2 * 1000, 'f', fOpt->dwAccuracy) + "m") + "<br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Parallel resistance: ", QString::number((ESR2 * (1 + Qr2 * Qr2)) / 1000, 'f', fOpt->dwAccuracy) + "k") + "<br/>";
                    sResult += "→ " + formattedOutput(fOpt, "Parallel capacitance: ", QString::number(Cs2, 'f', fOpt->dwAccuracy) + "p") + "<br/>";
                    sResult += "<u>SPICE directive</u><br/>";
                    sResult += "→ " + formattedOutput(fOpt, "", "K1 Lo Lc " + QString::number(k, 'f', fOpt->dwAccuracy)) + "<br/>";
                }
            } else {
                QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
                sResult += "<br/>" + formattedOutput(fOpt, tr("Coil self-resonance frequency") + " Fsr = ", roundTo(result.fourth/fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy),
                                                     qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8())) + "<br/>";
                sResult += "<span style=\"color:blue;\">" + message + "</span>";
            }
        }
    }
    sResult += "</p>";
    emit sendResult(sCaption + LIST_SEPARATOR + sImage + LIST_SEPARATOR + sInput + LIST_SEPARATOR + sResult);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_timer()
{
    timer->stop();
    ui->pushButton_calculate->setEnabled(true);
    ui->pushButton_calculate->setText(tr("Abort"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_calculation_started()
{
    timer->start(TIMER_INTERVAL);
    this->setCursor(Qt::WaitCursor);
    ui->pushButton_calculate->setEnabled(false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_calculation_finished()
{
    timer->stop();
    ui->pushButton_calculate->setEnabled(true);
    ui->pushButton_calculate->setText(tr("Calculate"));
    this->setCursor(Qt::ArrowCursor);
    thread->deleteLater();
    thread = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_tabWidget_currentChanged(int index)
{
    if (index == 0){
        ui->image->setPixmap(QPixmap(":/images/res/Coupled-coils.png"));
    } else if (index == 1){
        ui->image->setPixmap(QPixmap(":/images/res/Tapped-coil.png"));
    }
    ui->lineEdit_F->setFocus();
    ui->lineEdit_F->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_toolButton_spice_toggled(bool checked)
{
    fOpt->isShowLTSpice = checked;
    emit showLtspice(checked);
}

