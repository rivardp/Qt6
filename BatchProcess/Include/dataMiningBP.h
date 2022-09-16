
#ifndef DATAMINING_H
#define DATAMINING_H

#include <QString>
#include <QSqlQuery>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

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

signals:
    void finished();    // signal to close the application

public slots:
    void execute();
    void aboutToQuitApp();  // gets called after finished() signal

};

#endif // DATAMINING_H

