/* multisection.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "multisection.h"
#include "ui_multisection.h"

Multisection::Multisection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Multisection)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, DBL_MAX, 380);
    iv = new QIntValidator(1, INT16_MAX, this);
    awgV = new QRegExpValidator(QRegExp(AWG_REG_EX));
    ui->lineEdit_D->setValidator(dv);
    ui->lineEdit_M->setValidator(iv);
    ui->lineEdit_c->setValidator(dv);
    ui->lineEdit_d->setValidator(dv);
    ui->lineEdit_k->setValidator(dv);
    ui->lineEdit_l->setValidator(dv);
    ui->lineEdit_s->setValidator(dv);
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(on_timer()));
    thread = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Multisection::~Multisection()
{
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Multi_section" );
    settings->setValue("pos", this->pos());
    settings->setValue("D", D);
    settings->setValue("c", c);
    settings->setValue("l", l);
    settings->setValue("s", s);
    settings->setValue("k", k);
    settings->setValue("dw", dw);
    settings->setValue("M", M);
    int isOrthocyclic = ui->comboBox_is_orthocyclic->currentIndex();
    settings->setValue("isOrthocyclic", isOrthocyclic);
    settings->endGroup();
    if(thread != nullptr){
        if(thread->isRunning())
            thread->abort();
    }
    delete settings;
    delete fOpt;
    delete awgV;
    delete iv;
    delete dv;
    delete timer;
    delete ui;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multisection::getOpt(_OptionStruct gOpt)
{
    *fOpt = gOpt;
    ui->label_D->setText(tr("Former diameter") + " D:");
    ui->label_c->setText(tr("Thickness of the coil") + " c:");
    ui->label_l->setText(tr("Winding length") + " l:");
    ui->label_s->setText(tr("Section winding spacing") + " s:");
    ui->label_d->setText(tr("Wire diameter") + " d:");
    ui->label_k->setText(tr("Wire diameter with insulation") + " k:");
    ui->label_M->setText(tr("Number of sections") + " M:");
    ui->label_D_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_c_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_l_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_d_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_k_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_s_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Multi_section" );
    D = settings->value("D", 0).toDouble();
    c = settings->value("c", 0).toDouble();
    l = settings->value("l", 0).toDouble();
    s = settings->value("s", 0).toDouble();
    k = settings->value("k", 0).toDouble();
    M = settings->value("M", 0).toInt();
    dw = settings->value("dw", 0).toDouble();
    int isOrthocyclic = settings->value("isOrthocyclic", 0).toInt();
    ui->lineEdit_D->setText(roundTo(D / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_c->setText(roundTo(c / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_l->setText(roundTo(l / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_s->setText(roundTo(s / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_k->setText(roundTo(k / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_M->setText(QString::number(M));
    ui->comboBox_is_orthocyclic->setCurrentIndex(isOrthocyclic);
    if (fOpt->isAWG){
        ui->label_d_m->setText(tr("AWG"));
        ui->lineEdit_d->setValidator(awgV);
        if (dw > 0){
            ui->lineEdit_d->setText(converttoAWG(dw));
        } else
            ui->lineEdit_d->setText("");
    } else
        ui->lineEdit_d->setText(roundTo(dw / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    settings->endGroup();
    move(pos);
    delete settings;
    if (fOpt->styleGUI == _DarkStyle){
        ui->pushButton_calculate->setIcon(reverseIconColors(ui->pushButton_calculate->icon()));
        ui->pushButton_close->setIcon(reverseIconColors(ui->pushButton_close->icon()));
        ui->pushButton_help->setIcon(reverseIconColors(ui->pushButton_help->icon()));
    }
    ui->lineEdit_D->setFocus();
    ui->lineEdit_D->selectAll();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multisection::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multisection::on_pushButton_close_clicked()
{
    this->close();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multisection::on_pushButton_help_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/multi-layer-coil.html"));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multisection::on_lineEdit_d_editingFinished()
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
    double k_m = odCalc(dw);
    if (dw > 0){
        ui->lineEdit_k->setText( roundTo(k_m / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multisection::on_pushButton_calculate_clicked()
{
    if(thread != nullptr){
        if(thread->isRunning()){
            ui->pushButton_calculate->setEnabled(false);
            thread->abort();
            return;
        }
    }
    if ((ui->lineEdit_D->text().isEmpty())||(ui->lineEdit_c->text().isEmpty())||(ui->lineEdit_l->text().isEmpty())||
            (ui->lineEdit_s->text().isEmpty())||(ui->lineEdit_d->text().isEmpty())||(ui->lineEdit_k->text().isEmpty())||(ui->lineEdit_M->text().isEmpty())){
        showWarning(tr("Warning"), tr("One or more inputs are empty!"));
        return;
    }
    bool ok1, ok2, ok3, ok4, ok5, ok6, ok7;
    D = loc.toDouble(ui->lineEdit_D->text(), &ok1)*fOpt->dwLengthMultiplier;
    c = loc.toDouble(ui->lineEdit_c->text(), &ok2)*fOpt->dwLengthMultiplier;
    l = loc.toDouble(ui->lineEdit_l->text(), &ok3)*fOpt->dwLengthMultiplier;
    s = loc.toDouble(ui->lineEdit_s->text(), &ok4)*fOpt->dwLengthMultiplier;
    k = loc.toDouble(ui->lineEdit_k->text(), &ok5)*fOpt->dwLengthMultiplier;
    int isOrthocyclic = ui->comboBox_is_orthocyclic->currentIndex();
    M = loc.toInt(ui->lineEdit_M->text(), &ok6);
    if (fOpt->isAWG){
        dw = convertfromAWG(ui->lineEdit_d->text(), &ok7);
    } else {
        dw = loc.toDouble(ui->lineEdit_d->text(), &ok7)*fOpt->dwLengthMultiplier;
    }
    if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok7)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    if ((D == 0)||(dw == 0)||(c == 0)||(l == 0)||(k == 0)||(M == 0)){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        return;
    }
    thread = new MThread_calculate( _Multisection, -1, D, c, l, s, dw, k, isOrthocyclic, 0, Cu, 0, 0, 0, 0, 0, false, false, M);
    connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_Multisection_Result(_CoilResult)));
    connect(thread, SIGNAL(started()), this, SLOT(on_calculation_started()));
    connect(thread, SIGNAL(finished()), this, SLOT(on_calculation_finished()));
    thread->start();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multisection::get_Multisection_Result(_CoilResult result)
{
    M = loc.toInt(ui->lineEdit_M->text());
    QString sResult = "";
    QString sCaption = QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle();
    QString sImage = "<img src=\":/images/res/multisection.png\">";
    QString sInput = "<p><u>" + tr("Input data") + ":</u><br/>";
    sInput += ui->comboBox_is_orthocyclic->currentText() + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_D->text(), ui->lineEdit_D->text(), ui->label_D_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_l->text(), ui->lineEdit_l->text(), ui->label_l_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_c->text(), ui->lineEdit_c->text(), ui->label_c_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_d->text(), ui->lineEdit_d->text(), ui->label_d_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_k->text(), ui->lineEdit_k->text(), ui->label_k_m->text()) + "<br/>";
    if (M > 1)
        sInput += formattedOutput(fOpt, ui->label_s->text(), ui->lineEdit_s->text(), ui->label_s_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_M->text(), ui->lineEdit_M->text()) + "</p>";
    if(result.N > 0){
        sResult += "<p><u>" + tr("Result") + ":</u><br/>";
        sResult += formattedOutput(fOpt, tr("Number of turns of the coil") + " N = ", loc.toString(result.thd) + "..." + loc.toString(result.fourth)) + "<br/>";
        sResult += formattedOutput(fOpt, tr("Inductance") + " L = ", roundTo(result.N / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy) + "..."
                                   + roundTo(result.sec / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                   qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "</p>";

    } else {
        sResult += "<span style=\"color:red;\">" + tr("Calculation was aborted") + "</span>";
    }
    emit sendResult(sCaption + LIST_SEPARATOR + sImage + LIST_SEPARATOR + sInput + LIST_SEPARATOR + sResult);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multisection::on_timer()
{
    timer->stop();
    ui->pushButton_calculate->setEnabled(true);
    ui->pushButton_calculate->setText(tr("Abort"));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multisection::on_calculation_started()
{
    timer->start(TIMER_INTERVAL);
    this->setCursor(Qt::WaitCursor);
    ui->pushButton_calculate->setEnabled(false);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Multisection::on_calculation_finished()
{
    timer->stop();
    ui->pushButton_calculate->setEnabled(true);
    ui->pushButton_calculate->setText(tr("Calculate"));
    this->setCursor(Qt::ArrowCursor);
    thread->deleteLater();
    thread = nullptr;
}
