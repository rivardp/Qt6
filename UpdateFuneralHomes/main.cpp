#include <QApplication>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QTextCodec>
#include <QDebug>
#include <QTimer>
#include <QThread>


#include "UpdateFuneralHomes.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    /*******************************************/
    /*           Set Locale to UTF-8           */
    /*******************************************/

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    /*******************************************/
    /*           Run initial setup             */
    /*******************************************/

    GLOBALVARS globals;

    // Setup the MySQL connection with database manager 'db'
    if (!createConnection(globals.db, "deceasedUpdater"))
    {
        qDebug() << "Error connecting to SQL database 'death_audits'";
        globals.logMsg(ErrorConnection, QString("Error connecting to SQL database death_audits"));
        return 0;
    }
    else
    {
        qDebug() << "Connected to 'death_audits'...";
    }

    // Finish setup
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

    /*******************************************/
    /*  Create thread to handle all downloads  */
    /*******************************************/

    QThread* downloadThread = new QThread;
    DownloadWorker* www = new DownloadWorker();
    www->setGlobalVars(globals);
    www->moveToThread(downloadThread);

    QObject::connect(downloadThread, SIGNAL(started()), www, SLOT(initiate()));
    QObject::connect(www, SIGNAL(finished()), downloadThread, SLOT(quit()));
    QObject::connect(www, SIGNAL(finished()), www, SLOT(deleteLater()));
    QObject::connect(downloadThread, SIGNAL(finished()), downloadThread, SLOT(deleteLater()));

    downloadThread->start();

    /********************************************/
    /*  Create miner to manage all data mining  */
    /********************************************/

    MINER miner;
    miner.setDownloadWorker(www);
    miner.setGlobalVars(globals);

    QObject::connect(&miner, SIGNAL(finished()), &app, SLOT(quit()));
    QObject::connect(&app, SIGNAL(aboutToQuit()), &miner, SLOT(aboutToQuitApp()));

    QTimer::singleShot(10, &miner, SLOT(execute()));

    return app.exec();
}

