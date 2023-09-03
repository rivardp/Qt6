// BatchProcessLegacy

#include <QApplication>
#include <QTimer>
#include <QLibrary>
#include <iostream>

#include "../UpdateFuneralHomes/Include/globalVars.h"
#include "Include/dataMiningBPL.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GLOBALVARS globals;

    // Setup the MySQL connection with database manager 'db'
    if (!createConnection("deceasedUpdater"))
    {
        qDebug() << "Error connecting to SQL database 'death_audits'";
        globals.logMsg(ErrorConnection, QString("Error connecting to SQL database death_audits"));
        return 0;
    }
    else
    {
        qDebug() << "Connected to 'death_audits'...";
    }

    // Setup specific client target attributes
    globals.run = BatchProcess;
    globals.clientBaseDir = QString("");
    globals.clientKey = 0;

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
            *outputStream << QString("BatchProcess commenced at ") << QDateTime::currentDateTime().toString("h:mm") << Qt::endl;
        }

        delete outputStream;
        file->close();

        globals.forceYearFlag = true;
    }
    else
    {
        int choice;
        std::cout << "Do you want to suppress forced year on partial dates for historical runs?  (1 = Yes) " ;
        std::cin >> choice;
        if (choice == 1)
            globals.forceYearFlag = false;
        else
            globals.forceYearFlag = true;
    }

    /****************************************************/
    /*  Create miner to manage all data mining          */
    /****************************************************/

    MINER miner;
    miner.setGlobalVars(globals);

    QObject::connect(&miner, SIGNAL(finished()), &app, SLOT(quit()));
    QObject::connect(&app, SIGNAL(aboutToQuit()), &miner, SLOT(aboutToQuitApp()));

    QTimer::singleShot(10, &miner, SLOT(execute()));

    return app.exec();
}