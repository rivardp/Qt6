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
    connect(networkMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
}

void DownloadWorkerBR3::processDownloads()
{
    qDebug() << "Processing of downloads started";

    queueOpen = false;    // Don't allow additions while downloading

    QMutableListIterator<downloadInstructions> iter(currentDownloadQueue);

    while (iter.hasNext())
    {
        currentRequest = iter.next();
        redirectURL.clear();
        iter.remove();
        startNextDownload();
    }

    queueOpen = true;
}

void DownloadWorkerBR3::setGlobalVars(GLOBALVARS &gv)
{
    globals = &gv;
}

void DownloadWorkerBR3::appendToQueue(downloadInstructions newInstruction)
{
    qDebug () << "Adding download request(s) to the queue";

    while (!queueOpen)
    {
        // Don't add anything to the queue until the existing queue is processed
    }

    currentDownloadQueue += newInstruction;
    emit downloadQueueNotEmpty();
 }


void DownloadWorkerBR3::startNextDownload()
{
    // Three different types of requests
    // 1. Simple GET
    // 2. Simple POST
    // 3. MultiPart POST including form data

    qDebug() << "Network request sent";

    QNetworkRequest networkRequest(currentRequest.qUrl);
    networkRequest.setRawHeader("User-Agent", "Crawl Obits");
    //networkRequest.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml,application/signed-exchange");
    if (currentRequest.allowRedirects)
        networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QVariant(QNetworkRequest::UserVerifiedRedirectPolicy));

    if (currentRequest.POSTformRequest.getLength() > 0)
    {
        // Multi-part POST including form-data
        multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        QHttpPart postData;
        QByteArray param1Name, param1Value;
        int index;

        index = currentRequest.POSTformRequest.getString().indexOf("=");
        param1Name.append(currentRequest.POSTformRequest.left(index).getString());
        param1Value.append(currentRequest.POSTformRequest.right(currentRequest.POSTformRequest.getLength() - index - 1).getString());

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
        if (currentRequest.POSTrequest)
        {
            // Simple POST
            networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            networkRequest.setRawHeader("Accept", "application/json");

            // Split off arguments from URL
            QByteArray messageBody;
            QString tempURL = currentRequest.qUrl.toString();
            int index = tempURL.lastIndexOf("?");
            currentRequest.qUrl = tempURL.left(index);
            networkRequest.setUrl(currentRequest.qUrl);
            messageBody.append(tempURL.right(tempURL.length() - index - 1));

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

    connect(networkReply, SIGNAL(redirected(const QUrl&)), this, SLOT(handleRedirection(const QUrl&)));

    currentDownloadReplies.append(networkReply);
}

void DownloadWorkerBR3::downloadFinished(QNetworkReply *netReply)
{
    qDebug() << "Network returned finished() signal";

    if (netReply->error())
    {
        qDebug() << "Download Error: " << netReply->errorString();
        PQString errorMessage;
        errorMessage << QString("Error downloading ") << currentRequest.qUrl.toString();
        if (currentRequest.qUrl.toString().length() == 0)
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

        // Fix problem with encoding from site
        if (((currentRequest.providerID == 1044) || (currentRequest.providerID == 2067)) && (currentRequest.providerKey == 1))
            charset = UTF8;

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
            qFile = new QFile(currentRequest.outputs.HTMLfileName);
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

void DownloadWorkerBR3::handleRedirection(const QUrl &redirectedURL)
{
    if (currentRequest.allowRedirects && redirectedURL.isValid())
        emit networkReply->redirectAllowed();
}

void DownloadWorkerBR3::quit()
{
    networkMgr->deleteLater();
    emit finished();
}

bool DownloadWorkerBR3::downloadAndProcess(downloadInstructions &di)
{
    qDebug() << "downloadAndProcess call received for: " << di.outputs.ID;

    // Deal with existing files
    bool alreadyReadIn = QFileInfo::exists(di.outputs.HTMLfileName) && QFileInfo(di.outputs.HTMLfileName).isFile();
    if (alreadyReadIn && di.outputs.forceOverrides)
    {
        QFile oldFile (di.outputs.HTMLfileName);
        oldFile.remove();
        alreadyReadIn = false;
    }

    if (!alreadyReadIn)
    {
        di.outputs.fileAlreadyExists = false;
        di.qUrl = di.url.getString();
        downloadProcessing = true;
        downloadSuccessful = false;

        if (di.providerID == 1021)
            di.qUrl = di.url.getString() + QString("?ckprm=1");

        qDebug() << "Miner requested download: " << di.url.getString();

        if (di.qUrl.isValid())
            appendToQueue(di);
        else
        {
            qDebug() << "ERROR - Invalid URL";
            PQString errorMessage;
            errorMessage << QString("Invalid URL: ") << di.url.getString();
            globals->logMsg(ErrorURL, errorMessage);
            downloadProcessing = false;
        }

        addToURLlist(di.url, di.outputs);
    }
    else
    {
        di.outputs.fileAlreadyExists = true;
        qDebug() << "Using existing HTML file for:  " << di.outputs.ID;

        if (globals->runStatus != 3)
            addToURLlist(di.url, di.outputs);
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

    bool exception = (outputs.providerID == 2015);
    exception = false;  // Issue changed

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
    entry.append(outputs.yob);
    entry.append("||");
    entry.append(outputs.dod);
    entry.append("||");
    entry.append(outputs.yod);
    entry.append("||");
    entry.append(outputs.maidenNames);
    entry.append("||");
    entry.append(outputs.ID);
    entry.append("||");
    entry.append(outputs.pubDate);
    entry.append("||");
    entry.append(QString::number(outputs.ageAtDeath));
    entry.append("||");
    entry.append(outputs.pcKey);
    entry.append("||");

    outputs.obitListEntry = entry;
}

