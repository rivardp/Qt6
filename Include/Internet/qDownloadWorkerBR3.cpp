#include "../Include/Internet/qDownloadWorkerBR3.h"

DownloadWorkerBR3::DownloadWorkerBR3(QObject *parent) : QObject(parent)
{
    qDebug() << "Constructor initialized";

    connect(this, SIGNAL(downloadQueueNotEmpty()), this, SLOT(processDownloads()));

    pause = true;
    queueOpen = true;
    downloadProcessing = false;
}

 DownloadWorkerBR3::~DownloadWorkerBR3()
{
    qDebug() << "Destructor run";
}

void DownloadWorkerBR3::initiate()
{
    qDebug() << "initiate() slot triggered from the thread.start()";

    networkMgr = new QNetworkAccessManager(this);
    QObject::connect(networkMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
}

void DownloadWorkerBR3::processDownloads()
{
    qDebug() << "Processing of downloads started";

    queueOpen = false;    // Don't allow additions while downloading

    QMutableListIterator<DOWNLOADREQUEST> iter(currentDownloadQueue);

    while (iter.hasNext())
    {
        currentRequest = iter.next();
        redirectURL.clear();
        iter.remove();
        networkMgr->clearConnectionCache();
        //networkMgr->clearAccessCache();
        startNextDownload();
    }

    queueOpen = true;
}

void DownloadWorkerBR3::setGlobalVars(GLOBALVARS &gv)
{
    globals = &gv;
}

void DownloadWorkerBR3::appendToQueue(DOWNLOADREQUEST newRequest)
{
    qDebug () << "Adding download request(s) to the queue";

    while (!queueOpen)
    {
        // Don't add anything to the queue until the existing queue is processed
    }

    currentDownloadQueue += newRequest;
    emit downloadQueueNotEmpty();
 }


void DownloadWorkerBR3::startNextDownload()
{
    // Three different types of requests
    // 1. Simple GET
    // 2. Simple POST
    // 3. MultiPart POST including form data

    qDebug() << "Network request sent";

    QNetworkRequest networkRequest(currentRequest.instructions.qUrl);
    networkRequest.setTransferTimeout(30000);

    networkRequest.setRawHeader("User-Agent", "Crawl Obits");
    //networkRequest.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml,application/signed-exchange");
    if (currentRequest.instructions.followRedirects)
        networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QVariant(QNetworkRequest::UserVerifiedRedirectPolicy));

    if (currentRequest.instructions.POSTformRequest.length() > 0)
    {
        // Multi-part POST including form-data
        multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        QHttpPart postData;
        QByteArray param1Name, param1Value;
        int index;

        index = currentRequest.instructions.POSTformRequest.indexOf("=");
        param1Name.append(currentRequest.instructions.POSTformRequest.left(index).toUtf8());
        param1Value.append(currentRequest.instructions.POSTformRequest.right(currentRequest.instructions.POSTformRequest.length() - index - 1).toUtf8());

        postData.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name="+param1Name));
        postData.setBody(param1Value);
        multiPart->append(postData);

        QSslConfiguration config = networkRequest.sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        networkRequest.setSslConfiguration(config);

        networkReply = networkMgr->post(networkRequest, multiPart);
        multiPart->setParent(networkReply); // delete the multiPart with the reply
    }
    else
    {
        if (currentRequest.instructions.verb == QString("POST"))
        {
            // Simple POST
            networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            networkRequest.setRawHeader("Accept", "application/json");

            // Split off arguments from URL
            QByteArray messageBody;
            QString tempURL = currentRequest.instructions.qUrl.toString();
            int index = tempURL.lastIndexOf("?");
            currentRequest.instructions.qUrl = tempURL.left(index);
            networkRequest.setUrl(currentRequest.instructions.qUrl);
            messageBody.append(tempURL.right(tempURL.length() - index - 1).toUtf8());

            QSslConfiguration config = networkRequest.sslConfiguration();
            config.setPeerVerifyMode(QSslSocket::VerifyNone);
            networkRequest.setSslConfiguration(config);

            networkReply = networkMgr->post(networkRequest, messageBody);
        }
        else
        {
            // Simple GET
            networkReply = networkMgr->get(networkRequest);
        }
    }

    QObject::connect(networkReply, SIGNAL(redirected(const QUrl&)), this, SLOT(handleRedirection(const QUrl&)));

    currentDownloadReplies.append(networkReply);
}

