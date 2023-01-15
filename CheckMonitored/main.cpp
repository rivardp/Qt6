#include <QCoreApplication>
#include <QApplication>
#include <QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QDebug>
#include <iostream>

#include "../UpdateFuneralHomes/Include/globalVars.h"
#include "../UpdateFuneralHomes/Include/dataStructure.h"
#include "../UpdateFuneralHomes/Include/dataRecord.h"
#include "../UpdateFuneralHomes/Include/matchRecord.h"
#include "../Include/PMySQL/pMySQL.h"
#include "Include/CheckMonitored.h"
#include "Include/globals.h"


int main(int argc, char *argv[])
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

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
    /*     Set LastUpdated date for query      */
    /*******************************************/

    QDate lastUpdatedStartDate;
    bool needInput = true;
    bool createList = true;
    int input;

    while (needInput)
    {
        std::cout << "\n";
        std::cout << "Enter cutoff date for LastUpdated  (YYYYMMDD or 0) " ;
        std::cin >> input;

        if (input == 0)
        {
            lastUpdatedStartDate = QDate(1875,1,1);
            needInput = false;
        }

        if (needInput)
        {
            int yyyy, mm, dd;
            yyyy = input / 10000;
            mm = (input - yyyy * 10000) / 100;
            dd = input - yyyy * 10000 - mm * 100;
            lastUpdatedStartDate = QDate(yyyy, mm, dd);

            if (lastUpdatedStartDate.isValid() && (lastUpdatedStartDate <= globals.today))
                needInput = false;
            else
            {
                std::cin.clear(); // put us back in 'normal' operation mode
                std::cin.ignore(32767,'\n'); // and remove the bad input
            }
        }
    }

    needInput = true;
    while (needInput)
    {
        std::cout << "\n";
        std::cout << "Use existing ListOfDeceasedIDs?  (1 or 0) " ;
        std::cin >> input;

        if (input == 1)
            createList = false;

        needInput = false;
    }

    /*******************************************/
    /*     Create CSV file of deceasedIDs      */
    /*******************************************/

    QString filename("C:\\Obits\\Batch Read\\ListofDeceasedIDs.csv");
    QFile CSVfile(filename);
    MATCHRESULT matchScore;
    dataRecord dr, bestMatch;
    databaseSearches dbSearch;

    if (createList || !CSVfile.exists())
    {
        CSVfile.resize(0);
        createListOfDeceasedIDs(filename, lastUpdatedStartDate, globals);
    }

    /*******************************************/
    /*       Process and export records        */
    /*******************************************/

    QString exportFile("RankingResults.csv");
    int rankScore;

    if(CSVfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!CSVfile.atEnd())
        {
            bestMatch.clear();
            matchScore.clear();
            readLine(CSVfile, dr);
            dr = dbSearch.readRecord(dr.getDeceasedNumber(), dr.getLastName().getString(), &globals);
            rankScore = initialCheckRank(dr, bestMatch, matchScore);
            if (bestMatch.getDeceasedNumber() > 0)
            {
                dr.xport(exportFile, rankScore);
                bestMatch.xport(exportFile, rankScore);
            }
        }
    }
    else
    {
        PQString errorMessage;
        globals.logMsg(ErrorRunTime, errorMessage << "Could not open file: " << filename);
    }


    return 0;
}
