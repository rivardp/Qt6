#ifndef QDOWNLOADMANAGER_H
#define QDOWNLOADMANAGER_H

#include <QObject>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>

#include "../Include/Penser/OQString.h"
#include "../Include/Internet/qWebStructures.h"
#include "../Include/Internet/qWebPage.h"
#include "../UpdateFuneralHomes/Include/globalVars.h"

class DownloadManager : public QObject
{
    Q_OBJECT

 public :
    explicit DownloadManager(QObject *parent = 0);
    ~DownloadManager();

    void setGlobalVars(GLOBALVARS &gv);
    void downloadAndProcess(OQString &url, downloadOutputs &outputs);
    void quit();

    QWebEngineProfile *FHprofile;
    WebPage *webPage;

 private:
    GLOBALVARS *globals;            // Used for all output streams and logging

 public slots:
    void cleanUp(WebPage*);

 signals:
    void finished();
};


#endif // QDOWNLOADMANAGER_H

