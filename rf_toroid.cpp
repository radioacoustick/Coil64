/* rf_toroid.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
Copyright (C) 2021 Kustarev V.

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

#include "rf_toroid.h"
#include "ui_rf_toroid.h"


enum _ToroidDataTableColumn
{
    _FREQUENCY,
    _MU_PRIM,
    _MU_PRIM_PRIM
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RF_Toroid::RF_Toroid(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RF_Toroid)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, MAX_DOUBLE, 380);
    ui->lineEdit_N->setValidator(dv);
    ui->lineEdit_f->setValidator(dv);
    ui->lineEdit_d->setValidator(dv);
    ui->lineEdit_cs->setValidator(dv);
    ui->lineEdit_od->setValidator(dv);
    ui->lineEdit_id->setValidator(dv);
    ui->lineEdit_h->setValidator(dv);
    ui->lineEdit_c->setValidator(dv);
    ui->comboBox->clear();
    ui->comboBox->addItem(tr("Manually"));
    QStringList listFiles;
#if defined(Q_OS_MAC)
    QDir dir(":/data/data");
#else
    QDir dir(QApplication::applicationDirPath() + "/Data");
#endif
    if(dir.exists()){
        listFiles = dir.entryList(QStringList("*"), QDir::Files);
    }
    if (!listFiles.isEmpty()){
        for (int i = 0; i < listFiles.length(); i++){
            QString fileName = listFiles.at(i);
            QString itemName = fileName.left(fileName.indexOf("-")) + "-" + tr("Material");
            ui->comboBox->addItem(itemName);
        }
    }
    ui->comboBox_2->clear();
    ui->comboBox_2->addItem(tr("Manually"));
    for (int i = 0; i < 28; i++){
        QString fsise_str = FToroidSize[i];
        QStringList f_size_val = fsise_str.split(",");
        ui->comboBox_2->addItem("FT-" + f_size_val.at(0));
    }
    ui->label_N->setText(tr("Number of turns") + " N:");
    ui->label_f->setText(tr("Working frequency") + " f:");
    ui->label_cs->setText(tr("Self capacitance") + " Cs:");
    ui->label_od->setText(tr("Outside diameter") + " OD:");
    ui->label_id->setText(tr("Inside diameter") + " ID:");
    ui->label_h->setText(tr("Core height") + " H:");
    ui->label_c->setText(tr("Chamfer") + " C:");
    ui->label_d->setText(tr("Wire diameter") + " d:");
    ui->lineEdit_N->setFocus();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RF_Toroid::~RF_Toroid()
{
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "rf_torus" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", this->size());
    settings->setValue("matInit", ui->comboBox->currentIndex());
    settings->setValue("sizeInit", ui->comboBox_2->currentIndex());
    settings->setValue("N", N);
    settings->setValue("f", f);
    settings->setValue("Cs", Cs);
    settings->setValue("isCsAuto", isCsAuto);
    settings->setValue("od", od);
    settings->setValue("id", id);
    settings->setValue("h", h);
    settings->setValue("c", c);
    settings->setValue("d", d);
    settings->setValue("mu1", mu1);
    settings->setValue("mu2", mu2);
    settings->endGroup();
    delete settings;
    delete fOpt;
    delete dv;
    delete ui;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<double> RF_Toroid::readMaterialData(QString fileName, QStringList *lDataList, int column)
{
    std::vector <double> result;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream *out = new QTextStream (&file);
        QStringList lineList = out->readAll().split("\r");
        *lDataList = lineList.at(2).split(",");
        QStringList iMaterialDataList = {"0",lDataList->at(0),"0"};
        result.push_back(iMaterialDataList[column].toDouble());
        for(int i = 4 ; i < lineList.count() - 1 ; i++){
            QStringList materialDataList = lineList.at(i).split(",");
            result.push_back(materialDataList[column].toDouble());
        }
    }
    file.close();
    return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::getOpt(_OptionStruct gOpt)
{
    *fOpt = gOpt;
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "rf_torus" );
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    resize(size);
    move(pos);
    N = settings->value("N", 0).toDouble();
    f = settings->value("f", 0).toDouble();
    Cs = settings->value("Cs", 0).toDouble();
    isCsAuto = settings->value("isCsAuto", true).toBool();
    od = settings->value("od", 0).toDouble();
    id = settings->value("id", 0).toDouble();
    h = settings->value("h", 0).toDouble();
    c = settings->value("c", 0).toDouble();
    d = settings->value("d", 0).toDouble();
    mu1 = settings->value("mu1", 0).toDouble();
    mu2 = settings->value("mu2", 0).toDouble();
    int matInit = settings->value("matInit", 0).toInt();
    int sizeInit = settings->value("sizeInit", 0).toInt();
    settings->endGroup();
    delete settings;
    ui->label_f_m->setText(qApp->translate("Context", fOpt->ssFrequencyMeasureUnit.toUtf8()));
    ui->label_cs_m->setText(qApp->translate("Context", fOpt->ssCapacityMeasureUnit.toUtf8()));
    ui->label_01->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_02->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_03->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_04->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_d_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->lineEdit_N->setText(loc.toString(N));
    ui->lineEdit_f->setText(loc.toString(f / fOpt->dwFrequencyMultiplier));
    ui->lineEdit_cs->setText(loc.toString(Cs / fOpt->dwCapacityMultiplier));
    ui->lineEdit_od->setText(loc.toString(od / fOpt->dwLengthMultiplier));
    ui->lineEdit_id->setText(loc.toString(id / fOpt->dwLengthMultiplier));
    ui->lineEdit_h->setText(loc.toString(h / fOpt->dwLengthMultiplier));
    ui->lineEdit_c->setText(loc.toString(c / fOpt->dwLengthMultiplier));
    ui->lineEdit_d->setText(loc.toString(d / fOpt->dwLengthMultiplier));
    if (fOpt->styleGUI == _DarkStyle)
        styleInfoColor = "<span style=\"color:yellow;\">";
    else
        styleInfoColor = "<span style=\"color:blue;\">";
    ui->label_cs->setChecked(isCsAuto);
    on_label_cs_toggled(isCsAuto);
    ui->comboBox->setCurrentIndex(matInit);
    on_comboBox_currentIndexChanged(matInit);
    ui->comboBox_2->setCurrentIndex(sizeInit);
    on_comboBox_2_currentIndexChanged(sizeInit);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double RF_Toroid::getDCresistance(double N, double OD, double ID, double H, double dw)
{
    double P = (OD - ID) + (2 * H);
    double Resistivity = mtrl[Cu][Rho]*1e6;
    return N * (Resistivity * P * 4e-3) / (M_PI * dw * dw); // DC resistance of the wire;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_pushButton_close_clicked()
{
    this->close();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_comboBox_currentIndexChanged(int index)
{
    if (index > 0){
        ui->lineEdit_mu1->setReadOnly(true);
        ui->lineEdit_mu2->setReadOnly(true);
        QStringList mfeatures;
        QString arg = ui->comboBox->currentText().left(2);
#if defined(Q_OS_MAC)
        QString fileName = ":/data/data/" + arg + "-Material-Fair-Rite.csv";
#else
        QString fileName = QApplication::applicationDirPath() + "/Data/" + arg + "-Material-Fair-Rite.csv";
#endif
        std::vector <double> fVector = readMaterialData(fileName, &mfeatures, _FREQUENCY);
        if (fVector.size() > 0){
            QString info = "<p>" + tr("Initial magnetic permeability") + " (μ<sub>i</sub>): " + styleInfoColor + mfeatures.at(0)
                    + "</span><br/>";
            info += tr("Residual flux density") + " (Br): " + styleInfoColor + mfeatures.at(1) + "</span>&nbsp;Gs<br/>";
            info += tr("Coercive Force") + " (Hc): " + styleInfoColor + mfeatures.at(2) + "</span>&nbsp;Oe</p>";
            if ((f * 1e6 > fVector[0]) && (f * 1e6 < fVector[fVector.size() - 1])){
                std::vector <double> mu1Vector = readMaterialData(fileName, &mfeatures, _MU_PRIM);
                std::vector <double> mu2Vector = readMaterialData(fileName, &mfeatures, _MU_PRIM_PRIM);
                if ((mu1Vector.size() > 0) && (mu2Vector.size() > 0)){
                    tk::spline s;
                    s.set_points(fVector,mu1Vector);
                    ui->lineEdit_mu1->setText(loc.toString(s(f * 1e6)));
                    s.set_points(fVector,mu2Vector);
                    ui->lineEdit_mu2->setText(loc.toString(s(f * 1e6)));
                }
            } else {
                ui->lineEdit_mu1->setText("");
                ui->lineEdit_mu2->setText("");
                ui->lineEdit_mu1->setText("f ⊄ Δf");
                ui->lineEdit_mu2->setText("f ⊄ Δf");
                info += tr("The frequency is out of range");
            }
            ui->label_info->setText(info);
        }
    } else {
        ui->lineEdit_mu1->setReadOnly(false);
        ui->lineEdit_mu2->setReadOnly(false);
        ui->lineEdit_mu1->setText(loc.toString(mu1));
        ui->lineEdit_mu2->setText(loc.toString(mu2));
        ui->label_info->setText(tr(""));
        ui->lineEdit_mu1->setFocus();
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_comboBox_2_currentIndexChanged(int index)
{
    if (index > 0){
        double od = 0, id = 0, h = 0;
        ui->lineEdit_od->setReadOnly(true);
        ui->lineEdit_id->setReadOnly(true);
        ui->lineEdit_h->setReadOnly(true);
        QString fd_curr_size = ui->comboBox_2->currentText().mid(3);
        for (int i = 0; i < 28; i++) {
            QString fsise_str = FToroidSize[i];
            QStringList f_size_val = fsise_str.split(",");
            if (f_size_val[0] == fd_curr_size) {
                QString OD = f_size_val[1];
                QString ID = f_size_val[2];
                QString H = f_size_val[3];
                od = OD.toDouble()*25.4/fOpt->dwLengthMultiplier;
                id = ID.toDouble()*25.4/fOpt->dwLengthMultiplier;
                h = H.toDouble()*25.4/fOpt->dwLengthMultiplier;
            }
        }
        ui->lineEdit_od->setText(loc.toString(od));
        ui->lineEdit_id->setText(loc.toString(id));
        ui->lineEdit_h->setText(loc.toString(h));
    } else {
        ui->lineEdit_od->setReadOnly(false);
        ui->lineEdit_id->setReadOnly(false);
        ui->lineEdit_h->setReadOnly(false);
        ui->lineEdit_od->setText(loc.toString(od));
        ui->lineEdit_id->setText(loc.toString(id));
        ui->lineEdit_h->setText(loc.toString(h));
        ui->lineEdit_od->setFocus();
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_lineEdit_N_editingFinished()
{
    N = loc.toDouble(ui->lineEdit_N->text());
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_lineEdit_cs_editingFinished()
{
    Cs = loc.toDouble(ui->lineEdit_cs->text()) * fOpt->dwCapacityMultiplier;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_label_cs_toggled(bool checked)
{
    isCsAuto = checked;
    if (checked){
        ui->lineEdit_cs->setEnabled(false);
        ui->lineEdit_cs->setText("");

    } else {
        ui->lineEdit_cs->setEnabled(true);
        ui->lineEdit_cs->setText(loc.toString(Cs / fOpt->dwCapacityMultiplier, 'f', 1));
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_lineEdit_f_editingFinished()
{
    f = loc.toDouble(ui->lineEdit_f->text()) * fOpt->dwFrequencyMultiplier;
    if ((ui->comboBox->currentIndex() > 0))
        on_comboBox_currentIndexChanged(ui->comboBox->currentIndex());
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_lineEdit_od_editingFinished()
{
    od = loc.toDouble(ui->lineEdit_od->text()) * fOpt->dwLengthMultiplier;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_lineEdit_id_editingFinished()
{
    id = loc.toDouble(ui->lineEdit_id->text()) * fOpt->dwLengthMultiplier;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_lineEdit_h_editingFinished()
{
    h = loc.toDouble(ui->lineEdit_h->text()) * fOpt->dwLengthMultiplier;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_lineEdit_c_editingFinished()
{
    c = loc.toDouble(ui->lineEdit_c->text()) * fOpt->dwLengthMultiplier;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_lineEdit_d_editingFinished()
{
     d = loc.toDouble(ui->lineEdit_d->text()) * fOpt->dwLengthMultiplier;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_lineEdit_mu1_editingFinished()
{
    mu1 = loc.toDouble(ui->lineEdit_mu1->text());
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_lineEdit_mu2_editingFinished()
{
    mu2 = loc.toDouble(ui->lineEdit_mu2->text());
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_pushButton_calculate_clicked()
{
    bool ok1, ok2, ok3, ok4, ok5;
    double m1 = loc.toDouble(ui->lineEdit_mu1->text(), &ok1);
    double m2 = loc.toDouble(ui->lineEdit_mu2->text(), &ok2);
    od = loc.toDouble(ui->lineEdit_od->text(), &ok3) * fOpt->dwLengthMultiplier;;
    id = loc.toDouble(ui->lineEdit_id->text(), &ok4) * fOpt->dwLengthMultiplier;;
    h = loc.toDouble(ui->lineEdit_h->text(), &ok5) * fOpt->dwLengthMultiplier;;
    if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        ui->label_result->setText("");
        return;
    }
    if ((od < id) || (od <= c) || (id <= c) || (h <= c)){
        showWarning(tr("Warning"), tr("One or more dimensions have an invalid value!"));
        ui->label_result->setText("");
        return;
    }
    if ((f > 0) && (N > 0) && (od > 0) && (id > 0) && (h > 0) && (m1 > 0)){
        double freq = f * 1e6;
        double iR = id / 2;
        double oR = od / 2;
        double cr = c / M_SQRT2; //Chamfer radius
        //
        if((cr < (od - id) / 4) && (cr < h / 2)){
            double k = 0.8584 * pow(cr, 3) / (h * (oR - iR)); //correction factor for the chamfer
            double he = h * (1 - k); //correction C1 with chamfer by correcting h
            double C1 = 2 * M_PI / (he * log(od / id)); // C1
            double al = mu0 * m1  * 1e6 / C1; // AL-factor of the torus [nH/N^2]
            double lp = od - id + 2 * h;
            //start calculating the stray self-capacitance by
            //http://g3rbj.co.uk/wp-content/uploads/2015/08/Self-Resonance-in-Toroidal-Inductors.pdf
            double le = 2 * M_PI * log(od / id)/(1 / iR - 1 / oR); //Equivalent magnetic path length C1^2/C2 [mm]
            double A = (1.13e-3 * C1 * pow(lp, 2)) / (0.4 * M_PI);
            double B = (1.13e-1 * C1 * pow(0.9 * le, 2)) / (0.4 * M_PI);
            double Csm = A + B / pow(N, 2);//self-capacitance Csm
            //end calculating the stray self-capacitance
            double l = al * pow(N, 2) * 1e-3; //inductance of the coil [microH]
            double xl = 2 * M_PI * l * 1e-6 * freq; //reactance of the coil [Ohm]
            complex<double> zl = complex<double>(xl * m2 / m1, xl); //impedance of the coil without the stray capacitance (z = r + jx)
            complex<double> yc = complex<double>(0); //inverse impedance of the stray capacitance
            if (isCsAuto)
                yc = complex<double>(0, 2 * M_PI * Csm * 1e-12 * freq);
            else
                yc = complex<double>(0, 2 * M_PI * Cs * 1e-12 * freq);
            double Q = 0;
            complex<double> z = complex<double>(1) /((complex<double>(1) / zl) + yc); //summary impedance of the coil
            double Rdc = getDCresistance(N, od, id, h, d);//DC resistance of the winding [Ohm]
            if (m2 > 0)
                Q = z.imag() / (z.real() + Rdc); //Coil constructive Q-factor
            QString result = "<p>";
            QString sComplexZ = loc.toString(z.real(), 'f', fOpt->dwAccuracy) + " + j" + loc.toString(z.imag(), 'f', fOpt->dwAccuracy);
            if(z.imag() < 0)
                sComplexZ = loc.toString(z.real(), 'f', fOpt->dwAccuracy) + " - j" + loc.toString(-z.imag(), 'f', fOpt->dwAccuracy);
            result += "Z (Ω) = " + sComplexZ + "<br/>";
            result += "|Z| (Ω) = " + loc.toString(abs(z), 'f', fOpt->dwAccuracy) + "<br/><br/>";
            if (Q >= 0){
                result += tr("Equivalent series inductance") + " Ls = " +
                        loc.toString(z.imag() / (2*M_PI*freq)*1e6 / fOpt->dwInductanceMultiplier, 'f', fOpt->dwAccuracy) + " " +
                        qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()) + "<br/>";
                result += tr("Loss resistance") + " ESR = " + loc.toString(z.real() + Rdc, 'f', fOpt->dwAccuracy) + " " + tr("Ohm") + "<br/>";
                if (isCsAuto)
                    result += tr("Self capacitance") + " Cs = " + loc.toString(Csm, 'f', 1) + " "
                        + qApp->translate("Context", "pF") + "<br/>";
                result += tr("Coil constructive Q-factor") + " Q = " + loc.toString(Q, 'f', fOpt->dwAccuracy) + "<br/>";
                result += "A<sub>L</sub>&nbsp;= " + loc.toString(round(al))
                        + "&nbsp;" +  qApp->translate("Context","nH") + "/N<sup>2</sup></p>";
                result += "<hr><p>";
                result += "<u>" + tr("Input data for LTSpice") + ":</u><br/>";
                result += "Inductance: " + QString::number(z.imag() / (2*M_PI*freq)*1e6, 'f', fOpt->dwAccuracy) + "μ" + "<br/>";
                result += "Series resistance: " + QString::number(Rdc * 1000, 'f', 3) + "m" + "<br/>";
                result += "Parallel resistance: " + QString::number(Q * z.imag() / 1000, 'f', fOpt->dwAccuracy) + "k" + "<br/>";
                if (isCsAuto)
                    result += "Parallel capacitance: " + QString::number(Csm, 'f', fOpt->dwAccuracy) + "p";
                else
                    result += "Parallel capacitance: " + QString::number(Cs, 'f', fOpt->dwAccuracy) + "p";
            } else {
                result += "Q &lt; 0";
            }
            result += "</p>";
            ui->label_result->setText(result);
        } else {
            showWarning(tr("Warning"), tr("One or more dimensions have an invalid value!"));
            ui->label_result->setText("");
            return;
        }
    } else {
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        ui->label_result->setText("");
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_pushButton_export_clicked()
{
    if (!ui->label_result->text().isEmpty()){
        QString sResult = "<hr>";
        if (fOpt->isShowTitle){
            sResult = "<h2>" +QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - "
                    + windowTitle() + "</h2><br/>";
        }
        if (fOpt->isInsertImage){
            sResult += "<img src=\":/images/res/T-core.png\">";
        }
        sResult += "<p><u>" + tr("Input data") + ":</u><br/>";
        sResult += ui->label_N->text() + " " + ui->lineEdit_N->text() + "<br/>";
        sResult += ui->label_f->text() + " " + ui->lineEdit_f->text() + " " + ui->label_f_m->text() + "<br/>";
        if (!isCsAuto)
            sResult += ui->label_cs->text() + " " + ui->lineEdit_cs->text() + " " + ui->label_cs_m->text() + "<br/>";
        if (ui->comboBox->currentIndex() > 0)
            sResult += "<u>" + ui->comboBox->currentText() + "</u><br/>";
        sResult += ui->label_2->text() + " " + ui->lineEdit_mu1->text() + "<br/>";
        sResult += ui->label_3->text() + " " + ui->lineEdit_mu2->text() + "<br/>";
        if (ui->comboBox_2->currentIndex() > 0)
            sResult += "<u>" + ui->comboBox_2->currentText() + "</u><br/>";
        sResult += ui->label_od->text() + " " + ui->lineEdit_od->text() + " " + ui->label_01->text() + "<br/>";
        sResult += ui->label_id->text() + " " + ui->lineEdit_id->text() + " " + ui->label_02->text() + "<br/>";
        sResult += ui->label_h->text() + " " + ui->lineEdit_h->text() + " " + ui->label_03->text() + "<br/>";
        sResult += ui->label_c->text() + " " + ui->lineEdit_c->text() + " " + ui->label_04->text() + "<br/>";
        sResult += "<hr>";
        sResult += "<p><u>" + tr("Result") + ":</u><br/>";
        sResult += ui->label_result->text();
        sResult += "</p><hr>";
        emit sendResult(sResult);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Toroid::on_pushButton_help_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/rf-toroid.html"));
}

