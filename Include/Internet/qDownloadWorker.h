#ifndef QDOWNLOADWORKER_H
#define QDOWNLOADWORKER_H

#include <QThread>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QSslConfiguration>
#include <QUrl>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>

#include "../Include/Penser/OQString.h"
#include "../Include/Internet/qSourceFile.h"
#include "../Internet/qWebStructures.h"

enum CHARSET {UTF8, ISO88591, ISO88595, Windows1252, USASCII};


class DownloadWorker : public QObject
{
    Q_OBJECT

 public :
    explicit DownloadWorker(QObject *parent = nullptr);
    ~DownloadWorker();

    void quit();

    void setGlobalVars(GLOBALVARS &gv);
    void download(DOWNLOADREQUEST &dr);
    bool processingDownload();
    bool lastDownloadSuccessful();
    bool isGzipped(QNetworkReply *reply);

    PQString redirectURL;

 private:
    bool pause;
    bool queueOpen;                 // Used to delay additions to queue while it is processing
    bool downloadProcessing;        // Used to pause the miner who made a download request until it is completed
    bool downloadSuccessful;        // Set to true if a file is written to disk

    DOWNLOADREQUEST currentRequest;
    QList<DOWNLOADREQUEST> currentDownloadQueue;
    QList<QNetworkReply *> currentDownloadReplies;
    QNetworkAccessManager *networkMgr;
    QNetworkReply *networkReply;
    QFile *qFile;
    GLOBALVARS *globals;            // Used for all output streams and logging

    void startNextDownload();
    void appendToQueue(const QList<DOWNLOADREQUEST> &newRequestList);

public slots:
    void initiate();
    void downloadFinished(QNetworkReply *netReply);
    void handleRedirection(const QUrl &redirectedURL);
    void processDownloads();

 signals:
    void finished();
    void downloadQueueNotEmpty();
};


#endif // QDOWNLOADWORKER_H

