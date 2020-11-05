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
    data = new _Data;
    myOpt = new _OptionStruct;
    dv = new QDoubleValidator;


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
    myOpt->isConfirmExit = settings->value( "isConfirmExit", true ).toBool();
    myOpt->isConfirmClear = settings->value( "isConfirmClear", true ).toBool();
    myOpt->isConfirmDelete = settings->value( "isConfirmDelete", true ).toBool();
    myOpt->isAdditionalResult = settings->value( "isAdditionalResult", true ).toBool();
    myOpt->mainFontFamily = settings->value("MainFontFamily", QFontInfo(QFont()).family()).toString();
    myOpt->mainFontSize = settings->value("MainFontSize", QFontInfo(QFont()).pixelSize()).toInt();
    myOpt->textFontFamily = settings->value("TextFontFamily", QFontInfo(QFont()).family()).toString();
    myOpt->textFontSize = settings->value("TextFontSize", QFontInfo(QFont()).pixelSize()).toInt();
    myOpt->isEnglishLocale = settings->value( "isEnglishLocale", false ).toBool();
    myOpt->isPCBcoilSquare = settings->value("isPCBcoilSquare",true).toBool();
    myOpt->isSaveOnExit = settings->value("isSaveOnExit",true).toBool();
    myOpt->styleGUI = settings->value("styleGUI",1).toInt();
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
    mui->toolButton_showAdditional->setChecked(myOpt->isAdditionalResult);
    mui->toolButton_cbc->setChecked(myOpt->isConfirmClear);
    mui->toolButton_cbe->setChecked(myOpt->isConfirmExit);
    mui->toolButton_cdsr->setChecked(myOpt->isConfirmDelete);
    mui->toolButton_soe->setChecked(myOpt->isSaveOnExit);
    mui->toolButton_lShowFirst->setChecked(myOpt->isLastShowingFirst);

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

    //Start Allow only float values in input fields
    //QRegExpValidator *dv = new QRegExpValidator();
    //dv->setRegExp(QRegExp("([-]{0,1})([0-9]{0,9})([,.]{0,1}[0-9]{0,9})"));
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
    if ((settings->value("mainWindowGeometry").isValid()) && (settings->value("mainWindowState").isValid())){
        restoreGeometry(settings->value("mainWindowGeometry").toByteArray());
        restoreState(settings->value("mainWindowState").toByteArray());
    } else {
        this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->minimumSize(),qApp->primaryScreen()->availableGeometry()));
    }
    mui->retranslateUi(this);

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
    action0->setIcon(QPixmap(":/menu_ico/res/icons8-inconsistency-32.png"));
    action1->setShortcut(QKeySequence(Qt::ALT + Qt::Key_A));
    action1->setIcon(QPixmap(":/menu_ico/res/icons8-copy-32.png"));
    action2->setShortcut(QKeySequence::Save);
    action2->setIcon(QPixmap(":/menu_ico/res/icons8-save-32.png"));
    action3->setShortcut(QKeySequence::Open);
    action3->setIcon(QPixmap(":/menu_ico/res/icons8-opened-folder-32.png"));
    action4->setShortcut(QKeySequence::Delete);
    action4->setIcon(QPixmap(":/menu_ico/res/icons8-delete-30.png"));
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
    this->on_textBrowser_textChanged();
    on_tabWidget_currentChanged(tab);
    delete settings;
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
    delete popupmenu;
    delete data;
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
        settings->setValue("isAdditionalResult", myOpt->isAdditionalResult);
        settings->setValue("isConfirmExit", myOpt->isConfirmExit);
        settings->setValue("isConfirmClear", myOpt->isConfirmClear);
        settings->setValue("isConfirmDelete", myOpt->isConfirmDelete);
        settings->setValue("MainFontFamily", myOpt->mainFontFamily);
        settings->setValue("MainFontSize", myOpt->mainFontSize);
        settings->setValue("TextFontFamily", myOpt->textFontFamily);
        settings->setValue("TextFontSize", myOpt->textFontSize);
        settings->setValue("isEnglishLocale", myOpt->isEnglishLocale);
        settings->setValue("isPCBcoilSquare", myOpt->isPCBcoilSquare);
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
        settings->setValue("mainWindowGeometry", saveGeometry());
        settings->setValue("mainWindowState", saveState());
        QApplication::closeAllWindows();
        delete settings;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::showEvent(QShowEvent *event){
    event->accept();
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
    switch (myOpt->styleGUI) {
    case _DefaultStyle:
        on_actionThemeDefault_triggered();
        break;
    case _DarkStyle:
        on_actionThemeDark_triggered();
        break;
    default:
        break;
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
    mui->toolButton_cbc->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_cbe->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_cdsr->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_soe->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    mui->toolButton_lShowFirst->setIconSize(QSize(myOpt->mainFontSize * 2, myOpt->mainFontSize * 2));
    QFont f2 = mui->textBrowser->font();
    f2.setFamily(myOpt->textFontFamily);
    f2.setPixelSize(myOpt->textFontSize);
    mui->textBrowser->setFont(f2);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::prepareHeader(QTextCursor *c)
{
    if(myOpt->isLastShowingFirst)
        c->movePosition(QTextCursor::Start);
    else
        c->movePosition(QTextCursor::End);
    calc_count++;
    c->insertHtml("<hr/><a href=\"del\" title=\"" +tr("Delete this result") + "\">" + QString(QChar(CLEAR_CHAR)) + "</a><hr/><b>"
                  + QString::number(calc_count)+ "</b> - " + loc.toString(QDateTime::currentDateTime(), QLocale::ShortFormat));
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
        case _Onelayer:{
            QDesktopServices::openUrl(QUrl("https://coil32.net/coil-with-winding-pitch.html"));
            break;
        }
        case _Onelayer_p:{
            QDesktopServices::openUrl(QUrl("https://coil32.net/coil-with-winding-pitch.html"));
            break;
        }
        case _Onelayer_q:{
            QDesktopServices::openUrl(QUrl("https://coil32.net/coil-on-not-circular-former.html"));
            break;
        }
        case _Multilayer:{
            QDesktopServices::openUrl(QUrl("https://coil32.net/multi-layer-coil.html"));
            break;
        }
        case _Multilayer_p:{
            QDesktopServices::openUrl(QUrl("https://coil32.net/multi-layer-coil.html"));
            break;
        }
        case _Multilayer_r:{
            QDesktopServices::openUrl(QUrl("https://coil32.net/multilayer-rectangular.html"));
            break;
        }
        case _FerrToroid:{
            QDesktopServices::openUrl(QUrl("https://coil32.net/ferrite-toroid-core.html"));
            break;
        }
        case _PCB_coil:{
            QDesktopServices::openUrl(QUrl("https://coil32.net/pcb-coil.html"));
            break;
        }
        case _Flat_Spiral:{
            QDesktopServices::openUrl(QUrl("https://coil32.net/foil-wound-coil-calculation.html"));
            break;
        }
        case _Multilayer_f:{
            QDesktopServices::openUrl(QUrl("https://coil32.net/foil-wound-coil-calculation.html"));
            break;
        }
        default:
            break;
        }
    } else if (tab == 2){
        QDesktopServices::openUrl(QUrl("https://coil32.net/lc-resonance-calculation.html"));
    }
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
        case _Onelayer_cw:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil1.png"));
            mui->groupBox->setVisible(true);
            mui->groupBox_6->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            tmp_txt = tr("Former diameter") + " D:";
            mui->label_1->setText(tmp_txt);
            tmp_txt = tr("Wire diameter") + " d:";
            mui->label_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter with insulation") + " k:";
            mui->label_3->setText(tmp_txt);
            mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_freq->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
            mui->lineEdit_1->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2->setText("");
            } else
                mui->lineEdit_2->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_2->text().isEmpty() || (mui->lineEdit_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_3->setText(loc.toString(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_2_editingFinished();
            break;
        }
        case _Onelayer:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil2.png"));
            mui->groupBox->setVisible(true);
            mui->groupBox_6->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_freq->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
            mui->lineEdit_1->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2->setText("");
            } else
                mui->lineEdit_2->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_2->text().isEmpty() || (mui->lineEdit_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_3->setText(loc.toString(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_2_editingFinished();
            mui->lineEdit_4->setText(loc.toString(data->p / myOpt->dwLengthMultiplier));
            break;
        }
        case _Onelayer_p:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil2_square.png"));
            mui->groupBox->setVisible(true);
            mui->groupBox_6->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_freq->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
            mui->lineEdit_1->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2->setText(loc.toString(data->w / myOpt->dwLengthMultiplier));
            mui->lineEdit_3->setText(loc.toString(data->t / myOpt->dwLengthMultiplier));
            mui->lineEdit_4->setText(loc.toString(data->isol / myOpt->dwLengthMultiplier));
            mui->lineEdit_5->setText(loc.toString(data->p / myOpt->dwLengthMultiplier));
            break;
        }
        case _Onelayer_q:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil3.png"));
            mui->groupBox->setVisible(true);
            mui->groupBox_6->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_5->setVisible(true);
            mui->label_5->setVisible(true);
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
            tmp_txt = tr("Number of sides of the former")+" n:";
            mui->label_5->setText(tmp_txt);
            mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_freq->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
            mui->lineEdit_1->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2->setText("");
            } else
                mui->lineEdit_2->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_2->text().isEmpty() || (mui->lineEdit_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_3->setText(loc.toString(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_2_editingFinished();
            mui->lineEdit_4->setText(loc.toString(data->p / myOpt->dwLengthMultiplier));
            mui->lineEdit_5->setText(loc.toString(data->ns));
            break;
        }
        case _Multilayer:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2->setText(loc.toString(data->l / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_3->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_3->setText("");
            } else
                mui->lineEdit_3->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_3->text().isEmpty() || (mui->lineEdit_3->text() == "0")|| (data->k > 0))
                mui->lineEdit_4->setText(loc.toString(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_3_editingFinished();
            break;
        }
        case _Multilayer_p:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4-0.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2->setText(loc.toString(data->l / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_3->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_3->setText("");
            } else
                mui->lineEdit_3->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_3->text().isEmpty() || (mui->lineEdit_3->text() == "0")|| (data->k > 0))
                mui->lineEdit_4->setText(loc.toString(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_3_editingFinished();
            mui->lineEdit_5->setText(loc.toString(data->g / myOpt->dwLengthMultiplier));
            mui->lineEdit_6->setText(loc.toString(data->Ng));
            break;
        }
        case _Multilayer_r:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4_square.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->label_05->setVisible(true);
            mui->lineEdit_6->setVisible(false);
            mui->label_6->setVisible(false);
            mui->label_06->setVisible(false);
            mui->line_6->setVisible(false);
            tmp_txt = tr("Former width") + " a:";
            mui->label_1->setText(tmp_txt);
            tmp_txt = tr("Former height") + " b:";
            mui->label_2->setText(tmp_txt);
            tmp_txt = tr("Winding length") + " l:";
            mui->label_3->setText(tmp_txt);
            tmp_txt = tr("Wire diameter") + " d:";
            mui->label_4->setText(tmp_txt);
            tmp_txt = tr("Wire diameter with insulation")+" k:";
            mui->label_5->setText(tmp_txt);
            mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(loc.toString(data->a / myOpt->dwLengthMultiplier));
            mui->lineEdit_2->setText(loc.toString(data->b / myOpt->dwLengthMultiplier));
            mui->lineEdit_3->setText(loc.toString(data->l / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_4->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_4->setText("");
            } else
                mui->lineEdit_4->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_4->text().isEmpty() || (mui->lineEdit_4->text() == "0")|| (data->k > 0))
                mui->lineEdit_5->setText(loc.toString(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_4_editingFinished();
            mui->lineEdit_5->setText(loc.toString(data->k / myOpt->dwLengthMultiplier));
            break;
        }
        case _Multilayer_f:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil11.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_5->setVisible(false);
            mui->label_5->setVisible(false);
            mui->label_05->setVisible(false);
            mui->lineEdit_6->setVisible(false);
            mui->label_6->setVisible(false);
            mui->label_06->setVisible(false);
            mui->line_6->setVisible(false);
            tmp_txt = tr("Former diameter") + " D:";
            mui->label_1->setText(tmp_txt);
            tmp_txt = tr("Foil width") + " w:";
            mui->label_2->setText(tmp_txt);
            tmp_txt = tr("Foil thickness") + " t:";
            mui->label_3->setText(tmp_txt);
            tmp_txt = tr("Insulation thickness")+" g:";
            mui->label_4->setText(tmp_txt);
            mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2->setText(loc.toString(data->w / myOpt->dwLengthMultiplier));
            mui->lineEdit_3->setText(loc.toString(data->t / myOpt->dwLengthMultiplier));
            mui->lineEdit_4->setText(loc.toString(data->isol / myOpt->dwLengthMultiplier));
            break;
        }
        case _FerrToroid:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil6.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(loc.toString(data->Do / myOpt->dwLengthMultiplier));
            mui->lineEdit_2->setText(loc.toString(data->Di / myOpt->dwLengthMultiplier));
            mui->lineEdit_3->setText(loc.toString(data->h / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_4->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_4->setText("");
            } else
                mui->lineEdit_4->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
            mui->lineEdit_5->setText(loc.toString(data->mu));
            break;
        }
        case _PCB_coil:{
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(true);
            mui->comboBox_checkPCB->setVisible(true);
            if (myOpt->isPCBcoilSquare){
                mui->comboBox_checkPCB->setCurrentIndex(0);
                mui->image->setPixmap(QPixmap(":/images/res/Coil8.png"));
            } else {
                mui->comboBox_checkPCB->setCurrentIndex(1);
                mui->image->setPixmap(QPixmap(":/images/res/Coil9.png"));
            }
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
            tmp_txt = tr("Outside diameter")+" D:";
            mui->label_1->setText(tmp_txt);
            tmp_txt = tr("Inside diameter")+" d:";
            mui->label_2->setText(tmp_txt);
            mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(loc.toString(data->Do / myOpt->dwLengthMultiplier));
            mui->lineEdit_2->setText(loc.toString(data->Di / myOpt->dwLengthMultiplier));
            mui->horizontalSlider->setValue(data->ratio * 100);
            break;
        }
        case _Flat_Spiral:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil10.png"));
            mui->groupBox->setVisible(false);
            mui->groupBox_6->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
            mui->lineEdit_ind->selectAll();
            mui->lineEdit_1->setText(loc.toString(data->Di / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2->setText("");
            } else
                mui->lineEdit_2->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
            mui->lineEdit_3->setText(loc.toString(data->s / myOpt->dwLengthMultiplier));
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
        case _Onelayer_cw:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil1.png"));
            mui->groupBox_2->setVisible(true);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            tmp_txt = tr("Former diameter") + " D:";
            mui->label_1_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter") + " d:";
            mui->label_2_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter with insulation") + " k:";
            mui->label_3_2->setText(tmp_txt);
            mui->lineEdit_N->setText(loc.toString(data->N));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_freq2->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
            mui->lineEdit_1_2->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2_2->setText("");
            } else
                mui->lineEdit_2_2->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_2_2->text().isEmpty() || (mui->lineEdit_2_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_3_2->setText(loc.toString(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_2_2_editingFinished();
            break;
        }
        case _Onelayer:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil2.png"));
            mui->groupBox_2->setVisible(true);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_N->setText(loc.toString(data->N));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_freq2->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
            mui->lineEdit_1_2->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2_2->setText("");
            } else
                mui->lineEdit_2_2->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_2_2->text().isEmpty() || (mui->lineEdit_2_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_3_2->setText(loc.toString(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_2_2_editingFinished();
            mui->lineEdit_4_2->setText(loc.toString(data->p / myOpt->dwLengthMultiplier));
            break;
        }
        case _Onelayer_p:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil2_square.png"));
            mui->groupBox_2->setVisible(true);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_N->setText(loc.toString(data->N));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_freq2->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
            mui->lineEdit_1_2->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(loc.toString(data->w / myOpt->dwLengthMultiplier));
            mui->lineEdit_3_2->setText(loc.toString(data->t / myOpt->dwLengthMultiplier));
            mui->lineEdit_4_2->setText(loc.toString(data->isol / myOpt->dwLengthMultiplier));
            mui->lineEdit_5_2->setText(loc.toString(data->p / myOpt->dwLengthMultiplier));
            break;
        }
        case _Onelayer_q:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil3.png"));
            mui->groupBox_2->setVisible(true);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_5_2->setVisible(true);
            mui->label_5_2->setVisible(true);
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
            tmp_txt = tr("Number of sides of the former")+" n:";
            mui->label_5_2->setText(tmp_txt);
            mui->lineEdit_N->setText(loc.toString(data->N));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_freq2->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
            mui->lineEdit_1_2->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_2_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_2_2->setText("");
            } else
                mui->lineEdit_2_2->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_2_2->text().isEmpty() || (mui->lineEdit_2_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_3_2->setText(loc.toString(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_2_2_editingFinished();
            mui->lineEdit_4_2->setText(loc.toString(data->p / myOpt->dwLengthMultiplier));
            mui->lineEdit_5_2->setText(loc.toString(data->ns));
            break;
        }
        case _Multilayer:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(true);
            mui->radioButton_6->setText(tr("Number of turns of the coil") + " (N)");
            mui->radioButton_7->setText(tr("Thickness of the coil") + " (c)");
            mui->radioButton_8->setText(tr("Resistance of the coil") + " (Rdc)");
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_1_2->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(loc.toString(data->l / myOpt->dwLengthMultiplier));
            mui->lineEdit_3_2->setText(loc.toString(data->c / myOpt->dwLengthMultiplier));
            on_radioButton_6_toggled(mui->radioButton_6->isChecked());
            mui->lineEdit_5_2->setText(loc.toString(data->k / myOpt->dwLengthMultiplier));
            break;
        }
        case _Multilayer_p:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4-0.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_1_2->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(loc.toString(data->l / myOpt->dwLengthMultiplier));
            mui->lineEdit_3_2->setText(loc.toString(data->c / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_4_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_4_2->setText("");
            } else
                mui->lineEdit_4_2->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_4_2->text().isEmpty() || (mui->lineEdit_4_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_5_2->setText(loc.toString(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_4_2_editingFinished();
            mui->lineEdit_6_2->setText(loc.toString(data->g / myOpt->dwLengthMultiplier));
            mui->lineEdit_7_2->setText(loc.toString(data->Ng / myOpt->dwLengthMultiplier));
            break;
        }
        case _Multilayer_r:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil4_square.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
            if (myOpt->isAWG){
                mui->label_05_2->setText(tr("AWG"));
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
            mui->lineEdit_7_2->setVisible(false);
            mui->label_7_2->setVisible(false);
            mui->label_07_2->setVisible(false);
            tmp_txt = tr("Former width") + " a:";
            mui->label_1_2->setText(tmp_txt);
            tmp_txt = tr("Former height") + " b:";
            mui->label_2_2->setText(tmp_txt);
            tmp_txt = tr("Winding length") + " l:";
            mui->label_3_2->setText(tmp_txt);
            tmp_txt = tr("Thickness of the coil") + " c:";
            mui->label_4_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter") + " d:";
            mui->label_5_2->setText(tmp_txt);
            tmp_txt = tr("Wire diameter with insulation") + " k:";
            mui->label_6_2->setText(tmp_txt);
            mui->lineEdit_1_2->setText(loc.toString(data->a / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(loc.toString(data->b / myOpt->dwLengthMultiplier));
            mui->lineEdit_3_2->setText(loc.toString(data->l / myOpt->dwLengthMultiplier));
            mui->lineEdit_4_2->setText(loc.toString(data->c / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_5_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_5_2->setText("");
            } else
                mui->lineEdit_5_2->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
            if (mui->lineEdit_5_2->text().isEmpty() || (mui->lineEdit_4_2->text() == "0")|| (data->k > 0))
                mui->lineEdit_6_2->setText(loc.toString(data->k / myOpt->dwLengthMultiplier));
            else
                on_lineEdit_5_2_editingFinished();
            break;
        }
        case _Multilayer_f:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil11.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            tmp_txt = tr("Former diameter") + " D:";
            mui->label_1_2->setText(tmp_txt);
            tmp_txt = tr("Foil width") + " w:";
            mui->label_2_2->setText(tmp_txt);
            tmp_txt = tr("Foil thickness") + " t:";
            mui->label_3_2->setText(tmp_txt);
            tmp_txt = tr("Insulation thickness")+" g:";
            mui->label_4_2->setText(tmp_txt);
            mui->lineEdit_N->setText(loc.toString(data->N));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_freq2->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
            mui->lineEdit_1_2->setText(loc.toString(data->D / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(loc.toString(data->w / myOpt->dwLengthMultiplier));
            mui->lineEdit_3_2->setText(loc.toString(data->t / myOpt->dwLengthMultiplier));
            mui->lineEdit_4_2->setText(loc.toString(data->isol / myOpt->dwLengthMultiplier));
            break;
        }
        case _FerrToroid:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil6.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_N->setText(loc.toString(data->N));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_1_2->setText(loc.toString(data->Do / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(loc.toString(data->Di / myOpt->dwLengthMultiplier));
            mui->lineEdit_3_2->setText(loc.toString(data->h / myOpt->dwLengthMultiplier));
            mui->lineEdit_4_2->setText(loc.toString(data->mu / myOpt->dwLengthMultiplier));
            break;
        }
        case _PCB_coil:{
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(true);
            if (myOpt->isPCBcoilSquare){
                mui->comboBox_checkPCB->setCurrentIndex(0);
                mui->image->setPixmap(QPixmap(":/images/res/Coil8.png"));
                mui->lineEdit_3_2->setVisible(true);
                mui->label_3_2->setVisible(true);
                mui->label_03_2->setVisible(true);
                mui->lineEdit_4_2->setVisible(true);
                mui->label_4_2->setVisible(true);
                mui->label_04_2->setVisible(true);
            } else {
                mui->comboBox_checkPCB->setCurrentIndex(1);
                mui->image->setPixmap(QPixmap(":/images/res/Coil9.png"));
                mui->lineEdit_3_2->setVisible(false);
                mui->label_3_2->setVisible(false);
                mui->label_03_2->setVisible(false);
                mui->lineEdit_4_2->setVisible(false);
                mui->label_4_2->setVisible(false);
                mui->label_04_2->setVisible(false);
            }
            mui->label_N->setVisible(true);
            mui->lineEdit_N->setVisible(true);
            mui->label_freq2->setVisible(false);
            mui->label_freq_m2->setVisible(false);
            mui->lineEdit_freq2->setVisible(false);

            mui->lineEdit_5_2->setVisible(false);
            mui->label_5_2->setVisible(false);
            mui->label_05_2->setVisible(false);
            mui->lineEdit_6_2->setVisible(false);
            mui->label_6_2->setVisible(false);
            mui->label_06_2->setVisible(false);
            mui->lineEdit_7_2->setVisible(false);
            mui->label_7_2->setVisible(false);
            mui->label_07_2->setVisible(false);
            tmp_txt = tr("Outside diameter")+" D:";
            mui->label_1_2->setText(tmp_txt);
            tmp_txt = tr("Inside diameter")+" d:";
            mui->label_2_2->setText(tmp_txt);
            tmp_txt = tr("Winding pitch")+" s:";
            mui->label_3_2->setText(tmp_txt);
            tmp_txt = tr("Width of a PCB trace") + " W:";
            mui->label_4_2->setText(tmp_txt);
            mui->lineEdit_N->setText(loc.toString(data->N));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_1_2->setText(loc.toString(data->Do / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(loc.toString(data->Di / myOpt->dwLengthMultiplier));
            mui->lineEdit_3_2->setText(loc.toString(data->s / myOpt->dwLengthMultiplier));
            mui->lineEdit_4_2->setText(loc.toString(data->w / myOpt->dwLengthMultiplier));
            break;
        }
        case _Flat_Spiral:{
            mui->image->setPixmap(QPixmap(":/images/res/Coil10.png"));
            mui->groupBox_2->setVisible(false);
            mui->groupBox_7->setVisible(false);
            mui->comboBox_checkPCB->setVisible(false);
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
            mui->lineEdit_N->setText(loc.toString(data->N));
            mui->lineEdit_N->selectAll();
            mui->lineEdit_1_2->setText(loc.toString(data->Do / myOpt->dwLengthMultiplier));
            mui->lineEdit_2_2->setText(loc.toString(data->Di / myOpt->dwLengthMultiplier));
            if (myOpt->isAWG){
                if (data->d > 0){
                    mui->lineEdit_3_2->setText(converttoAWG(data->d));
                } else
                    mui->lineEdit_3_2->setText("");
            } else
                mui->lineEdit_3_2->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
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
    QSettings *settings;
    defineAppSettings(settings);
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
void MainWindow::on_actionThemeDefault_triggered()
{
    qApp->setStyleSheet("");
    mui->actionThemeDark->setChecked(false);
    mui->actionThemeDefault->setChecked(true);
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
    mui->actionThemeDark->setChecked(true);
    mui->actionThemeDefault->setChecked(false);
    myOpt->styleGUI = _DarkStyle;
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
        if (!sD.exists())
            sD.mkpath(savePath);
        QString filters(".pdf (*.pdf);;.odf (*.odf);;.htm (*.htm)");
        QString defaultFilter(".htm (*.htm)");
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save"), savePath, filters, &defaultFilter);
        if (!fileName.isEmpty()){
            QFileInfo info(fileName);
            savePath = info.absolutePath().toUtf8();
            QSettings *settings;
            defineAppSettings(settings);
            settings->beginGroup( "GUI" );
            settings->setValue("SaveDir", savePath);
            settings->endGroup();
            delete settings;
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
        mui->comboBox_checkPCB->setVisible(false);
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
    mui->lineEdit_1_3->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
    mui->groupBox_2_3->setTitle(tr("External capacitance"));
    mui->lineEdit_2_3->setText(loc.toString(data->capacitance / myOpt->dwCapacityMultiplier));
    mui->label_01_3->setText(qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()));
    mui->label_02_3->setText(qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_CF_clicked()
{
    mui->groupBox_1_3->setTitle(tr("External capacitance"));
    mui->lineEdit_1_3->setText(loc.toString(data->capacitance / myOpt->dwCapacityMultiplier));
    mui->groupBox_2_3->setTitle(tr("Frequency"));
    mui->lineEdit_2_3->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
    mui->label_01_3->setText(qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()));
    mui->label_02_3->setText(qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_LF_clicked()
{
    mui->groupBox_1_3->setTitle(tr("Inductance"));
    mui->lineEdit_1_3->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
    mui->groupBox_2_3->setTitle(tr("Frequency"));
    mui->lineEdit_2_3->setText(loc.toString(data->frequency / myOpt->dwFrequencyMultiplier));
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
void MainWindow::on_toolButton_Configure_clicked()
{
    on_actionOptions_triggered();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_6_toggled(bool checked)
{
    QString tmp_txt;
    if (checked){
        tmp_txt = tr("Number of turns of the coil") + " N:";
        mui->label_3_2->setText(tmp_txt);
        mui->label_03_2->setVisible(false);
        mui->lineEdit_3_2->setText(loc.toString(data->N));
    } else {
        tmp_txt = tr("Thickness of the coil") + " c:";
        mui->label_3_2->setText(tmp_txt);
        mui->label_03_2->setVisible(true);
        mui->lineEdit_3_2->setText(loc.toString(data->c));
    }
    on_radioButton_8_toggled(mui->radioButton_8->isChecked());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_radioButton_8_toggled(bool checked)
{
    QString tmp_txt;
    if (checked){
        tmp_txt = tr("Resistance of the coil") + " Rdc:";
        mui->label_4_2->setText(tmp_txt);
        mui->label_04_2->setText(tr("Ohm"));
        mui->lineEdit_4_2->setText(loc.toString(data->Rdc));
    } else {
        tmp_txt = tr("Wire diameter") + " d:";
        mui->label_4_2->setText(tmp_txt);
        mui->label_04_2->setText(tr("mm"));
        if (myOpt->isAWG){
            if (data->d > 0){
                mui->lineEdit_4_2->setText(converttoAWG(data->d));
            } else
                mui->lineEdit_4_2->setText("");
        } else
            mui->lineEdit_4_2->setText(loc.toString(data->d / myOpt->dwLengthMultiplier));
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
    mui->toolButton_cbc->setChecked(myOpt->isConfirmClear);
    mui->toolButton_cbe->setChecked(myOpt->isConfirmExit);
    mui->toolButton_cdsr->setChecked(myOpt->isConfirmDelete);
    mui->toolButton_soe->setChecked(myOpt->isSaveOnExit);
    mui->toolButton_lShowFirst->setChecked(myOpt->isLastShowingFirst);
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
            if (myOpt->isPCBcoilSquare)
                data->Di = data->Do * 0.362;
            else
                data->Di = data->Do * 0.4;
            mui->lineEdit_2->setText(loc.toString(data->Di));
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
        case _Onelayer_cw:
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
            double k_m = odCalc(d);
            if (d > 0){
                mui->lineEdit_3->setText( loc.toString(k_m / myOpt->dwLengthMultiplier));
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
                mui->lineEdit_4->setText( loc.toString(k_m / myOpt->dwLengthMultiplier));
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
                mui->lineEdit_5->setText( loc.toString(k_m / myOpt->dwLengthMultiplier));
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
        case _PCB_coil:
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
            double k_m = odCalc(d);
            if (d > 0){
                mui->lineEdit_3_2->setText( loc.toString(k_m/myOpt->dwLengthMultiplier));
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
        case _PCB_coil:
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
        case _Multilayer:
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
            if (mui->radioButton_7->isChecked()){
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
                    mui->lineEdit_5_2->setText( loc.toString(k_m/myOpt->dwLengthMultiplier));
                }
            } else {
                data->Rdc = loc.toDouble(mui->lineEdit_4_2->text(), &ok)*myOpt->dwLengthMultiplier;
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
                mui->lineEdit_5_2->setText( loc.toString(k_m/myOpt->dwLengthMultiplier));
            }
            break;
        }
        case _Multilayer_r:{
            data->c = loc.toDouble(mui->lineEdit_4_2->text(), &ok)*myOpt->dwLengthMultiplier;
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
            break;
        }
        case _FerrToroid:{
            data->mu = loc.toDouble(mui->lineEdit_4_2->text(), &ok);
            if (!ok) showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
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
                mui->lineEdit_6_2->setText( loc.toString(k_m/myOpt->dwLengthMultiplier));
            }
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
                if (mui->lineEdit_freq->text().isEmpty()||(mui->lineEdit_3->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5;
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
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((I == 0)||(D == 0)||(d == 0)||(k == 0)||(f == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (k < d){
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
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, d, k, I, f, 0, myOpt->dwAccuracy, mt );
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
                if (!myOpt->isAWG) mui->lineEdit_2->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                mui->lineEdit_3->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                mui->lineEdit_4->setText(loc.toString(p / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, d, p, I, f, 0, myOpt->dwAccuracy, mt );
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
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, w, t, p, I, f, myOpt->dwAccuracy, mt );
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
                if (!myOpt->isAWG) mui->lineEdit_2->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                mui->lineEdit_3->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                mui->lineEdit_4->setText(loc.toString(p / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, Dk, d, p, f, _n, 0, mt );
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
                data->inductance = I;
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_1->setText(loc.toString(D / myOpt->dwLengthMultiplier));
                mui->lineEdit_2->setText(loc.toString(l / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_3->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                mui->lineEdit_4->setText(loc.toString(k / myOpt->dwLengthMultiplier));
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
                if (!myOpt->isAWG) mui->lineEdit_3->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                mui->lineEdit_4->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                mui->lineEdit_5->setText(loc.toString(gap / myOpt->dwLengthMultiplier));
                mui->lineEdit_6->setText(loc.toString(ng / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, D, d, k, l, gap, ng );
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
                if (!myOpt->isAWG) mui->lineEdit_4->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                mui->lineEdit_5->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, a, b, l, d, k, 0 );
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
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, w, t, ins, I, 0, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerN_Foil_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _FerrToroid:{
                if ((mui->lineEdit_3->text().isEmpty())||(mui->lineEdit_4->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5, ok6;
                double I = loc.toDouble(mui->lineEdit_ind->text(),&ok1)*myOpt->dwInductanceMultiplier;
                double D1 = loc.toDouble(mui->lineEdit_1->text(),&ok2)*myOpt->dwLengthMultiplier;
                double D2 = loc.toDouble(mui->lineEdit_2->text(),&ok3)*myOpt->dwLengthMultiplier;
                double h = loc.toDouble(mui->lineEdit_3->text(),&ok4)*myOpt->dwLengthMultiplier;
                double d = 0;
                if (myOpt->isAWG){
                    d = convertfromAWG(mui->lineEdit_4->text(),&ok5);
                } else {
                    d = loc.toDouble(mui->lineEdit_4->text(),&ok5)*myOpt->dwLengthMultiplier;
                }
                double mu = loc.toDouble(mui->lineEdit_5->text(),&ok6);
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((I == 0)||(D1 == 0)||(D2 == 0)||(h == 0)||(d == 0)||(mu == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (D1 < D2){
                    showWarning(tr("Warning"), "D1 < D2");
                    return;
                }
                data->inductance = I;
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_1->setText(loc.toString(D1 / myOpt->dwLengthMultiplier));
                mui->lineEdit_2->setText(loc.toString(D2 / myOpt->dwLengthMultiplier));
                mui->lineEdit_3->setText(loc.toString(h / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_4->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                mui->lineEdit_5->setText(loc.toString(mu));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, D1, D2, h, d, mu, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_ferrToroidN_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _PCB_coil:{
                bool ok1, ok2, ok3;
                double I = loc.toDouble(mui->lineEdit_ind->text(),&ok1)*myOpt->dwInductanceMultiplier;
                double D1 = loc.toDouble(mui->lineEdit_1->text(),&ok2)*myOpt->dwLengthMultiplier;
                double D2 = loc.toDouble(mui->lineEdit_2->text(),&ok3)*myOpt->dwLengthMultiplier;
                double ratio = (double)mui->horizontalSlider->value()/100;
                int isPCBSquare = mui->comboBox_checkPCB->currentIndex();
                if((!ok1)||(!ok2)||(!ok3)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((I == 0)||(D1 == 0)||(D2 == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (D1 < D2){
                    showWarning(tr("Warning"), "D1 < D2");
                    return;
                }
                data->inductance = I;
                mui->lineEdit_ind->setText(loc.toString(data->inductance / myOpt->dwInductanceMultiplier));
                mui->lineEdit_1->setText(loc.toString(D1 / myOpt->dwLengthMultiplier));
                mui->lineEdit_2->setText(loc.toString(D2 / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, I, D1, D2, ratio, isPCBSquare, 0, 0 );
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
                if (!myOpt->isAWG) mui->lineEdit_2->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                mui->lineEdit_3->setText(loc.toString(s / myOpt->dwLengthMultiplier));
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
            case _Onelayer_cw:{
                if (mui->lineEdit_N->text().isEmpty()||(mui->lineEdit_freq2->text().isEmpty())){
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
                if (!myOpt->isAWG) mui->lineEdit_2_2->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                mui->lineEdit_3_2->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, d, k, N, f, 0, myOpt->dwAccuracy, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerI_roundW_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Onelayer:{
                if (mui->lineEdit_N->text().isEmpty()||(mui->lineEdit_freq2->text().isEmpty())||(mui->lineEdit_4_2->text().isEmpty())){
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
                if (!myOpt->isAWG) mui->lineEdit_2_2->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                mui->lineEdit_3_2->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                mui->lineEdit_4_2->setText(loc.toString(p / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, d, p, N, f, 0, myOpt->dwAccuracy, mt );
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
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, w, t, p, N, f, 0, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerI_rectW_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Onelayer_q:{
                if (mui->lineEdit_N->text().isEmpty()||(mui->lineEdit_freq2->text().isEmpty())||(mui->lineEdit_4_2->text().isEmpty())
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
                if (!myOpt->isAWG) mui->lineEdit_2_2->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                mui->lineEdit_3_2->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                mui->lineEdit_4_2->setText(loc.toString(p / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, Dk, d, p, N, f, _n, myOpt->dwAccuracy, mt );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_onelayerI_Poligonal_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Multilayer:{
                if ((mui->lineEdit_4_2->text().isEmpty())||(mui->lineEdit_5_2->text().isEmpty())){
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
                if (mui->radioButton_6->isChecked()){
                    MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, l, N, d, k, 0, 0 );
                    connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerI_Result(_CoilResult)));
                    thread->start();
                } else if (mui->radioButton_7->isChecked()){
                    MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, l, c, d, k, 0, 1 );
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
                if (!myOpt->isAWG) mui->lineEdit_4_2->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                mui->lineEdit_5_2->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                mui->lineEdit_6_2->setText(loc.toString(g / myOpt->dwLengthMultiplier));
                mui->lineEdit_7_2->setText(loc.toString(Ng));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, l, c, d, k, g, Ng );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerIgap_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Multilayer_r:{
                if ((mui->lineEdit_4_2->text().isEmpty())||(mui->lineEdit_5_2->text().isEmpty())||(mui->lineEdit_6_2->text().isEmpty())){
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
                if (!myOpt->isAWG) mui->lineEdit_5_2->setText(loc.toString(d / myOpt->dwLengthMultiplier));
                mui->lineEdit_6_2->setText(loc.toString(k / myOpt->dwLengthMultiplier));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, a, b, l, c, d, k, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerI_Rect_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Multilayer_f:{
                if ((mui->lineEdit_N->text().isEmpty()) ||(mui->lineEdit_4_2->text().isEmpty())){
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
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, D, w, t, ins, N, 0, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_multilayerI_Foil_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _FerrToroid:{
                if (mui->lineEdit_N->text().isEmpty()||(mui->lineEdit_4_2->text().isEmpty())){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5;
                double N = loc.toDouble(mui->lineEdit_N->text(), &ok1);
                double D1 = loc.toDouble(mui->lineEdit_1_2->text(), &ok2)*myOpt->dwLengthMultiplier;
                double D2 = loc.toDouble(mui->lineEdit_2_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                double h = loc.toDouble(mui->lineEdit_3_2->text(), &ok4)*myOpt->dwLengthMultiplier;
                double mu = loc.toDouble(mui->lineEdit_4_2->text(), &ok5);
                if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if ((N == 0)||(D1 == 0)||(D2 == 0)||(h == 0)||(mu == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (D1 < D2){
                    showWarning(tr("Warning"), "D1 < D2");
                    return;
                }
                mui->lineEdit_N->setText(loc.toString(N));
                mui->lineEdit_1_2->setText(loc.toString(D1 / myOpt->dwLengthMultiplier));
                mui->lineEdit_2_2->setText(loc.toString(D2 / myOpt->dwLengthMultiplier));
                mui->lineEdit_3_2->setText(loc.toString(h / myOpt->dwLengthMultiplier));
                mui->lineEdit_4_2->setText(loc.toString(mu));
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, N, D1, D2, h, mu, 0, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_ferriteI_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _PCB_coil:{
                if (mui->lineEdit_N->text().isEmpty()||((mui->lineEdit_4_2->text().isEmpty())&&(myOpt->isPCBcoilSquare))){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4, ok5;
                double N = loc.toDouble(mui->lineEdit_N->text(), &ok1);
                double D1 = loc.toDouble(mui->lineEdit_1_2->text(), &ok2)*myOpt->dwLengthMultiplier;
                double D2 = loc.toDouble(mui->lineEdit_2_2->text(), &ok3)*myOpt->dwLengthMultiplier;
                double s = 0;
                double W = 0;
                if ((N == 0)||(D1 == 0)||(D2 == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if((!ok1)||(!ok2)||(!ok3)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                if (myOpt->isPCBcoilSquare){
                    s = loc.toDouble(mui->lineEdit_3_2->text(), &ok4)*myOpt->dwLengthMultiplier;
                    W = loc.toDouble(mui->lineEdit_4_2->text(), &ok5)*myOpt->dwLengthMultiplier;
                    if((!ok4)||(!ok5)){
                        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                        return;
                    }
                    if ((s == 0)||(W == 0)){
                        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                        return;
                    }
                    if (s <= W){
                        showWarning(tr("Warning"), "s <= W");
                        return;
                    }
                }
                int isPCBSquare = mui->comboBox_checkPCB->currentIndex();
                if (D1 < D2){
                    showWarning(tr("Warning"), "D1 < D2");
                    return;
                }
                mui->lineEdit_N->setText(loc.toString(N));
                mui->lineEdit_1_2->setText(loc.toString(D1 / myOpt->dwLengthMultiplier));
                mui->lineEdit_2_2->setText(loc.toString(D2 / myOpt->dwLengthMultiplier));
                if (myOpt->isPCBcoilSquare){
                    mui->lineEdit_3_2->setText(loc.toString(s / myOpt->dwLengthMultiplier));
                    mui->lineEdit_4_2->setText(loc.toString(W / myOpt->dwLengthMultiplier));
                }
                MThread_calculate *thread= new MThread_calculate( FormCoil, tab, N, D1, D2, s, W, isPCBSquare, 0 );
                connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_pcbI_Result(_CoilResult)));
                thread->start();
                break;
            }
            case _Flat_Spiral:{
                if (mui->lineEdit_N->text().isEmpty()){
                    showWarning(tr("Warning"), tr("One or more inputs are empty!"));
                    return;
                }
                bool ok1, ok2, ok3, ok4;
                double N = loc.toDouble(mui->lineEdit_N->text(), &ok1);
                double D1 = loc.toDouble(mui->lineEdit_1_2->text(), &ok2)*myOpt->dwLengthMultiplier;
                double D2 = loc.toDouble(mui->lineEdit_2_2->text(), &ok3)*myOpt->dwLengthMultiplier;
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
                if ((N == 0)||(D1 == 0)||(D2 == 0)||(d == 0)){
                    showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
                    return;
                }
                if (D1 < D2){
                    showWarning(tr("Warning"), "D1 < D2");
                    return;
                }
                mui->lineEdit_N->setText(loc.toString(N));
                mui->lineEdit_1_2->setText(loc.toString(D1 / myOpt->dwLengthMultiplier));
                mui->lineEdit_2_2->setText(loc.toString(D2 / myOpt->dwLengthMultiplier));
                if (!myOpt->isAWG) mui->lineEdit_3_2->setText(loc.toString(d / myOpt->dwLengthMultiplier));
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
            QString tmpStr = "";
            bool ok1, ok2;
            if (mui->radioButton_LC->isChecked()){
                data->inductance = loc.toDouble(mui->lineEdit_1_3->text(), &ok1)*myOpt->dwInductanceMultiplier;
                data->capacitance = loc.toDouble(mui->lineEdit_2_3->text(), &ok2)*myOpt->dwCapacityMultiplier;
                if((!ok1)||(!ok2)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                data->frequency = CalcLC0(data->inductance, data->capacitance);
                tmpStr += tr("Frequency of a circuit") + " f = " + loc.toString(data->frequency / myOpt->dwFrequencyMultiplier, 'f', myOpt->dwAccuracy)
                        + " " + qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()) + "<br/>";
            } else if (mui->radioButton_CF->isChecked()){
                data->capacitance = loc.toDouble(mui->lineEdit_1_3->text(), &ok1)*myOpt->dwCapacityMultiplier;
                data->frequency = loc.toDouble(mui->lineEdit_2_3->text(), &ok2)*myOpt->dwFrequencyMultiplier;
                if((!ok1)||(!ok2)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                data->inductance = CalcLC1(data->capacitance, data->frequency);
                tmpStr += tr("Inductance of a circuit") + " L = " + loc.toString(data->inductance / myOpt->dwInductanceMultiplier, 'f', myOpt->dwAccuracy)
                        + " " + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()) + "<br/>";
            } else if (mui->radioButton_LF->isChecked()){
                data->inductance = loc.toDouble(mui->lineEdit_1_3->text(), &ok1)*myOpt->dwInductanceMultiplier;
                data->frequency = loc.toDouble(mui->lineEdit_2_3->text(), &ok2)*myOpt->dwFrequencyMultiplier;
                if((!ok1)||(!ok2)){
                    showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
                    return;
                }
                data->capacitance = CalcLC2(data->inductance, data->frequency);
                tmpStr += tr("Circuit capacitance") + " C = " + loc.toString(data->capacitance / myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy)
                        + " " + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            }
            QTextCursor c = mui->textBrowser->textCursor();
            prepareHeader(&c);
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
            Result += tmpStr;
            double ro = 1000 * sqrt(data->inductance / data->capacitance);
            Result += tr("Characteristic impedance") + " ρ = " + loc.toString(ro, 'f', myOpt->dwAccuracy) + " " + tr("Ohm");
            Result += "</p><hr>";
            c.insertHtml(Result);
            if(myOpt->isLastShowingFirst)
                c.movePosition(QTextCursor::Start);
            else
                c.movePosition(QTextCursor::End);
            mui->textBrowser->setTextCursor(c);
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
    prepareHeader(&c);
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        if (FormCoil == _Onelayer_cw)
            Input += "<img src=\":/images/res/Coil1.png\">";
        else
            Input += "<img src=\":/images/res/Coil2.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_ind->text() + " " + mui->lineEdit_ind->text() + " " + mui->label_ind_m->text() + "<br/>";
    Input += mui->label_freq->text() + " " + mui->lineEdit_freq->text() + " " + mui->label_freq_m->text() + "<br/>";
    Input += mui->label_1->text() + " " + mui->lineEdit_1->text() + " " + mui->label_01->text() + "<br/>";
    Input += mui->label_2->text() + " " + mui->lineEdit_2->text() + " " + mui->label_02->text() + "<br/>";
    if (FormCoil == _Onelayer_cw){
        Input += mui->label_3->text() + " " + mui->lineEdit_3->text() + " " + mui->label_03->text() + "</p>";
    } else {
        Input += mui->label_3->text() + " " + mui->lineEdit_3->text() + " " + mui->label_03->text() + "<br/>";
        Input += mui->label_4->text() + " " + mui->lineEdit_4->text() + " " + mui->label_04->text() + "</p>";
    }
    c.insertHtml(Input);
    QString Result = "<hr>";
    double I = loc.toDouble(mui->lineEdit_ind->text())*myOpt->dwInductanceMultiplier;
    double d = 0;
    if (myOpt->isAWG){
        d = convertfromAWG(mui->lineEdit_2->text());
    } else {
        d = loc.toDouble(mui->lineEdit_2->text())*myOpt->dwLengthMultiplier;
    }
    double k = loc.toDouble(mui->lineEdit_3->text())*myOpt->dwLengthMultiplier;
    double p = 0;
    if (FormCoil == _Onelayer_cw)
        p = k;
    else
        p = loc.toDouble(mui->lineEdit_4->text())*myOpt->dwLengthMultiplier;
    double f = loc.toDouble(mui->lineEdit_freq->text())*myOpt->dwFrequencyMultiplier;
    data->N = result.N;
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    Result += tr("Number of turns of the coil") + " N = " + loc.toString(result.N, 'f', myOpt->dwAccuracy) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + loc.toString(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Length of winding") + " l = " + loc.toString( (result.N * p + k)/myOpt->dwLengthMultiplier, 'f', myOpt->dwAccuracy ) + " " +
            qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double mass = 2.225 * M_PI * d * d * result.sec;
    Result += tr("Weight of wire") + " m = " + loc.toString(mass) + " " + tr("g") + "<br/>";
    double reactance = 2 * M_PI * I * f;
    Result += tr("Reactance of the coil") + " X = " + loc.toString(reactance, 'f', myOpt->dwAccuracy) + " " + tr("Ohm");
    Result += "<br/><br/>";
    if (f < 0.7 * result.fourth){
        Result += tr("Self capacitance") + " Cs = " + loc.toString(result.thd/myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
        Result += tr("Coil self-resonance frequency") + " Fsr = " + loc.toString(result.fourth/myOpt->dwFrequencyMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()) + "<br/>";
        double Ql = (double)result.six;
        Result += tr("Coil constructive Q-factor") + " Q = " + QString::number(result.six) + "<br/>";
        double capacitance = CalcLC2(I, f);
        Material mt = Cu;
        checkMaterial1(&mt);
        double Resistivity = mtrl[mt][Rho]*1e6;
        double R = (Resistivity * result.sec * 4) / (M_PI * d * d); // DC resistance of the wire
        Result += tr("Resistance of the coil") + " Rdc = " + loc.toString(R, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "<br/>";
        double ESR = (1e3 * sqrt(I / capacitance)) / Ql;
        Result += tr("Loss resistance") + " ESR = " + loc.toString(ESR, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "</p>";
        if(myOpt->isAdditionalResult){
            Result += "<hr><p>";
            Result += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcLC2(I, f);
            Result += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    loc.toString((data->capacitance - result.thd) / myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            Result += " => " + tr("Characteristic impedance") + ": ρ = " + loc.toString(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            Result += " => "  + tr("Equivalent resistance") + ": Re = " + loc.toString(Re / 1000, 'f', myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * data->frequency / Qs;
            Result += " => " + tr("Bandwidth") + ": 3dBΔf = " + loc.toString(deltaf, 'f', myOpt->dwAccuracy) + tr("kHz");
        }
    } else {
        QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
        mui->statusBar->showMessage(message);
        Result += "<span style=\"color:blue;\">" + message + "</span>";
    }
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerN_rectW_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
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
    double I = loc.toDouble(mui->lineEdit_ind->text())*myOpt->dwInductanceMultiplier;
    double w = loc.toDouble(mui->lineEdit_2->text())*myOpt->dwLengthMultiplier;
    double t = loc.toDouble(mui->lineEdit_3->text())*myOpt->dwLengthMultiplier;
    double p = loc.toDouble(mui->lineEdit_5->text())*myOpt->dwLengthMultiplier;
    double f = loc.toDouble(mui->lineEdit_freq->text())*myOpt->dwFrequencyMultiplier;
    data->N = result.N;
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    Result += tr("Number of turns of the coil") + " N = " + loc.toString(result.N, 'f', myOpt->dwAccuracy) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + loc.toString(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Length of winding") + " l = " + loc.toString( (result.N * p + w)/myOpt->dwLengthMultiplier, 'f', myOpt->dwAccuracy ) + " " +
            qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double mass = 8.9 * w * t * result.sec;
    Result += tr("Weight of wire") + " m = " + loc.toString(mass) + " " + tr("g") + "<br/>";
    double reactance = 2 * M_PI * I * f;
    Result += tr("Reactance of the coil") + " X = " + loc.toString(reactance, 'f', myOpt->dwAccuracy) + " " + tr("Ohm");
    Result += "<br/><br/>";
    if (f < 0.7 * result.fourth){
        Result += tr("Self capacitance") + " Cs = " + loc.toString(result.thd/myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
        Result += tr("Coil self-resonance frequency") + " Fsr = " + loc.toString(result.fourth/myOpt->dwFrequencyMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()) + "<br/>";
        double Ql = (double)result.six;
        Result += tr("Coil constructive Q-factor") + " Q = " + QString::number(result.six) + "<br/>";
        double Capacity = CalcLC2(I, f);
        Material mt = Cu;
        checkMaterial1(&mt);
        double Resistivity = mtrl[mt][Rho]*1e6;
        double R = (Resistivity * result.sec) / (w * t); // DC resistance of the wire
        Result += tr("Resistance of the coil") + " Rdc = " + loc.toString(R, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "<br/>";
        double ESR = (1e3 * sqrt(I / Capacity)) / Ql;
        Result += tr("Loss resistance") + " ESR = " + loc.toString(ESR, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "</p>";
        if(myOpt->isAdditionalResult){
            Result += "<hr><p>";
            Result += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcLC2(I, f);
            Result += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    loc.toString((data->capacitance - result.thd) / myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            Result += " => " + tr("Characteristic impedance") + ": ρ = " + loc.toString(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            Result += " => "  + tr("Equivalent resistance") + ": Re = " + loc.toString(Re / 1000, 'f', myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * data->frequency / Qs;
            Result += " => " + tr("Bandwidth") + ": 3dBΔf = " + loc.toString(deltaf, 'f', myOpt->dwAccuracy) + tr("kHz");
        }
    } else {
        QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
        mui->statusBar->showMessage(message);
        Result += "<span style=\"color:blue;\">" + message + "</span>";
    }
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerN_Poligonal_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil3.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_ind->text() + " " + mui->lineEdit_ind->text() + " " + mui->label_ind_m->text() + "<br/>";
    Input += mui->label_freq->text() + " " + mui->lineEdit_freq->text() + " " + mui->label_freq_m->text() + "<br/>";
    Input += mui->label_1->text() + " " + mui->lineEdit_1->text() + " " + mui->label_01->text() + "<br/>";
    Input += mui->label_2->text() + " " + mui->lineEdit_2->text() + " " + mui->label_02->text() + "<br/>";
    Input += mui->label_3->text() + " " + mui->lineEdit_3->text() + " " + mui->label_03->text() + "<br/>";
    Input += mui->label_4->text() + " " + mui->lineEdit_4->text() + " " + mui->label_04->text() + "<br/>";
    Input += mui->label_5->text() + " " + mui->lineEdit_5->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
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
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    Result += tr("Number of turns of the coil") + " N = " + loc.toString(result.N, 'f', myOpt->dwAccuracy) + "<br/>";
    QString _wire_length = formatLength(result.thd, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + loc.toString(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Length of winding") + " l = " + loc.toString( (result.N * p + k)/myOpt->dwLengthMultiplier, 'f', myOpt->dwAccuracy ) + " " +
            qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double mass = 2.225 * M_PI * d * d * result.thd;
    Result += tr("Weight of wire") + " m = " + loc.toString(mass) + " " + tr("g") + "<br/>";
    double reactance = 2 * M_PI * I * f;
    Result += tr("Reactance of the coil") + " X = " + loc.toString(reactance, 'f', myOpt->dwAccuracy) + " " + tr("Ohm");
    Result += "<br/><br/>";
    if (f < 0.7 * result.five){
        Result += tr("Self capacitance") + " Cs = " + loc.toString(result.fourth/myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
        Result += tr("Coil self-resonance frequency") + " Fsr = " + loc.toString(result.five/myOpt->dwFrequencyMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()) + "<br/>";
        double Ql = (double)result.six;
        Result += tr("Coil constructive Q-factor") + " Q = " + QString::number(result.six) + "<br/>";
        double capacitance = CalcLC2(I, f);
        Material mt = Cu;
        checkMaterial1(&mt);
        double Resistivity = mtrl[mt][Rho]*1e6;
        double R = (Resistivity * result.thd * 4) / (M_PI * d * d); // DC resistance of the wire
        Result += tr("Resistance of the coil") + " Rdc = " + loc.toString(R, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "<br/>";
        double ESR = (1e3 * sqrt(I / capacitance)) / Ql;
        Result += tr("Loss resistance") + " ESR = " + loc.toString(ESR, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "</p>";
        if(myOpt->isAdditionalResult){
            Result += "<hr><p>";
            Result += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcLC2(I, f);
            Result += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    loc.toString((data->capacitance - result.fourth) / myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            Result += " => " + tr("Characteristic impedance") + ": ρ = " + loc.toString(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            Result += " => "  + tr("Equivalent resistance") + ": Re = " + loc.toString(Re / 1000, 'f', myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * data->frequency / Qs;
            Result += " => " + tr("Bandwidth") + ": 3dBΔf = " + loc.toString(deltaf, 'f', myOpt->dwAccuracy) + tr("kHz");
        }
    } else {
        QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
        mui->statusBar->showMessage(message);
        Result += "<span style=\"color:blue;\">" + message + "</span>";
    }
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
    mui->lineEdit_ind->setFocus();
    mui->lineEdit_ind->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerN_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
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
    Result += tr("Thickness of the coil") + " c = " + loc.toString(result.fourth) + " "
            + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double D = loc.toDouble(mui->lineEdit_1->text())*myOpt->dwLengthMultiplier;
    double width = (2 * result.fourth + D) / myOpt->dwLengthMultiplier;
    Result += tr("Dimensions of inductor") + ": " + mui->lineEdit_2->text() + "x" + loc.toString(ceil(width))
            + "x" + loc.toString(ceil(width)) + " " + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    data->Rdc = result.N;
    Result += tr("Resistance of the coil") + " Rdc = " + loc.toString(result.N, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + loc.toString(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double d = 0;
    if (myOpt->isAWG){
        d = convertfromAWG(mui->lineEdit_3->text());
    } else {
        d = loc.toDouble(mui->lineEdit_3->text())*myOpt->dwLengthMultiplier;
    }
    double mass = 2.225 * M_PI * d * d * result.sec;
    Result += tr("Weight of wire") + " m = " + loc.toString(mass) + " " + tr("g") + "<br/>";
    Result += tr("Number of layers") + " Nl = " + loc.toString(result.thd);
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerNgap_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
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
    Result += tr("Thickness of the coil") + " c = " + loc.toString(result.fourth) + " "
            + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double D = loc.toDouble(mui->lineEdit_1->text())*myOpt->dwLengthMultiplier;
    double width = (2 * result.fourth + D) / myOpt->dwLengthMultiplier;
    Result += tr("Dimensions of inductor") + ": " + mui->lineEdit_2->text() + "x" + loc.toString(ceil(width))
            + "x" + loc.toString(ceil(width)) + " " + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    data->Rdc = result.N;
    Result += tr("Resistance of the coil") + " Rdc = " + loc.toString(result.N, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + loc.toString(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double d = 0;
    if (myOpt->isAWG){
        d = convertfromAWG(mui->lineEdit_3->text());
    } else {
        d = loc.toDouble(mui->lineEdit_3->text())*myOpt->dwLengthMultiplier;
    }
    double mass = 2.225 * M_PI * d * d * result.sec;
    Result += tr("Weight of wire") + " m = " + loc.toString(mass) + " " + tr("g") + "<br/>";
    Result += tr("Number of layers") + " Nl = " + loc.toString(result.thd) + "<br/>";
    Result += tr("Number of interlayers") + " Ng = " + loc.toString(result.five);
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerN_Rect_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil4_square.png\">";
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
    data->N = result.N;
    data->c = result.five;
    Result += tr("Number of turns of the coil") + " N = " + loc.toString(result.N) + "<br/>";
    Result += tr("Number of layers") + " Nl = " + loc.toString(result.sec) + "<br/>";
    Result += tr("Thickness of the coil") + " c = " + loc.toString(result.five) + " "
            + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double a = loc.toDouble(mui->lineEdit_1->text())*myOpt->dwLengthMultiplier;
    double width = (2 * result.five + a) / myOpt->dwLengthMultiplier;
    double b = loc.toDouble(mui->lineEdit_2->text())*myOpt->dwLengthMultiplier;
    double height = (2 * result.five + b) / myOpt->dwLengthMultiplier;
    Result += tr("Dimensions of inductor") + ": " + mui->lineEdit_3->text() + "x" + loc.toString(ceil(width))
            + "x" + loc.toString(ceil(height)) + " " + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    QString _wire_length = formatLength(result.thd, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + loc.toString(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double d = 0;
    if (myOpt->isAWG){
        d = convertfromAWG(mui->lineEdit_4->text());
    } else {
        d = loc.toDouble(mui->lineEdit_4->text())*myOpt->dwLengthMultiplier;
    }
    double mass = 2.225 * M_PI * d * d * result.thd;
    Result += tr("Weight of wire") + " m = " + loc.toString(mass) + " " + tr("g") + "<br/>";
    Result += tr("Resistance of the coil") + " Rdc = " + loc.toString(result.fourth, 'f', myOpt->dwAccuracy) + " " + tr("Ohm");
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerN_Foil_Result(_CoilResult result)
{
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil11.png\">";
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
    data->N = result.N;
    Result += tr("Number of turns of the coil") + " N = " + loc.toString(result.N) + "<br/>";
    Result += tr("Outside diameter") + " Do = " + loc.toString(result.thd, 'f', myOpt->dwAccuracy) + " "
            + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    QString _foil_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _foil_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_foil_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of the foil") + " lf = " + loc.toString(d_foil_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Resistance of the coil") + " Rdc = " + loc.toString(result.fourth, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + " (" + tr("Copper") + ")<br/>";
    Result += tr("Resistance of the coil") + " Rdc = " + loc.toString(result.five, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + " (" + tr("Aluminum") + ")";
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_ferrToroidN_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
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
        Result += tr("Length of wire without leads") + " lw = " + loc.toString(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
                qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
        data->N = result.N;
        Result += tr("Number of turns of the coil") + " N = " + loc.toString(result.N, 'f', myOpt->dwAccuracy);
    } else {
        Result += "<span style=\"color:red;\">" + tr("Coil can not be realized") + "! </span>";
        mui->statusBar->showMessage(tr("Coil can not be realized") + "!");
    }
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_pcbN_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2>";
    Input += "<h3>" + mui->comboBox_checkPCB->currentText() + "</h3><br/>";
    if (myOpt->isInsertImage){
        if (myOpt->isPCBcoilSquare)
            Input += "<img src=\":/images/res/Coil8.png\">";
        else Input += "<img src=\":/images/res/Coil9.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_ind->text() + " " + mui->lineEdit_ind->text() + " " + mui->label_ind_m->text() + "<br/>";
    Input += mui->label_1->text() + " " + mui->lineEdit_1->text() + " " + mui->label_01->text() + "<br/>";
    Input += mui->label_2->text() + " " + mui->lineEdit_2->text() + " " + mui->label_02->text() + "<br/>";
    Input += mui->groupBox_6->title() + " " + loc.toString((double)mui->horizontalSlider->value()/100) + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    data->N = result.N;
    Result += tr("Number of turns of the coil") + " N = " + loc.toString(result.N, 'f', myOpt->dwAccuracy) + "<br/>";
    Result += tr("Winding pitch") + " s = " + loc.toString(result.sec, 'f', myOpt->dwAccuracy) + " " + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Width of a PCB trace") + " W = " + loc.toString(result.thd, 'f', myOpt->dwAccuracy) + " " + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8());
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_spiralN_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
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
    Result += tr("Number of turns of the coil") + " N = " + loc.toString(result.N) + "<br/>";
    Result += tr("Outside diameter") + " Do = " + loc.toString(result.thd, 'f', myOpt->dwAccuracy) + " "
            + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + loc.toString(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "</p>";
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GET INDUCTANCE ROUTINES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerI_roundW_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        if (FormCoil == _Onelayer_cw)
            Input += "<img src=\":/images/res/Coil1.png\">";
        else
            Input += "<img src=\":/images/res/Coil2.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_N->text() + " " + mui->lineEdit_N->text() + "<br/>";
    Input += mui->label_freq2->text() + " " + mui->lineEdit_freq2->text() + " " + mui->label_freq_m2->text() + "<br/>";
    Input += mui->label_1_2->text() + " " + mui->lineEdit_1_2->text() + " " + mui->label_01_2->text() + "<br/>";
    Input += mui->label_2_2->text() + " " + mui->lineEdit_2_2->text() + " " + mui->label_02_2->text() + "<br/>";
    if (FormCoil == _Onelayer_cw)
        Input += mui->label_3_2->text() + " " + mui->lineEdit_3_2->text() + " " + mui->label_03_2->text() + "/p>";
    else {
        Input += mui->label_3_2->text() + " " + mui->lineEdit_3_2->text() + " " + mui->label_03_2->text() + "<br/>";
        Input += mui->label_4_2->text() + " " + mui->lineEdit_4_2->text() + " " + mui->label_04_2->text() + "</p>";
    }
    c.insertHtml(Input);
    QString Result = "<hr>";
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
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    double I = result.N;
    data->inductance = result.N;
    Result += tr("Inductance") + " L = " + loc.toString(I/myOpt->dwInductanceMultiplier, 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + loc.toString(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context * 4",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Length of winding") + " l = " + loc.toString( (N * p + k)/myOpt->dwLengthMultiplier, 'f', myOpt->dwAccuracy ) + " " +
            qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double mass = 2.225 * M_PI * d * d * result.sec;
    Result += tr("Weight of wire") + " m = " + loc.toString(mass) + " " + tr("g") + "<br/>";
    double reactance = 2 * M_PI * I * f;
    Result += tr("Reactance of the coil") + " X = " + loc.toString(reactance, 'f', myOpt->dwAccuracy) + " " + tr("Ohm");
    Result += "<br/><br/>";
    if (f < 0.7 * result.fourth){
        Result += tr("Self capacitance") + " Cs = " + loc.toString(result.thd/myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
        Result += tr("Coil self-resonance frequency") + " Fsr = " + loc.toString(result.fourth/myOpt->dwFrequencyMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()) + "<br/>";
        double Ql = (double)result.six;
        Result += tr("Coil constructive Q-factor") + " Q = " + QString::number(result.six) + "<br/>";
        double Capacity = CalcLC2(I, f);
        Material mt = Cu;
        checkMaterial2(&mt);
        double Resistivity = mtrl[mt][Rho]*1e6;
        double R = (Resistivity * result.sec * 4) / (M_PI * d * d); // DC resistance of the wire
        Result += tr("Resistance of the coil") + " Rdc = " + loc.toString(R, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "<br/>";
        double ESR = (1e3 * sqrt(I / Capacity)) / Ql;
        Result += tr("Loss resistance") + " ESR = " + loc.toString(ESR, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "</p>";
        if(myOpt->isAdditionalResult){
            Result += "<hr><p>";
            Result += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcLC2(I, f);
            Result += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    loc.toString((data->capacitance - result.thd) / myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            Result += " => " + tr("Characteristic impedance") + ": ρ = " + loc.toString(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            Result += " => "  + tr("Equivalent resistance") + ": Re = " + loc.toString(Re / 1000, 'f', myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * f / Qs;
            Result += " => " + tr("Bandwidth") + ": 3dBΔf = " + loc.toString(deltaf, 'f', myOpt->dwAccuracy) + tr("kHz");
        }
    } else {
        QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
        mui->statusBar->showMessage(message);
        Result += "<span style=\"color:blue;\">" + message + "</span>";
    }
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerI_rectW_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
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
    double N = loc.toDouble(mui->lineEdit_N->text());
    double w = loc.toDouble(mui->lineEdit_2_2->text())*myOpt->dwLengthMultiplier;
    double t = loc.toDouble(mui->lineEdit_3_2->text())*myOpt->dwLengthMultiplier;
    double p = loc.toDouble(mui->lineEdit_5_2->text())*myOpt->dwLengthMultiplier;
    double f = loc.toDouble(mui->lineEdit_freq2->text())*myOpt->dwFrequencyMultiplier;
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    double I = result.N;
    data->inductance = result.N;
    Result += tr("Inductance") + " L = " + loc.toString(I/myOpt->dwInductanceMultiplier, 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + loc.toString(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Length of winding") + " l = " + loc.toString( (N * p + w)/myOpt->dwLengthMultiplier, 'f', myOpt->dwAccuracy ) + " " +
            qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double mass = 8.9 * w * t * result.sec;
    Result += tr("Weight of wire") + " m = " + loc.toString(mass) + " " + tr("g") + "<br/>";
    double reactance = 2 * M_PI * I * f;
    Result += tr("Reactance of the coil") + " X = " + loc.toString(reactance, 'f', myOpt->dwAccuracy) + " " + tr("Ohm");
    Result += "<br/><br/>";
    if (f < 0.7 * result.fourth){
        Result += tr("Self capacitance") + " Cs = " + loc.toString(result.thd/myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
        Result += tr("Coil self-resonance frequency") + " Fsr = " + loc.toString(result.fourth/myOpt->dwFrequencyMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()) + "<br/>";
        double Ql = (double)result.six;
        Result += tr("Coil constructive Q-factor") + " Q = " + QString::number(result.six) + "<br/>";
        double Capacity = CalcLC2(I, f);
        Material mt = Cu;
        checkMaterial2(&mt);
        double Resistivity = mtrl[mt][Rho]*1e6;
        double R = (Resistivity * result.sec) / (w * t); // DC resistance of the wire
        Result += tr("Resistance of the coil") + " Rdc = " + loc.toString(R, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "<br/>";
        double ESR = (1e3 * sqrt(I / Capacity)) / Ql;
        Result += tr("Loss resistance") + " ESR = " + loc.toString(ESR, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "</p>";
        if(myOpt->isAdditionalResult){
            Result += "<hr><p>";
            Result += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcLC2(I, f);
            Result += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    loc.toString((data->capacitance - result.thd) / myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            Result += " => " + tr("Characteristic impedance") + ": ρ = " + loc.toString(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);// Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            Result += " => "  + tr("Equivalent resistance") + ": Re = " + loc.toString(Re / 1000, 'f', myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * data->frequency / Qs;
            Result += " => " + tr("Bandwidth") + ": 3dBΔf = " + loc.toString(deltaf, 'f', myOpt->dwAccuracy) + tr("kHz");
        }
    } else {
        QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
        mui->statusBar->showMessage(message);
        Result += "<span style=\"color:blue;\">" + message + "</span>";
    }
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_onelayerI_Poligonal_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil3.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_N->text() + " " + mui->lineEdit_N->text() + "<br/>";
    Input += mui->label_freq2->text() + " " + mui->lineEdit_freq2->text() + " " + mui->label_freq_m2->text() + "<br/>";
    Input += mui->label_1_2->text() + " " + mui->lineEdit_1_2->text() + " " + mui->label_01_2->text() + "<br/>";
    Input += mui->label_2_2->text() + " " + mui->lineEdit_2_2->text() + " " + mui->label_02_2->text() + "<br/>";
    Input += mui->label_3_2->text() + " " + mui->lineEdit_3_2->text() + " " + mui->label_03_2->text() + "<br/>";
    Input += mui->label_4_2->text() + " " + mui->lineEdit_4_2->text() + " " + mui->label_04_2->text() + "<br/>";
    Input += mui->label_5_2->text() + " " + mui->lineEdit_5_2->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
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
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    double I = result.sec;
    data->inductance = result.sec;
    Result += tr("Inductance") + " L = " + loc.toString(I/myOpt->dwInductanceMultiplier, 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()) + "<br/>";
    QString _wire_length = formatLength(result.thd, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + loc.toString(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Length of winding") + " l = " + loc.toString( (N * p + k)/myOpt->dwLengthMultiplier, 'f', myOpt->dwAccuracy ) + " " +
            qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    double mass = 2.225 * M_PI * d * d * result.thd;
    Result += tr("Weight of wire") + " m = " + loc.toString(mass) + " " + tr("g") + "<br/>";
    double reactance = 2 * M_PI * I * f;
    Result += tr("Reactance of the coil") + " X = " + loc.toString(reactance, 'f', myOpt->dwAccuracy) + " " + tr("Ohm");
    Result += "<br/><br/>";
    if (f < 0.7 * result.five){
        Result += tr("Self capacitance") + " Cs = " + loc.toString(result.fourth/myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
        Result += tr("Coil self-resonance frequency") + " Fsr = " + loc.toString(result.five/myOpt->dwFrequencyMultiplier, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssFrequencyMeasureUnit.toUtf8()) + "<br/>";
        double Ql = (double)result.six;
        Result += tr("Coil constructive Q-factor") + " Q = " + QString::number(result.six) + "<br/>";
        double Capacity = CalcLC2(I, f);
        Material mt = Cu;
        checkMaterial2(&mt);
        double Resistivity = mtrl[mt][Rho]*1e6;
        double R = (Resistivity * result.thd * 4) / (M_PI * d * d); // DC resistance of the wire
        Result += tr("Resistance of the coil") + " Rdc = " + loc.toString(R, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "<br/>";
        double ESR = (1e3 * sqrt(I / Capacity)) / Ql;
        Result += tr("Loss resistance") + " ESR = " + loc.toString(ESR, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + "</p>";
        if(myOpt->isAdditionalResult){
            Result += "<hr><p>";
            Result += "<u>" + tr("Additional results for parallel LC circuit at the working frequency") + ":</u><br/>";
            data->capacitance = CalcLC2(I, f);
            Result += " => "  + tr("Circuit capacitance") + ": Ck = " +
                    loc.toString((data->capacitance - result.fourth) / myOpt->dwCapacityMultiplier, 'f', myOpt->dwAccuracy) + ' '
                    + qApp->translate("Context", myOpt->ssCapacityMeasureUnit.toUtf8()) + "<br/>";
            double ro = 1000 * sqrt(I / data->capacitance);
            Result += " => " + tr("Characteristic impedance") + ": ρ = " + loc.toString(round(ro)) + " " + tr("Ohm") + "<br/>";
            double Qs= 1 / (0.001 + 1 / Ql);  //Complete LC Q-factor including capacitor Q-factor equal to 1000
            double Re = ro * Qs;
            Result += " => "  + tr("Equivalent resistance") + ": Re = " + loc.toString(Re / 1000, 'f', myOpt->dwAccuracy) + " " + tr("kOhm") + "<br/>";
            double deltaf = 1000 * f / Qs;
            Result += " => " + tr("Bandwidth") + ": 3dBΔf = " + loc.toString(deltaf, 'f', myOpt->dwAccuracy) + tr("kHz");
        }
    } else {
        QString message = tr("Working frequency") + " > 0.7 * " + tr("Coil self-resonance frequency") + "!";
        mui->statusBar->showMessage(message);
        Result += "<span style=\"color:blue;\">" + message + "</span>";
    }
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerI_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
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
    if (mui->radioButton_6->isChecked()){
        Result += tr("Inductance") + " L = " + loc.toString(result.N, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8());
        data->inductance = result.N;
    } else {
        double N1 = result.thd;
        double N2 = result.fourth;
        double L1 = result.N;
        double L2 = result.sec;
        data->inductance = sqrt(L1*L2);
        Result += tr("Number of turns of the coil") + " N = " + loc.toString(N1) + "..." + loc.toString(N2) + "<br/>";
        Result += tr("Inductance") + " L = " + loc.toString(L1, 'f', myOpt->dwAccuracy) + "..." + loc.toString(L2, 'f', myOpt->dwAccuracy) + " "
                + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8());
    }
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerIgap_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
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
    Result += tr("Number of turns of the coil") + " N = " + loc.toString(N1) + "..." + loc.toString(N2) + "<br/>";
    Result += tr("Inductance") + " L = " + loc.toString(L1, 'f', myOpt->dwAccuracy) + "..." + loc.toString(L2, 'f', myOpt->dwAccuracy) + " "
            + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8());
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerI_Rect_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil4_square.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_1_2->text() + " " + mui->lineEdit_1_2->text() + " " + mui->label_01_2->text() + "<br/>";
    Input += mui->label_2_2->text() + " " + mui->lineEdit_2_2->text() + " " + mui->label_02_2->text() + "<br/>";
    Input += mui->label_3_2->text() + " " + mui->lineEdit_3_2->text() + " " + mui->label_03_2->text() + "<br/>";
    Input += mui->label_4_2->text() + " " + mui->lineEdit_4_2->text() + " " + mui->label_04_2->text() + "<br/>";
    Input += mui->label_5_2->text() + " " + mui->lineEdit_5_2->text() + " " + mui->label_05_2->text() + "<br/>";
    Input += mui->label_6_2->text() + " " + mui->lineEdit_6_2->text() + " " + mui->label_06_2->text() + "</p>";
    c.insertHtml(Input);
    QString Result = "<hr>";
    double N1 = result.thd;
    double N2 = result.fourth;
    double L1 = result.N;
    double L2 = result.sec;
    data->inductance = sqrt(L1*L2);
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    Result += tr("Number of turns of the coil") + " N = " + loc.toString(N1) + "..." + loc.toString(N2) + "<br/>";
    Result += tr("Inductance") + " L = " + loc.toString(L1, 'f', myOpt->dwAccuracy) + "..." + loc.toString(L2, 'f', myOpt->dwAccuracy) + " "
            + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8());
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_multilayerI_Foil_Result(_CoilResult result)
{
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2><br/>";
    if (myOpt->isInsertImage){
        Input += "<img src=\":/images/res/Coil11.png\">";
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
    Result += tr("Inductance") + " L = " + loc.toString(result.N, 'f', myOpt->dwAccuracy) + " "
            + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()) + "<br/>";
    QString _foil_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _foil_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_foil_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of the foil") + " lf = " + loc.toString(d_foil_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Outside diameter") + " Do = " + loc.toString(result.thd, 'f', myOpt->dwAccuracy) + " "
            + qApp->translate("Context", myOpt->ssLengthMeasureUnit.toUtf8()) + "<br/>";
    Result += tr("Resistance of the coil") + " Rdc = " + loc.toString(result.fourth, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + " (" + tr("Copper") + ")<br/>";
    Result += tr("Resistance of the coil") + " Rdc = " + loc.toString(result.five, 'f', myOpt->dwAccuracy) + " " + tr("Ohm") + " (" + tr("Aluminum") + ")";
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_ferriteI_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
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
    Result += tr("Inductance") + " L = " + loc.toString(result.N, 'f', myOpt->dwAccuracy) + " " + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8());
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_pcbI_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
    QString Input = "<hr><h2>" + windowTitle() + " - " + mui->listWidget->currentItem()->text() + "</h2>";
    Input += "<h3>" + mui->comboBox_checkPCB->currentText() + "</h3><br/>";
    if (myOpt->isInsertImage){
        if (myOpt->isPCBcoilSquare)
            Input += "<img src=\":/images/res/Coil8.png\">";
        else Input += "<img src=\":/images/res/Coil9.png\">";
    }
    Input += "<p><u>" + tr("Input") + ":</u><br/>";
    Input += mui->label_N->text() + " " + mui->lineEdit_N->text() + "<br/>";
    Input += mui->label_1_2->text() + " " + mui->lineEdit_1_2->text() + " " + mui->label_01_2->text() + "<br/>";
    Input += mui->label_2_2->text() + " " + mui->lineEdit_2_2->text() + " " + mui->label_02_2->text() + "<br/>";
    if (myOpt->isPCBcoilSquare){
        Input += mui->label_3_2->text() + " " + mui->lineEdit_3_2->text() + " " + mui->label_03_2->text() + "<br/>";
        Input += mui->label_4_2->text() + " " + mui->lineEdit_4_2->text() + " " + mui->label_04_2->text() + "</p>";
    }
    c.insertHtml(Input);
    QString Result = "<hr>";
    Result += "<p><u>" + tr("Result") + ":</u><br/>";
    data->inductance = result.N;
    Result += tr("Inductance") + " L = " + loc.toString(result.N, 'f', myOpt->dwAccuracy) + " " + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8());
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::get_spiralI_Result(_CoilResult result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
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
    Result += tr("Inductance") + " L = " + loc.toString(result.N, 'f', myOpt->dwAccuracy) + " "
            + qApp->translate("Context", myOpt->ssInductanceMeasureUnit.toUtf8()) + "<br/>";
    QString _wire_length = formatLength(result.sec, myOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), QString::SkipEmptyParts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    Result += tr("Length of wire without leads") + " lw = " + loc.toString(d_wire_length.toDouble(), 'f', myOpt->dwAccuracy) + " " +
            qApp->translate("Context",_ssLengthMeasureUnit.toUtf8());
    Result += "</p><hr>";
    c.insertHtml(Result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
    mui->pushButton_Calculate->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
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
void MainWindow::getAddCalculationResult(QString result){
    QTextCursor c = mui->textBrowser->textCursor();
    prepareHeader(&c);
    c.insertHtml(result);
    if(myOpt->isLastShowingFirst)
        c.movePosition(QTextCursor::Start);
    else
        c.movePosition(QTextCursor::End);
    mui->textBrowser->setTextCursor(c);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_comboBox_checkPCB_activated(int index)
{
    if (index == 0){
        mui->image->setPixmap(QPixmap(":/images/res/Coil8.png"));
        myOpt->isPCBcoilSquare = true;
        mui->lineEdit_3_2->setVisible(true);
        mui->label_3_2->setVisible(true);
        mui->label_03_2->setVisible(true);
        mui->lineEdit_4_2->setVisible(true);
        mui->label_4_2->setVisible(true);
        mui->label_04_2->setVisible(true);
        data->Di = data->Do * 0.362;
    }
    if (index == 1) {
        mui->image->setPixmap(QPixmap(":/images/res/Coil9.png"));
        myOpt->isPCBcoilSquare = false;
        mui->lineEdit_3_2->setVisible(false);
        mui->label_3_2->setVisible(false);
        mui->label_03_2->setVisible(false);
        mui->lineEdit_4_2->setVisible(false);
        mui->label_4_2->setVisible(false);
        mui->label_04_2->setVisible(false);
        data->Di = data->Do * 0.4;
    }
    mui->lineEdit_2->setText(loc.toString(data->Di));
    if (data->Di > 0){
        mui->lineEdit_2->setFocus();
        mui->lineEdit_2->selectAll();
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
            cursor.setPosition(start + 2, QTextCursor::KeepAnchor);
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
        int i = start;
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
        cursor.setPosition(start - 2, QTextCursor::MoveAnchor);
        cursor.setPosition(i - 1, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.clearSelection();
        mui->textBrowser->setTextCursor(cursor);
        if ((mui->textBrowser->document()->lineCount() < 4) && (!mui->textBrowser->document()->isEmpty())){
                mui->textBrowser->clear();
                mui->statusBar->clearMessage();
        }
    }
}
