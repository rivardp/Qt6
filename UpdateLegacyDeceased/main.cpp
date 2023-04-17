#include <QApplication>
#include <QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QDebug>

#include "../UpdateFuneralHomes/Include/globalVars.h"
#include "../Include/PMySQL/pMySQL.h"
#include "../Include/Internet/qDownloadWorker.h"
#include "Include/dataMiningULD.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Intent here is to build a program under an independent thread that could be added to
    // the current UpdateLegacyDeceased executable as a separate thread if desired down the road

    /*******************************************/
    /*           Run initial setup             */
    /*******************************************/

    GLOBALVARS globals;

    // Start setup
    if (!globals.runSetups())
    {
        qDebug() << "Error in setup process";
        globals.logMsg(ErrorRunTime, QString("Error in setup process"));
        return 0;
    }
    else
    {
        qDebug() << "Setup completed successfully...";
    }

    // Setup the MySQL connection with database manager 'db'
    if (!createConnection(QString("deceasedLegacyUpdater")))
    {
        qDebug() << "Error connecting to SQL database 'death_audits'";
        globals.logMsg(ErrorConnection, QString("Error connecting to SQL database death_audits"));
        return 0;
    }
    else
    {
        qDebug() << "Connected to 'death_audits'...";
    }

    /****************************************/
    /*     Record if part of Batch Run      */
    /****************************************/

    if (globals.batchRunFlag)
    {
        QFile *file = new QFile(globals.batchRunFileName);
        QTextStream *outputStream = nullptr;

        if(file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            outputStream = new QTextStream(file);
            *outputStream << QString("UpdateLegacyDeceased commenced at ") << QDateTime::currentDateTime().toString("h:mm") << Qt::endl;
        }

        delete outputStream;
        file->close();
    }

    /*******************************************/
    /*  Create thread to handle all downloads  */
    /*******************************************/

    QThread* downloadThreadLegacy = new QThread;
    DownloadWorker* wwwLegacy = new DownloadWorker();
    wwwLegacy->setGlobalVars(globals);
    wwwLegacy->moveToThread(downloadThreadLegacy);

    QObject::connect(downloadThreadLegacy, SIGNAL(started()), wwwLegacy, SLOT(initiate()));
    QObject::connect(wwwLegacy, SIGNAL(finished()), downloadThreadLegacy, SLOT(quit()));
    QObject::connect(wwwLegacy, SIGNAL(finished()), wwwLegacy, SLOT(deleteLater()));
    QObject::connect(downloadThreadLegacy, SIGNAL(finished()), downloadThreadLegacy, SLOT(deleteLater()));

    downloadThreadLegacy->start();

    /********************************************/
    /*  Create miner to manage all data mining  */
    /********************************************/

    MINER miner;
    miner.setDownloadWorker(wwwLegacy);
    miner.setGlobalVars(globals);

    QObject::connect(&miner, SIGNAL(finished()), &app, SLOT(quit()));
    QObject::connect(&app, SIGNAL(aboutToQuit()), &miner, SLOT(aboutToQuitApp()));

    QTimer::singleShot(10, &miner, SLOT(execute()));

    return app.exec();

}
