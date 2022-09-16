// qSourceFile.cpp

#include "qSourceFile.h"

qSourceFile::qSourceFile()
{

}

qSourceFile::~qSourceFile()
{
}

void qSourceFile::setSourceFile(PQString &file)
{
    filename = file.getString();
    openFile(filename);
}

void qSourceFile::setSourceFile(OQString &file)
{
    filename = file.getString();
    openFile(filename);
}

void qSourceFile::setSourceFile(QString &file)
{
    filename = file;
    openFile(filename);
}

void qSourceFile::openFile(QString &file)
{
    QFile source(file);
    if(source.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        itsString = QString(QTextCodec::codecForMib(106)->toUnicode(source.readAll()));
        position = 0;
        if (itsString.length() > 0)
            EOS = false;
        else
            EOS = true;
    }
    else
    {
        PQString errorMessage;
        globals->logMsg(ErrorRunTime, errorMessage << "Could not open file: " << filename);
        EOS =  true;
    }
}

void qSourceFile::close()
{
    position = 0;
    EOS = true;
}

bool qSourceFile::isEOF() const
{
    return EOS;
}

void qSourceFile::setGlobalVars(GLOBALVARS &gv)
{
    globals = &gv;
}


