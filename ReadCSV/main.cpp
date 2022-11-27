#include <QCoreApplication>
#include <QApplication>
#include <QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
//#include <QTextCodec>
#include <QDebug>
#include <iostream>

#include "../UpdateFuneralHomes/Include/globalVars.h"
//#include "../UpdateFuneralHomes/Include/dataStructure.h"
#include "../UpdateFuneralHomes/Include/dataRecord.h"
#include "../UpdateFuneralHomes/Include/matchRecord.h"
#include "../Include/PMySQL/pMySQL.h"
#include "Include/readCSV.h"
#include "Include/globals.h"


int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    /*******************************************/
    /*       Override default test mode        */
    /*******************************************/

    int choice;
    std::cout << "Do you want to actually load records?  (1 = Yes) " ;
    std::cin >> choice;
    if (choice == 1)
        testMode = false;

    /*******************************************/
    /*           Run initial setup             */
    /*******************************************/

    GLOBALVARS globals;

    // Setup the MySQL connection with database manager 'db'
    if (!createConnection(QString("deceasedUpdater")))
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
    /*           Open and read file            */
    /*******************************************/

    QString filename("C:\\Obits\\Batch Read\\CSVtoBeLoaded.csv");
    QFile CSVfile(filename);

    dataRecord dr;
    globals.globalDr = &dr;
    dr.setGlobals(globals);

    bool newRec;
    MATCHKEY matchKeyResult;
    unsigned int matchCounts[22];
    for (int i = 0; i < 22; i++)
        matchCounts[i] = 0;

    if(CSVfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!CSVfile.atEnd())
        {
            readLine(CSVfile, dr, globals);
            if ((dr.getLastName() != PQString("Last Name") && (dr.getPreviouslyLoaded() != 999) && (dr.getID().getLength() > 0)))  // Don't load header record or records coded as "999"
            {
                recsRead++;
                if (recsRead % 100 == 0)
                    qDebug() << recsRead << " records read";
                dr.adjustHyphenatedLastNames();
                if (dr.getPreviouslyLoaded() == 900)
                    dr.standardizeBadRecord();
                if(loadRecord(dr, matchKeyResult, newRec))
                {
                    if (!testMode)
                    {
                        recsLoaded++;
                        //qDebug() << "Processed up to record: " << recsLoaded;
                        if ((dr.getPreviouslyLoaded() % 100) == 0)
                        {
                            updateFirstNameDB(dr);
                            namesLoaded++;
                        }

                        if (newRec)
                            recsNew++;
                    }
                }

                matchCounts[static_cast<int>(matchKeyResult)]++;
            }
        }
    }
    else
    {
        PQString errorMessage;
        globals.logMsg(ErrorRunTime, errorMessage << "Could not open file: " << filename);
    }

    // Output statistics
    PQString stats;
    globals.logMsg(AuditListing, stats << "Total records read: " << recsRead);
    qDebug() << stats.getString();
    stats.clear();
    globals.logMsg(AuditListing, stats << "Total records loaded: " << recsLoaded);
    qDebug() << stats.getString();
    stats.clear();
    globals.logMsg(AuditListing, stats << "Total names loaded: " << namesLoaded);
    qDebug() << stats.getString();
    stats.clear();
    globals.logMsg(AuditListing, stats << "New unique deceased loaded: " << recsNew);
    qDebug() << stats.getString();
    for (int i = 0; i < MatchTypes.size(); i++)
    {
        stats.clear();
        stats << "Number of records with " << MatchTypes.at(i) << ": " << matchCounts[i];
        if (i >= mkDODdobAltName)
            stats << "  ** loaded **";
        globals.logMsg(AuditListing, stats);
    }

    return 0;
}
