#ifndef LOOP_H
#define LOOP_H

#include <QDialog>

namespace Ui {
class Loop;
}

class Loop : public QDialog
{
    Q_OBJECT

public:
    explicit Loop(QWidget *parent = 0);
    ~Loop();

private:
    Ui::Loop *ui;
};

#endif // LOOP_H
