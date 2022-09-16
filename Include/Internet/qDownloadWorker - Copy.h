#ifndef QDOWNLOADWORKER_H
#define QDOWNLOADWORKER_H

#include <QThread>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>

#include "../Include/Penser/OQString.h"
#include "../Include/Internet/qSourceFile.h"
//#include "../UpdateFuneralHomes/Include/qutilities.h"

struct downloadRequest
{
    QUrl qUrl;
    QString outputFile;
};

class DownloadWorker : public QObject
{
    Q_OBJECT

 public :
    explicit DownloadWorker(QObject *parent = 0);
    ~DownloadWorker();

    void quit();

    void setGlobalVars(GLOBALVARS &gv);
    void download(OQString &url, OQString &fileName);
    bool processingDownload();
    bool lastDownloadSuccessful();

 private:
    bool pause;
    bool queueOpen;                 // Used to delay additions to queue while it is processing
    bool downloadProcessing;        // Used to pause the miner who made a download request until it is completed
    bool downloadSuccessful;        // Set to true if a file is written to disk
    QList<downloadRequest> currentDownloadQueue;
    QList<QNetworkReply *> currentDownloadReplies;
    QNetworkAccessManager *networkMgr;
    QNetworkReply *networkReply;
    QString outputFileName;
    QFile *qFile;
    QUrl qUrl;
    GLOBALVARS *globals;            // Used for all output streams and logging

    void startNextDownload();
    void appendToQueue(const QList<downloadRequest> &newRequestList);

public slots:
    void initiate();
    void downloadFinished(QNetworkReply *netReply);
    void processDownloads();

 signals:
    void finished();
    void downloadQueueNotEmpty();

//    void error(QString err);

};


#endif // QDOWNLOADWORKER_H

