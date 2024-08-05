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
#include "resolve_q.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mui(new Ui::MainWindow)
{
    mui->setupUi(this);
    net_manager = new QNetworkAccessManager();
    net_manager->setNetworkAccessible(QNetworkAccessManager::Accessible);
    connect(net_manager, SIGNAL(finished(QNetworkReply*)), SLOT(checkAppVersion(QNetworkReply*)));
    QString title = qApp->applicationName();
    title.append(" v");
    title.append(qApp->applicationVersion());
    setWindowTitle(title);
    translator = new QTranslator(qApp);

    mui->groupBox->setVisible(false);
    mui->groupBox_2->setVisible(false);
    mui->groupBox_6->setVisible(false);
    mui->groupBox_7->setVisible(false);
    mui->comboBox_checkPCB->setVisible(false);
    mui->comboBox_checkMLWinding->setVisible(false);
    data = new _Data;
    myOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, MAX_DOUBLE, 380);
    awgV = new QRegExpValidator(QRegExp(AWG_REG_EX));
    iv = new QIntValidator(this);

    mui->lineEdit_1_3->setValidator(dv);
    mui->lineEdit_2_3->setValidator(dv);

    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "GUI" );
    int tab = settings->value( "tab", 0 ).toInt();
    lang = settings->value("lang", QString::fromLatin1(QLocale::system().name().toLatin1())).toString();
    if (lang.length() > 2)
        lang.truncate(lang.lastIndexOf('_'));
    int form_coil = settings->value( "form_coil", -1 ).toInt();
    int init_cond = settings->value( "init_cond", 0 ).toInt();
    int wire_material = settings->value( "wire_material", 0 ).toInt();
    int init_data = settings->value( "init_data", 0 ).toInt();
    myOpt->isInsertImage = settings->value( "isInsertImage", true ).toBool();
    myOpt->isShowTitle = settings->value( "isShowTitle", true ).toBool();
    myOpt->isShowLTSpice = settings->value( "isShowLTSpice", true ).toBool();
    myOpt->isConfirmExit = settings->value( "isConfirmExit", true ).toBool();
    myOpt->isConfirmClear = settings->value( "isConfirmClear", true ).toBool();
    myOpt->isConfirmDelete = settings->value( "isConfirmDelete", true ).toBool();
    myOpt->isOutputValueColored = settings->value( "isOutputValueColored", false ).toBool();
    myOpt->isOutputInTwoColumns = settings->value( "isOutputInTwoColumns", false ).toBool();
    myOpt->isShowValueDescription = settings->value( "isShowValueDescription", true ).toBool();
    myOpt->isDockWidgetFloating = settings->value( "isDockWidgetFloating", false ).toBool();
    myOpt->isDockWidgetVisible = settings->value( "isDockWidgetVisible", false ).toBool();
    myOpt->isShowCalcNum = settings->value( "isShowCalcNum", true ).toBool();
    myOpt->isAdditionalResult = settings->value( "isAdditionalResult", true ).toBool();
    myOpt->isAWGhasInsulation = settings->value( "isAWGwithInsulation", true ).toBool();
    myOpt->mainFontFamily = settings->value("MainFontFamily", QFontInfo(QFont()).family()).toString();
    myOpt->mainFontSize = settings->value("MainFontSize", QFontInfo(QFont()).pixelSize()).toInt();
    myOpt->textFontFamily = settings->value("TextFontFamily", QFontInfo(QFont()).family()).toString();
    myOpt->textFontSize = settings->value("TextFontSize", QFontInfo(QFont()).pixelSize()).toInt();
    myOpt->styleGUI = settings->value("styleGUI",0).toInt();
    myOpt->vTextColor = settings->value("valueTextColor", 0).toInt();
    myOpt->isEnglishLocale = settings->value( "isEnglishLocale", false ).toBool();
    myOpt->layoutPCBcoil = settings->value("layoutPCBcoil",true).toInt();
    myOpt->windingKind = settings->value("windingKind",true).toInt();
    myOpt->isWindingLengthOneLayerInit = settings->value("isWindingLengthOneLayerInit",false).toBool();
    myOpt->isSaveOnExit = settings->value("isSaveOnExit",true).toBool();
    myOpt->isLastShowingFirst = settings->value("isLastShowingFirst",true).toBool();
    if (myOpt->isSaveOnExit)
        calc_count = settings->value( "calc_count", 0 ).toInt();
    else
        this->calc_count = 0;
    settings->endGroup();

    translator->load(":/lang/res/translations/Coil64_" + lang);
    qApp->installTranslator(translator);

    //Start add language menu group if additional languages are available
    QDir dir(":/lang/res/translations");
    QFileInfoList dirContent = dir.entryInfoList(QStringList() << "*.qm",QDir::Files);
    if (!dirContent.isEmpty()){
        bool isLangInList = false;
        QStringList langName;
        QStringList langList = translateInstalling(&langName);
        QString installLang;
        QString fileStr;
        QList<QMenu*> menus = this->menuBar()->findChildren<QMenu*>();
        QMenu* child = menus.takeAt(1);
        child->addSeparator();
        QMenu* submenuLanguage = child->addMenu(tr("Language"));
        submenuLanguage->setObjectName("Language");
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
                        isLangInList = true;
                        if(myOpt->isEnglishLocale)loc = QLocale::English;
                        else loc = getLanguageLocale(lang);
                        this->setLocale(loc);
                        dv->setLocale(loc);
                    }
                    mAction->setObjectName(fileStr);
                    connect(mAction, SIGNAL(triggered()), this, SLOT(setLanguage()));
                }
            }
        }
        if (!isLangInList){
            if(myOpt->isEnglishLocale)loc = QLocale::English;
            else loc = QLocale::system();
            this->setLocale(loc);
            dv->setLocale(loc);
        }
    }
    //End add language menu group if additional languages are available

    mui->toolButton_showImg->setChecked(myOpt->isInsertImage);
    mui->toolButton_showTitle->setChecked(myOpt->isShowTitle);
    mui->toolButton_ltspice->setChecked(myOpt->isShowLTSpice);
    mui->toolButton_showAdditional->setChecked(myOpt->isAdditionalResult);
    mui->toolButton_cbc->setChecked(myOpt->isConfirmClear);
    mui->toolButton_cbe->setChecked(myOpt->isConfirmExit);
    mui->toolButton_cdsr->setChecked(myOpt->isConfirmDelete);
    mui->toolButton_soe->setChecked(myOpt->isSaveOnExit);
    mui->toolButton_lShowFirst->setChecked(myOpt->isLastShowingFirst);
    mui->toolButton_Color->setChecked(myOpt->isOutputValueColored);
    mui->toolButton_Desc->setChecked(myOpt->isShowValueDescription);
    mui->toolButton_column->setChecked(myOpt->isOutputInTwoColumns);
    mui->toolButton_Num->setChecked(myOpt->isShowCalcNum);

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
    data->B = settings->value("B", 0).toDouble();
    data->D = settings->value("D", 0).toDouble();
    data->a = settings->value("a", 0).toDouble();
    data->b = settings->value("b", 0).toDouble();
    data->d = settings->value("dw", 0).toDouble();
    data->k = settings->value("k", 0).toDouble();
    data->p = settings->value("p", 0).toDouble();
    data->t = settings->value("t", 0).toDouble();
    data->w = settings->value("w", 0).toDouble();
    data->ns = settings->value("ns", 6).toInt();
    data->isol = settings->value("i", 0).toDouble();
    data->l = settings->value("l", 0).toDouble();
    data->c = settings->value("c", 0).toDouble();
    data->Ch = settings->value("Ch", 0).toDouble();
    data->g = settings->value("g", 0).toDouble();
    data->Ng = settings->value("Ng", 0).toDouble();
    data->Do = settings->value("Do", 0).toDouble();
    data->Di = settings->value("Di", 0).toDouble();
    data->th = settings->value("th", 0.035).toDouble();
    data->h = settings->value("h", 0).toDouble();
    data->mu = settings->value("mu", 0).toDouble();
    data->ratio = settings->value("ratio",0.5).toDouble();
    data->zo = settings->value("zo",0).toDouble();
    data->s = settings->value("s", 0).toDouble();
    data->Rdc = settings->value("Rdc", 0).toDouble();
    settings->endGroup();



    satCurrentDockWidget = new SaturationDockWidget(this);
    satCurrentDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    satCurrentDockWidget->setFloating(myOpt->isDockWidgetFloating);
    addDockWidget(Qt::RightDockWidgetArea, satCurrentDockWidget);
    connect(this, SIGNAL(sendFerriteData(_FerriteData)), satCurrentDockWidget, SLOT(getFerriteData(_FerriteData)));
    connect(this, SIGNAL(sendOptToDock(_OptionStruct)), satCurrentDockWidget, SLOT(getOpt(_OptionStruct)));
    connect(satCurrentDockWidget, SIGNAL(sendClose()), this, SLOT(on_dockWidgetClosed()));
    emit sendOptToDock(*myOpt);
    if (myOpt->isDockWidgetVisible)
        mui->toolButton_Saturation->setChecked(true);
    else
        mui->toolButton_Saturation->setChecked(false);

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
        FormCoil = _Onelayer_cw;
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
    else if (init_cond == 3){
        mui->radioButton_ZF->setChecked(true);
        on_radioButton_ZF_clicked();
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
    if (init_data == 0) mui->radioButton_6->setChecked(true);
    if (init_data == 1) mui->radioButton_7->setChecked(true);
    if (init_data == 2) mui->radioButton_8->setChecked(true);

    completeOptionsStructure(myOpt);
    if ((settings->value("mainWindowGeometry").isValid()) && (settings->value("mainWindowState").isValid())){
        restoreGeometry(settings->value("mainWindowGeometry").toByteArray());
        restoreState(settings->value("mainWindowState").toByteArray());
    } else {
        this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->minimumSize(),qApp->primaryScreen()->availableGeometry()));
    }
    mui->retranslateUi(this);

    QAction *buttonAction = new QAction(mui->pushButton_Calculate);
    buttonAction->setShortcuts({QKeySequence("Enter"),QKeySequence("Return")});
    mui->pushButton_Calculate->addAction(buttonAction);
    connect(buttonAction, &QAction::triggered, mui->pushButton_Calculate, &QPushButton::click);

    mui->textBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mui->textBrowser, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_textBrowser_customContextMenuRequested(QPoint)));
    mui->textBrowser->installEventFilter(this);
    popupmenu = mui->textBrowser->createStandardContextMenu();
    popupmenu->clear();
    QAction *action0 = popupmenu->addAction(tr("Copy selected to clipboard"));
    QAction *action1 = popupmenu->addAction(tr("Copy all to clipboard"));
    popupmenu->addSeparator();
    QAction *action2 = popupmenu->addAction(tr("Save"));
    QAction *action3 = popupmenu->addAction(tr("Open"));
    popupmenu->addSeparator();
    QAction *action4 = popupmenu->addAction(tr("Clear all"));
    action0->setShortcut(QKeySequence::Copy);
    action0->setIcon(QPixmap(":/menu_ico/res/ico2-inconsistency.png"));
    action1->setShortcut(QKeySequence(Qt::ALT + Qt::Key_A));
    action1->setIcon(QPixmap(":/menu_ico/res/ico2-copy.png"));
    action2->setShortcut(QKeySequence::Save);
    action2->setIcon(QPixmap(":/menu_ico/res/ico2-save.png"));
    action3->setShortcut(QKeySequence::Open);
    action3->setIcon(QPixmap(":/menu_ico/res/ico2-open-folder.png"));
    action4->setShortcut(QKeySequence::Delete);
    action4->setIcon(QPixmap(":/menu_ico/res/ico2-delete.png"));
    mui->textBrowser->addAction(action0);
    mui->textBrowser->addAction(action1);
    mui->textBrowser->addAction(action2);
    mui->textBrowser->addAction(action3);
    mui->textBrowser->addAction(action4);
    connect(action0, SIGNAL(triggered()), this, SLOT(on_toolButton_CopySel_clicked()));
    connect(action1, SIGNAL(triggered()), this, SLOT(on_toolButton_CopyAll_clicked()));
    connect(action2, SIGNAL(triggered()), this, SLOT(on_toolButton_Save_clicked()));
    connect(action3, SIGNAL(triggered()), this, SLOT(on_toolButton_Open_clicked()));
    connect(action4, SIGNAL(triggered()), this, SLOT(on_toolButton_Clear_clicked()));
    if (myOpt->isSaveOnExit){
        QString savePath = defineSavePath();
        QString fileName = savePath + AUTOSAVE_FILENAME;
        QDir dir(savePath);
        if (dir.exists()){
            if (!fileName.isEmpty()){
                QTextDocument *document = mui->textBrowser->document();
                QFile file(fileName);
                file.open(QIODevice::ReadOnly);
                document->setHtml(file.readAll().data());
                file.close();
                QTextCursor c = mui->textBrowser->textCursor();
                if(myOpt->isLastShowingFirst)
                    c.movePosition(QTextCursor::Start);
                else
                    c.movePosition(QTextCursor::End);
                mui->textBrowser->setTextCursor(c);
            }
        }
    }
    delete settings;

    this->on_textBrowser_textChanged();
    on_tabWidget_currentChanged(tab);
    switch (myOpt->styleGUI) {
    case _DefaultStyle:
        on_actionThemeDefault_triggered();
        break;
    case _DarkStyle:
        on_actionThemeDark_triggered();
        break;
    case _SunnyStyle:
        on_actionThemeSunny_triggered();
        break;
    default:
        break;
    }
    if(myOpt->styleGUI == _DarkStyle)
        invertGUIconColor();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::setLanguage(){
    QString slang = sender()->objectName();
    lang = slang;
    if(myOpt->isEnglishLocale)loc = QLocale::English;
    else loc = getLanguageLocale(lang);
    this->setLocale(loc);
    dv->setLocale(loc);
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    translator->load(":/lang/res/translations/Coil64_" + lang);
    qApp->installTranslator(translator);
    mui->retranslateUi(this);
    QList<QMenu*> menus = this->menuBar()->findChildren<QMenu*>();
    QList<QAction*> actions = menus.takeAt(1)->actions();
    QAction* child;
    if (actions.count() > 1){
        child = actions.last();
        child->setText(tr("Language"));
    }
    QList<QAction*> popupactions = popupmenu->actions();
    QAction *actCopySelected = popupactions.at(0);
    QAction *actCopyAll = popupactions.at(1);
    QAction *actSave = popupactions.at(3);
    QAction *actOpen = popupactions.at(4);
    QAction *actClearAll = popupactions.at(6);
    actCopySelected->setText(tr("Copy selected to clipboard"));
    actCopyAll->setText(tr("Copy all to clipboard"));
    actSave->setText(tr("Save"));
    actOpen->setText(tr("Open"));
    actClearAll->setText(tr("Clear all"));
    int tab = mui->tabWidget->currentIndex();
    on_tabWidget_currentChanged(tab);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
{
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation).toUtf8();
    QString hlptempFileName = tempDir.append("/onelayer.pdf");
    QFile hlptempFile(hlptempFileName);
    if (hlptempFile.exists())
        hlptempFile.remove();
    delete satCurrentDockWidget;
    delete popupmenu;
    delete data;
    delete dv;
    delete awgV;
    delete iv;
    delete myOpt;
    delete net_manager;
    delete mui;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_textBrowser_customContextMenuRequested(const QPoint &pos){
    popupmenu->popup(mui->textBrowser->viewport()->mapToGlobal(pos));
    QList<QAction*> popupactions = popupmenu->actions();
    QAction *actCopySelected = popupactions.at(0);
    QAction *actCopyAll = popupactions.at(1);
    QAction *actSave = popupactions.at(3);
    QAction *actOpen = popupactions.at(4);
    QAction *actClearAll = popupactions.at(6);
    if (mui->textBrowser->document()->isEmpty()){
        actCopyAll->setEnabled(false);
        actSave->setEnabled(false);
        actOpen->setEnabled(true);
        actClearAll->setEnabled(false);
    } else {
        actCopyAll->setEnabled(true);
        actSave->setEnabled(true);
        actOpen->setEnabled(true);
        actClearAll->setEnabled(true);
    }
    QTextCursor c = mui->textBrowser->textCursor();
    if (c.selectedText().isEmpty())
        actCopySelected->setEnabled(false);
    else
        actCopySelected->setEnabled(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_textBrowser_textChanged(){
    if (mui->textBrowser->document()->isEmpty()){
        mui->toolButton_CopySel->setEnabled(false);
        mui->toolButton_CopyAll->setEnabled(false);
        mui->toolButton_Open->setEnabled(true);
        mui->toolButton_Print->setEnabled(false);
        mui->toolButton_Save->setEnabled(false);
        mui->toolButton_Clear->setEnabled(false);
        mui->actionCopy->setEnabled(false);
        mui->actionSave->setEnabled(false);
        mui->actionPrint->setEnabled(false);
        mui->actionClear_all->setEnabled(false);
    } else {
        mui->toolButton_CopySel->setEnabled(true);
        mui->toolButton_CopyAll->setEnabled(true);
        mui->toolButton_Open->setEnabled(true);
        mui->toolButton_Print->setEnabled(true);
        mui->toolButton_Save->setEnabled(true);
        mui->toolButton_Clear->setEnabled(true);
        mui->actionCopy->setEnabled(true);
        mui->actionSave->setEnabled(true);
        mui->actionPrint->setEnabled(true);
        mui->actionClear_all->setEnabled(true);
    }
    QTextCursor c = mui->textBrowser->textCursor();
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::closeEvent(QCloseEvent *event){
    event->ignore();
    bool isConfirmed;
    if (myOpt->isConfirmExit){
        QMessageBox messageBox(QMessageBox::Question,
                               tr("Confirmation"),
                               tr("Are you sure?"),
                               QMessageBox::Yes | QMessageBox::No,
                               this);
        messageBox.setButtonText(QMessageBox::Yes, tr("Yes"));
        messageBox.setButtonText(QMessageBox::No, tr("No"));
        if (messageBox.exec()== QMessageBox::Yes) isConfirmed = true;
        else isConfirmed = false;
    } else isConfirmed = true;
    if (isConfirmed){
        event->accept();
        QSettings *settings;
        defineAppSettings(settings);
        settings->beginGroup( "GUI" );
        settings->setValue( "tab", mui->tabWidget->currentIndex() );
        settings->setValue( "form_coil", FormCoil);
        int init_cond = 0;
        if (mui->radioButton_CF->isChecked())
            init_cond = 1;
        if (mui->radioButton_LF->isChecked())
            init_cond = 2;
        if (mui->radioButton_ZF->isChecked())
            init_cond = 3;
        settings->setValue("lang",this->lang);
        settings->setValue( "init_cond", init_cond);
        if (mui->radioButton_1->isChecked()) settings->setValue("wire_material", 0);
        if (mui->radioButton_2->isChecked()) settings->setValue("wire_material", 1);
        if (mui->radioButton_3->isChecked()) settings->setValue("wire_material", 2);
        if (mui->radioButton_4->isChecked()) settings->setValue("wire_material", 3);
        if (mui->radioButton_6->isChecked()) settings->setValue("init_data", 0);
        if (mui->radioButton_7->isChecked()) settings->setValue("init_data", 1);
        if (mui->radioButton_8->isChecked()) settings->setValue("init_data", 2);
        settings->setValue("isInsertImage", myOpt->isInsertImage);
        settings->setValue("isShowTitle", myOpt->isShowTitle);
        settings->setValue("isShowLTSpice", myOpt->isShowLTSpice);
        settings->setValue("isAdditionalResult", myOpt->isAdditionalResult);
        settings->setValue("isAWGwithInsulation", myOpt->isAWGhasInsulation);
        settings->setValue("isConfirmExit", myOpt->isConfirmExit);
        settings->setValue("isConfirmClear", myOpt->isConfirmClear);
        settings->setValue("isConfirmDelete", myOpt->isConfirmDelete);
        settings->setValue("isOutputValueColored", myOpt->isOutputValueColored);
        settings->setValue("isOutputInTwoColumns", myOpt->isOutputInTwoColumns);
        settings->setValue("isShowValueDescription", myOpt->isShowValueDescription);
        settings->setValue("isShowCalcNum", myOpt->isShowCalcNum);
        settings->setValue("MainFontFamily", myOpt->mainFontFamily);
        settings->setValue("MainFontSize", myOpt->mainFontSize);
        settings->setValue("valueTextColor",  myOpt->vTextColor);
        settings->setValue("TextFontFamily", myOpt->textFontFamily);
        settings->setValue("TextFontSize", myOpt->textFontSize);
        settings->setValue("isEnglishLocale", myOpt->isEnglishLocale);
        settings->setValue("windingKind", myOpt->windingKind);
        settings->setValue("layoutPCBcoil", myOpt->layoutPCBcoil);
        settings->setValue("isWindingLengthOneLayerInit", myOpt->isWindingLengthOneLayerInit);
        settings->setValue("isSaveOnExit", myOpt->isSaveOnExit);
        settings->setValue("isLastShowingFirst", myOpt->isLastShowingFirst);
        settings->setValue("styleGUI", myOpt->styleGUI);
        if (myOpt->isSaveOnExit){
            QTextDocument *document = mui->textBrowser->document();
            QString savePath = defineSavePath();
            QDir sD(savePath);
            if (!sD.exists())
                sD.mkpath(savePath);
            QFile file(savePath + AUTOSAVE_FILENAME);
            file.open(QIODevice::WriteOnly);
            file.write(document->toHtml().toUtf8());
            file.close();
            settings->setValue("calc_count", calc_count);
        }
        if (satCurrentDockWidget != nullptr){
            myOpt->isDockWidgetFloating = satCurrentDockWidget->isFloating();
            myOpt->isDockWidgetVisible = satCurrentDockWidget->isVisible();
            settings->setValue("isDockWidgetFloating", myOpt->isDockWidgetFloating);
            settings->setValue("isDockWidgetVisible", myOpt->isDockWidgetVisible);
        }
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
        settings->setValue("B", data->B);
        settings->setValue("D", data->D);
        settings->setValue("a", data->a);
        settings->setValue("b", data->b);
        settings->setValue("dw", data->d);
        settings->setValue("k", data->k);
        settings->setValue("p", data->p);
        settings->setValue("t", data->t);
        settings->setValue("w", data->w);
        settings->setValue("ns", data->ns);
        settings->setValue("i", data->isol);
        settings->setValue("l", data->l);
        settings->setValue("c", data->c);
        settings->setValue("Ch", data->Ch);
        settings->setValue("g", data->g);
        settings->setValue("Ng", data->Ng);
        settings->setValue("Do", data->Do);
        settings->setValue("Di", data->Di);
        settings->setValue("th", data->th);
        settings->setValue("h", data->h);
        settings->setValue("mu", data->mu);
        settings->setValue("ratio", data->ratio);
        settings->setValue("zo", data->zo);
        settings->setValue("s", data->s);
        settings->setValue("Rdc", data->Rdc);
        settings->endGroup();
        settings->setValue("mainWindowGeometry", saveGeometry());
        settings->setValue("mainWindowState", saveState());
        QApplication::closeAllWindows();
        delete settings;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::resizeEvent(QResizeEvent *)
{
    int w = (int) 235 * sqrt(mui->toolButton_Configure->iconSize().width());
    if (this->width() < w){
        mui->horizontalLayout_5->insertWidget(0, mui->toolButton_lShowFirst);
        mui->horizontalLayout_5->insertWidget(1, mui->toolButton_column);
        mui->horizontalLayout_5->insertWidget(2, mui->toolButton_Num);
        mui->horizontalLayout_5->insertWidget(3, mui->line_11);
        mui->horizontalLayout_5->insertWidget(4, mui->toolButton_showTitle);
        mui->horizontalLayout_5->insertWidget(5, mui->toolButton_showImg);
        mui->horizontalLayout_5->insertWidget(6, mui->toolButton_Desc);
        mui->horizontalLayout_5->addWidget(mui->toolButton_Color);
        mui->horizontalLayout_5->insertWidget(7, mui->line_13);
    } else {
        mui->horizontalLayout_1->insertWidget(0, mui->toolButton_lShowFirst);
        mui->horizontalLayout_1->insertWidget(1, mui->toolButton_column);
        mui->horizontalLayout_1->insertWidget(2, mui->toolButton_Num);
        mui->horizontalLayout_1->insertWidget(3, mui->line_11);
        mui->horizontalLayout_1->insertWidget(4, mui->toolButton_showTitle);
        mui->horizontalLayout_1->insertWidget(5, mui->toolButton_showImg);
        mui->horizontalLayout_1->insertWidget(6, mui->toolButton_Desc);
        mui->horizontalLayout_1->insertWidget(7, mui->toolButton_Color);
        mui->horizontalLayout_1->insertWidget(8, mui->line_13);
    }
    if (satCurrentDockWidget != nullptr){
        if((satCurrentDockWidget->isFloating()) && (satCurrentDockWidget->isVisible())){
            satCurrentDockWidget->setVisible(false);
            satCurrentDockWidget->setVisible(true);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::moveEvent(QMoveEvent *)
{
    if (satCurrentDockWidget != nullptr){
        if (satCurrentDockWidget->isFloating()){
            QPoint p = satCurrentDockWidget->mapToGlobal(QPoint(0,0));
            p.setX(this->geometry().x() + this->size().width());
            p.setY(this->geometry().y());
            satCurrentDockWidget->move(p);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::changeEvent(QEvent *e)
{
    if( e->type() == QEvent::WindowStateChange ){
        QWindowStateChangeEvent* event = static_cast< QWindowStateChangeEvent* >( e );

        if( event->oldState() == Qt::WindowMaximized )
        {
            if (myOpt->isDockWidgetFloating)
                satCurrentDockWidget->setFloating(true);
        }
        else if( event->oldState() == Qt::WindowNoState && this->windowState() == Qt::WindowMaximized )
        {
            satCurrentDockWidget->setFloating(false);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool MainWindow::eventFilter(QObject *watched, QEvent *event){
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier) && keyEvent->key() == 'S') {
            on_toolButton_Save_clicked();
            event->ignore();
            return true;
        }
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier) && keyEvent->key() == 'O') {
            on_toolButton_Open_clicked();
            event->ignore();
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::resetUiFont(){
    QFont f1 = this->font();
    f1.setFamily(myOpt->mainFontFamily);
    f1.setPixelSize(myOpt->mainFontSize);
    qApp->setFont(f1);
    foreach (QWidget *widget, QApplication::allWidgets()) {
        widget->setFont(QApplication::font());
        widget->update();
    }
    sendOptToDock(*myOpt);
    mui->listWidget->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_Clear->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_CopyAll->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_CopySel->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_Help->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_Configure->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_Open->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_Print->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_Save->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_showAdditional->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_showImg->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_showTitle->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_cbc->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_cbe->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_cdsr->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_soe->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_lShowFirst->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_Color->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_Desc->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_column->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_ltspice->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_FAQ->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_Num->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_Saturation->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    QFont f2 = mui->textBrowser->font();
    f2.setFamily(myOpt->textFontFamily);
    f2.setPixelSize(myOpt->textFontSize);
    mui->textBrowser->setFont(f2);
    this->resizeEvent(NULL);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::invertGUIconColor()
{
    for (int i = 0; i < mui->listWidget->count(); i++){
        QIcon myicon = reverseIconColors(mui->listWidget->item(i)->icon());
        mui->listWidget->item(i)->setIcon(myicon);
    }
    QList<QAction*> pactions = popupmenu->actions();
    for (int i = 0; i < pactions.count(); i++){
        QIcon myicon = reverseIconColors(pactions.at(i)->icon());
        pactions.at(i)->setIcon(myicon);
    }
    QList<QMenu*> menus = this->menuBar()->findChildren<QMenu*>();
    for (int j = 0; j < menus.count(); j++){
        if (menus.at(j)->objectName() != "Language"){
            QList<QAction*> actions = menus.at(j)->actions();
            for (int i = 0; i < actions.count(); i++){
                QIcon myicon = reverseIconColors(actions.at(i)->icon());
                actions.at(i)->setIcon(myicon);
            }
        }
    }
    QList<QToolButton*> toolButtonsTopList = mui->centralWidget->findChildren<QToolButton*>();
    foreach( QToolButton* toolButtonsTop, toolButtonsTopList ){
        toolButtonsTop->setIcon(reverseIconColors(toolButtonsTop->icon()));
    }
    mui->pushButton_Calculate->setIcon(reverseIconColors(mui->pushButton_Calculate->icon()));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::showOutput(QString caption, QString image, QString input, QString result)
{
    QTextCursor c = mui->textBrowser->textCursor();
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    calc_count++;
    QString outputHTML = "";
    QString sTitle = "";
    if (myOpt->isShowCalcNum){
        QLocale dloc = getLanguageLocale(lang);
        c.insertHtml("<hr/><a href=\"del\" title=\"" +tr("Delete this result") + "\">" + QString(QChar(CLEAR_CHAR)) + "</a><b> – "
                     + QString::number(calc_count)+ "</b> - " + dloc.toString(QDateTime::currentDateTime(), QLocale::ShortFormat) + "<br/><br/>");
    } else if (!myOpt->isOutputInTwoColumns)
        c.insertHtml("<hr/><br/>");
    if (myOpt->isShowTitle)
        sTitle += "<h4>" + caption + "</h4>";
    if (myOpt->isInsertImage)
        sTitle += "<p>"+ image + "</p>";
    if (myOpt->isOutputInTwoColumns){
        outputHTML += "<p><table  border='1' style='border-style:solid;margin:2;'><tbody><tr><td style='padding:5'>" + sTitle + input +
                "</td><td style='padding:5'>" + result + "</td></tr></tbody></table><br/></p>";
    } else {
        outputHTML += "<p>" + sTitle + input + result + "<br/></p>";
    }
    c.insertHtml(outputHTML);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::checkMaterial1(Material *mt){
    if (mui->radioButton_2->isChecked()){
        *mt = Ag;
    }
    else if (mui->radioButton_3->isChecked()){
        *mt = Al;
    }
    else if (mui->radioButton_4->isChecked()){
        *mt = Ti;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::checkMaterial2(Material *mt){
    if (mui->radioButton_2_2->isChecked()){
        *mt = Ag;
    }
    else if (mui->radioButton_3_2->isChecked()){
        *mt = Al;
    }
    else if (mui->radioButton_4_2->isChecked()){
        *mt = Ti;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_dockWidgetClosed()
{
    myOpt->isDockWidgetVisible = false;
    mui->toolButton_Saturation->setChecked(false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::checkAppVersion(QNetworkReply *reply){
    if (reply->error()){
        showWarning(tr("Warning"), tr("Error. Can not open URL: ") + qApp->organizationDomain());
    } else {
        bool isRequirestUpdate = false;
        QString internetVersiontext = reply->readAll();
        QStringList internetVersion = internetVersiontext.split(QRegExp("\\."), skip_empty_parts);
        QString str = "%1";
        QString localVersiontext = str.arg(qApp->applicationVersion());
        QStringList localVersion = localVersiontext.split(QRegExp("\\."), skip_empty_parts);
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
        } else {
            showWarning(tr("Warning"), tr("Something gone wrong") + "!");
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
    QSettings *settings;
    defineAppSettings(settings);
    QDate date;
    myOpt->firstDate = QDate::fromString(settings->value("firstDate", date.currentDate().toString("dd.MM.yyyy")).toString(),"dd.MM.yyyy");
    delete settings;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionHelp_triggered()
{
    int tab = mui->tabWidget->currentIndex();
    if ((tab == 0) || (tab == 1)){
        switch (FormCoil) {
        case _Onelayer_cw:
        case _Onelayer:
        case _Onelayer_p:
        case _Onelayer_q:
            QDesktopServices::openUrl(QUrl("https://coil32.net/one-layer-air-core-coil.html"));
            break;
        case _Multilayer:
            QDesktopServices::openUrl(QUrl("https://coil32.net/multi-layer-coil.html"));
            break;
        case _Multilayer_p:
            QDesktopServices::openUrl(QUrl("https://coil32.net/multi-layer-coil.html"));
            break;
        case _Multilayer_r:
            QDesktopServices::openUrl(QUrl("https://coil32.net/multilayer-rectangular.html"));
            break;
        case _FerrToroid:
            QDesktopServices::openUrl(QUrl("https://coil32.net/ferrite-toroid-core.html"));
            break;
        case _PCB_coil:
            QDesktopServices::openUrl(QUrl("https://coil32.net/pcb-coil.html"));
            break;
        case _Flat_Spiral:
            QDesktopServices::openUrl(QUrl("https://coil32.net/foil-wound-coil-calculation.html"));
            break;
        case _Multilayer_f:
            QDesktopServices::openUrl(QUrl("https://coil32.net/foil-wound-coil-calculation.html"));
            break;
        default:
            break;
        }
    } else if (tab == 2){
        QDesktopServices::openUrl(QUrl("https://coil32.net/lc-resonance-calculation.html"));
    }
}

void MainWindow::on_actionIntrfaceHelp_triggered()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/detail/about-coil64.html"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionDonate_triggered()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/donate.html"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionHomePage_triggered()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionVersions_history_triggered()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/version-history.html"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
    int tab = mui->tabWidget->currentIndex();
    FormCoil = (_FormCoil)currentRow;
    switch (tab) {
    case 0:{
        mui->label_ind->setText(tr("Inductance") + " L:");
        mui->label_freq->setText(tr("Frequency") + " f:");
        mui->label_ind_m->setText(qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()));
        mui->label_01->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_02->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_03->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_04->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_05->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_06->setText("");
        mui->lineEdit_ind->setFocus();
        mui->lineEdit_1->setValidator(dv);
        mui->lineEdit_2->setValidator(dv);
        mui->lineEdit_3->setValidator(dv);
        mui->lineEdit_4->setValidator(dv);
        mui->lineEdit_5->setValidator(dv);
        mui->lineEdit_6->setValidator(dv);
        mui->lineEdit_freq->setValidator(dv);
        mui->lineEdit_ind->setValidator(dv);
        mui->toolButton_Saturation->setEnabled(false);
        mui->toolButton_ltspice->setEnabled(false);
        satCurrentDockWidget->hide();
        switch (FormCoil) {
        case _Onelayer_cw:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil1.png"));
            mui->groupBox->setVisible(true);
            mui->groupBox_6->setVisible(false);
            mui->groupBox_onelayer_init->setVisible(true);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(true);
            mui->toolButton_showAdditional->setEnabled(true);
            mui->toolButton_ltspice->setEnabled(true);
            mui->label_freq_m->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
            mui->label_freq->setVisible(true);
            mui->label_freq_m->setVisible(true);
            mui->lineEdit_freq->setVisible(true);
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
            mui->checkBox_isInsulation->setVisible(false);
            mui->lineEdit_3->setEnabled(true);
            mui->label_1->setText(tr("Former diameter") + " D:");
            mui->label_3->setText(tr("Wire diameter with insulation") + " k:");
            mui->lineEdit_ind->setText(roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_freq->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_1->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            on_radioButton_5_1_toggled(!myOpt->isWindingLengthOneLayerInit);
            if (myOpt->isWindingLengthOneLayerInit)
                mui->radioButton_5_2->setChecked(true);
            else
                mui->radioButton_5_1->setChecked(true);
            break;
        }
        case _Onelayer:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil2.png"));
            mui->groupBox->setVisible(true);
            mui->groupBox_6->setVisible(false);
            mui->groupBox_onelayer_init->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(true);
            mui->toolButton_showAdditional->setEnabled(true);
            mui->toolButton_ltspice->setEnabled(true);
            mui->label_freq_m->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
            if (myOpt->isAWG){
                mui->label_02->setText(tr("AWG"));
                mui->lineEdit_2->setValidator(awgV);
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
            mui->label_1->setText(tr("Former diameter") + " D:");
            mui->label_2->setText(tr("Wire diameter") + " d:");
            mui->label_3->setText(tr("Wire diameter with insulation") + " k:");
            mui->label_4->setText(tr("Winding pitch")+" p:");
            mui->lineEdit_ind->setText(roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_freq->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_1->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2->setText("");
                mui->checkBox_isInsulation->setVisible(true);
                mui->checkBox_isInsulation->setChecked(myOpt->isAWGhasInsulation);
                on_checkBox_isInsulation_toggled(myOpt->isAWGhasInsulation);
            } else {
                mui->lineEdit_2->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->checkBox_isInsulation->setVisible(false);
                mui->lineEdit_3->setEnabled(true);
            }
            if ((mui->lineEdit_2->text().isEmpty() || (mui->lineEdit_2->text() == "0")|| (data->k > 0)) && (mui->checkBox_isInsulation->isChecked()))
                mui->lineEdit_3->setText(roundTo(data->k / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            else
                on_lineEdit_2_editingFinished();
            mui->lineEdit_4->setText(roundTo(data->p / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            break;
        }
        case _Onelayer_p:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil2_square.png"));
            mui->groupBox->setVisible(true);
            mui->groupBox_6->setVisible(false);
            mui->groupBox_onelayer_init->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(true);
            mui->toolButton_showAdditional->setEnabled(true);
            mui->toolButton_ltspice->setEnabled(true);
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
            mui->checkBox_isInsulation->setVisible(false);
            mui->lineEdit_3->setEnabled(true);
            mui->label_1->setText(tr("Former diameter") +" D:");
            mui->label_2->setText(tr("Wire width") + " w:");
            mui->label_3->setText(tr("Wire thickness") + " t:");
            mui->label_4->setText(tr("Insulation thickness")+" i:");
            mui->label_5->setText(tr("Winding pitch")+" p:");
            mui->lineEdit_ind->setText(roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_freq->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_1->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2->setText(roundTo(data->w / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3->setText(roundTo(data->t / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_4->setText(roundTo(data->isol / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_5->setText(roundTo(data->p / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            break;
        }
        case _Onelayer_q:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil3.png"));
            mui->groupBox->setVisible(true);
            mui->groupBox_6->setVisible(false);
            mui->groupBox_onelayer_init->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(true);
            mui->toolButton_showAdditional->setEnabled(true);
            mui->toolButton_ltspice->setEnabled(true);
            mui->label_freq_m->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
            if (myOpt->isAWG){
                mui->label_02->setText(tr("AWG"));
                mui->lineEdit_2->setValidator(awgV);
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
            mui->lineEdit_5->setVisible(true);
            mui->label_5->setVisible(true);
            mui->label_05->setVisible(false);
            mui->lineEdit_6->setVisible(false);
            mui->label_6->setVisible(false);
            mui->label_06->setVisible(false);
            mui->checkBox_isInsulation->setVisible(false);
            mui->lineEdit_3->setEnabled(true);
            mui->label_1->setText(tr("Former diameter") + " D:");
            mui->label_2->setText(tr("Wire diameter") + " d:");
            mui->label_3->setText(tr("Wire diameter with insulation") + " k:");
            mui->label_4->setText(tr("Winding pitch")+" p:");
            mui->label_5->setText(tr("Number of sides of the former")+" n:");
            mui->lineEdit_ind->setText(roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_freq->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_1->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2->setText("");
            } else
                mui->lineEdit_2->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (mui->lineEdit_2->text().isEmpty() || (mui->lineEdit_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_3->setText(roundTo(data->k / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            else
                on_lineEdit_2_editingFinished();
            mui->lineEdit_4->setText(roundTo(data->p / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_5->setText(loc.toString(data->ns));
            break;
        }
        case _Multilayer:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            mui->groupBox_onelayer_init->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(true);
            mui->toolButton_FAQ->setVisible(false);
            mui->toolButton_showAdditional->setEnabled(false);
            if (myOpt->isAWG){
                mui->label_03->setText(tr("AWG"));
                mui->lineEdit_3->setValidator(awgV);
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
            mui->checkBox_isInsulation->setVisible(false);
            mui->lineEdit_3->setEnabled(true);
            mui->label_1->setText(tr("Former diameter") + " D:");
            mui->label_2->setText(tr("Winding length") + " l:");
            mui->label_3->setText(tr("Wire diameter") + " d:");
            mui->label_4->setText(tr("Wire diameter with insulation") + " k:");
            mui->lineEdit_ind->setText(roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2->setText(roundTo(data->l / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_3->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_3->setText("");
            } else
                mui->lineEdit_3->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (mui->lineEdit_3->text().isEmpty() || (mui->lineEdit_3->text() == "0")|| (data->k > 0))
                mui->lineEdit_4->setText(roundTo(data->k / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            else
                on_lineEdit_3_editingFinished();
            mui->comboBox_checkMLWinding->setCurrentIndex(myOpt->windingKind);
            on_comboBox_checkMLWinding_activated(myOpt->windingKind);
            break;
        }
        case _Multilayer_p:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4-0.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            mui->groupBox_onelayer_init->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(false);
            mui->toolButton_showAdditional->setEnabled(false);
            if (myOpt->isAWG){
                mui->label_03->setText(tr("AWG"));
                mui->lineEdit_3->setValidator(awgV);
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
            mui->checkBox_isInsulation->setVisible(false);
            mui->lineEdit_3->setEnabled(true);
            mui->label_1->setText(tr("Former diameter") + " D:");
            mui->label_2->setText(tr("Winding length") + " l:");
            mui->label_3->setText(tr("Wire diameter") + " d:");
            mui->label_4->setText(tr("Wire diameter with insulation") + " k:");
            mui->label_5->setText(tr("Insulation thickness")+" g:");
            mui->label_6->setText(tr("Layers number beetween insulating pads") + " Ng:");
            mui->lineEdit_ind->setText(roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2->setText(roundTo(data->l / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_3->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_3->setText("");
            } else
                mui->lineEdit_3->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (mui->lineEdit_3->text().isEmpty() || (mui->lineEdit_3->text() == "0")||(data->k > 0))
                mui->lineEdit_4->setText(roundTo(data->k / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            else
                on_lineEdit_3_editingFinished();
            mui->lineEdit_5->setText(roundTo(data->g / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_6->setText(loc.toString(data->Ng));
            break;
        }
        case _Multilayer_r:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4_square.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            mui->groupBox_onelayer_init->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(false);
            mui->toolButton_showAdditional->setEnabled(false);
            if (myOpt->isAWG){
                mui->label_04->setText(tr("AWG"));
                mui->lineEdit_4->setValidator(awgV);
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
            mui->lineEdit_6->setVisible(false);
            mui->label_6->setVisible(false);
            mui->label_06->setVisible(false);
            mui->checkBox_isInsulation->setVisible(false);
            mui->lineEdit_3->setEnabled(true);
            mui->label_1->setText(tr("Former width") + " a:");
            mui->label_2->setText(tr("Former height") + " b:");
            mui->label_3->setText(tr("Winding length") + " l:");
            mui->label_4->setText(tr("Wire diameter") + " d:");
            mui->label_5->setText(tr("Wire diameter with insulation")+" k:");
            mui->lineEdit_ind->setText(roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(roundTo(data->a / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2->setText(roundTo(data->b / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3->setText(roundTo(data->l / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_4->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_4->setText("");
            } else
                mui->lineEdit_4->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (mui->lineEdit_4->text().isEmpty() || (mui->lineEdit_4->text() == "0")|| (data->k > 0))
                mui->lineEdit_5->setText(roundTo(data->k / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            else
                on_lineEdit_4_editingFinished();
            mui->lineEdit_5->setText(roundTo(data->k / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            break;
        }
        case _Multilayer_f:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil11.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            mui->groupBox_onelayer_init->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(false);
            mui->toolButton_showAdditional->setEnabled(false);
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
            mui->checkBox_isInsulation->setVisible(false);
            mui->lineEdit_3->setEnabled(true);
            mui->label_1->setText(tr("Former diameter") + " D:");
            mui->label_2->setText(tr("Foil width") + " w:");
            mui->label_3->setText(tr("Foil thickness") + " t:");
            mui->label_4->setText(tr("Insulation thickness")+" g:");
            mui->lineEdit_ind->setText(roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2->setText(roundTo(data->w / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3->setText(roundTo(data->t / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_4->setText(roundTo(data->isol / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            break;
        }
        case _FerrToroid:{
            mui->image->setPixmap(QPixmap(":/images/res/T-core.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            mui->groupBox_onelayer_init->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(false);
            if(myOpt->isDockWidgetVisible)
                satCurrentDockWidget->show();
            mui->toolButton_Saturation->setEnabled(true);
            mui->toolButton_showAdditional->setEnabled(false);
            if (myOpt->isAWG){
                mui->label_04->setText(tr("AWG"));
                mui->lineEdit_4->setValidator(awgV);
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
            mui->lineEdit_6->setVisible(true);
            mui->label_6->setVisible(true);
            mui->label_06->setVisible(true);
            mui->checkBox_isInsulation->setVisible(false);
            mui->lineEdit_3->setEnabled(true);
            mui->label_1->setText(tr("Outside diameter")+" OD:");
            mui->label_2->setText(tr("Inside diameter")+" ID:");
            mui->label_3->setText(tr("Core height") + " h:");
            mui->label_4->setText(tr("Wire diameter") + " d:");
            mui->label_5->setText(tr("Magnetic permeability")+" μ:");
            mui->label_6->setText(tr("Chamfer") + " C:");
            mui->label_06->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
            mui->lineEdit_ind->setText(roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(roundTo(data->Do / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2->setText(roundTo(data->Di / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3->setText(roundTo(data->h / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_4->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_4->setText("");
            } else
                mui->lineEdit_4->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_5->setText(loc.toString(data->mu));
            mui->lineEdit_6->setText(roundTo(data->Ch / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            break;
        }
        case _PCB_coil:{
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(true);
            mui->groupBox_onelayer_init->setVisible(false);
            mui->comboBox_checkPCB->setVisible(true);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(false);
            mui->toolButton_showAdditional->setEnabled(false);
            mui->label_freq->setVisible(true);
            mui->label_freq_m->setVisible(true);
            mui->lineEdit_freq->setVisible(true);
            mui->lineEdit_3->setVisible(true);
            mui->label_3->setVisible(true);
            mui->label_03->setVisible(true);
            mui->lineEdit_5->setVisible(false);
            mui->label_5->setVisible(false);
            mui->label_05->setVisible(false);
            mui->lineEdit_6->setVisible(false);
            mui->label_6->setVisible(false);
            mui->label_06->setVisible(false);
            mui->checkBox_isInsulation->setVisible(false);
            mui->lineEdit_3->setEnabled(true);
            mui->lineEdit_ind->setText(roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_ind->selectAll();
            mui->label_freq_m->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
            mui->lineEdit_freq->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
            mui->horizontalSlider->setValue(data->ratio * 100);
            mui->horizontalSlider->setToolTip(loc.toString(data->ratio));
            mui->comboBox_checkPCB->setCurrentIndex(myOpt->layoutPCBcoil);
            on_comboBox_checkPCB_activated(myOpt->layoutPCBcoil);
            break;
        }
        case _Flat_Spiral:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil10.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            mui->groupBox_onelayer_init->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(false);
            mui->toolButton_showAdditional->setEnabled(false);
            if (myOpt->isAWG){
                mui->label_02->setText(tr("AWG"));
                mui->lineEdit_2->setValidator(awgV);
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
            mui->checkBox_isInsulation->setVisible(false);
            mui->lineEdit_3->setEnabled(true);
            mui->label_1->setText(tr("Inside diameter")+" Di:");
            mui->label_2->setText(tr("Wire diameter") + " d:");
            mui->label_3->setText(tr("Gap between turns") + " s:");
            mui->lineEdit_ind->setText(roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(roundTo(data->Di / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2->setText("");
            } else
                mui->lineEdit_2->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3->setText(roundTo(data->s / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            break;
        }
        default:
            break;
        }
        break;
    }
    case 1:{
        mui->label_N->setText(tr("Number of turns") + " N:");
        mui->label_freq2->setText(tr("Frequency") + " f:");
        mui->label_01_2->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_02_2->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_03_2->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_04_2->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_05_2->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->label_06_2->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
        mui->lineEdit_1_2->setValidator(dv);
        mui->lineEdit_2_2->setValidator(dv);
        mui->lineEdit_3_2->setValidator(dv);
        mui->lineEdit_4_2->setValidator(dv);
        mui->lineEdit_5_2->setValidator(dv);
        mui->lineEdit_6_2->setValidator(dv);
        mui->lineEdit_7_2->setValidator(dv);
        mui->lineEdit_freq2->setValidator(dv);
        mui->lineEdit_N->setValidator(dv);
        mui->lineEdit_1_2->setVisible(true);
        mui->label_1_2->setVisible(true);
        mui->label_01_2->setVisible(true);
        satCurrentDockWidget->hide();
        mui->toolButton_Saturation->setEnabled(false);
        mui->toolButton_ltspice->setEnabled(false);
        switch (FormCoil) {
        case _Onelayer_cw:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil1.png"));
            mui->groupBox_2->setVisible(true);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(true);
            mui->toolButton_showAdditional->setEnabled(true);
            mui->toolButton_ltspice->setEnabled(true);
            mui->label_freq_m2->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
            if (myOpt->isAWG){
                mui->label_02_2->setText(tr("AWG"));
                mui->lineEdit_2_2->setValidator(awgV);
            }
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->lineEdit_N->setFocus();
            mui->label_freq2->setVisible(true);
            mui->label_freq_m2->setVisible(true);
            mui->lineEdit_freq2->setVisible(true);
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
            mui->checkBox_isInsulation2->setVisible(false);
            mui->lineEdit_3_2->setEnabled(true);
            mui->label_1_2->setText(tr("Former diameter") + " D:");
            mui->label_2_2->setText(tr("Wire diameter") + " d:");
            mui->label_3_2->setText(tr("Wire diameter with insulation") + " k:");
            mui->lineEdit_N->setText(roundTo(data->N, loc, myOpt->dwAccuracy));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_freq2->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_1_2->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2_2->setText("");
            } else
                mui->lineEdit_2_2->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (mui->lineEdit_2_2->text().isEmpty() || (mui->lineEdit_2_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_3_2->setText(roundTo(data->k / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            else
                on_lineEdit_2_2_editingFinished();
            break;
        }
        case _Onelayer:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil2.png"));
            mui->groupBox_2->setVisible(true);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(true);
            mui->toolButton_showAdditional->setEnabled(true);
            mui->toolButton_ltspice->setEnabled(true);
            mui->label_freq_m2->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
            if (myOpt->isAWG){
                mui->label_02_2->setText(tr("AWG"));
                mui->lineEdit_2_2->setValidator(awgV);
            }
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->lineEdit_N->setFocus();
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
            mui->label_3_2->setEnabled(true);
            mui->label_1_2->setText(tr("Former diameter") + " D:");
            mui->label_2_2->setText(tr("Wire diameter") + " d:");
            mui->label_3_2->setText(tr("Wire diameter with insulation") + " k:");
            mui->label_4_2->setText(tr("Winding pitch")+" p:");
            mui->lineEdit_N->setText(roundTo(data->N, loc, myOpt->dwAccuracy));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_freq2->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_1_2->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2_2->setText("");
                mui->checkBox_isInsulation2->setVisible(true);
                mui->checkBox_isInsulation2->setChecked(myOpt->isAWGhasInsulation);
                on_checkBox_isInsulation2_toggled(myOpt->isAWGhasInsulation);
            } else {
                mui->lineEdit_2_2->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->checkBox_isInsulation2->setVisible(false);
                mui->lineEdit_3_2->setEnabled(true);
            }
            if ((mui->lineEdit_2_2->text().isEmpty() || (mui->lineEdit_2_2->text() == "0")|| (data->k > 0)) && (mui->checkBox_isInsulation2->isChecked()))
                mui->lineEdit_3_2->setText(roundTo(data->k / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            else
                on_lineEdit_2_2_editingFinished();
            mui->lineEdit_4_2->setText(roundTo(data->p / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            break;
        }
        case _Onelayer_p:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil2_square.png"));
            mui->groupBox_2->setVisible(true);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(true);
            mui->toolButton_showAdditional->setEnabled(true);
            mui->toolButton_ltspice->setEnabled(true);
            mui->label_freq_m2->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->lineEdit_N->setFocus();
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
            mui->checkBox_isInsulation2->setVisible(false);
            mui->lineEdit_3_2->setEnabled(true);
            mui->label_1_2->setText(tr("Former diameter") + " D:");
            mui->label_2_2->setText(tr("Wire width") + " w:");
            mui->label_3_2->setText(tr("Wire thickness") + " t:");
            mui->label_4_2->setText(tr("Insulation thickness")+" i:");
            mui->label_5_2->setText(tr("Winding pitch")+" p:");
            mui->lineEdit_N->setText(roundTo(data->N, loc, myOpt->dwAccuracy));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_freq2->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_1_2->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2_2->setText(roundTo(data->w / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3_2->setText(roundTo(data->t / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_4_2->setText(roundTo(data->isol / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_5_2->setText(roundTo(data->p / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            break;
        }
        case _Onelayer_q:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil3.png"));
            mui->groupBox_2->setVisible(true);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(true);
            mui->toolButton_showAdditional->setEnabled(true);
            mui->toolButton_ltspice->setEnabled(true);
            mui->label_freq_m2->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
            if (myOpt->isAWG){
                mui->label_02_2->setText(tr("AWG"));
                mui->lineEdit_2_2->setValidator(awgV);
            }
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->lineEdit_N->setFocus();
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
            mui->label_05_2->setVisible(false);
            mui->lineEdit_6_2->setVisible(false);
            mui->label_6_2->setVisible(false);
            mui->label_06_2->setVisible(false);
            mui->lineEdit_7_2->setVisible(false);
            mui->label_7_2->setVisible(false);
            mui->label_07_2->setVisible(false);
            mui->checkBox_isInsulation2->setVisible(false);
            mui->lineEdit_3_2->setEnabled(true);
            mui->label_1_2->setText(tr("Former diameter") + " D:");
            mui->label_2_2->setText(tr("Wire diameter") + " d:");
            mui->label_3_2->setText(tr("Wire diameter with insulation") + " k:");
            mui->label_4_2->setText(tr("Winding pitch")+" p:");
            mui->label_5_2->setText(tr("Number of sides of the former")+" n:");
            mui->lineEdit_N->setText(roundTo(data->N, loc, myOpt->dwAccuracy));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_freq2->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_1_2->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2_2->setText("");
            } else
                mui->lineEdit_2_2->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (mui->lineEdit_2_2->text().isEmpty() || (mui->lineEdit_2_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_3_2->setText(roundTo(data->k / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            else
                on_lineEdit_2_2_editingFinished();
            mui->lineEdit_4_2->setText(roundTo(data->p / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_5_2->setText(loc.toString(data->ns));
            break;
        }
        case _Multilayer:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(true);
            mui->radioButton_8->setVisible(true);
            mui->radioButton_6->setText(tr("Number of turns of the coil") + " (N)");
            mui->radioButton_7->setText(tr("Thickness of the coil") + " (c)");
            mui->radioButton_8->setText(tr("Resistance of the coil") + " (Rdc)");
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(true);
            mui->toolButton_FAQ->setVisible(false);
            mui->toolButton_showAdditional->setEnabled(false);
            if (myOpt->isAWG){
                mui->label_04_2->setText(tr("AWG"));
                mui->lineEdit_4_2->setValidator(awgV);
            }
            mui->label_N->setVisible(false);
            mui->lineEdit_N->setVisible(false);
            mui->lineEdit_1_2->setFocus();
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
            mui->checkBox_isInsulation2->setVisible(false);
            mui->lineEdit_3_2->setEnabled(true);
            mui->label_1_2->setText(tr("Former diameter") + " D:");
            mui->label_2_2->setText(tr("Winding length") + " l:");
            mui->label_3_2->setText(tr("Thickness of the coil") + " c:");
            mui->label_4_2->setText(tr("Wire diameter") + " d:");
            mui->label_5_2->setText(tr("Wire diameter with insulation") + " k:");
            mui->lineEdit_1_2->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2_2->setText(roundTo(data->l / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3_2->setText(roundTo(data->c / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            on_radioButton_6_toggled(mui->radioButton_6->isChecked());
            mui->lineEdit_5_2->setText(roundTo(data->k / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->comboBox_checkMLWinding->setCurrentIndex(myOpt->windingKind);
            on_comboBox_checkMLWinding_activated(myOpt->windingKind);
            mui->lineEdit_1_2->selectAll();
            break;
        }
        case _Multilayer_p:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4-0.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(false);
            mui->toolButton_showAdditional->setEnabled(false);
            if (myOpt->isAWG){
                mui->label_04_2->setText(tr("AWG"));
                mui->lineEdit_4_2->setValidator(awgV);
            }
            mui->label_N->setVisible(false);
            mui->lineEdit_N->setVisible(false);
            mui->lineEdit_1_2->setFocus();
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
            mui->checkBox_isInsulation2->setVisible(false);
            mui->lineEdit_3_2->setEnabled(true);
            mui->label_1_2->setText(tr("Former diameter") + " D:");
            mui->label_2_2->setText(tr("Winding length") + " l:");
            mui->label_3_2->setText(tr("Thickness of the coil") + " c:");
            mui->label_4_2->setText(tr("Wire diameter") + " d:");
            mui->label_5_2->setText(tr("Wire diameter with insulation") + " k:");
            mui->label_6_2->setText(tr("Insulation thickness")+" g:");
            mui->label_7_2->setText(tr("Layers number beetween insulating pads") + " Ng:");
            mui->lineEdit_1_2->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2_2->setText(roundTo(data->l / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3_2->setText(roundTo(data->c / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_4_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_4_2->setText("");
            } else
                mui->lineEdit_4_2->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (mui->lineEdit_4_2->text().isEmpty() || (mui->lineEdit_4_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_5_2->setText(roundTo(data->k / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            else
                on_lineEdit_4_2_editingFinished();
            mui->lineEdit_6_2->setText(roundTo(data->g / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_7_2->setText(roundTo(data->Ng / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_1_2->selectAll();
            break;
        }
        case _Multilayer_r:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4_square.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(true);
            mui->radioButton_8->setVisible(false);
            mui->radioButton_6->setText(tr("Number of turns of the coil") + " (N)");
            mui->radioButton_7->setText(tr("Thickness of the coil") + " (c)");
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(false);
            mui->toolButton_showAdditional->setEnabled(false);
            if (myOpt->isAWG){
                mui->label_05_2->setText(tr("AWG"));
                mui->lineEdit_5_2->setValidator(awgV);
            }
            mui->label_N->setVisible(false);
            mui->lineEdit_N->setVisible(false);
            mui->lineEdit_1_2->setFocus();
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
            mui->lineEdit_7_2->setVisible(false);
            mui->label_7_2->setVisible(false);
            mui->label_07_2->setVisible(false);
            mui->checkBox_isInsulation2->setVisible(false);
            mui->lineEdit_3_2->setEnabled(true);
            mui->label_1_2->setText(tr("Former width") + " a:");
            mui->label_2_2->setText(tr("Former height") + " b:");
            mui->label_3_2->setText(tr("Winding length") + " l:");
            mui->label_4_2->setText(tr("Thickness of the coil") + " c:");
            mui->label_5_2->setText(tr("Wire diameter") + " d:");
            mui->label_6_2->setText(tr("Wire diameter with insulation") + " k:");
            mui->lineEdit_1_2->setText(roundTo(data->a / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2_2->setText(roundTo(data->b / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3_2->setText(roundTo(data->l / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_4_2->setText(roundTo(data->c / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (mui->radioButton_8->isChecked())
                mui->radioButton_6->setChecked(true);
            on_radioButton_6_toggled(mui->radioButton_6->isChecked());
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_5_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_5_2->setText("");
            } else
                mui->lineEdit_5_2->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (mui->lineEdit_5_2->text().isEmpty() || (mui->lineEdit_4_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_6_2->setText(roundTo(data->k / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            else
                on_lineEdit_5_2_editingFinished();
            mui->lineEdit_1_2->selectAll();
            break;
        }
        case _Multilayer_f:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil11.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(false);
            mui->toolButton_showAdditional->setEnabled(false);
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->lineEdit_N->setFocus();
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
            mui->checkBox_isInsulation2->setVisible(false);
            mui->lineEdit_3_2->setEnabled(true);
            mui->label_1_2->setText(tr("Former diameter") + " D:");
            mui->label_2_2->setText(tr("Foil width") + " w:");
            mui->label_3_2->setText(tr("Foil thickness") + " t:");
            mui->label_4_2->setText(tr("Insulation thickness")+" g:");
            mui->lineEdit_N->setText(roundTo(data->N, loc, myOpt->dwAccuracy));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_freq2->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_1_2->setText(roundTo(data->D / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2_2->setText(roundTo(data->w / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3_2->setText(roundTo(data->t / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_4_2->setText(roundTo(data->isol / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            break;
        }
        case _FerrToroid:{
            mui->image->setPixmap(QPixmap(":/images/res/T-core.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(false);
            if(myOpt->isDockWidgetVisible)
                satCurrentDockWidget->show();
            mui->toolButton_Saturation->setEnabled(true);
            mui->toolButton_showAdditional->setEnabled(false);
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->lineEdit_N->setFocus();
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
            mui->label_05_2->setVisible(false);
            mui->lineEdit_6_2->setVisible(true);
            mui->label_6_2->setVisible(true);
            mui->label_06_2->setVisible(true);
            mui->lineEdit_7_2->setVisible(false);
            mui->label_7_2->setVisible(false);
            mui->label_07_2->setVisible(false);
            mui->checkBox_isInsulation2->setVisible(false);
            mui->lineEdit_3_2->setEnabled(true);
            mui->label_1_2->setText(tr("Outside diameter")+" OD:");
            mui->label_2_2->setText(tr("Inside diameter")+" ID:");
            mui->label_3_2->setText(tr("Core height") + " h:");
            mui->label_4_2->setText(tr("Wire diameter") + " d:");
            mui->label_5_2->setText(tr("Magnetic permeability")+" μ:");
            mui->lineEdit_N->setText(roundTo(data->N, loc, myOpt->dwAccuracy));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_1_2->setText(roundTo(data->Do / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2_2->setText(roundTo(data->Di / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3_2->setText(roundTo(data->h / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_4_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_4_2->setText("");
            } else
                mui->lineEdit_4_2->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_5_2->setText(roundTo(data->mu / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_6_2->setText(roundTo(data->Ch / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->label_6_2->setText(tr("Chamfer") + " C:");
            break;
        }
        case _PCB_coil:{
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(true);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(false);
            mui->toolButton_showAdditional->setEnabled(false);
            mui->lineEdit_3_2->setVisible(true);
            mui->label_3_2->setVisible(true);
            mui->label_03_2->setVisible(true);
            mui->lineEdit_4_2->setVisible(true);
            mui->label_4_2->setVisible(true);
            mui->label_04_2->setVisible(true);
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->lineEdit_N->setFocus();
            mui->label_freq2->setVisible(true);
            mui->label_freq_m2->setVisible(true);
            mui->lineEdit_freq2->setVisible(true);
            mui->lineEdit_5_2->setVisible(true);
            mui->label_5_2->setVisible(true);
            mui->label_05_2->setVisible(true);
            mui->lineEdit_6_2->setVisible(false);
            mui->label_6_2->setVisible(false);
            mui->label_06_2->setVisible(false);
            mui->lineEdit_7_2->setVisible(false);
            mui->label_7_2->setVisible(false);
            mui->label_07_2->setVisible(false);
            mui->checkBox_isInsulation2->setVisible(false);
            mui->lineEdit_3_2->setEnabled(true);
            mui->label_freq_m2->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
            mui->lineEdit_freq2->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
            mui->comboBox_checkPCB->setCurrentIndex(myOpt->layoutPCBcoil);
            on_comboBox_checkPCB_activated(myOpt->layoutPCBcoil);
            break;
        }
        case _Flat_Spiral:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil10.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            mui->comboBox_checkMLWinding->setVisible(false);
            mui->toolButton_FAQ->setVisible(false);
            mui->toolButton_showAdditional->setEnabled(false);
            if (myOpt->isAWG){
                mui->label_03_2->setText(tr("AWG"));
                mui->lineEdit_3_2->setValidator(awgV);
            }
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->lineEdit_N->setFocus();
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
            mui->checkBox_isInsulation2->setVisible(false);
            mui->lineEdit_3_2->setEnabled(true);
            mui->label_1_2->setText(tr("Outside diameter")+" Do:");
            mui->label_2_2->setText(tr("Inside diameter")+" Di:");
            mui->label_3_2->setText(tr("Wire diameter") + " d:");
            mui->lineEdit_N->setText(roundTo(data->N, loc, myOpt->dwAccuracy));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_1_2->setText(roundTo(data->Do / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2_2->setText(roundTo(data->Di / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_3_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_3_2->setText("");
            } else
                mui->lineEdit_3_2->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
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
    this->close();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionTo_null_data_triggered()
{
    QMessageBox messageBox(QMessageBox::Question,
                           tr("Confirmation"),
                           tr("Are you sure?"),
                           QMessageBox::Yes | QMessageBox::No,
                           this);
    messageBox.setButtonText(QMessageBox::Yes, tr("Yes"));
    messageBox.setButtonText(QMessageBox::No, tr("No"));
    if (messageBox.exec()== QMessageBox::Yes){
        data->N = 0;
        data->frequency = 0;
        data->inductance = 0;
        data->capacitance = 0;
        data->B = 0;
        data->D = 0;
        data->a = 0;
        data->b = 0;
        data->d = 0;
        data->k = 0;
        data->p = 0;
        data->w = 0;
        data->t = 0;
        data->isol = 0;
        data->l = 0;
        data->c = 0;
        data->Ch = 0;
        data->g = 0;
        data->Ng = 0;
        data->Do = 0;
        data->Di = 0;
        data->th = 0;
        data->h = 0;
        data->mu = 0;
        data->ratio = 0.5;
        data->zo = 0;
        data->s = 0;
        data->Rdc = 0;
        data->ns = 0;

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

        QSettings *settings;
        defineAppSettings(settings);
        settings->remove("Data");
        settings->remove("AirCoreToroid");
        settings->remove("AL");
        settings->remove("Amidon");
        settings->remove("ECore");
        settings->remove("FerriteRod");
        settings->remove("FindPermeability");
        settings->remove("Loop");
        settings->remove("Meander_PCB");
        settings->remove("Multi_loop");
        settings->remove("PotCore");
        settings->remove("rf_torus");
        settings->remove("Shield");
        settings->remove("UCore");
        settings->remove("RMcore");
        settings->remove("Bandspread");
        settings->remove("Crossover");
        settings->remove("Saturation_Dock");
        delete settings;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionThemeDefault_triggered()
{
    qApp->setStyleSheet("");
    mui->actionThemeSunny->setChecked(false);
    mui->actionThemeDark->setChecked(false);
    mui->actionThemeDefault->setChecked(true);
    if(myOpt->styleGUI == _DarkStyle)
        invertGUIconColor();
    myOpt->styleGUI = _DefaultStyle;
    resetUiFont();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionThemeDark_triggered()
{
    // loadstylesheet
    QFile qfDarkstyle(QStringLiteral(":/stylesheet/res/DarkStyle.qss"));
    if (qfDarkstyle.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // set stylesheet
        QString qsStylesheet = QString::fromLatin1(qfDarkstyle.readAll());
        qApp->setStyleSheet(qsStylesheet);
        qfDarkstyle.close();
    }
    //setStyleSheet(DARK_STYLE);
    mui->actionThemeSunny->setChecked(false);
    mui->actionThemeDark->setChecked(true);
    mui->actionThemeDefault->setChecked(false);
    if((myOpt->styleGUI == _DefaultStyle)||(myOpt->styleGUI == _SunnyStyle))
        invertGUIconColor();
    myOpt->styleGUI = _DarkStyle;
    resetUiFont();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionThemeSunny_triggered()
{
    // loadstylesheet
    QFile qfSunnystyle(QStringLiteral(":/stylesheet/res/SunnyStyle.qss"));
    if (qfSunnystyle.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // set stylesheet
        QString qsStylesheet = QString::fromLatin1(qfSunnystyle.readAll());
        qApp->setStyleSheet(qsStylesheet);
        qfSunnystyle.close();
    }
    //setStyleSheet(SUNNY_STYLE);
    mui->actionThemeSunny->setChecked(true);
    mui->actionThemeDark->setChecked(false);
    mui->actionThemeDefault->setChecked(false);
    if(myOpt->styleGUI == _DarkStyle)
        invertGUIconColor();
    myOpt->styleGUI = _SunnyStyle;
    resetUiFont();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionClear_all_triggered()
{
    if(!mui->textBrowser->document()->isEmpty()){
        bool isConfirmed;
        if (myOpt->isConfirmClear){
            QMessageBox messageBox(QMessageBox::Question,
                                   tr("Confirmation"),
                                   tr("Are you sure?"),
                                   QMessageBox::Yes | QMessageBox::No,
                                   this);
            messageBox.setButtonText(QMessageBox::Yes, tr("Yes"));
            messageBox.setButtonText(QMessageBox::No, tr("No"));
            if (messageBox.exec()== QMessageBox::Yes) isConfirmed = true;
            else isConfirmed = false;
        } else isConfirmed = true;
        if (isConfirmed){
            mui->textBrowser->clear();
            mui->statusBar->clearMessage();
            calc_count = 0;
        }
    }
    if (satCurrentDockWidget != nullptr){
        _FerriteData ferriteData;
        ferriteData.N = 0;
        ferriteData.mu = 0;
        ferriteData.le = 0;
        emit sendFerriteData(ferriteData);
    }
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
    mui->textBrowser->copy();
    QTextCursor c = mui->textBrowser->textCursor();
    c.movePosition(QTextCursor::Start);
    mui->textBrowser->setTextCursor(c);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionOpen_triggered()
{
    QString savePath = defineSavePath();
    QDir dir(savePath);
    if (dir.exists()){
        QString filters(".pdf (*.pdf);;.odf (*.odf);;.htm (*.htm)");
        QString defaultFilter(".htm (*.htm)");
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), savePath, filters, &defaultFilter);
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
                document->setHtml(file.readAll().data());
                file.close();
            }
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionSave_triggered()
{
    QTextDocument *document = mui->textBrowser->document();
    if (!document->isEmpty()){
        QString savePath = defineSavePath();
        QDir sD(savePath);
        if (!sD.exists()){
            sD.mkpath(savePath);
            QSettings *settings;
            defineAppSettings(settings);
            settings->beginGroup( "GUI" );
            settings->setValue("SaveDir", savePath);
            settings->endGroup();
            delete settings;
        }
        QString filters(".pdf (*.pdf);;.odf (*.odf);;.htm (*.htm)");
        if (myOpt->isOutputInTwoColumns){
            filters.remove(13, 14);
        }
        QString defaultFilter(".htm (*.htm)");
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save"), savePath + "/" + loc.toString(QDateTime::currentDateTime(), "yyyy-MM-dd_hh_mm_ss"), filters, &defaultFilter);
        if (!fileName.isEmpty()){
            QString ext = defaultFilter.mid(defaultFilter.indexOf("*") + 1, 4);
            int p = fileName.indexOf(".");
            if (p < 0){
                fileName.append(ext);
            }
            if (ext == ".htm"){
                QFile file(fileName);
                file.open(QIODevice::WriteOnly);
                file.write(document->toHtml().toUtf8());
                file.close();
            }
            if ((ext == ".odf") || (ext == ".pdf")){
                QString oldTxt = document->toHtml();
                QString newText = document->toHtml().remove(CLEAR_CHAR);
                document->setHtml(newText);
                if (ext == ".odf"){
                    QTextDocumentWriter textwriter(fileName);
                    textwriter.setFormat("odf");
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
                    if (myOpt->isOutputInTwoColumns)
                        newFont.setPointSize(8);
                    else
                        newFont.setPointSize(6);
                    document->setDefaultFont(newFont);
                    document->setPageSize(printer.pageRect().size());
                    document->print(&printer);
                    document->setDefaultFont(oldFont);
                }
                document->setHtml(oldTxt);
                QTextCursor c = mui->textBrowser->textCursor();
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionAbout_triggered()
{
    About *about = new About(this);
    about->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(this, SIGNAL(sendStyleGUI(int)), about, SLOT(getStyleGUI(int)));
    emit sendStyleGUI(myOpt->styleGUI);
    about->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_tabWidget_currentChanged(int index)
{
    switch (index){
    case 0:
    case 1:{
        mui->groupBox_List1->setVisible(true);
        mui->listWidget->setCurrentRow(FormCoil);
        on_listWidget_currentRowChanged(FormCoil);
        break;
    }
    case 2:{
        mui->groupBox_List1->setVisible(false);
        mui->comboBox_checkPCB->setVisible(false);
        mui->comboBox_checkMLWinding->setVisible(false);
        mui->toolButton_FAQ->setVisible(false);
        satCurrentDockWidget->hide();
        mui->toolButton_Saturation->setEnabled(false);
        mui->image->setPixmap(QPixmap(":/images/res/LC.png"));
        if (mui->radioButton_LC->isChecked())
            on_radioButton_LC_clicked();
        if (mui->radioButton_CF->isChecked())
            on_radioButton_CF_clicked();
        if (mui->radioButton_LF->isChecked())
            on_radioButton_LF_clicked();
        if (mui->radioButton_ZF->isChecked())
            on_radioButton_ZF_clicked();
        mui->lineEdit_1_3->setFocus();
        mui->lineEdit_1_3->selectAll();
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
    mui->lineEdit_1_3->setText(roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy));
    mui->groupBox_2_3->setTitle(tr("Circuit capacitance"));
    mui->lineEdit_2_3->setText(roundTo(data->capacitance / myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy));
    mui->label_01_3->setText(qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()));
    mui->label_02_3->setText(qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()));
    mui->lineEdit_1_3->setFocus();
    mui->lineEdit_1_3->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_CF_clicked()
{
    mui->groupBox_1_3->setTitle(tr("Circuit capacitance"));
    mui->lineEdit_1_3->setText(roundTo(data->capacitance / myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy));
    mui->groupBox_2_3->setTitle(tr("Frequency"));
    mui->lineEdit_2_3->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
    mui->label_01_3->setText(qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()));
    mui->label_02_3->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
    mui->lineEdit_1_3->setFocus();
    mui->lineEdit_1_3->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_LF_clicked()
{
    mui->groupBox_1_3->setTitle(tr("Inductance"));
    mui->lineEdit_1_3->setText(roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy));
    mui->groupBox_2_3->setTitle(tr("Frequency"));
    mui->lineEdit_2_3->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
    mui->label_01_3->setText(qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()));
    mui->label_02_3->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
    mui->lineEdit_1_3->setFocus();
    mui->lineEdit_1_3->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_ZF_clicked()
{
    mui->groupBox_1_3->setTitle(tr("Characteristic impedance"));
    mui->lineEdit_1_3->setText(roundTo(data->zo, loc, myOpt->dwAccuracy));
    mui->groupBox_2_3->setTitle(tr("Frequency"));
    mui->lineEdit_2_3->setText(roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy));
    mui->label_01_3->setText(tr("Ohm"));
    mui->label_02_3->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
    mui->lineEdit_1_3->setFocus();
    mui->lineEdit_1_3->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_checkBox_isInsulation_toggled(bool checked)
{
    if (checked){
        mui->lineEdit_3->setEnabled(true);
        myOpt->isAWGhasInsulation = true;
    } else {
        mui->lineEdit_3->setEnabled(false);
        myOpt->isAWGhasInsulation = false;
    }
    QToolTip::showText(mui->checkBox_isInsulation->mapToGlobal(QPoint( 0, 0 )), mui->checkBox_isInsulation->toolTip());
    on_lineEdit_2_editingFinished();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_checkBox_isInsulation2_toggled(bool checked)
{
    if (checked){
        mui->lineEdit_3_2->setEnabled(true);
        myOpt->isAWGhasInsulation = true;
    } else {
        mui->lineEdit_3_2->setEnabled(false);
        myOpt->isAWGhasInsulation = false;
    }
    QToolTip::showText(mui->checkBox_isInsulation2->mapToGlobal(QPoint( 0, 0 )), mui->checkBox_isInsulation2->toolTip());
    on_lineEdit_2_2_editingFinished();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_1_clicked(bool checked)
{
    mui->radioButton_1_2->setChecked(checked);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_2_clicked(bool checked)
{
    mui->radioButton_2_2->setChecked(checked);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_3_clicked(bool checked)
{
    mui->radioButton_3_2->setChecked(checked);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_4_clicked(bool checked)
{
    mui->radioButton_4_2->setChecked(checked);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_5_1_toggled(bool checked)
{
    if (FormCoil == _Onelayer_cw){
        if (checked){
            mui->label_2->setText(tr("Wire diameter") + " d:");
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2->setText("");
                mui->label_02->setText(tr("AWG"));
                mui->lineEdit_2->setValidator(awgV);
            } else {
                mui->lineEdit_2->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            }
            if (mui->lineEdit_2->text().isEmpty() || (mui->lineEdit_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_3->setText(roundTo(data->k / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            else
                on_lineEdit_2_editingFinished();
            mui->label_3->setVisible(true);
            mui->lineEdit_3->setVisible(true);
            mui->label_03->setVisible(true);
            myOpt->isWindingLengthOneLayerInit = false;
        } else {
            mui->label_2->setText(tr("Winding length") + " l:");
            mui->label_02->setText(qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()));
            mui->lineEdit_2->setValidator(dv);
            if (data->l > 0){
                mui->lineEdit_2->setText(roundTo(data->l / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            } else
                mui->lineEdit_2->setText("");
            mui->label_3->setVisible(false);
            mui->lineEdit_3->setVisible(false);
            mui->label_03->setVisible(false);
            myOpt->isWindingLengthOneLayerInit = true;
        }
        mui->lineEdit_ind->setFocus();
        mui->lineEdit_ind->selectAll();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_1_2_clicked(bool checked)
{
    mui->radioButton_1->setChecked(checked);
    mui->lineEdit_N->setFocus();
    mui->lineEdit_N->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_2_2_clicked(bool checked)
{
    mui->radioButton_2->setChecked(checked);
    mui->lineEdit_N->setFocus();
    mui->lineEdit_N->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_3_2_clicked(bool checked)
{
    mui->radioButton_3->setChecked(checked);
    mui->lineEdit_N->setFocus();
    mui->lineEdit_N->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_4_2_clicked(bool checked)
{
    mui->radioButton_4->setChecked(checked);
    mui->lineEdit_N->setFocus();
    mui->lineEdit_N->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_Num_clicked()
{
    myOpt->isShowCalcNum = mui->toolButton_Num->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_showTitle_clicked()
{
    myOpt->isShowTitle = mui->toolButton_showTitle->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_showImg_clicked()
{
    myOpt->isInsertImage = mui->toolButton_showImg->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_ltspice_clicked()
{
    myOpt->isShowLTSpice = mui->toolButton_ltspice->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_Color_clicked()
{
    myOpt->isOutputValueColored = mui->toolButton_Color->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_Desc_clicked()
{
    myOpt->isShowValueDescription = mui->toolButton_Desc->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_column_clicked()
{
    myOpt->isOutputInTwoColumns = mui->toolButton_column->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_Saturation_clicked()
{
        if (satCurrentDockWidget->isVisible()){
            satCurrentDockWidget->hide();
            myOpt->isDockWidgetVisible = false;
            mui->toolButton_Saturation->setChecked(false);
        } else {
            satCurrentDockWidget->show();
            myOpt->isDockWidgetVisible = true;
            mui->toolButton_Saturation->setChecked(true);
        }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::showSaturationDockWidget(bool isShow)
{
    if (isShow){
        satCurrentDockWidget->show();
        myOpt->isDockWidgetVisible = true;
        mui->toolButton_Saturation->setChecked(true);
    } else {
        satCurrentDockWidget->hide();
        myOpt->isDockWidgetVisible = false;
        mui->toolButton_Saturation->setChecked(false);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_showAdditional_clicked()
{
    myOpt->isAdditionalResult = mui->toolButton_showAdditional->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_cbe_clicked()
{
    myOpt->isConfirmExit = mui->toolButton_cbe->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_cbc_clicked()
{
    myOpt->isConfirmClear = mui->toolButton_cbc->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_cdsr_clicked()
{
    myOpt->isConfirmDelete = mui->toolButton_cdsr->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_soe_clicked()
{
    myOpt->isSaveOnExit = mui->toolButton_soe->isChecked();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_lShowFirst_clicked()
{
    myOpt->isLastShowingFirst = mui->toolButton_lShowFirst->isChecked();
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
void MainWindow::on_toolButton_FAQ_clicked()
{
    switch (FormCoil) {
    case _Onelayer_cw:
    case _Onelayer:
    case _Onelayer_p:
    case _Onelayer_q:{
        QFile helpFile(":/txt/res/onelayer.pdf");
        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation).toUtf8();
        QString tempFileName = tempDir.append("/onelayer.pdf");
        helpFile.copy(tempFileName);
        QDesktopServices::openUrl(QUrl::fromLocalFile(tempFileName));
        break;
    }
    default: break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_toolButton_Configure_clicked()
{
    on_actionOptions_triggered();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_6_toggled(bool checked)
{
    if (FormCoil == _Multilayer){
        if (checked){
            mui->label_3_2->setText(tr("Number of turns of the coil") + " N:");
            mui->label_03_2->setVisible(false);
            mui->lineEdit_3_2->setText(roundTo(data->N, loc, myOpt->dwAccuracy));
        } else {
            mui->label_3_2->setText(tr("Thickness of the coil") + " c:");
            mui->label_03_2->setVisible(true);
            mui->lineEdit_3_2->setText(roundTo(data->c / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
        }
        on_radioButton_8_toggled(mui->radioButton_8->isChecked());
    } else if (FormCoil == _Multilayer_r){
        if (checked){
            mui->label_4_2->setText(tr("Number of turns of the coil") + " N:");
            mui->label_04_2->setVisible(false);
            mui->lineEdit_4_2->setText(roundTo(data->N, loc, myOpt->dwAccuracy));
        } else {
            mui->label_4_2->setText(tr("Thickness of the coil") + " c:");
            mui->label_04_2->setVisible(true);
            mui->lineEdit_4_2->setText(roundTo(data->c / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
        }
        mui->lineEdit_1_2->setFocus();
        mui->lineEdit_1_2->selectAll();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_8_toggled(bool checked)
{
    if (FormCoil == _Multilayer){
        if (checked){
            mui->label_4_2->setText(tr("Resistance of the coil") + " Rdc:");
            mui->label_04_2->setText(tr("Ohm"));
            mui->lineEdit_4_2->setText(roundTo(data->Rdc, loc, myOpt->dwAccuracy));
        } else {
            mui->label_4_2->setText(tr("Wire diameter") + " d:");
            mui->label_04_2->setText(tr("mm"));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->label_04_2->setText(tr("AWG"));
                    mui->lineEdit_4_2->setText(converttoAWG(data->d));
                } else {
                    mui->lineEdit_4_2->setText("");
                }
            } else
                mui->lineEdit_4_2->setText(roundTo(data->d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
        }
        mui->lineEdit_1_2->setFocus();
        mui->lineEdit_1_2->selectAll();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::getOptionStruct(_OptionStruct gOpt){
    *myOpt = gOpt;
    completeOptionsStructure(myOpt);
    on_tabWidget_currentChanged(mui->tabWidget->currentIndex());
    mui->toolButton_showImg->setChecked(myOpt->isInsertImage);
    mui->toolButton_showTitle->setChecked(myOpt->isShowTitle);
    mui->toolButton_ltspice->setChecked(myOpt->isShowLTSpice);
    mui->toolButton_showAdditional->setChecked(myOpt->isAdditionalResult);
    mui->toolButton_cbc->setChecked(myOpt->isConfirmClear);
    mui->toolButton_cbe->setChecked(myOpt->isConfirmExit);
    mui->toolButton_cdsr->setChecked(myOpt->isConfirmDelete);
    mui->toolButton_soe->setChecked(myOpt->isSaveOnExit);
    mui->toolButton_lShowFirst->setChecked(myOpt->isLastShowingFirst);
    mui->toolButton_Desc->setChecked(myOpt->isShowValueDescription);
    mui->toolButton_column->setChecked(myOpt->isOutputInTwoColumns);
    mui->toolButton_Color->setChecked(myOpt->isOutputValueColored);
    mui->toolButton_Num->setChecked(myOpt->isShowCalcNum);
    if(myOpt->isEnglishLocale)loc = QLocale::English;
    else loc = getLanguageLocale(lang);
    this->setLocale(loc);
    dv->setLocale(loc);
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    int tab = mui->tabWidget->currentIndex();
    on_tabWidget_currentChanged(tab);
    switch (myOpt->styleGUI) {
    case _DefaultStyle:
        on_actionThemeDefault_triggered();
        break;
    case _DarkStyle:
        on_actionThemeDark_triggered();
        break;
    case _SunnyStyle:
        on_actionThemeSunny_triggered();
        break;
    default:
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_ind_editingFinished()
{
    if (!mui->lineEdit_ind->text().isEmpty()){
        bool ok;
        data->inductance = loc.toDouble(mui->lineEdit_ind->text(), &ok)*myOpt->dwInductanceMultiplier;
        if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_freq_editingFinished()
{
    if (!mui->lineEdit_freq->text().isEmpty()){
        bool ok;
        data->frequency = loc.toDouble(mui->lineEdit_freq->text(), &ok)*myOpt->dwFrequencyMultiplier;
        if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_1_editingFinished()
{
    if (!mui->lineEdit_1->text().isEmpty()){
        bool ok;
        switch (FormCoil) {
        case _Onelayer_cw:
        case _Onelayer:
        case _Onelayer_p:
        case _Onelayer_q:
        case _Multilayer:
        case _Multilayer_p:
        case _Multilayer_f:{
            data->D = loc.toDouble(mui->lineEdit_1->text(),&ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer_r:{
            data->a = loc.toDouble(mui->lineEdit_1->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _FerrToroid:
        case _PCB_coil:{
            data->Do = loc.toDouble(mui->lineEdit_1->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            switch (myOpt->layoutPCBcoil) {
            case 0:{
                data->Di = data->Do * 0.362;
                mui->lineEdit_2->setText(roundTo(data->Di / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                break;
            }
            case 1:{
                data->Di = data->Do * 0.4;
                mui->lineEdit_2->setText(roundTo(data->Di / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                break;
            }
            default:
                break;
            }
            break;
        }
        case _Flat_Spiral:{
            data->Di = loc.toDouble(mui->lineEdit_1->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
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
        bool ok;
        switch (FormCoil) {
        case _Onelayer_cw:{
            if (myOpt->isWindingLengthOneLayerInit){
                data->l = loc.toDouble(mui->lineEdit_2->text(), &ok)*myOpt->dwLengthMultiplier;
            } else {
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_2->text(), &ok);
                } else {
                    d = loc.toDouble(mui->lineEdit_2->text(), &ok)*myOpt->dwLengthMultiplier;
                }
                if (!ok){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                data->d = d;
                double k_m = odCalc(d);
                if (d > 0){
                    mui->lineEdit_3->setText( roundTo(k_m / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                }
            }
            break;
        }
        case _Onelayer:
        case _Onelayer_q:{
            double d = 0;
            if (myOpt->isAWG){
                d = convertfromAWG(mui->lineEdit_2->text(), &ok);
            } else {
                d = loc.toDouble(mui->lineEdit_2->text(), &ok)*myOpt->dwLengthMultiplier;
            }
            if (!ok){
                showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                return;
            }
            data->d = d;
            double k_m;
            if ((FormCoil == _Onelayer) && (!mui->checkBox_isInsulation->isChecked()) && (mui->checkBox_isInsulation->isVisible()))
                k_m = d;
            else
                k_m = odCalc(d);
            if (d > 0){
                mui->lineEdit_3->setText( roundTo(k_m / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            }
            break;
        }
        case _Onelayer_p:
        case _Multilayer_f:{
            data->w = loc.toDouble(mui->lineEdit_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer:
        case _Multilayer_p:{
            data->l = loc.toDouble(mui->lineEdit_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer_r:{
            data->b = loc.toDouble(mui->lineEdit_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _FerrToroid:
        case _PCB_coil:{
            data->Di = loc.toDouble(mui->lineEdit_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Flat_Spiral:{
            double d = 0;
            if (myOpt->isAWG){
                d = convertfromAWG(mui->lineEdit_2->text(), &ok);
            } else {
                d = loc.toDouble(mui->lineEdit_2->text(), &ok)*myOpt->dwLengthMultiplier;
            }
            if (!ok){
                showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                return;
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
        bool ok;
        switch (FormCoil) {
        case _Onelayer_cw:
        case _Onelayer:
        case _Onelayer_q:{
            data->k = loc.toDouble(mui->lineEdit_3->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Onelayer_p:
        case _Multilayer_f:{
            data->t = loc.toDouble(mui->lineEdit_3->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer:
        case _Multilayer_p:{
            double d = 0;
            if (myOpt->isAWG){
                d = convertfromAWG(mui->lineEdit_3->text(), &ok);
            } else {
                d = loc.toDouble(mui->lineEdit_3->text(), &ok)*myOpt->dwLengthMultiplier;
            }
            if (!ok){
                showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                return;
            }
            data->d = d;
            double k_m = odCalc(d);
            if (d > 0){
                mui->lineEdit_4->setText( roundTo(k_m / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            }
            break;
        }
        case _Multilayer_r:{
            data->l = loc.toDouble(mui->lineEdit_3->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _FerrToroid:{
            data->h = loc.toDouble(mui->lineEdit_3->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _PCB_coil:{
            data->a = loc.toDouble(mui->lineEdit_3->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Flat_Spiral:{
            data->s = loc.toDouble(mui->lineEdit_3->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
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
        bool ok;
        switch (FormCoil) {
        case _Onelayer:
        case _Onelayer_q:{
            data->p = loc.toDouble(mui->lineEdit_4->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Onelayer_p:
        case _Multilayer_f:{
            data->isol = loc.toDouble(mui->lineEdit_4->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer:
        case _Multilayer_p:{
            data->k = loc.toDouble(mui->lineEdit_4->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer_r:{
            double d = 0;
            if (myOpt->isAWG){
                d = convertfromAWG(mui->lineEdit_4->text(), &ok);
            } else {
                d = loc.toDouble(mui->lineEdit_4->text(), &ok)*myOpt->dwLengthMultiplier;
            }
            if (!ok){
                showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                return;
            }
            data->d = d;
            double k_m = odCalc(d);
            if (d > 0){
                mui->lineEdit_5->setText( roundTo(k_m / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            }
            break;
        }
        case _FerrToroid:{
            double d = 0;
            if (myOpt->isAWG){
                d = convertfromAWG(mui->lineEdit_4->text(), &ok);
            } else {
                d = loc.toDouble(mui->lineEdit_4->text(), &ok)*myOpt->dwLengthMultiplier;
            }
            if (!ok){
                showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                return;
            }
            data->d = d;
            break;
        }
        case _PCB_coil:{
            data->th = loc.toDouble(mui->lineEdit_3->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
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
        bool ok;
        switch (FormCoil) {
        case _Onelayer_p:{
            data->p = loc.toDouble(mui->lineEdit_5->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Onelayer_q:{
            data->ns = loc.toInt(mui->lineEdit_5->text(), &ok);
            if ((!ok) || (data->ns < 3)) showWarning(tr("Warning"), tr("The number of sides must be integer and at least three!"));
            break;
        }
        case _Multilayer_p:{
            data->g = loc.toDouble(mui->lineEdit_5->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer_r:{
            data->k = loc.toDouble(mui->lineEdit_5->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _FerrToroid:{
            data->mu = loc.toDouble(mui->lineEdit_5->text(), &ok);
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
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
        bool ok;
        switch (FormCoil) {
        case _FerrToroid:{
            data->Ch = loc.toDouble(mui->lineEdit_6->text(), &ok);
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer_p:{
            data->Ng = loc.toDouble(mui->lineEdit_6->text(), &ok);
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
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
void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    QString hint = loc.toString(position / 100.0);
    mui->horizontalSlider->setToolTip(hint);
    QToolTip::showText(QCursor::pos(), hint);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_N_editingFinished()
{
    if (!mui->lineEdit_N->text().isEmpty()){
        bool ok;
        data->N = loc.toDouble(mui->lineEdit_N->text(), &ok);
        if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_freq2_editingFinished()
{
    if (!mui->lineEdit_freq2->text().isEmpty()){
        bool ok;
        data->frequency = loc.toDouble(mui->lineEdit_freq2->text(), &ok)*myOpt->dwFrequencyMultiplier;
        if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_lineEdit_1_2_editingFinished()
{
    if (!mui->lineEdit_1_2->text().isEmpty()){
        bool ok;
        switch (FormCoil) {
        case _Onelayer_cw:
        case _Onelayer:
        case _Onelayer_p:
        case _Onelayer_q:
        case _Multilayer:
        case _Multilayer_p:
        case _Multilayer_f:{
            data->D = loc.toDouble(mui->lineEdit_1_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer_r:{
            data->a = loc.toDouble(mui->lineEdit_1_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _FerrToroid:
        case _PCB_coil:{
            data->Do = loc.toDouble(mui->lineEdit_1_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Flat_Spiral:{
            data->Do = loc.toDouble(mui->lineEdit_1_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
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
        bool ok;
        switch (FormCoil) {
        case _Onelayer_cw:
        case _Onelayer:
        case _Onelayer_q:{
            double d = 0;
            if (myOpt->isAWG){
                d = convertfromAWG(mui->lineEdit_2_2->text(), &ok);
            } else {
                d = loc.toDouble(mui->lineEdit_2_2->text(), &ok)*myOpt->dwLengthMultiplier;
            }
            if (!ok){
                showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                return;
            }
            data->d = d;
            double k_m;
            if ((FormCoil == _Onelayer) && (!mui->checkBox_isInsulation2->isChecked()) && (mui->checkBox_isInsulation2->isVisible()))
                k_m = d;
            else
                k_m = odCalc(d);
            if (d > 0){
                mui->lineEdit_3_2->setText( roundTo(k_m/myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            }
            break;
        }
        case _Onelayer_p:
        case _Multilayer_f:{
            data->w = loc.toDouble(mui->lineEdit_2_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer:
        case _Multilayer_p:{
            data->l = loc.toDouble(mui->lineEdit_2_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer_r:{
            data->b = loc.toDouble(mui->lineEdit_2_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _FerrToroid:
        case _PCB_coil:{
            if(mui->comboBox_checkPCB->currentIndex() != 2){
                data->Di = loc.toDouble(mui->lineEdit_2_2->text(), &ok)*myOpt->dwLengthMultiplier;
                if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            } else {
                data->B = loc.toDouble(mui->lineEdit_2_2->text(), &ok)*myOpt->dwLengthMultiplier;
                if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            }
            break;
        }
        case _Flat_Spiral:{
            data->Di = loc.toDouble(mui->lineEdit_2_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
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
        bool ok;
        switch (FormCoil) {
        case _Onelayer_cw:
        case _Onelayer:
        case _Onelayer_q:{
            data->k = loc.toDouble(mui->lineEdit_3_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Onelayer_p:
        case _Multilayer_f:{
            data->t = loc.toDouble(mui->lineEdit_3_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer:{
            if(mui->radioButton_6->isChecked()){
                data->N = loc.toDouble(mui->lineEdit_3_2->text(), &ok);
                if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            } else {
                data->c = loc.toDouble(mui->lineEdit_3_2->text(), &ok)*myOpt->dwLengthMultiplier;
                if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            }
            break;
        }
        case _Multilayer_p:{
            data->c = loc.toDouble(mui->lineEdit_3_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer_r:{
            data->l = loc.toDouble(mui->lineEdit_3_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _FerrToroid:{
            data->h = loc.toDouble(mui->lineEdit_3_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _PCB_coil:{
            data->s = loc.toDouble(mui->lineEdit_3_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Flat_Spiral:{
            double d = 0;
            if (myOpt->isAWG){
                d = convertfromAWG(mui->lineEdit_3_2->text(), &ok);
            } else {
                d = loc.toDouble(mui->lineEdit_3_2->text(), &ok)*myOpt->dwLengthMultiplier;
            }
            if (!ok){
                showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                return;
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
        bool ok;
        switch (FormCoil) {
        case _Onelayer:
        case _Onelayer_q:{
            data->p = loc.toDouble(mui->lineEdit_4_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Onelayer_p:
        case _Multilayer_f:{
            data->isol = loc.toDouble(mui->lineEdit_4_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer:{
            if ((mui->radioButton_6->isChecked()) || (mui->radioButton_7->isChecked())){
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_4_2->text(), &ok);
                } else {
                    d = loc.toDouble(mui->lineEdit_4_2->text(), &ok)*myOpt->dwLengthMultiplier;
                }
                if (!ok){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                data->d = d;
                double k_m = odCalc(d);
                if (d > 0){
                    mui->lineEdit_5_2->setText( roundTo(k_m/myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                }
            } else {
                data->Rdc = loc.toDouble(mui->lineEdit_4_2->text(), &ok);
                if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            }
            break;
        }
        case _Multilayer_p:{
            double d = 0;
            if (myOpt->isAWG){
                d = convertfromAWG(mui->lineEdit_4_2->text(), &ok);
            } else {
                d = loc.toDouble(mui->lineEdit_4_2->text(), &ok)*myOpt->dwLengthMultiplier;
            }
            if (!ok){
                showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                return;
            }
            data->d = d;
            double k_m = odCalc(d);
            if (d > 0){
                mui->lineEdit_5_2->setText( roundTo(k_m/myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            }
            break;
        }
        case _Multilayer_r:{
            data->c = loc.toDouble(mui->lineEdit_4_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _FerrToroid:{
            double d = 0;
            if (myOpt->isAWG){
                d = convertfromAWG(mui->lineEdit_4_2->text(), &ok);
            } else {
                d = loc.toDouble(mui->lineEdit_4_2->text(), &ok)*myOpt->dwLengthMultiplier;
            }
            if (!ok){
                showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                return;
            }
            data->d = d;
            break;
        }
        case _PCB_coil:{
            data->w = loc.toDouble(mui->lineEdit_4_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
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
        bool ok;
        switch (FormCoil) {
        case _Onelayer_p:{
            data->p = loc.toDouble(mui->lineEdit_5_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Onelayer_q:{
            data->ns = loc.toInt(mui->lineEdit_5_2->text(), &ok);
            if ((!ok) || (data->ns < 3)) showWarning(tr("Warning"), tr("The number of sides must be integer and at least three!"));
            break;
        }
        case _Multilayer:
        case _Multilayer_p:{
            data->k = loc.toDouble(mui->lineEdit_5_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer_r:{
            double d = 0;
            if (myOpt->isAWG){
                d = convertfromAWG(mui->lineEdit_5_2->text(), &ok);
            } else {
                d = loc.toDouble(mui->lineEdit_5_2->text(), &ok)*myOpt->dwLengthMultiplier;
            }
            if (!ok){
                showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                return;
            }
            data->d = d;
            double k_m = odCalc(d);
            if (d > 0){
                mui->lineEdit_6_2->setText( roundTo(k_m/myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            }
            break;
        }
        case _FerrToroid:{
            data->mu = loc.toDouble(mui->lineEdit_5_2->text(), &ok);
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _PCB_coil:{
            data->th = loc.toDouble(mui->lineEdit_5_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
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
        bool ok;
        switch (FormCoil) {
        case _Multilayer_p:{
            data->g = loc.toDouble(mui->lineEdit_6_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _Multilayer_r:{
            data->k = loc.toDouble(mui->lineEdit_6_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _FerrToroid:{
            data->Ch = loc.toDouble(mui->lineEdit_6_2->text(), &ok);
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
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
        bool ok;
        switch (FormCoil) {
        case _Multilayer_p:{
            data->Ng = loc.toDouble(mui->lineEdit_7_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
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
            case _Onelayer_cw:{
                if (mui->lineEdit_freq->text().isEmpty()||((mui->lineEdit_3->text().isEmpty()) && !myOpt->isWindingLengthOneLayerInit)){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5 = true;
                double I = loc.toDouble(mui->lineEdit_ind->text(),&ok1)*myOpt->dwInductanceMultiplier;
                double f = loc.toDouble(mui->lineEdit_freq->text(),&ok2)*myOpt->dwFrequencyMultiplier;
                double D = loc.toDouble(mui->lineEdit_1->text(),&ok3)*myOpt->dwLengthMultiplier;
                double d = 0;
                double k = 0;
                if ((myOpt->isWindingLengthOneLayerInit) && (FormCoil == _Onelayer_cw)){
                    d = loc.toDouble(mui->lineEdit_2->text(),&ok4)*myOpt->dwLengthMultiplier; //when winding length option is activated, winding length substitute wire diameter & k=0
                } else {
                    if (myOpt->isAWG){
                        d = convertfromAWG(mui->lineEdit_2->text(),&ok4);
                    } else {
                        d = loc.toDouble(mui->lineEdit_2->text(),&ok4)*myOpt->dwLengthMultiplier;
                    }
                    k = loc.toDouble(mui->lineEdit_3->text(),&ok5)*myOpt->dwLengthMultiplier;
                }
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((I == 0)||(D == 0)||(d == 0)||((k == 0) && !myOpt->isWindingLengthOneLayerInit)||(f == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if ((k < d) && !myOpt->isWindingLengthOneLayerInit){
                    showWarning(tr("Warning"), "k < d");
                    return;
                }
                data->inductance = I;
                data->frequency = f;

                double Dk = D + k;
                Material mt = Cu;
                checkMaterial1(&mt);
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_freq->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
                mui->lineEdit_1->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_2->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                mui->lineEdit_3->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, d, k, I, f, 0, 0, myOpt->dwAccuracy, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerN_roundW_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Onelayer:{
                if (mui->lineEdit_freq->text().isEmpty()||(mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6;
                double I = loc.toDouble(mui->lineEdit_ind->text(),&ok1)*myOpt->dwInductanceMultiplier;
                double f = loc.toDouble(mui->lineEdit_freq->text(),&ok2)*myOpt->dwFrequencyMultiplier;
                double D = loc.toDouble(mui->lineEdit_1->text(),&ok3)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_2->text(),&ok4);
                } else {
                    d = loc.toDouble(mui->lineEdit_2->text(),&ok4)*myOpt->dwLengthMultiplier;
                }
                double k = loc.toDouble(mui->lineEdit_3->text(),&ok5)*myOpt->dwLengthMultiplier;
                double p = loc.toDouble(mui->lineEdit_4->text(),&ok6)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
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
                checkMaterial1(&mt);
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_freq->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
                mui->lineEdit_1->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_2->setText(roundTo(d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->lineEdit_3->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                mui->lineEdit_4->setText(loc.toString(p / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, d, p, I, f, 0, 0, myOpt->dwAccuracy, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerN_roundW_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Onelayer_p:{
                if ((mui->lineEdit_freq->text().isEmpty())||(mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty()) ||(mui->lineEdit_5->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6, ok7;
                double I = loc.toDouble(mui->lineEdit_ind->text(),&ok1)*myOpt->dwInductanceMultiplier;
                double f = loc.toDouble(mui->lineEdit_freq->text(),&ok2)*myOpt->dwFrequencyMultiplier;
                double D = loc.toDouble(mui->lineEdit_1->text(),&ok3)*myOpt->dwLengthMultiplier;
                double w = loc.toDouble(mui->lineEdit_2->text(),&ok4)*myOpt->dwLengthMultiplier;
                double t = loc.toDouble(mui->lineEdit_3->text(),&ok5)*myOpt->dwLengthMultiplier;
                double ins = loc.toDouble(mui->lineEdit_4->text(),&ok6)*myOpt->dwLengthMultiplier;
                double p = loc.toDouble(mui->lineEdit_5->text(),&ok7)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok7)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((I == 0)||(D == 0)||(w == 0)||(t == 0)||(p == 0)||(f == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (p < (w + 2*ins)){
                    showWarning(tr("Warning"), "p < (w + 2i)");
                    return;
                }
                if ((ins == 0)&&(p <= w)){
                    showWarning(tr("Warning"), "p <= w");
                    return;
                }
                data->inductance = I;
                data->frequency = f;
                double Dk = D + t + ins;
                Material mt = Cu;
                checkMaterial1(&mt);
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_freq->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
                mui->lineEdit_1->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                mui->lineEdit_2->setText(loc.toString(w / myOpt->dwLengthMultiplier));
                mui->lineEdit_3->setText(loc.toString(t / myOpt->dwLengthMultiplier));
                mui->lineEdit_4->setText(loc.toString(ins / myOpt->dwLengthMultiplier));
                mui->lineEdit_5->setText(loc.toString(p / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, w, t, p, I, f, 0, myOpt->dwAccuracy, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerN_rectW_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Onelayer_q:{
                if (mui->lineEdit_freq->text().isEmpty()||(mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty())
                        ||(mui->lineEdit_5->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6, ok7;
                double I = loc.toDouble(mui->lineEdit_ind->text(),&ok1)*myOpt->dwInductanceMultiplier;
                double f = loc.toDouble(mui->lineEdit_freq->text(),&ok2)*myOpt->dwFrequencyMultiplier;
                double D = loc.toDouble(mui->lineEdit_1->text(),&ok3)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_2->text(),&ok4);
                } else {
                    d = loc.toDouble(mui->lineEdit_2->text(),&ok4)*myOpt->dwLengthMultiplier;
                }
                double k = loc.toDouble(mui->lineEdit_3->text(),&ok5)*myOpt->dwLengthMultiplier;
                double p = loc.toDouble(mui->lineEdit_4->text(),&ok6)*myOpt->dwLengthMultiplier;
                int n = loc.toInt(mui->lineEdit_5->text(), &ok7);
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
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
                if ((n < 3)||(!ok7)){
                    showWarning(tr("Warning"), tr("The number of sides must be integer and at least three!"));
                    return;
                }
                data->inductance = I;
                data->frequency = f;

                double Dk = D + k;
                double _n = (double) n;
                Material mt = Cu;
                checkMaterial1(&mt);
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_freq->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
                mui->lineEdit_1->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_2->setText(roundTo(d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->lineEdit_3->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                mui->lineEdit_4->setText(loc.toString(p / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, Dk, d, p, f, _n, 0, myOpt->dwAccuracy, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerN_Poligonal_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Multilayer:{
                if ((mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5;
                double I = loc.toDouble(mui->lineEdit_ind->text(),&ok1)*myOpt->dwInductanceMultiplier;
                double D = loc.toDouble(mui->lineEdit_1->text(),&ok2)*myOpt->dwLengthMultiplier;
                double l = loc.toDouble(mui->lineEdit_2->text(),&ok3)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_3->text(),&ok4);
                } else {
                    d = loc.toDouble(mui->lineEdit_3->text(),&ok4)*myOpt->dwLengthMultiplier;
                }
                double k = loc.toDouble(mui->lineEdit_4->text(),&ok5)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((I == 0)||(D == 0)||(l == 0)||(d == 0)||(k == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (k < d){
                    showWarning(tr("Warning"), "k < d");
                    return;
                }
                int windingKind = mui->comboBox_checkMLWinding->currentIndex();
                data->inductance = I;
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_1->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                mui->lineEdit_2->setText(loc.toString(l / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_3->setText(roundTo(d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->lineEdit_4->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, D, d, k, l, 0, 0, windingKind );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerN_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Multilayer_p:{
                if ((mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6, ok7;
                double I = loc.toDouble(mui->lineEdit_ind->text(),&ok1)*myOpt->dwInductanceMultiplier;
                double D = loc.toDouble(mui->lineEdit_1->text(),&ok2)*myOpt->dwLengthMultiplier;
                double l = loc.toDouble(mui->lineEdit_2->text(),&ok3)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_3->text(),&ok4);
                } else {
                    d = loc.toDouble(mui->lineEdit_3->text(),&ok4)*myOpt->dwLengthMultiplier;
                }
                double k = loc.toDouble(mui->lineEdit_4->text(),&ok5)*myOpt->dwLengthMultiplier;
                double gap = loc.toDouble(mui->lineEdit_5->text(),&ok6)*myOpt->dwLengthMultiplier;
                double ng = loc.toDouble(mui->lineEdit_6->text(),&ok7)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok7)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((I == 0)||(D == 0)||(l == 0)||(d == 0)||(k == 0)||(gap == 0)||(ng == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (k < d){
                    showWarning(tr("Warning"), "k < d");
                    return;
                }
                data->inductance = I;
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_1->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                mui->lineEdit_2->setText(loc.toString(l / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_3->setText(roundTo(d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->lineEdit_4->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                mui->lineEdit_5->setText(loc.toString(gap / myOpt->dwLengthMultiplier));
                mui->lineEdit_6->setText(loc.toString(ng / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, D, d, k, l, gap, ng, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerNgap_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Multilayer_r:{
                if ((mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty())||(mui->lineEdit_5->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6;
                double I = loc.toDouble(mui->lineEdit_ind->text(),&ok1)*myOpt->dwInductanceMultiplier;
                double a = loc.toDouble(mui->lineEdit_1->text(),&ok2)*myOpt->dwLengthMultiplier;
                double b = loc.toDouble(mui->lineEdit_2->text(),&ok3)*myOpt->dwLengthMultiplier;
                double l = loc.toDouble(mui->lineEdit_3->text(),&ok4)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_4->text(),&ok5);
                } else {
                    d = loc.toDouble(mui->lineEdit_4->text(),&ok5)*myOpt->dwLengthMultiplier;
                }
                double k = loc.toDouble(mui->lineEdit_5->text(),&ok6)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((I == 0)||(a == 0)||(b == 0)||(l == 0)||(d == 0)||(k == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (k < d){
                    showWarning(tr("Warning"), "k < d");
                    return;
                }
                data->inductance = I;
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_1->setText(loc.toString(a / myOpt->dwLengthMultiplier));
                mui->lineEdit_2->setText(loc.toString(b / myOpt->dwLengthMultiplier));
                mui->lineEdit_3->setText(loc.toString(l / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_4->setText(roundTo(d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->lineEdit_5->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, a, b, l, d, k, 0, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerN_Rect_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Multilayer_f:{
                if ((mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5;
                double I = loc.toDouble(mui->lineEdit_ind->text(),&ok1)*myOpt->dwInductanceMultiplier;
                double D = loc.toDouble(mui->lineEdit_1->text(),&ok2)*myOpt->dwLengthMultiplier;
                double w = loc.toDouble(mui->lineEdit_2->text(),&ok3)*myOpt->dwLengthMultiplier;
                double t = loc.toDouble(mui->lineEdit_3->text(),&ok4)*myOpt->dwLengthMultiplier;
                double ins = loc.toDouble(mui->lineEdit_4->text(),&ok5)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((I == 0)||(D == 0)||(w == 0)||(t == 0)||(ins == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (w/t < 100){
                    showWarning(tr("Warning"), "w/t < 100");
                    return;
                }
                if (w/D > 1){
                    showWarning(tr("Warning"), "w > D");
                    return;
                }
                data->inductance = I;
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_1->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                mui->lineEdit_2->setText(loc.toString(w / myOpt->dwLengthMultiplier));
                mui->lineEdit_3->setText(loc.toString(t / myOpt->dwLengthMultiplier));
                mui->lineEdit_4->setText(loc.toString(ins / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, w, t, ins, I, 0, 0, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerN_Foil_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _FerrToroid:{
                if ((mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty())||(mui->lineEdit_5->text().isEmpty())||(mui->lineEdit_6->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6, ok7;
                double I = loc.toDouble(mui->lineEdit_ind->text(),&ok1)*myOpt->dwInductanceMultiplier;
                double OD = loc.toDouble(mui->lineEdit_1->text(),&ok2)*myOpt->dwLengthMultiplier;
                double ID = loc.toDouble(mui->lineEdit_2->text(),&ok3)*myOpt->dwLengthMultiplier;
                double h = loc.toDouble(mui->lineEdit_3->text(),&ok4)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_4->text(),&ok5);
                } else {
                    d = loc.toDouble(mui->lineEdit_4->text(),&ok5)*myOpt->dwLengthMultiplier;
                }
                double mu = loc.toDouble(mui->lineEdit_5->text(),&ok6);
                double Ch = loc.toDouble(mui->lineEdit_6->text(),&ok7)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok7)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((I == 0)||(OD == 0)||(ID == 0)||(h == 0)||(d == 0)||(mu == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (OD < ID){
                    showWarning(tr("Warning"), "OD < ID");
                    return;
                }
                data->inductance = I;
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_1->setText(loc.toString(OD / myOpt->dwLengthMultiplier));
                mui->lineEdit_2->setText(loc.toString(ID / myOpt->dwLengthMultiplier));
                mui->lineEdit_3->setText(loc.toString(h / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_4->setText(roundTo(d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->lineEdit_5->setText(loc.toString(mu));
                mui->lineEdit_6->setText(loc.toString(Ch / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, OD, ID, h, d, mu, Ch, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_ferrToroidN_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _PCB_coil:{
                if (mui->lineEdit_freq->text().isEmpty()||(mui->lineEdit_4->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                int layoutPCB = mui->comboBox_checkPCB->currentIndex();
                bool ok1, ok2, ok3, ok4, ok5;
                if ((layoutPCB == 2) && (mui->lineEdit_3->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                double a =0;
                if (layoutPCB == 2){
                    bool ok6;
                    a = loc.toDouble(mui->lineEdit_3->text(),&ok6)*myOpt->dwLengthMultiplier;
                    if (!ok6){
                        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                        return;
                    }
                }
                double I = loc.toDouble(mui->lineEdit_ind->text(),&ok1)*myOpt->dwInductanceMultiplier;
                double f = loc.toDouble(mui->lineEdit_freq->text(),&ok2)*myOpt->dwFrequencyMultiplier;
                double OD = loc.toDouble(mui->lineEdit_1->text(),&ok3)*myOpt->dwLengthMultiplier;
                double ID = loc.toDouble(mui->lineEdit_2->text(),&ok4)*myOpt->dwLengthMultiplier;
                double th = loc.toDouble(mui->lineEdit_4->text(),&ok5)*myOpt->dwLengthMultiplier;
                double ratio = (double)mui->horizontalSlider->value()/100;

                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((I == 0)||(f == 0)||(OD == 0)||(ID == 0)||(th == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (OD < ID){
                    if (layoutPCB == 2)
                        showWarning(tr("Warning"), "A < B");
                    else
                        showWarning(tr("Warning"), "D < d");
                    return;
                }
                if ((layoutPCB == 2) && (OD < a)){
                    showWarning(tr("Warning"), "A < a");
                    return;
                }
                data->inductance = I;
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_1->setText(loc.toString(OD / myOpt->dwLengthMultiplier));
                mui->lineEdit_2->setText(loc.toString(ID / myOpt->dwLengthMultiplier));
                mui->lineEdit_4->setText(loc.toString(th / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, OD, ID, ratio, layoutPCB, th, f, a );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_pcbN_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Flat_Spiral:{
                bool ok1, ok2, ok3, ok4;
                double I = loc.toDouble(mui->lineEdit_ind->text(), &ok1)*myOpt->dwInductanceMultiplier;
                double Di = loc.toDouble(mui->lineEdit_1->text(), &ok2)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_2->text(),&ok3);
                } else {
                    d = loc.toDouble(mui->lineEdit_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                }
                double s = loc.toDouble(mui->lineEdit_3->text(), &ok4)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((I == 0)||(Di == 0)||(d == 0)||(s == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                data->inductance = I;
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_1->setText(loc.toString(Di / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_2->setText(roundTo(d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->lineEdit_3->setText(loc.toString(s / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, Di, d, s, 0, 0, 0, 0 );
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
            if ((mui->lineEdit_2_2->text().isEmpty()) ||(mui->lineEdit_3_2->text().isEmpty())){
                showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                return;
            }
            switch (FormCoil) {
            case _Onelayer_cw:{
                if ((mui->lineEdit_1_2->text().isEmpty())||mui->lineEdit_N->text().isEmpty()||(mui->lineEdit_freq2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5;
                double N = loc.toDouble(mui->lineEdit_N->text(), &ok1);
                double f = loc.toDouble(mui->lineEdit_freq2->text(), &ok2)*myOpt->dwFrequencyMultiplier;
                double D = loc.toDouble(mui->lineEdit_1_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_2_2->text(),&ok4);
                } else {
                    d = loc.toDouble(mui->lineEdit_2_2->text(), &ok4)*myOpt->dwLengthMultiplier;
                }
                double k = loc.toDouble(mui->lineEdit_3_2->text(), &ok5)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((N == 0)||(D == 0)||(d == 0)||(k == 0)||(f == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (k < d){
                    showWarning(tr("Warning"), "k < d");
                    return;
                }
                data->frequency = f;
                double Dk = D + k;
                Material mt = Cu;
                checkMaterial2(&mt);
                mui->lineEdit_N->setText(loc.toString(N));
                mui->lineEdit_freq2->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
                mui->lineEdit_1_2->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_2_2->setText(roundTo(d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->lineEdit_3_2->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, d, k, N, f, 0, 0, myOpt->dwAccuracy, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerI_roundW_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Onelayer:{
                if ((mui->lineEdit_1_2->text().isEmpty())||mui->lineEdit_N->text().isEmpty()||(mui->lineEdit_freq2->text().isEmpty())||(mui->lineEdit_4_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6;
                double N = loc.toDouble(mui->lineEdit_N->text(), &ok1);
                double f = loc.toDouble(mui->lineEdit_freq2->text(), &ok2)*myOpt->dwFrequencyMultiplier;
                double D = loc.toDouble(mui->lineEdit_1_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_2_2->text(),&ok4);
                } else {
                    d = loc.toDouble(mui->lineEdit_2_2->text(), &ok4)*myOpt->dwLengthMultiplier;
                }
                double k = loc.toDouble(mui->lineEdit_3_2->text(), &ok5)*myOpt->dwLengthMultiplier;
                double p = loc.toDouble(mui->lineEdit_4_2->text(), &ok6)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
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
                checkMaterial2(&mt);
                mui->lineEdit_N->setText(loc.toString(N));
                mui->lineEdit_freq2->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
                mui->lineEdit_1_2->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_2_2->setText(roundTo(d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->lineEdit_3_2->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                mui->lineEdit_4_2->setText(loc.toString(p / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, d, p, N, f, 0, 0, myOpt->dwAccuracy, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerI_roundW_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Onelayer_p:{
                if (((mui->lineEdit_1_2->text().isEmpty())||mui->lineEdit_N->text().isEmpty())||(mui->lineEdit_freq2->text().isEmpty()) ||(mui->lineEdit_4_2->text().isEmpty())
                        ||(mui->lineEdit_5_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6, ok7;
                double N = loc.toDouble(mui->lineEdit_N->text(), &ok1);
                double f = loc.toDouble(mui->lineEdit_freq2->text(), &ok2)*myOpt->dwFrequencyMultiplier;
                double D = loc.toDouble(mui->lineEdit_1_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                double w = loc.toDouble(mui->lineEdit_2_2->text(), &ok4)*myOpt->dwLengthMultiplier;
                double t = loc.toDouble(mui->lineEdit_3_2->text(), &ok5)*myOpt->dwLengthMultiplier;
                double ins = loc.toDouble(mui->lineEdit_4_2->text(), &ok6)*myOpt->dwLengthMultiplier;
                double p = loc.toDouble(mui->lineEdit_5_2->text(), &ok7)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok7)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((N == 0)||(D == 0)||(w == 0)||(t == 0)||(p == 0)||(f == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (p < (w + 2*ins)){
                    showWarning(tr("Warning"), "p < (w + 2i)");
                    return;
                }
                if ((ins == 0)&&(p <= w)){
                    showWarning(tr("Warning"), "p <= w");
                    return;
                }
                data->frequency = f;
                double Dk = D + t + ins;
                Material mt = Cu;
                checkMaterial2(&mt);
                mui->lineEdit_N->setText(loc.toString(N));
                mui->lineEdit_freq->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
                mui->lineEdit_1_2->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                mui->lineEdit_2_2->setText(loc.toString(w / myOpt->dwLengthMultiplier));
                mui->lineEdit_3_2->setText(loc.toString(t / myOpt->dwLengthMultiplier));
                mui->lineEdit_4_2->setText(loc.toString(ins / myOpt->dwLengthMultiplier));
                mui->lineEdit_5_2->setText(loc.toString(p / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, w, t, p, N, f, 0, myOpt->dwAccuracy, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerI_rectW_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Onelayer_q:{
                if ((mui->lineEdit_1_2->text().isEmpty())||mui->lineEdit_N->text().isEmpty()||(mui->lineEdit_freq2->text().isEmpty())||(mui->lineEdit_4_2->text().isEmpty())
                        ||(mui->lineEdit_5_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6, ok7;
                double N = loc.toDouble(mui->lineEdit_N->text(), &ok1);
                double f = loc.toDouble(mui->lineEdit_freq2->text(), &ok2)*myOpt->dwFrequencyMultiplier;
                double D = loc.toDouble(mui->lineEdit_1_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_2_2->text(),&ok4);
                } else {
                    d = loc.toDouble(mui->lineEdit_2_2->text(), &ok4)*myOpt->dwLengthMultiplier;
                }
                double k = loc.toDouble(mui->lineEdit_3_2->text(), &ok5)*myOpt->dwLengthMultiplier;
                double p = loc.toDouble(mui->lineEdit_4_2->text(), &ok6)*myOpt->dwLengthMultiplier;
                int n = loc.toInt(mui->lineEdit_5_2->text(), &ok7);
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
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
                if ((n < 3)||(!ok7)){
                    showWarning(tr("Warning"), tr("The number of sides must be integer and at least three!"));
                    return;
                }
                data->frequency = f;
                double Dk = D + k;
                double _n = (double) n;
                Material mt = Cu;
                checkMaterial2(&mt);
                mui->lineEdit_N->setText(loc.toString(N));
                mui->lineEdit_freq2->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
                mui->lineEdit_1_2->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_2_2->setText(roundTo(d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->lineEdit_3_2->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                mui->lineEdit_4_2->setText(loc.toString(p / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, d, p, N, f, _n, 0, myOpt->dwAccuracy, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerI_Poligonal_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Multilayer:{
                if (((mui->lineEdit_1_2->text().isEmpty())||mui->lineEdit_4_2->text().isEmpty())||(mui->lineEdit_5_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5;
                double D = loc.toDouble(mui->lineEdit_1_2->text(), &ok1)*myOpt->dwLengthMultiplier;
                double l = loc.toDouble(mui->lineEdit_2_2->text(), &ok2)*myOpt->dwLengthMultiplier;
                double c = 0;
                double N = 0;
                double d = 0;
                double Rm = 0;
                if (mui->radioButton_6->isChecked()){
                    N = loc.toDouble(mui->lineEdit_3_2->text(), &ok3);
                } else {
                    c = loc.toDouble(mui->lineEdit_3_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                }
                if (!mui->radioButton_8->isChecked()){
                    if (myOpt->isAWG){
                        d = convertfromAWG(mui->lineEdit_4_2->text(),&ok4);
                    } else {
                        d = loc.toDouble(mui->lineEdit_4_2->text(), &ok4)*myOpt->dwLengthMultiplier;
                    }
                } else {
                    Rm = loc.toDouble(mui->lineEdit_4_2->text(), &ok4);
                }
                double k = loc.toDouble(mui->lineEdit_5_2->text(), &ok5)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((D == 0)||((N == 0) && (mui->radioButton_6->isChecked()))||((c == 0) && (!mui->radioButton_6->isChecked()))
                        ||(l == 0)||(k == 0)||((d == 0) && (mui->radioButton_7->isChecked()))||((Rm == 0) && (mui->radioButton_8->isChecked())) ){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (k < d){
                    showWarning(tr("Warning"), "k < d");
                    return;
                }
                mui->lineEdit_1_2->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                mui->lineEdit_2_2->setText(loc.toString(l / myOpt->dwLengthMultiplier));
                mui->lineEdit_3_2->setText(loc.toString(c / myOpt->dwLengthMultiplier));
                if ((mui->radioButton_6->isChecked())) {
                    mui->lineEdit_3_2->setText(loc.toString(N));
                } else {
                    if ((mui->radioButton_7->isChecked())&&(!myOpt->isAWG))
                        mui->lineEdit_4_2->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                    if (mui->radioButton_8->isChecked())
                        mui->lineEdit_4_2->setText(loc.toString(Rm));
                }
                mui->lineEdit_5_2->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                int windingKind = mui->comboBox_checkMLWinding->currentIndex();
                if (mui->radioButton_6->isChecked()){
                    MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, l, N, d, k, 0, 0, windingKind );
                    connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerI_Result(_CoilResult)));
                    thread->start();
                } else if (mui->radioButton_7->isChecked()){
                    MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, l, c, d, k, 0, 1, windingKind );
                    connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerI_Result(_CoilResult)));
                    thread->start();
                } else if (mui->radioButton_8->isChecked()){
                    MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, l, c, Rm, k, 0, 2, windingKind );
                    connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerI_Result(_CoilResult)));
                    thread->start();
                }
                break;
            }
            case _Multilayer_p:{
                if (((mui->lineEdit_1_2->text().isEmpty())||mui->lineEdit_4_2->text().isEmpty())||(mui->lineEdit_5_2->text().isEmpty())||(mui->lineEdit_6_2->text().isEmpty())
                        ||(mui->lineEdit_7_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6, ok7;
                double D = loc.toDouble(mui->lineEdit_1_2->text(), &ok1)*myOpt->dwLengthMultiplier;
                double l = loc.toDouble(mui->lineEdit_2_2->text(), &ok2)*myOpt->dwLengthMultiplier;
                double c = loc.toDouble(mui->lineEdit_3_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_4_2->text(),&ok4);
                } else {
                    d = loc.toDouble(mui->lineEdit_4_2->text(), &ok4)*myOpt->dwLengthMultiplier;
                }
                double k = loc.toDouble(mui->lineEdit_5_2->text(), &ok5)*myOpt->dwLengthMultiplier;
                double g = loc.toDouble(mui->lineEdit_6_2->text(), &ok6)*myOpt->dwLengthMultiplier;
                double Ng = loc.toDouble(mui->lineEdit_7_2->text(), &ok7);
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok7)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((D == 0)||(l == 0)||(c == 0)||(d == 0)||(k == 0)||(g == 0)||(Ng == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                mui->lineEdit_1_2->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                mui->lineEdit_2_2->setText(loc.toString(l / myOpt->dwLengthMultiplier));
                mui->lineEdit_3_2->setText(loc.toString(c / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_4_2->setText(roundTo(d / myOpt->dwLengthMultiplier, loc,myOpt->dwAccuracy));
                mui->lineEdit_5_2->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                mui->lineEdit_6_2->setText(loc.toString(g / myOpt->dwLengthMultiplier));
                mui->lineEdit_7_2->setText(loc.toString(Ng));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, l, c, d, k, g, Ng, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerIgap_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Multilayer_r:{
                if (((mui->lineEdit_1_2->text().isEmpty())||mui->lineEdit_4_2->text().isEmpty())||(mui->lineEdit_5_2->text().isEmpty())||(mui->lineEdit_6_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6;
                double a = loc.toDouble(mui->lineEdit_1_2->text(), &ok1)*myOpt->dwLengthMultiplier;
                double b = loc.toDouble(mui->lineEdit_2_2->text(), &ok2)*myOpt->dwLengthMultiplier;
                double l = loc.toDouble(mui->lineEdit_3_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                double c = loc.toDouble(mui->lineEdit_4_2->text(), &ok4)*myOpt->dwLengthMultiplier;
                double d = 0;

                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_5_2->text(),&ok5);
                } else
                    d = loc.toDouble(mui->lineEdit_5_2->text(), &ok5)*myOpt->dwLengthMultiplier;

                double k = loc.toDouble(mui->lineEdit_6_2->text(), &ok6)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((a == 0)||(b == 0)||(c == 0)||(l == 0)||((d == 0)||(k == 0))){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (k < d){
                    showWarning(tr("Warning"), "k < d");
                    return;
                }
                mui->lineEdit_1_2->setText(loc.toString(a / myOpt->dwLengthMultiplier));
                mui->lineEdit_2_2->setText(loc.toString(b / myOpt->dwLengthMultiplier));
                mui->lineEdit_3_2->setText(loc.toString(l / myOpt->dwLengthMultiplier));
                mui->lineEdit_4_2->setText(loc.toString(c / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_5_2->setText(roundTo(d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->lineEdit_6_2->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                if (mui->radioButton_6->isChecked()){
                    MThread_calculate *thread= new MThread_calculate( FormCoil, tab, a, b, l, c, d, k, 0, 0 );
                    connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerI_Rect_Result(_CoilResult)));
                    thread->start();
                } else if (mui->radioButton_7->isChecked()){
                    MThread_calculate *thread= new MThread_calculate( FormCoil, tab, a, b, l, c, d, k, 1, 0 );
                    connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerI_Rect_Result(_CoilResult)));
                    thread->start();
                }
                break;
            }
            case _Multilayer_f:{
                if (((mui->lineEdit_1_2->text().isEmpty())||mui->lineEdit_N->text().isEmpty()) ||(mui->lineEdit_4_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5;
                double N = loc.toDouble(mui->lineEdit_N->text(), &ok1);
                double D = loc.toDouble(mui->lineEdit_1_2->text(), &ok2)*myOpt->dwLengthMultiplier;
                double w = loc.toDouble(mui->lineEdit_2_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                double t = loc.toDouble(mui->lineEdit_3_2->text(), &ok4)*myOpt->dwLengthMultiplier;
                double ins = loc.toDouble(mui->lineEdit_4_2->text(), &ok5)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((N == 0)||(D == 0)||(w == 0)||(t == 0)||(ins == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (w/t < 100){
                    showWarning(tr("Warning"), "w/t < 100");
                    return;
                }
                if (w/D > 1){
                    showWarning(tr("Warning"), "w > D");
                    return;
                }
                mui->lineEdit_N->setText(loc.toString(N));
                mui->lineEdit_1_2->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                mui->lineEdit_2_2->setText(loc.toString(w / myOpt->dwLengthMultiplier));
                mui->lineEdit_3_2->setText(loc.toString(t / myOpt->dwLengthMultiplier));
                mui->lineEdit_4_2->setText(loc.toString(ins / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, w, t, ins, N, 0, 0, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerI_Foil_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _FerrToroid:{
                if ((mui->lineEdit_1_2->text().isEmpty())||mui->lineEdit_N->text().isEmpty()||(mui->lineEdit_4_2->text().isEmpty())||(mui->lineEdit_5_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6, ok7;
                double N = loc.toDouble(mui->lineEdit_N->text(), &ok1);
                double OD = loc.toDouble(mui->lineEdit_1_2->text(), &ok2)*myOpt->dwLengthMultiplier;
                double ID = loc.toDouble(mui->lineEdit_2_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                double h = loc.toDouble(mui->lineEdit_3_2->text(), &ok4)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_4_2->text(),&ok5);
                } else {
                    d = loc.toDouble(mui->lineEdit_4_2->text(),&ok5)*myOpt->dwLengthMultiplier;
                }
                double mu = loc.toDouble(mui->lineEdit_5_2->text(), &ok6);
                double Ch = loc.toDouble(mui->lineEdit_6_2->text(), &ok7)*myOpt->dwLengthMultiplier;
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok7)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((N == 0)||(OD == 0)||(ID == 0)||(h == 0)||(mu == 0)||(d == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (OD < ID){
                    showWarning(tr("Warning"), "OD < ID");
                    return;
                }
                mui->lineEdit_N->setText(loc.toString(N));
                mui->lineEdit_1_2->setText(loc.toString(OD / myOpt->dwLengthMultiplier));
                mui->lineEdit_2_2->setText(loc.toString(ID / myOpt->dwLengthMultiplier));
                mui->lineEdit_3_2->setText(loc.toString(h / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_4_2->setText(roundTo(d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                mui->lineEdit_5_2->setText(loc.toString(mu));
                mui->lineEdit_6_2->setText(loc.toString(Ch / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, N, OD, ID, h, mu, Ch, d, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_ferriteI_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _PCB_coil:{
                if (mui->lineEdit_N->text().isEmpty()||((mui->lineEdit_4_2->text().isEmpty())&&(myOpt->layoutPCBcoil == 0))){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6;
                double N = loc.toDouble(mui->lineEdit_N->text(), &ok1);
                double f = loc.toDouble(mui->lineEdit_freq2->text(), &ok2)*myOpt->dwFrequencyMultiplier;
                int layoutPCB = mui->comboBox_checkPCB->currentIndex();
                double A = 0;
                if (layoutPCB == 2)
                    A = loc.toDouble(mui->lineEdit_1_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                double ID = loc.toDouble(mui->lineEdit_2_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                if ((N == 0)||(ID == 0)||(f == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if((!ok1)||(!ok2)||(!ok3)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                double s = loc.toDouble(mui->lineEdit_3_2->text(), &ok4)*myOpt->dwLengthMultiplier;
                double W = loc.toDouble(mui->lineEdit_4_2->text(), &ok5)*myOpt->dwLengthMultiplier;
                double th = loc.toDouble(mui->lineEdit_5_2->text(), &ok6)*myOpt->dwLengthMultiplier;
                if((!ok4)||(!ok5)||(!ok6)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((s == 0)||(W == 0)||(th == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (s <= W){
                    showWarning(tr("Warning"), "s <= W");
                    return;
                }
                if ((layoutPCB == 2) && (A < ID)){
                    showWarning(tr("Warning"), "A < B");
                    return;
                }
                mui->lineEdit_N->setText(loc.toString(N));
                mui->lineEdit_2_2->setText(loc.toString(ID / myOpt->dwLengthMultiplier));
                mui->lineEdit_3_2->setText(loc.toString(s / myOpt->dwLengthMultiplier));
                mui->lineEdit_4_2->setText(loc.toString(W / myOpt->dwLengthMultiplier));
                mui->lineEdit_5_2->setText(loc.toString(th / myOpt->dwLengthMultiplier));

                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, N, A, ID, s, W, layoutPCB, th, f );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_pcbI_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Flat_Spiral:{
                if ((mui->lineEdit_1_2->text().isEmpty())||mui->lineEdit_N->text().isEmpty()){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4;
                double N = loc.toDouble(mui->lineEdit_N->text(), &ok1);
                double OD = loc.toDouble(mui->lineEdit_1_2->text(), &ok2)*myOpt->dwLengthMultiplier;
                double ID = loc.toDouble(mui->lineEdit_2_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_3_2->text(),&ok4);
                } else {
                    d = loc.toDouble(mui->lineEdit_3_2->text(), &ok4)*myOpt->dwLengthMultiplier;
                }
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((N == 0)||(OD == 0)||(ID == 0)||(d == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (OD < ID){
                    showWarning(tr("Warning"), "OD < ID");
                    return;
                }
                mui->lineEdit_N->setText(loc.toString(N));
                mui->lineEdit_1_2->setText(loc.toString(OD / myOpt->dwLengthMultiplier));
                mui->lineEdit_2_2->setText(loc.toString(ID / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_3_2->setText(roundTo(d / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, N, OD, ID, d, 0, 0, 0, 0 );
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
            QString tmpStr = "";
            QString label1 = "";
            QString label2 = "";
            bool ok1, ok2;
            if (mui->radioButton_LC->isChecked()){
                data->inductance = loc.toDouble(mui->lineEdit_1_3->text(), &ok1)*myOpt->dwInductanceMultiplier;
                data->capacitance = loc.toDouble(mui->lineEdit_2_3->text(), &ok2)*myOpt->dwCapacityMultiplier;
                if((!ok1)||(!ok2)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                data->frequency = CalcLC0(data->inductance, data->capacitance);
                tmpStr += formattedOutput(myOpt, tr("Frequency of a circuit") + " f = ", roundTo(data->frequency / myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy),
                                          qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8())) + "<br/>";
                label1 = " L: ";
                label2 = " C: ";
            } else if (mui->radioButton_CF->isChecked()){
                data->capacitance = loc.toDouble(mui->lineEdit_1_3->text(), &ok1)*myOpt->dwCapacityMultiplier;
                data->frequency = loc.toDouble(mui->lineEdit_2_3->text(), &ok2)*myOpt->dwFrequencyMultiplier;
                if((!ok1)||(!ok2)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                data->inductance = CalcLC1(data->capacitance, data->frequency);
                tmpStr += formattedOutput(myOpt, tr("Inductance of a circuit") + " L = ", roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy),
                                          qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
                label1 = " C: ";
                label2 = " f: ";
            } else if (mui->radioButton_LF->isChecked()){
                data->inductance = loc.toDouble(mui->lineEdit_1_3->text(), &ok1)*myOpt->dwInductanceMultiplier;
                data->frequency = loc.toDouble(mui->lineEdit_2_3->text(), &ok2)*myOpt->dwFrequencyMultiplier;
                if((!ok1)||(!ok2)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                data->capacitance = CalcLC2(data->inductance, data->frequency);
                tmpStr += formattedOutput(myOpt, tr("Circuit capacitance") + " C = ", roundTo(data->capacitance / myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy),
                                          qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
                label1 = " L: ";
                label2 = " f: ";
            } else if (mui->radioButton_ZF->isChecked()){
                data->zo = loc.toDouble(mui->lineEdit_1_3->text(), &ok1);
                data->frequency = loc.toDouble(mui->lineEdit_2_3->text(), &ok2)*myOpt->dwFrequencyMultiplier;
                if((!ok1)||(!ok2)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                _CoilResult result;
                CalcLC3(data->zo,data->frequency,&result);
                data->capacitance = toNearestE24(result.N, myOpt->dwAccuracy);
                data->inductance = CalcLC1(data->capacitance, data->frequency);
                tmpStr += formattedOutput(myOpt, tr("Circuit capacitance") + " C = ", roundTo(data->capacitance / myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy),
                                          qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
                tmpStr += formattedOutput(myOpt, tr("Inductance of a circuit") + " L = ", roundTo(data->inductance / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy),
                                          qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
                label1 = " Z: ";
                label2 = " f: ";
            }
            QString sCaption = windowTitle() + " - " + tr("LC tank calculation");
            QString sImage = "<img src=\":/images/res/LC.png\">";
            QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
            sInput += formattedOutput(myOpt, mui->groupBox_1_3->title() + label1, mui->lineEdit_1_3->text(), mui->label_01_3->text()) + "<br/>";
            sInput += formattedOutput(myOpt, mui->groupBox_2_3->title() + label2, mui->lineEdit_2_3->text(), mui->label_02_3->text()) + "</p>";
            QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
            sResult += tmpStr;
            if (!mui->radioButton_ZF->isChecked()){
                data->zo = 1000 * sqrt(data->inductance / data->capacitance);
                sResult += formattedOutput(myOpt, tr("Characteristic impedance") + " Z<sub>0</sub> = ", roundTo(data->zo, loc, myOpt->dwAccuracy), tr("Ohm"));
            }
            sResult += "</p>";
            showOutput(sCaption, sImage, sInput, sResult);
            mui->lineEdit_1_3->setFocus();
            mui->lineEdit_1_3->selectAll();
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
void MainWindow::get_onelayerN_roundW_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "";
    if (myOpt->isInsertImage){
        if (FormCoil == _Onelayer_cw)
            sImage = "<img src=\":/images/res/Coil1.png\">";
        else
            sImage = "<img src=\":/images/res/Coil2.png\">";
    }
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_ind->text(), mui->lineEdit_ind->text(), mui->label_ind_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_freq->text(), mui->lineEdit_freq->text(), mui->label_freq_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1->text(), mui->lineEdit_1->text(), mui->label_01->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2->text(), mui->lineEdit_2->text(), mui->label_02->text()) + "<br/>";
    if (!((myOpt->isWindingLengthOneLayerInit) && (FormCoil == _Onelayer_cw))){
        if (mui->lineEdit_3->isEnabled())
            sInput += formattedOutput(myOpt, mui->label_3->text(), mui->lineEdit_3->text(), mui->label_03->text()) + "<br/>";
        if (!(FormCoil == _Onelayer_cw))
            sInput += formattedOutput(myOpt, mui->label_4->text(), mui->lineEdit_4->text(), mui->label_04->text()) + "<br/>";
    }
    QString material_name = getCheckedRadioButton(mui->groupBox)->text();
    sInput += formattedOutput(myOpt, mui->groupBox->title().mid(0, mui->groupBox->title().length() - 1) + " Mt:", material_name) + "</p>";
    double I = loc.toDouble(mui->lineEdit_ind->text())*myOpt->dwInductanceMultiplier;
    double d = 0;
    double k = 0;
    double p = 0;
    if ((myOpt->isWindingLengthOneLayerInit) && (FormCoil == _Onelayer_cw)){
        p = loc.toDouble(mui->lineEdit_4->text())*myOpt->dwLengthMultiplier;
    } else {
        if (myOpt->isAWG){
            d = convertfromAWG(mui->lineEdit_2->text());
        } else {
            d = loc.toDouble(mui->lineEdit_2->text())*myOpt->dwLengthMultiplier;
        }
        k = loc.toDouble(mui->lineEdit_3->text())*myOpt->dwLengthMultiplier;
        if (FormCoil == _Onelayer_cw)
            p = k;
        else
            p = loc.toDouble(mui->lineEdit_4->text())*myOpt->dwLengthMultiplier;
    }
    double f = loc.toDouble(mui->lineEdit_freq->text())*myOpt->dwFrequencyMultiplier;
    QString sResult = "";
    if (result.N > 0){
        data->N = result.N;
        sResult += "<p><u>" + tr("Result") + ":</u><br/>";
        sResult += formattedOutput(myOpt, tr("Number of turns of the coil") + " N = ", roundTo(result.N, loc, myOpt->dwAccuracy)) + "<br/>";
        QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
        QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
        QString d_wire_length = list[0];
        QString _ssLengthMeasureUnit = list[1];
        sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                                  qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
        if ((myOpt->isWindingLengthOneLayerInit) && (FormCoil == _Onelayer_cw)){
            d = result.five;
            data->d = d;
            if (myOpt->isAWG)
                sResult += formattedOutput(myOpt, tr("Wire diameter") + " AWG# = ", converttoAWG(d))  + "<br/>";
            else
                sResult += formattedOutput(myOpt, tr("Wire diameter") + " dw = ", roundTo( (result.five)/myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy ),
                                          qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
        } else {
            data->l = result.N * p + k;
            sResult += formattedOutput(myOpt, tr("Length of winding") + " l = ", roundTo( (data->l)/myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy ),
                                      qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
        }
        Material mt = Cu;
        checkMaterial1(&mt);
        double dencity = mtrl[mt][Dencity];
        double mass = 0.25 * dencity * M_PI * d * d * result.sec;
        double Resistivity = mtrl[mt][Rho]*1e6;
        double R = (Resistivity * result.sec * 4) / (M_PI * d * d); // DC resistance of the wire
        sResult += formattedOutput(myOpt, tr("Weight of wire") + " m = ", roundTo(mass, loc, myOpt->dwAccuracy), tr("g")) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(R, loc, myOpt->dwAccuracy), tr("Ohm")) + "<br/>";
        double reactance = 2 * M_PI * I * f;
        sResult += formattedOutput(myOpt, tr("Reactance of the coil") + " X = ", roundTo(reactance, loc, myOpt->dwAccuracy), tr("Ohm"));
        sResult += "<br/><br/>";
        if (f < 0.7 * result.fourth){
            sResult += formattedOutput(myOpt, tr("Self capacitance") + " Cs = ", roundTo(result.thd/myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy),
                                      qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
            sResult += formattedOutput(myOpt, tr("Coil self-resonance frequency") + " Fsr = ", roundTo(result.fourth/myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy),
                                      qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8())) + "<br/>";
            double Ql = (double)result.six;
            sResult += formattedOutput(myOpt, tr("Coil constructive Q-factor") + " Q = ", QString::number(result.six)) + "<br/>";
            double ESR = result.seven;
            sResult += formattedOutput(myOpt, tr("Loss resistance") + " ESR = ", roundTo(ESR, loc, myOpt->dwAccuracy), tr("Ohm")) + "</p>";
            if(myOpt->isAdditionalResult){
                sResult += "<hr/><p>";
                sResult += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
                data->capacitance = CalcLC2(I, f);
                sResult += " => "  + tr("Circuit capacitance") + ": Ck = " + roundTo((data->capacitance - result.thd) / myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy) + " "
                        + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
                double ro = 1000 * sqrt(I / data->capacitance);
                sResult += " => " + tr("Characteristic impedance") + ": ρ = " + loc.toString(round(ro)) + " " + tr("Ohm") + "<br/>";
                double Qs= 1 / (0.001 + 1 / Ql);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
                double Re = ro * Qs;
                sResult += " => "  + tr("Equivalent resistance") + ": Re = " + roundTo(Re / 1000, loc, myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
                double deltaf = 1000 * data->frequency / Qs;
                sResult += " => " + tr("Bandwidth") + ": 3dBΔf = " + roundTo(deltaf, loc, myOpt->dwAccuracy)+ " " + tr("kHz");
            }
            if(myOpt->isShowLTSpice){
                sResult += "<hr/><p>";
                sResult += "<u>" + tr("Input data for LTSpice") + ":</u><br/>";
                sResult += "Inductance: " + QString::number(data->inductance * ((1 + Ql * Ql)/(Ql * Ql)), 'f', myOpt->dwAccuracy) + "μ" + "<br/>";
                sResult += "Series resistance: " + QString::number(R * 1000, 'f', myOpt->dwAccuracy) + "m" + "<br/>";
                sResult += "Parallel resistance: " + QString::number((ESR * (1 + Ql * Ql)) / 1000, 'f', myOpt->dwAccuracy) + "k" + "<br/>";
                sResult += "Parallel capacitance: " + QString::number(result.thd, 'f', myOpt->dwAccuracy) + "p" + "</p>";
            }
        } else {
            QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
            mui->statusBar->showMessage(message);
            sResult += "<span style=\"color:blue;\">" + message + "</span>";
        }
        sResult += "</p>";
    } else {
        sResult += "<span style=\"color:red;\">" + tr("Coil can not be realized") + "! </span>";
    }
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerN_rectW_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/Coil2_square.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_ind->text(), mui->lineEdit_ind->text(), mui->label_ind_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_freq->text(), mui->lineEdit_freq->text(), mui->label_freq_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1->text(), mui->lineEdit_1->text(), mui->label_01->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2->text(), mui->lineEdit_2->text(), mui->label_02->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3->text(), mui->lineEdit_3->text(), mui->label_03->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4->text(), mui->lineEdit_4->text(), mui->label_04->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_5->text(), mui->lineEdit_5->text(), mui->label_05->text()) + "<br/>";
    QString material_name = getCheckedRadioButton(mui->groupBox)->text();
    sInput += formattedOutput(myOpt, mui->groupBox->title().mid(0, mui->groupBox->title().length() - 1) + " Mt:", material_name) + "</p>";
    double I = loc.toDouble(mui->lineEdit_ind->text())*myOpt->dwInductanceMultiplier;
    double w = loc.toDouble(mui->lineEdit_2->text())*myOpt->dwLengthMultiplier;
    double t = loc.toDouble(mui->lineEdit_3->text())*myOpt->dwLengthMultiplier;
    double p = loc.toDouble(mui->lineEdit_5->text())*myOpt->dwLengthMultiplier;
    double f = loc.toDouble(mui->lineEdit_freq->text())*myOpt->dwFrequencyMultiplier;
    data->N = result.N;
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    sResult += formattedOutput(myOpt, tr("Number of turns of the coil") + " N = ", roundTo(result.N, loc, myOpt->dwAccuracy)) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                              qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Length of winding") + " l = ", roundTo( (result.N * p + w)/myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy ),
                              qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    Material mt = Cu;
    checkMaterial1(&mt);
    double dencity = mtrl[mt][Dencity];
    double mass = dencity * w * t * result.sec;
    double Resistivity = mtrl[mt][Rho]*1e6;
    double R = (Resistivity * result.sec) / (w * t); // DC resistance of the wire
    sResult += formattedOutput(myOpt, tr("Weight of wire") + " m = ", roundTo(mass, loc, myOpt->dwAccuracy), tr("g")) + "<br/>";
    double reactance = 2 * M_PI * I * f;
    sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(R, loc, myOpt->dwAccuracy), tr("Ohm")) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Reactance of the coil") + " X = ", roundTo(reactance, loc, myOpt->dwAccuracy), tr("Ohm"));
    sResult += "<br/><br/>";
    if (f < 0.7 * result.fourth){
        sResult += formattedOutput(myOpt, tr("Self capacitance") + " Cs = ", roundTo(result.thd/myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Coil self-resonance frequency") + " Fsr = ", roundTo(result.fourth/myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8())) + "<br/>";
        double Ql = (double)result.six;
        sResult += formattedOutput(myOpt, tr("Coil constructive Q-factor") + " Q = ", QString::number(result.six)) + "<br/>";
        double ESR = result.seven;
        sResult += formattedOutput(myOpt, tr("Loss resistance") + " ESR = ", roundTo(ESR, loc, myOpt->dwAccuracy), tr("Ohm")) + "</p>";
        if(myOpt->isAdditionalResult){
            sResult += "<hr/><p>";
            sResult += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcLC2(I, f);
            sResult += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    roundTo((data->capacitance - result.thd) / myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            sResult += " => " + tr("Characteristic impedance") + ": ρ = " + loc.toString(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            sResult += " => "  + tr("Equivalent resistance") + ": Re = " + roundTo(Re / 1000, loc, myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * data->frequency / Qs;
            sResult += " => " + tr("Bandwidth") + ": 3dBΔf = " + roundTo(deltaf, loc, myOpt->dwAccuracy) + tr("kHz");
        }
        if(myOpt->isShowLTSpice){
            sResult += "<hr/><p>";
            sResult += "<u>" + tr("Input data for LTSpice") + ":</u><br/>";
            sResult += "Inductance: " + QString::number(data->inductance * ((1 + Ql * Ql)/(Ql * Ql)), 'f', myOpt->dwAccuracy) + "μ" + "<br/>";
            sResult += "Series resistance: " + QString::number(R * 1000, 'f', myOpt->dwAccuracy) + "m" + "<br/>";
            sResult += "Parallel resistance: " + QString::number((ESR * (1 + Ql * Ql)) / 1000, 'f', myOpt->dwAccuracy) + "k" + "<br/>";
            sResult += "Parallel capacitance: " + QString::number(result.thd, 'f', myOpt->dwAccuracy) + "p" + "</p>";
        }
    } else {
        QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
        mui->statusBar->showMessage(message);
        sResult += "<span style=\"color:blue;\">" + message + "</span>";
    }
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerN_Poligonal_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/Coil3.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_ind->text(), mui->lineEdit_ind->text(), mui->label_ind_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_freq->text(), mui->lineEdit_freq->text(), mui->label_freq_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1->text(), mui->lineEdit_1->text(), mui->label_01->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2->text(), mui->lineEdit_2->text(), mui->label_02->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3->text(), mui->lineEdit_3->text(), mui->label_03->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4->text(), mui->lineEdit_4->text(), mui->label_04->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_5->text(), mui->lineEdit_5->text()) + "<br/>";
    QString material_name = getCheckedRadioButton(mui->groupBox)->text();
    sInput += formattedOutput(myOpt, mui->groupBox->title().mid(0, mui->groupBox->title().length() - 1) + " Mt:", material_name) + "</p>";
    double I = loc.toDouble(mui->lineEdit_ind->text())*myOpt->dwInductanceMultiplier;
    double d = 0;
    if (myOpt->isAWG){
        d = convertfromAWG(mui->lineEdit_2->text());
    } else {
        d = loc.toDouble(mui->lineEdit_2->text())*myOpt->dwLengthMultiplier;
    }
    double k = loc.toDouble(mui->lineEdit_3->text())*myOpt->dwLengthMultiplier;
    double p = loc.toDouble(mui->lineEdit_4->text())*myOpt->dwLengthMultiplier;
    double f = loc.toDouble(mui->lineEdit_freq->text())*myOpt->dwFrequencyMultiplier;
    data->N = result.N;
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    sResult += formattedOutput(myOpt, tr("Number of turns of the coil") + " N = ", roundTo(result.N, loc, myOpt->dwAccuracy)) + "<br/>";
    QString _wire_length = formatLength(result.thd, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                              qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Length of winding") + " l = ", roundTo( (result.N * p + k)/myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy ),
                              qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    Material mt = Cu;
    checkMaterial1(&mt);
    double dencity = mtrl[mt][Dencity];
    double mass = 0.25 * dencity * M_PI * d * d * result.thd;
    double Resistivity = mtrl[mt][Rho]*1e6;
    double R = (Resistivity * result.thd * 4) / (M_PI * d * d); // DC resistance of the wire
    sResult += formattedOutput(myOpt, tr("Weight of wire") + " m = ", roundTo(mass, loc, myOpt->dwAccuracy), tr("g")) + "<br/>";
    double reactance = 2 * M_PI * I * f;
    sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(R, loc, myOpt->dwAccuracy), tr("Ohm")) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Reactance of the coil") + " X = ", roundTo(reactance, loc, myOpt->dwAccuracy), tr("Ohm"));
    sResult += "<br/><br/>";
    if (f < 0.7 * result.five){
        sResult += formattedOutput(myOpt, tr("Self capacitance") + " Cs = ", roundTo(result.fourth/myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Coil self-resonance frequency") + " Fsr = " + roundTo(result.five/myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8())) + "<br/>";
        double Ql = (double)result.six;
        sResult += formattedOutput(myOpt, tr("Coil constructive Q-factor") + " Q = ", QString::number(result.six)) + "<br/>";
        double ESR = result.seven;
        sResult += formattedOutput(myOpt, tr("Loss resistance") + " ESR = ", roundTo(ESR, loc, myOpt->dwAccuracy), tr("Ohm")) + "</p>";
        if(myOpt->isAdditionalResult){
            sResult += "<hr/><p>";
            sResult += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcLC2(I, f);
            sResult += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    roundTo((data->capacitance - result.fourth) / myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            sResult += " => " + tr("Characteristic impedance") + ": ρ = " + loc.toString(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            sResult += " => "  + tr("Equivalent resistance") + ": Re = " + roundTo(Re / 1000, loc, myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * data->frequency / Qs;
            sResult += " => " + tr("Bandwidth") + ": 3dBΔf = " + roundTo(deltaf, loc, myOpt->dwAccuracy) + tr("kHz");
        }
        if(myOpt->isShowLTSpice){
            sResult += "<hr/><p>";
            sResult += "<u>" + tr("Input data for LTSpice") + ":</u><br/>";
            sResult += "Inductance: " + QString::number(data->inductance * ((1 + Ql * Ql)/(Ql * Ql)), 'f', myOpt->dwAccuracy) + "μ" + "<br/>";
            sResult += "Series resistance: " + QString::number(R * 1000, 'f', myOpt->dwAccuracy) + "m" + "<br/>";
            sResult += "Parallel resistance: " + QString::number((ESR * (1 + Ql * Ql)) / 1000, 'f', myOpt->dwAccuracy) + "k" + "<br/>";
            sResult += "Parallel capacitance: " + QString::number(result.fourth, 'f', myOpt->dwAccuracy) + "p" + "</p>";
        }
    } else {
        QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
        mui->statusBar->showMessage(message);
        sResult += "<span style=\"color:blue;\">" + message + "</span>";
    }
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerN_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "";
    if (myOpt->windingKind == 0)
        sImage = "<img src=\":/images/res/Coil4.png\">";
    else
        sImage = "<img src=\":/images/res/Coil4o.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_ind->text(), mui->lineEdit_ind->text(), mui->label_ind_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1->text(), mui->lineEdit_1->text(), mui->label_01->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2->text(), mui->lineEdit_2->text(), mui->label_02->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3->text(), mui->lineEdit_3->text(), mui->label_03->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4->text(), mui->lineEdit_4->text(), mui->label_04->text()) + "<br/>";
    sInput += mui->comboBox_checkMLWinding->currentText() + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    data->N = result.six;
    sResult += formattedOutput(myOpt, tr("Number of turns of the coil") + " N = ", QString::number(result.six)) + "<br/>";
    data->c = result.fourth;
    sResult += formattedOutput(myOpt, tr("Thickness of the coil") + " c = ", roundTo(result.fourth / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                              qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    double D = loc.toDouble(mui->lineEdit_1->text())*myOpt->dwLengthMultiplier;
    double width = (2 * result.fourth + D) / myOpt->dwLengthMultiplier;
    sResult += formattedOutput(myOpt, tr("Dimensions of inductor") + " S: ", mui->lineEdit_2->text() + "x" + loc.toString(ceil(width / myOpt->dwLengthMultiplier))
                              + "x" + loc.toString(ceil(width / myOpt->dwLengthMultiplier)), qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                              qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
    double d = 0;
    if (myOpt->isAWG){
        d = convertfromAWG(mui->lineEdit_3->text());
    } else {
        d = loc.toDouble(mui->lineEdit_3->text())*myOpt->dwLengthMultiplier;
    }
    double dencity = mtrl[Cu][Dencity];
    double mass = 0.25 * dencity * M_PI * d * d * result.sec;
    double Resistivity = mtrl[Cu][Rho]*1e6;
    double Rdc = (Resistivity * result.sec * 4) / (M_PI * d * d); // DC resistance of the wire
    data->Rdc = Rdc;
    sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(Rdc, loc, myOpt->dwAccuracy), tr("Ohm")) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Weight of wire") + " m = ", roundTo(mass, loc, myOpt->dwAccuracy), tr("g")) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Number of layers") + " Nl = ", loc.toString(result.thd));
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerNgap_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/Coil4-0.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_ind->text(), mui->lineEdit_ind->text(), mui->label_ind_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1->text(), mui->lineEdit_1->text(), mui->label_01->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2->text(), mui->lineEdit_2->text(), mui->label_02->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3->text(), mui->lineEdit_3->text(), mui->label_03->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4->text(), mui->lineEdit_4->text(), mui->label_04->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_5->text(), mui->lineEdit_5->text(), mui->label_05->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_6->text(), mui->lineEdit_6->text(), mui->label_06->text()) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    data->N = result.six;
    sResult += formattedOutput(myOpt, tr("Number of turns of the coil") + " N = ", QString::number(result.six)) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Thickness of the coil") + " c = ", roundTo(result.fourth / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                              qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    double D = loc.toDouble(mui->lineEdit_1->text())*myOpt->dwLengthMultiplier;
    double width = (2 * result.fourth + D) / myOpt->dwLengthMultiplier;
    sResult += formattedOutput(myOpt, tr("Dimensions of inductor") + " S: ", mui->lineEdit_2->text() + "x" + loc.toString(ceil(width / myOpt->dwLengthMultiplier))
                              + "x" + loc.toString(ceil(width / myOpt->dwLengthMultiplier)), qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                              qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
    double d = 0;
    if (myOpt->isAWG){
        d = convertfromAWG(mui->lineEdit_3->text());
    } else {
        d = loc.toDouble(mui->lineEdit_3->text())*myOpt->dwLengthMultiplier;
    }
    double dencity = mtrl[Cu][Dencity];
    double mass = 0.25 * dencity * M_PI * d * d * result.sec;
    double Resistivity = mtrl[Cu][Rho]*1e6;
    double Rdc = (Resistivity * result.sec * 4) / (M_PI * d * d); // DC resistance of the wire
    data->Rdc = Rdc;
    sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(Rdc, loc, myOpt->dwAccuracy), tr("Ohm")) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Weight of wire") + " m = ", roundTo(mass, loc, myOpt->dwAccuracy), tr("g")) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Number of layers") + " Nl = ", loc.toString(result.thd)) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Number of interlayers") + " Np = ", loc.toString(result.five));
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerN_Rect_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/Coil4_square.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_ind->text(), mui->lineEdit_ind->text(), mui->label_ind_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1->text(), mui->lineEdit_1->text(), mui->label_01->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2->text(), mui->lineEdit_2->text(), mui->label_02->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3->text(), mui->lineEdit_3->text(), mui->label_03->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4->text(), mui->lineEdit_4->text(), mui->label_04->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_5->text(), mui->lineEdit_5->text()) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    data->N = result.N;
    data->c = result.five;
    sResult += formattedOutput(myOpt, tr("Number of turns of the coil") + " N = ", loc.toString(result.N)) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Number of layers") + " Nl = ", loc.toString(result.sec)) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Thickness of the coil") + " c = ", roundTo(result.five / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                              qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    double a = loc.toDouble(mui->lineEdit_1->text())*myOpt->dwLengthMultiplier;
    double width = (2 * result.five + a) / myOpt->dwLengthMultiplier;
    double b = loc.toDouble(mui->lineEdit_2->text())*myOpt->dwLengthMultiplier;
    double height = (2 * result.five + b) / myOpt->dwLengthMultiplier;
    sResult += formattedOutput(myOpt, tr("Dimensions of inductor") + " S: ", mui->lineEdit_3->text() + "x" + loc.toString(ceil(width))
                              + "x" + loc.toString(ceil(height)) + " ", qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    QString _wire_length = formatLength(result.thd, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                              qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
    double d = 0;
    if (myOpt->isAWG){
        d = convertfromAWG(mui->lineEdit_4->text());
    } else {
        d = loc.toDouble(mui->lineEdit_4->text())*myOpt->dwLengthMultiplier;
    }
    double dencity = mtrl[Cu][Dencity];
    double mass = 0.25 * dencity * M_PI * d * d * result.thd;
    double Resistivity = mtrl[Cu][Rho]*1e6;
    double Rdc = (Resistivity * result.thd * 4) / (M_PI * d * d); // DC resistance of the wire
    data->Rdc = Rdc;
    sResult += formattedOutput(myOpt, tr("Weight of wire") + " m = ", roundTo(mass, loc, myOpt->dwAccuracy), tr("g")) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(result.fourth, loc, myOpt->dwAccuracy), tr("Ohm"));
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerN_Foil_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/Coil11.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_ind->text(), mui->lineEdit_ind->text(), mui->label_ind_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1->text(), mui->lineEdit_1->text(), mui->label_01->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2->text(), mui->lineEdit_2->text(), mui->label_02->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3->text(), mui->lineEdit_3->text(), mui->label_03->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4->text(), mui->lineEdit_4->text(), mui->label_04->text()) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    data->N = result.N;
    sResult += formattedOutput(myOpt, tr("Number of turns of the coil") + " N = ", loc.toString(result.N)) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Outside diameter") + " Do = ", roundTo(result.thd / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                              qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    QString _foil_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _foil_length.split(QRegExp(" "), skip_empty_parts);
    QString d_foil_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    sResult += formattedOutput(myOpt, tr("Length of the foil") + " lf = ", roundTo(d_foil_length.toDouble(), loc, myOpt->dwAccuracy),
                              qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(result.fourth, loc, myOpt->dwAccuracy), tr("Ohm")) + " (" + tr("Copper") + ")<br/>";
    sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(result.five, loc, myOpt->dwAccuracy), tr("Ohm")) + " (" + tr("Aluminum") + ")";
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_ferrToroidN_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/T-core.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_ind->text(), mui->lineEdit_ind->text(), mui->label_ind_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1->text(), mui->lineEdit_1->text(), mui->label_01->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2->text(), mui->lineEdit_2->text(), mui->label_02->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3->text(), mui->lineEdit_3->text(), mui->label_03->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4->text(), mui->lineEdit_4->text(), mui->label_04->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_5->text(), mui->lineEdit_5->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_6->text(), mui->lineEdit_6->text(), mui->label_06->text()) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    if (result.sec > 0) {
        data->N = result.N;
        sResult += formattedOutput(myOpt, tr("Number of turns of the coil") + " N = ", roundTo(result.N, loc, myOpt->dwAccuracy)) + "<br/>";
        QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
        QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
        QString d_wire_length = list[0];
        QString _ssLengthMeasureUnit = list[1];
        sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                                  qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(myOpt, "A<sub>L</sub> = ", loc.toString(result.thd),  qApp->translate("Context","nH") + "/N<sup>2</sup>");
        sResult += "<br/><br/>" + formattedOutput(myOpt, tr("Effective magnetic path length") + " (l<sub>e</sub>): ",
                                                  roundTo(result.fourth/myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                                                  qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Effective area of magnetic path") + " (A<sub>e</sub>): ",
                                   roundTo(result.five/(myOpt->dwLengthMultiplier * myOpt->dwLengthMultiplier), loc, myOpt->dwAccuracy),
                                   qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<sup>2</sup><br/>";
        sResult += formattedOutput(myOpt, tr("Effective volume") + " (V<sub>e</sub>): ",
                                   roundTo(result.fourth * result.five/(myOpt->dwLengthMultiplier * myOpt->dwLengthMultiplier * myOpt->dwLengthMultiplier), loc, myOpt->dwAccuracy),
                                   qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<sup>3</sup><br/>";
    } else {
        sResult += "<span style=\"color:red;\">" + tr("Coil can not be realized") + "! </span>";
        mui->statusBar->showMessage(tr("Coil can not be realized") + "!");
    }
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    if (satCurrentDockWidget != nullptr){
        if(satCurrentDockWidget->isVisible()){
            _FerriteData ferriteData;
            ferriteData.N = result.N;
            ferriteData.mu = loc.toDouble(mui->lineEdit_5->text());
            ferriteData.le = result.fourth;
            emit sendFerriteData(ferriteData);
        }
    }
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_pcbN_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    sCaption += mui->comboBox_checkPCB->currentText() + "<br/>";
    QString sImage = "";
    switch (myOpt->layoutPCBcoil) {
    case 0:{
        sImage += "<img src=\":/images/res/Coil8.png\">";
        break;
    }
    case 1:{
        sImage += "<img src=\":/images/res/Coil9.png\">";
        break;
    }
    case 2:{
        sImage += "<img src=\":/images/res/Coil8r.png\">";
        break;
    }
    default:
        break;
    }
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_ind->text(), mui->lineEdit_ind->text(), mui->label_ind_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_freq->text(), mui->lineEdit_freq->text(), mui->label_freq_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1->text(), mui->lineEdit_1->text(), mui->label_01->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2->text(), mui->lineEdit_2->text(), mui->label_02->text()) + "<br/>";
    if (mui->comboBox_checkPCB->currentIndex() == 2)
        sInput += formattedOutput(myOpt, mui->label_3->text(), mui->lineEdit_3->text(), mui->label_03->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4->text(), mui->lineEdit_4->text(), mui->label_04->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->groupBox_6->title(), loc.toString((double)mui->horizontalSlider->value()/100)) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    if ((result.N > 0) && (result.sec > 0) && (result.thd > 0)){
        data->N = result.N;
        data->s = QString::number(result.sec, 'f', myOpt->dwAccuracy).toDouble();
        data->w = QString::number(result.thd, 'f', myOpt->dwAccuracy).toDouble();
        sResult += formattedOutput(myOpt, tr("Number of turns of the coil") + " N = ", roundTo(result.N, loc, myOpt->dwAccuracy)) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Winding pitch") + " s = ", roundTo(result.sec / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Width of a PCB trace") + " W = ", roundTo(result.thd / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Coil constructive Q-factor") + " Q ≈ ", QString::number(round(result.fourth)));
    } else {
        sResult += tr("Invalid input parameter combination");
    }
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_spiralN_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/Coil10.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_ind->text(), mui->lineEdit_ind->text(), mui->label_ind_m->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1->text(), mui->lineEdit_1->text(), mui->label_01->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2->text(), mui->lineEdit_2->text(), mui->label_02->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3->text(), mui->lineEdit_3->text(), mui->label_03->text()) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    data->N = result.N;
    data->Do = result.thd;
    sResult += formattedOutput(myOpt, tr("Number of turns of the coil") + " N = ", loc.toString(result.N)) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Outside diameter") + " Do = ", roundTo(result.thd / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                              qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                              qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "</p>";
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GET INDUCTANCE ROUTINES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerI_roundW_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "";
    if (FormCoil == _Onelayer_cw)
        sImage = "<img src=\":/images/res/Coil1.png\">";
    else
        sImage = "<img src=\":/images/res/Coil2.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_N->text(), mui->lineEdit_N->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_freq2->text(), mui->lineEdit_freq2->text(), mui->label_freq_m2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1_2->text(), mui->lineEdit_1_2->text(), mui->label_01_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2_2->text(), mui->lineEdit_2_2->text(), mui->label_02_2->text()) + "<br/>";
    if (FormCoil == _Onelayer_cw)
        sInput += formattedOutput(myOpt, mui->label_3_2->text(), mui->lineEdit_3_2->text(), mui->label_03_2->text()) + "<br/>";
    else {
        if (mui->lineEdit_3_2->isEnabled())
            sInput += formattedOutput(myOpt, mui->label_3_2->text(), mui->lineEdit_3_2->text(), mui->label_03_2->text()) + "<br/>";
        sInput += formattedOutput(myOpt, mui->label_4_2->text(), mui->lineEdit_4_2->text(), mui->label_04_2->text()) + "<br/>";
    }
    QString material_name = getCheckedRadioButton(mui->groupBox_2)->text();
    sInput += formattedOutput(myOpt, mui->groupBox_2->title().mid(0, mui->groupBox_2->title().length() - 1) + " Mt:", material_name) + "</p>";
    double N = loc.toDouble(mui->lineEdit_N->text());
    double d = 0;
    if (myOpt->isAWG){
        d = convertfromAWG(mui->lineEdit_2_2->text());
    } else {
        d = loc.toDouble(mui->lineEdit_2_2->text())*myOpt->dwLengthMultiplier;
    }
    double k = loc.toDouble(mui->lineEdit_3_2->text())*myOpt->dwLengthMultiplier;
    double p = 0;
    if (FormCoil == _Onelayer_cw)
        p = k;
    else
        p = loc.toDouble(mui->lineEdit_4_2->text())*myOpt->dwLengthMultiplier;
    double f = loc.toDouble(mui->lineEdit_freq2->text())*myOpt->dwFrequencyMultiplier;
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    double I = result.N;
    data->inductance = result.N;
    sResult += formattedOutput(myOpt, tr("Inductance") + " L = ", roundTo(I/myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy),
                              qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                              qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Length of winding") + " l = ", roundTo( (N * p + k)/myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy ),
                              qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    Material mt = Cu;
    checkMaterial2(&mt);
    double dencity = mtrl[mt][Dencity];
    double mass = 0.25 * dencity * M_PI * d * d * result.sec;
    double Resistivity = mtrl[mt][Rho]*1e6;
    double R = (Resistivity * result.sec * 4) / (M_PI * d * d); // DC resistance of the wire
    sResult += formattedOutput(myOpt, tr("Weight of wire") + " m = ", roundTo(mass, loc, myOpt->dwAccuracy), tr("g")) + "<br/>";
    double reactance = 2 * M_PI * I * f;
    sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(R, loc, myOpt->dwAccuracy), tr("Ohm")) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Reactance of the coil") + " X = ", roundTo(reactance, loc, myOpt->dwAccuracy), tr("Ohm"));
    sResult += "<br/><br/>";
    if (f < 0.7 * result.fourth){
        sResult += formattedOutput(myOpt, tr("Self capacitance") + " Cs = ", roundTo(result.thd/myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Coil self-resonance frequency") + " Fsr = ", roundTo(result.fourth/myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8())) + "<br/>";
        double Ql = (double)result.six;
        sResult += formattedOutput(myOpt, tr("Coil constructive Q-factor") + " Q = ", QString::number(result.six)) + "<br/>";
        double ESR = result.seven;
        sResult += formattedOutput(myOpt, tr("Loss resistance") + " ESR = ", roundTo(ESR, loc, myOpt->dwAccuracy), tr("Ohm")) + "</p>";
        if(myOpt->isAdditionalResult){
            sResult += "<hr/><p>";
            sResult += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcLC2(I, f);
            sResult += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    roundTo((data->capacitance - result.thd) / myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            sResult += " => " + tr("Characteristic impedance") + ": ρ = " + loc.toString(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            sResult += " => "  + tr("Equivalent resistance") + ": Re = " + roundTo(Re / 1000, loc, myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * f / Qs;
            sResult += " => " + tr("Bandwidth") + ": 3dBΔf = " + roundTo(deltaf, loc, myOpt->dwAccuracy) + tr("kHz");
        }
        if(myOpt->isShowLTSpice){
            sResult += "<hr/><p>";
            sResult += "<u>" + tr("Input data for LTSpice") + ":</u><br/>";
            sResult += "Inductance: " + QString::number(data->inductance * ((1 + Ql * Ql)/(Ql * Ql)), 'f', myOpt->dwAccuracy) + "μ" + "<br/>";
            sResult += "Series resistance: " + QString::number(R * 1000, 'f', myOpt->dwAccuracy) + "m" + "<br/>";
            sResult += "Parallel resistance: " + QString::number((ESR * (1 + Ql * Ql)) / 1000, 'f', myOpt->dwAccuracy) + "k" + "<br/>";
            sResult += "Parallel capacitance: " + QString::number(result.thd, 'f', myOpt->dwAccuracy) + "p" + "</p>";
        }
    } else {
        QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
        mui->statusBar->showMessage(message);
        sResult += "<span style=\"color:blue;\">" + message + "</span>";
    }
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_N->setFocus();
    mui->lineEdit_N->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerI_rectW_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/Coil2_square.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_N->text(), mui->lineEdit_N->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_freq2->text(), mui->lineEdit_freq2->text(), mui->label_freq_m2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1_2->text(), mui->lineEdit_1_2->text(), mui->label_01_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2_2->text(), mui->lineEdit_2_2->text(), mui->label_02_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3_2->text(), mui->lineEdit_3_2->text(), mui->label_03_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4_2->text(), mui->lineEdit_4_2->text(), mui->label_04_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_5_2->text(), mui->lineEdit_5_2->text(), mui->label_05_2->text()) + "<br/>";
    QString material_name = getCheckedRadioButton(mui->groupBox_2)->text();
    sInput += formattedOutput(myOpt, mui->groupBox_2->title().mid(0, mui->groupBox_2->title().length() - 1) + " Mt:", material_name) + "</p>";
    double N = loc.toDouble(mui->lineEdit_N->text());
    double w = loc.toDouble(mui->lineEdit_2_2->text())*myOpt->dwLengthMultiplier;
    double t = loc.toDouble(mui->lineEdit_3_2->text())*myOpt->dwLengthMultiplier;
    double p = loc.toDouble(mui->lineEdit_5_2->text())*myOpt->dwLengthMultiplier;
    double f = loc.toDouble(mui->lineEdit_freq2->text())*myOpt->dwFrequencyMultiplier;
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    double I = result.N;
    data->inductance = result.N;
    sResult += formattedOutput(myOpt, tr("Inductance") + " L = ", roundTo(I/myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy),
                              qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                              qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Length of winding") + " l = ", roundTo( (N * p + w)/myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy ),
                              qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    Material mt = Cu;
    checkMaterial2(&mt);
    double dencity = mtrl[mt][Dencity];
    double mass = dencity * w * t * result.sec;
    sResult += formattedOutput(myOpt, tr("Weight of wire") + " m = ", roundTo(mass, loc, myOpt->dwAccuracy), tr("g")) + "<br/>";
    double reactance = 2 * M_PI * I * f;
    double Resistivity = mtrl[mt][Rho]*1e6;
    double R = (Resistivity * result.sec) / (w * t); // DC resistance of the wire
    sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(R, loc, myOpt->dwAccuracy), tr("Ohm")) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Reactance of the coil") + " X = ", roundTo(reactance, loc, myOpt->dwAccuracy), tr("Ohm"));
    sResult += "<br/><br/>";
    if (f < 0.7 * result.fourth){
        sResult += formattedOutput(myOpt, tr("Self capacitance") + " Cs = ", roundTo(result.thd/myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Coil self-resonance frequency") + " Fsr = ", roundTo(result.fourth/myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8())) + "<br/>";
        double Ql = (double)result.six;
        sResult += formattedOutput(myOpt, tr("Coil constructive Q-factor") + " Q = ", QString::number(result.six)) + "<br/>";
        double ESR = result.seven;
        sResult += formattedOutput(myOpt, tr("Loss resistance") + " ESR = ", roundTo(ESR, loc, myOpt->dwAccuracy), tr("Ohm")) + "</p>";
        if(myOpt->isAdditionalResult){
            sResult += "<hr/><p>";
            sResult += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcLC2(I, f);
            sResult += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    roundTo((data->capacitance - result.thd) / myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            sResult += " => " + tr("Characteristic impedance") + ": ρ = " + loc.toString(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);// Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            sResult += " => "  + tr("Equivalent resistance") + ": Re = " + roundTo(Re / 1000, loc, myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * data->frequency / Qs;
            sResult += " => " + tr("Bandwidth") + ": 3dBΔf = " + roundTo(deltaf, loc, myOpt->dwAccuracy) + tr("kHz");
        }
        if(myOpt->isShowLTSpice){
            sResult += "<hr/><p>";
            sResult += "<u>" + tr("Input data for LTSpice") + ":</u><br/>";
            sResult += "Inductance: " + QString::number(data->inductance * ((1 + Ql * Ql)/(Ql * Ql)), 'f', myOpt->dwAccuracy) + "μ" + "<br/>";
            sResult += "Series resistance: " + QString::number(R * 1000, 'f', myOpt->dwAccuracy) + "m" + "<br/>";
            sResult += "Parallel resistance: " + QString::number((ESR * (1 + Ql * Ql)) / 1000, 'f', myOpt->dwAccuracy) + "k" + "<br/>";
            sResult += "Parallel capacitance: " + QString::number(result.thd, 'f', myOpt->dwAccuracy) + "p" + "</p>";
        }
    } else {
        QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
        mui->statusBar->showMessage(message);
        sResult += "<span style=\"color:blue;\">" + message + "</span>";
    }
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_N->setFocus();
    mui->lineEdit_N->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerI_Poligonal_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/Coil3.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_N->text(), mui->lineEdit_N->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_freq2->text(), mui->lineEdit_freq2->text(), mui->label_freq_m2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1_2->text(), mui->lineEdit_1_2->text(), mui->label_01_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2_2->text(), mui->lineEdit_2_2->text(), mui->label_02_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3_2->text(), mui->lineEdit_3_2->text(), mui->label_03_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4_2->text(), mui->lineEdit_4_2->text(), mui->label_04_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_5_2->text(), mui->lineEdit_5_2->text()) + "<br/>";
    QString material_name = getCheckedRadioButton(mui->groupBox_2)->text();
    sInput += formattedOutput(myOpt, mui->groupBox_2->title().mid(0, mui->groupBox_2->title().length() - 1) + " Mt:", material_name) + "</p>";
    double N = loc.toDouble(mui->lineEdit_N->text());
    double d = 0;
    if (myOpt->isAWG){
        d = convertfromAWG(mui->lineEdit_2_2->text());
    } else {
        d = loc.toDouble(mui->lineEdit_2_2->text())*myOpt->dwLengthMultiplier;
    }
    double k = loc.toDouble(mui->lineEdit_3_2->text())*myOpt->dwLengthMultiplier;
    double p = loc.toDouble(mui->lineEdit_4_2->text())*myOpt->dwLengthMultiplier;
    double f = loc.toDouble(mui->lineEdit_freq2->text())*myOpt->dwFrequencyMultiplier;
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    double I = result.sec;
    data->inductance = result.sec;
    sResult += formattedOutput(myOpt, tr("Inductance") + " L = ", roundTo(I/myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy),
                              qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
    double lW = result.thd;
    QString _wire_length = formatLength(lW, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                              qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Length of winding") + " l = ", roundTo( (N * p + k)/myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                              qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    Material mt = Cu;
    checkMaterial2(&mt);
    double dencity = mtrl[mt][Dencity];
    double mass = 0.25 * dencity * M_PI * d * d * lW;
    sResult += formattedOutput(myOpt, tr("Weight of wire") + " m = ", roundTo(mass, loc, myOpt->dwAccuracy), tr("g")) + "<br/>";
    double reactance = 2 * M_PI * I * f;
    double Resistivity = mtrl[mt][Rho]*1e6;
    double R = (Resistivity * lW * 4) / (M_PI * d * d); // DC resistance of the wire
    sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(R, loc, myOpt->dwAccuracy), tr("Ohm")) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Reactance of the coil") + " X = ", roundTo(reactance, loc, myOpt->dwAccuracy), tr("Ohm"));
    sResult += "<br/><br/>";
    if (f < 0.7 * result.five){
        sResult += formattedOutput(myOpt, tr("Self capacitance") + " Cs = ", roundTo(result.fourth/myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Coil self-resonance frequency") + " Fsr = ", roundTo(result.five/myOpt->dwFrequencyMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8())) + "<br/>";
        double Ql = (double)result.six;
        sResult += formattedOutput(myOpt, tr("Coil constructive Q-factor") + " Q = ", QString::number(result.six)) + "<br/>";
        double ESR = result.seven;
        sResult += formattedOutput(myOpt, tr("Loss resistance") + " ESR = ", roundTo(ESR, loc, myOpt->dwAccuracy), tr("Ohm")) + "</p>";
        if(myOpt->isAdditionalResult){
            sResult += "<hr/><p>";
            sResult += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcLC2(I, f);
            sResult += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    roundTo((data->capacitance - result.fourth) / myOpt->dwCapacityMultiplier, loc, myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            sResult += " => " + tr("Characteristic impedance") + ": ρ = " + loc.toString(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            sResult += " => "  + tr("Equivalent resistance") + ": Re = " + roundTo(Re / 1000, loc, myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * f / Qs;
            sResult += " => " + tr("Bandwidth") + ": 3dBΔf = " + roundTo(deltaf, loc, myOpt->dwAccuracy) + tr("kHz");
        }
        if(myOpt->isShowLTSpice){
            sResult += "<hr/><p>";
            sResult += "<u>" + tr("Input data for LTSpice") + ":</u><br/>";
            sResult += "Inductance: " + QString::number(data->inductance * ((1 + Ql * Ql)/(Ql * Ql)), 'f', myOpt->dwAccuracy) + "μ" + "<br/>";
            sResult += "Series resistance: " + QString::number(R * 1000, 'f', myOpt->dwAccuracy) + "m" + "<br/>";
            sResult += "Parallel resistance: " + QString::number((ESR * (1 + Ql * Ql)) / 1000, 'f', myOpt->dwAccuracy) + "k" + "<br/>";
            sResult += "Parallel capacitance: " + QString::number(result.fourth, 'f', myOpt->dwAccuracy) + "p" + "</p>";
        }
    } else {
        QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
        mui->statusBar->showMessage(message);
        sResult += "<span style=\"color:blue;\">" + message + "</span>";
    }
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_N->setFocus();
    mui->lineEdit_N->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerI_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "";
    if (myOpt->windingKind == 0)
        sImage = "<img src=\":/images/res/Coil4.png\">";
    else
        sImage = "<img src=\":/images/res/Coil4o.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_1_2->text(), mui->lineEdit_1_2->text(), mui->label_01_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2_2->text(), mui->lineEdit_2_2->text(), mui->label_02_2->text()) + "<br/>";
    if (mui->radioButton_6->isChecked())
        sInput += formattedOutput(myOpt, mui->label_3_2->text(), mui->lineEdit_3_2->text()) + "<br/>";
    else
        sInput += formattedOutput(myOpt, mui->label_3_2->text(), mui->lineEdit_3_2->text(), mui->label_03_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4_2->text(), mui->lineEdit_4_2->text(), mui->label_04_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_5_2->text(), mui->lineEdit_5_2->text(), mui->label_05_2->text()) + "<br/>";
    sInput += mui->comboBox_checkMLWinding->currentText() + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    if (mui->radioButton_6->isChecked()){
        sResult += formattedOutput(myOpt, tr("Inductance") + " L = ", roundTo(result.N, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
        data->inductance = result.N;
        sResult += formattedOutput(myOpt, tr("Thickness of the coil") + " c = " + roundTo(result.five / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
        data->c = result.five;
        sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(result.fourth, loc, myOpt->dwAccuracy), tr("Ohm")) + "<br/>";
        data->Rdc = result.fourth;
        QString _wire_length = formatLength(result.thd, myOpt->dwLengthMultiplier);
        QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
        QString d_wire_length = list[0];
        QString _ssLengthMeasureUnit = list[1];
        sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                                  qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
        double d = 0;
        if (myOpt->isAWG){
            d = convertfromAWG(mui->lineEdit_4_2->text());
        } else {
            d = loc.toDouble(mui->lineEdit_4_2->text())*myOpt->dwLengthMultiplier;
        }
        double dencity = mtrl[Cu][Dencity];
        double mass = 0.25 * dencity * M_PI * d * d * result.thd;
        sResult += formattedOutput(myOpt, tr("Weight of wire") + " m = ", roundTo(mass, loc, myOpt->dwAccuracy), tr("g")) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Number of layers") + " Nl = ", loc.toString(result.sec));
    } else {
        double N1 = result.thd;
        double N2 = result.fourth;
        double L1 = result.N;
        double L2 = result.sec;
        data->inductance = (L1 + L2)/2;
        sResult += formattedOutput(myOpt, tr("Number of turns of the coil") + " N = ", loc.toString(N1) + "..." + loc.toString(N2)) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Inductance") + " L = ", roundTo(L1 / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy) + "..."
                                  + roundTo(L2 / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()));
    }
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_1_2->setFocus();
    mui->lineEdit_1_2->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerIgap_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/Coil4-0.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_1_2->text(), mui->lineEdit_1_2->text(), mui->label_01_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2_2->text(), mui->lineEdit_2_2->text(), mui->label_02_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3_2->text(), mui->lineEdit_3_2->text(), mui->label_03_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4_2->text(), mui->lineEdit_4_2->text(), mui->label_04_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_5_2->text(), mui->lineEdit_5_2->text(), mui->label_05_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_6_2->text(), mui->lineEdit_6_2->text(), mui->label_06_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_7_2->text(), mui->lineEdit_7_2->text()) + "</p>";
    double N1 = result.thd;
    double N2 = result.fourth;
    double L1 = result.N;
    double L2 = result.sec;
    data->inductance = (L1 + L2) / 2;
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    sResult += formattedOutput(myOpt, tr("Number of turns of the coil") + " N = ", loc.toString(N1) + "..." + loc.toString(N2)) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Inductance") + " L = ", roundTo(L1 / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy) + "..."
                              + roundTo(L2 / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy),
                              qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()));
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_1_2->setFocus();
    mui->lineEdit_1_2->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerI_Rect_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/Coil4_square.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_1_2->text(), mui->lineEdit_1_2->text(), mui->label_01_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2_2->text(), mui->lineEdit_2_2->text(), mui->label_02_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3_2->text(), mui->lineEdit_3_2->text(), mui->label_03_2->text()) + "<br/>";
    if (mui->radioButton_6->isChecked())
        sInput += formattedOutput(myOpt, mui->label_4_2->text(), mui->lineEdit_4_2->text()) + "<br/>";
    else
        sInput += formattedOutput(myOpt, mui->label_4_2->text(), mui->lineEdit_4_2->text(), mui->label_04_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_5_2->text(), mui->lineEdit_5_2->text(), mui->label_05_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_6_2->text(), mui->lineEdit_6_2->text(), mui->label_06_2->text()) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    if (mui->radioButton_6->isChecked()){
        sResult += formattedOutput(myOpt, tr("Inductance") + " L = ", roundTo(result.N, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
        data->inductance = result.N;
        sResult += formattedOutput(myOpt, tr("Thickness of the coil") + " c = ", roundTo(result.five / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
        data->c = result.five;
        QString _wire_length = formatLength(result.thd, myOpt->dwLengthMultiplier);
        QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
        QString d_wire_length = list[0];
        QString _ssLengthMeasureUnit = list[1];
        sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                                  qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
        double d = 0;
        if (myOpt->isAWG){
            d = convertfromAWG(mui->lineEdit_5_2->text());
        } else {
            d = loc.toDouble(mui->lineEdit_5_2->text())*myOpt->dwLengthMultiplier;
        }
        double dencity = mtrl[Cu][Dencity];
        double mass = 0.25 * dencity * M_PI * d * d * result.thd;
        double Resistivity = mtrl[Cu][Rho]*1e6;
        double Rdc = (Resistivity * result.thd * 4) / (M_PI * d * d); // DC resistance of the wire
        sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(Rdc, loc, myOpt->dwAccuracy), tr("Ohm")) + "<br/>";
        data->Rdc = result.fourth;
        sResult += formattedOutput(myOpt, tr("Weight of wire") + " m = ", roundTo(mass, loc, myOpt->dwAccuracy), tr("g")) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Number of layers") + " Nl = ", loc.toString(result.sec));
    } else {
        double N1 = result.thd;
        double N2 = result.fourth;
        double L1 = result.N;
        double L2 = result.sec;
        data->inductance = (L1 + L2) / 2;
        sResult += formattedOutput(myOpt, tr("Number of turns of the coil") + " N = ", loc.toString(N1) + "..." + loc.toString(N2)) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Inductance") + " L = ", roundTo(L1 / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy) + "..."
                                  + roundTo(L2 / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()));
    }
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_1_2->setFocus();
    mui->lineEdit_1_2->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerI_Foil_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/Coil11.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_N->text(), mui->lineEdit_N->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1_2->text(), mui->lineEdit_1_2->text(), mui->label_01_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2_2->text(), mui->lineEdit_2_2->text(), mui->label_02_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3_2->text(), mui->lineEdit_3_2->text(), mui->label_03_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4_2->text(), mui->lineEdit_4_2->text(), mui->label_04_2->text()) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    data->inductance = result.N;
    sResult += formattedOutput(myOpt, tr("Inductance") + " L = ", roundTo(result.N, loc, myOpt->dwAccuracy),
                              qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
    QString _foil_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _foil_length.split(QRegExp(" "), skip_empty_parts);
    QString d_foil_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    sResult += formattedOutput(myOpt, tr("Length of the foil") + " lf = ", roundTo(d_foil_length.toDouble(), loc, myOpt->dwAccuracy),
                              qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Outside diameter") + " Do = ", roundTo(result.thd / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                              qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
    sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(result.fourth, loc, myOpt->dwAccuracy), tr("Ohm")) + " (" + tr("Copper") + ")<br/>";
    sResult += formattedOutput(myOpt, tr("Resistance of the coil") + " Rdc = ", roundTo(result.five, loc, myOpt->dwAccuracy), tr("Ohm")) + " (" + tr("Aluminum") + ")";
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_N->setFocus();
    mui->lineEdit_N->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_ferriteI_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/T-core.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_N->text(), mui->lineEdit_N->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1_2->text(), mui->lineEdit_1_2->text(), mui->label_01_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2_2->text(), mui->lineEdit_2_2->text(), mui->label_02_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3_2->text(), mui->lineEdit_3_2->text(), mui->label_03_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4_2->text(), mui->lineEdit_4_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_5_2->text(), mui->lineEdit_5_2->text(), mui->label_05_2->text()) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    if (result.sec > 0) {
        data->inductance = result.N;
        QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
        QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
        QString d_wire_length = list[0];
        QString _ssLengthMeasureUnit = list[1];
        sResult += formattedOutput(myOpt, tr("Inductance") + " L = ", roundTo(result.N / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy),
                                   qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                                   qApp->translate("Context",_ssLengthMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(myOpt, "A<sub>L</sub> = ", loc.toString(result.thd), qApp->translate("Context","nH") + "/N<sup>2</sup>");
        sResult += "<br/><br/>" + formattedOutput(myOpt, tr("Effective magnetic path length") + " (l<sub>e</sub>): ",
                                                  roundTo(result.fourth/myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                                                  qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(myOpt, tr("Effective area of magnetic path") + " (A<sub>e</sub>): ",
                                   roundTo(result.five/(myOpt->dwLengthMultiplier * myOpt->dwLengthMultiplier), loc, myOpt->dwAccuracy),
                                   qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<sup>2</sup><br/>";
        sResult += formattedOutput(myOpt, tr("Effective volume") + " (V<sub>e</sub>): ",
                                   roundTo(result.fourth * result.five/(myOpt->dwLengthMultiplier * myOpt->dwLengthMultiplier * myOpt->dwLengthMultiplier), loc, myOpt->dwAccuracy),
                                   qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<sup>3</sup><br/>";
    } else {
        sResult += "<span style=\"color:red;\">" + tr("Coil can not be realized") + "! </span>";
        mui->statusBar->showMessage(tr("Coil can not be realized") + "!");
    }
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    if (satCurrentDockWidget != nullptr){
        if(satCurrentDockWidget->isVisible()){
            _FerriteData ferriteData;
            ferriteData.N = loc.toDouble(mui->lineEdit_N->text());
            ferriteData.mu = loc.toDouble(mui->lineEdit_4_2->text());
            ferriteData.le = result.fourth;
            emit sendFerriteData(ferriteData);
        }
    }
    mui->lineEdit_N->setFocus();
    mui->lineEdit_N->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_pcbI_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    sCaption += mui->comboBox_checkPCB->currentText() + "<br/>";
    QString sImage = "";
    switch (myOpt->layoutPCBcoil) {
    case 0:{
        sImage = "<img src=\":/images/res/Coil8.png\">";
        break;
    }
    case 1:{
        sImage = "<img src=\":/images/res/Coil9.png\">";
        break;
    }
    case 2:{
        sImage = "<img src=\":/images/res/Coil8r.png\">";
        break;
    }
    default:
        break;
    }
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_N->text(), mui->lineEdit_N->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_freq2->text(), mui->lineEdit_freq2->text(), mui->label_freq_m2->text()) + "<br/>";
    if (mui->comboBox_checkPCB->currentIndex() == 2)
        sInput += formattedOutput(myOpt, mui->label_1_2->text(), mui->lineEdit_1_2->text(), mui->label_01_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2_2->text(), mui->lineEdit_2_2->text(), mui->label_02_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3_2->text(), mui->lineEdit_3_2->text(), mui->label_03_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_4_2->text(), mui->lineEdit_4_2->text(), mui->label_04_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_5_2->text(), mui->lineEdit_5_2->text(), mui->label_05_2->text()) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    if (result.N > 0){
        data->inductance = result.N;
        sResult += formattedOutput(myOpt, tr("Inductance") + " L = ", roundTo(result.N / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy),
                                  qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
        if (mui->comboBox_checkPCB->currentIndex() != 2){
            sResult += formattedOutput(myOpt, tr("Outside diameter")+" D = ", roundTo(result.five / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                                      qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
            data->Do = QString::number(result.five, 'f', myOpt->dwAccuracy).toDouble();
        } else {
            sResult += formattedOutput(myOpt, tr("Inside dimension")+" a = ", roundTo(result.five / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy),
                                      qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8())) + "<br/>";
            data->Di = QString::number(result.five, 'f', myOpt->dwAccuracy).toDouble();
        }
        sResult += formattedOutput(myOpt, tr("Coil constructive Q-factor") + " Q ≈ ", QString::number(round(result.fourth)));
    } else
        sResult += tr("Invalid input parameter combination");
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_N->setFocus();
    mui->lineEdit_N->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_spiralI_Result(_CoilResult result)
{
    QString sCaption = windowTitle() + " - " + mui->listWidget->currentItem()->text();
    QString sImage = "<img src=\":/images/res/Coil10.png\">";
    QString sInput = "<p><u>" + tr("Input") + ":</u><br/>";
    sInput += formattedOutput(myOpt, mui->label_N->text(), mui->lineEdit_N->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_1_2->text(), mui->lineEdit_1_2->text(), mui->label_01_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_2_2->text(), mui->lineEdit_2_2->text(), mui->label_02_2->text()) + "<br/>";
    sInput += formattedOutput(myOpt, mui->label_3_2->text(), mui->lineEdit_3_2->text(), mui->label_03_2->text()) + "</p>";
    QString sResult = "<p><u>" + tr("Result") + ":</u><br/>";
    data->inductance = result.N;
    sResult += formattedOutput(myOpt, tr("Inductance") + " L = ", roundTo(result.N / myOpt->dwInductanceMultiplier, loc, myOpt->dwAccuracy),
                              qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    sResult += formattedOutput(myOpt, tr("Length of wire without leads") + " lw = ", roundTo(d_wire_length.toDouble(), loc, myOpt->dwAccuracy),
                              qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()));
    sResult += "</p>";
    showOutput(sCaption, sImage, sInput, sResult);
    mui->lineEdit_N->setFocus();
    mui->lineEdit_N->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ADDITIONAL CALCULATIONS MENU
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionFerrite_toroid_permeability_triggered()
{
    Find_Permeability *fPerm = new Find_Permeability();
    fPerm->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(fPerm, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fPerm, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), fPerm, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    fPerm->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionAL_factor_calculation_triggered()
{
    AL *fAL = new AL();
    fAL->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fAL, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), fAL, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    fAL->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionToroidal_inductor_at_RF_triggered()
{
    RF_Toroid *rf_toroid = new RF_Toroid();
    rf_toroid->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(rf_toroid, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), rf_toroid, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), rf_toroid, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    rf_toroid->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionCross_over_inductor_triggered()
{
    Crossover *fCrossover = new Crossover();
    fCrossover->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fCrossover, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), fCrossover, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    fCrossover->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionBandspread_Calculator_triggered()
{
    Bandspread *fBandspread = new Bandspread();
    fBandspread->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(fBandspread, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fBandspread, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), fBandspread, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    fBandspread->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionRM_core_coil_triggered()
{
    RMcore *fRMcore = new RMcore();
    fRMcore->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(fRMcore, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(fRMcore, SIGNAL(showSaturation(bool)), this, SLOT(showSaturationDockWidget(bool)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fRMcore, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), fRMcore, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    fRMcore->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionSaturation_current_triggered()
{
    if(satCurrentDockWidget != nullptr)
        if (satCurrentDockWidget->isHidden()){
            satCurrentDockWidget->show();
            myOpt->isDockWidgetVisible = true;
            mui->toolButton_Saturation->setChecked(true);
        }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionCoil_on_a_ferrite_rod_triggered()
{
    Ferrite_Rod *fRod = new Ferrite_Rod();
    fRod->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(fRod, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fRod, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), fRod, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    fRod->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionPCB_meandr_coil_triggered()
{
    Meander_pcb *fMeandr = new Meander_pcb();
    fMeandr->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(fMeandr, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fMeandr, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), fMeandr, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    fMeandr->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionMetal_detector_search_coil_triggered()
{
    Multiloop *fMultiloop = new Multiloop();
    fMultiloop->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(fMultiloop, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fMultiloop, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), fMultiloop, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    fMultiloop->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionLoop_of_one_turn_triggered()
{
    Loop *floop = new Loop();
    floop->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(floop, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), floop, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), floop, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    floop->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionShiels_influence_triggered()
{
    Shield *fshield = new Shield();
    fshield->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(fshield, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fshield, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), fshield, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    fshield->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionAmidon_cores_triggered()
{
    Amidon *famidon = new Amidon();
    famidon->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(famidon, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), famidon, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), famidon, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    famidon->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionAir_core_toroid_coil_triggered()
{
    AirCoreToroid *faircoretoroid = new AirCoreToroid();
    faircoretoroid->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(faircoretoroid, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), faircoretoroid, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), faircoretoroid, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    faircoretoroid->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionPot_core_coil_triggered()
{
    PotCore *fpotcore = new PotCore();
    fpotcore->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(fpotcore, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(fpotcore, SIGNAL(showSaturation(bool)), this, SLOT(showSaturationDockWidget(bool)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fpotcore, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), fpotcore, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    fpotcore->show();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionE_core_coil_triggered()
{
    ECore *fecore = new ECore();
    fecore->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(fecore, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(fecore, SIGNAL(showSaturation(bool)), this, SLOT(showSaturationDockWidget(bool)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fecore, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), fecore, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    fecore->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionU_core_coil_triggered()
{
    UCore *fucore = new UCore();
    fucore->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(fucore, SIGNAL(sendResult(QString)), this, SLOT(getAddCalculationResult(QString)));
    connect(fucore, SIGNAL(showSaturation(bool)), this, SLOT(showSaturationDockWidget(bool)));
    connect(this, SIGNAL(sendOpt(_OptionStruct)), fucore, SLOT(getOpt(_OptionStruct)));
    connect(this, SIGNAL(sendLocale(QLocale)), fucore, SLOT(getCurrentLocale(QLocale)));
    emit sendLocale(loc);
    emit sendOpt(*myOpt);
    fucore->exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::getAddCalculationResult(QString resultTxt){
    QStringList result = resultTxt.split(LIST_SEPARATOR);
    showOutput(result[0], result[1], result[2], result[3]);
    if (satCurrentDockWidget != nullptr){
        if (satCurrentDockWidget->isVisible()){
            _FerriteData ferriteData;
            if(result.length() > 4){
                QString satData = result[4];
                QStringList satDataList = satData.split(";");
                ferriteData.N = loc.toDouble(satDataList[0]);
                ferriteData.mu = loc.toDouble(satDataList[1]);
                ferriteData.le = loc.toDouble(satDataList[2]);
            } else {
                ferriteData.N = 0;
                ferriteData.mu = 0;
                ferriteData.le = 0;
            }
            emit sendFerriteData(ferriteData);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_comboBox_checkPCB_activated(int index)
{
    int tab = mui->tabWidget->currentIndex();
    if (index != 2){
        if (tab == 0){
            mui->lineEdit_3->setVisible(false);
            mui->label_3->setVisible(false);
            mui->label_03->setVisible(false);
            mui->label_1->setText(tr("Outside diameter")+" D:");
            mui->label_2->setText(tr("Inside diameter")+" d:");
            mui->label_4->setText(tr("PCB trace thickness")+" t:");
            mui->lineEdit_1->setText(roundTo(data->Do / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2->setText(roundTo(data->Di / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_4->setText(roundTo(data->th / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
        } else {
            mui->lineEdit_1_2->setVisible(false);
            mui->label_1_2->setVisible(false);
            mui->label_01_2->setVisible(false);
            mui->lineEdit_N->setValidator(dv);
            mui->lineEdit_N->setText(roundTo(data->N, loc, myOpt->dwAccuracy));
            mui->lineEdit_N->selectAll();
            mui->label_1_2->setText(tr("Outside diameter")+" D:");
            mui->label_2_2->setText(tr("Inside diameter")+" d:");
            mui->label_3_2->setText(tr("Winding pitch")+" s:");
            mui->label_4_2->setText(tr("Width of a PCB trace") + " W:");
            mui->label_5_2->setText(tr("PCB trace thickness")+" t:");
            mui->lineEdit_2_2->setText(roundTo(data->Di / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3_2->setText(roundTo(data->s / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_4_2->setText(roundTo(data->w / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_5_2->setText(roundTo(data->th / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
        }
    } else {
        if (tab == 0){
            mui->lineEdit_3->setVisible(true);
            mui->label_3->setVisible(true);
            mui->label_03->setVisible(true);
            mui->label_1->setText(tr("Outside dimension")+" A:");
            mui->label_2->setText(tr("Outside dimension")+" B:");
            mui->label_3->setText(tr("Inside dimension")+" a:");
            mui->label_4->setText(tr("PCB trace thickness")+" t:");
            mui->lineEdit_1->setText(roundTo(data->Do / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2->setText(roundTo(data->B / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3->setText(roundTo(data->Di / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_4->setText(roundTo(data->th / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
        } else {
            mui->lineEdit_1_2->setVisible(true);
            mui->label_1_2->setVisible(true);
            mui->label_01_2->setVisible(true);
            mui->lineEdit_N->setValidator(iv);
            mui->lineEdit_N->setText(roundTo(int(data->N), loc, myOpt->dwAccuracy));
            mui->lineEdit_N->selectAll();
            mui->label_1_2->setText(tr("Outside dimension")+" A:");
            mui->label_2_2->setText(tr("Outside dimension")+" B:");
            mui->label_3_2->setText(tr("Winding pitch")+" s:");
            mui->label_4_2->setText(tr("Width of a PCB trace") + " W:");
            mui->label_5_2->setText(tr("PCB trace thickness")+" t:");
            mui->lineEdit_1_2->setText(roundTo(data->Do / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_2_2->setText(roundTo(data->B / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_3_2->setText(roundTo(data->s / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_4_2->setText(roundTo(data->w / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
            mui->lineEdit_5_2->setText(roundTo(data->th / myOpt->dwLengthMultiplier, loc, myOpt->dwAccuracy));
        }
    }
    if (index == 0){
        mui->image->setPixmap(QPixmap(":/images/res/Coil8.png"));
        myOpt->layoutPCBcoil = 0;
    }
    if (index == 1) {
        mui->image->setPixmap(QPixmap(":/images/res/Coil9.png"));
        myOpt->layoutPCBcoil = 1;
    }
    if (index == 2) {
        mui->image->setPixmap(QPixmap(":/images/res/Coil8r.png"));
        myOpt->layoutPCBcoil = 2;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_comboBox_checkMLWinding_activated(int index)
{
    if (index == 0){
        mui->image->setPixmap(QPixmap(":/images/res/Coil4.png"));
        myOpt->windingKind = 0;
    }
    if (index == 1) {
        mui->image->setPixmap(QPixmap(":/images/res/Coil4o.png"));
        myOpt->windingKind = 1;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_textBrowser_anchorClicked(const QUrl &arg1)
{
    QString url = arg1.toDisplayString();
    if (url == "del"){
        QString searchString = QString(QChar(CLEAR_CHAR));
        QTextCursor cursor =  mui->textBrowser->textCursor();
        int start = mui->textBrowser->textCursor().position();
        if (myOpt->isConfirmDelete){
            cursor.setPosition(start + 3, QTextCursor::KeepAnchor);
            cursor.select(QTextCursor::WordUnderCursor);
            QString sCalc_number = cursor.selectedText();
            QMessageBox messageBox(QMessageBox::Question,
                                   tr("Confirmation"),
                                   "#" + sCalc_number + tr(" calculation will be deleted") + ". \n" + tr("Are you sure?"),
                                   QMessageBox::Yes | QMessageBox::No,
                                   this);
            messageBox.setButtonText(QMessageBox::Yes, tr("Yes"));
            messageBox.setButtonText(QMessageBox::No, tr("No"));
            if (messageBox.exec()== QMessageBox::No)
                return;
        }
        int i = mui->textBrowser->textCursor().position();
        while (!cursor.isNull() && !cursor.atEnd()) {
            i++;
            cursor.setPosition(i, QTextCursor::KeepAnchor);
            if (!cursor.isNull()) {
                cursor.select(QTextCursor::WordUnderCursor);
                QString word = cursor.selectedText();
                if (word == searchString)
                    break;
            }
        }
        cursor.setPosition(start - 1, QTextCursor::MoveAnchor);
        cursor.setPosition(i, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.clearSelection();
        mui->textBrowser->setTextCursor(cursor);
        if ((mui->textBrowser->document()->lineCount() < 4) && (!mui->textBrowser->document()->isEmpty())){
            mui->textBrowser->clear();
            mui->statusBar->clearMessage();
        }
    }
}
