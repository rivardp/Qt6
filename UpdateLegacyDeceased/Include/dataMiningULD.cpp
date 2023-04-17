#include "Include/dataMiningULD.h"

MINER::MINER(QObject *parent) : QObject(parent)
{
    // get the instance of the main application
    app = QCoreApplication::instance();

    // any other additional setup here
}

void MINER::execute()
{
    qDebug() << "Data mining has started";

   mineData();

   quit();
}

void MINER::setDownloadThread(QThread *thread)
{
    downloadThread = thread;
}

void MINER::setDownloadWorker(DownloadWorker *downloadworker)
{
    www = downloadworker;
}

void MINER::setGlobalVars(GLOBALVARS &gv)
{
    globals = &gv;
}

void MINER::quit()
{
    // you can do some cleanup here
    // then do emit finished to signal CoreApplication to quit
    emit finished();
}

// shortly after quit is called the CoreApplication will signal this routine
// this is a good place to delete any objects that were created in the
// constructor and/or to stop any threads
void MINER::aboutToQuitApp()
{
    // stop threads
    // sleep(1);   // wait for threads to stop.
    // delete any objects
    // flush and close all streams
}

void MINER::mineData()
{
    /*******************************************/
    /*  Determine run type based on arguments  */
    /*******************************************/

    // From QtCreator, no arguments provided (by choice), thus defaulting to funeral homes where fhRunStatus = 2
    // For production version, a single argument of "1" will be provided, overriding the fhRunStatus

    int argValue;
    QStringList argList = QCoreApplication::arguments();
    if (argList.size() == 1)
        argValue = 2;
    else
    {
        QString argString = argList.at(1);
        argValue = argString.toInt();
    }
    globals->runStatus = argValue;

    DOWNLOADREQUEST downloadRequest;
    RECORD record;
    QList<RECORD> records;
    PAGEVARIABLES pageVariables;
    pageVariables.providerID = 1;
    pageVariables.providerKey = 0;
    //pageVariables.latestDODdate = lastDODdate;
    //pageVariables.lastRunDate = lastRunDate;
    //pageVariables.priorDODdate = lastDODdate;
    pageVariables.firstAvailableDate = QDate(2020,1,1);
    //pageVariables.queryTargetDate = globals->today;
    //pageVariables.lastID = lastID;
    //pageVariables.sequentialID = (fhSequentialId == QString("Y"));
    //pageVariables.fhURLid = fhURLid;
    //pageVariables.fhURLidDivider = fhURLidDivider;
    //pageVariables.alphabeticalListing = (fhAlphabeticalListing == QString("Y"));
    pageVariables.paginatedResult = false;
    pageVariables.usePubDateForCutOff = false;

    PQString baseURL = QString("https://www.legacy.com/ca/obituaries/thestar/name/george-fehlings-obituary?id=");

    downloadRequest.instructions.providerID = pageVariables.providerID;
    downloadRequest.instructions.providerKey = pageVariables.providerKey;
    downloadRequest.instructions.followRedirects = true;

    downloadRequest.instructions.verb = QString("GET");
    downloadRequest.instructions.Accept = QString("text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7");
    downloadRequest.instructions.ContentTypeHeader = QString("application/x-www-form-urlencoded; charset=UTF-8");
    downloadRequest.instructions.Origin = QString("https://www.legacy.com");
    downloadRequest.instructions.Sec_Fetch_Mode = QString("navigate");
    downloadRequest.instructions.Sec_Fetch_Site = QString("none");
    downloadRequest.instructions.Sec_Fetch_Dest = QString("document");
    //downloadRequest.instructions.Accept_Encoding = QString("gzip, deflate, br");
    //downloadRequest.instructions.Accept_Encoding = QString("br");
    downloadRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");

    downloadRequest.outputs.downloadFileName = QString("tempLegacyWebPage.htm");

    // Following structured only exist to avoid read errors on some unstructured content functions that refer to dataRecord
    dataRecord dr;
    globals->globalDr = &dr;
    dr.setGlobals(*globals);

    qSourceFile sourceFile;
    PQString outputFile;
    QString affiliate;


    // Setup valid Legacy newspapers to match against
    QList<QString> publicationNames;
    QList<long int> publicationCodes;
    QSqlQuery query;
    bool success;

    success = query.prepare("SELECT providerKey, fhURLparam1 FROM death_audits.funeralhomedata WHERE dts.providerID = 1 AND fhRunStatus = :fhRunStatus");
    query.bindValue(":fhRunStatus", QVariant(globals->runStatus));
    success = query.exec();

    if (success)
    {
        while(query.next())
        {
            publicationNames.append(query.value(0).toString());
            publicationCodes.append(query.value(1).toLongLong());
        }
    }

    long int firstID = 51648276;
    long int lastID  = firstID + 100;
    bool keepGoing = true;
    int numConsecutiveErrors = 0;
    long int i;
    int index;

    // Create first part of output file name
    outputFile << "UpdateLegacyDeceased - For IDs " << QString::number(firstID) << " to " << QString::number(lastID) << " - Run on ";
    outputFile += globals->today.toString("yyyyMMdd");
    outputFile += PQString(".LegacyObitURLlist");

    i = firstID;

    while (keepGoing && (i < lastID))
    {
        downloadRequest.instructions.url = baseURL + PQString(QString::number(i));
        downloadRequest.instructions.Referer = downloadRequest.instructions.url.getString();

        www->download(downloadRequest);
        while(www->processingDownload()){};
        if(www->lastDownloadSuccessful())
        {
            sourceFile.setSourceFile(downloadRequest.outputs.downloadFileName);
            sourceFile.setGlobalVars(*globals);
            sourceFile.beg();

            record.clear();
            pageVariables.reset();

            if (sourceFile.consecutiveMovesTo(40, "dataLayer.push", "Affiliate", ":"))
            {
                affiliate = sourceFile.readNextBetween(QUOTES).getString();
                index = publicationNames.indexOf(affiliate);
                if (index >= 0)
                {
                    pageVariables.providerKey = publicationCodes.at(index);
                    numConsecutiveErrors = 0;
                    // save file
                    // create obitList record if not already exist
                }
                else
                    numConsecutiveErrors++;
            }
        }

        if (numConsecutiveErrors >= 3)
            keepGoing = false;
        else
            i++;
    }

    if (globals->batchRunFlag)
    {
        QFile *file = new QFile(globals->batchRunFileName);
        QTextStream *outputStream = nullptr;

        if(file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            outputStream = new QTextStream(file);
            *outputStream << QString("UpdateLegacyDeceased completed running and exited normally at ") << QDateTime::currentDateTime().toString("h:mm") << Qt::endl;
            *outputStream << QString("Total downloads was: ") << globals->totalDownloads << Qt::endl;
        }

        delete outputStream;
        file->close();
    }

    qDebug() << "Total downloads was: " << globals->totalDownloads;
}

