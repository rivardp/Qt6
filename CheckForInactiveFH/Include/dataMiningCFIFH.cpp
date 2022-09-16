#include "Include/dataMiningCFIFH.h"

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

    // Testcase coding
    unsigned int testCaseFHid = 1008;
    unsigned int testCaseFHkey = 67;

    /********************************************************/
    /*     Create list of sites to visit, then process      */
    /********************************************************/

    QSqlQuery query;
    QSqlError error;

    unsigned int providerID, providerKey;
    unsigned int processCount = 0;
    QString fhURLforUpdate;
    PQString errorMessage, statusMsg;

    bool success;

    // Start with actual funeral homes
    //success = query.prepare("SELECT providerID, providerKey, fhURLforUpdate FROM funeralhomedata WHERE providerID = 1008 and providerKey = 67" );
    success = query.prepare("SELECT providerID, providerKey, fhURLforUpdate FROM funeralhomedata WHERE fhRunStatus = 1 or fhRunStatus = 100" );
    query.bindValue(":fhRunStatus", QVariant(argValue));
    success = query.exec();

    if (!success || (query.size() == 0))
    {
        error = query.lastError();

        errorMessage << QString("SQL problem retrieving list of funeral home websites to visit");
        globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }
    else
    {
        while (query.next())
        {
            processCount++;
            providerID = query.value(0).toUInt();
            providerKey = query.value(1).toUInt();
            fhURLforUpdate = query.value(2).toString();

            statusMsg.clear();
            statusMsg << "Started processing: " << providerID << " : " << providerKey;
            qDebug() << statusMsg.getString();
            globals->logMsg(AuditListing, statusMsg);

            flagNewQuestionable(providerID, providerKey);
            //flagQuestionable(providerID, providerKey, fhURLforUpdate);
            //flagQuestionable(testCaseFHid, testCaseFHkey, fhURLforUpdate);
        }
    }

    errorMessage.clear();
    errorMessage << QString("Total sites processed was: ") << processCount;
    qDebug() << errorMessage.getString();
    globals->logMsg(ActionRequired, errorMessage);
}

void MINER::flagQuestionable(const unsigned int &providerID, unsigned int &providerKey, QString &URL)
{

    QSqlQuery query;
    QSqlError error;

    QDate today(QDate::currentDate());
    QDate minDate, maxDate, cutOff;
    QString url;
    double count, period, avgGap, threshold;

    PQString errMsg, dateSQLmin, dateSQLmax;

    bool success;

    // First step is to determine the applicable averaging period (max 2 years)
    success = query.prepare("SELECT max(DOD), min(DOD) FROM deceased WHERE (providerID = :providerID AND providerKey = :providerKey) OR "
                                                                          "(altProviderID = :providerID AND altProviderKey = :providerKey) OR "
                                                                          "(altProviderID2 = :providerID AND altProviderKey2 = :providerKey)");
    query.bindValue(":providerID", QVariant(providerID));
    query.bindValue(":providerKey", QVariant(providerKey));
    success = query.exec();

    if (!success || (query.size() != 1))
    {
        error = query.lastError();
        errMsg << QString("SQL problem retrieving the last update date for: ") << providerID << " : " << providerKey << " :" << URL;
        globals->logMsg(ErrorSQL, errMsg, static_cast<int>(error.type()));
    }
    else
    {
        query.next();
        maxDate = query.value(0).toDate();
        minDate = query.value(1).toDate();
    }

    cutOff = QDate(today.year() - 2, today.month(), today.day());
    if (minDate < cutOff)
        minDate = cutOff;

    query.clear();

    if (minDate > maxDate)
    {
        errMsg << QString("Inactive FH should be closed for  :") << providerID << " : " << providerKey << " :" << URL;
        globals->logMsg(ActionRequired, errMsg);
        return;
    }

    // Second step is to count the number of deaths within the period
    dateSQLmin.clear();
    dateSQLmax.clear();
    dateSQLmin << minDate.toString("yyyy/MM/dd") << QString(" 0:0:0");
    dateSQLmax << maxDate.toString("yyyy/MM/dd") << QString(" 0:0:0");

    success = query.prepare("SELECT SUM(weight) FROM deceased WHERE ((providerID = :providerID AND providerKey = :providerKey) OR "
                                                                     "(altProviderID = :providerID AND altProviderKey = :providerKey) OR "
                                                                     "(altProviderID2 = :providerID AND altProviderKey2 = :providerKey)) AND "
                                                                     "DOD >= :dateSQLmin AND DOD <= :dateSQLmax");
    query.bindValue(":providerID", QVariant(providerID));
    query.bindValue(":providerKey", QVariant(providerKey));
    query.bindValue(":dateSQLmin", QVariant(dateSQLmin.getString()));
    query.bindValue(":dateSQLmax", QVariant(dateSQLmax.getString()));
    success = query.exec();

    if (success)
    {
        query.next();
        count = query.value(0).toDouble();
        if (count > 0)
        {
            avgGap = (maxDate.toJulianDay() - minDate.toJulianDay()) / count;
            if (avgGap < 1)
                avgGap = 1;
            period = (today.toJulianDay() - maxDate.toJulianDay());

            // Allow for a 3-day delay on average in best case
            if (period >= 3)
                period -= 3;
            else
                period = 0;

            threshold = 3 * avgGap;
            if (period > threshold)
            {
                errMsg << "Expected gap of " << "|" << QString::number(threshold) << "|";
                errMsg << "but actual gap of " << "|" << QString::number(period) << "|";
                errMsg << "for a ratio of: " << "|"  << QString::number(period / threshold) << "|";
                errMsg << "for " << "|" << providerID << "|" << providerKey << "|" << URL;
                globals->logMsg(ErrorRecord, errMsg);
            }
        }
        else
        {
            errMsg << "No deaths recorded at all for " << providerID << " : " << providerKey << " :" << URL;
            globals->logMsg(ErrorRecord, errMsg);
        }

    }

    query.clear();

}

