#include "../Include/Internet/qDownloadManager.h"

DownloadManager::DownloadManager(QObject *parent) : QObject(parent)
{
    qDebug() << "DownloadManager constructor initialized";

    FHprofile = QWebEngineProfile::defaultProfile();
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
    emit finished();
}

void DownloadManager::downloadAndProcess(OQString &url, downloadOutputs &targets)
{
    webPage = new WebPage(FHprofile, this);
    webPage->setOutputs(targets);

    QUrl potentialURL(url.getString());
    potentialURL.setScheme("http");
    if (potentialURL.isValid())
    {
        // Use an event loop to make sure all tasks are completed before moving on
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(webPage, SIGNAL(allTasksCompleted()), &timer, SLOT(stop())); // Stops timer to prevent logging a timeout
        connect(webPage, SIGNAL(allTasksCompleted()), &loop, SLOT(quit()));
        connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        connect(&timer, SIGNAL(timeout()), webPage, SLOT(logTimeout()));

        timer.start(10000);     // 10 second maximum per download
        webPage->load(potentialURL);
        loop.exec();            // Will stop at earlier of 10 secs or task completion
        timer.stop();

        qDebug() << "Miner requested download: " << url.getString();

        // Final processing is triggered by the loadFinished signal and completed by webPage class
        // A subsequent signal triggers cleanUP(Webpage*), which deletes the WebPage
    }
    else
    {
        qDebug() << "ERROR - Invalid URL";
        PQString errorMessage;
        errorMessage << QString("Invalid URL: ") << url.getString();
        globals->logMsg(ErrorURL, errorMessage);
    }
}

void DownloadManager::cleanUp(WebPage *wp)
{
    delete wp;
}




