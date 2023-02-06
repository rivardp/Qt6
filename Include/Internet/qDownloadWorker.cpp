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

    QMutableListIterator<DOWNLOADREQUEST> iter(currentDownloadQueue);

    while (iter.hasNext())
    {
        currentRequest.clear();
        currentRequest = iter.next();
        redirectURL.clear();
        iter.remove();
        if (currentRequest.instructions.qUrl.isValid())  // Redundant check for  now - see download(url,file) for first check
        {
            networkMgr->clearConnectionCache();
            networkMgr->clearAccessCache();
            startNextDownload();
        }
        else
            qDebug() << "Invalid url encountered: " << currentRequest.instructions.qUrl;
    }

    queueOpen = true;
}

void DownloadWorker::setGlobalVars(GLOBALVARS &gv)
{
    globals = &gv;
}

void DownloadWorker::appendToQueue(const QList<DOWNLOADREQUEST> &newRequestList)
{
    qDebug () << "Adding download request(s) to the queue";

    while (!queueOpen)
    {
        // Don't add anything to the queue until the existing queue is processed
    }

    int i, end;
    for (end = newRequestList.size(), i = 0; i != end; ++i)
    {
         currentDownloadQueue += newRequestList;
    }

    emit downloadQueueNotEmpty();
 }


