#Coil64.pro - general project file to Coil64 - Radio frequency inductor and choke calculator
#Copyright (C) 2018 - 2019 Kustarev V.

#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 3 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <https://www.gnu.org/licenses

#-------------------------------------------------
#
# Project created by QtCreator 2019-01-30T22:37:11
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Coil64
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    resolves.cpp \
    bessel.cpp \
    resolve_q.cpp \
    resolve_srf_cs.cpp \
    mthread_calculate.cpp \
    options.cpp \
    about.cpp \
    system_functions.cpp \
    find_permeability.cpp \
    ferrite_rod.cpp

HEADERS  += mainwindow.h \
    resolves.h \
    bessel.h \
    resolve_q.h \
    resolve_srf_cs.h \
    customvalidator.h \
    mthread_calculate.h \
    options.h \
    about.h \
    definitions.h \
    system_functions.h \
    find_permeability.h \
    ferrite_rod.h \
    version.h

FORMS    += mainwindow.ui \
    options.ui \
    about.ui \
    find_permeability.ui \
    ferrite_rod.ui

RESOURCES += \
    res.qrc

TRANSLATIONS += lang/Coil64_bg.ts \
    lang/Coil64_de.ts \
    lang/Coil64_el.ts \
    lang/Coil64_en.ts \
    lang/Coil64_es.ts \
    lang/Coil64_fa.ts \
    lang/Coil64_fr.ts \
    lang/Coil64_hr.ts \
    lang/Coil64_hu.ts \
    lang/Coil64_it.ts \
    lang/Coil64_mk.ts \
    lang/Coil64_nl.ts \
    lang/Coil64_pl.ts \
    lang/Coil64_pt.ts \
    lang/Coil64_ro.ts \
    lang/Coil64_ru.ts \
    lang/Coil64_sr.ts \
    lang/Coil64_tr.ts \
    lang/Coil64_zh.ts


win32:RC_FILE = resource.rc
macx: ICON = res/coil32_icon_48.icns
