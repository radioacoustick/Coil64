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
    dv = new QDoubleValidator(0.0, DBL_MAX, 380);
    ui->lineEdit_ind->setValidator(dv);
    ui->lineEdit_flo->setValidator(dv);
    ui->lineEdit_fhi->setValidator(dv);
    ui->lineEdit_cvmin->setValidator(dv);
    ui->lineEdit_cvmax->setValidator(dv);
    ui->lineEdit_cs->setValidator(dv);
    ui->lineEdit_ind_r->setValidator(dv);
    ui->lineEdit_flo_r->setValidator(dv);
    ui->lineEdit_fhi_r->setValidator(dv);
    ui->lineEdit_cvmin_r->setValidator(dv);
    ui->lineEdit_cvmax_r->setValidator(dv);
    ui->lineEdit_cs_r->setValidator(dv);
    ui->lineEdit_flo_t->setValidator(dv);
    ui->lineEdit_fhi_t->setValidator(dv);
    ui->lineEdit_cvmin_t->setValidator(dv);
    ui->lineEdit_cvmax_t->setValidator(dv);
    ui->lineEdit_cs_t->setValidator(dv);
    ui->lineEdit_IF->setValidator(dv);
    ui->lineEdit_Le->setValidator(dv);
    QAction *buttonAction = new QAction(ui->pushButton_export);
    buttonAction->setShortcuts({QKeySequence("Ctrl+Enter"),QKeySequence("Ctrl+Return")});
    ui->pushButton_export->addAction(buttonAction);
    connect(buttonAction, &QAction::triggered, ui->pushButton_export, &QPushButton::click);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Bandspread::~Bandspread()
{
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Bandspread" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", size());
    settings->setValue("f_low", f_low);
    settings->setValue("f_high", f_high);
    settings->setValue("f_i", f_i);
    settings->setValue("Cv_min", Cv_min);
    settings->setValue("Cv_max", Cv_max);
    settings->setValue("Cs", Cs);
    settings->setValue("localOscillatorInjection", ui->comboBox->currentIndex());
    settings->setValue("L_value_pos", L_value_pos);
    settings->setValue("isAutomaticLe", ui->checkBox->isChecked());
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
    ui->label_flo_m_t->setText(qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
    ui->label_fhi_m_t->setText(qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
    ui->label_IF_m->setText(qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
    ui->label_cvmin_m_t->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_cvmax_m_t->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_cs_m_t->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_ind_m_t->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_flo->setText(tr("Lowest Frequency") + " F_low:");
    ui->label_fhi->setText(tr("Highest Frequency") + " F_high:");
    ui->label_cvmin->setText(tr("Tuning Capacitor Minimum") + " Cv_min:");
    ui->label_cvmax->setText(tr("Tuning Capacitor Maximum") + " Cv_max:");
    ui->label_cs->setText(tr("Stray Capacitance") + " Cs:");
    ui->label_ind->setText(tr("Inductance") + " L:");
    ui->label_ct->setText(tr("Parallel Capacitor") + " C1:");
    ui->label_cp->setText(tr("Serial Capacitor") + " C2:");
    ui->label_flo_r->setText(tr("Lowest Frequency") + " F_low:");
    ui->label_fhi_r->setText(tr("Highest Frequency") + " F_high:");
    ui->label_cvmin_r->setText(tr("Tuning Capacitor Minimum") + " Cv_min:");
    ui->label_cvmax_r->setText(tr("Tuning Capacitor Maximum") + " Cv_max:");
    ui->label_cs_r->setText(tr("Stray Capacitance") + " Cs:");
    ui->label_ind_r->setText(tr("Inductance") + " L:");
    ui->label_ct_r->setText(tr("Parallel Capacitor") + " C1:");
    ui->label_cp_r->setText(tr("Serial Capacitor") + " C2:");
    ui->groupBox_L->setTitle(tr("Inductance") + " L [" + qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())
                              + "] (" + tr("Valid value range") + ")");
    ui->groupBox_Le->setTitle(tr("Inductance") + " Le [" + qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())
                              + "] (" + tr("Valid value range") + ")");
    ui->label_L_min->setText("");
    ui->label_L_max->setText("");
    ui->label_Le_min->setText("");
    ui->label_Le_max->setText("");
    ui->label_info->setText("");
    ui->label_info_r->setText("");
    ui->checkBox->setText(tr("Automatically find the optimal") + " Le");
    ui->label_flo_t->setText(tr("Lowest Frequency") + " RF_low:");
    ui->label_fhi_t->setText(tr("Highest Frequency") + " RF_high:");
    ui->label_cvmin_t->setText(tr("Tuning Capacitor Minimum") + " Cv_min:");
    ui->label_cvmax_t->setText(tr("Tuning Capacitor Maximum") + " Cv_max:");
    ui->label_cs_t->setText(tr("Stray Capacitance") + " Cs:");
    ui->label_IF->setText(tr("Intermediate frequency") + " IF:");
    ui->label_ind_t->setText(tr("Inductance") + " Le:");
    ui->tabWidget->setCurrentIndex(0);
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Bandspread" );
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    f_low = settings->value("f_low", 0).toDouble();
    f_high = settings->value("f_high", 0).toDouble();
    f_i = settings->value("f_i", 0).toDouble();
    Cv_min = settings->value("Cv_min", 0).toDouble();
    Cv_max = settings->value("Cv_max", 0).toDouble();
    Cs = settings->value("Cs", 0).toDouble();
    L_value_pos = settings->value("L_value_pos", 0).toInt();
    bool isAutomaticLe = settings->value("isAutomaticLe", false).toBool();
    int localOscillatorInjection = settings->value("localOscillatorInjection", 0).toInt();
    settings->endGroup();
    delete settings;
    resize(size);
    move(pos);
    ui->checkBox->setChecked(isAutomaticLe);
    ui->comboBox->setCurrentIndex(localOscillatorInjection);
    on_tabWidget_currentChanged(0);
    ui->lineEdit_flo->setFocus();
    ui->lineEdit_flo->selectAll();
    if (fOpt->styleGUI == _DarkStyle){
        ui->pushButton_calculate->setIcon(reverseIconColors(ui->pushButton_calculate->icon()));
        ui->pushButton_close->setIcon(reverseIconColors(ui->pushButton_close->icon()));
        ui->pushButton_help->setIcon(reverseIconColors(ui->pushButton_help->icon()));
        ui->pushButton_export->setIcon(reverseIconColors(ui->pushButton_export->icon()));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_tabWidget_currentChanged(int index)
{
    switch (index) {
    case 0:
    {
        ui->image->setPixmap(QPixmap(":/images/res/bandspread.png"));
        ui->lineEdit_cvmin->setText(roundTo(Cv_min / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_cvmax->setText(roundTo(Cv_max / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_cs->setText(roundTo(Cs / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_ind->setText(roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_flo->setText(roundTo(f_low / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_fhi->setText(roundTo(f_high / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
        showInductanceRange();
        ui->horizontalSlider_L->setValue(L_value_pos);
        on_horizontalSlider_L_valueChanged(L_value_pos);
        ui->lineEdit_flo->setFocus();
        ui->lineEdit_flo->selectAll();
    }
        break;
    case 1:
    {
        ui->image->setPixmap(QPixmap(":/images/res/bandspread.png"));
        ui->lineEdit_cvmin_r->setText(roundTo(Cv_min / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_cvmax_r->setText(roundTo(Cv_max / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_cs_r->setText(roundTo(Cs / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_ind_r->setText(roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
        double dCt = loc.toDouble(ui->lineEdit_ct->text());
        ui->lineEdit_ct_r->setText(loc.toString(toNearestE24(dCt, fOpt->dwAccuracy)));
        double dCp = loc.toDouble(ui->lineEdit_cp->text());
        ui->lineEdit_cp_r->setText(loc.toString(toNearestE24(dCp, fOpt->dwAccuracy)));
        ui->lineEdit_ct_r->setFocus();
        ui->lineEdit_ct_r->selectAll();
    }
        break;
    case 2:
    {
        ui->image->setPixmap(QPixmap(":/images/res/bandspread-t.png"));
        ui->lineEdit_cvmin_t->setText(roundTo(Cv_min / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_cvmax_t->setText(roundTo(Cv_max / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_cs_t->setText(roundTo(Cs / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_flo_t->setText(roundTo(f_low / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_fhi_t->setText(roundTo(f_high / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
        ui->lineEdit_IF->setText(roundTo(f_i / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
        showInductanceRange();
        ui->horizontalSlider_Le->setValue(L_value_pos);
        on_horizontalSlider_Le_valueChanged(L_value_pos);
        ui->lineEdit_flo_t->setFocus();
        ui->lineEdit_flo_t->selectAll();
    }
        break;
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
            showWarning(tr("Warning"), "F_high &lt;= F_low");
            return;
        }
        if (Cv_min >= Cv_max){
            ui->lineEdit_ct->setText("");
            ui->lineEdit_cp->setText("");
            ui->label_info->setText("");
            showWarning(tr("Warning"), "Cv_max &lt;= Cv_min");
            return;
        }
        ui->horizontalSlider_L->setFocus();
        QVector<double> *range = new QVector<double>();
        calcInductanceRange(f_low, f_high, Cv_min, Cv_max, Cs, range, fOpt->dwAccuracy);
        double Lmin = 0.0;
        double Lmax = DBL_MAX;
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
                ui->lineEdit_ct->setText(roundTo(Ct/fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
                ui->lineEdit_cp->setText(roundTo(Cp/fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy));
                ui->label_info->setText(formattedOutput(fOpt, tr("Minimum LC circuit Capacitance") + " Cmin: ",
                                                        roundTo(cap->at(2)/fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy),
                                                        qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>" +
                                        formattedOutput(fOpt, tr("Maximum LC circuit Capacitance") + " Cmax: ",
                                                        roundTo(cap->at(3)/fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy),
                                                        qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8())));
            } else {
                ui->lineEdit_ct->setText("");
                ui->lineEdit_cp->setText("");
                ui->label_info->setText("");
                showWarning(tr("Warning"), tr("The Cv capacitance range (combined with the circuit stray capacitance) is too narrow to overlap the desired frequency range. "
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
            showWarning(tr("Warning"), "Cv_max &lt;= Cv_min");
            return;
        }
        QVector<double> *freq = new QVector<double>();
        calcFrequencyRange(Ct, Cp, Cv_min, Cv_max, Cs, ind, freq, fOpt->dwAccuracy);
        if(!freq->isEmpty()){
            f_low = freq->at(0);
            f_high = freq->at(1);
            ui->lineEdit_flo_r->setText(roundTo(f_low/fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
            ui->lineEdit_fhi_r->setText(roundTo(f_high/fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy));
            ui->label_info_r->setText(formattedOutput(fOpt, tr("Minimum LC circuit Capacitance") + " Cmin: ",
                                                      roundTo(freq->at(2)/fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy),
                                                      qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>" +
                                      formattedOutput(fOpt, tr("Maximum LC circuit Capacitance") + " Cmax: ",
                                                      roundTo(freq->at(3)/fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy),
                                                      qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8())));
        }
        delete freq;
    }
        break;
    case 2: {
        bool ok1, ok2, ok3, ok4, ok5, ok6, ok7;
        f_low = loc.toDouble(ui->lineEdit_flo_t->text(), &ok1)*fOpt->dwFrequencyMultiplier;
        f_high = loc.toDouble(ui->lineEdit_fhi_t->text(), &ok2)*fOpt->dwFrequencyMultiplier;
        f_i = loc.toDouble(ui->lineEdit_IF->text(), &ok3)*fOpt->dwFrequencyMultiplier;
        Cv_min = loc.toDouble(ui->lineEdit_cvmin_t->text(), &ok4)*fOpt->dwCapacityMultiplier;
        Cv_max = loc.toDouble(ui->lineEdit_cvmax_t->text(), &ok5)*fOpt->dwCapacityMultiplier;
        Cs = loc.toDouble(ui->lineEdit_cs_t->text(), &ok6)*fOpt->dwCapacityMultiplier;
        ind = loc.toDouble(ui->lineEdit_Le->text(), &ok7)*fOpt->dwInductanceMultiplier;
        bool isHighSideInjection = ui->comboBox->currentIndex();
        double lo_f_low = f_low + f_i;
        double lo_f_high = f_high + f_i;
        double f_im_low = f_low + 2 * f_i;
        double f_im_high = f_high + 2 * f_i;
        if(!isHighSideInjection){
            lo_f_low = f_low - f_i;
            lo_f_high = f_high - f_i;
            f_im_low = f_low - 2 * f_i;
            if(f_im_low < 0)
                f_im_low = 0.0;
            f_im_high = f_high - 2 * f_i;
        }
        if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)){
            ui->label_result_t->setText("");
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if ((f_low == 0)||(f_high == 0)||(Cv_min == 0)||(Cv_max == 0)||(f_i == 0)){
            ui->label_result_t->setText("");
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        if (f_low >= f_high){
            ui->label_result_t->setText("");
            showWarning(tr("Warning"), "F_high &lt;= F_low");
            return;
        }
        if (Cv_min >= Cv_max){
            ui->label_result_t->setText("");
            showWarning(tr("Warning"), "Cv_max &lt;= Cv_min");
            return;
        }
        if ((f_i < f_high) && (f_i > f_low)){
            ui->label_result_t->setText("");
            showWarning(tr("Warning"), "F_low &lt; IF &lt; F_high");
            return;
        }
        if (ui->checkBox->isChecked()){
            ind = 0;
        } else {
            if(!ok7){
                ui->label_result_t->setText("");
                showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                return;
            }
            if (ind == 0){
                ui->label_result_t->setText("");
                showWarning(tr("Warning"), "Le = 0!");
                return;
            }
        }
        ui->horizontalSlider_Le->setFocus();
        QVector<double> *res = new QVector<double>();
        QVector<double> *conFreq = new QVector<double>();
        double maxDelta = 0.0;
        int result  = trackingLocalOscillator(f_low, f_high, f_i, Cv_min, Cv_max, Cs, &ind, res, conFreq, fOpt->dwAccuracy,
                                              isHighSideInjection, ui->checkBox->isChecked(), &maxDelta);
        if(result == 0){
            QString out = "<p><u>" + tr("Result") + ":</u><br/>";
            out += formattedOutput(fOpt, tr("Parallel Capacitor") + " C1 = ", roundTo(res->at(0) / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy),
                                       qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
            out += formattedOutput(fOpt, tr("Serial Capacitor") + " C2 = ", roundTo(res->at(1) / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy),
                                       qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
            if (ui->checkBox->isChecked()){
                out += "<br/>" + formattedOutput(fOpt, tr("Inductance") + " Le = ", roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                       qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
            }
            out += "</p><p>" + formattedOutput(fOpt, tr("Parallel Capacitor") + " C1o = ", roundTo(res->at(2) / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy),
                                       qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
            out += formattedOutput(fOpt, tr("Serial Capacitor") + " C2o = ", roundTo(res->at(3) / fOpt->dwCapacityMultiplier, loc, fOpt->dwAccuracy),
                                       qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
            out += formattedOutput(fOpt, tr("Inductance") + " Lo = ", roundTo(res->at(4) / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                       qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "</p>";
            out += "<p>" + tr("Zero error tracking points") + ":<br/>";
            if (conFreq->isEmpty()){
                out += "- - -<br/>";
            } else {
                for (int k = 0; k < conFreq->length(); k++){
                    out += formattedOutput(fOpt, "f" + QString::number(k + 1) + " = ", roundTo(conFreq->at(k) / fOpt->dwFrequencyMultiplier,
                                                                                                      loc, fOpt->dwAccuracy),
                                                      qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8())) + "; ";
                }
                out += "<br/>";
            }
            out += formattedOutput(fOpt, tr("Peak tracking error") + " Δf_err = ", roundTo(maxDelta / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy),
                                   qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
            out += formattedOutput(fOpt, " (", roundTo((200.0 * maxDelta / (f_high + f_low)), loc, 2), "% )<br/>");
            out += formattedOutput(fOpt, tr("Local oscillator frequency range") + " Δf_LO = ",
                                   roundTo(lo_f_low / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy) + " ... " +
                                   roundTo(lo_f_high / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy),
                                   qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
            if (f_im_high > 0.0){
                out += "<br/>" + formattedOutput(fOpt, tr("Image frequency range") + " Δf_image = ",
                                                 roundTo(f_im_low / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy) + " ... " +
                                                 roundTo(f_im_high / fOpt->dwFrequencyMultiplier, loc, fOpt->dwAccuracy),
                                                 qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
            }
            out += "</p>";
            ui->label_result_t->setText(out);
        } else if(result == 1){
            ui->label_result_t->setText(tr("Solution failed"));
            showWarning(tr("Warning"), tr("Unable to create Local Oscillator LC-circuit. Try changing the LO frequency position and try again."));
        } else if(result == -1){
            ui->label_result_t->setText(tr("Solution failed"));
            showWarning(tr("Warning"), tr("The Cv capacitance range (combined with the circuit stray capacitance) is too narrow to overlap the desired frequency range. "
                                          "Please change your frequency range or variable capacitor parameters and try again."));
        } else if(result == -2){
            ui->label_result_t->setText(tr("Solution failed"));
            showWarning(tr("Warning"), tr("The inductance value is out of range."));
        } else {
            ui->label_result_t->setText(tr("Solution failed"));
        }
        delete res;
        delete conFreq;
    }
        break;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_pushButton_export_clicked()
{
    QString sCaption = QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle();
    sCaption += "<p>" + ui->tabWidget->tabText(ui->tabWidget->currentIndex()) + "</p>";
    QString sInput = "<p><u>" + tr("Input data") + ":</u><br/>";
    switch (ui->tabWidget->currentIndex()){
    case 0:{
        if ((!ui->lineEdit_ct->text().isEmpty()) && (!ui->lineEdit_cp->text().isEmpty())){
            QString sImage = "<img src=\":/images/res/bandspread.png\">";
            sInput += formattedOutput(fOpt, ui->label_flo->text(), ui->lineEdit_flo->text(), ui->label_flo_m->text()) + "<br/>";
            sInput += formattedOutput(fOpt, ui->label_fhi->text(), ui->lineEdit_fhi->text(), ui->label_fhi_m->text()) + "<br/>";
            sInput += formattedOutput(fOpt, ui->label_cvmin->text(), ui->lineEdit_cvmin->text(), ui->label_cvmin_m->text()) + "<br/>";
            sInput += formattedOutput(fOpt, ui->label_cvmax->text(), ui->lineEdit_cvmax->text(), ui->label_cvmax_m->text()) + "<br/>";
            sInput += formattedOutput(fOpt, ui->label_cs->text(), ui->lineEdit_cs->text(), ui->label_cs_m->text()) + "<br/>";
            sInput += formattedOutput(fOpt, ui->label_ind->text(), ui->lineEdit_ind->text(), ui->label_ind_m->text()) + "</p>";
            QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
            sResult += formattedOutput(fOpt, ui->label_ct->text(), ui->lineEdit_ct->text(), ui->label_ct_m->text()) + "<br/>";
            sResult += formattedOutput(fOpt, ui->label_cp->text(), ui->lineEdit_cp->text(), ui->label_cp_m->text()) + "</p>";
            sResult += ui->label_info->text();
            emit sendResult(sCaption + LIST_SEPARATOR + sImage + LIST_SEPARATOR + sInput + LIST_SEPARATOR + sResult);
        }
    }
        break;
    case 1:{
        if ((!ui->lineEdit_flo_r->text().isEmpty()) && (!ui->lineEdit_fhi_r->text().isEmpty())){
            QString sImage = "<img src=\":/images/res/bandspread.png\">";
            sInput += formattedOutput(fOpt, ui->label_ct_r->text(), ui->lineEdit_ct_r->text(), ui->label_ct_m_r->text()) + "<br/>";
            sInput += formattedOutput(fOpt, ui->label_cp_r->text(), ui->lineEdit_cp_r->text(), ui->label_cp_m_r->text()) + "<br/>";
            sInput += formattedOutput(fOpt, ui->label_cvmin_r->text(), ui->lineEdit_cvmin_r->text(), ui->label_cvmin_m_r->text()) + "<br/>";
            sInput += formattedOutput(fOpt, ui->label_cvmax_r->text(), ui->lineEdit_cvmax_r->text(), ui->label_cvmax_m_r->text()) + "<br/>";
            sInput += formattedOutput(fOpt, ui->label_cs_r->text(), ui->lineEdit_cs_r->text(), ui->label_cs_m_r->text()) + "<br/>";
            sInput += formattedOutput(fOpt, ui->label_ind_r->text(), ui->lineEdit_ind_r->text(), ui->label_ind_m_r->text()) + "</p>";
            QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
            sResult += formattedOutput(fOpt, ui->label_flo_r->text(), ui->lineEdit_flo_r->text(), ui->label_flo_m_r->text()) + "<br/>";
            sResult += formattedOutput(fOpt, ui->label_fhi_r->text(), ui->lineEdit_fhi_r->text(), ui->label_fhi_m_r->text()) + "</p>";
            sResult += ui->label_info_r->text();
            emit sendResult(sCaption + LIST_SEPARATOR + sImage + LIST_SEPARATOR + sInput + LIST_SEPARATOR + sResult);
        }
    }
        break;
    case 2:{
        if (!ui->label_result_t->text().isEmpty()){
            if (ui->label_result_t->text() != tr("Solution failed")){
                QString s = ui->label_result_t->text();
                QString sImage = "<img src=\":/images/res/bandspread-t.png\">";
                sInput += formattedOutput(fOpt, ui->label_flo_t->text(), ui->lineEdit_flo_t->text(), ui->label_flo_m_t->text()) + "<br/>";
                sInput += formattedOutput(fOpt, ui->label_fhi_t->text(), ui->lineEdit_fhi_t->text(), ui->label_fhi_m_t->text()) + "<br/>";
                sInput += formattedOutput(fOpt, ui->label_IF->text(), ui->lineEdit_IF->text(), ui->label_IF_m->text()) + "<br/>";
                sInput += ui->label->text() + ": " + ui->comboBox->currentText() + "<br/>";
                sInput += formattedOutput(fOpt, ui->label_cvmin_t->text(), ui->lineEdit_cvmin_t->text(), ui->label_cvmin_m_t->text()) + "<br/>";
                sInput += formattedOutput(fOpt, ui->label_cvmax_t->text(), ui->lineEdit_cvmax_t->text(), ui->label_cvmax_m_t->text()) + "<br/>";
                sInput += formattedOutput(fOpt, ui->label_cs_t->text(), ui->lineEdit_cs_t->text(), ui->label_cs_m_t->text());
                if (!ui->checkBox->isChecked()){
                    sInput += "<br/>" + formattedOutput(fOpt, ui->label_ind_t->text(), ui->lineEdit_Le->text(), ui->label_ind_m_t->text());
                }
                sInput += "</p>";
                QString sResult = ui->label_result_t->text();
                emit sendResult(sCaption + LIST_SEPARATOR + sImage + LIST_SEPARATOR + sInput + LIST_SEPARATOR + sResult);
            }
        }
    }
        break;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_pushButton_help_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/bandspread.html"));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_pushButton_close_clicked()
{
    this->close();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_lineEdit_ind_editingFinished()
{
    ind = loc.toDouble(ui->lineEdit_ind->text())*fOpt->dwInductanceMultiplier;
    ui->lineEdit_ind->clearFocus();
    int minValue = ui->horizontalSlider_L->minimum();
    int maxValue = ui->horizontalSlider_L->maximum();
    double Lmin = loc.toDouble(ui->label_L_min->text())*fOpt->dwInductanceMultiplier;
    double Lmax = loc.toDouble(ui->label_L_max->text())*fOpt->dwInductanceMultiplier;
    double p = 1.0 / (maxValue);
    double step = p * (Lmax - Lmin);
    int value = round((ind - Lmin) / step);
    if ((value >= minValue) && (value <= maxValue)){
        L_value_pos = value;
        ui->horizontalSlider_L->setValue(value);
    }
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
void Bandspread::on_lineEdit_flo_t_editingFinished()
{
    f_low = loc.toDouble(ui->lineEdit_flo_t->text())*fOpt->dwFrequencyMultiplier;
    ui->lineEdit_flo_t->clearFocus();
    showInductanceRange();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_lineEdit_fhi_t_editingFinished()
{
    f_high = loc.toDouble(ui->lineEdit_fhi_t->text())*fOpt->dwFrequencyMultiplier;
    ui->lineEdit_fhi_t->clearFocus();
    showInductanceRange();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_lineEdit_IF_editingFinished()
{
    f_i = loc.toDouble(ui->lineEdit_IF->text())*fOpt->dwFrequencyMultiplier;
    ui->lineEdit_IF->clearFocus();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_lineEdit_cvmin_t_editingFinished()
{
    Cv_min = loc.toDouble(ui->lineEdit_cvmin_t->text())*fOpt->dwCapacityMultiplier;
    ui->lineEdit_cvmin_t->clearFocus();
    showInductanceRange();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_lineEdit_cvmax_t_editingFinished()
{
    Cv_max = loc.toDouble(ui->lineEdit_cvmax_t->text())*fOpt->dwCapacityMultiplier;
    ui->lineEdit_cvmax_t->clearFocus();
    showInductanceRange();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_lineEdit_cs_t_editingFinished()
{
    Cs = loc.toDouble(ui->lineEdit_cs_t->text())*fOpt->dwCapacityMultiplier;
    ui->lineEdit_cs_t->clearFocus();
    showInductanceRange();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_lineEdit_Le_editingFinished()
{
    ind = loc.toDouble(ui->lineEdit_Le->text())*fOpt->dwInductanceMultiplier;
    ui->lineEdit_Le->clearFocus();
    int minValue = ui->horizontalSlider_Le->minimum();
    int maxValue = ui->horizontalSlider_Le->maximum();
    double Lmin = loc.toDouble(ui->label_Le_min->text())*fOpt->dwInductanceMultiplier;
    double Lmax = loc.toDouble(ui->label_Le_max->text())*fOpt->dwInductanceMultiplier;
    double p = 1.0 / (maxValue);
    double step = p * (Lmax - Lmin);
    int value = round((ind - Lmin) / step);
    if ((value >= minValue) && (value <= maxValue)){
        L_value_pos = value;
        ui->horizontalSlider_Le->setValue(value);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::showInductanceRange()
{
    if ((f_low > 0) && (f_high > 0) && (Cv_min > 0) && (Cv_max > 0)){
        QVector<double> *range = new QVector<double>();
        calcInductanceRange(f_low, f_high, Cv_min, Cv_max, Cs, range, fOpt->dwAccuracy);
        if (!range->isEmpty()){
            if (ui->tabWidget->currentIndex() == 0){
                ui->label_L_min->setText(roundTo(range->at(0) / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
                ui->label_L_max->setText(roundTo(range->at(1) / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
                on_horizontalSlider_L_valueChanged(L_value_pos);
            } else if (ui->tabWidget->currentIndex() == 2){
                ui->label_Le_min->setText(roundTo(range->at(0) / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
                ui->label_Le_max->setText(roundTo(range->at(1) / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
                on_horizontalSlider_Le_valueChanged(L_value_pos);
            }
        } else {
            if (ui->tabWidget->currentIndex() == 0){
                ui->label_L_min->setText("");
                ui->label_L_max->setText("");
            } else if (ui->tabWidget->currentIndex() == 2){
                ui->label_Le_min->setText("");
                ui->label_Le_max->setText("");
            }
        }
        delete range;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_horizontalSlider_L_valueChanged(int value)
{
    if (!ui->label_L_min->text().isEmpty() && !ui->label_L_max->text().isEmpty()){
        double Lmin = loc.toDouble(ui->label_L_min->text())*fOpt->dwInductanceMultiplier;
        double Lmax = loc.toDouble(ui->label_L_max->text())*fOpt->dwInductanceMultiplier;
        double p = 1.0 / (ui->horizontalSlider_L->maximum());
        double step = p * (Lmax - Lmin);
        double L = Lmin + value * step;
        if (L > Lmax)
            L = Lmax;
        ui->lineEdit_ind->setText(roundTo(L / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
        L_value_pos = value;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_horizontalSlider_Le_valueChanged(int value)
{
    if (!ui->label_Le_min->text().isEmpty() && !ui->label_Le_max->text().isEmpty()){
        double Lmin = loc.toDouble(ui->label_Le_min->text())*fOpt->dwInductanceMultiplier;
        double Lmax = loc.toDouble(ui->label_Le_max->text())*fOpt->dwInductanceMultiplier;
        double p = 1.0 / (ui->horizontalSlider_Le->maximum());
        double step = p * (Lmax - Lmin);
        double L = Lmin + value * step;
        if (L > Lmax)
            L = Lmax;
        ui->lineEdit_Le->setText(roundTo(L / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
        L_value_pos = value;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_toolButton_plus_clicked()
{
    bool ok = false;
    double L = loc.toDouble(ui->lineEdit_Le->text(), &ok);
    if(ok){
        double Lmax = loc.toDouble(ui->label_Le_max->text(), &ok);
        if(ok){
            double s = pow(10, (int)-fOpt->dwAccuracy);
            double nL = L + s;
            if (nL > Lmax)
                nL = Lmax;
            ui->lineEdit_Le->setText(roundTo(nL, loc, fOpt->dwAccuracy));
            on_lineEdit_Le_editingFinished();
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_toolButton_minus_clicked()
{
    bool ok = false;
    double L = loc.toDouble(ui->lineEdit_Le->text(), &ok);
    if(ok){
        double Lmin = loc.toDouble(ui->label_Le_min->text(), &ok);
        if(ok){
            double s = pow(10, (int)-fOpt->dwAccuracy);
            double nL = L - s;
            if (nL < Lmin)
                nL = Lmin;
            ui->lineEdit_Le->setText(roundTo(nL, loc, fOpt->dwAccuracy));
            on_lineEdit_Le_editingFinished();
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_toolButton_plus_L_clicked()
{
    bool ok = false;
    double L = loc.toDouble(ui->lineEdit_ind->text(), &ok);
    if(ok){
        double Lmax = loc.toDouble(ui->label_L_max->text(), &ok);
        if(ok){
            double s = pow(10, (int)-fOpt->dwAccuracy);
            double nL = L + s;
            if (nL > Lmax)
                nL = Lmax;
            ui->lineEdit_ind->setText(roundTo(nL, loc, fOpt->dwAccuracy));
            on_lineEdit_ind_editingFinished();
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_toolButton_minus_L_clicked()
{
    bool ok = false;
    double L = loc.toDouble(ui->lineEdit_ind->text(), &ok);
    if(ok){
        double Lmin = loc.toDouble(ui->label_L_min->text(), &ok);
        if(ok){
            double s = pow(10, (int)-fOpt->dwAccuracy);
            double nL = L - s;
            if (nL < Lmin)
                nL = Lmin;
            ui->lineEdit_ind->setText(roundTo(nL, loc, fOpt->dwAccuracy));
            on_lineEdit_ind_editingFinished();
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Bandspread::on_checkBox_toggled(bool checked)
{
    if (checked){
        ui->label_ind_t->setVisible(false);
        ui->label_ind_m_t->setVisible(false);
        ui->lineEdit_Le->setVisible(false);
        ui->groupBox_Le->setVisible(false);
    } else {
        ui->label_ind_t->setVisible(true);
        ui->label_ind_m_t->setVisible(true);
        ui->lineEdit_Le->setVisible(true);
        ui->groupBox_Le->setVisible(true);
    }
    ui->label_result_t->clear();
}
