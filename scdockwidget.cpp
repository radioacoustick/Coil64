#include "scdockwidget.h"
#include "ui_scdockwidget.h"

QString FToroidSaturation[] = {
    "31,3400","33,2500","43,2950","44,3000","51,3200","52,4200","61,2400","64,2200","67,2300","68,2500",
    "73,3900","75,4300","76,4000","77,4900","78,4800","83,3900","85,4200","F,4700","H,4200","J,4300","K,4600","W,4300"
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SaturationDockWidget::SaturationDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::SaturationDockWidget)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, MAX_DOUBLE, 380);
    ui->lineEdit_N->setValidator(dv);
    ui->lineEdit_mu->setValidator(dv);
    ui->lineEdit_le->setValidator(dv);
    ui->lineEdit_Bs->setValidator(dv);
    ui->comboBox_material->clear();
    ui->comboBox_material->addItem(tr("Manually"));
    for(unsigned int i = 0; i < (sizeof(FToroidSaturation)/sizeof(FToroidSaturation[0])); i++){
        QString saturation_str = FToroidSaturation[i];
        QStringList saturation_lst = saturation_str.split(",");
        QString saturation_mtr = saturation_lst.at(0);
        ui->comboBox_material->addItem(tr("Material") + " - " + saturation_mtr);
    }
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Saturation_Dock" );
    int material = settings->value( "material", 0 ).toInt();
    ui->comboBox_material->setCurrentIndex(material);
    settings->endGroup();
    delete settings;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SaturationDockWidget::~SaturationDockWidget()
{
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Saturation_Dock" );
    settings->setValue( "material", ui->comboBox_material->currentIndex() );
    if (fOpt->isSaveOnExit){
       settings->setValue("turns", ui->lineEdit_N->text());
       settings->setValue("mu", ui->lineEdit_mu->text());
       settings->setValue("le", ui->lineEdit_le->text());
    }
    settings->endGroup();
    delete settings;
    delete dv;
    delete ui;
}

void SaturationDockWidget::closeEvent(QCloseEvent *event)
{
    emit sendClose();
    QDockWidget::closeEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SaturationDockWidget::getOpt(_OptionStruct gOpt)
{
    *fOpt = gOpt;
    ui->label_le_unit->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Saturation_Dock" );
    if (fOpt->isSaveOnExit){
        QString sn = settings->value( "turns", "" ).toString();
        QString smu = settings->value( "mu", "" ).toString();
        QString sle = settings->value( "le", "" ).toString();
        ui->lineEdit_N->setText(sn);
        ui->lineEdit_mu->setText(smu);
        ui->lineEdit_le->setText(sle);
    }
    settings->endGroup();
    delete settings;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SaturationDockWidget::getFerriteData(_FerriteData fData)
{
    if ((fData.N > 0) && (fData.mu > 0) && (fData.le > 0)){
        ui->lineEdit_N->setText(roundTo(fData.N, loc, fOpt->dwAccuracy));
        ui->lineEdit_mu->setText(roundTo(fData.mu, loc, fOpt->dwAccuracy));
        ui->lineEdit_le->setText(roundTo(fData.le, loc, fOpt->dwAccuracy));
    } else {
        ui->lineEdit_N->setText("");
        ui->lineEdit_mu->setText("");
        ui->lineEdit_le->setText("");
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SaturationDockWidget::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SaturationDockWidget::on_SaturationDockWidget_visibilityChanged(bool visible)
{
    if (visible){
        if (this->isFloating()){
            QPoint pp = ((QWidget *) parent())->mapToGlobal(QPoint(0,0));
            QSize sp = ((QWidget *) parent())->size();
            pp.setX(pp.x() + sp.width());
            move(pp);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SaturationDockWidget::on_comboBox_material_currentIndexChanged(int index)
{
    if (index == 0){
        ui->lineEdit_Bs->setText("");
    } else {
        QString saturation_str = FToroidSaturation[index - 1];
        QStringList saturation_lst = saturation_str.split(",");
        QString saturation_value = saturation_lst.at(1);
        ui->lineEdit_Bs->setText(saturation_value);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SaturationDockWidget::on_lineEdit_Bs_textChanged()
{
    if((!ui->lineEdit_Bs->text().isEmpty()) && (!ui->lineEdit_N->text().isEmpty()) && (!ui->lineEdit_mu->text().isEmpty()) && (!ui->lineEdit_le->text().isEmpty())){
        double Bs = loc.toDouble(ui->lineEdit_Bs->text());
        double N = loc.toDouble(ui->lineEdit_N->text());
        double mu = loc.toDouble(ui->lineEdit_mu->text());
        double le = loc.toDouble(ui->lineEdit_le->text());
        try{
            double satI = getSaturationCurrent(Bs, le, mu, N);
            if (std::isnormal(satI))
                ui->label_result->setText(formattedOutput(fOpt, tr("Saturation current") + ":<br/>I<sub>s</sub> = ", roundTo(satI, loc , fOpt->dwAccuracy), tr("mA")));
            else
                ui->label_result->setText("");
        } catch(...){
            ui->label_result->setText("");
        }
    } else {
        ui->label_result->setText("");
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SaturationDockWidget::on_lineEdit_N_textChanged()
{
    on_lineEdit_Bs_textChanged();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SaturationDockWidget::on_lineEdit_mu_textChanged()
{
    on_lineEdit_Bs_textChanged();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SaturationDockWidget::on_lineEdit_le_textChanged()
{
    on_lineEdit_Bs_textChanged();
}
