#-------------------------------------------------
#
# Project created by QtCreator 2014-10-30T22:19:03
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = demo1
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    demo1.cpp

INCLUDEPATH += "C:/Users/Phil/Documents/Qt/Builds"

win32:CONFIG(release, debug|release): LIBS += -L"C:/Users/Phil/Documents/Qt/Builds/build-SMTPEmail-Desktop_Qt_6_3_2_MSVC2019_64bit-Release/release/Libs" -lSmtpMime2
else:win32:CONFIG(debug, debug|release): LIBS += -L"C:/Users/Phil/Documents/Qt/Builds/build-SMTPEmail-Desktop_Qt_6_3_2_MSVC2019_64bit-Debug/debug/Libs" -lSmtpMime2
else:unix: LIBS += -L$$SMTP_LIBRARY_LOCATION -lSmtpMime

#INCLUDEPATH += $$SMTP_LIBRARY_LOCATION
#DEPENDPATH += $$SMTP_LIBRARY_LOCATION
