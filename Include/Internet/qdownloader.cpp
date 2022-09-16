#include "../Include/Internet/qdownloader.h"

DOWNLOADER::DOWNLOADER(QObject *parent) : QObject(parent)
{
    qDebug() << "DOWNLOADER created";

    // Get the instance of the main application
    app = QCoreApplication::instance();

    connect(&manager, SIGNAL (finished(QNetworkReply*)),
            SLOT (processDownload(QNetworkReply*)));

}

void DOWNLOADER::setupAfterThreadStarted()
{
    qDebug() << "Completing downloader setup after thread started";
}


void DOWNLOADER::download(OString &url, OString &fileName)
{
    qDebug() << "Download request called";

    downloadOK = false;
    qFileName = fileName.getQString();
    qURL = QUrl(url.getQString());
    if (!qURL.isValid())
    {
        qDebug() << "ERROR! - Invalid URL";
        globals->logMsg(ErrorURL, url);
    }

    QNetworkRequest myRequest(qURL);
    netReply = manager.get(myRequest);

    QCoreApplication::processEvents();
    app->QCoreApplication::processEvents();

    qDebug() << "QNetworkRequest made";
    if (netReply->isFinished())
        qDebug() << "Download request finished";
    else
        qDebug() << "Download request hung";

//    connect(netReply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error(QNetworkReply::NetworkError)));
//    bool slotCheck = netReply->isSignalConnected(finished(QNetworkReply*));

}

void DOWNLOADER::processDownload(QNetworkReply *netReply)
{
    qDebug() << "Finished() signal sent and received";

    downloadOK = false;

    if(netReply->error())
    {
        qDebug() << "ERROR - " << netReply->errorString();
        PQString errorMsg;
        errorMsg << "Error downloading " << qURL.toString() << " : " << netReply->errorString();
        globals->logMsg(ErrorConnection, errorMsg);
    }
    else
    {
        downloadedData = netReply->readAll();
        file = new QFile(qFileName);
        if (file->open(QIODevice::WriteOnly))
        {
            file->write(netReply->readAll());
            file->close();
            downloadOK = true;
        }
    }

    netReply->deleteLater();
    delete file;
}


bool DOWNLOADER::downloadSuccessful() const
{
    return downloadOK;
}

void DOWNLOADER::error(QNetworkReply::NetworkError code)
{
    qDebug() << "QNetworkReply::NetworkError " << code << "received";
}

void DOWNLOADER::setGlobalVars(GLOBALVARS &gv)
{
    globals = &gv;
}

