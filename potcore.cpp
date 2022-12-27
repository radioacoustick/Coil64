/* potcore.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "potcore.h"
#include "ui_potcore.h"


QString PotCoreStandartSize[] = {
    "7x4,7.09-7.39,5.74-5.74,3-3,1.04-1.13,4.06-4.26,2.79-2.79,4-4,1.52-1.52",
    "9x5,9-9.3,7.5-7.75,3.7-3.9,2-2.2,5.1-5.4,3.6-3.9,1.6-2.4",
    "11x7,10.9-11.3,9-9.4,4.5-4.7,2-2.2,6.3-6.6,4.4-4.7,1.6-2.6",
    "14x8,13.8-14.3,11.6-12,5.8-6,3-3.2,8.2-8.5,5.6-6,2-4.1",
    "18x11,17.6-18.4,14.9-15.4,7.3-7.6,3-3.2,10.4-10.7,7.2-7.6,2-4.4",
    "22x13,21.2-22,17.9-18.5,9.1-9.4,4.4-4.7,13.2-13.6,9.2-9.6,2.5-4.4",
    "26x16,25-26,21.2-22,11.1-11.5,5.4-5.7,15.9-16.3,11-11.4,2.5-4.4",
    "28x23,27.3-28.1,22-22,12.88-12.88,5.46-5.66,22.56-23.16,16.3-16.3,3.81-3.81",
    "30x19,29.5-30.5,25-25.8,13.1-13.5,5.4-5.7,18.6-19,13-13.4,3-5.3",
    "36x22,35-36.2,29.9-30.9,15.6-16.2,5.4-5.7,21.4-22,14.6-15,3.5-5.6",
    "42x29,41.7-43.1,35.6-37,17.1-17.7,5.4-5.7,29.3-29.9,20.3-20.7,4-4"
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PotCore::PotCore(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PotCore)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, MAX_DOUBLE, 380);
    ui->lineEdit_d1->setValidator(dv);
    ui->lineEdit_d2->setValidator(dv);
    ui->lineEdit_d3->setValidator(dv);
    ui->lineEdit_d4->setValidator(dv);
    ui->lineEdit_h1->setValidator(dv);
    ui->lineEdit_h2->setValidator(dv);
    ui->lineEdit_b->setValidator(dv);
    ui->lineEdit_g->setValidator(dv);
    ui->lineEdit_mu->setValidator(dv);
    ui->comboBox->clear();
    ui->comboBox->addItem(tr("Custom"));
    for (unsigned int i = 0; i < sizeof(PotCoreStandartSize)/sizeof(PotCoreStandartSize[0]); i++) {
        QString fsise_str = PotCoreStandartSize[i];
        QStringList f_size_val = fsise_str.split(",");
        ui->comboBox->addItem(f_size_val.at(0));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PotCore::~PotCore()
{
    bool isReverse = ui->checkBox_isReverce->isChecked();
    if (isReverse)
        N = loc.toDouble(ui->lineEdit_N->text());
    else
        ind = loc.toDouble(ui->lineEdit_N->text())*fOpt->dwInductanceMultiplier;
    d1 = loc.toDouble(ui->lineEdit_d1->text())*fOpt->dwLengthMultiplier;
    d2 = loc.toDouble(ui->lineEdit_d2->text())*fOpt->dwLengthMultiplier;
    d3 = loc.toDouble(ui->lineEdit_d3->text())*fOpt->dwLengthMultiplier;
    d4 = loc.toDouble(ui->lineEdit_d4->text())*fOpt->dwLengthMultiplier;
    h1 = loc.toDouble(ui->lineEdit_h1->text())*fOpt->dwLengthMultiplier;
    h2 = loc.toDouble(ui->lineEdit_h2->text())*fOpt->dwLengthMultiplier;
    b = loc.toDouble(ui->lineEdit_b->text())*fOpt->dwLengthMultiplier;
    g = loc.toDouble(ui->lineEdit_g->text())*fOpt->dwLengthMultiplier;
    mu = loc.toDouble(ui->lineEdit_mu->text());

    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "PotCore" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", this->size());
    settings->setValue("isReverse", ui->checkBox_isReverce->isChecked());
    settings->setValue("currStdCore", ui->comboBox->currentIndex());
    settings->setValue("ind", ind);
    settings->setValue("N", N);
    settings->setValue("d1", d1);
    settings->setValue("d2", d2);
    settings->setValue("d3", d3);
    settings->setValue("d4", d4);
    settings->setValue("h1", h1);
    settings->setValue("h2", h2);
    settings->setValue("b", b);
    settings->setValue("g", g);
    settings->setValue("mu", mu);
    settings->endGroup();

    delete settings;
    delete fOpt;
    delete dv;
    delete ui;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PotCore::getOpt(_OptionStruct gOpt)
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
    settings->beginGroup( "PotCore" );
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    bool isReverse = settings->value("isReverse", false).toBool();
    currStdCore = settings->value("currStdCore", 1).toInt();

    N = settings->value("N", 0).toDouble();
    ind = settings->value("ind", 0).toDouble();
    d1 = settings->value("d1", 0).toDouble();
    d2 = settings->value("d2", 0).toDouble();
    d3 = settings->value("d3", 0).toDouble();
    d4 = settings->value("d4", 0).toDouble();
    h1 = settings->value("h1", 0).toDouble();
    h2 = settings->value("h2", 0).toDouble();
    b = settings->value("b", 0).toDouble();
    g = settings->value("g", 0).toDouble();
    mu = settings->value("mu", 100).toDouble();
    settings->endGroup();
    if (isReverse)
        ui->lineEdit_N->setText(roundTo(N, loc, fOpt->dwAccuracy));
    else
        ui->lineEdit_N->setText(roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_d1->setText(roundTo(d1 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_d2->setText(roundTo(d2 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_d3->setText(roundTo(d3 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_d4->setText(roundTo(d4 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_h1->setText(roundTo(h1 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_h2->setText(roundTo(h2 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_b->setText(roundTo(b / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_g->setText(roundTo(g / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_mu->setText(roundTo(mu, loc, fOpt->dwAccuracy));

    ui->label_d1->setText(tr("Outside diameter")+" D1:");
    ui->label_d2->setText(tr("Inside diameter")+" D2:");
    ui->label_d3->setText(tr("Centerpost diameter") + " D3:");
    ui->label_d4->setText(tr("Centerpost hole diameter") + " D4:");
    ui->label_h1->setText(tr("Core height")+" h1:");
    ui->label_h2->setText(tr("Centerpost height")+" h2:");
    ui->label_b->setText(tr("Slot size")+" b:");
    ui->label_g->setText(tr("Centerpost gap")+" g:");
    ui->label_mu->setText(tr("Magnetic permeability")+" µ:");
    ui->comboBox->setCurrentIndex(currStdCore);
    resize(size);
    move(pos);
    ui->checkBox_isReverce->setChecked(isReverse);
    on_checkBox_isReverce_clicked();
    delete settings;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PotCore::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString PotCore::getPotCoreSize(QString sizes)
{
    QStringList size_list = sizes.split("-");
    QString s_size_min = size_list.at(0);
    QString s_size_max = size_list.at(1);
    double dw_size_min = s_size_min.toDouble();
    double dw_size_max = s_size_max.toDouble();
    double dw_size_average = ((dw_size_max + dw_size_min)/2);
    int accuracy = fOpt->indexLengthMultiplier + 1;
    if (accuracy == 4)
        accuracy = 0;
    QString sResult = roundTo(dw_size_average/fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy);
    return sResult;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PotCore::on_pushButton_close_clicked()
{
    this->close();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PotCore::on_checkBox_isReverce_clicked()
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
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PotCore::on_comboBox_currentIndexChanged(int index)
{
    if ((fOpt->mainFontSize > 0) && (index > 0)){
        QString fsise_str = PotCoreStandartSize[index - 1];
        QStringList f_size_val = fsise_str.split(",");
        ui->lineEdit_d1->setText(getPotCoreSize(f_size_val.at(1)));
        ui->lineEdit_d2->setText(getPotCoreSize(f_size_val.at(2)));
        ui->lineEdit_d3->setText(getPotCoreSize(f_size_val.at(3)));
        ui->lineEdit_d4->setText(getPotCoreSize(f_size_val.at(4)));
        ui->lineEdit_h1->setText(getPotCoreSize(f_size_val.at(5)));
        ui->lineEdit_h2->setText(getPotCoreSize(f_size_val.at(6)));
        ui->lineEdit_b->setText(getPotCoreSize(f_size_val.at(7)));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PotCore::on_pushButton_help_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/ferrite-pot-core.html"));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PotCore::on_pushButton_calculate_clicked()
{
    if ((ui->lineEdit_N->text().isEmpty())||(ui->lineEdit_d1->text().isEmpty())||(ui->lineEdit_d2->text().isEmpty())
            ||(ui->lineEdit_d3->text().isEmpty()) ||(ui->lineEdit_d4->text().isEmpty()) ||(ui->lineEdit_h1->text().isEmpty())
            ||(ui->lineEdit_h2->text().isEmpty())){
        showWarning(tr("Warning"), tr("One or more inputs are empty!"));
        return;
    }
    bool ok1, ok2, ok3, ok4, ok5, ok6, ok7, ok8, ok9;
    d1 = loc.toDouble(ui->lineEdit_d1->text(), &ok1)*fOpt->dwLengthMultiplier;
    d2 = loc.toDouble(ui->lineEdit_d2->text(), &ok2)*fOpt->dwLengthMultiplier;
    d3 = loc.toDouble(ui->lineEdit_d3->text(), &ok3)*fOpt->dwLengthMultiplier;
    d4 = loc.toDouble(ui->lineEdit_d4->text(), &ok4)*fOpt->dwLengthMultiplier;
    h1 = loc.toDouble(ui->lineEdit_h1->text(), &ok5)*fOpt->dwLengthMultiplier;
    h2 = loc.toDouble(ui->lineEdit_h2->text(), &ok6)*fOpt->dwLengthMultiplier;
    b = loc.toDouble(ui->lineEdit_b->text(), &ok7)*fOpt->dwLengthMultiplier;
    g = loc.toDouble(ui->lineEdit_g->text(), &ok8)*fOpt->dwLengthMultiplier;
    mu = loc.toDouble(ui->lineEdit_mu->text(), &ok9);
    if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok7)||(!ok8)||(!ok9)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    if ((d1 == 0)||(d2 == 0)||(d3 == 0)||(d4 == 0)||(h1 == 0)||(h2 == 0)||(mu == 0)){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        return;
    }
    if (d1 < d2){
        showWarning(tr("Warning"), "d1 < d2");
        return;
    }
    if (d2 < d3){
        showWarning(tr("Warning"), "d3 < d3");
        return;
    }
    if (d3 < d4){
        showWarning(tr("Warning"), "d3 < d4");
        return;
    }
    if (h1 < h2){
        showWarning(tr("Warning"), "h1 < h2");
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
        ind = findPotCore_I(N, d1, d2, d3, d4, h1, h2, g, b, mu, &result);
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
        N = findPotCore_N(ind, d1, d2, d3, d4, h1, h2, g, b, mu, &result);
    }
    QString sResult = "<hr>";
    if (fOpt->isShowTitle){
        sResult = "<h2>" +QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() +
                " - " + windowTitle() + "</h2><br/>";
    }
    if (fOpt->isInsertImage)
        sResult += "<img src=\":/images/res/Coil7.png\">";
    sResult += "<p><u>" + tr("Input data") + ":</u><br/>";
    if (ui->checkBox_isReverce->isChecked())
        sResult += ui->label_N->text() + " " + ui->lineEdit_N->text() + "<br/>";
    else
        sResult += ui->label_N->text() + " " + ui->lineEdit_N->text() + " " + ui->label_N_m->text() + "<br/>";
    sResult += ui->label_d1->text() + " " + ui->lineEdit_d1->text() + " " + ui->label_01->text() + "<br/>";
    sResult += ui->label_d2->text() + " " + ui->lineEdit_d2->text() + " " + ui->label_02->text() + "<br/>";
    sResult += ui->label_d3->text() + " " + ui->lineEdit_d3->text() + " " + ui->label_03->text() + "<br/>";
    sResult += ui->label_d4->text() + " " + ui->lineEdit_d4->text() + " " + ui->label_04->text() + "<br/>";
    sResult += ui->label_h1->text() + " " + ui->lineEdit_h1->text() + " " + ui->label_05->text() + "<br/>";
    sResult += ui->label_h2->text() + " " + ui->lineEdit_h2->text() + " " + ui->label_06->text() + "<br/>";
    sResult += ui->label_b->text() + " " + ui->lineEdit_b->text() + " " + ui->label_07->text() + "<br/>";
    sResult += ui->label_g->text() + " " + ui->lineEdit_g->text() + " " + ui->label_08->text() + "<br/>";
    sResult += ui->label_mu->text() + " " + ui->lineEdit_mu->text() + "<br/>";
    sResult += "<hr>";
    sResult += "<p><u>" + tr("Result") + ":</u><br/>";
    if (ui->checkBox_isReverce->isChecked()){
        sResult += tr("Inductance") + " L = " + roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy) + " "
                + qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8());
    } else {
        sResult += tr("Number of turns of the coil") + " N = " + QString::number(N);
    }
    sResult += "<br/><br/>" + tr("Effective magnetic path length") + " (l<sub>e</sub>): "
            + roundTo(result.N/fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy)
            + "&nbsp;" + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    sResult += tr("Effective area of magnetic path") + " (A<sub>e</sub>): "
            + roundTo(result.sec/(fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier), loc, fOpt->dwAccuracy)
            + "&nbsp;" + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<sup>2</sup><br/>";
    sResult += tr("Effective volume") + " (V<sub>e</sub>): "
            + roundTo(result.N * result.sec/(fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier * fOpt->dwLengthMultiplier), loc, fOpt->dwAccuracy)
            + "&nbsp;" + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "<sup>3</sup><br/>";
    sResult += tr("Effective magnetic permeability of the core") + " μ<sub>e</sub> = " + roundTo(result.thd, loc, 0);
    sResult += "</p><hr>";
    emit sendResult(sResult);
}
