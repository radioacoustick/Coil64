#ifndef COUPLEDCOIL_H
#define COUPLEDCOIL_H

#include <QDialog>
#include <QScreen>
#include <QDoubleValidator>
#include <QString>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>

#include "mthread_calculate.h"
#include "system_functions.h"
#include "resolves.h"

namespace Ui {
class CoupledCoil;
}

class CoupledCoil : public QDialog
{
    Q_OBJECT

public:
    explicit CoupledCoil(QWidget *parent = nullptr);
    ~CoupledCoil();
signals:
    void sendResult(QString);

private slots:
    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);
    void on_pushButton_close_clicked();
    void on_pushButton_help_clicked();
    void on_pushButton_calculate_clicked();
    void get_CoupledCoils_Result(_CoilResult result);
    void on_timer();
    void on_calculation_started();
    void on_calculation_finished();

private:
    Ui::CoupledCoil *ui;

    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QIntValidator *iv;
    QRegExpValidator *awgV;
    QLocale loc;
    MThread_calculate *thread;
    QTimer *timer;
    double D1, D2, dw, l1, l2, xs;
    int N1, N2;
};

#endif // COUPLEDCOIL_H
