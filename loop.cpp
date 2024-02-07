/* loop.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "loop.h"
#include "ui_loop.h"

Loop::Loop(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Loop)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, MAX_DOUBLE, 380);
    awgV = new QRegExpValidator(QRegExp(AWG_REG_EX));
    ui->lineEdit_1->setValidator(dv);
    ui->lineEdit_2->setValidator(dv);
    ui->lineEdit_3->setValidator(dv);
    ui->lineEdit_N->setValidator(dv);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Loop::~Loop()
{
    ind = loc.toDouble(ui->lineEdit_N->text())*fOpt->dwInductanceMultiplier;
    a = loc.toDouble(ui->lineEdit_1->text())*fOpt->dwLengthMultiplier;
    b = loc.toDouble(ui->lineEdit_2->text())*fOpt->dwLengthMultiplier;
    if (fOpt->isAWG){
        dw = convertfromAWG(ui->lineEdit_3->text());
    } else {
        dw = loc.toDouble(ui->lineEdit_3->text())*fOpt->dwLengthMultiplier;
    }
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Loop" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", size());
    settings->setValue("ind", ind);
    settings->setValue("a", a);
    settings->setValue("b", b);
    settings->setValue("d", dw);
    settings->setValue("isReverse", ui->checkBox_isReverce->isChecked());
    settings->setValue("loopKind", loopKind);
    settings->endGroup();
    delete settings;
    delete fOpt;
    delete awgV;
    delete dv;
    delete ui;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Loop::getCurrentLocale(QLocale locale){
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Loop::getOpt(_OptionStruct gOpt){
    *fOpt = gOpt;
    ui->label_N_m->setText(qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8()));
    ui->label_01->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_02->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_03->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Loop" );
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    bool isReverse = settings->value("isReverse", false).toBool();
    loopKind = settings->value("loopKind", 0).toInt();
    ind = settings->value("ind", 0).toDouble();
    a = settings->value("a", 0).toDouble();
    b = settings->value("b", 0).toDouble();
    dw = settings->value("d", 0).toDouble();
    settings->endGroup();
    if (fOpt->isAWG){
        ui->label_03->setText(tr("AWG"));
        ui->lineEdit_3->setValidator(awgV);
        if (dw > 0){
            ui->lineEdit_3->setText(converttoAWG(dw));
        } else
            ui->lineEdit_3->setText("");
    } else
        ui->lineEdit_3->setText(roundTo(dw / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    resize(size);
    move(pos);
    ui->checkBox_isReverce->setChecked(isReverse);
    QString tmpTxt = tr("Wire diameter") + " d:";
    ui->label_3->setText(tmpTxt);
    on_checkBox_isReverce_clicked();
    delete settings;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Loop::on_radioButton_round_clicked()
{
    loopKind = 0;
    ui->image->setPixmap(QPixmap(":/images/res/loop0.png"));
    ui->label_02->setVisible(false);
    ui->label_2->setVisible(false);
    ui->lineEdit_2->setVisible(false);
    if (ui->checkBox_isReverce->isChecked()){
        ui->label_01->setVisible(false);
        ui->label_1->setVisible(false);
        ui->lineEdit_1->setVisible(false);
        ui->lineEdit_N->setFocus();
        ui->lineEdit_N->selectAll();
    } else {
        ui->label_01->setVisible(true);
        ui->label_1->setVisible(true);
        ui->lineEdit_1->setVisible(true);
        ui->lineEdit_1->setText(roundTo(a / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
        QString tmpTxt = tr("Loop diameter") + " D:";
        ui->label_1->setText(tmpTxt);
        ui->lineEdit_1->setFocus();
        ui->lineEdit_1->selectAll();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Loop::on_radioButton_triangle_clicked()
{
    loopKind = 1;
    ui->image->setPixmap(QPixmap(":/images/res/loop3.png"));
    if (ui->checkBox_isReverce->isChecked()){
        ui->label_01->setVisible(false);
        ui->label_1->setVisible(false);
        ui->lineEdit_1->setVisible(false);
        ui->label_02->setVisible(false);
        ui->label_2->setVisible(false);
        ui->lineEdit_2->setVisible(false);
        ui->lineEdit_N->setFocus();
        ui->lineEdit_N->selectAll();
    } else {
        ui->label_01->setVisible(true);
        ui->label_1->setVisible(true);
        ui->lineEdit_1->setVisible(true);
        ui->lineEdit_1->setText(roundTo(a / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
        QString tmpTxt = tr("Side of triangle") + " a:";
        ui->label_1->setText(tmpTxt);
        ui->label_02->setVisible(true);
        ui->label_2->setVisible(true);
        ui->lineEdit_2->setVisible(true);
        ui->lineEdit_2->setText(roundTo(b / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
        tmpTxt = tr("Side of triangle") + " b:";
        ui->label_2->setText(tmpTxt);
        ui->lineEdit_1->setFocus();
        ui->lineEdit_1->selectAll();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Loop::on_radioButton_rectangle_clicked()
{
    loopKind = 2;
    ui->image->setPixmap(QPixmap(":/images/res/loop4.png"));
    if (ui->checkBox_isReverce->isChecked()){
        ui->label_01->setVisible(false);
        ui->label_1->setVisible(false);
        ui->lineEdit_1->setVisible(false);
        ui->label_02->setVisible(false);
        ui->label_2->setVisible(false);
        ui->lineEdit_2->setVisible(false);
        ui->lineEdit_N->setFocus();
        ui->lineEdit_N->selectAll();
    } else {
        ui->label_01->setVisible(true);
        ui->label_1->setVisible(true);
        ui->lineEdit_1->setVisible(true);
        ui->lineEdit_1->setText(roundTo(a / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
        QString tmpTxt = tr("Side of rectangle") + " a:";
        ui->label_1->setText(tmpTxt);
        ui->label_02->setVisible(true);
        ui->label_2->setVisible(true);
        ui->lineEdit_2->setVisible(true);
        ui->lineEdit_2->setText(roundTo(b / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
        tmpTxt = tr("Side of rectangle") + " b:";
        ui->label_2->setText(tmpTxt);
        ui->lineEdit_1->setFocus();
        ui->lineEdit_1->selectAll();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Loop::on_checkBox_isReverce_clicked()
{
    if (ui->checkBox_isReverce->isChecked()){
        ui->label_N->setVisible(true);
        ui->lineEdit_N->setText(roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy));
        ui->label_N_m->setVisible(true);
        ui->lineEdit_N->setVisible(true);
        QString tmpTxt = tr("Inductance") + " L:";
        ui->label_N->setText(tmpTxt);
    } else {
        ui->label_N->setVisible(false);
        ui->label_N_m->setVisible(false);
        ui->lineEdit_N->setVisible(false);
    }
    switch (loopKind) {
    case 0:{
        ui->radioButton_round->setChecked(true);
        on_radioButton_round_clicked();
        break;
    }
    case 1:{
        ui->radioButton_triangle->setChecked(true);
        on_radioButton_triangle_clicked();
        break;
    }
    case 2:{
        ui->radioButton_rectangle->setChecked(true);
        on_radioButton_rectangle_clicked();
        break;
    }
    default:
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Loop::on_pushButton_2_clicked()
{
    this->close();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Loop::on_pushButton_3_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/one-turn-loop-coil.html"));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Loop::on_pushButton_clicked()
{
    QString sCaption = QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle();
    QString sImage = "";
    QString sInput = "";
    QString sResult = "";
    if(loopKind == 0){
        sCaption += " (" + ui->radioButton_round->text() +  + ")";
        sImage = "<img src=\":/images/res/loop0.png\">";
    } else if (loopKind == 1){
        sCaption += " (" + ui->radioButton_triangle->text() +  + ")";
        sImage = "<img src=\":/images/res/loop3.png\">";
    } else if (loopKind == 2){
        sCaption += " (" + ui->radioButton_rectangle->text() +  + ")";
        sImage = "<img src=\":/images/res/loop4.png\">";
    }
    if (ui->checkBox_isReverce->isChecked()){
        if ((ui->lineEdit_N->text().isEmpty())||(ui->lineEdit_3->text().isEmpty())){
            showWarning(tr("Warning"), tr("One or more inputs are empty!"));
            return;
        }
        bool ok1, ok3;
        ind = loc.toDouble(ui->lineEdit_N->text(), &ok1)*fOpt->dwInductanceMultiplier;
        if (fOpt->isAWG){
            dw = convertfromAWG(ui->lineEdit_3->text(), &ok3);
        } else {
            dw = loc.toDouble(ui->lineEdit_3->text(), &ok3)*fOpt->dwLengthMultiplier;
        }
        if((!ok1)||(!ok3)){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if ((ind == 0)||(dw == 0)){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        sInput = "<p><u>" + tr("Input data") + ":</u><br/>";
        sInput += formattedOutput(fOpt, ui->label_N->text(), ui->lineEdit_N->text(), ui->label_N_m->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_3->text(), ui->lineEdit_3->text(), ui->label_03->text()) + "</p>";
        sResult = "<p><u>" + tr("Result") + ":</u><br/>";
        if(loopKind == 0){
            a = findRoundLoop_D(ind, dw);
            sResult += formattedOutput(fOpt, tr("Loop diameter") + " D: ", roundTo(a / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy),
                                       qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
        } else if (loopKind == 1){
            a = findIsoIsoscelesTriangleLoop_a(ind, dw);
            b = a;
            sResult += formattedOutput(fOpt, tr("The side of the equilateral triangle") + " a = b: ", roundTo(a / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy),
                                       qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
        } else if (loopKind == 2){
            a = findRectangleLoop_a(ind, dw);
            b = a;
            sResult += formattedOutput(fOpt, tr("The side of quadrate") + " a = b: ", roundTo(a / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy),
                                       qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
        }
        if (a < 0){
            a = 0;
            b = 0;
            showWarning(tr("Warning"),tr("Coil can not be realized") + "!");
            return;
        }
    } else {
        if ((ui->lineEdit_1->text().isEmpty())||(ui->lineEdit_3->text().isEmpty())){
            showWarning(tr("Warning"), tr("One or more inputs are empty!"));
            return;
        }
        bool ok1, ok2, ok3;
        a = loc.toDouble(ui->lineEdit_1->text(), &ok1)*fOpt->dwLengthMultiplier;
        if (fOpt->isAWG){
            dw = convertfromAWG(ui->lineEdit_3->text(), &ok3);
        } else {
            dw = loc.toDouble(ui->lineEdit_3->text(), &ok3)*fOpt->dwLengthMultiplier;
        }
        if((!ok1)||(!ok3)){
            showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            return;
        }
        if ((a == 0)||(dw == 0)){
            showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
            return;
        }
        if ((loopKind == 1) || (loopKind == 2)){
            if (ui->lineEdit_2->text().isEmpty()){
                showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                return;
            }
            b = loc.toDouble(ui->lineEdit_2->text(), &ok2)*fOpt->dwLengthMultiplier;
            if(!ok2){
                showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                return;
            }
            if (b == 0){
                showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                return;
            }
        }
        sInput = "<p><u>" + tr("Dimensions") + ":</u><br/>";
        sInput += formattedOutput(fOpt, ui->label_1->text(), ui->lineEdit_1->text(), ui->label_01->text()) + "<br/>";
        if (loopKind != 0)
            sInput += formattedOutput(fOpt, ui->label_2->text(), ui->lineEdit_2->text(), ui->label_02->text()) + "<br/>";
        sInput += formattedOutput(fOpt, ui->label_3->text(), ui->lineEdit_3->text(), ui->label_03->text()) + "</p>";
        if(loopKind == 0){
            ind = findRoundLoop_I(a, dw);
        } else if (loopKind == 1){
            ind = findIsoIsoscelesTriangleLoop_I(a, b, dw);
        } else if (loopKind == 2){
            ind = findRectangleLoop_I(a, b, dw);
        }
        sResult = "<p><u>" + tr("Result") + ":</u><br/>";
        sResult += formattedOutput(fOpt, tr("Inductance") + " L = ", roundTo(ind / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                   qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
    }
    sResult += "</p>";
    emit sendResult(sCaption + LIST_SEPARATOR + sImage + LIST_SEPARATOR + sInput + LIST_SEPARATOR + sResult);
}

