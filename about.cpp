/* about.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "about.h"
#include "ui_about.h"
#include "definitions.h"


About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    QString title = QCoreApplication::applicationName();
    QString lable = QCoreApplication::applicationName();
    title.append(" v");
    title.append(QCoreApplication::applicationVersion());
    ui->label_2->setText(title);
    ui->tabWidget->setCurrentIndex(0);

    QResource r( ":/txt/res/gpl-3.0.html" );
    QByteArray b( reinterpret_cast< const char* >( r.data() ), r.size() );
    ui->textBrowser->setHtml(QString::fromStdString(b.toStdString()));
    QResource r2( ":/txt/res/translators.html" );
    QByteArray b2( reinterpret_cast< const char* >( r2.data() ), r2.size() );
    ui->textBrowser_2->setHtml(QString::fromStdString(b2.toStdString()));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
About::~About()
{
    delete ui;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void About::on_pushButton_clicked()
{
    this->close();
}
