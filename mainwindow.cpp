/* mainwindow.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "mainwindow.h"
#include "ui_mainwindow.h"



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mui(new Ui::MainWindow)
{
    mui->setupUi(this);
    net_manager = new QNetworkAccessManager();
    connect(net_manager, SIGNAL(finished(QNetworkReply*)), SLOT(checkAppVersion(QNetworkReply*)));

    QString title = qApp->applicationName();
    title.append(" v");
    title.append(qApp->applicationVersion());
    setWindowTitle(title);
    translator = new QTranslator(qApp);
    this->calc_count = 0;

    mui->groupBox->setVisible(false);
    mui->groupBox_2->setVisible(false);
    mui->groupBox_6->setVisible(false);
    mui->groupBox_7->setVisible(false);
    data = new _Data;
    myOpt = new _OptionStruct;

#if defined(Q_OS_MAC) || (Q_WS_X11) || defined(Q_OS_LINUX)
    QSettings *settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#elif defined(Q_WS_WIN) || defined(Q_OS_WIN)
    QSettings *settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#else
    QSettings *settings = new QSettings(QDir::currentPath() + "/Coil64.conf", QSettings::IniFormat);
#endif
    settings->beginGroup( "GUI" );
    int tab = settings->value( "tab", 0 ).toInt();
    lang = settings->value("lang", QString::fromLatin1(QLocale::system().name().toLatin1())).toString();
    if (lang.length() > 2)
        lang.truncate(lang.lastIndexOf('_'));
    int form_coil = settings->value( "form_coil", -1 ).toInt();
    QPoint pos = settings->value("pos", QPoint(100, 100)).toPoint();
    QSize size = settings->value("size", QSize(100, 100)).toSize();
    int init_cond = settings->value( "init_cond", 0 ).toInt();
    int wire_material = settings->value( "wire_material", 0 ).toInt();
    int init_data = settings->value( "init_data", 0 ).toInt();
    myOpt->isInsertImage = settings->value( "isInsertImage", true ).toBool();
    myOpt->isAdditionalResult = settings->value( "isAdditionalResult", true ).toBool();
    myOpt->mainFontFamily = settings->value("MainFontFamily", QFontInfo(QFont()).family()).toString();
    myOpt->mainFontSize = settings->value("MainFontSize", QFontInfo(QFont()).pixelSize()).toInt();
    myOpt->textFontFamily = settings->value("TextFontFamily", QFontInfo(QFont()).family()).toString();
    myOpt->textFontSize = settings->value("TextFontSize", QFontInfo(QFont()).pixelSize()).toInt();
    settings->endGroup();

    translator->load(":/lang/res/translations/Coil64_" + lang);
    qApp->installTranslator(translator);

    //Start add language menu group if additional languages are available
    QDir dir(":/lang/res/translations");
    QFileInfoList dirContent = dir.entryInfoList(QStringList() << "*.qm",QDir::Files);
    if (!dirContent.isEmpty()){
        QStringList langName;
        QStringList langList = translateInstalling(&langName);
        QString installLang;
        QString fileStr;
        QList<QMenu*> menus = this->menuBar()->findChildren<QMenu*>();
        QMenu* child = menus.takeAt(1);
        child->addSeparator();
        QMenu* submenuLanguage = child->addMenu(tr("Language"));
        QActionGroup *langGroup = new QActionGroup(submenuLanguage);
        langGroup->setExclusive(true);
        for (int i = 0; i < langList.size(); ++i){
            installLang = langList[i];
            for (int j = 0; j < dirContent.size(); ++j) {
                fileStr = dirContent.at(j).fileName().mid(dirContent.at(j).fileName().indexOf("_") + 1,2);
                if (installLang == fileStr) {
                    QString icoPath = ":/flags/res/flags/" + installLang + ".png";
                    QIcon ico = QIcon(QPixmap(icoPath));
                    QAction *mAction = submenuLanguage->addAction(ico,langName[i]);
                    langGroup->addAction(mAction);
                    mAction->setCheckable(true);
                    if (fileStr == lang){
                        mAction->setChecked(true);
                    }
                    mAction->setObjectName(fileStr);
                    connect(mAction, SIGNAL(triggered()), this, SLOT(setLanguage()));
                }
            }
        }
    }
    //End add language menu group if additional languages are available

    mui->toolButton_showImg->setChecked(myOpt->isInsertImage);
    mui->toolButton_showAdditional->setChecked(myOpt->isAdditionalResult);

    settings->beginGroup("Measure_Units");
    myOpt->dwAccuracy = settings->value("Accuracy", 3).toUInt();
    myOpt->isAWG = settings->value("isAWG", false).toBool();
    myOpt->indexCapacityMultiplier = settings->value("Capacity_Unit", 0).toInt();
    myOpt->indexInductanceMultiplier = settings->value("Inductance_Unit", 1).toInt();
    myOpt->indexFrequencyMultiplier = settings->value("Frequency_Unit", 2).toInt();
    myOpt->indexLengthMultiplier = settings->value("Length_Unit", 0).toInt();
    settings->endGroup();

    settings->beginGroup("UpdateSettings");
    myOpt->isAutomaticUpdate = settings->value("isAutomaticUpdate",true).toBool();
    myOpt->upDateInterval = settings->value("upDateInterval",60).toInt();
    QDate date;
    myOpt->firstDate = QDate::fromString(settings->value("firstDate", date.currentDate().toString("dd.MM.yyyy")).toString(),"dd.MM.yyyy");
    settings->endGroup();

    settings->beginGroup("Data");
    data->N = settings->value("N", 0).toDouble();
    data->capacitance = settings->value("capacitance", 0).toDouble();
    data->frequency = settings->value("frequency", 0).toDouble();
    data->inductance = settings->value("inductance", 0).toDouble();
    data->D = settings->value("D", 0).toDouble();
    data->d = settings->value("dw", 0).toDouble();
    data->k = settings->value("k", 0).toDouble();
    data->p = settings->value("p", 0).toDouble();
    data->t = settings->value("t", 0).toDouble();
    data->w = settings->value("w", 0).toDouble();
    data->isol = settings->value("i", 0).toDouble();
    data->l = settings->value("l", 0).toDouble();
    data->c = settings->value("c", 0).toDouble();
    data->g = settings->value("g", 0).toDouble();
    data->Ng = settings->value("Ng", 0).toDouble();
    data->Do = settings->value("Do", 0).toDouble();
    data->Di = settings->value("Di", 0).toDouble();
    data->h = settings->value("h", 0).toDouble();
    data->mu = settings->value("mu", 0).toDouble();
    data->ratio = settings->value("ratio",0.5).toDouble();
    data->s = settings->value("s", 0).toDouble();
    data->Rdc = settings->value("Rdc", 0).toDouble();
    settings->endGroup();


    if (myOpt->isAutomaticUpdate){
        switch (myOpt->upDateInterval) {
        case 60:{
            if ((date.currentDate().toJulianDay() - myOpt->firstDate.toJulianDay()) > 60){
                on_actionCheck_for_update_triggered();
            }
            break;
        }
        case 120:{
            if ((date.currentDate().toJulianDay() - myOpt->firstDate.toJulianDay()) > 120){
                on_actionCheck_for_update_triggered();
            }
            break;
        }
        case 180:{
            if ((date.currentDate().toJulianDay() - myOpt->firstDate.toJulianDay()) > 180){
                on_actionCheck_for_update_triggered();
            }
            break;
        }
        default:
            break;
        }
    }

    if (tab > 0){
        mui->tabWidget->setCurrentIndex(tab);
    } else {
        mui->tabWidget->setCurrentIndex(0);
    }
    if (form_coil > 0){
        FormCoil = (_FormCoil)form_coil;
    } else {
        FormCoil = _Onelayer;
    }

    if (init_cond == 0){
        mui->radioButton_LC->setChecked(true);
        on_radioButton_LC_clicked();
    }
    else if (init_cond == 1){
        mui->radioButton_CF->setChecked(true);
        on_radioButton_CF_clicked();
    }
    else if (init_cond == 2){
        mui->radioButton_LF->setChecked(true);
        on_radioButton_LF_clicked();
    }
    if (wire_material == 0){
        mui->radioButton_1->setChecked(true);
        mui->radioButton_1_2->setChecked(true);
    }
    else if (wire_material == 1){
        mui->radioButton_2->setChecked(true);
        mui->radioButton_2_2->setChecked(true);
    }
    else if (wire_material == 2){
        mui->radioButton_3->setChecked(true);
        mui->radioButton_3_2->setChecked(true);
    }
    else if (wire_material == 3){
        mui->radioButton_4->setChecked(true);
        mui->radioButton_4_2->setChecked(true);
    }
    if (init_data == 0) mui->radioButton_7->setChecked(true);
    if (init_data == 1) mui->radioButton_8->setChecked(true);

    //Start Allow only float values in input fields
    QDoubleValidator *dv = new QDoubleValidator(0.0, 1e18, 5);
    QLocale locale(QLocale::English);
    dv->setLocale(locale);
    mui->lineEdit_1->setValidator(dv);
    mui->lineEdit_2->setValidator(dv);
    mui->lineEdit_3->setValidator(dv);
    mui->lineEdit_4->setValidator(dv);
    mui->lineEdit_5->setValidator(dv);
    mui->lineEdit_6->setValidator(dv);
    mui->lineEdit_freq->setValidator(dv);
    mui->lineEdit_ind->setValidator(dv);
    //End Allow only float values in input fields

    resetUiFont();
    completeOptionsStructure(myOpt);

    on_tabWidget_currentChanged(tab);
    resize(size);
    move(pos);
    mui->retranslateUi(this);
    delete settings;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::setLanguage(){
    QString slang = sender()->objectName();
    lang = slang;
    translator->load(":/lang/res/translations/Coil64_" + lang);
    qApp->installTranslator(translator);
    mui->retranslateUi(this);
    QList<QMenu*> menus = this->menuBar()->findChildren<QMenu*>();
    QList<QAction*> actions = menus.takeAt(1)->actions();
    QAction* child;
    if (actions.count() > 1){
        child = actions.at(2);
        child->setText(tr("Language"));
    }
    int tab = mui->tabWidget->currentIndex();
    on_tabWidget_currentChanged(tab);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
{
#if defined(Q_OS_MAC) || (Q_WS_X11) || defined(Q_OS_LINUX)
    QSettings *settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#elif defined(Q_WS_WIN) || defined(Q_OS_WIN)
    QSettings *settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#else
    QSettings *settings = new QSettings(QDir::currentPath() + "/Coil64.conf", QSettings::IniFormat);
#endif
    settings->beginGroup( "GUI" );
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->setValue( "tab", mui->tabWidget->currentIndex() );
    settings->setValue( "form_coil", FormCoil);
    int init_cond = 0;
    if (mui->radioButton_CF->isChecked())
        init_cond = 1;
    if (mui->radioButton_LF->isChecked())
        init_cond = 2;
    settings->setValue("lang",this->lang);
    settings->setValue( "init_cond", init_cond);
    if (mui->radioButton_1->isChecked()) settings->setValue("wire_material", 0);
    if (mui->radioButton_2->isChecked()) settings->setValue("wire_material", 1);
    if (mui->radioButton_3->isChecked()) settings->setValue("wire_material", 2);
    if (mui->radioButton_4->isChecked()) settings->setValue("wire_material", 3);
    if (mui->radioButton_7->isChecked()) settings->setValue("init_data", 0);
    if (mui->radioButton_8->isChecked()) settings->setValue("init_data", 1);
    settings->setValue("isInsertImage", myOpt->isInsertImage);
    settings->setValue("isAdditionalResult", myOpt->isAdditionalResult);
    settings->setValue( "MainFontFamily", myOpt->mainFontFamily);
    settings->setValue( "MainFontSize", myOpt->mainFontSize);
    settings->setValue( "TextFontFamily", myOpt->textFontFamily);
    settings->setValue( "TextFontSize", myOpt->textFontSize);
    settings->endGroup();

    settings->beginGroup( "Measure_Units" );
    settings->setValue("Accuracy", myOpt->dwAccuracy);
    settings->setValue("isAWG", myOpt->isAWG);
    settings->setValue("Capacity_Unit", myOpt->indexCapacityMultiplier);
    settings->setValue("Inductance_Unit", myOpt->indexInductanceMultiplier);
    settings->setValue("Frequency_Unit", myOpt->indexFrequencyMultiplier);
    settings->setValue("Length_Unit", myOpt->indexLengthMultiplier);
    settings->endGroup();

    settings->beginGroup("UpdateSettings");
    settings->setValue("isAutomaticUpdate",myOpt->isAutomaticUpdate);
    settings->setValue("upDateInterval",myOpt->upDateInterval);
    settings->setValue("firstDate", myOpt->firstDate.toString("dd.MM.yyyy"));
    settings->endGroup();

    settings->beginGroup("Data");
    settings->setValue("N", data->N);
    settings->setValue("capacitance", data->capacitance);
    settings->setValue("inductance", data->inductance);
    settings->setValue("frequency", data->frequency);
    settings->setValue("D", data->D);
    settings->setValue("dw", data->d);
    settings->setValue("k", data->k);
    settings->setValue("p", data->p);
    settings->setValue("t", data->t);
    settings->setValue("w", data->w);
    settings->setValue("i", data->isol);
    settings->setValue("l", data->l);
    settings->setValue("c", data->c);
    settings->setValue("g", data->g);
    settings->setValue("Ng", data->Ng);
    settings->setValue("Do", data->Do);
    settings->setValue("Di", data->Di);
    settings->setValue("h", data->h);
    settings->setValue("mu", data->mu);
    settings->setValue("ratio", data->ratio);
    settings->setValue("s", data->s);
    settings->setValue("Rdc", data->Rdc);
    settings->endGroup();

    delete data;
    delete myOpt;
    delete net_manager;
    delete settings;
    delete mui;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::closeEvent(QCloseEvent *){
    QApplication::closeAllWindows();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::resetUiFont(){
    QFont f1 = this->font();
    f1.setFamily(myOpt->mainFontFamily);
    f1.setPixelSize(myOpt->mainFontSize);
    this->setFont(f1);
    QList<QAction*> actions;
    QAction* child;
    QList<QMenu*> menus = this->menuBar()->findChildren<QMenu*>();
    for (int i = 0; i < menus.count(); i++){
      actions.clear();
      actions = menus.at(i)->actions();
      for (int j = 0; j < actions.count(); j++){
          child = actions.at(j);
          child->setFont(f1);
      }
    }
    QFont f2 = mui->textBrowser->font();
    f2.setFamily(myOpt->textFontFamily);
    f2.setPixelSize(myOpt->textFontSize);
    mui->textBrowser->setFont(f2);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::checkAppVersion(QNetworkReply *reply){
    if (reply->error()){
        showWarning(tr("Warning"), tr("Error. Can not open URL: ") + qApp->organizationDomain());
    } else {
        bool isRequirestUpdate = false;
        QString internetVersiontext = reply->readAll();
        QStringList internetVersion = internetVersiontext.split(QRegExp("\\."), QString::SkipEmptyParts);
        QString str = "%1";
        QString localVersiontext = str.arg(qApp->applicationVersion());
        QStringList localVersion = localVersiontext.split(QRegExp("\\."), QString::SkipEmptyParts);
        QString iMajor = internetVersion[0];
        QString iMinor = internetVersion[1];
        QString iRelease = internetVersion[2];
        QString lMajor = localVersion[0];
        QString lMinor = localVersion[1];
        QString lRelease = localVersion[2];
        if (iMajor.toInt() > lMajor.toInt()){
            isRequirestUpdate = true;
        } else if ((iMajor.toInt() == lMajor.toInt()) && (iMinor.toInt() > lMinor.toInt())){
            isRequirestUpdate = true;
        } else if ((iMajor.toInt() == lMajor.toInt()) && (iMinor.toInt() == lMinor.toInt()) && (iRelease.toInt() > lRelease.toInt())){
            isRequirestUpdate = true;
        } else if ((iRelease.toInt() == lRelease.toInt()) && (iMinor.toInt() == lMinor.toInt()) && (iMajor.toInt() == lMajor.toInt())){
            showInfo(tr("Information"), tr("The version is up to date: ") + internetVersiontext);
        }
        if (isRequirestUpdate){
            QString message = "<p align='center'>" + tr("There is a new version of the app.") + "<br/>" + internetVersiontext +
                    "<br/>" + tr("Please visit the URL to download") + ": <br/> <a href=\"" + qApp->organizationDomain() +"\">"
                    + qApp->organizationDomain() + "</a></p>";
            showInfo(tr("Information"), message);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionPrint_triggered()
{
    QTextDocument *document = mui->textBrowser->document();
    if (!document->isEmpty()){
        QPrinter printer;
        QPrintDialog *dlg = new QPrintDialog(&printer, this);
        if (dlg->exec() != QDialog::Accepted)
            return;
        document->print(&printer);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionCheck_for_update_triggered()
{
    QString osVersion = getOSVersion();
    QString str1 = "%1/%2";
    QString c_url = str1.arg(qApp->organizationDomain()).arg(QCoreApplication::organizationName());
    QString str2 = "%1/%2/%3";
    QString user_agent = str2.arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion()).arg(osVersion);
    QNetworkRequest myReq;
    myReq.setRawHeader("User-Agent", user_agent.toLatin1().data());
    myReq.setUrl(QUrl(c_url.toLatin1().data()));
    net_manager->get(myReq);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionHelp_triggered()
{
    int tab = mui->tabWidget->currentIndex();
    if ((tab == 0) || (tab == 1)){
        switch (FormCoil) {
        case _Onelayer:{
            QDesktopServices::openUrl(QUrl("http://coil32.net/coil-with-winding-pitch.html"));
            break;
        }
        case _Onelayer_p:{
            QDesktopServices::openUrl(QUrl("http://coil32.net/coil-with-winding-pitch.html"));
            break;
        }
        case _Multilayer:{
            QDesktopServices::openUrl(QUrl("http://coil32.net/multi-layer-coil.html"));
            break;
        }
        case _Multilayer_p:{
            QDesktopServices::openUrl(QUrl("http://coil32.net/multi-layer-coil.html"));
            break;
        }
        case _FerrToroid:{
            QDesktopServices::openUrl(QUrl("http://coil32.net/ferrite-toroid-core.html"));
            break;
        }
        case _PCB_square:{
            QDesktopServices::openUrl(QUrl("http://coil32.net/pcb-coil.html"));
            break;
        }
        case _Flat_Spiral:{
            //            QDesktopServices::openUrl(QUrl("http://coil32.net/pcb-coil.html"));
            break;
        }
        default:
            break;
        }
    } else if (tab == 2){
        QDesktopServices::openUrl(QUrl("http://coil32.net/lc-resonance-calculation.html"));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionHomePage_triggered()
{
    QDesktopServices::openUrl(QUrl("http://coil32.net"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
    int tab = mui->tabWidget->currentIndex();
    FormCoil = (_FormCoil)currentRow;
    QString tmp_txt;
    switch (tab) {
    case 0:{
        tmp_txt = tr("Inductance") + " L:";
        mui->label_ind->setText(tmp_txt);
        tmp_txt = tr("Frequency") + " f:";
        mui->label_freq->setText(tmp_txt);
        mui->label_ind_m->setText(qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()));
        mui->label_01->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_02->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_03->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_04->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_05->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->lineEdit_ind->setFocus();
        switch (FormCoil) {
        case _Onelayer:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil2.png"));
            mui->groupBox->setVisible(true);
            mui->groupBox_6->setVisible(false);
            mui->label_freq_m->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
            if (myOpt->isAWG){
                mui->label_02->setText(tr("AWG"));
            }
            mui->label_freq->setVisible(true);
            mui->label_freq_m->setVisible(true);
            mui->lineEdit_freq->setVisible(true);
            mui->lineEdit_3->setVisible(true);
            mui->label_3->setVisible(true);
            mui->label_03->setVisible(true);
            mui->lineEdit_4->setVisible(true);
            mui->label_4->setVisible(true);
            mui->label_04->setVisible(true);
            mui->lineEdit_5->setVisible(false);
            mui->label_5->setVisible(false);
            mui->label_05->setVisible(false);
            mui->lineEdit_6->setVisible(false);
            mui->label_6->setVisible(false);
            mui->label_06->setVisible(false);
            mui->line_6->setVisible(true);
            tmp_txt = tr("Former diameter") + " D:";
            mui->label_1->setText(tmp_txt);
            tmp_txt = tr("Wire diameter") + " d:";
            mui->label_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter with insulation") + " k:";
            mui->label_3->setText(tmp_txt);
            tmp_txt = tr("Winding pitch")+" p:";
            mui->label_4->setText(tmp_txt);
            mui->lineEdit_ind->setText(QString::number(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_freq->setText(QString::number(data->frequency / myOpt->dwFrequencyMultiplier));
            mui->lineEdit_1->setText(QString::number(data->D / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    double AWG = round(-39 * log(data->d / 0.127) / log(92) + 36);
                    mui->lineEdit_2->setText(QString::number(AWG));
                } else
                    mui->lineEdit_2->setText("");
            } else
                mui->lineEdit_2->setText(QString::number(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_2->text().isEmpty() || (mui->lineEdit_2->text() == "0"))
                mui->lineEdit_3->setText(QString::number(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_2_editingFinished();
            mui->lineEdit_4->setText(QString::number(data->p / myOpt->dwLengthMultiplier));
            break;
        }
        case _Onelayer_p:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil2_square.png"));
            mui->groupBox->setVisible(true);
            mui->groupBox_6->setVisible(false);
            mui->label_freq_m->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
            mui->label_freq->setVisible(true);
            mui->label_freq_m->setVisible(true);
            mui->lineEdit_freq->setVisible(true);
            mui->lineEdit_3->setVisible(true);
            mui->label_3->setVisible(true);
            mui->label_03->setVisible(true);
            mui->lineEdit_4->setVisible(true);
            mui->label_4->setVisible(true);
            mui->label_04->setVisible(true);
            mui->lineEdit_5->setVisible(true);
            mui->label_5->setVisible(true);
            mui->label_05->setVisible(true);
            mui->lineEdit_6->setVisible(false);
            mui->label_6->setVisible(false);
            mui->label_06->setVisible(false);
            mui->line_6->setVisible(true);
            tmp_txt = tr("Former diameter") +" D:";
            mui->label_1->setText(tmp_txt);
            tmp_txt = tr("Wire width") + " w:";
            mui->label_2->setText(tmp_txt);
            tmp_txt = tr("Wire thickness") + " t:";
            mui->label_3->setText(tmp_txt);
            tmp_txt = tr("Insulation thickness")+" i:";
            mui->label_4->setText(tmp_txt);
            tmp_txt = tr("Winding pitch")+" p:";
            mui->label_5->setText(tmp_txt);
            mui->lineEdit_ind->setText(QString::number(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_freq->setText(QString::number(data->frequency / myOpt->dwFrequencyMultiplier));
            mui->lineEdit_1->setText(QString::number(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2->setText(QString::number(data->w / myOpt->dwLengthMultiplier));
            mui->lineEdit_3->setText(QString::number(data->t / myOpt->dwLengthMultiplier));
            mui->lineEdit_4->setText(QString::number(data->isol / myOpt->dwLengthMultiplier));
            mui->lineEdit_5->setText(QString::number(data->p / myOpt->dwLengthMultiplier));
            break;
        }
        case _Multilayer:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            if (myOpt->isAWG){
                mui->label_03->setText(tr("AWG"));
            }
            mui->label_freq->setVisible(false);
            mui->label_freq_m->setVisible(false);
            mui->lineEdit_freq->setVisible(false);
            mui->lineEdit_3->setVisible(true);
            mui->label_3->setVisible(true);
            mui->label_03->setVisible(true);
            mui->lineEdit_4->setVisible(true);
            mui->label_4->setVisible(true);
            mui->label_04->setVisible(true);
            mui->lineEdit_5->setVisible(false);
            mui->label_5->setVisible(false);
            mui->label_05->setVisible(false);
            mui->lineEdit_6->setVisible(false);
            mui->label_6->setVisible(false);
            mui->label_06->setVisible(false);
            mui->line_6->setVisible(false);
            tmp_txt = tr("Former diameter") + " D:";
            mui->label_1->setText(tmp_txt);
            tmp_txt = tr("Winding length") + " l:";
            mui->label_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter") + " d:";
            mui->label_3->setText(tmp_txt);
            tmp_txt = tr("Wire diameter with insulation") + " k:";
            mui->label_4->setText(tmp_txt);
            mui->lineEdit_ind->setText(QString::number(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(QString::number(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2->setText(QString::number(data->l / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    double AWG = round(-39 * log(data->d / 0.127) / log(92) + 36);
                    mui->lineEdit_3->setText(QString::number(AWG));
                } else
                    mui->lineEdit_3->setText("");
            } else
                mui->lineEdit_3->setText(QString::number(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_3->text().isEmpty() || (mui->lineEdit_3->text() == "0"))
                mui->lineEdit_4->setText(QString::number(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_3_editingFinished();
            break;
        }
        case _Multilayer_p:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4-0.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            if (myOpt->isAWG){
                mui->label_03->setText(tr("AWG"));
            }
            mui->label_freq->setVisible(false);
            mui->label_freq_m->setVisible(false);
            mui->lineEdit_freq->setVisible(false);
            mui->lineEdit_3->setVisible(true);
            mui->label_3->setVisible(true);
            mui->label_03->setVisible(true);
            mui->lineEdit_4->setVisible(true);
            mui->label_4->setVisible(true);
            mui->label_04->setVisible(true);
            mui->lineEdit_5->setVisible(true);
            mui->label_5->setVisible(true);
            mui->label_05->setVisible(true);
            mui->lineEdit_6->setVisible(true);
            mui->label_6->setVisible(true);
            mui->label_06->setVisible(true);
            mui->line_6->setVisible(false);
            tmp_txt = tr("Former diameter") + " D:";
            mui->label_1->setText(tmp_txt);
            tmp_txt = tr("Winding length") + " l:";
            mui->label_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter") + " d:";
            mui->label_3->setText(tmp_txt);
            tmp_txt = tr("Wire diameter with insulation") + " k:";
            mui->label_4->setText(tmp_txt);
            tmp_txt = tr("Insulation thickness")+" g:";
            mui->label_5->setText(tmp_txt);
            tmp_txt = tr("Layers number beetween insulating pads") + " Ng:";
            mui->label_6->setText(tmp_txt);
            mui->lineEdit_ind->setText(QString::number(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(QString::number(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2->setText(QString::number(data->l / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    double AWG = round(-39 * log(data->d / 0.127) / log(92) + 36);
                    mui->lineEdit_3->setText(QString::number(AWG));
                } else
                    mui->lineEdit_3->setText("");
            } else
                mui->lineEdit_3->setText(QString::number(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_3->text().isEmpty() || (mui->lineEdit_3->text() == "0"))
                mui->lineEdit_4->setText(QString::number(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_3_editingFinished();
            mui->lineEdit_5->setText(QString::number(data->g / myOpt->dwLengthMultiplier));
            mui->lineEdit_6->setText(QString::number(data->Ng));
            break;
        }
        case _FerrToroid:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil6.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            if (myOpt->isAWG){
                mui->label_04->setText(tr("AWG"));
            }
            mui->label_freq->setVisible(false);
            mui->label_freq_m->setVisible(false);
            mui->lineEdit_freq->setVisible(false);
            mui->lineEdit_3->setVisible(true);
            mui->label_3->setVisible(true);
            mui->label_03->setVisible(true);
            mui->lineEdit_4->setVisible(true);
            mui->label_4->setVisible(true);
            mui->label_04->setVisible(true);
            mui->lineEdit_5->setVisible(true);
            mui->label_5->setVisible(true);
            mui->label_05->setVisible(false);
            mui->lineEdit_6->setVisible(false);
            mui->label_6->setVisible(false);
            mui->label_06->setVisible(false);
            mui->line_6->setVisible(false);
            tmp_txt = tr("Outside diameter")+" D1:";
            mui->label_1->setText(tmp_txt);
            tmp_txt = tr("Inside diameter")+" D2:";
            mui->label_2->setText(tmp_txt);
            tmp_txt = tr("Core height") + " h:";
            mui->label_3->setText(tmp_txt);
            tmp_txt = tr("Wire diameter") + " d:";
            mui->label_4->setText(tmp_txt);
            tmp_txt = tr("Init magnetic permeability")+" μ:";
            mui->label_5->setText(tmp_txt);
            mui->lineEdit_ind->setText(QString::number(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(QString::number(data->Do / myOpt->dwLengthMultiplier));
            mui->lineEdit_2->setText(QString::number(data->Di / myOpt->dwLengthMultiplier));
            mui->lineEdit_3->setText(QString::number(data->h / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    double AWG = round(-39 * log(data->d / 0.127) / log(92) + 36);
                    mui->lineEdit_4->setText(QString::number(AWG));
                } else
                    mui->lineEdit_4->setText("");
            } else
                mui->lineEdit_4->setText(QString::number(data->d / myOpt->dwLengthMultiplier));
            mui->lineEdit_5->setText(QString::number(data->mu));
            break;
        }
        case _PCB_square:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil8.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(true);
            mui->label_freq->setVisible(false);
            mui->label_freq_m->setVisible(false);
            mui->lineEdit_freq->setVisible(false);
            mui->lineEdit_3->setVisible(false);
            mui->label_3->setVisible(false);
            mui->label_03->setVisible(false);
            mui->lineEdit_4->setVisible(false);
            mui->label_4->setVisible(false);
            mui->label_04->setVisible(false);
            mui->lineEdit_5->setVisible(false);
            mui->label_5->setVisible(false);
            mui->label_05->setVisible(false);
            mui->lineEdit_6->setVisible(false);
            mui->label_6->setVisible(false);
            mui->label_06->setVisible(false);
            mui->line_6->setVisible(true);
            tmp_txt = tr("Outside diameter")+" D1:";
            mui->label_1->setText(tmp_txt);
            tmp_txt = tr("Inside diameter")+" D2:";
            mui->label_2->setText(tmp_txt);
            mui->lineEdit_ind->setText(QString::number(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(QString::number(data->Do / myOpt->dwLengthMultiplier));
            mui->lineEdit_2->setText(QString::number(data->Di / myOpt->dwLengthMultiplier));
            mui->horizontalSlider->setValue(data->ratio * 100);
            break;
        }
        case _Flat_Spiral:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil10.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            if (myOpt->isAWG){
                mui->label_02->setText(tr("AWG"));
            }
            mui->label_freq->setVisible(false);
            mui->label_freq_m->setVisible(false);
            mui->lineEdit_freq->setVisible(false);
            mui->lineEdit_3->setVisible(true);
            mui->label_3->setVisible(true);
            mui->label_03->setVisible(true);
            mui->lineEdit_4->setVisible(false);
            mui->label_4->setVisible(false);
            mui->label_04->setVisible(false);
            mui->lineEdit_5->setVisible(false);
            mui->label_5->setVisible(false);
            mui->label_05->setVisible(false);
            mui->lineEdit_6->setVisible(false);
            mui->label_6->setVisible(false);
            mui->label_06->setVisible(false);
            mui->line_6->setVisible(false);
            tmp_txt = tr("Inside diameter")+" Di:";
            mui->label_1->setText(tmp_txt);
            tmp_txt = tr("Wire diameter") + " d:";
            mui->label_2->setText(tmp_txt);
            mui->label_3->setText(tr("Gap between turns") + " s:");
            mui->lineEdit_ind->setText(QString::number(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(QString::number(data->Di / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    double AWG = round(-39 * log(data->d / 0.127) / log(92) + 36);
                    mui->lineEdit_2->setText(QString::number(AWG));
                } else
                    mui->lineEdit_2->setText("");
            } else
                mui->lineEdit_2->setText(QString::number(data->d / myOpt->dwLengthMultiplier));
            mui->lineEdit_3->setText(QString::number(data->s / myOpt->dwLengthMultiplier));
            break;
        }
        default:
            break;
        }
        break;
    }
    case 1:{
        tmp_txt = tr("Number of turns") + " N:";
        mui->label_N->setText(tmp_txt);
        tmp_txt = tr("Frequency") + " f:";
        mui->label_freq2->setText(tmp_txt);
        mui->label_01_2->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_02_2->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_03_2->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_04_2->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_05_2->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_06_2->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->lineEdit_N->setFocus();
        switch (FormCoil) {
        case _Onelayer:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil2.png"));
            mui->groupBox_2->setVisible(true);
            mui->groupBox_7->setVisible(false);
            mui->label_freq_m2->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
            if (myOpt->isAWG){
                mui->label_02_2->setText(tr("AWG"));
            }
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->label_freq2->setVisible(true);
            mui->label_freq_m2->setVisible(true);
            mui->lineEdit_freq2->setVisible(true);
            mui->lineEdit_3_2->setVisible(true);
            mui->label_3_2->setVisible(true);
            mui->label_03_2->setVisible(true);
            mui->lineEdit_4_2->setVisible(true);
            mui->label_4_2->setVisible(true);
            mui->label_04_2->setVisible(true);
            mui->lineEdit_5_2->setVisible(false);
            mui->label_5_2->setVisible(false);
            mui->label_05_2->setVisible(false);
            mui->lineEdit_6_2->setVisible(false);
            mui->label_6_2->setVisible(false);
            mui->label_06_2->setVisible(false);
            mui->lineEdit_7_2->setVisible(false);
            mui->label_7_2->setVisible(false);
            mui->label_07_2->setVisible(false);
            tmp_txt = tr("Former diameter") + " D:";
            mui->label_1_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter") + " d:";
            mui->label_2_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter with insulation") + " k:";
            mui->label_3_2->setText(tmp_txt);
            tmp_txt = tr("Winding pitch")+" p:";
            mui->label_4_2->setText(tmp_txt);
            mui->lineEdit_N->setText(QString::number(data->N));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_freq2->setText(QString::number(data->frequency / myOpt->dwFrequencyMultiplier));
            mui->lineEdit_1_2->setText(QString::number(data->D / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    double AWG = round(-39 * log(data->d / 0.127) / log(92) + 36);
                    mui->lineEdit_2_2->setText(QString::number(AWG));
                } else
                    mui->lineEdit_2_2->setText("");
            } else
                mui->lineEdit_2_2->setText(QString::number(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_2_2->text().isEmpty() || (mui->lineEdit_2_2->text() == "0"))
                mui->lineEdit_3_2->setText(QString::number(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_2_2_editingFinished();
            mui->lineEdit_4_2->setText(QString::number(data->p / myOpt->dwLengthMultiplier));
            break;
        }
        case _Onelayer_p:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil2_square.png"));
            mui->groupBox_2->setVisible(true);
            mui->groupBox_7->setVisible(false);
            mui->label_freq_m2->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->label_freq2->setVisible(true);
            mui->label_freq_m2->setVisible(true);
            mui->lineEdit_freq2->setVisible(true);
            mui->lineEdit_3_2->setVisible(true);
            mui->label_3_2->setVisible(true);
            mui->label_03_2->setVisible(true);
            mui->lineEdit_4_2->setVisible(true);
            mui->label_4_2->setVisible(true);
            mui->label_04_2->setVisible(true);
            mui->lineEdit_5_2->setVisible(true);
            mui->label_5_2->setVisible(true);
            mui->label_05_2->setVisible(true);
            mui->lineEdit_6_2->setVisible(false);
            mui->label_6_2->setVisible(false);
            mui->label_06_2->setVisible(false);
            mui->lineEdit_7_2->setVisible(false);
            mui->label_7_2->setVisible(false);
            mui->label_07_2->setVisible(false);
            tmp_txt = tr("Former diameter") + " D:";
            mui->label_1_2->setText(tmp_txt);
            tmp_txt = tr("Wire width") + " w:";
            mui->label_2_2->setText(tmp_txt);
            tmp_txt = tr("Wire thickness") + " t:";
            mui->label_3_2->setText(tmp_txt);
            tmp_txt = tr("Insulation thickness")+" i:";
            mui->label_4_2->setText(tmp_txt);
            tmp_txt = tr("Winding pitch")+" p:";
            mui->label_5_2->setText(tmp_txt);
            mui->lineEdit_N->setText(QString::number(data->N));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_freq2->setText(QString::number(data->frequency / myOpt->dwFrequencyMultiplier));
            mui->lineEdit_1_2->setText(QString::number(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(QString::number(data->w / myOpt->dwLengthMultiplier));
            mui->lineEdit_3_2->setText(QString::number(data->t / myOpt->dwLengthMultiplier));
            mui->lineEdit_4_2->setText(QString::number(data->isol / myOpt->dwLengthMultiplier));
            mui->lineEdit_5_2->setText(QString::number(data->p / myOpt->dwLengthMultiplier));
            break;
        }
        case _Multilayer:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(true);
            if (myOpt->isAWG){
                mui->label_04_2->setText(tr("AWG"));
            }
            mui->label_N->setVisible(false);
            mui->lineEdit_N->setVisible(false);
            mui->label_freq2->setVisible(false);
            mui->label_freq_m2->setVisible(false);
            mui->lineEdit_freq2->setVisible(false);
            mui->lineEdit_3_2->setVisible(true);
            mui->label_3_2->setVisible(true);
            mui->label_03_2->setVisible(true);
            mui->lineEdit_4_2->setVisible(true);
            mui->label_4_2->setVisible(true);
            mui->label_04_2->setVisible(true);
            mui->lineEdit_5_2->setVisible(true);
            mui->label_5_2->setVisible(true);
            mui->label_05_2->setVisible(true);
            mui->lineEdit_6_2->setVisible(false);
            mui->label_6_2->setVisible(false);
            mui->label_06_2->setVisible(false);
            mui->lineEdit_7_2->setVisible(false);
            mui->label_7_2->setVisible(false);
            mui->label_07_2->setVisible(false);
            tmp_txt = tr("Former diameter") + " D:";
            mui->label_1_2->setText(tmp_txt);
            tmp_txt = tr("Winding length") + " l:";
            mui->label_2_2->setText(tmp_txt);
            tmp_txt = tr("Thickness of the coil") + " c:";
            mui->label_3_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter") + " d:";
            mui->label_4_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter with insulation") + " k:";
            mui->label_5_2->setText(tmp_txt);
            mui->lineEdit_1_2->setText(QString::number(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(QString::number(data->l / myOpt->dwLengthMultiplier));
            mui->lineEdit_3_2->setText(QString::number(data->c / myOpt->dwLengthMultiplier));
            on_radioButton_8_toggled(mui->radioButton_8->isChecked());
            mui->lineEdit_5_2->setText(QString::number(data->k / myOpt->dwLengthMultiplier));
            break;
        }
        case _Multilayer_p:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4-0.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            if (myOpt->isAWG){
                mui->label_04_2->setText(tr("AWG"));
            }
            mui->label_N->setVisible(false);
            mui->lineEdit_N->setVisible(false);
            mui->label_freq2->setVisible(false);
            mui->label_freq_m2->setVisible(false);
            mui->lineEdit_freq2->setVisible(false);
            mui->lineEdit_3_2->setVisible(true);
            mui->label_3_2->setVisible(true);
            mui->label_03_2->setVisible(true);
            mui->lineEdit_4_2->setVisible(true);
            mui->label_4_2->setVisible(true);
            mui->label_04_2->setVisible(true);
            mui->lineEdit_5_2->setVisible(true);
            mui->label_5_2->setVisible(true);
            mui->label_05_2->setVisible(true);
            mui->lineEdit_6_2->setVisible(true);
            mui->label_6_2->setVisible(true);
            mui->label_06_2->setVisible(true);
            mui->lineEdit_7_2->setVisible(true);
            mui->label_7_2->setVisible(true);
            mui->label_07_2->setVisible(true);
            tmp_txt = tr("Former diameter") + " D:";
            mui->label_1_2->setText(tmp_txt);
            tmp_txt = tr("Winding length") + " l:";
            mui->label_2_2->setText(tmp_txt);
            tmp_txt = tr("Thickness of the coil") + " c:";
            mui->label_3_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter") + " d:";
            mui->label_4_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter with insulation") + " k:";
            mui->label_5_2->setText(tmp_txt);
            tmp_txt = tr("Insulation thickness")+" g:";
            mui->label_6_2->setText(tmp_txt);
            tmp_txt = tr("Layers number beetween insulating pads") + " Ng:";
            mui->label_7_2->setText(tmp_txt);
            mui->lineEdit_1_2->setText(QString::number(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(QString::number(data->l / myOpt->dwLengthMultiplier));
            mui->lineEdit_3_2->setText(QString::number(data->c / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    double AWG = round(-39 * log(data->d / 0.127) / log(92) + 36);
                    mui->lineEdit_4_2->setText(QString::number(AWG));
                } else
                    mui->lineEdit_4_2->setText("");
            } else
                mui->lineEdit_4_2->setText(QString::number(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_4_2->text().isEmpty() || (mui->lineEdit_4_2->text() == "0"))
                mui->lineEdit_5_2->setText(QString::number(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_4_2_editingFinished();
            mui->lineEdit_6_2->setText(QString::number(data->g / myOpt->dwLengthMultiplier));
            mui->lineEdit_7_2->setText(QString::number(data->Ng / myOpt->dwLengthMultiplier));
            break;
        }
        case _FerrToroid:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil6.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->label_freq2->setVisible(false);
            mui->label_freq_m2->setVisible(false);
            mui->lineEdit_freq2->setVisible(false);
            mui->lineEdit_3_2->setVisible(true);
            mui->label_3_2->setVisible(true);
            mui->label_03_2->setVisible(true);
            mui->lineEdit_4_2->setVisible(true);
            mui->label_4_2->setVisible(true);
            mui->label_04_2->setVisible(false);
            mui->lineEdit_5_2->setVisible(false);
            mui->label_5_2->setVisible(false);
            mui->label_05_2->setVisible(false);
            mui->lineEdit_6_2->setVisible(false);
            mui->label_6_2->setVisible(false);
            mui->label_06_2->setVisible(false);
            mui->lineEdit_7_2->setVisible(false);
            mui->label_7_2->setVisible(false);
            mui->label_07_2->setVisible(false);
            tmp_txt = tr("Outside diameter")+" D1:";
            mui->label_1_2->setText(tmp_txt);
            tmp_txt = tr("Inside diameter")+" D2:";
            mui->label_2_2->setText(tmp_txt);
            tmp_txt = tr("Core height") + " h:";
            mui->label_3_2->setText(tmp_txt);
            tmp_txt = tr("Init magnetic permeability")+" μ:";
            mui->label_4_2->setText(tmp_txt);
            mui->lineEdit_N->setText(QString::number(data->N));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_1_2->setText(QString::number(data->Do / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(QString::number(data->Di / myOpt->dwLengthMultiplier));
            mui->lineEdit_3_2->setText(QString::number(data->h / myOpt->dwLengthMultiplier));
            mui->lineEdit_4_2->setText(QString::number(data->mu / myOpt->dwLengthMultiplier));
            break;
        }
        case _PCB_square:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil8.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->label_freq2->setVisible(false);
            mui->label_freq_m2->setVisible(false);
            mui->lineEdit_freq2->setVisible(false);
            mui->lineEdit_3_2->setVisible(true);
            mui->label_3_2->setVisible(true);
            mui->label_03_2->setVisible(true);
            mui->lineEdit_4_2->setVisible(true);
            mui->label_4_2->setVisible(true);
            mui->label_04_2->setVisible(true);
            mui->lineEdit_5_2->setVisible(false);
            mui->label_5_2->setVisible(false);
            mui->label_05_2->setVisible(false);
            mui->lineEdit_6_2->setVisible(false);
            mui->label_6_2->setVisible(false);
            mui->label_06_2->setVisible(false);
            mui->lineEdit_7_2->setVisible(false);
            mui->label_7_2->setVisible(false);
            mui->label_07_2->setVisible(false);
            tmp_txt = tr("Outside diameter")+" D1:";
            mui->label_1_2->setText(tmp_txt);
            tmp_txt = tr("Inside diameter")+" D2:";
            mui->label_2_2->setText(tmp_txt);
            tmp_txt = tr("Winding pitch")+" s:";
            mui->label_3_2->setText(tmp_txt);
            tmp_txt = tr("Width of a PCB trace") + " W:";
            mui->label_4_2->setText(tmp_txt);
            mui->lineEdit_N->setText(QString::number(data->N));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_1_2->setText(QString::number(data->Do / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(QString::number(data->Di / myOpt->dwLengthMultiplier));
            mui->lineEdit_3_2->setText(QString::number(data->s / myOpt->dwLengthMultiplier));
            mui->lineEdit_4_2->setText(QString::number(data->w / myOpt->dwLengthMultiplier));
            break;
        }
        case _Flat_Spiral:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil10.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            if (myOpt->isAWG){
                mui->label_03_2->setText(tr("AWG"));
            }
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->label_freq2->setVisible(false);
            mui->label_freq_m2->setVisible(false);
            mui->lineEdit_freq2->setVisible(false);
            mui->lineEdit_3_2->setVisible(true);
            mui->label_3_2->setVisible(true);
            mui->label_03_2->setVisible(true);
            mui->lineEdit_4_2->setVisible(false);
            mui->label_4_2->setVisible(false);
            mui->label_04_2->setVisible(false);
            mui->lineEdit_5_2->setVisible(false);
            mui->label_5_2->setVisible(false);
            mui->label_05_2->setVisible(false);
            mui->lineEdit_6_2->setVisible(false);
            mui->label_6_2->setVisible(false);
            mui->label_06_2->setVisible(false);
            mui->lineEdit_7_2->setVisible(false);
            mui->label_7_2->setVisible(false);
            mui->label_07_2->setVisible(false);
            tmp_txt = tr("Outside diameter")+" Do:";
            mui->label_1_2->setText(tmp_txt);
            tmp_txt = tr("Inside diameter")+" Di:";
            mui->label_2_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter") + " d:";
            mui->label_3_2->setText(tmp_txt);
            mui->lineEdit_N->setText(QString::number(data->N));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_1_2->setText(QString::number(data->Do / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(QString::number(data->Di / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    double AWG = round(-39 * log(data->d / 0.127) / log(92) + 36);
                    mui->lineEdit_3_2->setText(QString::number(AWG));
                } else
                    mui->lineEdit_3_2->setText("");
            } else
                mui->lineEdit_3_2->setText(QString::number(data->d / myOpt->dwLengthMultiplier));
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionExit_triggered()
{
    QApplication::closeAllWindows();
    QCoreApplication::exit();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionTo_null_data_triggered()
{
#if defined(Q_OS_MAC) || (Q_WS_X11) || defined(Q_OS_LINUX)
    QSettings *settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#elif defined(Q_WS_WIN) || defined(Q_OS_WIN)
    QSettings *settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#else
    QSettings *settings = new QSettings(QDir::currentPath() + "/Coil64.conf", QSettings::IniFormat);
#endif
    data->N = 0;
    data->frequency = 0;
    data->inductance = 0;
    data->capacitance = 0;
    data->D = 0;
    data->d = 0;
    data->k = 0;
    data->p = 0;
    data->w = 0;
    data->t = 0;
    data->isol = 0;
    data->l = 0;
    data->c = 0;
    data->g = 0;
    data->Ng = 0;
    data->Do = 0;
    data->Di = 0;
    data->h = 0;
    data->mu = 0;
    data->ratio = 0.5;
    data->s = 0;
    data->Rdc = 0;

    mui->lineEdit_ind->clear();
    mui->lineEdit_freq->clear();
    mui->lineEdit_N->clear();
    mui->lineEdit_freq2->clear();
    mui->lineEdit_1->clear();
    mui->lineEdit_2->clear();
    mui->lineEdit_3->clear();
    mui->lineEdit_4->clear();
    mui->lineEdit_5->clear();
    mui->lineEdit_6->clear();
    mui->lineEdit_1_2->clear();
    mui->lineEdit_2_2->clear();
    mui->lineEdit_3_2->clear();
    mui->lineEdit_4_2->clear();
    mui->lineEdit_5_2->clear();
    mui->lineEdit_6_2->clear();
    mui->lineEdit_7_2->clear();
    mui->lineEdit_1_3->clear();
    mui->lineEdit_2_3->clear();

    settings->remove("Data");
    settings->remove("FerriteRod");
    settings->remove("FindPermeability");
    delete settings;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionClear_all_triggered()
{
    mui->textBrowser->clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionOptions_triggered()
{
    Options *opt = new Options(this);
    opt->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(opt, SIGNAL(sendOpt(_OptionStruct)), this, SLOT(getOptionStruct(_OptionStruct)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), opt, SLOT(getOpt(_OptionStruct)));
    emit sendOpt(*myOpt);
    opt->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionCopy_triggered()
{
    mui->textBrowser->selectAll();
    mui->textBrowser->copy();;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionOpen_triggered()
{
#if defined(Q_OS_MAC) || (Q_WS_X11) || defined(Q_OS_LINUX)
        QSettings *settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#elif defined(Q_WS_WIN) || defined(Q_OS_WIN)
        QSettings *settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#else
        QSettings *settings = new QSettings(QDir::currentPath() + "/Coil64.conf", QSettings::IniFormat);
#endif
        settings->beginGroup( "GUI" );
        QString saveDir = settings->value("SaveDir", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).toUtf8()).toString();
        QDir dir(saveDir);
        if (dir.exists()){
            QString filters(".pdf (*.pdf);;.odf (*.odf);;.htm (*.htm)");
            QString defaultFilter(".htm (*.htm)");
            QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), saveDir, filters, &defaultFilter);
            if (!fileName.isEmpty()){
                QString ext = defaultFilter.mid(7,4);
                if ((ext == ".odf") || (ext == ".pdf")){
                    QUrl url = QUrl::fromLocalFile(fileName);
                    QDesktopServices::openUrl(url);
                }
                else {
                    QTextDocument *document = mui->textBrowser->document();
                    QFile file(fileName);
                    file.open(QIODevice::ReadOnly);
                    document->setHtml(file.readAll().toStdString().data());
                }
            }
        }
    settings->endGroup();
    delete settings;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionSave_triggered()
{
    QTextDocument *document = mui->textBrowser->document();
    if (!document->isEmpty()){
#if defined(Q_OS_MAC) || (Q_WS_X11) || defined(Q_OS_LINUX)
        QSettings *settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#elif defined(Q_WS_WIN) || defined(Q_OS_WIN)
        QSettings *settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#else
        QSettings *settings = new QSettings(QDir::currentPath() + "/Coil64.conf", QSettings::IniFormat);
#endif
        settings->beginGroup( "GUI" );
        QString saveDir = settings->value("SaveDir", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).toUtf8()).toString();
        QString filters(".pdf (*.pdf);;.odf (*.odf);;.htm (*.htm)");
        QString defaultFilter(".htm (*.htm)");
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save"), saveDir, filters, &defaultFilter);
        if (!fileName.isEmpty()){
            QFileInfo info(fileName);
            saveDir = info.absolutePath().toUtf8();
            settings->setValue("SaveDir", saveDir);
            QString ext = defaultFilter.mid(7,4);
            int p = fileName.indexOf(".");
            if (p < 0){
                fileName.append(ext);
            }
            if (ext == ".odf"){
                QTextDocumentWriter textwriter(fileName);
                textwriter.write(document);
            }
            if (ext == ".pdf"){
                QPrinter printer(QPrinter::PrinterResolution);
                printer.setOutputFormat(QPrinter::PdfFormat);
                printer.setPaperSize(QPrinter::A4);
                printer.setOutputFileName(fileName);
                printer.setPageMargins(QMarginsF(50, 50, 50, 50));
                QFont oldFont(document->defaultFont());
                QFont newFont = oldFont;
                newFont.setFamily("Courier New");
                newFont.setPointSize(6);
                document->setDefaultFont(newFont);
                document->setPageSize(printer.pageRect().size());
                document->print(&printer);
                document->setDefaultFont(oldFont);
            }
            if (ext == ".htm"){
                QFile file(fileName);
                file.open(QIODevice::WriteOnly);
                file.write(document->toHtml().toUtf8());
                file.close();
            }
        }
        settings->endGroup();
        delete settings;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionAbout_triggered()
{
    About *about = new About(this);
    about->setAttribute(Qt::WA_DeleteOnClose, true);
    about->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_tabWidget_currentChanged(int index)
{
    int tab = index;
    switch (tab){
    case 0:
    case 1:{
        mui->groupBox_List1->setVisible(true);
        mui->listWidget->setCurrentRow(FormCoil);
        on_listWidget_currentRowChanged(FormCoil);
        break;
    }
    case 2:{
        mui->groupBox_List1->setVisible(false);
        mui->image->setPixmap(QPixmap(":/images/res/LC.png"));
        mui->lineEdit_1_3->setFocus();
        if (mui->radioButton_LC->isChecked())
            on_radioButton_LC_clicked();
        if (mui->radioButton_CF->isChecked())
            on_radioButton_CF_clicked();
        if (mui->radioButton_LF->isChecked())
            on_radioButton_LF_clicked();
        break;
    }
    default:
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_LC_clicked()
{
    mui->groupBox_1_3->setTitle(tr("Inductance"));
    mui->lineEdit_1_3->setText(QString::number(data->inductance / myOpt->dwInductanceMultiplier));
    mui->groupBox_2_3->setTitle(tr("External capacitance"));
    mui->lineEdit_2_3->setText(QString::number(data->capacitance / myOpt->dwCapacityMultiplier));
    mui->label_01_3->setText(qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()));
    mui->label_02_3->setText(qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_CF_clicked()
{
    mui->groupBox_1_3->setTitle(tr("External capacitance"));
    mui->lineEdit_1_3->setText(QString::number(data->capacitance / myOpt->dwCapacityMultiplier));
    mui->groupBox_2_3->setTitle(tr("Frequency"));
    mui->lineEdit_2_3->setText(QString::number(data->frequency / myOpt->dwFrequencyMultiplier));
    mui->label_01_3->setText(qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()));
    mui->label_02_3->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_LF_clicked()
{
    mui->groupBox_1_3->setTitle(tr("Inductance"));
    mui->lineEdit_1_3->setText(QString::number(data->inductance / myOpt->dwInductanceMultiplier));
    mui->groupBox_2_3->setTitle(tr("Frequency"));
    mui->lineEdit_2_3->setText(QString::number(data->frequency / myOpt->dwFrequencyMultiplier));
    mui->label_01_3->setText(qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()));
    mui->label_02_3->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_1_clicked(bool checked)
{
    mui->radioButton_1_2->setChecked(checked);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_2_clicked(bool checked)
{
    mui->radioButton_2_2->setChecked(checked);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_3_clicked(bool checked)
{
    mui->radioButton_3_2->setChecked(checked);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_4_clicked(bool checked)
{
    mui->radioButton_4_2->setChecked(checked);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_1_2_clicked(bool checked)
{
    mui->radioButton_1->setChecked(checked);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_2_2_clicked(bool checked)
{
    mui->radioButton_2->setChecked(checked);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_3_2_clicked(bool checked)
{
    mui->radioButton_3->setChecked(checked);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_4_2_clicked(bool checked)
{
    mui->radioButton_4->setChecked(checked);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_showImg_clicked()
{
    myOpt->isInsertImage = mui->toolButton_showImg->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_showAdditional_clicked()
{
    myOpt->isAdditionalResult = mui->toolButton_showAdditional->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_CopySel_clicked()
{
    mui->textBrowser->copy();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_CopyAll_clicked()
{
    on_actionCopy_triggered();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_Clear_clicked()
{
    on_actionClear_all_triggered();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_Print_clicked()
{
    on_actionPrint_triggered();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_Open_clicked()
{
    on_actionOpen_triggered();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_Save_clicked()
{
    on_actionSave_triggered();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_Help_clicked()
{
    on_actionHelp_triggered();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_8_toggled(bool checked)
{
    QString tmp_txt;
    if (checked){
        tmp_txt = tr("Resistance of the coil") + " Rdc:";
        mui->label_4_2->setText(tmp_txt);
        mui->label_04_2->setText(tr("Ohm"));
        mui->lineEdit_4_2->setText(QString::number(data->Rdc));
    } else {
        tmp_txt = tr("Wire diameter") + " d:";
        mui->label_4_2->setText(tmp_txt);
        mui->label_04_2->setText(tr("mm"));
        if (myOpt->isAWG){
            if (data->d > 0){
                double AWG = round(-39 * log(data->d / 0.127) / log(92) + 36);
                mui->lineEdit_4_2->setText(QString::number(AWG));
            } else
                mui->lineEdit_4_2->setText("");
        } else
            mui->lineEdit_4_2->setText(QString::number(data->d / myOpt->dwLengthMultiplier));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::getOptionStruct(_OptionStruct gOpt){
    *myOpt = gOpt;
    completeOptionsStructure(myOpt);
    resetUiFont();
    on_tabWidget_currentChanged(mui->tabWidget->currentIndex());
    mui->toolButton_showImg->setChecked(myOpt->isInsertImage);
    mui->toolButton_showAdditional->setChecked(myOpt->isAdditionalResult);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_ind_editingFinished()
{
    if (!mui->lineEdit_ind->text().isEmpty())
        data->inductance = mui->lineEdit_ind->text().toDouble()*myOpt->dwInductanceMultiplier;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_freq_editingFinished()
{
    if (!mui->lineEdit_freq->text().isEmpty())
        data->frequency = mui->lineEdit_freq->text().toDouble()*myOpt->dwFrequencyMultiplier;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_1_editingFinished()
{
    if (!mui->lineEdit_1->text().isEmpty()){
        switch (FormCoil) {
        case _Onelayer:
        case _Onelayer_p:
        case _Multilayer:
        case _Multilayer_p:{
            data->D = mui->lineEdit_1->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _FerrToroid:
        case _PCB_square:{
            data->Do = mui->lineEdit_1->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Flat_Spiral:{
            data->Di = mui->lineEdit_1->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        default:
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_2_editingFinished()
{
    if (!mui->lineEdit_2->text().isEmpty()){
        switch (FormCoil) {
        case _Onelayer:{
            double d = 0;
            if (myOpt->isAWG){
                double AWG = mui->lineEdit_2->text().toDouble();
                d = 0.127 * pow(92, (36 - AWG) / 39);
            } else {
                d = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
            }
            data->d = d;
            double k_m = odCalc(d);
            if (d > 0){
                mui->lineEdit_3->setText( QString::number(k_m / myOpt->dwLengthMultiplier));
            }
            break;
        }
        case _Onelayer_p:{
            data->w = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Multilayer:
        case _Multilayer_p:{
            data->l = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _FerrToroid:
        case _PCB_square:{
            data->Di = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Flat_Spiral:{
            double d = 0;
            if (myOpt->isAWG){
                double AWG = mui->lineEdit_2->text().toDouble();
                d = 0.127 * pow(92, (36 - AWG) / 39);
            } else {
                d = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
            }
            data->d = d;
            break;
        }
        default:
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_3_editingFinished()
{
    if (!mui->lineEdit_3->text().isEmpty()){
        switch (FormCoil) {
        case _Onelayer:{
            data->k = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Onelayer_p:{
            data->t = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Multilayer:
        case _Multilayer_p:{
            double d = 0;
            if (myOpt->isAWG){
                double AWG = mui->lineEdit_3->text().toDouble();
                d = 0.127 * pow(92, (36 - AWG) / 39);
            } else {
                d = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
            }
            data->d = d;
            double k_m = odCalc(d);
            if (d > 0){
                mui->lineEdit_4->setText( QString::number(k_m / myOpt->dwLengthMultiplier));
            }
            break;
        }
        case _FerrToroid:{
            data->h = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Flat_Spiral:{
            data->s = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        default:
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_4_editingFinished()
{
    if (!mui->lineEdit_4->text().isEmpty()){
        switch (FormCoil) {
        case _Onelayer:{
            data->p = mui->lineEdit_4->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Onelayer_p:{
            data->isol = mui->lineEdit_4->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Multilayer:
        case _Multilayer_p:{
            data->k = mui->lineEdit_4->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _FerrToroid:{
            double d = 0;
            if (myOpt->isAWG){
                double AWG = mui->lineEdit_4->text().toDouble();
                d = 0.127 * pow(92, (36 - AWG) / 39);
            } else {
                d = mui->lineEdit_4->text().toDouble()*myOpt->dwLengthMultiplier;
            }
            data->d = d;
            break;
        }
        default:
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_5_editingFinished()
{
    if (!mui->lineEdit_5->text().isEmpty()){
        switch (FormCoil) {
        case _Onelayer_p:{
            data->p = mui->lineEdit_5->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Multilayer_p:{
            data->g = mui->lineEdit_5->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _FerrToroid:{
            data->mu = mui->lineEdit_5->text().toDouble();
            break;
        }
        default:
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_6_editingFinished()
{
    if (!mui->lineEdit_6->text().isEmpty()){
        switch (FormCoil) {
        case _Multilayer_p:{
            data->Ng = mui->lineEdit_6->text().toDouble();
            break;
        }
        default:
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    data->ratio = (double)value / 100;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_N_editingFinished()
{
    if (!mui->lineEdit_N->text().isEmpty())
        data->N = mui->lineEdit_N->text().toDouble();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_freq2_editingFinished()
{
    if (!mui->lineEdit_freq2->text().isEmpty())
        data->frequency = mui->lineEdit_freq2->text().toDouble()*myOpt->dwFrequencyMultiplier;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_1_2_editingFinished()
{
    if (!mui->lineEdit_1_2->text().isEmpty()){
        switch (FormCoil) {
        case _Onelayer:
        case _Onelayer_p:
        case _Multilayer:
        case _Multilayer_p:{
            data->D = mui->lineEdit_1_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _FerrToroid:
        case _PCB_square:
        case _Flat_Spiral:{
            data->Do = mui->lineEdit_1_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        default:
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_2_2_editingFinished()
{
    if (!mui->lineEdit_2_2->text().isEmpty()){
        switch (FormCoil) {
        case _Onelayer:{
            double d = 0;
            if (myOpt->isAWG){
                double AWG = mui->lineEdit_2_2->text().toDouble();
                d = 0.127 * pow(92, (36 - AWG) / 39);
            } else {
                d = mui->lineEdit_2_2->text().toDouble()*myOpt->dwLengthMultiplier;
            }
            data->d = d;
            double k_m = odCalc(d);
            if (d > 0){
                mui->lineEdit_3_2->setText( QString::number(k_m/myOpt->dwLengthMultiplier));
            }
            break;
        }
        case _Onelayer_p:{
            data->w = mui->lineEdit_2_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Multilayer:
        case _Multilayer_p:{
            data->l = mui->lineEdit_2_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _FerrToroid:
        case _PCB_square:
        case _Flat_Spiral:{
            data->Di = mui->lineEdit_2_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        default:
            break;
        }
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_3_2_editingFinished()
{
    if (!mui->lineEdit_3_2->text().isEmpty()){
        switch (FormCoil) {
        case _Onelayer:{
            data->k = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Onelayer_p:{
            data->t = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Multilayer:
        case _Multilayer_p:{
            data->c = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _FerrToroid:{
            data->h = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _PCB_square:{
            data->s = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Flat_Spiral:{
            double d = 0;
            if (myOpt->isAWG){
                double AWG = mui->lineEdit_3_2->text().toDouble();
                d = 0.127 * pow(92, (36 - AWG) / 39);
            } else {
                d = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
            }
            data->d = d;
            break;
        }
        default:
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_4_2_editingFinished()
{
    if (!mui->lineEdit_4_2->text().isEmpty()){
        switch (FormCoil) {
        case _Onelayer:{
            data->p = mui->lineEdit_4_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Onelayer_p:{
            data->isol = mui->lineEdit_4_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Multilayer:{
            if (mui->radioButton_7->isChecked()){
                double d = 0;
                if (myOpt->isAWG){
                    double AWG = mui->lineEdit_4_2->text().toDouble();
                    d = 0.127 * pow(92, (36 - AWG) / 39);
                } else {
                    d = mui->lineEdit_4_2->text().toDouble()*myOpt->dwLengthMultiplier;
                }
                data->d = d;
                double k_m = odCalc(d);
                if (d > 0){
                    mui->lineEdit_5_2->setText( QString::number(k_m/myOpt->dwLengthMultiplier));
                }
            } else {
                data->Rdc = mui->lineEdit_4_2->text().toDouble()*myOpt->dwLengthMultiplier;
            }
            break;
        }
        case _Multilayer_p:{
            double d = 0;
            if (myOpt->isAWG){
                double AWG = mui->lineEdit_4_2->text().toDouble();
                d = 0.127 * pow(92, (36 - AWG) / 39);
            } else {
                d = mui->lineEdit_4_2->text().toDouble()*myOpt->dwLengthMultiplier;
            }
            data->d = d;
            double k_m = odCalc(d);
            if (d > 0){
                mui->lineEdit_5_2->setText( QString::number(k_m/myOpt->dwLengthMultiplier));
            }
            break;
        }
        case _FerrToroid:{
            data->mu = mui->lineEdit_4_2->text().toDouble();
            break;
        }
        case _PCB_square:{
            data->w = mui->lineEdit_4_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        default:
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_5_2_editingFinished()
{
    if (!mui->lineEdit_5_2->text().isEmpty()){
        switch (FormCoil) {
        case _Onelayer_p:{
            data->p = mui->lineEdit_5_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        case _Multilayer:
        case _Multilayer_p:{
            data->k = mui->lineEdit_5_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        default:
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_6_2_editingFinished()
{
    if (!mui->lineEdit_6_2->text().isEmpty()){
        switch (FormCoil) {
        case _Multilayer_p:{
            data->g = mui->lineEdit_6_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        default:
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_7_2_editingFinished()
{
    if (!mui->lineEdit_7_2->text().isEmpty()){
        switch (FormCoil) {
        case _Multilayer_p:{
            data->Ng = mui->lineEdit_7_2->text().toDouble()*myOpt->dwLengthMultiplier;
            break;
        }
        default:
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_pushButton_Calculate_clicked()
{
    int tab = mui->tabWidget->currentIndex();
    mui->statusBar->clearMessage();
    try{
        switch (tab) {
        case 0:{
            if ((mui->lineEdit_ind->text().isEmpty())||(mui->lineEdit_1->text().isEmpty())||(mui->lineEdit_2->text().isEmpty())){
                showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                return;
            }
            switch (FormCoil) {
            case _Onelayer:{
                if (mui->lineEdit_freq->text().isEmpty()||(mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                double I = mui->lineEdit_ind->text().toDouble()*myOpt->dwInductanceMultiplier;
                double f = mui->lineEdit_freq->text().toDouble()*myOpt->dwFrequencyMultiplier;
                double D = mui->lineEdit_1->text().toDouble()*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    double AWG = mui->lineEdit_2->text().toDouble();
                    d = 0.127 * pow(92, (36 - AWG) / 39);

                } else {
                    d = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
                }
                double k = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
                double p = mui->lineEdit_4->text().toDouble()*myOpt->dwLengthMultiplier;
                if ((I == 0)||(D == 0)||(d == 0)||(k == 0)||(p == 0)||(f == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (k < d){
                    showWarning(tr("Warning"), "k < d");
                    return;
                }
                if (p < k){
                    showWarning(tr("Warning"), "p < k");
                    return;
                }
                data->inductance = I;
                data->frequency = f;
                double Dk = D + k;
                Material mt = Cu;
                if (mui->radioButton_2->isChecked()){
                    mt = Ag;
                }
                else if (mui->radioButton_3->isChecked()){
                    mt = Al;
                }
                else if (mui->radioButton_4->isChecked()){
                    mt = Ti;
                }
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, d, p, I, f, 0, 0, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerN_roundW_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Onelayer_p:{
                if ((mui->lineEdit_freq->text().isEmpty())||(mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty()) ||(mui->lineEdit_5->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                double I = mui->lineEdit_ind->text().toDouble()*myOpt->dwInductanceMultiplier;
                double f = mui->lineEdit_freq->text().toDouble()*myOpt->dwFrequencyMultiplier;
                double D = mui->lineEdit_1->text().toDouble()*myOpt->dwLengthMultiplier;
                double w = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double t = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
                double ins = mui->lineEdit_4->text().toDouble()*myOpt->dwLengthMultiplier;
                double p = mui->lineEdit_5->text().toDouble()*myOpt->dwLengthMultiplier;
                if ((I == 0)||(D == 0)||(w == 0)||(t == 0)||(p == 0)||(f == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (p < (w + 2*ins)){
                    showWarning(tr("Warning"), "p < (w + 2i)");
                    return;
                }
                data->inductance = I;
                data->frequency = f;
                double Dk = D + t + ins;
                Material mt = Cu;
                if (mui->radioButton_2->isChecked()){
                    mt = Ag;
                }
                else if (mui->radioButton_3->isChecked()){
                    mt = Al;
                }
                else if (mui->radioButton_4->isChecked()){
                    mt = Ti;
                }
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, w, t, p, I, f, 0, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerN_rectW_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Multilayer:{
                if ((mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                double I = mui->lineEdit_ind->text().toDouble()*myOpt->dwInductanceMultiplier;
                double D = mui->lineEdit_1->text().toDouble()*myOpt->dwLengthMultiplier;
                double l = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    double AWG = mui->lineEdit_3->text().toDouble();
                    d = 0.127 * pow(92, (36 - AWG) / 39);

                } else {
                    d = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
                }
                double k = mui->lineEdit_4->text().toDouble()*myOpt->dwLengthMultiplier;
                if ((I == 0)||(D == 0)||(l == 0)||(d == 0)||(k == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (k < d){
                    showWarning(tr("Warning"), "k < d");
                    return;
                }
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, D, d, k, l, 0, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerN_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Multilayer_p:{
                if ((mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                double I = mui->lineEdit_ind->text().toDouble()*myOpt->dwInductanceMultiplier;
                double D = mui->lineEdit_1->text().toDouble()*myOpt->dwLengthMultiplier;
                double l = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    double AWG = mui->lineEdit_3->text().toDouble();
                    d = 0.127 * pow(92, (36 - AWG) / 39);

                } else {
                    d = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
                }
                double k = mui->lineEdit_4->text().toDouble()*myOpt->dwLengthMultiplier;
                double gap = mui->lineEdit_5->text().toDouble()*myOpt->dwLengthMultiplier;
                double ng = mui->lineEdit_6->text().toDouble()*myOpt->dwLengthMultiplier;
                if ((I == 0)||(D == 0)||(l == 0)||(d == 0)||(k == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (k < d){
                    showWarning(tr("Warning"), "k < d");
                    return;
                }
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, D, d, k, l, gap, ng );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerNgap_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _FerrToroid:{
                if ((mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                double I = mui->lineEdit_ind->text().toDouble()*myOpt->dwInductanceMultiplier;
                double D1 = mui->lineEdit_1->text().toDouble()*myOpt->dwLengthMultiplier;
                double D2 = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double h = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    double AWG = mui->lineEdit_4->text().toDouble();
                    d = 0.127 * pow(92, (36 - AWG) / 39);

                } else {
                    d = mui->lineEdit_4->text().toDouble()*myOpt->dwLengthMultiplier;
                }
                double mu = mui->lineEdit_5->text().toDouble();
                if ((I == 0)||(D1 == 0)||(D2 == 0)||(h == 0)||(d == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (D1 < D2){
                    showWarning(tr("Warning"), "D1 < D2");
                    return;
                }
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, D1, D2, h, d, mu, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_ferrToroidN_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _PCB_square:{
                double I = mui->lineEdit_ind->text().toDouble()*myOpt->dwInductanceMultiplier;
                double D1 = mui->lineEdit_1->text().toDouble()*myOpt->dwLengthMultiplier;
                double D2 = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double ratio = (double)mui->horizontalSlider->value()/100;
                if (D1 < D2){
                    showWarning(tr("Warning"), "D1 < D2");
                    return;
                }
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, D1, D2, ratio, 0, 0, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_pcbN_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Flat_Spiral:{
                double I = mui->lineEdit_ind->text().toDouble()*myOpt->dwInductanceMultiplier;
                double Di = mui->lineEdit_1->text().toDouble()*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    double AWG = mui->lineEdit_2->text().toDouble();
                    d = 0.127 * pow(92, (36 - AWG) / 39);

                } else {
                    d = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
                }
                double s = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, Di, d, s, 0, 0, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_spiralN_Result(_CoilResult)));
                thread->start();
                break;
                break;
            }
            default:
                break;
            }
            mui->pushButton_Calculate->setEnabled(false);
            this->setCursor(Qt::WaitCursor);
            break;
        }
        case 1:{
            if ((mui->lineEdit_1_2->text().isEmpty())||(mui->lineEdit_2_2->text().isEmpty()) ||(mui->lineEdit_3_2->text().isEmpty())){
                showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                return;
            }
            switch (FormCoil) {
            case _Onelayer:{
                if (mui->lineEdit_N->text().isEmpty()||(mui->lineEdit_freq2->text().isEmpty())||(mui->lineEdit_4_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                double N = mui->lineEdit_N->text().toDouble();
                double f = mui->lineEdit_freq2->text().toDouble()*myOpt->dwFrequencyMultiplier;
                double D = mui->lineEdit_1_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    double AWG = mui->lineEdit_2_2->text().toDouble();
                    d = 0.127 * pow(92, (36 - AWG) / 39);

                } else {
                    d = mui->lineEdit_2_2->text().toDouble()*myOpt->dwLengthMultiplier;
                }
                double k = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double p = mui->lineEdit_4_2->text().toDouble()*myOpt->dwLengthMultiplier;
                if ((N == 0)||(D == 0)||(d == 0)||(k == 0)||(p == 0)||(f == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (k < d){
                    showWarning(tr("Warning"), "k < d");
                    return;
                }
                if (p < k){
                    showWarning(tr("Warning"), "p < k");
                    return;
                }
                data->frequency = f;
                double Dk = D + k;
                Material mt = Cu;
                if (mui->radioButton_2_2->isChecked()){
                    mt = Ag;
                }
                else if (mui->radioButton_3_2->isChecked()){
                    mt = Al;
                }
                else if (mui->radioButton_4_2->isChecked()){
                    mt = Ti;
                }
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, d, p, N, f, 0, 0, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerI_roundW_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Onelayer_p:{
                if ((mui->lineEdit_N->text().isEmpty())||(mui->lineEdit_freq2->text().isEmpty()) ||(mui->lineEdit_4_2->text().isEmpty())
                        ||(mui->lineEdit_5_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                double N = mui->lineEdit_N->text().toDouble();
                double f = mui->lineEdit_freq2->text().toDouble()*myOpt->dwFrequencyMultiplier;
                double D = mui->lineEdit_1_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double w = mui->lineEdit_2_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double t = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double ins = mui->lineEdit_4_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double p = mui->lineEdit_5_2->text().toDouble()*myOpt->dwLengthMultiplier;
                if ((N == 0)||(D == 0)||(w == 0)||(t == 0)||(p == 0)||(f == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (p < w){
                    showWarning(tr("Warning"), "p < w");
                    return;
                }
                data->frequency = f;
                double Dk = D + t + ins;
                Material mt = Cu;
                if (mui->radioButton_2_2->isChecked()){
                    mt = Ag;
                }
                else if (mui->radioButton_3_2->isChecked()){
                    mt = Al;
                }
                else if (mui->radioButton_4_2->isChecked()){
                    mt = Ti;
                }
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, w, t, p, N, f, 0, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerI_rectW_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Multilayer:{
                if ((mui->lineEdit_4_2->text().isEmpty())||(mui->lineEdit_5_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                double D = mui->lineEdit_1_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double l = mui->lineEdit_2_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double c = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double d = 0;
                double Rm = 0;
                if (mui->radioButton_7->isChecked()){
                    if (myOpt->isAWG){
                        double AWG = mui->lineEdit_4_2->text().toDouble();
                        d = 0.127 * pow(92, (36 - AWG) / 39);

                    } else {
                        d = mui->lineEdit_4_2->text().toDouble()*myOpt->dwLengthMultiplier;
                    }
                } else if (mui->radioButton_8->isChecked()){
                    Rm = mui->lineEdit_4_2->text().toDouble();
                }
                double k = mui->lineEdit_5_2->text().toDouble()*myOpt->dwLengthMultiplier;
                if ((D == 0)||(c == 0)||(l == 0)||(k == 0)||((d == 0) && (mui->radioButton_7->isChecked()))
                        ||((Rm == 0) && (mui->radioButton_8->isChecked())) ){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (k < d){
                    showWarning(tr("Warning"), "k < d");
                    return;
                }
                if (mui->radioButton_7->isChecked()){
                    MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, l, c, d, k, 0, 0 );
                    connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerI_Result(_CoilResult)));
                    thread->start();
                } else if (mui->radioButton_8->isChecked()){
                    MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, l, c, Rm, k, 0, 2 );
                    connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerI_Result(_CoilResult)));
                    thread->start();
                }
                break;
            }
            case _Multilayer_p:{
                if ((mui->lineEdit_4_2->text().isEmpty())||(mui->lineEdit_5_2->text().isEmpty())||(mui->lineEdit_6_2->text().isEmpty())
                        ||(mui->lineEdit_7_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                double D = mui->lineEdit_1_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double l = mui->lineEdit_2_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double c = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    double AWG = mui->lineEdit_4_2->text().toDouble();
                    d = 0.127 * pow(92, (36 - AWG) / 39);

                } else {
                    d = mui->lineEdit_4_2->text().toDouble()*myOpt->dwLengthMultiplier;
                }
                double k = mui->lineEdit_5_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double g = mui->lineEdit_6_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double Ng = mui->lineEdit_7_2->text().toDouble();
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, l, c, d, k, g, Ng );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerIp_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _FerrToroid:{
                if (mui->lineEdit_N->text().isEmpty()||(mui->lineEdit_4_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                double N = mui->lineEdit_N->text().toDouble();
                double D1 = mui->lineEdit_1_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double D2 = mui->lineEdit_2_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double h = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double mu = mui->lineEdit_4_2->text().toDouble();
                if ((N == 0)||(D1 == 0)||(D2 == 0)||(h == 0)||(mu == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (D1 < D2){
                    showWarning(tr("Warning"), "D1 < D2");
                    return;
                }
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, N, D1, D2, h, mu, 0, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_ferriteI_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _PCB_square:{
                if (mui->lineEdit_N->text().isEmpty()||(mui->lineEdit_4_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                double N = mui->lineEdit_N->text().toDouble();
                double D1 = mui->lineEdit_1_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double D2 = mui->lineEdit_2_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double s = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double W = mui->lineEdit_4_2->text().toDouble()*myOpt->dwLengthMultiplier;
                if ((N == 0)||(D1 == 0)||(D2 == 0)||(s == 0)||(W == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (D1 < D2){
                    showWarning(tr("Warning"), "D1 < D2");
                    return;
                }
                if (s < W){
                    showWarning(tr("Warning"), "s < W");
                    return;
                }
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, N, D1, D2, s, W, 0, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_pcbI_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Flat_Spiral:{
                if (mui->lineEdit_N->text().isEmpty()){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                double N = mui->lineEdit_N->text().toDouble();
                double D1 = mui->lineEdit_1_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double D2 = mui->lineEdit_2_2->text().toDouble()*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    double AWG = mui->lineEdit_3_2->text().toDouble();
                    d = 0.127 * pow(92, (36 - AWG) / 39);

                } else {
                    d = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
                }
                if ((N == 0)||(D1 == 0)||(D2 == 0)||(d == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (D1 < D2){
                    showWarning(tr("Warning"), "D1 < D2");
                    return;
                }
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, N, D1, D2, d, 0, 0, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_spiralI_Result(_CoilResult)));
                thread->start();
                break;
            }
            default:
                break;
            }
            mui->pushButton_Calculate->setEnabled(false);
            this->setCursor(Qt::WaitCursor);
            break;
        }
        case 2:{
            if ((mui->lineEdit_1_3->text().isEmpty()) || (mui->lineEdit_2_3->text().isEmpty())){
                showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                return;
            }
            if ((mui->lineEdit_1_3->text() == "0") || (mui->lineEdit_2_3->text() == "0")){
                showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                return;
            }
            QTextCursor c = mui->textBrowser->textCursor();
            c.movePosition(QTextCursor::Start);
            calc_count++;
            c.insertHtml(QString::number(calc_count));
            QString Input = "<hr><h2>" + windowTitle() + " - " + tr("LC tank calculation") + "</h2><br/>";
            if (myOpt->isInsertImage){
                Input += "<img src=\":/images/res/LC.png\">";
            }
            Input += "<p><u>" + tr("Input") + ":</u><br/>";
            Input += mui->groupBox_1_3->title() + ": " + mui->lineEdit_1_3->text() + " " + mui->label_01_3->text() + "<br/>";
            Input += mui->groupBox_2_3->title() + ": " + mui->lineEdit_2_3->text() + " " + mui->label_02_3->text() + "</p>";
            c.insertHtml(Input);
            QString Result = "<hr>";
            Result += "<p><u>" + tr("Result") + ":</u><br/>";
            if (mui->radioButton_LC->isChecked()){
                data->inductance = mui->lineEdit_1_3->text().toDouble()*myOpt->dwInductanceMultiplier;
                data->capacitance = mui->lineEdit_2_3->text().toDouble()*myOpt->dwCapacityMultiplier;
                data->frequency = CalcCONTUR0(data->inductance, data->capacitance);
                Result += tr("Frequency of a circuit") + " f = " + QString::number(data->frequency / myOpt->dwFrequencyMultiplier, 'f', myOpt->dwAccuracy)
                        + " " + qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()) + "<br/>";
            } else if (mui->radioButton_CF->isChecked()){
                data->capacitance = mui->lineEdit_1_3->text().toDouble()*myOpt->dwCapacityMultiplier;
                data->frequency = mui->lineEdit_2_3->text().toDouble()*myOpt->dwFrequencyMultiplier;
                data->inductance = CalcCONTUR1(data->capacitance, data->frequency);
                Result += tr("Inductance of a circuit") + " L = " + QString::number(data->inductance / myOpt->dwInductanceMultiplier, 'f', myOpt->dwAccuracy)
                        + " " + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()) + "<br/>";
            } else if (mui->radioButton_LF->isChecked()){
                data->inductance = mui->lineEdit_1_3->text().toDouble()*myOpt->dwInductanceMultiplier;
                data->frequency = mui->lineEdit_2_3->text().toDouble()*myOpt->dwFrequencyMultiplier;
                data->capacitance = CalcCONTUR2(data->inductance, data->frequency);
                Result += tr("Circuit capacitance") + " C = " + QString::number(data->capacitance / myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy)
                        + " " + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            }
            double ro = 1000 * sqrt(data->inductance / data->capacitance);
            Result += tr("Characteristic impedance") + " ρ = " + QString::number(ro, 'f', myOpt->dwAccuracy) + " " + tr("Ohm");
            Result += "</p><hr>";
            c.insertHtml(Result);
            mui->pushButton_Calculate->setEnabled(true);
            this->setCursor(Qt::ArrowCursor);
            break;
        }
        default:
            break;
        }
    } catch (...) {
        showWarning(tr("Warning"), tr("Wrong data was entered") + "!");
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GET NUMBER OF TURNS ROUTINES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerN_roundW_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil2.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_ind->text() + " " + mui->lineEdit_ind->text() + " " + mui->label_ind_m->text() + "<br/>";
    Input += mui->label_freq->text() + " " + mui->lineEdit_freq->text() + " " + mui->label_freq_m->text() + "<br/>";
    Input += mui->label_1->text() + " " + mui->lineEdit_1->text() + " " + mui->label_01->text() + "<br/>";
    Input += mui->label_2->text() + " " + mui->lineEdit_2->text() + " " + mui->label_02->text() + "<br/>";
    Input += mui->label_3->text() + " " + mui->lineEdit_3->text() + " " + mui->label_03->text() + "<br/>";
    Input += mui->label_4->text() + " " + mui->lineEdit_4->text() + " " + mui->label_04->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    double I = mui->lineEdit_ind->text().toDouble()*myOpt->dwInductanceMultiplier;
    double d = 0;
    if (myOpt->isAWG){
        double AWG = mui->lineEdit_2->text().toDouble();
        d = 0.127 * pow(92, (36 - AWG) / 39);

    } else {
        d = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
    }
    double p = mui->lineEdit_4->text().toDouble()*myOpt->dwLengthMultiplier;
    double f = mui->lineEdit_freq->text().toDouble()*myOpt->dwFrequencyMultiplier;
    data->N = result.N;
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    Result += tr("Number of turns of the coil") + " N = " + QString::number(result.N, 'f', myOpt->dwAccuracy) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + QString::number(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Length of winding") + " l = " + QString::number( (result.N * p + d)/myOpt->dwLengthMultiplier, 'f', myOpt->dwAccuracy ) + " " +
            qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double mass = 2.225 * M_PI * d * d * result.sec;
    Result += tr("Weight of wire") + " m = " + QString::number(mass) + " " + tr("g") + "<br/>";
    double reactance = 2 * M_PI * I * f;
    Result += tr("Reactance of the coil") + " X = " + QString::number(reactance, 'f', myOpt->dwAccuracy) + " " + tr("Ohm");
    Result += "<br/><br/>";
    if (f < 0.7 * result.fourth){
        Result += tr("Self capacitance") + " Cs = " + QString::number(result.thd/myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
        Result += tr("Coil self-resonance frequency") + " Fsr = " + QString::number(result.fourth/myOpt->dwFrequencyMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()) + "<br/>";
        double Ql = (double)result.six;
        Result += tr("Coil constructive Q-factor") + " Q = " + QString::number(result.six) + "<br/>";
        double capacitance = CalcCONTUR2(I, f);
        double ESR = (1e3 * sqrt(I / capacitance)) / Ql;
        Result += tr("Loss resistance") + " ESR = " + QString::number(ESR, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "</p>";
        if(myOpt->isAdditionalResult){
            Result += "<hr><p>";
            Result += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcCONTUR2(I, f);
            Result += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    QString::number((data->capacitance - result.thd) / myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            Result += " => " + tr("Characteristic impedance") + ": ρ = " + QString::number(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            Result += " => "  + tr("Equivalent resistance") + ": Re = " + QString::number(Re / 1000, 'f', myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * data->frequency / Qs;
            Result += " => " + tr("Bandwidth") + ": 3dBΔf = " + QString::number(deltaf, 'f', myOpt->dwAccuracy) + tr("kHz");
        }
    } else {
        mui->statusBar->showMessage(tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!");
    }
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerN_rectW_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil2_square.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_ind->text() + " " + mui->lineEdit_ind->text() + " " + mui->label_ind_m->text() + "<br/>";
    Input += mui->label_freq->text() + " " + mui->lineEdit_freq->text() + " " + mui->label_freq_m->text() + "<br/>";
    Input += mui->label_1->text() + " " + mui->lineEdit_1->text() + " " + mui->label_01->text() + "<br/>";
    Input += mui->label_2->text() + " " + mui->lineEdit_2->text() + " " + mui->label_02->text() + "<br/>";
    Input += mui->label_3->text() + " " + mui->lineEdit_3->text() + " " + mui->label_03->text() + "<br/>";
    Input += mui->label_4->text() + " " + mui->lineEdit_4->text() + " " + mui->label_04->text() + "<br/>";
    Input += mui->label_5->text() + " " + mui->lineEdit_5->text() + " " + mui->label_05->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    double I = mui->lineEdit_ind->text().toDouble()*myOpt->dwInductanceMultiplier;
    double w = mui->lineEdit_2->text().toDouble()*myOpt->dwLengthMultiplier;
    double t = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
    double p = mui->lineEdit_5->text().toDouble()*myOpt->dwLengthMultiplier;
    double f = mui->lineEdit_freq->text().toDouble()*myOpt->dwFrequencyMultiplier;
    data->N = result.N;
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    Result += tr("Number of turns of the coil") + " N = " + QString::number(result.N, 'f', myOpt->dwAccuracy) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + QString::number(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Length of winding") + " l = " + QString::number( (result.N * p + w)/myOpt->dwLengthMultiplier, 'f', myOpt->dwAccuracy ) + " " +
            qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double mass = 8.9 * w * t * result.sec;
    Result += tr("Weight of wire") + " m = " + QString::number(mass) + " " + tr("g") + "<br/>";
    double reactance = 2 * M_PI * I * f;
    Result += tr("Reactance of the coil") + " X = " + QString::number(reactance, 'f', myOpt->dwAccuracy) + " " + tr("Ohm");
    Result += "<br/><br/>";
    if (f < 0.7 * result.fourth){
        Result += tr("Self capacitance") + " Cs = " + QString::number(result.thd/myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
        Result += tr("Coil self-resonance frequency") + " Fsr = " + QString::number(result.fourth/myOpt->dwFrequencyMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()) + "<br/>";
        double Ql = (double)result.six;
        Result += tr("Coil constructive Q-factor") + " Q = " + QString::number(result.six) + "<br/>";
        double Capacity = CalcCONTUR2(I, f);
        double ESR = (1e3 * sqrt(I / Capacity)) / Ql;
        Result += tr("Loss resistance") + " ESR = " + QString::number(ESR, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "</p>";
        if(myOpt->isAdditionalResult){
            Result += "<hr><p>";
            Result += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcCONTUR2(I, f);
            Result += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    QString::number((data->capacitance - result.thd) / myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            Result += " => " + tr("Characteristic impedance") + ": ρ = " + QString::number(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            Result += " => "  + tr("Equivalent resistance") + ": Re = " + QString::number(Re / 1000, 'f', myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * data->frequency / Qs;
            Result += " => " + tr("Bandwidth") + ": 3dBΔf = " + QString::number(deltaf, 'f', myOpt->dwAccuracy) + tr("kHz");
        }
    } else {
        mui->statusBar->showMessage(tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!");
    }
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerN_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil4.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_ind->text() + " " + mui->lineEdit_ind->text() + " " + mui->label_ind_m->text() + "<br/>";
    Input += mui->label_1->text() + " " + mui->lineEdit_1->text() + " " + mui->label_01->text() + "<br/>";
    Input += mui->label_2->text() + " " + mui->lineEdit_2->text() + " " + mui->label_02->text() + "<br/>";
    Input += mui->label_3->text() + " " + mui->lineEdit_3->text() + " " + mui->label_03->text() + "<br/>";
    Input += mui->label_4->text() + " " + mui->lineEdit_4->text() + " " + mui->label_04->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    data->N = result.six;
    Result += tr("Number of turns of the coil") + " N = " + QString::number(result.six) + "<br/>";
    data->c = result.fourth;
    Result += tr("Thickness of the coil") + " c = " + QString::number(result.fourth) + " "
            + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double D = mui->lineEdit_1->text().toDouble()*myOpt->dwLengthMultiplier;
    double width = (2 * result.fourth + D) / myOpt->dwLengthMultiplier;
    Result += tr("Dimensions of inductor") + ": " + mui->lineEdit_2->text() + "x" + QString::number(ceil(width))
            + "x" + QString::number(ceil(width)) + " " + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    data->Rdc = result.N;
    Result += tr("Resistance of the coil") + " R = " + QString::number(result.N, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + QString::number(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double d = 0;
    if (myOpt->isAWG){
        double AWG = mui->lineEdit_3->text().toDouble();
        d = 0.127 * pow(92, (36 - AWG) / 39);

    } else {
        d = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
    }
    double mass = 2.225 * M_PI * d * d * result.sec;
    Result += tr("Weight of wire") + " m = " + QString::number(mass) + " " + tr("g") + "<br/>";
    Result += tr("Number of layers") + " Nl = " + QString::number(result.thd);
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerNgap_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil4-0.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_ind->text() + " " + mui->lineEdit_ind->text() + " " + mui->label_ind_m->text() + "<br/>";
    Input += mui->label_1->text() + " " + mui->lineEdit_1->text() + " " + mui->label_01->text() + "<br/>";
    Input += mui->label_2->text() + " " + mui->lineEdit_2->text() + " " + mui->label_02->text() + "<br/>";
    Input += mui->label_3->text() + " " + mui->lineEdit_3->text() + " " + mui->label_03->text() + "<br/>";
    Input += mui->label_4->text() + " " + mui->lineEdit_4->text() + " " + mui->label_04->text() + "<br/>";
    Input += mui->label_5->text() + " " + mui->lineEdit_5->text() + " " + mui->label_05->text() + "<br/>";
    Input += mui->label_6->text() + " " + mui->lineEdit_6->text() + " " + mui->label_06->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    data->N = result.six;
    Result += tr("Number of turns of the coil") + " N = " + QString::number(result.six) + "<br/>";
    Result += tr("Thickness of the coil") + " c = " + QString::number(result.fourth) + " "
            + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double D = mui->lineEdit_1->text().toDouble()*myOpt->dwLengthMultiplier;
    double width = (2 * result.fourth + D) / myOpt->dwLengthMultiplier;
    Result += tr("Dimensions of inductor") + ": " + mui->lineEdit_2->text() + "x" + QString::number(ceil(width))
            + "x" + QString::number(ceil(width)) + " " + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    data->Rdc = result.N;
    Result += tr("Resistance of the coil") + " R = " + QString::number(result.N, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + QString::number(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double d = 0;
    if (myOpt->isAWG){
        double AWG = mui->lineEdit_3->text().toDouble();
        d = 0.127 * pow(92, (36 - AWG) / 39);

    } else {
        d = mui->lineEdit_3->text().toDouble()*myOpt->dwLengthMultiplier;
    }
    double mass = 2.225 * M_PI * d * d * result.sec;
    Result += tr("Weight of wire") + " m = " + QString::number(mass) + " " + tr("g") + "<br/>";
    Result += tr("Number of layers") + " Nl = " + QString::number(result.thd) + "<br/>";
    Result += tr("Number of interlayers") + " Ng = " + QString::number(result.five);
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_ferrToroidN_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil6.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_ind->text() + " " + mui->lineEdit_ind->text() + " " + mui->label_ind_m->text() + "<br/>";
    Input += mui->label_1->text() + " " + mui->lineEdit_1->text() + " " + mui->label_01->text() + "<br/>";
    Input += mui->label_2->text() + " " + mui->lineEdit_2->text() + " " + mui->label_02->text() + "<br/>";
    Input += mui->label_3->text() + " " + mui->lineEdit_3->text() + " " + mui->label_03->text() + "<br/>";
    Input += mui->label_4->text() + " " + mui->lineEdit_4->text() + " " + mui->label_04->text() + "<br/>";
    Input += mui->label_5->text() + " " + mui->lineEdit_5->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    if (result.sec > 0) {
        QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
        QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
        QString d_wire_length = list[0];
        QString _ssLengthMeasureUnit = list[1];
        Result += tr("Length of wire without leads") + " lw = " + QString::number(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
                qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
        data->N = result.N;
        Result += tr("Number of turns of the coil") + " N = " + QString::number(result.N, 'f', myOpt->dwAccuracy);
    } else {
        Result += "<span style=\"color:red;\">" + tr("Coil can not be realized") + "!";
        mui->statusBar->showMessage(tr("Coil can not be realized") + "!");
    }
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_pcbN_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil8.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_ind->text() + " " + mui->lineEdit_ind->text() + " " + mui->label_ind_m->text() + "<br/>";
    Input += mui->label_1->text() + " " + mui->lineEdit_1->text() + " " + mui->label_01->text() + "<br/>";
    Input += mui->label_2->text() + " " + mui->lineEdit_2->text() + " " + mui->label_02->text() + "<br/>";
    Input += mui->groupBox_6->title() + " " + QString::number((double)mui->horizontalSlider->value()/100) + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    data->N = result.N;
    Result += tr("Number of turns of the coil") + " N = " + QString::number(result.N, 'f', myOpt->dwAccuracy) + "<br/>";
    Result += tr("Winding pitch") + " s = " + QString::number(result.sec, 'f', myOpt->dwAccuracy) + " " + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Width of a PCB trace") + " W = " + QString::number(result.thd, 'f', myOpt->dwAccuracy) + " " + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8());
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_spiralN_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil10.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_ind->text() + " " + mui->lineEdit_ind->text() + " " + mui->label_ind_m->text() + "<br/>";
    Input += mui->label_1->text() + " " + mui->lineEdit_1->text() + " " + mui->label_01->text() + "<br/>";
    Input += mui->label_2->text() + " " + mui->lineEdit_2->text() + " " + mui->label_02->text() + "<br/>";
    Input += mui->label_3->text() + " " + mui->lineEdit_3->text() + " " + mui->label_03->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    data->N = result.N;
    Result += tr("Number of turns of the coil") + " N = " + QString::number(result.N) + "<br/>";
    Result += tr("Outside diameter") + " Do = " + QString::number(result.thd, 'f', myOpt->dwAccuracy) + " "
            + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + QString::number(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "</p>";
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GET INDUCTANCE ROUTINES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerI_roundW_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil2.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_N->text() + " " + mui->lineEdit_N->text() + "<br/>";
    Input += mui->label_freq2->text() + " " + mui->lineEdit_freq2->text() + " " + mui->label_freq_m2->text() + "<br/>";
    Input += mui->label_1_2->text() + " " + mui->lineEdit_1_2->text() + " " + mui->label_01_2->text() + "<br/>";
    Input += mui->label_2_2->text() + " " + mui->lineEdit_2_2->text() + " " + mui->label_02_2->text() + "<br/>";
    Input += mui->label_3_2->text() + " " + mui->lineEdit_3_2->text() + " " + mui->label_03_2->text() + "<br/>";
    Input += mui->label_4_2->text() + " " + mui->lineEdit_4_2->text() + " " + mui->label_04_2->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    double N = mui->lineEdit_N->text().toDouble();
    double d = 0;
    if (myOpt->isAWG){
        double AWG = mui->lineEdit_2_2->text().toDouble();
        d = 0.127 * pow(92, (36 - AWG) / 39);

    } else {
        d = mui->lineEdit_2_2->text().toDouble()*myOpt->dwLengthMultiplier;
    }
    double p = mui->lineEdit_4_2->text().toDouble()*myOpt->dwLengthMultiplier;
    double f = mui->lineEdit_freq2->text().toDouble()*myOpt->dwFrequencyMultiplier;
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    double I = result.N;
    data->inductance = result.N;
    Result += tr("Inductance") + " L = " + QString::number(I/myOpt->dwInductanceMultiplier, 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + QString::number(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Length of winding") + " l = " + QString::number( (N * p + d)/myOpt->dwLengthMultiplier, 'f', myOpt->dwAccuracy ) + " " +
            qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double mass = 2.225 * M_PI * d * d * result.sec;
    Result += tr("Weight of wire") + " m = " + QString::number(mass) + " " + tr("g") + "<br/>";
    double reactance = 2 * M_PI * I * f;
    Result += tr("Reactance of the coil") + " X = " + QString::number(reactance, 'f', myOpt->dwAccuracy) + " " + tr("Ohm");
    Result += "<br/><br/>";
    if (f < 0.7 * result.fourth){
        Result += tr("Self capacitance") + " Cs = " + QString::number(result.thd/myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
        Result += tr("Coil self-resonance frequency") + " Fsr = " + QString::number(result.fourth/myOpt->dwFrequencyMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()) + "<br/>";
        double Ql = (double)result.six;
        Result += tr("Coil constructive Q-factor") + " Q = " + QString::number(result.six) + "<br/>";
        double Capacity = CalcCONTUR2(I, f);
        double ESR = (1e3 * sqrt(I / Capacity)) / Ql;
        Result += tr("Loss resistance") + " ESR = " + QString::number(ESR, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "</p>";
        if(myOpt->isAdditionalResult){
            Result += "<hr><p>";
            Result += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcCONTUR2(I, f);
            Result += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    QString::number((data->capacitance - result.thd) / myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            Result += " => " + tr("Characteristic impedance") + ": ρ = " + QString::number(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            Result += " => "  + tr("Equivalent resistance") + ": Re = " + QString::number(Re / 1000, 'f', myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * f / Qs;
            Result += " => " + tr("Bandwidth") + ": 3dBΔf = " + QString::number(deltaf, 'f', myOpt->dwAccuracy) + tr("kHz");
        }
    } else {
        mui->statusBar->showMessage(tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!");
    }
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerI_rectW_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil2_square.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_N->text() + " " + mui->lineEdit_N->text() + "<br/>";
    Input += mui->label_freq2->text() + " " + mui->lineEdit_freq2->text() + " " + mui->label_freq_m2->text() + "<br/>";
    Input += mui->label_1_2->text() + " " + mui->lineEdit_1_2->text() + " " + mui->label_01_2->text() + "<br/>";
    Input += mui->label_2_2->text() + " " + mui->lineEdit_2_2->text() + " " + mui->label_02_2->text() + "<br/>";
    Input += mui->label_3_2->text() + " " + mui->lineEdit_3_2->text() + " " + mui->label_03_2->text() + "<br/>";
    Input += mui->label_4_2->text() + " " + mui->lineEdit_4_2->text() + " " + mui->label_04_2->text() + "<br/>";
    Input += mui->label_5_2->text() + " " + mui->lineEdit_5_2->text() + " " + mui->label_05_2->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    double N = mui->lineEdit_N->text().toDouble();
    double w = mui->lineEdit_2_2->text().toDouble()*myOpt->dwLengthMultiplier;
    double t = mui->lineEdit_3_2->text().toDouble()*myOpt->dwLengthMultiplier;
    double p = mui->lineEdit_5_2->text().toDouble()*myOpt->dwLengthMultiplier;
    double f = mui->lineEdit_freq2->text().toDouble()*myOpt->dwFrequencyMultiplier;
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    double I = result.N;
    data->inductance = result.N;
    Result += tr("Inductance") + " L = " + QString::number(I/myOpt->dwInductanceMultiplier, 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + QString::number(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Length of winding") + " l = " + QString::number( (N * p + w)/myOpt->dwLengthMultiplier, 'f', myOpt->dwAccuracy ) + " " +
            qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double mass = 8.9 * w * t * result.sec;
    Result += tr("Weight of wire") + " m = " + QString::number(mass) + " " + tr("g") + "<br/>";
    double reactance = 2 * M_PI * I * f;
    Result += tr("Reactance of the coil") + " X = " + QString::number(reactance, 'f', myOpt->dwAccuracy) + " " + tr("Ohm");
    Result += "<br/><br/>";
    if (f < 0.7 * result.fourth){
        Result += tr("Self capacitance") + " Cs = " + QString::number(result.thd/myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
        Result += tr("Coil self-resonance frequency") + " Fsr = " + QString::number(result.fourth/myOpt->dwFrequencyMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()) + "<br/>";
        double Ql = (double)result.six;
        Result += tr("Coil constructive Q-factor") + " Q = " + QString::number(result.six) + "<br/>";
        double Capacity = CalcCONTUR2(I, f);
        double ESR = (1e3 * sqrt(I / Capacity)) / Ql;
        Result += tr("Loss resistance") + " ESR = " + QString::number(ESR, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "</p>";
        if(myOpt->isAdditionalResult){
            Result += "<hr><p>";
            Result += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcCONTUR2(I, f);
            Result += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    QString::number((data->capacitance - result.thd) / myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            Result += " => " + tr("Characteristic impedance") + ": ρ = " + QString::number(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);// Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            Result += " => "  + tr("Equivalent resistance") + ": Re = " + QString::number(Re / 1000, 'f', myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * data->frequency / Qs;
            Result += " => " + tr("Bandwidth") + ": 3dBΔf = " + QString::number(deltaf, 'f', myOpt->dwAccuracy) + tr("kHz");
        }
    } else {
        mui->statusBar->showMessage(tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!");
    }
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerI_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil4.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_1_2->text() + " " + mui->lineEdit_1_2->text() + " " + mui->label_01_2->text() + "<br/>";
    Input += mui->label_2_2->text() + " " + mui->lineEdit_2_2->text() + " " + mui->label_02_2->text() + "<br/>";
    Input += mui->label_3_2->text() + " " + mui->lineEdit_3_2->text() + " " + mui->label_03_2->text() + "<br/>";
    Input += mui->label_4_2->text() + " " + mui->lineEdit_4_2->text() + " " + mui->label_04_2->text() + "<br/>";
    Input += mui->label_5_2->text() + " " + mui->lineEdit_5_2->text() + " " + mui->label_05_2->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    double N1 = result.thd;
    double N2 = result.fourth;
    double L1 = result.N;
    double L2 = result.sec;
    data->inductance = sqrt(L1*L2);
    Result += tr("Number of turns of the coil") + " N = " + QString::number(N1) + "..." + QString::number(N2) + "<br/>";
    Result += tr("Inductance") + " L = " + QString::number(L1, 'f', myOpt->dwAccuracy) + "..." + QString::number(L2, 'f', myOpt->dwAccuracy) + " "
            + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8());
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerIp_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil4-0.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_1_2->text() + " " + mui->lineEdit_1_2->text() + " " + mui->label_01_2->text() + "<br/>";
    Input += mui->label_2_2->text() + " " + mui->lineEdit_2_2->text() + " " + mui->label_02_2->text() + "<br/>";
    Input += mui->label_3_2->text() + " " + mui->lineEdit_3_2->text() + " " + mui->label_03_2->text() + "<br/>";
    Input += mui->label_4_2->text() + " " + mui->lineEdit_4_2->text() + " " + mui->label_04_2->text() + "<br/>";
    Input += mui->label_5_2->text() + " " + mui->lineEdit_5_2->text() + " " + mui->label_05_2->text() + "<br/>";
    Input += mui->label_6_2->text() + " " + mui->lineEdit_6_2->text() + " " + mui->label_06_2->text() + "<br/>";
    Input += mui->label_7_2->text() + " " + mui->lineEdit_7_2->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    double N1 = result.thd;
    double N2 = result.fourth;
    double L1 = result.N;
    double L2 = result.sec;
    data->inductance = sqrt(L1*L2);
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    Result += tr("Number of turns of the coil") + " N = " + QString::number(N1) + "..." + QString::number(N2) + "<br/>";
    Result += tr("Inductance") + " L = " + QString::number(L1, 'f', myOpt->dwAccuracy) + "..." + QString::number(L2, 'f', myOpt->dwAccuracy) + " "
            + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8());
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_ferriteI_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil6.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_N->text() + " " + mui->lineEdit_N->text() + "<br/>";
    Input += mui->label_1_2->text() + " " + mui->lineEdit_1_2->text() + " " + mui->label_01_2->text() + "<br/>";
    Input += mui->label_2_2->text() + " " + mui->lineEdit_2_2->text() + " " + mui->label_02_2->text() + "<br/>";
    Input += mui->label_3_2->text() + " " + mui->lineEdit_3_2->text() + " " + mui->label_03_2->text() + "<br/>";
    Input += mui->label_4_2->text() + " " + mui->lineEdit_4_2->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    data->inductance = result.N;
    Result += tr("Inductance") + " L = " + QString::number(result.N, 'f', myOpt->dwAccuracy) + " " + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8());
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_pcbI_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil8.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_N->text() + " " + mui->lineEdit_N->text() + "<br/>";
    Input += mui->label_1_2->text() + " " + mui->lineEdit_1_2->text() + " " + mui->label_01_2->text() + "<br/>";
    Input += mui->label_2_2->text() + " " + mui->lineEdit_2_2->text() + " " + mui->label_02_2->text() + "<br/>";
    Input += mui->label_3_2->text() + " " + mui->lineEdit_3_2->text() + " " + mui->label_03_2->text() + "<br/>";
    Input += mui->label_4_2->text() + " " + mui->lineEdit_4_2->text() + " " + mui->label_04_2->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    data->inductance = result.N;
    Result += tr("Inductance") + " L = " + QString::number(result.N, 'f', myOpt->dwAccuracy) + " " + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8());
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_spiralI_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil10.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_N->text() + " " + mui->lineEdit_N->text() + "<br/>";
    Input += mui->label_1_2->text() + " " + mui->lineEdit_1_2->text() + " " + mui->label_01_2->text() + "<br/>";
    Input += mui->label_2_2->text() + " " + mui->lineEdit_2_2->text() + " " + mui->label_02_2->text() + "<br/>";
    Input += mui->label_3_2->text() + " " + mui->lineEdit_3_2->text() + " " + mui->label_03_2->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    data->inductance = result.N;
    Result += tr("Inductance") + " L = " + QString::number(result.N, 'f', myOpt->dwAccuracy) + " "
            + (myOpt->ssInductanceMeasureUnit.toUtf8()) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + QString::number(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8());
    Result += "</p><hr>";
    c.insertHtml(Result);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionFerrite_toroid_permeability_triggered()
{
    Find_Permeability *fPerm = new Find_Permeability();
    fPerm->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(fPerm, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fPerm, SLOT(getOpt(_OptionStruct)));
    emit sendOpt(*myOpt);
    fPerm->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionCoil_on_a_ferrite_rod_triggered()
{
    Ferrite_Rod *fRod = new Ferrite_Rod();
    fRod->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(fRod, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fRod, SLOT(getOpt(_OptionStruct)));
    emit sendOpt(*myOpt);
    fRod->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::getAddCalculationResult(QString result){
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    calc_count++;
    c.insertHtml(QString::number(calc_count));
    c.insertHtml(result);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
