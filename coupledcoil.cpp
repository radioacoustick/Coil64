#include "coupledcoil.h"
#include "ui_coupledcoil.h"

CoupledCoil::CoupledCoil(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CoupledCoil)
{
    ui->setupUi(this);
    fOpt = new _OptionStruct;
    dv = new QDoubleValidator(0.0, DBL_MAX, 380);
    iv = new QIntValidator(1, INT16_MAX, this);
    awgV = new QRegExpValidator(QRegExp(AWG_REG_EX));
    ui->lineEdit_D1->setValidator(dv);
    ui->lineEdit_D2->setValidator(dv);
    ui->lineEdit_d->setValidator(dv);
    ui->lineEdit_l1->setValidator(dv);
    ui->lineEdit_l2->setValidator(dv);
    ui->lineEdit_x->setValidator(dv);
    ui->lineEdit_N1->setValidator(iv);
    ui->lineEdit_N2->setValidator(iv);
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(on_timer()));
    thread = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CoupledCoil::~CoupledCoil()
{
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Coupled" );
    settings->setValue("pos", this->pos());
    settings->setValue("size", size());
    settings->setValue("D1", D1);
    settings->setValue("D2", D2);
    settings->setValue("dw", dw);
    settings->setValue("l1", l1);
    settings->setValue("l2", l2);
    settings->setValue("x", xs);
    settings->setValue("N1", N1);
    settings->setValue("N2", N2);
    settings->endGroup();
    delete settings;
    if(thread != nullptr){
        if(thread->isRunning())
            thread->abort();
    }
    delete fOpt;
    delete awgV;
    delete iv;
    delete dv;
    delete timer;
    delete ui;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::getOpt(_OptionStruct gOpt)
{
    *fOpt = gOpt;
    ui->label_D1_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_D2_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_d_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_l1_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_l2_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    ui->label_x_m->setText(qApp->translate("Context", fOpt->ssLengthMeasureUnit.toUtf8()));
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "Coupled" );
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    QPoint pos = settings->value("pos", QPoint(x, y)).toPoint();
    QSize size = settings->value("size", this->minimumSize()).toSize();
    D1 = settings->value("D1", 0).toDouble();
    D2 = settings->value("D2", 0).toDouble();
    dw = settings->value("dw", 0).toDouble();
    l1 = settings->value("l1", 0).toDouble();
    l2 = settings->value("l2", 0).toDouble();
    xs = settings->value("x", 0).toDouble();
    N1 = settings->value("N1", 0).toInt();
    N2 = settings->value("N2", 0).toInt();
    settings->endGroup();
    resize(size);
    move(pos);
    if (fOpt->isAWG){
        ui->label_d_m->setText(tr("AWG"));
        ui->lineEdit_d->setValidator(awgV);
        if (dw > 0){
            ui->lineEdit_d->setText(converttoAWG(dw));
        } else
            ui->lineEdit_d->setText("");
    } else
        ui->lineEdit_d->setText(roundTo(dw / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_D1->setText(roundTo(D1 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_D2->setText(roundTo(D2 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_l1->setText(roundTo(l1 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_l2->setText(roundTo(l2 / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_x->setText(roundTo(xs / fOpt->dwLengthMultiplier, loc, fOpt->dwAccuracy));
    ui->lineEdit_N1->setText(QString::number(N1));
    ui->lineEdit_N2->setText(QString::number(N2));
    ui->label_D1->setText(tr("Winding diameter") + " D1:");
    ui->label_D2->setText(tr("Winding diameter") + " D2:");
    ui->label_d->setText(tr("Wire diameter") + " d:");
    ui->label_l1->setText(tr("Winding length") + " l1:");
    ui->label_l2->setText(tr("Winding length") + " l2:");
    ui->label_x->setText(tr("Coils offset") + " x:");
    ui->label_N1->setText(tr("Number of turns") + " N1:");
    ui->label_N2->setText(tr("Number of turns") + " N2:");
    delete settings;
    if (fOpt->styleGUI == _DarkStyle){
        ui->pushButton_calculate->setIcon(reverseIconColors(ui->pushButton_calculate->icon()));
        ui->pushButton_close->setIcon(reverseIconColors(ui->pushButton_close->icon()));
        ui->pushButton_help->setIcon(reverseIconColors(ui->pushButton_help->icon()));
    }
    ui->lineEdit_D1->selectAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::getCurrentLocale(QLocale locale)
{
    this->loc = locale;
    this->setLocale(loc);
    dv->setLocale(loc);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_pushButton_close_clicked()
{
    this->close();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_pushButton_help_clicked()
{
    QDesktopServices::openUrl(QUrl("https://coil32.net/coupled-coils.html"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_pushButton_calculate_clicked()
{
    if(thread != nullptr){
        if(thread->isRunning()){
            ui->pushButton_calculate->setEnabled(false);
            thread->abort();
            return;
        }
    }
    bool ok1, ok2, ok3, ok4, ok5, ok6, ok7, ok8;
    D1 = loc.toDouble(ui->lineEdit_D1->text(), &ok1)*fOpt->dwLengthMultiplier;
    D2 = loc.toDouble(ui->lineEdit_D2->text(), &ok2)*fOpt->dwLengthMultiplier;
    if (fOpt->isAWG){
        dw = convertfromAWG(ui->lineEdit_d->text(), &ok3);
    } else {
        dw = loc.toDouble(ui->lineEdit_d->text(), &ok3)*fOpt->dwLengthMultiplier;
    }
    l1 = loc.toDouble(ui->lineEdit_l1->text(), &ok4)*fOpt->dwLengthMultiplier;
    l2 = loc.toDouble(ui->lineEdit_l2->text(), &ok5)*fOpt->dwLengthMultiplier;
    xs = loc.toDouble(ui->lineEdit_x->text(), &ok6)*fOpt->dwLengthMultiplier;
    N1 = loc.toInt(ui->lineEdit_N1->text(), &ok7);
    N2 = loc.toInt(ui->lineEdit_N2->text(), &ok8);
    if((!ok1)||(!ok2)||(!ok3)||(!ok4)||(!ok5)||(!ok6)||(!ok7)||(!ok8)){
        showWarning(tr("Warning"), tr("One or more inputs have an illegal format!"));
        return;
    }
    if ((D1 == 0)||(D2 == 0)||(dw == 0)||(l1 == 0)||(l2 == 0)||(N1 == 0)||(N2 == 0)){
        showWarning(tr("Warning"), tr("One or more inputs are equal to null!"));
        return;
    }
    double p1 = l1 / N1;
    double p2 = l2 / N2;
    if (p1 < dw){
        showWarning(tr("Warning"), "p1 < d");
        return;
    }
    if (p2 < dw){
        showWarning(tr("Warning"), "p2 < d");
        return;
    }
    thread = new MThread_calculate( _CoupledCoils, -1, D1, D2, dw, l1, l2, N1, N2, fOpt->dwAccuracy, Cu, xs);
    connect(thread, SIGNAL(sendResult(_CoilResult)), this, SLOT(get_CoupledCoils_Result(_CoilResult)));
    connect(thread, SIGNAL(started()), this, SLOT(on_calculation_started()));
    connect(thread, SIGNAL(finished()), this, SLOT(on_calculation_finished()));
    thread->start();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::get_CoupledCoils_Result(_CoilResult result)
{
    double L1 = result.N;
    double L2 = result.sec;
    double M = result.thd;
    double k = M / sqrt(L1 * L2);
    QString sCaption = QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + " - " + windowTitle();
    QString sImage = "<img src=\":/images/res/Coupled-coils.png\">";
    QString sInput = "<p><u>" + tr("Input data") + ":</u><br/>";
    sInput += formattedOutput(fOpt, ui->label_D1->text(), ui->lineEdit_D1->text(), ui->label_D1_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_D2->text(), ui->lineEdit_D2->text(), ui->label_D2_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_l1->text(), ui->lineEdit_l1->text(), ui->label_l1_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_l2->text(), ui->lineEdit_l2->text(), ui->label_l2_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_d->text(), ui->lineEdit_d->text(), ui->label_d_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_x->text(), ui->lineEdit_x->text(), ui->label_x_m->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_N1->text(), ui->lineEdit_N1->text()) + "<br/>";
    sInput += formattedOutput(fOpt, ui->label_N2->text(), ui->lineEdit_N2->text()) + "</p>";
    QString sResult = "";
    if ((M > 0.0) && (L1 > 0.0) && (L2 > 0.0) && ( k <= 1.0)){
        sResult += "<p><u>" + tr("Result") + ":</u><br/>";
        sResult += formattedOutput(fOpt, tr("Inductance") + " L<sub>1</sub> = ", roundTo(L1 / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                   qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(fOpt, tr("Inductance") + " L<sub>2</sub> = ", roundTo(L2 / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                   qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(fOpt, tr("Mutual Inductance") + " M<sub>12</sub> = ", roundTo(M / fOpt->dwInductanceMultiplier, loc, fOpt->dwAccuracy),
                                   qApp->translate("Context", fOpt->ssInductanceMeasureUnit.toUtf8())) + "<br/>";
        sResult += formattedOutput(fOpt, tr("Coupling coefficient") + " k = ", roundTo(k, loc, fOpt->dwAccuracy));
        sResult += "</p>";
    } else if ((M == 0.0) || (L1 == 0.0) || (L2 == 0.0) || (k > 1.0)){
        sResult += "<span style=\"color:red;\">" + tr("Calculation was failed") + "</span>";
    } else {
        sResult += "<span style=\"color:red;\">" + tr("Calculation was aborted") + "</span>";
    }
    emit sendResult(sCaption + LIST_SEPARATOR + sImage + LIST_SEPARATOR + sInput + LIST_SEPARATOR + sResult);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_timer()
{
    timer->stop();
    ui->pushButton_calculate->setEnabled(true);
    ui->pushButton_calculate->setText(tr("Abort"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_calculation_started()
{
    timer->start(TIMER_INTERVAL);
    this->setCursor(Qt::WaitCursor);
    ui->pushButton_calculate->setEnabled(false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CoupledCoil::on_calculation_finished()
{
    timer->stop();
    ui->pushButton_calculate->setEnabled(true);
    ui->pushButton_calculate->setText(tr("Calculate"));
    this->setCursor(Qt::ArrowCursor);
    thread->deleteLater();
    thread = nullptr;
}
