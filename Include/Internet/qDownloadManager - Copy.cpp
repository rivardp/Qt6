#include "../Include/Internet/qDownloadManager.h"

DownloadManager::DownloadManager(QObject *parent) : QObject(parent)
{
    qDebug() << "DownloadManager constructor initialized";

    networkMgr = new QNetworkAccessManager(this);
    connect(networkMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(processNetReplyResponse(QNetworkReply*)));
}

 DownloadManager::~DownloadManager()
{
    qDebug() << "DownloadManager destructor run";
}

void DownloadManager::setGlobalVars(GLOBALVARS &gv)
{
    globals = &gv;
}

void DownloadManager::quit()
{
    QTimer waitToQuit;
    waitToQuit.setSingleShot(true);
    waitToQuit.start(10000);
    qDebug() << "Timer just before quitting...";
    while (waitToQuit.isActive())
        QApplication::processEvents();

    emit finished();
}

void DownloadManager::downloadAndProcess(OQString &url, downloadOutputs &targets)
{
    Outputs = targets;
    qDebug() << "downloadAndProcess call received for: " << Outputs.ID;

    qUrl = url.getString();
    qUrl.setScheme("http");
    if (qUrl.isValid())
    {
        qDebug() << "Miner requested download: " << url.getString();

        // Use an event loop to make sure all tasks are completed before returning from this function
        QEventLoop loop;
        QTimer timer;
        connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.setSingleShot(true);

        webView = new WebView(this);
        webView->setOutputs(targets);
        webView->setUrl(qUrl);

        // The first 5 connect statements show order of processing
        connect(webView, SIGNAL(loadFinished(bool)), this, SLOT(confirmWebViewLoadedProperly(bool)));
            // webView does a bunch of processing until allTasksCompleted
        connect(webView, SIGNAL(allTasksCompleted()), &timer, SLOT(stop())); // Stops timer to prevent logging a timeout
        connect(webView, SIGNAL(allTasksCompleted()), &loop, SLOT(quit()));

        // The remaining connect statements handle contingencies
        connect(this, SIGNAL(downloadOrProcessingError()), &loop, SLOT(quit()));
        connect(&timer, SIGNAL(timeout()), webView, SLOT(logTimeout()));

        timer.start(30000);     // 30 second maximum per download - Allows for subsequent loops to timeout first
        startDownload(qUrl);
        // Further processing and tasks triggered by signals until quit() signal is emitted on completion or error
        loop.exec();            // Will stop at earlier of xx secs or task completion

    }
    else
    {
        qDebug() << "ERROR - Invalid URL";
        PQString errorMessage;
        errorMessage << QString("Invalid URL: ") << url.getString();
        globals->logMsg(ErrorURL, errorMessage);
    }
    webView->deleteLater();
}

void DownloadManager::startDownload(QUrl qUrl)
{
    qDebug() << "Network request sent - " << Outputs.ID;

    QNetworkRequest networkRequest(qUrl);
    networkReply = networkMgr->get(networkRequest);

    // Upon completion networkMgr will emit signal processNetReplyResponse
}

void DownloadManager::processNetReplyResponse(QNetworkReply *netReply)
{
    qDebug() << "Network returned finished() signal  " << Outputs.ID;

    if (netReply->error())
    {
        qDebug() << "Download Error: " << netReply->errorString();
        PQString errorMessage;
        errorMessage << QString("Error downloading ") << Outputs.ID;
        errorMessage << QString(" : ") << netReply->errorString();
        globals->logMsg(ErrorConnection, errorMessage);

        emit downloadOrProcessingError();
    }
    else
    {
        // First step is to save the HTML file
        HTMLstring = QString::fromUtf8(netReply->readAll());
        QFile *qFile = new QFile(Outputs.HTMLfileName);
        if (qFile->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream textStream(qFile);
            textStream << HTMLstring;
            qFile->close();

            // Second step is to load the webView
            qDebug() << "HTML file saved:  " << Outputs.ID;
            qDebug() << "Attempt to load webView:  " << Outputs.ID;

            //webView->page()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
            //webView->page()->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
            //webView->setUrl(qUrl);
            webView->setHtml(HTMLstring, qUrl);
            //webView->load(qUrl);

        }
        else
        {
            // While not necessarily fatal, treated as fatal error
            PQString errorMessage;
            errorMessage << QString("Unable to create HTML file: ") << Outputs.ID;
            globals->logMsg(ErrorConnection, errorMessage);

            emit downloadOrProcessingError();
        }

        delete qFile;
        qFile = 0;
   }
   netReply->deleteLater();
}

void DownloadManager::confirmWebViewLoadedProperly(bool success)
{
    if (success)
    {
        // Determined not to be necessary

        /*qDebug() << "Additional time allowance to permit rendering of load:  " << Outputs.ID;

        webView->update();
        // A second wait loop is required to allow the rendering to complete, which is later than loadFinished signal
        QTimer waitFor;
        waitFor.setSingleShot(true);
        waitFor.start(1000);
        qDebug() << "Timer started...  " << Outputs.ID;
        while (waitFor.isActive())
            QApplication::processEvents();
        qDebug() << "Timer ended:  " << Outputs.ID;*/

        emit webView->loadingComplete(true);
    }
    else
    {
        qDebug() << "HTML load failed - timeout expected:  " << Outputs.ID;
    }
}





