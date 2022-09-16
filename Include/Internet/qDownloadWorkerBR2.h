#ifndef QDOWNLOADWORKERBR2_H
#define QDOWNLOADWORKERBR2_H

#include <QThread>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
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


class DownloadWorkerBR2 : public QObject
{
    Q_OBJECT

 public :
    explicit DownloadWorkerBR2(QObject *parent = nullptr);
    ~DownloadWorkerBR2();

    void quit();

    void setGlobalVars(GLOBALVARS &gv);
    bool downloadAndProcess(PQString &url, downloadOutputs &outputs, bool allowRedirects, bool POSTrequest = false);

    bool processingDownload();
    bool lastDownloadSuccessful();

    PQString redirectURL;

 private:
    bool pause;
    bool queueOpen;                 // Used to delay additions to queue while it is processing
    bool downloadProcessing;        // Used to pause the miner who made a download request until it is completed
    bool downloadSuccessful;        // Set to true if a file is written to disk
    bool followRedirects;           // Only use where absolutely necessary to minimize hits on website
    bool POSTinstead;               // Only use where data can only be received as a JSON request

    QList<downloadRequest> currentDownloadQueue;

    QList<QNetworkReply *> currentDownloadReplies;
    QNetworkAccessManager *networkMgr;
    QNetworkReply *networkReply;

    GLOBALVARS *globals;            // Used for all output streams and logging
    QFile *qFile;
    QUrl qUrl;
    downloadOutputs Outputs;
    QString HTMLstring;             // The actual string containing the entire file contents

    void startNextDownload();
    void appendToQueue(const QList<downloadRequest> &newRequestList);
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


#endif // QDOWNLOADWORKERBR2_H

