/* options.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "options.h"
#include "ui_options.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Options::Options(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Options)
{
    ui->setupUi(this);
    qRegisterMetaType<_OptionStruct>();
    oOpt = new _OptionStruct;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Options::~Options()
{
    delete oOpt;
    delete ui;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Options::getOpt(_OptionStruct gOpt){
    ui->tabWidget->setCurrentIndex(0);
    *oOpt = gOpt;
    ui->spinBox->setValue(oOpt->dwAccuracy);
    ui->comboBox_capacitance->setCurrentIndex(oOpt->indexCapacityMultiplier);
    ui->comboBox_frequency->setCurrentIndex(oOpt->indexFrequencyMultiplier);
    ui->comboBox_inductance->setCurrentIndex(oOpt->indexInductanceMultiplier);
    ui->comboBox_length->setCurrentIndex(oOpt->indexLengthMultiplier);

    ui->comboBox_wire->setCurrentIndex(oOpt->isAWG);
    ui->checkBox->setChecked(oOpt->isInsertImage);
    ui->checkBox_2->setChecked(oOpt->isAdditionalResult);
    ui->checkBox_4->setChecked(oOpt->isConfirmExit);
    ui->checkBox_5->setChecked(oOpt->isConfirmClear);

    ui->fontComboBox_1->setCurrentText(oOpt->mainFontFamily);
    ui->fontspinBox_1->setValue(oOpt->mainFontSize);
    ui->fontComboBox_2->setCurrentText(oOpt->textFontFamily);
    ui->fontspinBox_2->setValue(oOpt->textFontSize);

    ui->radioButton_loc2->setChecked(oOpt->isEnglishLocale);

    ui->checkBox_3->setChecked(oOpt->isAutomaticUpdate);
    switch (oOpt->upDateInterval) {
    case 60:{
        ui->radioButton_1->setChecked(true);
        break;
    }
    case 120:{
        ui->radioButton_2->setChecked(true);
        break;
    }
    case 180:{
        ui->radioButton_3->setChecked(true);
        break;
    }
    default:
        break;
    }
    QFont f1 = this->font();
    f1.setFamily(oOpt->mainFontFamily);
    f1.setPixelSize(oOpt->mainFontSize);
    this->setFont(f1);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Options::on_pushButton_clicked()
{
    ((MainWindow *) parent())->on_actionCheck_for_update_triggered();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Options::on_pushButton_2_clicked()
{
    oOpt->dwAccuracy = ui->spinBox->value();
    oOpt->indexCapacityMultiplier = ui->comboBox_capacitance->currentIndex();
    oOpt->indexFrequencyMultiplier = ui->comboBox_frequency->currentIndex();
    oOpt->indexInductanceMultiplier = ui->comboBox_inductance->currentIndex();
    oOpt->indexLengthMultiplier = ui->comboBox_length->currentIndex();

    oOpt->isAWG = ui->comboBox_wire->currentIndex();
    oOpt->isInsertImage = ui->checkBox->isChecked();
    oOpt->isAdditionalResult = ui->checkBox_2->isChecked();
    oOpt->isConfirmExit = ui->checkBox_4->isChecked();
    oOpt->isConfirmClear = ui->checkBox_5->isChecked();

    oOpt->mainFontFamily = ui->fontComboBox_1->currentText();
    oOpt->textFontFamily = ui->fontComboBox_2->currentText();
    oOpt->mainFontSize = ui->fontspinBox_1->value();
    oOpt->textFontSize = ui->fontspinBox_2->value();

    oOpt->isEnglishLocale = ui->radioButton_loc2->isChecked();

    oOpt->isAutomaticUpdate = ui->checkBox_3->isChecked();
    if (ui->radioButton_1->isChecked()){
        oOpt->upDateInterval = 60;
    } else if (ui->radioButton_2->isChecked()){
        oOpt->upDateInterval = 120;
    } else {
        oOpt->upDateInterval = 180;
    }
    emit sendOpt(*oOpt);
    this->close();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Options::on_pushButton_3_clicked()
{
    this->close();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Options::on_pushButton_4_clicked()
{
    ui->spinBox->setValue(3);
    ui->comboBox_capacitance->setCurrentIndex(0);
    ui->comboBox_frequency->setCurrentIndex(2);
    ui->comboBox_inductance->setCurrentIndex(1);
    ui->comboBox_length->setCurrentIndex(0);
    ui->comboBox_wire->setCurrentIndex(0);
    ui->checkBox->setChecked(true);
    ui->checkBox_2->setChecked(true);
    ui->radioButton_loc1->setChecked(true);
}
