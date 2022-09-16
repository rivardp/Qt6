#include "../Include/Internet/qDownloadWorkerBR2.h"

DownloadWorkerBR2::DownloadWorkerBR2(QObject *parent) : QObject(parent)
{
    qDebug() << "Constructor initialized";

    connect(this, SIGNAL(downloadQueueNotEmpty()), this, SLOT(processDownloads()));

    pause = true;
    queueOpen = true;
    downloadProcessing = false;
}

 DownloadWorkerBR2::~DownloadWorkerBR2()
{
    qDebug() << "Destructor run";
}

void DownloadWorkerBR2::initiate()
{
    qDebug() << "initiate() slot triggered from the thread.start()";

    networkMgr = new QNetworkAccessManager(this);
    connect(networkMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
}

void DownloadWorkerBR2::processDownloads()
{
    qDebug() << "Processing of downloads started";

    queueOpen = false;    // Don't allow additions while downloading

    QMutableListIterator<downloadRequest> iter(currentDownloadQueue);
    downloadRequest currentRequest;

    while (iter.hasNext())
    {
        currentRequest = iter.next();
        qUrl = currentRequest.qUrl;
        Outputs.HTMLfileName = currentRequest.outputFile;
        redirectURL.clear();
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

void DownloadWorkerBR2::setGlobalVars(GLOBALVARS &gv)
{
    globals = &gv;
}

void DownloadWorkerBR2::appendToQueue(const QList<downloadRequest> &newRequestList)
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


void DownloadWorkerBR2::startNextDownload()
{
    qDebug() << "Network request sent";

    QNetworkRequest networkRequest(qUrl);
    networkRequest.setRawHeader("User-Agent", "PenserCrawler/1.0 (Nokia; Qt)");
    if (followRedirects)
        networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QVariant(QNetworkRequest::UserVerifiedRedirectPolicy));

    if (POSTinstead)
    {
        networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        networkRequest.setRawHeader("Accept", "application/json");

        QSslConfiguration config = networkRequest.sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        networkRequest.setSslConfiguration(config);

        // Split off arguments from URL
        QByteArray messageBody;
        QString tempURL = qUrl.toString();
        int index = tempURL.lastIndexOf("?");
        qUrl = tempURL.left(index);
        networkRequest.setUrl(qUrl);
        messageBody.append(tempURL.right(tempURL.length() - index - 1));

        networkReply = networkMgr->post(networkRequest, messageBody);
    }
    else
        networkReply = networkMgr->get(networkRequest);

    connect(networkReply, SIGNAL(redirected(const QUrl&)), this, SLOT(handleRedirection(const QUrl&)));

    currentDownloadReplies.append(networkReply);
}

void DownloadWorkerBR2::downloadFinished(QNetworkReply *netReply)
{
    qDebug() << "Network returned finished() signal";

    if (netReply->error())
    {
        qDebug() << "Download Error: " << netReply->errorString();
        PQString errorMessage;
        errorMessage << QString("Error downloading ") << qUrl.toString();
        if (qUrl.toString().length() == 0)
        {
            int dummyBreak = 0;
        }
        errorMessage << QString(" : ") << netReply->errorString();
        globals->logMsg(ErrorConnection, errorMessage);
    }
    else
    {
        // Extra coding to use when a redirection is occurring
        if (netReply->error() == QNetworkReply::NoError)
        {
            QList<QByteArray> headerList;
            QUrl redirectUrl;
            QVariant location;
            int statusCode;

            statusCode = netReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            switch (statusCode)
            {
            case 100:   // Continue
                qDebug() << "Encountered status code == 100";
                break;

            case 301:   // Redirected
                redirectUrl = netReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
                break;

            case 302:   // Temporary redirection
                headerList = netReply->rawHeaderList();
                foreach(QByteArray head, headerList) {
                    qDebug() << head << ":" << netReply->rawHeader(head);
                }
                location = netReply->rawHeader("Location");
                if (location.isValid())
                    redirectURL = location.toString();

                break;

            default:
                break;

            }
        }

        // Determine charset encoding
        CHARSET charset = UTF8;
        OQString quote("\"");
        QByteArray encodedString = netReply->readAll();
        OQStream tempStream(QString::fromUtf8(encodedString));
        OQString word, nextChar;
        if (tempStream.moveTo(QString("charset=")))
        {
            nextChar = tempStream.peekAtNext(1);
            if (nextChar == quote)
                word = tempStream.readNextBetween(QUOTES);
            else
                word = tempStream.getUntil(quote.getString(), 20, true);

            if ((word == OQString("ISO-8859-1")) || (word == OQString("iso-8859-1")))
                charset = ISO88591;
            if ((word == OQString("ISO-8859-5")) || (word == OQString("iso-8859-5")))
                charset = ISO88595;
            if (word == OQString("windows-1252"))
                charset = Windows1252;
        }

        // Convert to UTF-8 where necessary
        QTextCodec *codec = nullptr;
        switch (charset)
        {
        case ISO88591:
            codec = QTextCodec::codecForName("ISO 8859-1");
            break;

        case ISO88595:
            codec = QTextCodec::codecForName("ISO 8859-5");
            break;

        case Windows1252:
            codec = QTextCodec::codecForName("Windows-1252");
            break;

        case UTF8:
        default:
            codec = QTextCodec::codecForName("UTF-8");
            break;
        }

        QString decodedString = codec->toUnicode(encodedString);
        QString cleanString = PQString(decodedString).replaceLigatures();

        if (cleanString.size() > 0)
        {
            qFile = new QFile(Outputs.HTMLfileName);
            if (qFile->open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream streamFileOut(qFile);
                streamFileOut.setCodec("UTF-8");
                streamFileOut << cleanString;
                streamFileOut.flush();

                qFile->close();
                downloadSuccessful = true;
            }
            delete qFile;
            qFile = nullptr;
        }
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

void DownloadWorkerBR2::handleRedirection(const QUrl &redirectedURL)
{
    if (followRedirects && redirectedURL.isValid())
        emit networkReply->redirectAllowed();
}

void DownloadWorkerBR2::quit()
{
    networkMgr->deleteLater();
    emit finished();
}

bool DownloadWorkerBR2::downloadAndProcess(PQString &url, downloadOutputs &targets, bool allowRedirects, bool POSTrequest)
{
    qDebug() << "downloadAndProcess call received for: " << targets.ID;

    downloadRequest request;
    request.qUrl = url.getString();
    request.outputFile = targets.HTMLfileName;

    // Deal with existing files
    bool alreadyReadIn = QFileInfo::exists(targets.HTMLfileName) && QFileInfo(targets.HTMLfileName).isFile();
    if (alreadyReadIn && targets.forceOverrides)
    {
        QFile oldFile (targets.HTMLfileName);
        oldFile.remove();
        alreadyReadIn = false;
    }

    if (!alreadyReadIn)
    {
        targets.fileAlreadyExists = false;
        downloadProcessing = true;
        downloadSuccessful = false;
        followRedirects = allowRedirects;
        POSTinstead = POSTrequest;

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
    else
    {
        targets.fileAlreadyExists = true;
        qDebug() << "Using existing HTML file for:  " << targets.ID;
    }

    addToURLlist(url, targets);

    return !alreadyReadIn;
}

bool DownloadWorkerBR2::processingDownload()
{
    return downloadProcessing;
}

bool DownloadWorkerBR2::lastDownloadSuccessful()
{
    return downloadSuccessful;
}

void DownloadWorkerBR2::addToURLlist(PQString &qUrl, downloadOutputs &outputs)
{
    qDebug() << "Made it to addFileToQueue " << qUrl.getString();

    bool exception = (outputs.providerID == 2015);

    //QTextStream outputStream(outputs.queue);
    QString entry(outputs.PDFfileName);
    entry.append("||");
    if (exception)
        entry.append(QString("https://schuler-lefebvrefuneralchapel.com/index.html"));
    else
        entry.append(qUrl.getString());
    entry.append("||");
    entry.append(outputs.dob);
    entry.append("||");
    entry.append(outputs.dod);
    entry.append("||");
    entry.append(outputs.maidenNames);
    entry.append("||");
    entry.append(outputs.pubDate);
    // outputStream << entry << endl;

    outputs.obitListEntry = entry;
}

