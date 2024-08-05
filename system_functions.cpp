/* system_functions.cpp - source text to Coil64 - Radio frequency inductor and choke calculator
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

#include "system_functions.h"

QString getOSVersion(){
    static QString osVersion;
    if(osVersion.isEmpty())
    {
#if defined(Q_OS_MAC)
        switch(QSysInfo::MacintoshVersion)
        {
        case QSysInfo::MV_SIERRA:
            osVersion = QLatin1String("MacOS 10.12(Sierra)");
            break;
        case QSysInfo::MV_ELCAPITAN:
            osVersion = QLatin1String("MacOS 10.11(El Capitan)");
            break;
        case QSysInfo::MV_YOSEMITE:
            osVersion = QLatin1String("MacOS 10.10(Yosemite)");
            break;
        case QSysInfo::MV_MAVERICKS:
            osVersion = QLatin1String("MacOS 10.9(Mavericks)");
            break;
        case QSysInfo::MV_MOUNTAINLION:
            osVersion = QLatin1String("MacOS 10.8(Mountain Lion)");
            break;
        case QSysInfo::MV_LION:
            osVersion = QLatin1String("MacOS 10.7(Lion)");
            break;
        case QSysInfo::MV_SNOWLEOPARD:
            osVersion = QLatin1String("MacOS 10.6(Snow Leopard)");
            break;
        case QSysInfo::MV_LEOPARD:
            osVersion = QLatin1String("MacOS 10.5(Leopard)");
            break;
        case QSysInfo::MV_TIGER:
            osVersion = QLatin1String("MacOS 10.4(Tiger)");
            break;
        case QSysInfo::MV_PANTHER:
            osVersion = QLatin1String("MacOS 10.3(Panther)");
            break;
        case QSysInfo::MV_JAGUAR:
            osVersion = QLatin1String("MacOS 10.2(Jaguar)");
            break;
        case QSysInfo::MV_PUMA:
            osVersion = QLatin1String("MacOS 10.1(Puma)");
            break;
        case QSysInfo::MV_CHEETAH:
            osVersion = QLatin1String("MacOS 10.0(Cheetah)");
            break;
        case QSysInfo::MV_9:
            osVersion = QLatin1String("MacOS 9");
            break;
        case QSysInfo::MV_Unknown:
        default:
            osVersion.append(QSysInfo::currentCpuArchitecture()).append(QLatin1Char('-'));
            osVersion.append(QSysInfo::productType()).append(QLatin1Char(' '));
            osVersion.append(QSysInfo::productVersion());
            break;
        }
#elif defined(Q_WS_X11) || defined(Q_OS_LINUX)
        utsname buf;
        if(uname(&buf) != -1)
        {
            osVersion.append(buf.release).append(QLatin1Char(' '));
            osVersion.append(buf.sysname).append(QLatin1Char('-'));
            osVersion.append(buf.machine).append(QLatin1Char(' '));
            QFile file("/etc/os-release");
            if ((file.exists())&&(file.open(QIODevice::ReadOnly)))
            {
                QStringList strList;
                while(!file.atEnd())
                {
                    strList << file.readLine();
                }
                file.close();
                int index=strList.indexOf(QRegExp("^PRETTY_NAME.*"));
                QString sname = strList[index];
                QStringList _name = sname.split(QRegExp("="), skip_empty_parts);
                QString name = _name[1];
                name.truncate(name.lastIndexOf('\n'));
                name.remove(QRegExp("\""));
                osVersion.append(QLatin1String("(")).append(name).append(QLatin1Char(')'));
            }
        }
        else
        {
            osVersion = QLatin1String("Linux/Unix(unknown)");
        }
#elif defined(Q_WS_WIN) || defined(Q_OS_WIN)
        osVersion.append((QSysInfo::currentCpuArchitecture().toLatin1())).append(QLatin1String(" - "));
        switch(QSysInfo::WindowsVersion)
        {
        case QSysInfo::WV_WINDOWS10:
            osVersion = QLatin1String("Windows 10");
            break;
        case QSysInfo::WV_WINDOWS8_1:
            osVersion = QLatin1String("Windows 8.1");
            break;
        case QSysInfo::WV_WINDOWS8:
            osVersion = QLatin1String("Windows 8");
            break;
        case QSysInfo::WV_WINDOWS7:
            osVersion = QLatin1String("Windows 7");
            break;
        case QSysInfo::WV_CE_6:
            osVersion = QLatin1String("Windows CE 6.x");
            break;
        case QSysInfo::WV_CE_5:
            osVersion = QLatin1String("Windows CE 5.x");
            break;
        case QSysInfo::WV_CENET:
            osVersion = QLatin1String("Windows CE .NET");
            break;
        case QSysInfo::WV_CE:
            osVersion = QLatin1String("Windows CE");
            break;
        case QSysInfo::WV_VISTA:
            osVersion = QLatin1String("Windows Vista");
            break;
        case QSysInfo::WV_2003:
            osVersion = QLatin1String("Windows Server 2003");
            break;
        case QSysInfo::WV_XP:
            osVersion = QLatin1String("Windows XP");
            break;
        case QSysInfo::WV_2000:
            osVersion = QLatin1String("Windows 2000");
            break;
        case QSysInfo::WV_NT:
            osVersion = QLatin1String("Windows NT");
            break;
        case QSysInfo::WV_Me:
            osVersion = QLatin1String("Windows Me");
            break;
        case QSysInfo::WV_98:
            osVersion = QLatin1String("Windows 98");
            break;
        case QSysInfo::WV_95:
            osVersion = QLatin1String("Windows 95");
            break;
        case QSysInfo::WV_32s:
            osVersion = QLatin1String("Windows 3.1 with Win32s");
            break;
        default:
            osVersion.append(QLatin1Char(' '));
            osVersion.append(QSysInfo::productType()).append(QLatin1Char(' '));
            osVersion.append(QSysInfo::productVersion());
            break;
        }
        if(QSysInfo::WindowsVersion & QSysInfo::WV_CE_based)
            osVersion.append(QLatin1String(" (CE-based)"));
        else if(QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)
            osVersion.append(QLatin1String(" (NT-based)"));
        else if(QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)
            osVersion.append(QLatin1String(" (MS-DOS-based)"));
#else
        return QLatin1String("Unknown");
#endif
    }

    return osVersion;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QStringList translateInstalling(QStringList *lang){
    QStringList langList = (QStringList()
                            << QLocale(QLocale::Bulgarian).name().mid(0,2)
                            << QLocale(QLocale::Chinese).name().mid(0,2)
                            << QLocale(QLocale::Croatian).name().mid(0,2)
                            << QLocale(QLocale::Czech).name().mid(0,2)
                            << QLocale(QLocale::Dutch).name().mid(0,2)
                            << QLocale(QLocale::English).name().mid(0,2)
                            << QLocale(QLocale::Estonian).name().mid(0,2)
                            << QLocale(QLocale::Persian).name().mid(0,2)
                            << QLocale(QLocale::French).name().mid(0,2)
                            << QLocale(QLocale::German).name().mid(0,2)
                            << QLocale(QLocale::Greek).name().mid(0,2)
                            << QLocale(QLocale::Hungarian).name().mid(0,2)
                            << QLocale(QLocale::Italian).name().mid(0,2)
                            << QLocale(QLocale::Lithuanian).name().mid(0,2)
                            << QLocale(QLocale::Macedonian).name().mid(0,2)
                            << QLocale(QLocale::Polish).name().mid(0,2)
                            << QLocale(QLocale::Portuguese).name().mid(0,2)
                            << QLocale(QLocale::Romanian).name().mid(0,2)
                            << QLocale(QLocale::Russian).name().mid(0,2)
                            << QLocale(QLocale::Serbian).name().mid(0,2)
                            << QLocale(QLocale::Spanish).name().mid(0,2)
                            << QLocale(QLocale::Turkish).name().mid(0,2)
                            << QLocale(QLocale::Ukrainian).name().mid(0,2)
                            << QLocale(QLocale::Vietnamese).name().mid(0,2)
                            );
    *lang = (QStringList()
             << QLocale(QLocale::Bulgarian).nativeLanguageName().toUpper()
             << QLocale(QLocale::Chinese).nativeLanguageName().toUpper()
             << QLocale(QLocale::Croatian).nativeLanguageName().toUpper()
             << QLocale(QLocale::Czech).nativeLanguageName().toUpper()
             << QLocale(QLocale::Dutch).nativeLanguageName().toUpper()
             << QLocale(QLocale::English).nativeLanguageName().toUpper()
             << QLocale(QLocale::Estonian).nativeLanguageName().toUpper()
             << QLocale(QLocale::Persian).nativeLanguageName().toUpper()
             << QLocale(QLocale::French).nativeLanguageName().toUpper()
             << QLocale(QLocale::German).nativeLanguageName().toUpper()
             << QLocale(QLocale::Greek).nativeLanguageName().toUpper()
             << QLocale(QLocale::Hungarian).nativeLanguageName().toUpper()
             << QLocale(QLocale::Italian).nativeLanguageName().toUpper()
             << QLocale(QLocale::Lithuanian).nativeLanguageName().toUpper()
             << QLocale(QLocale::Macedonian).nativeLanguageName().toUpper()
             << QLocale(QLocale::Polish).nativeLanguageName().toUpper()
             << QLocale(QLocale::Portuguese).nativeLanguageName().toUpper()
             << QLocale(QLocale::Romanian).nativeLanguageName().toUpper()
             << QLocale(QLocale::Russian).nativeLanguageName().toUpper()
             << QLocale(QLocale::Serbian).nativeLanguageName().toUpper()
             << QLocale(QLocale::Spanish).nativeLanguageName().toUpper()
             << QLocale(QLocale::Turkish).nativeLanguageName().toUpper()
             << QLocale(QLocale::Ukrainian).nativeLanguageName().toUpper()
             << QLocale(QLocale::Vietnamese).nativeLanguageName().toUpper()
             );
    return langList;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QLocale getLanguageLocale (QString lang){
 QLocale loc;
 if (lang == "bg") loc = QLocale::Bulgarian;
 else if (lang == "zh") loc =  QLocale::Chinese;
 else if (lang == "hr") loc =  QLocale::Croatian;
 else if (lang == "cs") loc =  QLocale::Czech;
 else if (lang == "nl") loc =  QLocale::Dutch;
 else if (lang == "en") loc =  QLocale::English;
 else if (lang == "es") loc =  QLocale::Spanish;
 else if (lang == "et") loc =  QLocale::Estonian;
 else if (lang == "fa") loc =  QLocale::Persian;
 else if (lang == "fr") loc =  QLocale::French;
 else if (lang == "de") loc =  QLocale::German;
 else if (lang == "el") loc =  QLocale::Greek;
 else if (lang == "hu") loc =  QLocale::Hungarian;
 else if (lang == "it") loc =  QLocale::Italian;
 else if (lang == "lt") loc =  QLocale::Lithuanian;
 else if (lang == "mk") loc =  QLocale::Macedonian;
 else if (lang == "pl") loc =  QLocale::Polish;
 else if (lang == "pt") loc =  QLocale::Portuguese;
 else if (lang == "ro") loc =  QLocale::Romanian;
 else if (lang == "ru") loc =  QLocale::Russian;
 else if (lang == "sr") loc =  QLocale::Serbian;
 else if (lang == "tr") loc =  QLocale::Turkish;
 else if (lang == "uk") loc =  QLocale::Ukrainian;
 else if (lang == "vi") loc =  QLocale::Vietnamese;
 else loc = QLocale::system();
 return loc;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void showWarning(QString title, QString msg){
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(msg);
    msgBox.exec();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void showInfo(QString title, QString msg){
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(msg);
    msgBox.exec();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void completeOptionsStructure(_OptionStruct *opt){
    switch (opt->indexFrequencyMultiplier) {

    case 0:{
        opt->dwFrequencyMultiplier = 1e-6;
        opt->ssFrequencyMeasureUnit = QT_TRANSLATE_NOOP("Context","Hz");
        break;
    }
    case 1:{
        opt->dwFrequencyMultiplier = 1e-3;
        opt->ssFrequencyMeasureUnit = QT_TRANSLATE_NOOP("Context","kHz");
        break;
    }
    case 2:{
        opt->dwFrequencyMultiplier = 1;
        opt->ssFrequencyMeasureUnit = QT_TRANSLATE_NOOP("Context","MHz");
        break;
    }
    default:
        break;
    }
    switch (opt->indexCapacityMultiplier) {
    case 0:{
        opt->dwCapacityMultiplier = 1;
        opt->ssCapacityMeasureUnit = QT_TRANSLATE_NOOP("Context","pF");
        break;
    }
    case 1:{
        opt->dwCapacityMultiplier = 1e3;
        opt->ssCapacityMeasureUnit = QT_TRANSLATE_NOOP("Context","nF");
        break;
    }
    case 2:{
        opt->dwCapacityMultiplier = 1e6;
        opt->ssCapacityMeasureUnit = QT_TRANSLATE_NOOP("Context","microF");
        break;
    }
    default:
        break;
    }
    switch (opt->indexInductanceMultiplier) {
    case 0:{
        opt->dwInductanceMultiplier = 1e-3;
        opt->ssInductanceMeasureUnit = QT_TRANSLATE_NOOP("Context","nH");
        break;
    }
    case 1:{
        opt->dwInductanceMultiplier = 1;
        opt->ssInductanceMeasureUnit = QT_TRANSLATE_NOOP("Context","microH");
        break;
    }
    case 2:{
        opt->dwInductanceMultiplier = 1e3;
        opt->ssInductanceMeasureUnit = QT_TRANSLATE_NOOP("Context","mH");
        break;
    }
    default:
        break;
    }
    switch (opt->indexLengthMultiplier) {
    case 0:{
        opt->dwLengthMultiplier = 1;
        opt->ssLengthMeasureUnit = QT_TRANSLATE_NOOP("Context","mm");
        break;
    }
    case 1:{
        opt->dwLengthMultiplier = 10;
        opt->ssLengthMeasureUnit = QT_TRANSLATE_NOOP("Context","cm");
        break;
    }
    case 2:{
        opt->dwLengthMultiplier = 25.4;
        opt->ssLengthMeasureUnit = QT_TRANSLATE_NOOP("Context","in");
        break;
    }
    case 3:{
        opt->dwLengthMultiplier = 0.0254;
        opt->ssLengthMeasureUnit = QT_TRANSLATE_NOOP("Context","mil");
        break;
    }
    default:
        break;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString formatLength(double length, double lengthMultiplyer) {
    //Formatting output of the wire length
    double L;
    double Result = 0;
    QString LengthStr = "";

    if ((lengthMultiplyer == 1) || (lengthMultiplyer == 10)) {
        L = length * 1000;
        if (L < 10) {
            Result = L;
            LengthStr = "mm";
        } else if (L < 1000) {
            Result = L / 10;
            LengthStr = "cm";
        } else {
            Result = L / 1000;
            LengthStr = QT_TRANSLATE_NOOP("Context","m");
        }
    }
    if ((lengthMultiplyer == 25.4) || (lengthMultiplyer == 0.0254)) {
        L = length * 1000 / 24.5;
        if (L < 12) {
            Result = L;
            LengthStr = "in";
        } else {
            Result = L / 12;
            LengthStr = QT_TRANSLATE_NOOP("Context","fit");
        }
    }
    QString s = QString::number(Result) + " " + LengthStr;
    return s;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool isAppPortable(){
    QString appPath = qApp->applicationDirPath();
    bool result = true;
#if defined(Q_OS_LINUX)
    if (appPath == "/opt/coil64")
        result = false;
#elif defined(Q_OS_MAC)
    result = false;
#elif defined(Q_WS_WIN) || defined(Q_OS_WIN)
    QStringList env_list(QProcess::systemEnvironment());
    int idx = env_list.indexOf(QRegExp("^PROGRAMFILES=.*", Qt::CaseInsensitive));
    QStringList programFilesEnv = env_list[idx].split('=');
    QString programFilesDir = programFilesEnv[1];
    appPath.truncate(appPath.indexOf('/',3));
    appPath = QDir::toNativeSeparators(appPath);
    if (programFilesDir == appPath)
        result = false;
#endif
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void defineAppSettings(QSettings *&settings){
#if defined(Q_OS_MAC) || defined(Q_WS_X11) || defined(Q_OS_LINUX)
    if (isAppPortable())
        settings = new QSettings(qApp->applicationDirPath() + "/Coil64.conf", QSettings::IniFormat);
    else
        settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#elif defined(Q_WS_WIN) || defined(Q_OS_WIN)
    if (isAppPortable())
        settings = new QSettings(qApp->applicationDirPath() + "/Coil64.ini", QSettings::IniFormat);
    else
        settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName(),QCoreApplication::applicationName());
#else
    settings = new QSettings(qApp->applicationDirPath() + "/Coil64.conf", QSettings::IniFormat);
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString defineSavePath(){
    QSettings *settings;
    defineAppSettings(settings);
    settings->beginGroup( "GUI" );
    QString savePath;
    if (isAppPortable())
        savePath = settings->value("SaveDir", qApp->applicationDirPath() + PORTABLE_SAVE_LOCATION).toString();
    else
        savePath = settings->value("SaveDir", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).toUtf8() + USER_SAVE_LOCATION).toString();
    settings->endGroup();
    delete settings;
    return savePath;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QPixmap reversePixmapColors(const QPixmap *pm){
    QImage image(pm->toImage());
    image.invertPixels();
    QPixmap am = QPixmap::fromImage(image);
    return am;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QIcon reverseIconColors(QIcon ico){
    QPixmap pm = ico.pixmap(ico.actualSize(QSize(32, 32)));
    QImage image(pm.toImage());
    image.invertPixels();
    QPixmap am = QPixmap::fromImage(image);
    QIcon ricon(am);
    return ricon;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString roundTo(double num, QLocale locale, int accuracy){
    QString snum = locale.toString(num, 'f', accuracy);
    char decSeparator = locale.decimalPoint().toLatin1();
    std::string str = snum.toStdString();
    std::size_t found = str.find(decSeparator);
    if (found != std::string::npos){
        str.erase(str.find_last_not_of('0') + 1, std::string::npos); //erase any trailing zeroes
        if (str.back() == decSeparator)  //remove remaining dot if necessary
            str.pop_back();
    }
    QString res_num = QString::fromStdString(str);
    return res_num;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QStringList getValueTextColorNames(int styleGUI)
{
    QStringList colorNames =  (QStringList()<<"blue"<<"darkCyan"<<"red"<<"green"<<"black"<<"yellow"<<"cyan"<<"magenta"<<"gray");
    if (styleGUI == _DarkStyle){
        colorNames.clear();
        colorNames <<"yellow"<<"cyan"<<"magenta"<<"gray"<<"white"<<"blue"<<"darkCyan"<<"red"<<"green";
    }
    return colorNames;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QRadioButton* getCheckedRadioButton(QWidget *w)
{
    int ii = 0;
    foreach (QRadioButton *button, w->findChildren<QRadioButton*>()) {
        if (button->isChecked()) {
            return button;
        }
        ii++;
    }
    return NULL;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// formatHeader() utility function for formattedOutput()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void formatHeader(QString *header, QString fmt){
    if (header->indexOf(fmt) > 0){
        int in = header->indexOf(fmt);
        int space_cnt = 0;
        for (int i = in; i >= 0; i--){
            in = i;
            if (header->at(i) == ' '){
                space_cnt++;
                if ((space_cnt == 1) && (fmt == ":"))
                    break;
                else if (space_cnt > 1)
                    break;
            }
        }
        *header = header->mid(in);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString formattedOutput(_OptionStruct *mOpt, QString header, QString value, QString footer)
{
    QString valurTextColor = getValueTextColorNames(mOpt->styleGUI)[mOpt->vTextColor];
    QString output = "";
    if (!mOpt->isShowValueDescription){
        formatHeader(&header, "=");
        formatHeader(&header, ":");
        formatHeader(&header, "≈");
    }
    if (mOpt->isOutputValueColored)
        output = header + " <span style=\"color:"+ valurTextColor +";\">" + value + "</span> " + footer;
    else
        output = header + " <span>" + value + "</span> " + footer;
    return output;
}
