/* bandspread.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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
#include "bandspread.h"
#include "ui_bandspread.h"

Bandspread::Bandspread(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Bandspread)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, MAX_DOUBLE, 380);
    ui->lineEdit_ind->setValidator(dv);
    ui->lineEdit_flo->setValidator(dv);
    ui->lineEdit_fhi->setValidator(dv);
    ui->lineEdit_cvmin->setValidator(dv);
    ui->lineEdit_cvmax->setValidator(dv);
    ui->lineEdit_cs->setValidator(dv);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Bandspread::~Bandspread()
{
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Bandspread" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", size());
    settings->setValue("ind", ind);
    settings->setValue("f_low", f_low);
    settings->setValue("f_high", f_high);
    settings->setValue("Cv_min", Cv_min);
    settings->setValue("Cv_max", Cv_max);
    settings->setValue("Cs", Cs);
    settings->endGroup();
    delete settings;
    delete fOpt;
    delete dv;
    delete ui;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::getOpt(_OptionStruct gOpt)
{
    *fOpt = gOpt;
    ui->label_flo_m->setText(qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
    ui->label_fhi_m->setText(qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
    ui->label_cvmin_m->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_cvmax_m->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_cs_m->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_ind_m->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_ct_m->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_cp_m->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_flo_m_r->setText(qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
    ui->label_fhi_m_r->setText(qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
    ui->label_cvmin_m_r->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_cvmax_m_r->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_cs_m_r->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_ind_m_r->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_ct_m_r->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_cp_m_r->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_flo->setText(tr("Lowest Frequency") + " F<sub>low</sub>:");
    ui->label_fhi->setText(tr("Highest Frequency") + " F<sub>high</sub>:");
    ui->label_cvmin->setText(tr("Tuning Capacitor Minimum") + " C<sub>V</sub>_min:");
    ui->label_cvmax->setText(tr("Tuning Capacitor Maximum") + " C<sub>V</sub>_max:");
    ui->label_cs->setText(tr("Stray Capacitance") + " C<sub>s</sub>:");
    ui->label_ind->setText(tr("Inductance") + " L:");
    ui->label_ct->setText(tr("Parallel Capacitor") + " C1:");
    ui->label_cp->setText(tr("Serial Capacitor") + " C2:");
    ui->label_flo_r->setText(tr("Lowest Frequency") + " F<sub>low</sub>:");
    ui->label_fhi_r->setText(tr("Highest Frequency") + " F<sub>high</sub>:");
    ui->label_cvmin_r->setText(tr("Tuning Capacitor Minimum") + " C<sub>V</sub>_min:");
    ui->label_cvmax_r->setText(tr("Tuning Capacitor Maximum") + " C<sub>V</sub>_max:");
    ui->label_cs_r->setText(tr("Stray Capacitance") + " C<sub>s</sub>:");
    ui->label_ind_r->setText(tr("Inductance") + " L:");
    ui->label_ct_r->setText(tr("Parallel Capacitor") + " C1:");
    ui->label_cp_r->setText(tr("Serial Capacitor") + " C2:");
    ui->label_hint->setText(tr("Valid value range") + " :");
    ui->label_info->setText("");
    ui->label_info_r->setText("");
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Bandspread" );
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    ind = settings->value("ind", 0).toDouble();
    f_low = settings->value("f_low", 0).toDouble();
    f_high = settings->value("f_high", 0).toDouble();
    Cv_min = settings->value("Cv_min", 0).toDouble();
    Cv_max = settings->value("Cv_max", 0).toDouble();
    Cs = settings->value("Cs", 0).toDouble();
    settings->endGroup();
    delete settings;
    ui->tabWidget->setCurrentIndex(0);
    ui->lineEdit_ind->setText(roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_flo->setText(roundTo(f_low / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_fhi->setText(roundTo(f_high / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_cvmin->setText(roundTo(Cv_min / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_cvmax->setText(roundTo(Cv_max / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_cs->setText(roundTo(Cs / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
    resize(size);
    move(pos);
    showInductanceRange();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void calcInductanceRange(double f_low, double f_high, double Cv_low, double Cv_high, double Cstray, QVector<double> *range, int accuracy) {
    int appr = (int) pow(10, accuracy);
    double ratioFreq = f_high / f_low;
    double ratioCap = ratioFreq * ratioFreq;
    double cpmin = findPadderCapacitance(0, Cv_low, Cv_high, Cstray, ratioCap);
    double ctmax = findTrimmerCapacitance(0,Cv_low, Cv_high, Cstray, ratioCap);
    double w = 1e3/(2 * M_PI * f_low);
    double chpppm = Cv_high * cpmin / (Cv_high + cpmin) + Cstray;
    double Lmin = w * w / chpppm;
    double chppp = Cv_high + Cstray + ctmax;
    double Lmax = w * w / chppp;
    range->clear();
    if ((Lmin > 0) && (Lmax > 0) && (ctmax > 0) && (cpmin > 0)){
        range->push_front(round(Lmin * appr) / appr);
        range->push_front(round(Lmax * appr) / appr);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void calcCapacitors(double f_low, double f_high, double Cv_low, double Cv_high, double Cstray, double ind, QVector<double> *cap, int accuracy){
    int appr = (int) pow(10, accuracy);
    double w = 1e3/(2 * M_PI * f_low);
    double chppp = w * w / ind;
    w = 1e3/(2 * M_PI * f_high);
    double clppp = w * w / ind;
    double alpha = Cstray - chppp;
    double beta = Cstray - clppp;
    double k1 = Cv_high - chppp - Cv_low + clppp;
    double k2 = alpha - beta;
    double k3 = alpha * Cv_high - beta * Cv_low;
    double k4 = alpha * Cv_high;
    double k5 = Cv_high + alpha;
    double a = -k2 / k1;
    double b = alpha - (k3 + k2 * k5) / k1;
    double c = k4 - (k3 * k5) / k1;
    double Ct = (-b + sqrt(b * b - 4 * a * c)) / (2.0 * a);
    double Cp = -(k2 * Ct + k3) / k1;
    double netCmin = CalcLC2(ind, f_high);
    double netCmax = CalcLC2(ind, f_low);
    cap->clear();
    if ((std::isnormal(Ct)) && (std::isnormal(Cp)) && (Ct > 0) && (Cp > 0)){
        cap->push_front(round(netCmax * appr) / appr);
        cap->push_front(round(netCmin * appr) / appr);
        cap->push_front(round(Cp * appr) / appr);
        cap->push_front(round(Ct * appr) / appr);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void calcFrequencyRange(double Ct, double Cp, double Cv_low, double Cv_high, double Cstray, double ind, QVector<double> *freq, int accuracy){
    int appr = (int) pow(10, accuracy);
    double chppp = (Cp * (Cv_high + Ct) / (Cv_high + Ct + Cp)) + Cstray;
    double clppp = (Cp * (Cv_low + Ct) / (Cv_low + Ct + Cp)) + Cstray;
    double low_freq = CalcLC0(ind, chppp);
    double high_freq = CalcLC0(ind, clppp);
    double netCmin = CalcLC2(ind, high_freq);
    double netCmax = CalcLC2(ind, low_freq);
    freq->clear();
    if ((low_freq > 0) && (high_freq > 0)){
        freq->push_front(round(netCmax * appr) / appr);
        freq->push_front(round(netCmin * appr) / appr);
        freq->push_front(round(high_freq * appr) / appr);
        freq->push_front(round(low_freq * appr) / appr);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::showInductanceRange()
{
    if ((f_low > 0) && (f_high > 0) && (Cv_min > 0) && (Cv_max > 0)){
        QVector<double> *range = new QVector<double>();
        calcInductanceRange(f_low, f_high, Cv_min, Cv_max, Cs, range, fOpt->dwAccuracy);
        if (!range->isEmpty()){
            ui->label_hint->setText(tr("Valid value range") + ": " + QString::number(range->at(0)/fOpt->dwInductanceMultiplier) + " ... "
                                    + QString::number(range->at(1)/fOpt->dwInductanceMultiplier)
                                    + " [" + qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()) + "]");
        } else {
            ui->label_hint->setText(tr("Valid value range") + ":");
        }
        delete range;
    } else {
        ui->label_hint->setText(tr("Valid value range") + ":");
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_pushButton_calculate_clicked()
{
    switch (ui->tabWidget->currentIndex()){
    case 0:{
        bool ok1, ok2, ok3, ok4, ok5, ok6;
        f_low = loc.toDouble(ui->lineEdit_flo->text(), &ok1)*fOpt->dwFrequencyMultiplier;
        f_high = loc.toDouble(ui->lineEdit_fhi->text(), &ok2)*fOpt->dwFrequencyMultiplier;
        Cv_min = loc.toDouble(ui->lineEdit_cvmin->text(), &ok3)*fOpt->dwCapacityMultiplier;
        Cv_max = loc.toDouble(ui->lineEdit_cvmax->text(), &ok4)*fOpt->dwCapacityMultiplier;
        Cs = loc.toDouble(ui->lineEdit_cs->text(), &ok5)*fOpt->dwCapacityMultiplier;
        ind = loc.toDouble(ui->lineEdit_ind->text(), &ok6)*fOpt->dwInductanceMultiplier;
        if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)){
            ui->lineEdit_ct->setText("");
            ui->lineEdit_cp->setText("");
            ui->label_info->setText("");
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if ((f_low == 0)||(f_high == 0)||(Cv_min == 0)||(Cv_max == 0)||(ind == 0)){
            ui->lineEdit_ct->setText("");
            ui->lineEdit_cp->setText("");
            ui->label_info->setText("");
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        if (f_low >= f_high){
            ui->lineEdit_ct->setText("");
            ui->lineEdit_cp->setText("");
            ui->label_info->setText("");
            showWarning(tr("Warning"), "F<sub>high</sub> &lt;= F<sub>low</sub>");
            return;
        }
        if (Cv_min >= Cv_max){
            ui->lineEdit_ct->setText("");
            ui->lineEdit_cp->setText("");
            ui->label_info->setText("");
            showWarning(tr("Warning"), "C<sub>V</sub>_max &lt;= C<sub>V</sub>_min");
            return;
        }
        showInductanceRange();
        QVector<double> *range = new QVector<double>();
        calcInductanceRange(f_low, f_high, Cv_min, Cv_max, Cs, range, fOpt->dwAccuracy);
        double Lmin = 0.0;
        double Lmax = MAX_DOUBLE;
        if (!range->isEmpty()){
            Lmin = range->at(0);
            Lmax = range->at(1);
        }
        if ((ind >= Lmin) && (ind <= Lmax)){
            QVector<double> *cap = new QVector<double>();
            calcCapacitors(f_low, f_high, Cv_min, Cv_max, Cs, ind, cap, fOpt->dwAccuracy);
            if ((!cap->isEmpty()) && (!range->isEmpty())){
                Ct = cap->at(0);
                Cp = cap->at(1);
                ui->lineEdit_ct->setText(QString::number(Ct/fOpt->dwCapacityMultiplier));
                ui->lineEdit_cp->setText(QString::number(Cp/fOpt->dwCapacityMultiplier));
                ui->label_info->setText(tr("Minimum LC circuit Capacitance") + ": " + QString::number(cap->at(2)/fOpt->dwCapacityMultiplier) + " "
                                        + qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>" + tr("Maximum LC circuit Capacitance") + ": "
                                        + QString::number(cap->at(3)/fOpt->dwCapacityMultiplier) + " " + qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
            } else {
                ui->lineEdit_ct->setText("");
                ui->lineEdit_cp->setText("");
                ui->label_info->setText("");
                showWarning(tr("Warning"), tr("The C<sub>V</sub> capacitance range (combined with the circuit stray capacitance) is too narrow to overlap the desired frequency range. "
                                              "Please change your frequency range or variable capacitor parameters and try again."));
            }
            delete cap;
        } else {
            ui->lineEdit_ct->setText("");
            ui->lineEdit_cp->setText("");
            ui->label_info->setText("");
            showWarning(tr("Warning"), tr("The inductance value is out of range."));
        }
        delete range;

    }
        break;
    case 1:{
        bool ok1, ok2, ok3, ok4, ok5, ok6;
        Ct = loc.toDouble(ui->lineEdit_ct_r->text(), &ok1)*fOpt->dwCapacityMultiplier;
        Cp = loc.toDouble(ui->lineEdit_cp_r->text(), &ok2)*fOpt->dwCapacityMultiplier;
        Cv_min = loc.toDouble(ui->lineEdit_cvmin_r->text(), &ok3)*fOpt->dwCapacityMultiplier;
        Cv_max = loc.toDouble(ui->lineEdit_cvmax_r->text(), &ok4)*fOpt->dwCapacityMultiplier;
        Cs = loc.toDouble(ui->lineEdit_cs_r->text(), &ok5)*fOpt->dwCapacityMultiplier;
        ind = loc.toDouble(ui->lineEdit_ind_r->text(), &ok6)*fOpt->dwInductanceMultiplier;
        if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)){
            ui->lineEdit_flo_r->setText("");
            ui->lineEdit_fhi_r->setText("");
            ui->label_info_r->setText("");
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if ((Ct == 0)||(Cp == 0)||(Cv_min == 0)||(Cv_max == 0)||(ind == 0)){
            ui->lineEdit_flo_r->setText("");
            ui->lineEdit_fhi_r->setText("");
            ui->label_info_r->setText("");
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        if (Cv_min >= Cv_max){
            ui->lineEdit_flo_r->setText("");
            ui->lineEdit_fhi_r->setText("");
            ui->label_info_r->setText("");
            showWarning(tr("Warning"), "C<sub>V</sub>_max &lt;= C<sub>V</sub>_min");
            return;
        }
        QVector<double> *freq = new QVector<double>();
        calcFrequencyRange(Ct, Cp, Cv_min, Cv_max, Cs, ind, freq, fOpt->dwAccuracy);
        if(!freq->isEmpty()){
            f_low = freq->at(0);
            f_high = freq->at(1);
            ui->lineEdit_flo_r->setText(QString::number(f_low/fOpt->dwFrequencyMultiplier));
            ui->lineEdit_fhi_r->setText(QString::number(f_high/fOpt->dwFrequencyMultiplier));
            ui->label_info_r->setText(tr("Minimum LC circuit Capacitance") + ": " + QString::number(freq->at(2)/fOpt->dwCapacityMultiplier) + " "
                                    + qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>" + tr("Maximum LC circuit Capacitance") + ": "
                                    + QString::number(freq->at(3)/fOpt->dwCapacityMultiplier) + " " + qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
        }
        delete freq;
    }
        break;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_pushButton_export_clicked()
{
    QString sResult = "<hr>";
    if (fOpt->isShowTitle){
        sResult = "<h2>" +QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - "
                + windowTitle() + "</h2><br/>";
    }
    if (fOpt->isInsertImage){
        sResult += "<img src=\":/images/res/bandspread.png\">";
    }
    sResult += "<p>" + ui->tabWidget->tabText(ui->tabWidget->currentIndex()) + "</p>";
    sResult += "<p><u>" + tr("Input data") + ":</u><br/>";
    switch (ui->tabWidget->currentIndex()){
    case 0:{
        if ((!ui->lineEdit_ct->text().isEmpty()) && (!ui->lineEdit_cp->text().isEmpty())){
            sResult += ui->label_flo->text() + " " + ui->lineEdit_flo->text() + " " + ui->label_flo_m->text() + "<br/>";
            sResult += ui->label_fhi->text() + " " + ui->lineEdit_fhi->text() + " " + ui->label_fhi_m->text() + "<br/>";
            sResult += ui->label_cvmin->text() + " " + ui->lineEdit_cvmin->text() + " " + ui->label_cvmin_m->text() + "<br/>";
            sResult += ui->label_cvmax->text() + " " + ui->lineEdit_cvmax->text() + " " + ui->label_cvmax_m->text() + "<br/>";
            sResult += ui->label_cs->text() + " " + ui->lineEdit_cs->text() + " " + ui->label_cs_m->text() + "<br/>";
            sResult += ui->label_ind->text() + " " + ui->lineEdit_ind->text() + " " + ui->label_ind_m->text() + "</p>";
            sResult += "<hr>";
            sResult += "<p><u>" + tr("Result") + ":</u><br/>";
            sResult += ui->label_ct->text() + " " + ui->lineEdit_ct->text() + " " + ui->label_ct_m->text() + "<br/>";
            sResult += ui->label_cp->text() + " " + ui->lineEdit_cp->text() + " " + ui->label_cp_m->text() + "</p>";
            sResult += ui->label_info->text();
            sResult += "<hr>";
            emit sendResult(sResult);
        }
    }
        break;
    case 1:{
        if ((!ui->lineEdit_flo_r->text().isEmpty()) && (!ui->lineEdit_fhi_r->text().isEmpty())){
            sResult += ui->label_ct_r->text() + " " + ui->lineEdit_ct_r->text() + " " + ui->label_ct_m_r->text() + "<br/>";
            sResult += ui->label_cp_r->text() + " " + ui->lineEdit_cp_r->text() + " " + ui->label_cp_m_r->text() + "<br/>";
            sResult += ui->label_cvmin_r->text() + " " + ui->lineEdit_cvmin_r->text() + " " + ui->label_cvmin_m_r->text() + "<br/>";
            sResult += ui->label_cvmax_r->text() + " " + ui->lineEdit_cvmax_r->text() + " " + ui->label_cvmax_m_r->text() + "<br/>";
            sResult += ui->label_cs_r->text() + " " + ui->lineEdit_cs_r->text() + " " + ui->label_cs_m_r->text() + "<br/>";
            sResult += ui->label_ind_r->text() + " " + ui->lineEdit_ind_r->text() + " " + ui->label_ind_m_r->text() + "</p>";
            sResult += "<hr>";
            sResult += "<p><u>" + tr("Result") + ":</u><br/>";
            sResult += ui->label_flo_r->text() + " " + ui->lineEdit_flo_r->text() + " " + ui->label_flo_m_r->text() + "<br/>";
            sResult += ui->label_fhi_r->text() + " " + ui->lineEdit_fhi_r->text() + " " + ui->label_fhi_m_r->text() + "</p>";
            sResult += ui->label_info_r->text();
            sResult += "<hr>";
            emit sendResult(sResult);
        }
    }
        break;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_tabWidget_currentChanged(int index)
{
    if (index == 0){
        ui->lineEdit_cvmin->setText(ui->lineEdit_cvmin_r->text());
        ui->lineEdit_cvmax->setText(ui->lineEdit_cvmax_r->text());
        ui->lineEdit_cs->setText(ui->lineEdit_cs_r->text());
        ui->lineEdit_ind->setText(ui->lineEdit_ind_r->text());
        if ((!ui->lineEdit_flo_r->text().isEmpty()) && (!ui->lineEdit_fhi_r->text().isEmpty())){
            ui->lineEdit_flo->setText(roundTo(f_low / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
            ui->lineEdit_fhi->setText(roundTo(f_high / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
        }
    } else {
        ui->lineEdit_cvmin_r->setText(ui->lineEdit_cvmin->text());
        ui->lineEdit_cvmax_r->setText(ui->lineEdit_cvmax->text());
        ui->lineEdit_cs_r->setText(ui->lineEdit_cs->text());
        ui->lineEdit_ind_r->setText(ui->lineEdit_ind->text());

        QString ssCt = ui->lineEdit_ct->text();
        double dCt = ssCt.toDouble();
        ui->lineEdit_ct_r->setText(loc.toString(toNearestE24(dCt, fOpt->dwAccuracy)));

        QString ssCp = ui->lineEdit_cp->text();
        double dCp = ssCp.toDouble();
        ui->lineEdit_cp_r->setText(loc.toString(toNearestE24(dCp, fOpt->dwAccuracy)));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_lineEdit_ind_editingFinished()
{
    ind = loc.toDouble(ui->lineEdit_ind->text())*fOpt->dwInductanceMultiplier;
    ui->lineEdit_ind->clearFocus();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_lineEdit_flo_editingFinished()
{
    f_low = loc.toDouble(ui->lineEdit_flo->text())*fOpt->dwFrequencyMultiplier;
    ui->lineEdit_flo->clearFocus();
    showInductanceRange();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_lineEdit_fhi_editingFinished()
{
    f_high = loc.toDouble(ui->lineEdit_fhi->text())*fOpt->dwFrequencyMultiplier;
    ui->lineEdit_fhi->clearFocus();
    showInductanceRange();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_lineEdit_cvmin_editingFinished()
{
    Cv_min = loc.toDouble(ui->lineEdit_cvmin->text())*fOpt->dwCapacityMultiplier;
    ui->lineEdit_cvmin->clearFocus();
    showInductanceRange();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_lineEdit_cvmax_editingFinished()
{
    Cv_max = loc.toDouble(ui->lineEdit_cvmax->text())*fOpt->dwCapacityMultiplier;
    ui->lineEdit_cvmax->clearFocus();
    showInductanceRange();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_lineEdit_cs_editingFinished()
{
    Cs = loc.toDouble(ui->lineEdit_cs->text())*fOpt->dwCapacityMultiplier;
    ui->lineEdit_cs->clearFocus();
    showInductanceRange();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_pushButton_close_clicked()
{
    this->close();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_pushButton_help_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/bandspread.html"));
}
