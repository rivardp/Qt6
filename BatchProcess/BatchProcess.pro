TEMPLATE = app

QT += sql core webenginewidgets

DEFINES += \
    UNICODE \
    _UNICODE \

LIBS += user32.lib

TARGET = BatchProcess

SOURCES += main.cpp \
    ../Include/Penser/PString.cpp \
    ../Include/Penser/PQString.cpp \
    ../Include/Penser/OQString.cpp \
    ../Include/Penser/OQStream.cpp \
    ../Include/PMySQL/pMySQL.cpp \
    ../Include/Penser/QtUtilities.cpp \
    ../Include/Unstructured/qUnstructuredContent.cpp \
    ../Include/Unstructured/qMakeDate.cpp \
    ../PostalCodes/postalCodeInfo.cpp \
    ../PostalCodes/postalcodes.cpp \
    ../UpdateFuneralHomes/Include/dataStructure.cpp \
    Include/dataMiningBP.cpp \
    Include/globalVarsBP.cpp \
    ../Include/Unstructured/readObit.cpp \
    ../Include/Actuary/PQDate.cpp \
    ../Include/PMySQL/databaseSearches.cpp \
    ../UpdateFuneralHomes/Include/dataRecord.cpp \
    ../UpdateFuneralHomes/Include/mostCommonName.cpp \
    ../Include/PMySQL/funeralHomeSQL.cpp

HEADERS += \
    ../Include/Penser/PString.h \
    ../Include/Penser/PQString.h \
    ../Include/Penser/OQString.h \
    ../Include/Penser/OQStream.h \
    ../Include/PMySQL/pMySQL.h \
    ../Include/Penser/QtUtilities.h \
    ../Include/Unstructured/qUnstructuredContent.h \
    ../Include/Unstructured/qMakeDate.h \
    ../PostalCodes/postalCodeInfo.h \
    ../PostalCodes/postalcodes.h \
    ../UpdateFuneralHomes/Include/globalVars.h \
    ../UpdateFuneralHomes/Include/dataStructure.h \
    ../UpdateFuneralHomes/Include/providers.h \
    Include/dataMiningBP.h \
    ../Include/Unstructured/readObit.h \
    ../Include/Actuary/PQDate.h \
    ../Include/Penser/qchartype.h \
    ../Include/PMySQL/databaseSearches.h \
    ../UpdateFuneralHomes/Include/dataRecord.h \
    ../UpdateFuneralHomes/Include/mostCommonName.h \
    ../Include/PMySQL/funeralHomeSQL.h \
    ../UpdateFuneralHomes/Include/enums.h

