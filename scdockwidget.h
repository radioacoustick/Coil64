#ifndef MXCTDOCKWIDGET_H
#define MXCTDOCKWIDGET_H

#include <QDockWidget>
#include <QSettings>
#include <QDoubleValidator>

#include "system_functions.h"
#include "resolves.h"

namespace Ui {
class SaturationDockWidget;
}

class SaturationDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit SaturationDockWidget(QWidget *parent = 0);
    ~SaturationDockWidget();

signals:
    void sendClose();

private slots:
    void closeEvent(QCloseEvent *event);
    void getOpt(_OptionStruct gOpt);
    void getFerriteData(_FerriteData fData);
    void getCurrentLocale(QLocale locale);
    void on_SaturationDockWidget_visibilityChanged(bool visible);
    void on_comboBox_material_currentIndexChanged(int index);
    void on_lineEdit_Bs_textChanged();
    void on_lineEdit_N_textChanged();
    void on_lineEdit_mu_textChanged();
    void on_lineEdit_le_textChanged();

private:
    Ui::SaturationDockWidget *ui;
    _OptionStruct *fOpt;
    QDoubleValidator *dv;
    QLocale loc;
};

#endif // MXCTDOCKWIDGET_H
