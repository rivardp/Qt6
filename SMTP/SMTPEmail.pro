#-------------------------------------------------
#
# Project created by QtCreator 2011-08-11T20:59:25
#
#-------------------------------------------------

CONFIG	+= c++11

QT = core network

VERSION = 2.0.0
VER_MAJ = 2
VER_MIN = 0
VER_PAT = 0

TARGET = Libs/SmtpMime
TEMPLATE = lib

DEFINES += SMTP_MIME_LIBRARY

SOURCES += \
    Include/emailaddress.cpp \
    Include/mimeattachment.cpp \
    Include/mimefile.cpp \
    Include/mimehtml.cpp \
    Include/mimeinlinefile.cpp \
    Include/mimemessage.cpp \
    Include/mimepart.cpp \
    Include/mimetext.cpp \
    Include/smtpclient.cpp \
    Include/quotedprintable.cpp \
    Include/mimemultipart.cpp \
    Include/mimecontentencoder.cpp \
    Include/mimebase64encoder.cpp \
    Include/mimeqpencoder.cpp \
    Include/mimeqpformatter.cpp \
    Include/mimebase64formatter.cpp \
    Include/mimecontentformatter.cpp

HEADERS  += \
    Include/emailaddress.h \
    Include/mimeattachment.h \
    Include/mimefile.h \
    Include/mimehtml.h \
    Include/mimeinlinefile.h \
    Include/mimemessage.h \
    Include/mimepart.h \
    Include/mimetext.h \
    Include/smtpclient.h \
    Include/SmtpMime \
    Include/quotedprintable.h \
    Include/mimemultipart.h \
    Include/smtpmime_global.h \
    Include/mimecontentencoder.h \
    Include/mimebase64encoder.h \
    Include/mimeqpencoder.h \
    Include/mimeqpformatter.h \
    Include/mimebase64formatter.h \
    Include/mimecontentformatter.h

OTHER_FILES += \
    LICENSE \
    README.md

FORMS +=

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
