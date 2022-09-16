#include "../Include/Internet/qDownloadManager.h"

DownloadManager::DownloadManager(QObject *parent) : QObject(parent)
{
    qDebug() << "DownloadManager constructor initialized";

    // No need for QNetworkAccessManager
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

bool DownloadManager::downloadAndProcess(OQString &url, downloadOutputs &targets)
{
    Outputs = targets;
    qUrl.setUrl(url.getString());
    qDebug() << "downloadAndProcess call received for: " << Outputs.ID;

    bool alreadyReadIn = QFileInfo::exists(targets.HTMLfileName) && QFileInfo(targets.HTMLfileName).isFile();
    if (alreadyReadIn && targets.forceOverrides)
    {
        QFile oldFile (targets.HTMLfileName);
        oldFile.remove();
        alreadyReadIn = false;
    }

    if (!alreadyReadIn)
    {
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
            webView->setURL(qUrl);  // This just sets the variable in webView

            // The first 5 connect statements show order of processing
            connect(webView, SIGNAL(loadFinished(bool)), this, SLOT(confirmWebViewLoadedProperly(bool)));
                // webView does a bunch of processing until allTasksCompleted
            connect(webView, SIGNAL(allDoneWith(WebView*)), this, SLOT(cleanUp(WebView*)));
            connect(webView, SIGNAL(allTasksCompleted()), &timer, SLOT(stop())); // Stops timer to prevent logging a timeout
            connect(webView, SIGNAL(allTasksCompleted()), &loop, SLOT(quit()));

            // The remaining connect statements handle contingencies
            connect(this, SIGNAL(downloadOrProcessingError()), &loop, SLOT(quit()));
            connect(&timer, SIGNAL(timeout()), webView, SLOT(logTimeout()));
            //connect(webView, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )),this, SLOT(onSslErrors(QNetworkReply*, const QList<QSslError> & )));

            webView->skipHTMLcreation = false;
            webView->includeInList = true;
            targets.fileAlreadyExists = false;
            qDebug() << "Attempt to load webView:  " << Outputs.ID;
            timer.start(75000);     // 45 second maximum per download - Allows for subsequent loops to timeout first
            webView->setUrl(qUrl);  // This triggers the loading of the url
            // Further processing and tasks triggered by signals until quit() signal is emitted on completion or error
            loop.exec();            // Will stop at earlier of i) task completion or ii) timer timeout
        }
        else
        {
            qDebug() << "ERROR - Invalid URL";
            PQString errorMessage;
            errorMessage << QString("Invalid URL: ") << url.getString();
            globals->logMsg(ErrorURL, errorMessage);
        }
    }
    else
    {
        targets.fileAlreadyExists = true;
        qDebug() << "Using existing HTML file for:  " << Outputs.ID;

    }

    return !alreadyReadIn;
}

void DownloadManager::confirmWebViewLoadedProperly(bool success)
{
    if (success)
    {
        // Created to allow time to permit proper rendering
        // Determined not to be necessary
        qDebug() << "Successfully loaded: " << Outputs.ID;

        emit webView->loadingComplete(true);
    }
    else
    {
        qDebug() << "****************** HTML load failed - timeout expected:  " << Outputs.ID << "     *****************";
        emit webView->loadingComplete(true);
    }
}

void DownloadManager::cleanUp(WebView *wv)
{
    wv->deleteLater();
}

/*void DownloadManager::onSslErrors(QNetworkReply *reply, const QList<QSslError> & )
{
    reply->ignoreSSLerrors();
}*/






