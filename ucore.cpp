/* ucore.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
Copyright (C) 2020 Kustarev V.

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

#include "ucore.h"
#include "ui_ucore.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UCore::UCore(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UCore)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, DBL_MAX, 380);
    ui->lineEdit_a->setValidator(dv);
    ui->lineEdit_b->setValidator(dv);
    ui->lineEdit_c->setValidator(dv);
    ui->lineEdit_d->setValidator(dv);
    ui->lineEdit_e->setValidator(dv);
    ui->lineEdit_f->setValidator(dv);
    ui->lineEdit_s->setValidator(dv);
    ui->lineEdit_mu->setValidator(dv);
    thread = nullptr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UCore::~UCore()
{
    bool isReverse = ui->checkBox_isReverce->isChecked();
    if (isReverse)
        N = loc.toDouble(ui->lineEdit_N->text());
    else
        ind = loc.toDouble(ui->lineEdit_N->text())*fOpt->dwInductanceMultiplier;
    A = loc.toDouble(ui->lineEdit_a->text())*fOpt->dwLengthMultiplier;
    B = loc.toDouble(ui->lineEdit_b->text())*fOpt->dwLengthMultiplier;
    C = loc.toDouble(ui->lineEdit_c->text())*fOpt->dwLengthMultiplier;
    D = loc.toDouble(ui->lineEdit_d->text())*fOpt->dwLengthMultiplier;
    E = loc.toDouble(ui->lineEdit_e->text())*fOpt->dwLengthMultiplier;
    F = loc.toDouble(ui->lineEdit_f->text())*fOpt->dwLengthMultiplier;
    s = loc.toDouble(ui->lineEdit_s->text())*fOpt->dwLengthMultiplier;
    mu = loc.toDouble(ui->lineEdit_mu->text());

    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "UCore" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", this->size());
    settings->setValue("isReverse", ui->checkBox_isReverce->isChecked());
    settings->setValue("isShowSaturation", ui->toolButton_saturation->isChecked());
    settings->setValue("currStdCore", ui->comboBox->currentIndex());
    settings->setValue("ind", ind);
    settings->setValue("N", N);
    settings->setValue("A", A);
    settings->setValue("B", B);
    settings->setValue("C", C);
    settings->setValue("D", D);
    settings->setValue("E", E);
    settings->setValue("F", F);
    settings->setValue("s", s);
    settings->setValue("mu", mu);
    settings->endGroup();

    delete settings;
    delete fOpt;
    delete dv;
    delete ui;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UCore::getOpt(_OptionStruct gOpt)
{
    *fOpt = gOpt;
    ui->label_N_m->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_01->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_02->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_03->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_04->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_05->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_06->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_07->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "UCore" );
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    bool isReverse = settings->value("isReverse", false).toBool();
    bool isShowSaturation = settings->value("isShowSaturation", false).toBool();
    currStdCore = settings->value("currStdCore", 0).toInt();

    N = settings->value("N", 0).toDouble();
    ind = settings->value("ind", 0).toDouble();
    A = settings->value("A", 0).toDouble();
    B = settings->value("B", 0).toDouble();
    C = settings->value("C", 0).toDouble();
    D = settings->value("D", 0).toDouble();
    E = settings->value("E", 0).toDouble();
    F = settings->value("F", 0).toDouble();
    s = settings->value("s", 0).toDouble();
    mu = settings->value("mu", 100).toDouble();
    settings->endGroup();
    if (isReverse)
        ui->lineEdit_N->setText(roundTo(N, loc, fOpt->dwAccuracy));
    else
        ui->lineEdit_N->setText(roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_a->setText(roundTo(A / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_b->setText(roundTo(B / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_c->setText(roundTo(C / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_d->setText(roundTo(D / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_e->setText(roundTo(E / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_f->setText(roundTo(F / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_s->setText(roundTo(s / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_mu->setText(roundTo(mu, loc, fOpt->dwAccuracy));

    ui->label_s->setText(tr("Slot size")+" s:");
    ui->label_mu->setText(tr("Magnetic permeability")+" µ:");
    ui->comboBox->setCurrentIndex(currStdCore);
    on_comboBox_currentIndexChanged(currStdCore);
    resize(size);
    move(pos);
    ui->checkBox_isReverce->setChecked(isReverse);
    on_checkBox_isReverce_clicked();
    ui->toolButton_saturation->setChecked(isShowSaturation);
    ui->toolButton_saturation->setIconSize(QSize(fOpt->mainFontSize * 2, fOpt->mainFontSize * 2));
    delete settings;
    if (fOpt->styleGUI == _DarkStyle){
        ui->pushButton_calculate->setIcon(reverseIconColors(ui->pushButton_calculate->icon()));
        ui->pushButton_close->setIcon(reverseIconColors(ui->pushButton_close->icon()));
        ui->pushButton_help->setIcon(reverseIconColors(ui->pushButton_help->icon()));
        ui->toolButton_saturation->setIcon(reverseIconColors(ui->toolButton_saturation->icon()));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UCore::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UCore::on_checkBox_isReverce_clicked()
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
    ui->lineEdit_N->setFocus();
    ui->lineEdit_N->selectAll();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UCore::on_pushButton_close_clicked()
{
    this->close();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UCore::on_comboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:{
        ui->image->setPixmap(QPixmap(":/images/res/UU-core.png"));
        ui->label_s->setVisible(false);
        ui->lineEdit_s->setVisible(false);
        ui->label_07->setVisible(false);
        ui->label_f->setVisible(false);
        ui->lineEdit_f->setVisible(false);
        ui->label_06->setVisible(false);
    }
        break;
    case 1:{
        ui->image->setPixmap(QPixmap(":/images/res/UR-core.png"));
        ui->label_s->setVisible(true);
        ui->lineEdit_s->setVisible(true);
        ui->label_07->setVisible(true);
        ui->label_f->setVisible(true);
        ui->lineEdit_f->setVisible(true);
        ui->label_06->setVisible(true);
    }
        break;
    case 2:{
        ui->image->setPixmap(QPixmap(":/images/res/UY-core.png"));
        ui->label_s->setVisible(true);
        ui->lineEdit_s->setVisible(true);
        ui->label_07->setVisible(true);
        ui->label_f->setVisible(false);
        ui->lineEdit_f->setVisible(false);
        ui->label_06->setVisible(false);
    }
        break;
    default:
        break;
    }
    ui->lineEdit_N->setFocus();
    ui->lineEdit_N->selectAll();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UCore::on_pushButton_help_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/ferrite-u-core.html"));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UCore::on_pushButton_calculate_clicked()
{
    int index = ui->comboBox->currentIndex();
    if ((ui->lineEdit_N->text().isEmpty())||(ui->lineEdit_a->text().isEmpty())||(ui->lineEdit_b->text().isEmpty())
            ||(ui->lineEdit_c->text().isEmpty()) ||(ui->lineEdit_d->text().isEmpty()) ||(ui->lineEdit_e->text().isEmpty())){
        showWarning(tr("Warning"), tr("One or more inputs are empty!"));
        return;
    }
    bool ok1, ok2, ok3, ok4, ok5, ok6, ok7, ok9;
    A = loc.toDouble(ui->lineEdit_a->text(), &ok1)*fOpt->dwLengthMultiplier;
    B = loc.toDouble(ui->lineEdit_b->text(), &ok2)*fOpt->dwLengthMultiplier;
    C = loc.toDouble(ui->lineEdit_c->text(), &ok3)*fOpt->dwLengthMultiplier;
    D = loc.toDouble(ui->lineEdit_d->text(), &ok4)*fOpt->dwLengthMultiplier;
    E = loc.toDouble(ui->lineEdit_e->text(), &ok5)*fOpt->dwLengthMultiplier;
    if (index == 0){
        F = 0;
    } else if (index == 1){
        F = loc.toDouble(ui->lineEdit_f->text(), &ok6)*fOpt->dwLengthMultiplier;
        if (!ok6){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if (F == 0){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
    } else if (index == 2){
        F = -1;
    }
    if (index > 0){
        s = loc.toDouble(ui->lineEdit_s->text(), &ok7)*fOpt->dwLengthMultiplier;
        if (!ok7){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
    } else {
        s = 0;
    }
    mu = loc.toDouble(ui->lineEdit_mu->text(), &ok9);
    if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok9)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    if ((A == 0)||(B == 0)||(C == 0)||(D == 0)||(E == 0)||(mu == 0)){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        return;
    }
    if (B < D){
        showWarning(tr("Warning"), "B < D");
        return;
    }
    if (ui->checkBox_isReverce->isChecked()){
        N = loc.toDouble(ui->lineEdit_N->text(), &ok1);
        if (!ok1){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if (N == 0){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        //ind = findUCore_I(N,A,B,C,D,E,F,s,mu,&result);
        thread = new MThread_calculate( _U_Core, -2, N, A, B, C, D, E, F, s, Cu, 1, mu);
    } else {
        ind = loc.toDouble(ui->lineEdit_N->text(), &ok1)*fOpt->dwInductanceMultiplier;
        if (!ok1){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if (ind == 0){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        //N = findUCore_N(ind,A,B,C,D,E,F,s,mu,&result);
        thread = new MThread_calculate( _U_Core, -2, ind, A, B, C, D, E, F, s, Cu, 0, mu);
    }
    connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_UCore_Result(_CoilResult)));
    connect(thread, SIGNAL(finished()), this, SLOT(on_calculation_finished()));
    thread->start();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UCore::get_UCore_Result(_CoilResult result)
{
    int index = ui->comboBox->currentIndex();
    if (ui->checkBox_isReverce->isChecked()){
        ind = result.seven;
    } else {
        N = result.six;
    }
    QString sResult = "";
    QString sSatData = "";
    QString sCaption = QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle();
    QString sImage = "";
    switch (index) {
    case 0:
        sImage = "<img src=\":/images/res/UU-core.png\">";
        break;
    case 1:
        sImage = "<img src=\":/images/res/UR-core.png\">";
        break;
    case 2:
        sImage = "<img src=\":/images/res/UY-core.png\">";
        break;
    default:
        break;
    }
    QString sInput = "<p><u>" + tr("Input data") + ":</u><br/>";
    if (ui->checkBox_isReverce->isChecked())
        sInput += formattedOutput(fOpt, ui->label_N->text(), ui->lineEdit_N->text()) + "<br/>";
    else
        sInput += formattedOutput(fOpt, ui->label_N->text(), ui->lineEdit_N->text(), ui->label_N_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_a->text(), ui->lineEdit_a->text(), ui->label_01->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_b->text(), ui->lineEdit_b->text(), ui->label_02->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_c->text(), ui->lineEdit_c->text(), ui->label_03->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_d->text(), ui->lineEdit_d->text(), ui->label_04->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_e->text(), ui->lineEdit_e->text(), ui->label_05->text()) + "<br/>";
    if (index == 1)
        sInput += formattedOutput(fOpt, ui->label_f->text(), ui->lineEdit_f->text(), ui->label_06->text()) + "<br/>";
    if (index > 0)
        sInput += formattedOutput(fOpt, ui->label_s->text(), ui->lineEdit_s->text(), ui->label_07->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_mu->text(), ui->lineEdit_mu->text()) + "<br/>";
    if((unsigned long)N < ULONG_MAX){
        sResult += "<p><u>" + tr("Result") + ":</u><br/>";
        if (ui->checkBox_isReverce->isChecked()){
            sResult += formattedOutput(fOpt, tr("Inductance") + " L = ", roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                       qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
        } else {
            sResult += formattedOutput(fOpt, tr("Number of turns of the coil") + " N = ", QString::number(N));
        }
        sResult += "<br/><br/>" + formattedOutput(fOpt, tr("Effective magnetic path length") + " (l<sub>e</sub>): ",
                                                  roundTo(result.N/fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy),
                                                  qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(fOpt, tr("Effective area of magnetic path") + " (A<sub>e</sub>): ",
                                   roundTo(result.sec/(fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier), loc, fOpt->dwAccuracy),
                                   qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<sup>2</sup>") + "<br/>";
        sResult += formattedOutput(fOpt, tr("Effective volume") + " (V<sub>e</sub>): ",
                                   roundTo(result.N * result.sec/(fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier), loc, fOpt->dwAccuracy),
                                   qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<sup>3</sup>") + "<br/>";
        sResult += "</p>";
        sResult += "</p>";
        QString n = "";
        if (ui->checkBox_isReverce->isChecked())
            n = ui->lineEdit_N->text();
        else
            n = QString::number(N);
        sSatData = n + ";" + ui->lineEdit_mu->text() + ";" + loc.toString(result.N);
    } else {
        sResult += "<span style=\"color:red;\">" + tr("Coil can not be realized") + "! </span>";
    }
    emit sendResult(sCaption + LIST_SEPARATOR + sImage + LIST_SEPARATOR + sInput + LIST_SEPARATOR + sResult + LIST_SEPARATOR + sSatData);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UCore::on_calculation_finished()
{
    thread->deleteLater();
    thread = nullptr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UCore::on_toolButton_saturation_toggled(bool checked)
{
    emit showSaturation(checked);
}
