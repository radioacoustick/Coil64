/* crossover.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "crossover.h"
#include "ui_crossover.h"


int mWire_size = 58;
int maxAwgSize = 1;
double mWire[] = {0.2,0.21,0.224,0.236,0.25,0.265,0.28,0.3,0.315,0.355,
                  0.38,0.4,0.425,0.45,0.475,0.5,0.53,0.56,0.6,0.63,
                  0.67,0.71,0.75,0.8,0.85,0.9,0.93,0.95,1.0,1.06,
                  1.08,1.12,1.18,1.25,1.32,1.4,1.45,1.5,1.56,1.6,
                  1.7,1.74,1.8,1.9,2.0,2.12,2.24,2.36,2.5,2.9,
                  3.26, 3.66, 4.11, 4.62,5.19, 5.83, 6.54,7.35};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Crossover::Crossover(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Crossover)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, MAX_DOUBLE, 380);
    ui->lineEdit_ind->setValidator(dv);
    ui->comboBox_N_m->addItem(qApp->translate("Context","microH"));
    ui->comboBox_N_m->addItem(qApp->translate("Context","mH"));
    ui->label_3->setText(tr("Axial") + " pa/d:");
    ui->label_4->setText(tr("Radial") + " pr/d:");
    model = new QStandardItemModel;
    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter | (Qt::Alignment) Qt::TextWordWrap);
    //Table columns headers
    QStringList horizontalHeader;
    horizontalHeader.append(tr("Wire diameter") + "\nd");
    horizontalHeader.append(tr("Number of turns of the coil"));
    horizontalHeader.append(tr("Number of layers"));
    horizontalHeader.append(tr("Number of turns per layer"));
    horizontalHeader.append(tr("Dimensions of inductor") + "\nDxHxW");
    horizontalHeader.append(tr("Length of wire without leads"));
    horizontalHeader.append(tr("Weight of wire") + "\n[" + tr("g") + "]");
    horizontalHeader.append(tr("Resistance of the coil") + "\n[" + tr("Ohm") + "]");

    for (int k = 0; k < horizontalHeader.count(); k++){
        QString tStr = horizontalHeader[k];
        QStringList  words = tStr.split(" ");
        int wNumbers = words.count();
        if (wNumbers > 1){
            words.insert(wNumbers, "\n");
            tStr = words.join(" ");
        }
        horizontalHeader[k] = tStr;
    }
    model->setHorizontalHeaderLabels(horizontalHeader);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, SIGNAL(customContextMenuRequested( const QPoint& )),
            this, SLOT(on_tableView_customContextMenuRequested( const QPoint& )));

    ui->tableView->setModel(model);
    ui->tableView->resizeRowsToContents();
    ui->tableView->resizeColumnsToContents();
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    QHeaderView *tHeader = ui->tableView->horizontalHeader();
    tHeader->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tHeader, SIGNAL(customContextMenuRequested( const QPoint& )),
            this, SLOT(tableHeaderCustomContextMenuRequested( const QPoint& )));
    tHeader->setToolTip(tr("Click right mouse button to open menu"));
    headerpopupmenu=new QMenu(this);
    for (int i = 0; i < horizontalHeader.count(); i ++){
        QAction *action = headerpopupmenu->addAction(horizontalHeader.at(i));
        action->setCheckable(true);
        if (i == 0)
            action->setEnabled(false);
    }
    tablepopupmenu = new QMenu(this);
    QAction *action0 = new QAction(tr("Remove row"), this);
    action0->setIcon(QPixmap(":/menu_ico/res/ico2-delete.png"));
    tablepopupmenu->addAction(action0);
    connect(action0, SIGNAL(triggered()), this, SLOT(on_removeRow()));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Crossover::~Crossover()
{
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Crossover" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", this->size());
    settings->setValue("indUnit", ui->comboBox_N_m->currentIndex());
    settings->setValue("wmin", ui->comboBox->currentIndex());
    settings->setValue("wmax", ui->comboBox_2->currentIndex());
    settings->setValue("ind", ind);
    settings->setValue("pa", ui->doubleSpinBox_ax->value());
    settings->setValue("pr", ui->doubleSpinBox_rad->value());
    int chk = header_bits.to_ulong();
    settings->setValue("check", chk);
    settings->endGroup();
    delete settings;
    delete fOpt;
    delete dv;
    delete ui;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::getOpt(_OptionStruct gOpt)
{
    *fOpt = gOpt;
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Crossover" );
    ind = settings->value("ind", 0).toDouble();
    int wmin = settings->value("wmin", 0).toInt();
    int wmax = settings->value("wmax", 0).toInt();
    int indUnit = settings->value("indUnit", 0).toInt();
    double pa = settings->value("pa", 1).toDouble();
    double pr = settings->value("pr", 0.87).toDouble();
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    int chk = settings->value("check", 255).toInt();
    header_bits = chk;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    resize(size);
    move(pos);
    ui->comboBox->clear();
    if (fOpt->isAWG){
        for (int i = 32; i > maxAwgSize; i--)
            ui->comboBox->addItem(QString::number(i));
        ui->label_wmax_unit->setText(tr("AWG"));
        ui->label_wmin_unit->setText(tr("AWG"));
    } else {
        for (int j = 0; j < mWire_size - 1; j++)
            ui->comboBox->addItem(QString::number(mWire[j]));
        ui->label_wmax_unit->setText(tr("mm"));
        ui->label_wmin_unit->setText(tr("mm"));
    }
    ui->comboBox_N_m->setCurrentIndex(indUnit);
    if (ui ->comboBox_N_m->currentIndex() == 0)
        ui->lineEdit_ind->setText(loc.toString(ind));
    else
        ui->lineEdit_ind->setText(loc.toString(ind / 1000));

    ui->lineEdit_ind->selectAll();
    ui->comboBox->setCurrentIndex(wmin);
    ui->comboBox_2->setCurrentIndex(wmax);
    ui->doubleSpinBox_ax->setValue(pa);
    ui->doubleSpinBox_rad->setValue(pr);
    QList<QAction*> hpopupactions = headerpopupmenu->actions();
    for (int i = 0; i < hpopupactions.count(); i ++){
        if (header_bits.test(i))
            hpopupactions.at(i)->setChecked(true);
        else
            hpopupactions.at(i)->setChecked(false);
    }
    settings->endGroup();
    delete settings;
    switchColumnVisible();
    ui->toolButton_Help->setIconSize(QSize(fOpt->mainFontSize * 2, fOpt->mainFontSize * 2));
    ui->toolButton_Save->setIconSize(QSize(fOpt->mainFontSize * 2, fOpt->mainFontSize * 2));
    ui->toolButton_Clear->setIconSize(QSize(fOpt->mainFontSize * 2, fOpt->mainFontSize * 2));
    ui->pushButton_close->setIconSize(QSize(fOpt->mainFontSize * 2, fOpt->mainFontSize * 2));
    if (fOpt->styleGUI == _DarkStyle){
        ui->pushButton_calculate->setIcon(reverseIconColors(ui->pushButton_calculate->icon()));
        ui->pushButton_close->setIcon(reverseIconColors(ui->pushButton_close->icon()));
        ui->toolButton_Help->setIcon(reverseIconColors(ui->toolButton_Help->icon()));
        ui->toolButton_Clear->setIcon(reverseIconColors(ui->toolButton_Clear->icon()));
        ui->toolButton_Save->setIcon(reverseIconColors(ui->toolButton_Save->icon()));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::on_comboBox_currentIndexChanged(int index)
{
    ui->comboBox_2->clear();
    if (fOpt->isAWG){
        int j = ui->comboBox->currentText().toInt();
        for (int i = j - 1; i > maxAwgSize - 1; i--)
            ui->comboBox_2->addItem(QString::number(i));
    } else {
        for (int j = index + 1; j < mWire_size; j++)
            ui->comboBox_2->addItem(QString::number(mWire[j]));
    }
    ui->comboBox_2->setCurrentIndex(ui->comboBox_2->count() - 1);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::fillTable(QStandardItem *item, int count, double wire_d, int awgNumber)
{
    double n; double nLayer = 0; double Nc = 0; double c = 0; double lengthWire = 0; double massWire=0; double DCR=0;
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Wire diameter") + " ["
                         + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "]\nd");
    if (fOpt->isAWG)
        item = new QStandardItem(roundTo(wire_d / fOpt->dwLengthMultiplier, loc, 2) + " - (" + QString::number(awgNumber) + " AWG)");
    else
        item = new QStandardItem(roundTo(wire_d / fOpt->dwLengthMultiplier, loc, 2));
    model->setItem(count - 1, 0, item);
    findBrooksCoil(ind, wire_d, ui->doubleSpinBox_ax->value(), ui->doubleSpinBox_rad->value(),
                   n, nLayer, Nc, c, lengthWire, massWire, DCR);
    item = new QStandardItem(roundTo(n, loc, 1));
    model->setItem(count - 1, 1, item);
    item = new QStandardItem(QString::number(ceil(nLayer)));
    model->setItem(count - 1, 2, item);
    item = new QStandardItem(QString::number(round(Nc)));
    model->setItem(count - 1, 3, item);
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Dimensions of inductor")
                         + " [" + qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()) + "]\n DxHxW ");
    item = new QStandardItem(roundTo(2 * c / fOpt->dwLengthMultiplier, loc, 1)
                             + "x" + roundTo(4 * c / fOpt->dwLengthMultiplier, loc, 1)
                             + "x" + roundTo(c / fOpt->dwLengthMultiplier, loc, 1));
    model->setItem(count - 1, 4, item);
    QString _wire_length = formatLength(lengthWire, fOpt->dwLengthMultiplier);
    QStringList list = _wire_length.split(QRegExp(" "), skip_empty_parts);
    QString d_wire_length = list[0];
    QString _ssLengthMeasureUnit = list[1];
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Length of wire without leads") + "\n[" +
                         qApp->translate("Context", _ssLengthMeasureUnit.toUtf8()) + "]");
    item = new QStandardItem(roundTo(d_wire_length.toDouble(), loc, fOpt->dwAccuracy));
    model->setItem(count - 1, 5, item);
    item = new QStandardItem(roundTo(massWire, loc, fOpt->dwAccuracy));
    model->setItem(count - 1, 6, item);
    item = new QStandardItem(roundTo(DCR, loc, fOpt->dwAccuracy));
    model->setItem(count - 1, 7, item);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::on_pushButton_calculate_clicked()
{
    if (ind == 0){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        return;
    }
    QStandardItem *item = NULL;
    if (model->rowCount() > 0)
        model->removeRows(0,model->rowCount());
    //Table rows headers
    QStringList verticalHeader;
    int imin = 0;
    int imax = 0;
    QString smin = ui->comboBox->itemText(ui->comboBox->currentIndex());
    QString smax = ui->comboBox_2->itemText(ui->comboBox_2->currentIndex());
    int count = 0;
    if (fOpt->isAWG){
        for (int i = 32; i > maxAwgSize - 1; i--){
            if (smin == QString::number(i))
                imin = i;
            if (smax == QString::number(i))
                imax = i;
        }
        for (int i = imin; i > imax - 1; i--){
            count++;
            verticalHeader.append(QString::number(count));
            double wire_d = convertfromAWG(QString::number(i));
            fillTable(item, count, wire_d, i);
        }
    } else {
        for (int j = 0; j < mWire_size; j++){
            if (smin == QString::number(mWire[j]))
                imin = j;
            if (smax == QString::number(mWire[j]))
                imax = j;
        }
        for (int i = imin; i < imax + 1; i++){
            count++;
            verticalHeader.append(QString::number(count));
            fillTable(item, count, mWire[i], 0);
        }
    }
    model->setVerticalHeaderLabels(verticalHeader);
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::on_lineEdit_ind_editingFinished()
{
    if (ui ->comboBox_N_m->currentIndex() == 0)
        ind = loc.toDouble(ui->lineEdit_ind->text());
    else
        ind = loc.toDouble(ui->lineEdit_ind->text()) * 1000;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::on_comboBox_N_m_currentIndexChanged(int index)
{
    if (index == 0){
        ui->lineEdit_ind->setText(loc.toString(ind));
    } else {
        ui->lineEdit_ind->setText(loc.toString(ind / 1000));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::on_tableView_customContextMenuRequested(const QPoint &pos)
{
    if (ui->tableView->model()->rowCount() > 0){
        tablepopupmenu->popup(ui->tableView->viewport()->mapToGlobal(pos));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::tableHeaderCustomContextMenuRequested(const QPoint &pos)
{
    headerpopupmenu->popup(ui->tableView->horizontalHeader()->viewport()->mapToGlobal(pos));
    QList<QAction*> popupactions = headerpopupmenu->actions();
    for (int i = 1; i < popupactions.count(); i ++){
        popupactions[i]->setObjectName(QString::number(i));
        popupactions[i]->connect(popupactions[i], SIGNAL(triggered()), this, SLOT(switchColumnVisible()));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::on_removeRow()
{
    int row = ui->tableView->selectionModel()->currentIndex().row();
    if(row >= 0){
        ui->tableView->model()->removeRow(row);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::on_toolButton_Clear_clicked()
{
    ui->tableView->model()->removeRows(0, ui->tableView->model()->rowCount());
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::on_pushButton_close_clicked()
{
    this->close();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::on_toolButton_Help_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/crossover.html"));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::switchColumnVisible()
{
    QList<QAction*> popupactions = headerpopupmenu->actions();
    for (int i = 1; i < popupactions.count(); i ++){
        if (popupactions[i]->isChecked()){
            ui->tableView->showColumn(i);
            header_bits.set(i);
        } else {
            ui->tableView->setColumnHidden(i, true);
            header_bits.reset(i);
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Crossover::on_toolButton_Save_clicked()
{
    QAbstractItemModel *model = ui->tableView->model();
    if (model->rowCount() > 0){
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
        QString filters(".csv (*.csv);;All files (*.*)");
        QString defaultFilter(".csv (*.csv)");
        QString dataDelimiter = ",";
        if (loc.decimalPoint() == ',')
            dataDelimiter = ";";
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save"), savePath, filters, &defaultFilter);
        if (!fileName.isEmpty()){
            QString ext = defaultFilter.mid(defaultFilter.indexOf("*") + 1, 4);
            int p = fileName.indexOf(".");
            if (p < 0){
                fileName.append(ext);
            }
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                QTextStream data(&file);
                QStringList strList;
                strList.append("\"" + ui->groupBox->title() + ": " + ui->lineEdit_ind->text() +  " " + ui->comboBox_N_m->currentText() + "\"");
                data << strList.at(0) << "\n";
                strList.clear();
                for (int i = 0; i < model->columnCount(); i++) {
                    if (model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString().length() > 0)
                        strList.append("\"" + model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() + "\"");
                    else
                        strList.append("");
                }
                data << strList.join(dataDelimiter) << "\n";
                for (int i = 0; i < model->rowCount(); i++) {
                    strList.clear();
                    for (int j = 0; j < model->columnCount(); j++) {
                        if (model->data(model->index(i, j)).toString().length() > 0)
                            strList.append("\"" + model->data(model->index(i, j)).toString() + "\"");
                        else
                            strList.append("");
                    }
                    data << strList.join(dataDelimiter) + "\n";
                }
                file.close();
            }
        }
    }
}
