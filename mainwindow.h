/* mainwindow.h - header text to Coil64 - Radio frequency inductor and choke calculator
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStyle>
#include <QScreen>
#include <QSettings>
#include <QCoreApplication>
#include <QTranslator>
#include <QListWidgetItem>
#include <QString>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDate>
#include <QDesktopServices>
#include <QClipboard>
#include <QPrintDialog>
#include <QPrinter>
#include <QFileDialog>
#include <QTextDocumentWriter>
#include <QCloseEvent>
#include <QToolTip>
#include <bitset>

#include "options.h"
#include "about.h"
#include "find_permeability.h"
#include "ferrite_rod.h"
#include "meander_pcb.h"
#include "multiloop.h"
#include "loop.h"
#include "shield.h"
#include "amidon.h"
#include "aircoretoroid.h"
#include "potcore.h"
#include "ecore.h"
#include "ucore.h"
#include "al.h"
#include "crossover.h"
#include "rf_toroid.h"
#include "bandspread.h"
#include "rmcore.h"

#include "mthread_calculate.h"
#include "resolves.h"
#include "system_functions.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QTranslator *translator; //Global app translator

signals:
    void sendOpt(_OptionStruct);
    void sendStyleGUI(int);
    void sendLocale(QLocale);
public slots:
    void on_actionCheck_for_update_triggered();

private slots:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void on_textBrowser_customContextMenuRequested(const QPoint &pos);
    void on_textBrowser_textChanged();
    void on_textBrowser_anchorClicked(const QUrl &arg1);
    void on_listWidget_currentRowChanged(int currentRow);
    void on_tabWidget_currentChanged(int index);
    void on_comboBox_checkPCB_activated(int index);
    void on_comboBox_checkMLWinding_activated(int index);
    void on_pushButton_Calculate_clicked();

    void on_lineEdit_ind_editingFinished();
    void on_lineEdit_freq_editingFinished();
    void on_lineEdit_1_editingFinished();
    void on_lineEdit_2_editingFinished();
    void on_lineEdit_3_editingFinished();
    void on_lineEdit_4_editingFinished();
    void on_lineEdit_5_editingFinished();
    void on_lineEdit_6_editingFinished();
    void on_horizontalSlider_valueChanged(int value);
    void on_horizontalSlider_sliderMoved(int position);

    void on_lineEdit_N_editingFinished();
    void on_lineEdit_freq2_editingFinished();
    void on_lineEdit_1_2_editingFinished();
    void on_lineEdit_2_2_editingFinished();
    void on_lineEdit_3_2_editingFinished();
    void on_lineEdit_4_2_editingFinished();
    void on_lineEdit_5_2_editingFinished();
    void on_lineEdit_6_2_editingFinished();
    void on_lineEdit_7_2_editingFinished();

    void on_radioButton_1_clicked(bool checked);
    void on_radioButton_2_clicked(bool checked);
    void on_radioButton_3_clicked(bool checked);
    void on_radioButton_4_clicked(bool checked);
    void on_radioButton_5_1_toggled(bool checked);
    void on_radioButton_6_toggled(bool checked);
    void on_radioButton_8_toggled(bool checked);
    void on_radioButton_1_2_clicked(bool checked);
    void on_radioButton_2_2_clicked(bool checked);
    void on_radioButton_3_2_clicked(bool checked);
    void on_radioButton_4_2_clicked(bool checked);
    void on_radioButton_LC_clicked();
    void on_radioButton_CF_clicked();
    void on_radioButton_LF_clicked();
    void on_radioButton_ZF_clicked();
    void on_checkBox_isInsulation_toggled(bool checked);
    void on_checkBox_isInsulation2_toggled(bool checked);

    void get_onelayerN_roundW_Result(_CoilResult result);
    void get_onelayerN_rectW_Result(_CoilResult result);
    void get_onelayerN_Poligonal_Result(_CoilResult result);
    void get_multilayerN_Result(_CoilResult result);
    void get_multilayerNgap_Result(_CoilResult result);
    void get_multilayerN_Rect_Result(_CoilResult result);
    void get_multilayerN_Foil_Result(_CoilResult result);
    void get_ferrToroidN_Result(_CoilResult result);
    void get_pcbN_Result(_CoilResult result);
    void get_spiralN_Result(_CoilResult result);

    void get_onelayerI_roundW_Result(_CoilResult result);
    void get_onelayerI_rectW_Result(_CoilResult result);
    void get_onelayerI_Poligonal_Result(_CoilResult result);
    void get_multilayerI_Result(_CoilResult result);
    void get_multilayerIgap_Result(_CoilResult result);
    void get_multilayerI_Rect_Result(_CoilResult result);
    void get_multilayerI_Foil_Result(_CoilResult result);
    void get_ferriteI_Result(_CoilResult result);
    void get_pcbI_Result(_CoilResult result);
    void get_spiralI_Result(_CoilResult result);
    void getAddCalculationResult(QString result);

    void getOptionStruct(_OptionStruct gOpt);
    void checkAppVersion(QNetworkReply *reply);
    void setLanguage();
    void resetUiFont();
    void invertGUIconColor();
    void showOutput(QString caption, QString image, QString input, QString result);
    void checkMaterial1(Material *mt);
    void checkMaterial2(Material *mt);

    void on_actionOptions_triggered();
    void on_actionExit_triggered();
    void on_actionPrint_triggered();
    void on_actionAbout_triggered();
    void on_actionHelp_triggered();
    void on_actionIntrfaceHelp_triggered();
    void on_actionHomePage_triggered();
    void on_actionVersions_history_triggered();
    void on_actionDonate_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionCopy_triggered();
    void on_actionClear_all_triggered();
    void on_actionTo_null_data_triggered();
    void on_actionThemeDefault_triggered();
    void on_actionThemeDark_triggered();
    void on_actionThemeSunny_triggered();

    void on_toolButton_Help_clicked();
    void on_toolButton_FAQ_clicked();
    void on_toolButton_Configure_clicked();
    void on_toolButton_Clear_clicked();
    void on_toolButton_CopySel_clicked();
    void on_toolButton_CopyAll_clicked();
    void on_toolButton_Print_clicked();
    void on_toolButton_Num_clicked();
    void on_toolButton_showTitle_clicked();
    void on_toolButton_showImg_clicked();
    void on_toolButton_showAdditional_clicked();
    void on_toolButton_cbe_clicked();
    void on_toolButton_cbc_clicked();
    void on_toolButton_cdsr_clicked();
    void on_toolButton_soe_clicked();
    void on_toolButton_lShowFirst_clicked();
    void on_toolButton_Open_clicked();
    void on_toolButton_Save_clicked();
    void on_toolButton_ltspice_clicked();
    void on_toolButton_Color_clicked();
    void on_toolButton_Desc_clicked();
    void on_toolButton_column_clicked();

    void on_actionFerrite_toroid_permeability_triggered();
    void on_actionCoil_on_a_ferrite_rod_triggered();
    void on_actionPCB_meandr_coil_triggered();
    void on_actionMetal_detector_search_coil_triggered();
    void on_actionLoop_of_one_turn_triggered();
    void on_actionShiels_influence_triggered();
    void on_actionAmidon_cores_triggered();
    void on_actionAir_core_toroid_coil_triggered();
    void on_actionPot_core_coil_triggered();
    void on_actionE_core_coil_triggered();
    void on_actionU_core_coil_triggered();
    void on_actionAL_factor_calculation_triggered();
    void on_actionToroidal_inductor_at_RF_triggered();
    void on_actionCross_over_inductor_triggered();
    void on_actionBandspread_Calculator_triggered();
    void on_actionRM_core_coil_triggered();

private:
    Ui::MainWindow *mui;
    QNetworkAccessManager *net_manager;
    QDoubleValidator *dv;
    QRegExpValidator *awgV;
    QIntValidator *iv;
    _FormCoil FormCoil; //Enum variable of a coil form
    _Data *data; //Structure stores the data for calculating
    _OptionStruct *myOpt; //Options Structure contains all app settings
    QString lang; //Current app GUI language
    unsigned int calc_count; //Calculating number
    QLocale loc;
    QMenu *popupmenu;
};

#endif // MAINWINDOW_H
