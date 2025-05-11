/* ecore.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "ecore.h"
#include "ui_ecore.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ECore::ECore(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ECore)
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
    ui->lineEdit_g->setValidator(dv);
    ui->lineEdit_mu->setValidator(dv);
    thread = nullptr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ECore::~ECore()
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
    g = loc.toDouble(ui->lineEdit_g->text())*fOpt->dwLengthMultiplier;
    mu = loc.toDouble(ui->lineEdit_mu->text());

    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "ECore" );
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
    settings->setValue("g", g);
    settings->setValue("mu", mu);
    settings->endGroup();
    delete settings;
    delete fOpt;
    delete dv;
    delete ui;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ECore::getOpt(_OptionStruct gOpt)
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
    ui->label_08->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "ECore" );
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    bool isReverse = settings->value("isReverse", false).toBool();
    bool isShowSaturation = settings->value("isShowSaturation", false).toBool();
    currStdCore = settings->value("currStdCore", 1).toInt();

    N = settings->value("N", 0).toDouble();
    ind = settings->value("ind", 0).toDouble();
    A = settings->value("A", 0).toDouble();
    B = settings->value("B", 0).toDouble();
    C = settings->value("C", 0).toDouble();
    D = settings->value("D", 0).toDouble();
    E = settings->value("E", 0).toDouble();
    F = settings->value("F", 0).toDouble();
    s = settings->value("s", 0).toDouble();
    g = settings->value("g", 0).toDouble();
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
    ui->lineEdit_g->setText(roundTo(g / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_mu->setText(roundTo(mu, loc, fOpt->dwAccuracy));

    ui->label_s->setText(tr("Slot size")+" b:");
    ui->label_g->setText(tr("Centerpost gap")+" g:");
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
void ECore::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ECore::on_pushButton_close_clicked()
{
    this->close();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ECore::on_comboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:{
        ui->image->setPixmap(QPixmap(":/images/res/EC-core-round.png"));
        ui->label_s->setVisible(true);
        ui->lineEdit_s->setVisible(true);
        ui->label_07->setVisible(true);
    }
        break;
    case 1:{
        ui->image->setPixmap(QPixmap(":/images/res/EE-core-round.png"));
        ui->label_s->setVisible(false);
        ui->lineEdit_s->setVisible(false);
        ui->label_07->setVisible(false);
    }
        break;
    case 2:{
        ui->image->setPixmap(QPixmap(":/images/res/EE-core-square.png"));
        ui->label_s->setVisible(false);
        ui->lineEdit_s->setVisible(false);
        ui->label_07->setVisible(false);
    }
        break;
    case 3:{
        ui->image->setPixmap(QPixmap(":/images/res/EI-core-round.png"));
        ui->label_s->setVisible(false);
        ui->lineEdit_s->setVisible(false);
        ui->label_07->setVisible(false);
    }
        break;
    case 4:{
        ui->image->setPixmap(QPixmap(":/images/res/EI-core-square.png"));
        ui->label_s->setVisible(false);
        ui->lineEdit_s->setVisible(false);
        ui->label_07->setVisible(false);
    }
        break;
    default:
        break;
    }
    ui->lineEdit_N->setFocus();
    ui->lineEdit_N->selectAll();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ECore::on_checkBox_isReverce_clicked()
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
void ECore::on_pushButton_help_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/ferrite-e-core.html"));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ECore::on_pushButton_calculate_clicked()
{
    int index = ui->comboBox->currentIndex();
    bool isEI = false;
    if (index > 2)
        isEI = true;
    bool isRound = true;
    if ((index == 2) || (index == 4))
        isRound = false;
    if ((ui->lineEdit_N->text().isEmpty())||(ui->lineEdit_a->text().isEmpty())||(ui->lineEdit_b->text().isEmpty())
            ||(ui->lineEdit_c->text().isEmpty()) ||(ui->lineEdit_d->text().isEmpty()) ||(ui->lineEdit_e->text().isEmpty())
            ||(ui->lineEdit_f->text().isEmpty())){
        showWarning(tr("Warning"), tr("One or more inputs are empty!"));
        return;
    }
    bool ok1, ok2, ok3, ok4, ok5, ok6, ok7, ok8, ok9;
    A = loc.toDouble(ui->lineEdit_a->text(), &ok1)*fOpt->dwLengthMultiplier;
    B = loc.toDouble(ui->lineEdit_b->text(), &ok2)*fOpt->dwLengthMultiplier;
    C = loc.toDouble(ui->lineEdit_c->text(), &ok3)*fOpt->dwLengthMultiplier;
    D = loc.toDouble(ui->lineEdit_d->text(), &ok4)*fOpt->dwLengthMultiplier;
    E = loc.toDouble(ui->lineEdit_e->text(), &ok5)*fOpt->dwLengthMultiplier;
    F = loc.toDouble(ui->lineEdit_f->text(), &ok6)*fOpt->dwLengthMultiplier;
    if (index == 0){
        s = loc.toDouble(ui->lineEdit_s->text(), &ok7)*fOpt->dwLengthMultiplier;
        if (!ok7){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
    } else {
        s = 0;
    }
    g = loc.toDouble(ui->lineEdit_g->text(), &ok8)*fOpt->dwLengthMultiplier;
    mu = loc.toDouble(ui->lineEdit_mu->text(), &ok9);
    if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok8)||(!ok9)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    if ((A == 0)||(B == 0)||(C == 0)||(D == 0)||(E == 0)||(F == 0)||(mu == 0)){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        return;
    }
    if (A <= E){
        showWarning(tr("Warning"), "A <= E");
        return;
    }
    if (B <= D){
        showWarning(tr("Warning"), "B <= D");
        return;
    }
    if (E <= F){
        showWarning(tr("Warning"), "E <= F");
        return;
    }
    if (C <= s){
        showWarning(tr("Warning"), "C <= b");
        return;
    }
    if (D <= (2 * g)) {
        showWarning(tr("Warning"), "D <= (2 * g)");
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
        thread = new MThread_calculate( _E_Core, -2, N, A, B, C, D, E, F, g, Cu, 1, s, mu, 0, 0, isEI, isRound);
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
        thread = new MThread_calculate( _E_Core, -2, ind, A, B, C, D, E, F, g, Cu, 0, s, mu, 0, 0, isEI, isRound);
    }
    connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_ECore_Result(_CoilResult)));
    connect(thread, SIGNAL(finished()), this, SLOT(on_calculation_finished()));
    thread->start();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ECore::get_ECore_Result(_CoilResult result)
{
    if (ui->checkBox_isReverce->isChecked()){
        ind = result.seven;
    } else {
        N = result.six;
    }
    QString sResult = "";
    QString sSatData = "";
    int index = ui->comboBox->currentIndex();
    QString sCaption = QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle();
    QString sImage = "";
    switch (index) {
    case 0:
        sImage = "<img src=\":/images/res/EC-core-round.png\">";
        break;
    case 1:
        sImage = "<img src=\":/images/res/EE-core-round.png\">";
        break;
    case 2:
        sImage = "<img src=\":/images/res/EE-core-square.png\">";
        break;
    case 3:
        sImage = "<img src=\":/images/res/EI-core-round.png\">";
        break;
    case 4:
        sImage = "<img src=\":/images/res/EI-core-square.png\">";
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
    sInput += formattedOutput(fOpt, ui->label_f->text(), ui->lineEdit_f->text(), ui->label_06->text()) + "<br/>";
    if (index == 0)
        sInput += formattedOutput(fOpt, ui->label_s->text(), ui->lineEdit_s->text(), ui->label_07->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_g->text(), ui->lineEdit_g->text(), ui->label_08->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_mu->text(), ui->lineEdit_mu->text()) + "</p>";
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
                                   qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8())) + "<sup>2</sup><br/>";
        sResult += formattedOutput(fOpt, tr("Effective volume") + " (V<sub>e</sub>): ",
                                   roundTo(result.N * result.sec/(fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier), loc, fOpt->dwAccuracy),
                                   qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8())) + "<sup>3</sup><br/>";
        sResult += formattedOutput(fOpt, tr("Effective magnetic permeability of the core") + " μ<sub>e</sub> = ", roundTo(result.thd, loc, 0));
        sResult += "</p>";
        QString n = "";
        if (ui->checkBox_isReverce->isChecked())
            n = ui->lineEdit_N->text();
        else
            n = QString::number(N);
        sSatData = n + ";" + loc.toString(result.thd) + ";" + loc.toString(result.N);
    } else {
        sResult += "<span style=\"color:red;\">" + tr("Coil can not be realized") + "! </span>";
    }
    emit sendResult(sCaption + LIST_SEPARATOR + sImage + LIST_SEPARATOR + sInput + LIST_SEPARATOR + sResult + LIST_SEPARATOR + sSatData);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ECore::on_calculation_finished()
{
    thread->deleteLater();
    thread = nullptr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ECore::on_toolButton_saturation_toggled(bool checked)
{
    emit showSaturation(checked);
}
