#ifndef MULTILOOP_H
#define MULTILOOP_H

#include <QDialog>
#include <QDoubleValidator>
#include <QString>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>

#include "system_functions.h"
#include "resolves.h"

namespace Ui {
class Multiloop;
}

class Multiloop : public QDialog
{
    Q_OBJECT

public:
    explicit Multiloop(QWidget *parent = 0);
    ~Multiloop();

signals:
    void sendResult(QString);

private slots:

    void getOpt(_OptionStruct gOpt);
    void getCurrentLocale(QLocale locale);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_lineEdit_2_editingFinished();
    void on_checkBox_isReverce_clicked();

private:
    Ui::Multiloop *ui;
    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QLocale loc;
    double ind;
    double dw;
    int nTurns;
};

#endif // MULTILOOP_H
