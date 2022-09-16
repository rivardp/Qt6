QT += core network sql widgets gui

TARGET = UpdateFuneralHomes
CONFIG += console gui
CONFIG -= app_bundle

TEMPLATE = app

DEFINES += \
    UNICODE \
    _UNICODE

LIBS += user32.lib

SOURCES += main.cpp \
    ../Include/Actuary/PQDate.cpp \
    ../Include/Internet/qDownloadWorker.cpp \
    ../Include/PMySQL/databaseSearches.cpp \
    ../Include/PMySQL/pMySQL.cpp \
    ../Include/Internet/UnQuoteHTML.cpp \
    ../Include/Internet/URLUtilities.cpp \
    ../Include/Internet/qSourceFile.cpp \
    ../Include/Penser/NumToString.cpp \
    ../Include/Penser/PQString.cpp \
    ../Include/Penser/OQString.cpp \
    ../Include/Penser/PQStream.cpp \
    ../Include/Penser/OQStream.cpp \
    ../Include/Penser/PString.cpp \
    ../Include/PMySQL/pMySQL.cpp \
    ../Include/PMySQL/funeralHomeSQL.cpp \
    ../Include/Unstructured/qMakeDate.cpp \
    ../Include/Unstructured/qUnstructuredContent.cpp \
    ../Include/Unstructured/readObit.cpp \
    Include/dataMiningFH.cpp \
    Include/dataRecord.cpp \
    Include/dataStructure.cpp \
    Include/globalVarsFH.cpp \
    Include/mostCommonName.cpp

HEADERS += \
    ../Include/Actuary/PQDate.h \
    ../Include/Internet/qDownloadWorker.h \
    ../Include/PMySQL/databaseSearches.h \
    ../Include/PMySQL/pMySQL.h \
    ../Include/Penser/qchartype.h \
    ../Include/Internet/qDownloadWorker.h \
    ../Include/Internet/unQuoteHTML.h \
    ../Include/Internet/URLUtilities.h \
    ../Include/Internet/qSourceFile.h \
    ../Include/Penser/NumToString.h \
    ../Include/Penser/PQString.h \
    ../Include/Penser/OQString.h \
    ../Include/Penser/PQStream.h \
    ../Include/Penser/OQStream.h \
    ../Include/Penser/PString.h \
    ../Include/PMySQL/pMySQL.h \
    ../Include/PMySQL/funeralHomeSQL.h \
    ../Include/Unstructured/qMakeDate.h \
    ../Include/Unstructured/qUnstructuredContent.h \
    ../Include/Unstructured/readObit.h \
    Include/dataMiningFH.h \
    Include/dataRecord.h \
    Include/dataStructure.h \
    Include/globalVars.h \
    Include/mostCommonName.h \
    Include/providers.h \
    UpdateFuneralHomes.h
