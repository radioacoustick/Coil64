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
#if defined(__linux__) && defined(__GLIBC__)
#include <gnu/libc-version.h>
#endif
#include <QSslSocket>

QString getOSVersion(){
    static QString osVersion;
    if(osVersion.isEmpty())
    {
#if defined(Q_OS_MAC)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // Qt6: Use QSysInfo::productType() and productVersion()
        osVersion.append(QSysInfo::currentCpuArchitecture()).append(QLatin1Char('-'));
        osVersion.append(QSysInfo::productType()).append(QLatin1Char(' '));
        osVersion.append(QSysInfo::productVersion());
#else
        // Qt5: Use deprecated MacintoshVersion enum
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
#endif
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                int index = -1;
                QRegularExpression re("^PRETTY_NAME.*");
                for (int i = 0; i < strList.size(); ++i) {
                    if (re.match(strList[i]).hasMatch()) {
                        index = i;
                        break;
                    }
                }
#else
                int index=strList.indexOf(QRegExp("^PRETTY_NAME.*"));
#endif
                QString sname = strList[index];
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QStringList _name = sname.split(QRegularExpression("="), skip_empty_parts);
#else
                QStringList _name = sname.split(QRegExp("="), skip_empty_parts);
#endif
                QString name = _name[1];
                name.truncate(name.lastIndexOf('\n'));
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                name.remove(QRegularExpression("\""));
#else
                name.remove(QRegExp("\""));
#endif
                osVersion.append(QLatin1String("(")).append(name).append(QLatin1Char(')'));
            }
        }
        else
        {
            osVersion = QLatin1String("Linux/Unix(unknown)");
        }
#elif defined(Q_WS_WIN) || defined(Q_OS_WIN)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // Qt6: Use QSysInfo::productType() and productVersion()
        osVersion.append(QSysInfo::currentCpuArchitecture()).append(QLatin1String(" - "));
        osVersion.append(QSysInfo::productType()).append(QLatin1Char(' '));
        osVersion.append(QSysInfo::productVersion());
#else
        // Qt5: Use deprecated WindowsVersion enum
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
#endif
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
                            << QString("tw") // for the reason that there's not TraditionalChinese, so use this for substitution
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
             << QString("繁體中文")
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
 else if (lang == "tw") loc =  QLocale::LiteraryChinese;
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    int idx = -1;
    QRegularExpression re("^PROGRAMFILES=.*", QRegularExpression::CaseInsensitiveOption);
    for (int i = 0; i < env_list.size(); ++i) {
        if (re.match(env_list[i]).hasMatch()) {
            idx = i;
            break;
        }
    }
#else
    int idx = env_list.indexOf(QRegExp("^PROGRAMFILES=.*", Qt::CaseInsensitive));
