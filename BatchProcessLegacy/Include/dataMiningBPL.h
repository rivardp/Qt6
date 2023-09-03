
#ifndef DATAMINING_H
#define DATAMINING_H

#include <QString>
#include <QSqlQuery>
#include <QTextStream>
#include <QCoreApplication>
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <iostream>

#include "../Include/Penser/OQString.h"
#include "../Include/Unstructured/qUnstructuredContent.h"
#include "../Include/Unstructured/readObit.h"
#include "../Include/PMySQL/pMySQL.h"
#include "../Include/Internet/URLUtilities.h"
#include "../UpdateFuneralHomes/Include/providers.h"
#include "../UpdateFuneralHomes/Include/globalVars.h"

class MINER : public QObject
{

    Q_OBJECT

private:

    QCoreApplication *app;
    GLOBALVARS *globals;

public:
    explicit MINER(QObject *parent = 0);

    void setGlobalVars(GLOBALVARS &gv);
    void quit();    // call to quit application

    void processObits();   // custom
    void processDefdMessages();

    bool loadDr(dataRecord &dr, OQStream &LegacyStream);
    bool loadDr(dataRecord &dr, QString &fullLine, QString &providerString, QString &keyString, QString &idString);

    bool fillLegacyStream(OQStream &LegacyStream, QString &residual, QFile* obitListFile);
    void logNewFH(QString &tempString, GLOBALVARS *gv);

signals:
    void finished();    // signal to close the application

public slots:
    void execute();
    void aboutToQuitApp();  // gets called after finished() signal

};

#endif // DATAMINING_H

