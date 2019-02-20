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
    QString tmp_txt = tr("Rod diameter") + " Dr:";
    ui->label_Dr->setText(tmp_txt);
    tmp_txt = tr("Rod length") + " Lr:";
    ui->label_Lr->setText(tmp_txt);
    tmp_txt = tr("Magnetic permeability") + " µ:";
    ui->label_mu->setText(tmp_txt);
    tmp_txt = tr("Former diameter") + "  dc:";
    ui->label_dc->setText(tmp_txt);

    tmp_txt = tr("Shift") + "  s:";
    ui->label_s->setText(tmp_txt);
    tmp_txt = tr("Wire diameter") + " dw:";
    ui->label_dw->setText(tmp_txt);
    tmp_txt = tr("Winding pitch") + " p:";
    ui->label_p->setText(tmp_txt);
}

Ferrite_Rod::~Ferrite_Rod()
{
    double I = ui->lineEdit_ind->text().toDouble()*fOpt->dwInductanceMultiplier;
    double Dr = ui->lineEdit_Dr->text().toDouble()*fOpt->dwLengthMultiplier;
    double Lr = ui->lineEdit_Lr->text().toDouble()*fOpt->dwLengthMultiplier;
    double mu = ui->lineEdit_mu->text().toDouble();
    double dc = ui->lineEdit_dc->text().toDouble()*fOpt->dwLengthMultiplier;
    double s = ui->lineEdit_s->text().toDouble()*fOpt->dwLengthMultiplier;
    double dw = ui->lineEdit_dw->text().toDouble()*fOpt->dwLengthMultiplier;
    double p = ui->lineEdit_p->text().toDouble()*fOpt->dwLengthMultiplier;
#if defined(Q_OS_MAC) || (Q_WS_X11) || defined(Q_OS_LINUX)
    QSettings *settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#elif defined(Q_WS_WIN) || defined(Q_OS_WIN)
    QSettings *settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#else
    QSettings *settings = new QSettings(QDir::currentPath() + "/Coil64.conf", QSettings::IniFormat);
#endif
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
    delete ui;
}

void Ferrite_Rod::getOpt(_OptionStruct gOpt){
    *fOpt = gOpt;
    ui->label_ind_m->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_Dr_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_Lr_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_dc_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_s_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_dw_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_p_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    QFont f1 = this->font();
    f1.setFamily(fOpt->mainFontFamily);
    f1.setPixelSize(fOpt->mainFontSize);
    this->setFont(f1);
#if defined(Q_OS_MAC) || (Q_WS_X11) || defined(Q_OS_LINUX)
    QSettings *settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#elif defined(Q_WS_WIN) || defined(Q_OS_WIN)
    QSettings *settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#else
    QSettings *settings = new QSettings(QDir::currentPath() + "/Coil64.conf", QSettings::IniFormat);
#endif
    settings->beginGroup( "FerriteRod" );
    double I = settings->value("L", 0).toDouble();
    double Dr = settings->value("Dr", 0).toDouble();
    double mu = settings->value("mu", 0).toDouble();
    double Lr = settings->value("Lr", 0).toDouble();
    double dc = settings->value("dc", 0).toDouble();
    double s = settings->value("s", 0).toDouble();
    double dw = settings->value("dw", 0).toDouble();
    double p = settings->value("p", 0).toDouble();
    QPoint pos = settings->value("pos", QPoint(300, 300)).toPoint();
    QSize size = settings->value("size", QSize(100, 100)).toSize();
    settings->endGroup();
    ui->lineEdit_ind->setText(QString::number(I / fOpt->dwInductanceMultiplier));
    ui->lineEdit_Dr->setText(QString::number(Dr / fOpt->dwLengthMultiplier));
    ui->lineEdit_Lr->setText(QString::number(Lr / fOpt->dwLengthMultiplier));
    ui->lineEdit_mu->setText(QString::number(mu));
    ui->lineEdit_dc->setText(QString::number(dc / fOpt->dwLengthMultiplier));
    ui->lineEdit_s->setText(QString::number(s / fOpt->dwLengthMultiplier));
    ui->lineEdit_dw->setText(QString::number(dw / fOpt->dwLengthMultiplier));
    ui->lineEdit_p->setText(QString::number(p / fOpt->dwLengthMultiplier));
    ui->lineEdit_ind->setFocus();
    ui->lineEdit_ind->selectAll();
    resize(size);
    move(pos);
    delete settings;
}

void Ferrite_Rod::on_pushButton_close_clicked()
{
    this->close();
}

void Ferrite_Rod::on_pushButton_calculate_clicked()
{
    if ((ui->lineEdit_ind->text().isEmpty())||(ui->lineEdit_Dr->text().isEmpty())||(ui->lineEdit_Lr->text().isEmpty())||(ui->lineEdit_mu->text().isEmpty())
            ||(ui->lineEdit_dc->text().isEmpty())||(ui->lineEdit_s->text().isEmpty())||(ui->lineEdit_dw->text().isEmpty())||(ui->lineEdit_p->text().isEmpty())){
        showWarning(tr("Warning"), tr("One or more inputs are empty!"));
        return;
    }
    double I = ui->lineEdit_ind->text().toDouble()*fOpt->dwInductanceMultiplier;
    double Dr = ui->lineEdit_Dr->text().toDouble()*fOpt->dwLengthMultiplier;
    double Lr = ui->lineEdit_Lr->text().toDouble()*fOpt->dwLengthMultiplier;
    double mu = ui->lineEdit_mu->text().toDouble();
    double dc = ui->lineEdit_dc->text().toDouble()*fOpt->dwLengthMultiplier;
    double s = ui->lineEdit_s->text().toDouble()*fOpt->dwLengthMultiplier;
    double dw = ui->lineEdit_dw->text().toDouble()*fOpt->dwLengthMultiplier;
    double p = ui->lineEdit_p->text().toDouble()*fOpt->dwLengthMultiplier;
    if ((I == 0)||(Dr == 0)||(Lr == 0)||(mu == 0)||(dc == 0)||(dw == 0)||(p == 0)){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
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
    QString sResult = "<hr><h2>" +QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle() + "</h2><br/>";
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
    sResult += tr("Number of turns of the coil") + " N = " + QString::number(result.N, 'f', fOpt->dwAccuracy) + "<br/>";
    sResult += tr("Length of winding") + " lc = " + QString::number(result.thd, 'f', fOpt->dwAccuracy) + "<br/>";
    sResult += tr("Effective magnetic permeability of the rod") + " μ<sub>eff</sub> = " + QString::number(result.sec, 'f', 0);
    sResult += "</p><hr>";
    emit sendResult(sResult);
}
