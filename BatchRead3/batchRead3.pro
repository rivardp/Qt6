QT += core network sql widgets gui

TARGET = BatchRead3

CONFIG += console gui
CONFIG -= app_bundle

TEMPLATE = app

DEFINES += \
    UNICODE \
    _UNICODE \

#LIBS += user32.lib

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
    ../Include/Internet/qDownloadWorkerBR3.h \
    ../Include/Internet/qWebStructures.h \
    ../Include/PMySQL/pMySQL.h \
    ../Include/Penser/QtUtilities.h \
    ../PostalCodes/postalCodeInfo.h \
    ../UpdateFuneralHomes/Include/globalVars.h \
    Include/dataMiningBR3.h \
    ../Include/Unstructured/qUnstructuredContent.h \
    ../Include/Penser/PQString.h \
    ../Include/Penser/OQString.h \
    ../Include/Penser/OQStream.h \
    ../Include/Internet/qSourceFile.h \
    ../UpdateFuneralHomes/Include/dataStructure.h \
    ../Include/Penser/PString.h \
    ../Include/Unstructured/qMakeDate.h \
    ../Include/Actuary/PQDate.h \
    ../Include/Internet/URLUtilities.h \
    ../Include/PMySQL/databaseSearches.h \
    ../UpdateFuneralHomes/Include/providers.h \
    ../UpdateFuneralHomes/Include/dataRecord.h \
    ../Include/Unstructured/readObit.h \
    ../Include/PMySQL/funeralHomeSQL.h \
    ../UpdateFuneralHomes/Include/mostCommonName.h \
    ../Include/Penser/qchartype.h

SOURCES += main.cpp \
    ../Include/Internet/qDownloadWorkerBR3.cpp \
    ../Include/PMySQL/pMySQL.cpp \
    ../Include/Penser/QtUtilities.cpp \
    ../PostalCodes/postalCodeInfo.cpp \
    Include/globalVarsBR3.cpp \
    Include/dataMiningBR3.cpp \
    ../Include/Unstructured/qUnstructuredContent.cpp \
    ../Include/Penser/PQString.cpp \
    ../Include/Penser/OQString.cpp \
    ../Include/Penser/OQStream.cpp \
    ../Include/Internet/qSourceFile.cpp \
    ../UpdateFuneralHomes/Include/dataStructure.cpp \
    ../Include/Penser/PString.cpp \
    ../Include/Unstructured/qMakeDate.cpp \
    ../Include/Actuary/PQDate.cpp \
    ../Include/Internet/URLUtilities.cpp \
    ../Include/PMySQL/databaseSearches.cpp \
    ../UpdateFuneralHomes/Include/dataRecord.cpp \
    ../Include/Unstructured/readObit.cpp \
    ../Include/PMySQL/funeralHomeSQL.cpp \
    ../UpdateFuneralHomes/Include/mostCommonName.cpp


