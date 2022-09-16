#ifndef QDOWNLOADER_H
#define QDOWNLOADER_H


#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QCoreApplication>

#include "../Include/Penser/OString.h"
#include "../UpdateFuneralHomes/Include/globalvars.h"
//#include "../UpdateFuneralHomes/Include/qutilities.h"


class DOWNLOADER : public QObject
{
    Q_OBJECT

private:

   QCoreApplication *app;

   QNetworkAccessManager manager;
   QNetworkReply *netReply;

   QFile *file;
   QString qFileName;
   QByteArray downloadedData;
   QUrl qURL;
   bool downloadOK;

   GLOBALVARS *globals;

public:
   explicit DOWNLOADER(QObject *parent = 0);

   void download(OString &url, OString &fileName);
   bool downloadSuccessful() const;

   void setGlobalVars(GLOBALVARS &gv);

signals:

public slots:
   void processDownload(QNetworkReply *reply);
   void error(QNetworkReply::NetworkError code);
   void setupAfterThreadStarted();

};



#endif // QDOWNLOADER_H

