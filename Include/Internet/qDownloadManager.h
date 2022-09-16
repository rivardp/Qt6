#ifndef QDOWNLOADMANAGER_H
#define QDOWNLOADMANAGER_H

#include <QObject>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QApplication>
#include <QTimer>

#include "../Include/Penser/OQString.h"
#include "../Include/Internet/qWebStructures.h"
#include "../Include/Internet/qWebview.h"
#include "../UpdateFuneralHomes/Include/globalVars.h"

class DownloadManager : public QObject
{
    Q_OBJECT

 public :
    explicit DownloadManager(QObject *parent = nullptr);
    ~DownloadManager();

    void setGlobalVars(GLOBALVARS &gv);
    bool downloadAndProcess(OQString &url, downloadOutputs &outputs);
    void quit();

    WebView *webView;

 private:
    downloadOutputs Outputs;
    GLOBALVARS *globals;        // Used for all output streams and logging
    QString HTMLstring;         // The actual string containing the entire file contents
    QUrl qUrl;                  // The url

 public slots:
    void confirmWebViewLoadedProperly(bool success);
    void cleanUp(WebView *wv);
    //void onSslErrors(QNetworkReply *reply, const QList<QSslError> & );

 signals:
    void finished();
    void downloadOrProcessingError();
};


#endif // QDOWNLOADMANAGER_H

