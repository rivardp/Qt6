
#ifndef DATAMINING_H
#define DATAMINING_H

#include <QString>
#include <QSqlQuery>
#include <QThread>
#include <QTextStream>
#include <QDebug>
#include <QVariant>
#include <QList>
#include <QRandomGenerator>

#include "../Include/Actuary/PQDate.h"
#include "../Include/Penser/OQString.h"
#include "../UpdateFuneralHomes/Include/dataRecord.h"
#include "../Include/Unstructured/qUnstructuredContent.h"
#include "../Include/PMySQL/pMySQL.h"
#include "../Include/PMySQL/funeralHomeSQL.h"
#include "../Include/Internet/URLUtilities.h"
#include "../Include/Internet/qDownloadWorker.h"
#include "../Include/Internet/qSourceFile.h"
#include "../UpdateFuneralHomes/Include/providers.h"
#include "../UpdateFuneralHomes/Include/globalVars.h"
#include "../UpdateFuneralHomes/Include/dataMiningStructure.h"

class MINER : public QObject
{

    Q_OBJECT

private:

    QCoreApplication *app;
    QThread *downloadThread;
    DownloadWorker *www;
    GLOBALVARS *globals;

public:
    explicit MINER(QObject *parent = nullptr);

    void setDownloadThread(QThread *thread);
    void setDownloadWorker(DownloadWorker *downloadworker);
    void setGlobalVars(GLOBALVARS &gv);
    void quit();    // call to quit application

    void mineData();

signals:
    void finished();    // signal to close the application

public slots:
    void execute();
    void aboutToQuitApp();  // gets called after finished() signal

};

#endif // DATAMINING_H