#endif
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
QPixmap reversePixmapColors(const QPixmap &pm){
    QImage image(pm.toImage());
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    char decSeparator = locale.decimalPoint().at(0).toLatin1();
#else
    char decSeparator = locale.decimalPoint().toLatin1();
#endif
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
std::pair<QString, QString> getResistancePair(double value, QLocale loc, int precision) {
    if (value >= 1000.0) {
        return { roundTo(value / 1000.0, loc, precision), "kOhm" };
    }
    return { roundTo(value, loc, precision), "Ohm" };
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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// functions to create tab "libraries" of the About Window
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString cppStandard()
{
#if __cplusplus >= 202302L
    return "C++23";
#elif __cplusplus >= 202002L
    return "C++20";
#elif __cplusplus >= 201703L
    return "C++17";
#elif __cplusplus >= 201402L
    return "C++14";
#elif __cplusplus >= 201103L
    return "C++11";
#elif __cplusplus >= 199711L
    return "C++98";
#else
    return "pre-standard C++";
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString compilerName()
{
#if defined(__MINGW32__)
    return "MinGW32";
#elif defined(__MINGW64__)
    return "MinGW64";
#elif defined(_MSC_VER)
    return "MSVC";
#elif defined(__clang__)
    return "Clang";
#elif defined(__GNUC__)
    return "GCC";
#else
    return "Unknown compiler";
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString compilerVersion()
{
#if defined(__clang__)
    return QString("%1.%2.%3")
        .arg(__clang_major__)
        .arg(__clang_minor__)
        .arg(__clang_patchlevel__);

#elif defined(__GNUC__)
    return QString("%1.%2.%3")
        .arg(__GNUC__)
        .arg(__GNUC_MINOR__)
        .arg(__GNUC_PATCHLEVEL__);

#elif defined(_MSC_VER)
    return QString("%1.%2")
        .arg(_MSC_VER / 100)
        .arg(_MSC_VER % 100);

#else
    return "?";
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString targetCpu()
{
#if defined(__x86_64__) || defined(_M_X64)
    return "x86_64 (64-bit)";
#elif defined(__i386__) || defined(_M_IX86)
    return "i686 (32-bit)";
#elif defined(__aarch64__)
    return "arm64";
#elif defined(__arm__)
    return "arm";
#elif defined(__wasm32__)
    return "wasm32";
#elif defined(__riscv)
    return "riscv";
#else
    return "unknown-cpu";
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString targetBitness()
{
#if QT_POINTER_SIZE == 8
    return "64-bit";
#elif QT_POINTER_SIZE == 4
    return "32-bit";
#else
    return "unknown-bitness";
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString osBitness()
{
    const QString arch = QSysInfo::currentCpuArchitecture();
    return arch.contains("64") ? "64-bit" : "32-bit";
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString formatCompileDate()
{
    // __DATE__ = "Mmm dd yyyy"
    QString dateStr(__DATE__);
    QString monthStr = dateStr.left(3);
    QString dayStr = dateStr.mid(4, 2).trimmed();
    QString yearStr = dateStr.mid(7, 4);

    QString monthNum;
    if (monthStr == "Jan") monthNum = "01";
    else if (monthStr == "Feb") monthNum = "02";
    else if (monthStr == "Mar") monthNum = "03";
    else if (monthStr == "Apr") monthNum = "04";
    else if (monthStr == "May") monthNum = "05";
    else if (monthStr == "Jun") monthNum = "06";
    else if (monthStr == "Jul") monthNum = "07";
    else if (monthStr == "Aug") monthNum = "08";
    else if (monthStr == "Sep") monthNum = "09";
    else if (monthStr == "Oct") monthNum = "10";
    else if (monthStr == "Nov") monthNum = "11";
    else if (monthStr == "Dec") monthNum = "12";
    else monthNum = "00";
    // Return in format YYYY-MM-DD
    return QString("%1-%2-%3").arg(yearStr).arg(monthNum).arg(dayStr);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static QString buildType()
{
#ifdef QT_DEBUG
    return "Debug";
#else
    return "Release";
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString systemLocaleInfo()
{
    QLocale l = QLocale::system();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QString(" | %1 (%2, %3)").arg(l.name(), QLocale::languageToString(l.language()), QLocale::territoryToString(l.territory()));
#else
    return QString(" | %1 (%2)").arg(l.name(), QLocale::languageToString(l.language()), QLocale::countryToString(l.country()));
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString getAppBuildInfo()
{
    QString hInfo = "<html><head/><body><p>";
    hInfo.append("<b>").append(QCoreApplication::applicationName());
    hInfo.append(" v").append(QCoreApplication::applicationVersion());
    hInfo.append(" (").append(targetBitness()).append(")").append("</b></p><p>");
#if defined(Q_OS_WIN)
#if QT_VERSION > QT_VERSION_CHECK(5, 7, 1)
    hInfo.append("<img src=\":/menu_ico/res/ico2-windows.png\" style=\"vertical-align: middle;\"/> Windows 7-11");
#else
    hInfo.append("<img src=\":/menu_ico/res/ico2-windows-xp.png\" style=\"vertical-align: middle;\"/> Windows XP");
#endif
#elif defined(Q_OS_LINUX)
    hInfo.append("<img src=\":/menu_ico/res/ico2-linux.png\" style=\"vertical-align: middle;\"/> Linux");
#elif defined(Q_OS_MACOS)
    hInfo.append("<img src=\":/menu_ico/res/ico2-mac-os.png\" style=\"vertical-align: middle;\"/> macOS");
#elif defined(Q_OS_IOS)
    hInfo.append("<img src=\":/menu_ico/res/ico2-apple.png\" style=\"vertical-align: middle;\"/> iOS");
#elif defined(Q_OS_FREEBSD)
    hInfo.append("<img src=\":/menu_ico/res/ico2-bsd.png\" style=\"vertical-align: middle;\"/> FreeBSD");
#elif defined(Q_OS_UNIX)
    hInfo.append("Unix");
#else
    hInfo.append("Unknown OS");
#endif
    hInfo.append(" | ").append(QString("Qt %1").arg(QT_VERSION_STR));
#ifdef QT_STATIC
    hInfo.append(" static");
#else
    hInfo.append(" shared");
#endif
    hInfo.append("<hr/>Compiler: ").append(compilerName() + " " + compilerVersion()).append(", ");
    hInfo.append(cppStandard()).append(", ").append(QString("%1").arg(targetCpu()));
#if defined(__linux__) && defined(__GLIBC__)
    hInfo.append(" | ").append(QString("glibc %1").arg(gnu_get_libc_version()));
#endif
    hInfo.append("<hr/>").append(buildType() + ": ").append(QString("%1 %2").arg(formatCompileDate()).arg(__TIME__));
    hInfo.append("<hr/>OS: ").append(QString("%1, %2").arg(QSysInfo::prettyProductName()).arg(osBitness()));
    hInfo.append(systemLocaleInfo());
    hInfo.append("</p></body></html>");
    return hInfo;
}
