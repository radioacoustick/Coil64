#include "loop.h"
#include "ui_loop.h"

Loop::Loop(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Loop)
{
    ui->setupUi(this);
}

Loop::~Loop()
{
    delete ui;
}
