#include "../Include/Internet/qDownloadWorker.h"

DownloadWorker::DownloadWorker(QObject *parent) : QObject(parent)
{
    qDebug() << "Constructor initialized";

    connect(this, SIGNAL(downloadQueueNotEmpty()), this, SLOT(processDownloads()));

    pause = true;
    queueOpen = true;
    downloadProcessing = false;
}

 DownloadWorker::~DownloadWorker()
{
    qDebug() << "Destructor run";
}

void DownloadWorker::initiate()
{
    qDebug() << "initiate() slot triggered from the thread.start()";

    networkMgr = new QNetworkAccessManager(this);
    connect(networkMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
}

void DownloadWorker::processDownloads()
{
    qDebug() << "Processing of downloads started";

    queueOpen = false;    // Don't allow additions while downloading

    QMutableListIterator<downloadRequest> iter(currentDownloadQueue);
    downloadRequest currentRequest;

    while (iter.hasNext())
    {
        currentRequest = iter.next();
        qUrl = currentRequest.qUrl;
        outputFileName = currentRequest.outputFile;
        iter.remove();
        if (qUrl.isValid())  // Redundant check for  now - see download(url,file) for first check
        {
            startNextDownload();
        }
        else
            qDebug() << "Invalid url encountered: " << qUrl;
    }

    queueOpen = true;
}

void DownloadWorker::setGlobalVars(GLOBALVARS &gv)
{
    globals = &gv;
}

void DownloadWorker::appendToQueue(const QList<downloadRequest> &newRequestList)
{
    qDebug () << "Adding download request(s) to the queue";

    while (!queueOpen)
    {
        // Don't add anything to the queue until the existing queue is processed
    }

    for (auto end = newRequestList.size(), i = 0; i != end; ++i)
    {
         currentDownloadQueue += newRequestList;
    }

    emit downloadQueueNotEmpty();
 }


void DownloadWorker::startNextDownload()
{
    qDebug() << "Network request sent";

    QNetworkRequest networkRequest(qUrl);
//    networkRequest.setRawHeader( "charset", "utf-8" );
    networkReply = networkMgr->get(networkRequest);
    currentDownloadReplies.append(networkReply);
}

void DownloadWorker::downloadFinished(QNetworkReply *netReply)
{
    qDebug() << "Network returned finished() signal";

    if (netReply->error())
    {
        qDebug() << "Download Error: " << netReply->errorString();
        PQString errorMessage;
        errorMessage << QString("Error downloading ") << qUrl.toString();
        errorMessage << QString(" : ") << netReply->errorString();
        globals->logMsg(ErrorConnection, errorMessage);
    }
    else
    {
        qFile = new QFile(outputFileName);
        if (qFile->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qFile->write(netReply->readAll());
            qFile->close();
            downloadSuccessful = true;
        }
        delete qFile;
        qFile = 0;
    }

     netReply->deleteLater();
     currentDownloadReplies.removeAll(netReply);
     if (currentDownloadReplies.isEmpty())
     {
         currentDownloadQueue.clear();
         pause = true;
     }
     downloadProcessing = false;
}

void DownloadWorker::quit()
{
    networkMgr->deleteLater();
    emit finished();
}

void DownloadWorker::download(OQString &url, OQString &fileName)
{
    OQString protocol("http://");

    downloadProcessing = true;
    downloadSuccessful = false;

    downloadRequest request;
    if (url.left(4) == protocol.left(4))
        request.qUrl = url.getString();
    else
        request.qUrl = (protocol + url).getString();
    request.outputFile = fileName.getString();

    qDebug() << "Miner requested download: " << request.qUrl;

    if (!request.qUrl.isValid())
    {
        qDebug() << "ERROR - Invalid URL";
        PQString errorMessage;
        errorMessage << QString("Invalid URL: ") << url.getString();
        globals->logMsg(ErrorURL, errorMessage);
        downloadProcessing = false;
    }

    // Proceed with download
    QList<downloadRequest> newRequestList;
    newRequestList += request;
    appendToQueue(newRequestList);
}

bool DownloadWorker::processingDownload()
{
    return downloadProcessing;
}

bool DownloadWorker::lastDownloadSuccessful()
{
    return downloadSuccessful;
}
