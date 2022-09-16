QT += core network sql widgets gui

DEFINES += \
    UNICODE \
    _UNICODE \
    QT_NO_SSL

CONFIG += c++11

LIBS += user32.lib

TARGET = ReadCSV
CONFIG += console gui
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../Include/Actuary/PQDate.cpp \
    ../Include/PMySQL/pMySQL.cpp \
    ../Include/Penser/QtUtilities.cpp \
    ../PostalCodes/postalCodeInfo.cpp \
    ../PostalCodes/postalcodes.cpp \
    Include/globalVarsReadCSV.cpp \
    ../UpdateFuneralHomes/Include/dataStructure.cpp \
    Include/readCSV.cpp \
    ../Include/Penser/PQStream.cpp \
    ../Include/Penser/PQString.cpp \
    ../UpdateFuneralHomes/Include/matchRecord.cpp \
    ../Include/Penser/OQStream.cpp \
    ../Include/Penser/OQString.cpp \
    ../Include/Penser/PString.cpp \
    Include/globals.cpp \
    ../Include/PMySQL/databaseSearches.cpp \
    ../UpdateFuneralHomes/Include/dataRecord.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    ../Include/Actuary/PQDate.h \
    ../Include/Penser/QtUtilities.h \
    ../PostalCodes/postalCodeInfo.h \
    ../PostalCodes/postalcodes.h \
    ../UpdateFuneralHomes/Include/globalVars.h \
    ../Include/PMySQL/pMySQL.h \
    ../UpdateFuneralHomes/Include/dataStructure.h \
    Include/readCSV.h \
    ../Include/Penser/PQStream.h \
    ../Include/Penser/PQString.h \
    ../UpdateFuneralHomes/Include/matchRecord.h \
    ../Include/Penser/OStream.h \
    ../Include/Penser/OQString.h \
    ../Include/Penser/PString.h \
    Include/globals.h \
    ../Include/PMySQL/databaseSearches.h \
    ../UpdateFuneralHomes/Include/dataRecord.h
