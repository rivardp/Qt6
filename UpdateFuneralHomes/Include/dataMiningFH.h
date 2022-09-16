
#ifndef DATAMINING_H
#define DATAMINING_H

#include <QString>
#include <QSqlQuery>
#include <QThread>
#include <QTextStream>

#include "../Include/Penser/OQString.h"
#include "../Include/providers.h"
#include "../Include/PMySQL/pMySQL.h"
#include "../Include/Internet/URLUtilities.h"
#include "../Include/Internet/qDownloadWorker.h"
#include "../Include/PMySQL/funeralHomeSQL.h"
#include "../Include/Internet/qSourceFile.h"

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

    void mineFHdata();
    void mineLegacyData();

    bool getFuneralHomeData(qSourceFile &sf, PROVIDER &provider, fhRecordStructure &fhRS);
    unsigned int getNumFH(qSourceFile &sf, PROVIDER &provider);
    unsigned int getFuneralHomeID(qSourceFile &sf, PROVIDER &provider);
    QString getFuneralHomeURL(qSourceFile &sf, PROVIDER &provider);

signals:
    void finished();    // signal to close the application

public slots:
    void execute();
    void aboutToQuitApp();  // gets called after finished() signal

};

#endif // DATAMINING_H

