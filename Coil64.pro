#Coil64.pro - general project file to Coil64 - Radio frequency inductor and choke calculator
#Copyright (C) 2018 - 2021 Kustarev V.

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
    ferrite_rod.cpp \
    meander_pcb.cpp \
    multiloop.cpp \
    loop.cpp \
    shield.cpp \
    amidon.cpp \
    aircoretoroid.cpp \
    potcore.cpp \
    ecore.cpp \
    al.cpp \
    ucore.cpp \
    crossover.cpp \
    rf_toroid.cpp \
    bandspread.cpp \
    rmcore.cpp \
    scdockwidget.cpp

HEADERS  += mainwindow.h \
    resolves.h \
    bessel.h \
    resolve_q.h \
    resolve_srf_cs.h \
    mthread_calculate.h \
    options.h \
    about.h \
    definitions.h \
    system_functions.h \
    find_permeability.h \
    ferrite_rod.h \
    version.h \
    meander_pcb.h \
    multiloop.h \
    loop.h \
    shield.h \
    amidon.h \
    aircoretoroid.h \
    potcore.h \
    ecore.h \
    al.h \
    ucore.h \
    crossover.h \
    spline.h \
    rf_toroid.h \
    bandspread.h \
    rmcore.h \
    scdockwidget.h

FORMS    += mainwindow.ui \
    options.ui \
    about.ui \
    find_permeability.ui \
    ferrite_rod.ui \
    meander_pcb.ui \
    multiloop.ui \
    loop.ui \
    shield.ui \
    amidon.ui \
    aircoretoroid.ui \
    potcore.ui \
    ecore.ui \
    al.ui \
    ucore.ui \
    crossover.ui \
    rf_toroid.ui \
    bandspread.ui \
    rmcore.ui \
    scdockwidget.ui

TRANSLATIONS += lang/Coil64_bg.ts \
    lang/Coil64_cs.ts \
    lang/Coil64_de.ts \
    lang/Coil64_el.ts \
    lang/Coil64_en.ts \
    lang/Coil64_es.ts \
    lang/Coil64_et.ts \
    lang/Coil64_fa.ts \
    lang/Coil64_fr.ts \
    lang/Coil64_hr.ts \
    lang/Coil64_hu.ts \
    lang/Coil64_it.ts \
    lang/Coil64_lt.ts \
    lang/Coil64_mk.ts \
    lang/Coil64_nl.ts \
    lang/Coil64_pl.ts \
    lang/Coil64_pt.ts \
    lang/Coil64_ro.ts \
    lang/Coil64_ru.ts \
    lang/Coil64_sr.ts \
    lang/Coil64_tr.ts \
    lang/Coil64_uk.ts \
    lang/Coil64_vi.ts \
    lang/Coil64_zh.ts

RESOURCES += res.qrc

win32:{
  RC_FILE = resource.rc
  DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050200
}
unix:{
  !macx:{
    QMAKE_LFLAGS += -no-pie
    QMAKE_LFLAGS += -static-libstdc++
    PKGCONFIG += openssl
  }else{
    ICON = res/coil64_icon_48.icns
    RESOURCES += mac.qrc
  }
}