void DownloadWorkerBR3::downloadFinished(QNetworkReply *netReply)
{
    qDebug() << "Network returned finished() signal";

    if (netReply->error())
    {
        qDebug() << "Download Error: " << netReply->errorString();
        PQString errorMessage;
        errorMessage << QString("Error downloading ") << currentRequest.instructions.providerID << " - " << currentRequest.instructions.providerKey << " " << currentRequest.instructions.qUrl.toString();
        if (currentRequest.instructions.qUrl.toString().length() == 0)
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
            case 303:
                headerList = netReply->rawHeaderList();
                foreach(QByteArray head, headerList) {
                    qDebug() << head << ":" << netReply->rawHeader(head);
                }
                location = netReply->rawHeader("Location");
                if (location.isValid())
                    redirectURL = location.toString();
                if (statusCode == 303)
                {
                    currentRequest.instructions.verb = QString("GET");
                    redirectURL.clear();
                }
                break;

            default:
                break;

            }
        }

        // Determine charset encoding
        QStringConverter::Encoding encoding;

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

            // problem with 1144 with charset commented out
            if (tempStream.moveTo("-->", 10))
                charset = ISO88591;
        }

        // Fix problem with encoding from site
        if (((currentRequest.instructions.providerID == 1044) || (currentRequest.instructions.providerID == 2067)) && (currentRequest.instructions.providerKey == 1))
            charset = UTF8;
        if (currentRequest.instructions.providerID == 1144)
            charset = ISO88591;

        switch(charset)
        {
        case ISO88591:
        case ISO88595:
            encoding = QStringConverter::Latin1;
            break;

        case Windows1252:
        case USASCII:
            encoding = QStringConverter::System;
            break;

        default:
            encoding = QStringConverter::Utf8;
        }

        //std::optional<QStringConverter::Encoding> encoding = QStringConverter::encodingForHtml(encodedString);
        auto toUtf16 = QStringDecoder(encoding);

        // Fix problem with encoding from site
        /*if (((currentRequest.instructions.providerID == 1044) || (currentRequest.instructions.providerID == 2067)) && (currentRequest.instructions.providerKey == 1))
            charset = UTF8;
        if (currentRequest.instructions.providerID == 1144)
            charset = ISO88591;*/

        QString decodedString = toUtf16(encodedString);
        QString cleanString = PQString(decodedString).replaceLigatures();

        if (cleanString.size() > 0)
        {
            qFile = new QFile(currentRequest.outputs.HTMLfileName);
            if (qFile->open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream streamFileOut(qFile);
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

void DownloadWorkerBR3::handleRedirection(const QUrl &redirectedURL)
{
    if (currentRequest.instructions.followRedirects && redirectedURL.isValid())
        emit networkReply->redirectAllowed();
}

void DownloadWorkerBR3::quit()
{
    networkMgr->deleteLater();
    emit finished();
}

bool DownloadWorkerBR3::downloadAndProcess(DOWNLOADREQUEST &downloadRequest)
{
    qDebug() << "downloadAndProcess call received for: " << downloadRequest.outputs.ID;

    // Deal with existing files
    bool alreadyReadIn = QFileInfo::exists(downloadRequest.outputs.HTMLfileName) && QFileInfo(downloadRequest.outputs.HTMLfileName).isFile();
    if (alreadyReadIn && downloadRequest.outputs.forceOverrides)
    {
        QFile oldFile (downloadRequest.outputs.HTMLfileName);
        oldFile.remove();
        alreadyReadIn = false;
    }

    if (!alreadyReadIn)
    {
        downloadRequest.outputs.fileAlreadyExists = false;
        downloadRequest.instructions.qUrl = downloadRequest.instructions.url.getString();
        downloadProcessing = true;
        downloadSuccessful = false;

        if (downloadRequest.instructions.providerID == 1021)
            downloadRequest.instructions.qUrl = downloadRequest.instructions.url.getString() + QString("?ckprm=1");

        qDebug() << "Miner requested download: " << downloadRequest.instructions.url.getString();

        if (downloadRequest.instructions.qUrl.isValid())
            appendToQueue(downloadRequest);
        else
        {
            qDebug() << "ERROR - Invalid URL";
            PQString errorMessage;
            errorMessage << QString("Invalid URL: ") << downloadRequest.instructions.url.getString();
            globals->logMsg(ErrorURL, errorMessage);
            downloadProcessing = false;
        }

        addToURLlist(downloadRequest.instructions.url, downloadRequest.outputs);
    }
    else
    {
        downloadRequest.outputs.fileAlreadyExists = true;
        qDebug() << "Using existing HTML file for:  " << downloadRequest.outputs.ID;

        if (globals->runStatus != 3)
            addToURLlist(downloadRequest.instructions.url, downloadRequest.outputs);
    }

    return !alreadyReadIn;
}

bool DownloadWorkerBR3::processingDownload()
{
    return downloadProcessing;
}

bool DownloadWorkerBR3::lastDownloadSuccessful()
{
    return downloadSuccessful;
}

void DownloadWorkerBR3::addToURLlist(PQString &qUrl, downloadOutputs &outputs)
{
    qDebug() << "Made it to addFileToQueue " << qUrl.getString();

    outputs.createObitEntry(qUrl.getString());
}

void DownloadWorkerBR3::delay(int seconds)
{
    QTime dieTime= QTime::currentTime().addSecs(seconds);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

