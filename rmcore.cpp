/* rmcore.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
Copyright (C) 2022 Kustarev V.

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

#include "rmcore.h"
#include "ui_rmcore.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RMcore::RMcore(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RMcore)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, MAX_DOUBLE, 380);
    ui->lineEdit_a->setValidator(dv);
    ui->lineEdit_b->setValidator(dv);
    ui->lineEdit_c->setValidator(dv);
    ui->lineEdit_e->setValidator(dv);
    ui->lineEdit_d2->setValidator(dv);
    ui->lineEdit_d3->setValidator(dv);
    ui->lineEdit_d4->setValidator(dv);
    ui->lineEdit_h1->setValidator(dv);
    ui->lineEdit_h2->setValidator(dv);
    ui->lineEdit_g->setValidator(dv);
    ui->lineEdit_mu->setValidator(dv);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RMcore::~RMcore()
{
    bool isReverse = ui->checkBox_isReverce->isChecked();
    if (isReverse)
        N = loc.toDouble(ui->lineEdit_N->text());
    else
        ind = loc.toDouble(ui->lineEdit_N->text())*fOpt->dwInductanceMultiplier;
    a = loc.toDouble(ui->lineEdit_a->text())*fOpt->dwLengthMultiplier;
    b = loc.toDouble(ui->lineEdit_b->text())*fOpt->dwLengthMultiplier;
    c = loc.toDouble(ui->lineEdit_c->text())*fOpt->dwLengthMultiplier;
    e = loc.toDouble(ui->lineEdit_e->text())*fOpt->dwLengthMultiplier;
    d2 = loc.toDouble(ui->lineEdit_d2->text())*fOpt->dwLengthMultiplier;
    d3 = loc.toDouble(ui->lineEdit_d3->text())*fOpt->dwLengthMultiplier;
    d4 = loc.toDouble(ui->lineEdit_d4->text())*fOpt->dwLengthMultiplier;
    h1 = loc.toDouble(ui->lineEdit_h1->text())*fOpt->dwLengthMultiplier;
    h2 = loc.toDouble(ui->lineEdit_h2->text())*fOpt->dwLengthMultiplier;
    g = loc.toDouble(ui->lineEdit_g->text())*fOpt->dwLengthMultiplier;
    mu = loc.toDouble(ui->lineEdit_mu->text());

    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "RMcore" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", this->size());
    settings->setValue("isReverse", isReverse);
    settings->setValue("isShowSaturation", ui->toolButton_saturation->isChecked());
    settings->setValue("currTypeCore", ui->comboBox->currentIndex());
    settings->setValue("ind", ind);
    settings->setValue("N", N);
    settings->setValue("a", a);
    settings->setValue("b", b);
    settings->setValue("c", c);
    settings->setValue("e", e);
    settings->setValue("d2", d2);
    settings->setValue("d3", d3);
    settings->setValue("d4", d4);
    settings->setValue("h1", h1);
    settings->setValue("h2", h2);
    settings->setValue("g", g);
    settings->setValue("mu", mu);
    settings->endGroup();

    delete settings;
    delete fOpt;
    delete dv;
    delete ui;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RMcore::getOpt(_OptionStruct gOpt)
{
    *fOpt = gOpt;
    ui->label_N_m->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_am->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_bm->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_cm->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_em->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_d2m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_d3m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_d4m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_h1m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_h2m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_gm->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "RMcore" );
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    bool isReverse = settings->value("isReverse", false).toBool();
    bool isShowSaturation = settings->value("isShowSaturation", false).toBool();
    currTypeCore = settings->value("currTypeCore", 1).toInt();

    N = settings->value("N", 0).toDouble();
    ind = settings->value("ind", 0).toDouble();
    a = settings->value("a", 0).toDouble();
    b = settings->value("b", 0).toDouble();
    c = settings->value("c", 0).toDouble();
    e = settings->value("e", 0).toDouble();
    d2 = settings->value("d2", 0).toDouble();
    d3 = settings->value("d3", 0).toDouble();
    d4 = settings->value("d4", 0).toDouble();
    h1 = settings->value("h1", 0).toDouble();
    h2 = settings->value("h2", 0).toDouble();
    g = settings->value("g", 0).toDouble();
    mu = settings->value("mu", 100).toDouble();
    settings->endGroup();
    if (isReverse)
        ui->lineEdit_N->setText(roundTo(N, loc, fOpt->dwAccuracy));
    else
        ui->lineEdit_N->setText(roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_a->setText(roundTo(a / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_b->setText(roundTo(b / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_c->setText(roundTo(c / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_e->setText(roundTo(e / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_d2->setText(roundTo(d2 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_d3->setText(roundTo(d3 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_d4->setText(roundTo(d4 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_h1->setText(roundTo(h1 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_h2->setText(roundTo(h2 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_g->setText(roundTo(g / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_mu->setText(roundTo(mu, loc, fOpt->dwAccuracy));

    ui->label_g->setText(tr("Centerpost gap")+" g:");
    ui->label_mu->setText(tr("Magnetic permeability")+" µ:");
    ui->comboBox->setCurrentIndex(currTypeCore);
    on_comboBox_currentIndexChanged(currTypeCore);
    resize(size);
    move(pos);
    ui->checkBox_isReverce->setChecked(isReverse);
    on_checkBox_isReverce_clicked();
    ui->toolButton_saturation->setChecked(isShowSaturation);
    ui->toolButton_saturation->setIconSize(QSize(fOpt->mainFontSize * 2, fOpt->mainFontSize * 2));
    delete settings;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RMcore::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RMcore::on_pushButton_close_clicked()
{
    this->close();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RMcore::on_comboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
    case 1:
        ui->image->setPixmap(QPixmap(":/images/res/rm-core1.png"));
        break;
    case 2:
        ui->image->setPixmap(QPixmap(":/images/res/rm-core2.png"));
        break;
    case 3:
        ui->image->setPixmap(QPixmap(":/images/res/rm-core3.png"));
        break;
    default:
        break;
    }
    ui->lineEdit_N->setFocus();
    ui->lineEdit_N->selectAll();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RMcore::on_checkBox_isReverce_clicked()
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
void RMcore::on_pushButton_help_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/rm-core.html"));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RMcore::on_pushButton_calculate_clicked()
{
    int index = ui->comboBox->currentIndex();
    if ((ui->lineEdit_N->text().isEmpty())||(ui->lineEdit_a->text().isEmpty())||(ui->lineEdit_b->text().isEmpty()) ||(ui->lineEdit_c->text().isEmpty())
            ||(ui->lineEdit_e->text().isEmpty())||(ui->lineEdit_d2->text().isEmpty()) ||(ui->lineEdit_d3->text().isEmpty())
            ||(ui->lineEdit_h1->text().isEmpty()) ||(ui->lineEdit_h2->text().isEmpty()) ||(ui->lineEdit_mu->text().isEmpty())){
        showWarning(tr("Warning"), tr("One or more inputs are empty!"));
        return;
    }
    bool ok1, ok2, ok3, ok4, ok5, ok6, ok7, ok8, ok9, ok10, ok11;
    a = loc.toDouble(ui->lineEdit_a->text(), &ok1)*fOpt->dwLengthMultiplier;
    b = loc.toDouble(ui->lineEdit_b->text(), &ok3)*fOpt->dwLengthMultiplier;
    c = loc.toDouble(ui->lineEdit_c->text(), &ok4)*fOpt->dwLengthMultiplier;
    e = loc.toDouble(ui->lineEdit_e->text(), &ok2)*fOpt->dwLengthMultiplier;
    d2 = loc.toDouble(ui->lineEdit_d2->text(), &ok6)*fOpt->dwLengthMultiplier;
    d3 = loc.toDouble(ui->lineEdit_d3->text(), &ok7)*fOpt->dwLengthMultiplier;
    d4 = loc.toDouble(ui->lineEdit_d4->text(), &ok5)*fOpt->dwLengthMultiplier;
    h1 = loc.toDouble(ui->lineEdit_h1->text(), &ok8)*fOpt->dwLengthMultiplier;
    h2 = loc.toDouble(ui->lineEdit_h2->text(), &ok9)*fOpt->dwLengthMultiplier;
    g = loc.toDouble(ui->lineEdit_g->text(), &ok10)*fOpt->dwLengthMultiplier;
    mu = loc.toDouble(ui->lineEdit_mu->text(), &ok11);
    if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok8)||(!ok9)||(!ok10)||(!ok11)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    if (d2 <= d3){
        showWarning(tr("Warning"), "d2 <= d3");
        return;
    }
    if (d3 <= d4){
        showWarning(tr("Warning"), "d3 <= d4");
        return;
    }
    if (h1 <= h2){
        showWarning(tr("Warning"), "h1 <= h2");
        return;
    }
    _CoilResult result;
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
        ind = findRMCore_I (N, a, b, c, e, d2, d3, d4, h1, h2, g, mu, index, &result);
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
        N = findRMCore_N (ind, a, b, c, e, d2, d3, d4, h1, h2, g, mu, index, &result);
    }
    QString sCaption = QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle();
    QString sImage = "";
    switch (index) {
    case 0:
    case 1:
        sImage = "<img src=\":/images/res/rm-core1.png\">";
        break;
    case 2:
        sImage = "<img src=\":/images/res/rm-core2.png\">";
        break;
    case 3:
        sImage = "<img src=\":/images/res/rm-core3.png\">";
        break;
    default:
        break;
    }
    QString sInput = "<p><u>" + tr("Input data") + ":</u><br/>";
    if (ui->checkBox_isReverce->isChecked())
        sInput += formattedOutput(fOpt, ui->label_N->text(), ui->lineEdit_N->text()) + "<br/>";
    else
        sInput += formattedOutput(fOpt, ui->label_N->text(), ui->lineEdit_N->text(), ui->label_N_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_a->text(), ui->lineEdit_a->text(), ui->label_am->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_b->text(), ui->lineEdit_b->text(), ui->label_bm->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_c->text(), ui->lineEdit_c->text(), ui->label_cm->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_e->text(), ui->lineEdit_e->text(), ui->label_em->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_d2->text(), ui->lineEdit_d2->text(), ui->label_d2m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_d3->text(), ui->lineEdit_d3->text(), ui->label_d3m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_d4->text(), ui->lineEdit_d4->text(), ui->label_d4m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_h1->text(), ui->lineEdit_h1->text(), ui->label_h1m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_h2->text(), ui->lineEdit_h2->text(), ui->label_h2m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_g->text(), ui->lineEdit_g->text(), ui->label_gm->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_mu->text(), ui->lineEdit_mu->text()) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
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
    sResult += formattedOutput(fOpt, tr("Effective magnetic permeability of the core") + " μ<sub>e</sub> = ", roundTo(result.thd, loc, 0));
    sResult += "</p>";
    QString n = "";
    if (ui->checkBox_isReverce->isChecked())
        n = ui->lineEdit_N->text();
    else
        n = QString::number(N);
    QString sSatData = n + ";" + loc.toString(result.thd) + ";" + loc.toString(result.N);
    emit sendResult(sCaption + LIST_SEPARATOR + sImage + LIST_SEPARATOR + sInput + LIST_SEPARATOR + sResult + LIST_SEPARATOR + sSatData);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RMcore::on_toolButton_saturation_toggled(bool checked)
{
    emit showSaturation(checked);
}