void MINER::flagNewQuestionable(const unsigned int &providerID, unsigned int &providerKey)
{

    QSqlQuery query;
    QSqlError error;

    QDate today(QDate::currentDate());
    QDate DOD, cutOff, minDate, maxDate;
    QList<QDate> DODlist;
    double period, avgGap, threshold;

    PQString errMsg;

    bool success;
    cutOff = QDate(today.year() - 2, today.month(), today.day());

    success = query.prepare("SELECT d.DOD FROM death_audits.deceased d INNER JOIN "
                            "(SELECT dts.deceasedNumber FROM death_audits.deceasedidinfo dts WHERE dts.providerID = :providerID AND dts.providerKey = :providerKey) "
                            "AS ft USING(deceasedNumber)  WHERE d.lastName NOT LIKE \"BR_%\" GROUP BY d.DOD ORDER BY d.DOD");
    query.bindValue(":providerID", QVariant(providerID));
    query.bindValue(":providerKey", QVariant(providerKey));

    success = query.exec();

    if (!success)
    {
        error = query.lastError();
        errMsg << QString("SQL problem retrieving listing of historical DODs for: ") << providerID << " : " << providerKey;
        globals->logMsg(ErrorSQL, errMsg, static_cast<int>(error.type()));
    }
    else
    {
        while(query.next())
        {
            DOD = query.value(0).toDate();

            if (DOD.isValid() && (DOD > cutOff) && (DOD <= today))
                DODlist.append(DOD);
        }
    }

    if (DODlist.size() > 0)
    {
        minDate = DODlist.at(0);
        maxDate = DODlist.at(DODlist.size() - 1);

        period = (today.toJulianDay() - maxDate.toJulianDay());
        avgGap = (maxDate.toJulianDay() - minDate.toJulianDay()) / DODlist.size();

        // Allow for a 3-day delay on average in best case
        if (period >= 3)
            period -= 3;
        else
            period = 0;

        threshold = 3 * avgGap;
        if ((period > threshold) || (period > 21))
        {
            errMsg << "Expected gap of " << "|" << QString::number(threshold) << "|";
            errMsg << "but actual gap of " << "|" << QString::number(period) << "|";
            errMsg << "for a ratio of: " << "|"  << QString::number(period / threshold) << "|";
            errMsg << "for " << "|" << providerID << "|" << providerKey;
            globals->logMsg(ErrorRecord, errMsg);
        }
    }
    else
    {
        errMsg << "No deaths found for " << "|" << providerID << "|" << providerKey;
        globals->logMsg(ErrorRecord, errMsg);
    }
}


