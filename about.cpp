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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString reversePixmapColorHTML(QString input, int start){
    int i1 = input.indexOf(".png", start);
    int i2 = input.indexOf(">", start);
    QString sImg1 = input.mid(start + 10, i1 - start - 6);
    QPixmap *pixmap = new QPixmap();
    pixmap->load(sImg1, "PNG");
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    QImage image(pixmap->toImage());
    image.invertPixels();
    QPixmap am = QPixmap::fromImage(image);
    am.save(&buffer, "PNG");
    QString url = QString("<img src=\"data:image/png;base64,") + byteArray.toBase64() + "\" style=\"vertical-align: middle;\" />";
    input.remove(start, i2 - start + 1);
    input.insert(start, url);
    delete pixmap;
    return input;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void reverseLabelRichTextIconColor(QLabel *label){
    QString txt = label->text();
    int j = 0;
    do {
        if (j == 0){
            j = txt.indexOf("<img", j);
            txt = reversePixmapColorHTML(txt, j);
        } else {
            j = txt.indexOf("<img", j + 1);
            if (j > 0)
                txt = reversePixmapColorHTML(txt, j);
        }
    } while(j > 0);
    label->setText(txt);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    QString title = QCoreApplication::applicationName();
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
    QResource r3( ":/txt/res/thanks.html" );
    QByteArray b3( reinterpret_cast< const char* >( r3.data() ), r3.size() );
    ui->textBrowser_3->setHtml(QString::fromStdString(b3.toStdString()));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
About::~About()
{
    delete ui;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void About::getStyleGUI(int styleGUI)
{
    if (styleGUI == _DarkStyle){
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        ui->label_27->setPixmap(reversePixmapColors(ui->label_27->pixmap()));
        ui->label_18->setPixmap(reversePixmapColors(ui->label_18->pixmap()));
#else
        ui->label_27->setPixmap(reversePixmapColors(*ui->label_27->pixmap()));
        ui->label_18->setPixmap(reversePixmapColors(*ui->label_18->pixmap()));
#endif
        ui->pushButton->setIcon(reverseIconColors(ui->pushButton->icon()));
        reverseLabelRichTextIconColor(ui->label_13);
        reverseLabelRichTextIconColor(ui->label_21);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void About::on_pushButton_clicked()
{
    this->close();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

