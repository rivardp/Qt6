#ifndef QDOWNLOADWORKERBR3_H
#define QDOWNLOADWORKERBR3_H

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
#include <QRandomGenerator>

#include "../Include/Penser/OQString.h"
#include "../Include/Internet/qSourceFile.h"
#include "../Internet/qWebStructures.h"

enum CHARSET {UTF8, ISO88591, ISO88595, Windows1252};


class DownloadWorkerBR3 : public QObject
{
    Q_OBJECT

 public :
    explicit DownloadWorkerBR3(QObject *parent = nullptr);
    ~DownloadWorkerBR3();

    void quit();

    void setGlobalVars(GLOBALVARS &gv);
    bool downloadAndProcess(DOWNLOADREQUEST &downloadRequest);

    bool processingDownload();
    bool lastDownloadSuccessful();

    PQString redirectURL;

 private:
    bool pause;
    bool queueOpen;                         // Used to delay additions to queue while it is processing
    bool downloadProcessing;                // Used to pause the miner who made a download request until it is completed
    bool downloadSuccessful;                // Set to true if a file is written to disk
    DOWNLOADREQUEST currentRequest;    //

    QList<DOWNLOADREQUEST> currentDownloadQueue;

    QList<QNetworkReply *> currentDownloadReplies;
    QNetworkAccessManager *networkMgr;
    QNetworkReply *networkReply;
    QHttpMultiPart *multiPart;

    GLOBALVARS *globals;            // Used for all output streams and logging
    QFile *qFile;

    void startNextDownload();
    void appendToQueue(DOWNLOADREQUEST newRequest);
    void addToURLlist(PQString &qUrl, downloadOutputs &outputs);

public slots:
    void initiate();
    void downloadFinished(QNetworkReply *netReply);
    void handleRedirection(const QUrl &redirectedURL);
    void processDownloads();

 signals:
    void finished();
    void downloadQueueNotEmpty();

};


#endif // QDOWNLOADWORKERBR3_H