void DownloadWorker::startNextDownload()
{
    qDebug() << "Creating network request";

    QNetworkRequest networkRequest(currentRequest.instructions.qUrl);

    /******************/
    /*     TimeOut    */
    /******************/

    networkRequest.setTransferTimeout(30000);

    /******************/
    /*      Verb      */
    /******************/

    if (currentRequest.instructions.verb.length() == 0)
        currentRequest.instructions.verb = QString("GET");

    /******************/
    /* Configurations */
    /******************/

    if ((currentRequest.instructions.SSL == "true") && ((currentRequest.instructions.verb == "POST") || (currentRequest.instructions.verb == "OPTIONS")))
    {
        QSslConfiguration config = networkRequest.sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        networkRequest.setSslConfiguration(config);
    }

    /******************/
    /*   Attributes   */
    /******************/

    if (currentRequest.instructions.followRedirects)
    {
        //networkRequest.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QVariant(QNetworkRequest::UserVerifiedRedirectPolicy));
    }

    /******************/
    /*   RawHeaders   */
    /******************/

    // User-Agent
    QByteArray ua;
    if (currentRequest.instructions.User_Agent.length() > 0)
    {
        if (currentRequest.instructions.User_Agent == "STD")
            ua = QString("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.110 Safari/537.36").toUtf8();
        else
            ua = currentRequest.instructions.User_Agent.toUtf8();
    }
    else
        ua = QString(QString("Crawler Test ") + QString::number(globals->today.month()) + QString::number(globals->today.day())).toUtf8();
    networkRequest.setRawHeader("User-Agent", ua);

    // Accept
    QByteArray accept;
    if ((currentRequest.instructions.verb == "POST") && (currentRequest.instructions.Accept.length() == 0))
        accept = QString("application/json").toUtf8();
    else
    {
        if (currentRequest.instructions.Accept.length() > 0)
        {
            if (currentRequest.instructions.Accept == "ALL")
                accept = QString("*/*").toUtf8();
            else
            {
                if (currentRequest.instructions.Accept == "STD")
                    accept = QString("text/html,application/xhtml+xml,application/xml;q=0.8,application/signed-exchange;v=b3;q=0.9").toUtf8();
                else
                    accept = currentRequest.instructions.Accept.toUtf8();
            }
        }
        else
            accept = QString("text/html,application/xhtml+xml,application/xml;q=0.8,application/signed-exchange;v=b3;q=0.9").toUtf8();
    }
    networkRequest.setRawHeader("Accept", accept);

    // Encoding
    QByteArray encoding;
    if (currentRequest.instructions.Accept_Encoding.length() > 0)
    {
        if (currentRequest.instructions.Accept_Encoding == "ALL")
            encoding = QString("gzip, deflate, br").toUtf8();
        else
        {
            if (currentRequest.instructions.Accept_Encoding == "STD")
                encoding = QString("gzip, deflate").toUtf8();
            else
                encoding = currentRequest.instructions.Accept_Encoding.toUtf8();
        }
        networkRequest.setRawHeader("Accept-Encoding", encoding);
    }

    // Language
    QByteArray lang;
    if (currentRequest.instructions.Accept_Language.length() > 0)
    {
        if (currentRequest.instructions.Accept_Language == "ALL")
            lang = QString("en-CA,en-US,en,*").toUtf8();
        else
        {
            if (currentRequest.instructions.Accept_Language == "STD")
                lang = QString("en-US,en;q=0.9").toUtf8();
            else
                lang = currentRequest.instructions.Accept_Language.toUtf8();
        }
    }
    else
        lang = QString("en-CA,*").toUtf8();
    networkRequest.setRawHeader("Accept-Language", lang);

    // Allow Origin
    QByteArray allowOrigin;
    if (currentRequest.instructions.Access_Control_Allow_Origin.length() > 0)
    {
        if (currentRequest.instructions.Access_Control_Allow_Origin == "ALL")
            allowOrigin = QString("*").toUtf8();
        else
        {
            if (currentRequest.instructions.Access_Control_Allow_Origin == "STD")
                allowOrigin = QString("*").toUtf8();
            else
                allowOrigin = currentRequest.instructions.Access_Control_Allow_Origin.toUtf8();
        }
        networkRequest.setRawHeader("Access-Control-Allow-Origin", allowOrigin);
    }

    // Allow Methods
    QByteArray allowMethods;
    if (currentRequest.instructions.Access_Control_Allow_Methods.length() > 0)
    {
        if (currentRequest.instructions.Access_Control_Allow_Methods == "ALL")
            allowMethods = QString("GET, POST, OPTIONS").toUtf8();
        else
        {
            if (currentRequest.instructions.Access_Control_Allow_Methods == "STD")
                allowMethods = QString("GET, POST, OPTIONS").toUtf8();
            else
                allowMethods = currentRequest.instructions.Access_Control_Allow_Methods.toUtf8();
        }
        networkRequest.setRawHeader("Access-Control-Allow-Methods", allowMethods);
    }

    // Request Methods
    QByteArray requestMethods;
    if (currentRequest.instructions.Access_Control_Request_Method.length() > 0)
    {
        if (currentRequest.instructions.Access_Control_Request_Method == "ALL")
            requestMethods = QString("GET, POST, OPTIONS").toUtf8();
        else
        {
            if (currentRequest.instructions.Access_Control_Request_Method == "STD")
                requestMethods = QString("GET").toUtf8();
            else
                requestMethods = currentRequest.instructions.Access_Control_Request_Method.toUtf8();
        }
        //networkRequest.setRawHeader("Access-Contorl-Request-Methods", requestMethods);
        networkRequest.setRawHeader("Access-Control-Request-Method", requestMethods);
    }

    // Request Headers
    QByteArray requestHeaders;
    if (currentRequest.instructions.Access_Control_Request_Headers.length() > 0)
    {
        if (currentRequest.instructions.Access_Control_Request_Headers == "ALL")
            requestHeaders = QString("domainid").toUtf8();
        else
        {
            if (currentRequest.instructions.Access_Control_Request_Headers == "STD")
                requestHeaders = QString("domainid").toUtf8();
            else
                requestHeaders = currentRequest.instructions.Access_Control_Request_Headers.toUtf8();
        }
        networkRequest.setRawHeader("Access-Control-Request-Headers", requestHeaders);
    }

    // Security Fetch Mode
    QByteArray fetchMode;
    if (currentRequest.instructions.Sec_Fetch_Mode.length() > 0)
    {
        if (currentRequest.instructions.Sec_Fetch_Mode == "ALL")
            fetchMode = QString("cors").toUtf8();
        else
        {
            if (currentRequest.instructions.Sec_Fetch_Mode == "STD")
                fetchMode = QString("cors").toUtf8();
            else
                fetchMode = currentRequest.instructions.Sec_Fetch_Mode.toUtf8();
        }
        networkRequest.setRawHeader("Sec-Fetch-Mode", fetchMode);
    }

    // Security Fetch Site
    QByteArray fetchSite;
    if (currentRequest.instructions.Sec_Fetch_Site.length() > 0)
    {
        if (currentRequest.instructions.Sec_Fetch_Site == "ALL")
            fetchSite = QString("cross-site").toUtf8();
        else
        {
            if (currentRequest.instructions.Sec_Fetch_Site == "STD")
                fetchSite = QString("cross-site").toUtf8();
            else
                fetchSite = currentRequest.instructions.Sec_Fetch_Site.toUtf8();
        }
        networkRequest.setRawHeader("Sec-Fetch-Site", fetchSite);
    }

    // Security Fetch Dest
    QByteArray fetchDest;
    if (currentRequest.instructions.Sec_Fetch_Dest.length() > 0)
    {
        if (currentRequest.instructions.Sec_Fetch_Dest == "ALL")
            fetchDest = QString("empty").toUtf8();
        else
        {
            if (currentRequest.instructions.Sec_Fetch_Dest == "STD")
                fetchDest = QString("empty").toUtf8();
            else
                fetchDest = currentRequest.instructions.Sec_Fetch_Dest.toUtf8();
        }
        networkRequest.setRawHeader("Sec-Fetch-Dest", fetchDest);
    }

    // Headers
    if (currentRequest.instructions.ContentTypeHeader.length() > 0)
        networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, currentRequest.instructions.ContentTypeHeader.toUtf8());
    else
    {
        if (currentRequest.instructions.verb == "POST")
            networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/x-www-form-urlencoded").toUtf8());
    }

    // Domain ID
    if (currentRequest.instructions.DomainID.length() > 0)
        networkRequest.setRawHeader("DomainId", currentRequest.instructions.DomainID.toUtf8());

    // Origin
    if (currentRequest.instructions.Origin.length() > 0)
        networkRequest.setRawHeader("Origin", currentRequest.instructions.Origin.toUtf8());

    // Referer
    if (currentRequest.instructions.Referer.length() > 0)
        networkRequest.setRawHeader("Referer", currentRequest.instructions.Referer.toUtf8());

    // Instance
    if (currentRequest.instructions.Instance.length() > 0)
        networkRequest.setRawHeader("Instance", currentRequest.instructions.Instance.toUtf8());

    // Authorization
    if (currentRequest.instructions.Authorization.length() > 0)
        networkRequest.setRawHeader("Authorization", currentRequest.instructions.Authorization.toUtf8());

    // X-RequestedWith
    if (currentRequest.instructions.X_Requested_With.length() > 0)
        networkRequest.setRawHeader("X-Requested-With", currentRequest.instructions.X_Requested_With.toUtf8());

    // X-Origin
    if (currentRequest.instructions.X_Origin.length() > 0)
        networkRequest.setRawHeader("X-Origin", currentRequest.instructions.X_Origin.toUtf8());

    // Process the actual request
    if (currentRequest.instructions.verb == "GET")
        networkReply = networkMgr->get(networkRequest);
    else
    {
        if (currentRequest.instructions.verb == "OPTIONS")
        {
            networkMgr->sendCustomRequest(networkRequest, "OPTIONS", QByteArray());
        }
        else
        {
            if (currentRequest.instructions.verb == "MULTIPART")
            {
                QHttpMultiPart *multiPart = currentRequest.instructions.multiPartList.takeFirst();
                networkReply = networkMgr->post(networkRequest, multiPart);
                multiPart->setParent(networkReply);
            }
            else
            {
                // Normal POST
                if (currentRequest.instructions.payload.length() > 0)
                {
                    networkRequest.setHeader(QNetworkRequest::ContentLengthHeader, currentRequest.instructions.payload.count());

                    networkReply = networkMgr->post(networkRequest, currentRequest.instructions.payload);
                }
                else
                {
                    QByteArray messageBody;
                    if (currentRequest.instructions.X_Origin.length() == 0)
                    {
                        // Split off arguments from URL
                        QString tempURL = currentRequest.instructions.qUrl.toString();
                        int index = tempURL.lastIndexOf("?");
                        currentRequest.instructions.qUrl = tempURL.left(index);
                        networkRequest.setUrl(currentRequest.instructions.qUrl);
                        messageBody.append(tempURL.right(tempURL.length() - index - 1).toUtf8());
                    }
                    networkRequest.setHeader(QNetworkRequest::ContentLengthHeader, messageBody.count());

                    networkReply = networkMgr->post(networkRequest, messageBody);
                }
            }
        }
    }

    qDebug() << "Network request sent";
    globals->totalDownloads++;

    if (!(currentRequest.instructions.verb == "OPTIONS"))
        connect(networkReply, SIGNAL(redirected(const QUrl)), this, SLOT(handleRedirection(const QUrl)));

    currentDownloadReplies.append(networkReply);
}

