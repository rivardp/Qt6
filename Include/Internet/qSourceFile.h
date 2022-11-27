// qSourceFile.h

#ifndef QSOURCEFILE_H
#define QSOURCEFILE_H

#include <QFile>
//#include <QTextCodec>
#include <QTextStream>
#include "../Include/Penser/PString.h"
#include "../Include/Penser/OQString.h"
#include "../Include/Penser/OQStream.h"
#include "../UpdateFuneralHomes/Include/globalVars.h"

class qSourceFile : public OQStream
{
public:
    qSourceFile();
    ~qSourceFile();

	// Assigning and opening the file
    void setSourceFile(PQString &file);
    void setSourceFile(OQString &file);
    void setSourceFile(QString &file);
    void openFile(QString &file);

	// Closing file
	void close();

	// Getting information about file
	bool isEOF() const;

    // Other methods
    void setGlobalVars(GLOBALVARS &gv);

private:
    QString filename;
    GLOBALVARS *globals;
};

#endif  //QSOURCEFILE_H

