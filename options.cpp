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
#include "system_functions.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Options::Options(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Options)
{
    ui->setupUi(this);
    qRegisterMetaType<_OptionStruct>();
    oOpt = new _OptionStruct;
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Options" );
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    settings->endGroup();
    resize(size);
    move(pos);
    delete settings;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Options::~Options()
{
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Options" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", size());
    settings->endGroup();
    delete settings;
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
    ui->checkBox_6->setChecked(oOpt->isConfirmDelete);
    ui->checkBox_7->setChecked(oOpt->isSaveOnExit);
    ui->checkBox_8->setChecked(oOpt->isLastShowingFirst);
    ui->checkBox_9->setChecked(oOpt->isShowTitle);
    ui->checkBox_10->setChecked(oOpt->isShowLTSpice);
    ui->checkBox_11->setChecked(oOpt->isOutputValueColored);
    ui->checkBox_12->setChecked(oOpt->isShowValueDescription);
    ui->checkBox_13->setChecked(oOpt->isOutputInTwoColumns);
    ui->checkBox_14->setChecked(oOpt->isShowCalcNum);

    ui->fontComboBox_1->setCurrentText(oOpt->mainFontFamily);
    ui->fontspinBox_1->setValue(oOpt->mainFontSize);
    ui->fontComboBox_2->setCurrentText(oOpt->textFontFamily);
    ui->fontspinBox_2->setValue(oOpt->textFontSize);
    ui->radioButton_loc2->setChecked(oOpt->isEnglishLocale);
    ui->comboBox_style->setCurrentIndex(oOpt->styleGUI);

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

    QComboBox *comboBox = ui->comboBox;

    colorNames = getValueTextColorNames(oOpt->styleGUI);
    comboBox->setFocusPolicy(Qt::NoFocus);
    int sze = comboBox ->style()->pixelMetric(QStyle::PM_IconViewIconSize);
    QPixmap pixmap(sze, sze - 2);
    foreach (const QString &colorName, colorNames) {
        pixmap.fill(QColor(colorName));
        QIcon *ico = new QIcon(pixmap);
        comboBox ->addItem(*ico, QString(), colorName);
    }
    ui->comboBox->setCurrentIndex(oOpt->vTextColor);
    if (oOpt->styleGUI == _DarkStyle){
        ui->pushButton->setIcon(reverceIconColors(ui->pushButton->icon()));
        ui->pushButton_2->setIcon(reverceIconColors(ui->pushButton_2->icon()));
        ui->pushButton_3->setIcon(reverceIconColors(ui->pushButton_3->icon()));
        ui->pushButton_4->setIcon(reverceIconColors(ui->pushButton_4->icon()));
    }
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
    oOpt->isConfirmDelete = ui->checkBox_6->isChecked();
    oOpt->isSaveOnExit = ui->checkBox_7->isChecked();
    oOpt->isLastShowingFirst = ui->checkBox_8->isChecked();
    oOpt->isShowTitle = ui->checkBox_9->isChecked();
    oOpt->isShowLTSpice = ui->checkBox_10->isChecked();
    oOpt->isOutputValueColored = ui->checkBox_11->isChecked();
    oOpt->isShowValueDescription = ui->checkBox_12->isChecked();
    oOpt->isOutputInTwoColumns = ui->checkBox_13->isChecked();
    oOpt->isShowCalcNum = ui->checkBox_14->isChecked();
    oOpt->styleGUI = ui->comboBox_style->currentIndex();
    oOpt->vTextColor = ui->comboBox->currentIndex();


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
    ui->checkBox_4->setChecked(true);
    ui->checkBox_5->setChecked(true);
    ui->checkBox_6->setChecked(true);
    ui->checkBox_7->setChecked(true);
    ui->checkBox_8->setChecked(true);
    ui->checkBox_9->setChecked(true);
    ui->checkBox_10->setChecked(true);
    ui->checkBox_11->setChecked(false);
    ui->checkBox_12->setChecked(true);
    ui->checkBox_13->setChecked(false);
    ui->checkBox_14->setChecked(true);
    ui->comboBox->setCurrentIndex(0);
    ui->radioButton_loc1->setChecked(true);
}