void DownloadWorker::downloadFinished(QNetworkReply *netReply)
{
    qDebug() << "Network returned finished() signal";
    qDebug() << " ";

    if (netReply->error())
    {
        qDebug() << "Download Error: " << netReply->errorString();
        PQString errorMessage;
        errorMessage << QString("Error downloading ") << currentRequest.instructions.providerID << " - " << currentRequest.instructions.providerKey << " " << currentRequest.instructions.qUrl.toString();
        if ((currentRequest.instructions.qUrl.toString().length() == 0) || (currentRequest.instructions.qUrl.toString().right(3) == QString("php")))
            errorMessage << " " << currentRequest.outputs.downloadFileName;
        errorMessage << QString(" : ") << netReply->errorString();
        globals->logMsg(ErrorConnection, errorMessage);
    }
    else
    {
        // Extra coding to use when a redirection is occurring
        if (netReply->error() == QNetworkReply::NoError)
        {
            QList<QByteArray> headerList;
            QVariant location;
            int statusCode;

            statusCode = netReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            switch (statusCode)
            {
            case 100:   // Continue
                qDebug() << "Encountered status code == 100";
                break;

            case 301:   // Redirected
                redirectURL = netReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl().toString();
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

            if (word == OQString("us-ascii"))
                charset = USASCII;
            if ((word == OQString("ISO-8859-1")) || (word == OQString("iso-8859-1")))
                charset = ISO88591;
            if ((word == OQString("ISO-8859-5")) || (word == OQString("iso-8859-5")))
                charset = ISO88595;

            // problem with 1144 with charset commented out
            if (tempStream.moveTo("-->", 10))
                charset = ISO88591;
        }

        switch (charset)
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

        QString decodedString = toUtf16(encodedString);
        QString cleanString = PQString(decodedString).replaceLigatures();

        if (decodedString.size() > 0)
        {
            qFile = new QFile(currentRequest.outputs.downloadFileName);
            if (qFile->open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream streamFileOut(qFile);
                streamFileOut << decodedString;
                streamFileOut.flush();

                qFile->close();
                downloadSuccessful = true;
            }
            delete qFile;
            qFile = nullptr;
        }
        else
        {
            if (currentRequest.instructions.verb == "OPTIONS")
                downloadSuccessful = true;
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

void DownloadWorker::handleRedirection(const QUrl &redirectedURL)
{
    if (currentRequest.instructions.followRedirects && redirectedURL.isValid())
        emit networkReply->redirectAllowed();
}

void DownloadWorker::quit()
{
    networkMgr->deleteLater();
    emit finished();
}

void DownloadWorker::download(DOWNLOADREQUEST &dr)
{
    downloadProcessing = true;
    downloadSuccessful = false;

    if (!dr.instructions.qUrl.isValid() && (dr.instructions.url.getLength() > 0))
        dr.instructions.qUrl.setUrl(dr.instructions.url.getString());

    DOWNLOADREQUEST request = dr;

    qDebug() << "Miner requested download: " << request.instructions.qUrl;

    if (!request.instructions.qUrl.isValid())
    {
        qDebug() << "ERROR - Invalid URL";
        PQString errorMessage;
        errorMessage << QString("Invalid URL: ") << request.instructions.url.getString();
        globals->logMsg(ErrorURL, errorMessage);
        downloadProcessing = false;
    }

    // Proceed with download
    QList<DOWNLOADREQUEST> newRequestList;
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

bool DownloadWorker::isGzipped(QNetworkReply *reply)
{
    bool isGzipped = false;

    QList rhplist = reply->rawHeaderPairs();
    std::pair<QByteArray, QByteArray> pair;
    QListIterator<std::pair<QByteArray, QByteArray>> iter(rhplist);
    while (iter.hasNext() && !isGzipped)
    {
      pair = iter.next();
      if ((pair.first=="Content-Encoding") && (pair.second=="gzip"))
        isGzipped = true ;
    }
    return isGzipped;
}
