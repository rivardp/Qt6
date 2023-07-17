#include "Include/dataMiningUD.h"

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

    /********************************************************/
    /*     Create list of sites to visit, then process      */
    /********************************************************/

    QSqlQuery query;
    QSqlError error;

    int numActiveSites, numProcessed;
    PQString errorMessage;
    UDrequestParam UDrequest;
    QList<UDrequestParam> UDrequestList;

    bool success;
    numProcessed = 0;


    success = query.prepare("SELECT providerID, providerKey, fhHTTP, fhWWW, fhURLforUpdate, fhURLparam1, fhURLparam2, fhURLid, fhURLidDivider, "
                            "       fhSpecialCode, fhFirstObit, fhSequentialID, fhAlphabeticalListing, fhFollowRedirects, fhLastRun "
                            "FROM death_audits.funeralhomedata WHERE fhRunStatus = :fhRunStatus AND providerID <> 1" );
    query.bindValue(":fhRunStatus", QVariant(argValue));
    success = query.exec();
    numActiveSites = query.size();

    if (!success || (numActiveSites == 0))
    {
        error = query.lastError();

        errorMessage << QString("SQL problem retrieving list of funeral home websites to visit");
        globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }
    else
    {
        while (query.next())
        {
            UDrequest.clear();
            numProcessed++;

            UDrequest.providerID = query.value(0).toUInt();
            UDrequest.providerKey = query.value(1).toUInt();
            UDrequest.fhHTTP = query.value(2).toString();
            UDrequest.fhWWW = query.value(3).toString();
            UDrequest.fhURLforUpdate = query.value(4).toString();
            UDrequest.fhParam1 = query.value(5).toString();
            UDrequest.fhParam2 = query.value(6).toString();
            UDrequest.fhURLid = query.value(7).toString();
            UDrequest.fhURLidDivider = query.value(8).toString();
            UDrequest.fhSpecialCode = query.value(9).toUInt();
            UDrequest.fhFirstObit = query.value(10).toDate();
            UDrequest.fhSequentialID = query.value(11).toString();
            UDrequest.fhAlphabeticalListing = query.value(12).toString();
            UDrequest.fhFollowRedirects = query.value(13).toString();
            UDrequest.fhLastRun = query.value(14).toDate();
            if (!UDrequest.fhFirstObit.isValid())
                UDrequest.fhFirstObit = globals->today;
            if (!UDrequest.fhLastRun.isValid())
                UDrequest.fhLastRun = UDrequest.fhFirstObit;

            UDrequest.originalOrder = numProcessed;

            UDrequestList.append(UDrequest);
        }
    }

    // Randomize order of downloads
    QRandomGenerator *generator = QRandomGenerator::global();
    QVector<quint32> vector;
    vector.resize(numProcessed);
    generator->fillRange(vector.data(), vector.size());
    for (int i = 0; i < numProcessed; i++){
        UDrequestList[i].randomOrder = vector.at(i);
    }
    vector.clear();
    std::sort(UDrequestList.begin(), UDrequestList.end(), [](const UDrequestParam &udr1, const UDrequestParam &udr2){return udr1.randomOrder < udr2.randomOrder;});

    for (int i = 0; i < numProcessed; i++)
    {
        errorMessage.clear();
        errorMessage << QString("Started processing: ") << UDrequestList[i].providerID << QString(" - ") << UDrequestList[i].providerKey;
        globals->logMsg(AuditListing, errorMessage);

        qDebug() << "Processing site " << i << " of " << numActiveSites << Qt::endl;

        updateDeceased(UDrequestList[i].providerID, UDrequestList[i].providerKey, UDrequestList[i].fhHTTP, UDrequestList[i].fhWWW, UDrequestList[i].fhURLforUpdate, UDrequestList[i].fhParam1, UDrequestList[i].fhParam2,
                       UDrequestList[i].fhURLid, UDrequestList[i].fhURLidDivider, UDrequestList[i].fhSpecialCode, UDrequestList[i].fhFirstObit, UDrequestList[i].fhSequentialID, UDrequestList[i].fhAlphabeticalListing,
                       UDrequestList[i].fhFollowRedirects, UDrequestList[i].fhLastRun);
    }

    if (globals->batchRunFlag)
    {
        QFile *file = new QFile(globals->batchRunFileName);
        QTextStream *outputStream = nullptr;

        if(file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            outputStream = new QTextStream(file);
            *outputStream << QString("UpdateDeceased completed running and exited normally at ") << QDateTime::currentDateTime().toString("h:mm") << Qt::endl;
            *outputStream << QString("Total downloads was: ") << globals->totalDownloads << Qt::endl;
        }

        delete outputStream;
        file->close();
    }

    qDebug() << "Total downloads was: " << globals->totalDownloads;
}

void MINER::updateDeceased(const unsigned int &providerID, const unsigned int &providerKey, const QString &HTTP, const QString &WWW, const QString &URL, QString &fhParam1, QString &fhParam2,
                           const QString &fhURLid, const QString &fhURLidDivider, const unsigned int &fhSpecialCode, QDate fhFirstObit, const QString &fhSequentialID,
                           const QString fhAlphabeticalListing, QString &fhFollowRedirects, QDate fhLastRun)
{
    Q_UNUSED(fhFollowRedirects);

    QSqlQuery query;
    QSqlError error;

    QDate lastRunDate(fhLastRun);
    QDate lastDODdate;
    QString lastID;

    PQString errorMessage, dateSQL1, dateSQL2;

    bool success;
    int numRecsFound;

    success = query.prepare("SELECT MAX(d.DOD), MAX(ft.ID) FROM "
                            "death_audits.deceased d INNER JOIN "
                            "(SELECT dts.deceasedNumber, dts.ID FROM death_audits.deceasedidinfo dts WHERE dts.providerID = :providerID AND dts.providerKey = :providerKey ORDER BY dts.deceasedNumber) "
                            "AS ft USING(deceasedNumber)");
    query.bindValue(":providerID", QVariant(providerID));
    query.bindValue(":providerKey", QVariant(providerKey));
    success = query.exec();

    if (!success || (query.size() != 1))
    {
        error = query.lastError();
        errorMessage << QString("SQL problem retrieving the last update date for :") << providerID << " - " << providerKey;
        globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }
    else
    {
        query.next();
        lastDODdate = query.value(0).toDate();
        lastID = query.value(1).toString();
    }
    if (!lastDODdate.isValid())
        lastDODdate.setDate(1900,1,1);
    query.clear();

    // Second step is to update the last run date and first obit date for the provider
    if (lastRunDate.isValid())
    {
        numRecsFound = 0;

        errorMessage.clear();
        errorMessage << QString("Mid-point processing: ") << providerID << QString(" - ") << providerKey;
        globals->logMsg(AuditListing, errorMessage);

        createUpdateObitURLlist(providerID, providerKey, HTTP, WWW, URL, lastDODdate, lastRunDate, lastID, fhParam1, fhParam2, fhURLid, fhURLidDivider, fhSpecialCode, fhFirstObit,
                                fhSequentialID, fhAlphabeticalListing, fhFollowRedirects, numRecsFound);

        if (numRecsFound > 0)
        {
            dateSQL1.clear();
            dateSQL2.clear();
            lastRunDate = QDate::currentDate();
            dateSQL1 << lastRunDate.toString("yyyy/MM/dd") << QString(" 0:0:0");
            dateSQL2 << lastDODdate.toString("yyyy/MM/dd") << QString(" 0:0:0");

            query.prepare("UPDATE death_audits.funeralhomedata SET fhLastRun = :lastUpdateDate, fhLastObit = :lastObitDate WHERE providerID = :providerID AND providerKey = :providerKey");
            query.bindValue(":providerID", QVariant(providerID));
            query.bindValue(":providerKey", QVariant(providerKey));
            query.bindValue(":lastUpdateDate", QVariant(dateSQL1.getString()));
            query.bindValue(":lastObitDate", QVariant(dateSQL2.getString()));
            success = query.exec();
            query.clear();
        }

        errorMessage.clear();
        if (numRecsFound > 0)
            errorMessage << QString("Report the processing of ") << numRecsFound << QString(" records for: ") << providerID << QString(" - ") << providerKey;
        else
            errorMessage << QString("NOTHING FOUND for: ") << providerID << QString(" - ") << providerKey;
        globals->logMsg(AuditListing, errorMessage);
    }
}

void MINER::createUpdateObitURLlist(const unsigned int &providerID, const unsigned int &providerKey, const QString &http, const QString &WWW, const QString &URL,
                                    const QDate &lastDODdate, const QDate &lastRunDate, const QString &lastID, QString &fhParam1, QString &fhParam2,
                                    const QString &fhURLid, const QString &fhURLidDivider, const unsigned int &fhSpecialCode, QDate fhFirstObit,
                                    const QString &fhSequentialId, const QString &fhAlphabeticalListing, QString &fhFollowRedirects, int &numRecsFound)
{
    // Some funeral homes can be processed with a single query, others may require two or more

    Q_UNUSED(lastID)   // For possible later usage
    Q_UNUSED(fhParam2) // For possible later usage
    Q_UNUSED(fhSpecialCode) // For possible use if different approaches required for same providerID
    Q_UNUSED(fhFollowRedirects)

    PQString paramPlaceholder("%p%");
    PQString HTTP(http);
    PQString hostname(WWW);
    PQString URLaddressTemplate, URLaddress, URLbase, URLext, URLtemp, baseURL;
    PQString URLparam1, URLparam2, URLparam3, URLparam4, URLparam5;
    PQString yyyymm;
    QString lastDate, string, tempID, startDate, endDate;
    PQString outputFile, errorMessage;
    OQString maidenNames, word, singleChar;
    QString dateFormat("yyyyMMdd");
    DOWNLOADREQUEST downloadRequest, preflightRequest, followUpRequest, initialRequest;

    RECORD record;
    QList<RECORD> records;
    URLPARAMS URLparams;
    PAGEVARIABLES pageVariables;
    FLOWPARAMETERS flowParameters;
    QHttpMultiPart *multiPart = nullptr;

    // Following structured only exist to avoid read errors on some unstructured content functions that refer to dataRecord
    dataRecord dr;
    globals->globalDr = &dr;
    dr.setGlobals(*globals);

    qSourceFile sourceFile;
    unstructuredContent temp;

    LANGUAGE lang = english;
    unsigned int nonConstantProviderKey = providerKey;

    unsigned int numTemp;
    int index, daysOfOverlap;

    daysOfOverlap = 61;

    flowParameters.initialSetup = false;
    flowParameters.numBlankDownloads = 0;
    flowParameters.numBlankDownloadLimit = 3;
    flowParameters.numRecordsRead = 0;
    flowParameters.indexOffset = 0;
    flowParameters.flowIncrement = 1;

    pageVariables.providerID = providerID;
    pageVariables.providerKey = providerKey;
    pageVariables.latestDODdate = lastDODdate;
    pageVariables.lastRunDate = lastRunDate;
    pageVariables.priorDODdate = lastDODdate;
    pageVariables.firstAvailableDate = fhFirstObit;
    pageVariables.queryTargetDate = globals->today;
    pageVariables.lastID = lastID;
    pageVariables.sequentialID = (fhSequentialId == QString("Y"));
    pageVariables.fhURLid = fhURLid;
    pageVariables.fhURLidDivider = fhURLidDivider;
    pageVariables.alphabeticalListing = (fhAlphabeticalListing == QString("Y"));
    pageVariables.paginatedResult = false;
    pageVariables.usePubDateForCutOff = false;

    downloadRequest.instructions.providerID = providerID;
    downloadRequest.instructions.providerKey = providerKey;
    /*if (fhFollowRedirects == QString("Y"))
        downloadRequest.instructions.followRedirects = true;
    else
        downloadRequest.instructions.followRedirects = false;*/
    downloadRequest.instructions.followRedirects = true;
    downloadRequest.outputs.downloadFileName = QString("tempWebPage.htm");

    preflightRequest.instructions.providerID = providerID;
    preflightRequest.instructions.providerKey = providerKey;
    followUpRequest.instructions.providerID = providerID;
    followUpRequest.instructions.providerKey = providerKey;
    initialRequest.instructions.providerID = providerID;
    initialRequest.instructions.providerKey = providerKey;

    errorMessage.clear();
    errorMessage << QString("Output file created for: ") << providerID << QString(" - ") << providerKey;
    globals->logMsg(AuditListing, errorMessage);

    // Create first part of output file name
    outputFile << "UpdateDeceased " << providerID << " " << providerKey;
    outputFile += PQString(" History From ");
    outputFile += pageVariables.queryTargetDate.toString("yyyyMMdd");

    // Common base settings
    baseURL = URL;
    validateURL(HTTP, hostname, baseURL);

    // Create the initial URL to start the data mining process for the target month
    // This initial query, where possible, sets up and executes a single query to extract the correct range of obit names.
    // Otherwise, just some basic setup occurs here

    switch(providerID)
    {
    case 1:     // Legacy
    {
        // No history will ever be downloaded - Only look at most recent obits
        //https://www.legacy.com/ca/obituaries/nsnews/browse

        daysOfOverlap = 0;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate = URLbase + PQString("/ca/obituaries/") + fhParam1 + PQString("/browse");
        URLparams.numParams = 0;

        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 2:  // Winnipeg Free Press - Passages
    {
        daysOfOverlap = 61;
        pageVariables.firstAvailableDate.setDate(fhFirstObit.year(), fhFirstObit.month(), 1);
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        if (flowParameters.initialSetup)
            URLaddressTemplate = URLbase + PQString("/passages-search/date-range-all/publication-id-") + paramPlaceholder;
        else
            URLaddressTemplate = URLbase + PQString("/passages-search/date-range-month/publication-id-") + paramPlaceholder;
        URLaddressTemplate += PQString("/classification-id-1/order-run_date/dir-desc?page=") + paramPlaceholder;

        URLparams.numParams = 2;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &nonConstantProviderKey;
        URLparams.param2Type = ptUint;
        URLparams.UIparam2 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = fhParam2.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 5:  // Brandon Sun
    {
        daysOfOverlap = 61;
        pageVariables.firstAvailableDate.setDate(fhFirstObit.year(), fhFirstObit.month(), 1);
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 15;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/passages-search/date-range-month/classification-id-1/order-run_date/dir-desc?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        flowParameters.endingPosition = fhParam2.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 6:  // Cooperative Funeraire
    {
        daysOfOverlap = 91;
        pageVariables.firstAvailableDate.setDate(fhFirstObit.year(), fhFirstObit.month(), 1);
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        switch (providerKey)
        {
        case 90:
            URLaddressTemplate  = URLbase + fhParam1 + PQString("/avis-de-deces/?page=") + paramPlaceholder;
            break;

        case 111:
            URLaddressTemplate  = URLbase + fhParam1 + PQString("/death-notices/?page=") + paramPlaceholder;
            break;

        default:
            URLaddressTemplate  = URLbase + fhParam1 + PQString("/avis-de-deces/?page=") + paramPlaceholder;
            break;
        }

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 91;
        else
            flowParameters.endingPosition = 10;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 8:  // Domaine Funeraire
    {
        daysOfOverlap = 61;
        pageVariables.firstAvailableDate.setDate(fhFirstObit.year(), fhFirstObit.month(), 1);
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        if (fhSpecialCode == 1)
        {
            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/recherche/avis-de-deces/page-") + paramPlaceholder  + PQString("?companySlug=") + paramPlaceholder;

            URLparams.numParams = 2;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;
            URLparams.param2Type = ptQS;
            URLparams.QSparam2 = &fhParam1;
        }
        else
        {
            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/recherche/avis-de-deces/") + paramPlaceholder + PQString("/page-") + paramPlaceholder;

            URLparams.numParams = 2;
            URLparams.param1Type = ptQS;
            URLparams.QSparam1 = &fhParam1;
            URLparams.param2Type = ptUint;
            URLparams.UIparam2 = &flowParameters.currentPosition;
        }

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 100;
        else
            flowParameters.endingPosition = 10;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        //downloadRequest.instructions.Accept_Encoding = QString("br");
    }
        break;

    case 11:  // Great West
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        if (providerKey == 18)
            pageVariables.usePubDateForCutOff = true;

        // https://kitchener.citynews.ca/obituaries?page=2
        URLbase = baseURL;
        URLaddressTemplate = URLbase + PQString("/obituaries?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 15;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 12:  // ObitTree
    {
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/search?page=") + paramPlaceholder;
        URLaddressTemplate += PQString("&state=") + paramPlaceholder;
        URLaddressTemplate += PQString("&dateStart=") + paramPlaceholder + PQString("&dateEnd=") + paramPlaceholder;
        URLaddressTemplate += PQString("&datetype=Date%20range");

        URLparams.numParams = 4;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;
        URLparams.param2Type = ptQS;
        URLparams.QSparam2 = &fhParam1;
        URLparams.param3Type = ptQTDmask;
        if (flowParameters.initialSetup)
            URLparams.QTDdate3 = &fhFirstObit;
        else
            URLparams.QTDdate3 = &pageVariables.cutoffDate;
        URLparams.maskType3 = mtYYYYhMMhDD;
        URLparams.param4Type = ptQTDmask;
        URLparams.QTDdate4 = &globals->today;
        URLparams.maskType4 = mtYYYYhMMhDD;

        if (flowParameters.initialSetup)
            daysOfOverlap = 999;
        else
            daysOfOverlap = 61;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 10000;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // ObitTree

    case 13:  // Echovita
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate = URLbase + PQString("/ca/obituaries") + fhParam1 + PQString("?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = fhParam2.toUInt();
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 14:  // Saltwire
    {
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = -1;
        pageVariables.usePubDateForCutOff = true;

        if (flowParameters.initialSetup)
            pageVariables.cutoffDate = pageVariables.firstAvailableDate;
        else
            pageVariables.cutoffDate = pageVariables.latestDODdate.addDays(-daysOfOverlap);

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/") + fhParam1 + PQString("/obituaries/") + paramPlaceholder;
        URLaddressTemplate += PQString("/") + paramPlaceholder;
        URLaddressTemplate += PQString("/") + paramPlaceholder + PQString("/");

        flowParameters.currentPosition = globals->today.toJulianDay();
        flowParameters.endingPosition = pageVariables.cutoffDate.toJulianDay();

        URLparams.numParams = 3;
        URLparams.param1Type = ptJulian;
        URLparams.Julian1 = &flowParameters.currentPosition;
        URLparams.maskType1 = mtYYYY;
        URLparams.param2Type = ptJulian;
        URLparams.Julian2 = &flowParameters.currentPosition;
        URLparams.maskType2 = mtMMMM;
        URLparams.param3Type = ptJulian;
        URLparams.Julian3 = &flowParameters.currentPosition;
        URLparams.maskType3 = mtD;

        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // Saltwire

    case 15:  // Black Media
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        if (fhSpecialCode == 1)
        {
            URLbase = baseURL;
            URLaddressTemplate = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup)
                flowParameters.endingPosition = fhParam2.toUInt();
            else
                flowParameters.endingPosition = 1;
        }
        else
        {
            URLbase = baseURL;
            //https://obits_feed.blackpress.ca/index.full.php?pub=PNR
            URLaddressTemplate = URLbase + PQString("/index.full.php?pub=") + fhParam1;

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 16:  // Aberdeen
    {
        daysOfOverlap = 91;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        downloadRequest.instructions.verb = QString("POST");

        //

        URLbase = baseURL;
        URLaddressTemplate = URLbase + PQString("/wp-admin/admin-ajax.php?");
        URLaddressTemplate += PQString("action=dpdfg_get_posts_data_action&page=") + paramPlaceholder;
        URLaddressTemplate += PQString("&module_data%5Bcustom_query%5D=advanced&module_data%5Bsupport_for%5D=sfp&module_data%5Bsfp_id%5D=&module_data%5Binclude_categories%5D=&module_data%5Bcurrent_post_type%5D=on");
        URLaddressTemplate += PQString("&module_data%5Bmultiple_cpt%5D=obituary&module_data%5Buse_taxonomy_terms%5D=off&module_data%5Bmultiple_taxonomies%5D=category&module_data%5Btaxonomies_relation%5D=OR");
        URLaddressTemplate += PQString("&module_data%5Binclude_terms%5D=&module_data%5Bterms_relation%5D=IN&module_data%5Binclude_children_terms%5D=on&module_data%5Bexclude_taxonomies%5D=category");
        URLaddressTemplate += PQString("&module_data%5Bexclude_taxonomies_relation%5D=OR&module_data%5Bexclude_terms%5D=&module_data%5Brelated_taxonomies%5D=&module_data%5Brelated_criteria%5D=one_in_one");
        URLaddressTemplate += PQString("&module_data%5Bposts_ids%5D=&module_data%5Bpost_number%5D=10&module_data%5Boffset_number%5D=0&module_data%5Border%5D=DESC&module_data%5Borderby%5D=date&module_data%5Bmeta_key%5D=");
        URLaddressTemplate += PQString("&module_data%5Bmeta_type%5D=CHAR&module_data%5Bshow_private%5D=off&module_data%5Bcurrent_author%5D=off&module_data%5Bsticky_posts%5D=off&module_data%5Bremove_current_post%5D=off");
        URLaddressTemplate += PQString("&module_data%5Bno_results%5D=No+results+found.&module_data%5Bthumbnail_action%5D=link&module_data%5Bgallery_cf_name%5D=&module_data%5Bshow_thumbnail%5D=on&module_data%5Bthumbnail_size%5D=400x284");
        URLaddressTemplate += PQString("&module_data%5Buse_overlay%5D=on&module_data%5Bshow_title%5D=on&module_data%5Btitle_link%5D=on&module_data%5Bshow_post_meta%5D=on&module_data%5Bmeta_separator%5D=+%7C+");
        URLaddressTemplate += PQString("&module_data%5Bshow_author%5D=off&module_data%5Bauthor_prefix_text%5D=By+&module_data%5Bshow_date%5D=on&module_data%5Bdate_format%5D=F+j%2C+Y&module_data%5Bshow_terms%5D=off");
        URLaddressTemplate += PQString("&module_data%5Bshow_terms_taxonomy%5D=category&module_data%5Bterms_separator%5D=%2C&module_data%5Bterms_links%5D=on&module_data%5Bshow_comments%5D=off&module_data%5Bshow_content%5D=on");
        URLaddressTemplate += PQString("&module_data%5Bcontent_length%5D=excerpt&module_data%5Btruncate_content%5D=120&module_data%5Btruncate_excerpt%5D=off&module_data%5Bstrip_html%5D=on&module_data%5Baction_button%5D=off");
        URLaddressTemplate += PQString("&module_data%5Baction_button_text%5D=Click+Action+Button&module_data%5Bread_more%5D=off&module_data%5Bread_more_text%5D=Read+More&module_data%5Bread_more_window%5D=off");
        URLaddressTemplate += PQString("&module_data%5Bshow_custom_fields%5D=on&module_data%5Bcustom_fields%5D=%26%2391%3B%7B%22name%22%3A%22died%22%2C%22label%22%3A%22%22%7D%26%2393%3B&module_data%5Bshow_custom_content%5D=on");
        URLaddressTemplate += PQString("&module_data%5Bcustom_content_container%5D=on&module_data%5Bcustom_url%5D=off&module_data%5Bcustom_url_field_name%5D=&module_data%5Bcustom_url_target%5D=same");
        URLaddressTemplate += PQString("&module_data%5Bshow_video_preview%5D=off&module_data%5Bvideo_module%5D=off&module_data%5Bvideo_action%5D=play&module_data%5Bvideo_action_priority%5D=on&module_data%5Bvideo_overlay%5D=on");
        URLaddressTemplate += PQString("&module_data%5Bvideo_overlay_icon%5D=on&module_data%5Bvideo_icon%5D=%25%2540%25%25&module_data%5Bvideo_icon_color%5D=%23ffffff&module_data%5Bvideo_overlay_color%5D=rgba(0%2C0%2C0%2C0.6)");
        URLaddressTemplate += PQString("&module_data%5Bshow_filters%5D=off&module_data%5Bajax_filters%5D=off&module_data%5Bfilter_layout%5D=button&module_data%5Bmultifilter%5D=off&module_data%5Bmultifilter_relation%5D=OR");
        URLaddressTemplate += PQString("&module_data%5Bmultilevel%5D=off&module_data%5Bmultilevel_hierarchy%5D=off&module_data%5Bmultilevel_hierarchy_tax%5D=%5B%7B%22name%22%3A%22category%22%2C%22label%22%3A%22%22%2C%22all%22%3A%22%22%7D%5D");
        URLaddressTemplate += PQString("&module_data%5Bmultilevel_relation%5D=AND&module_data%5Bmultilevel_tax_data%5D=%5B%7B%22name%22%3A%22category%22%2C%22label%22%3A%22%22%2C%22all%22%3A%22%22%7D%5D");
        URLaddressTemplate += PQString("&module_data%5Buse_custom_terms_filters%5D=off&module_data%5Bfilter_taxonomies%5D=category&module_data%5Bfilter_terms%5D=&module_data%5Bdefault_filter%5D=All&module_data%5Bhide_all%5D=off");
        URLaddressTemplate += PQString("&module_data%5Ball_text%5D=All&module_data%5Bfilters_order%5D=ASC&module_data%5Bfilters_sort%5D=id&module_data%5Bfilters_custom%5D=&module_data%5Bshow_pagination%5D=on");
        URLaddressTemplate += PQString("&module_data%5Bpagination_type%5D=paged&module_data%5Bajax_load_more_text%5D=Load+More&module_data%5Bprevious_icon%5D=%25%2519%25%25&module_data%5Bnext_icon%5D=%25%2520%25%25");
        URLaddressTemplate += PQString("&module_data%5Bprevious_text%5D=&module_data%5Bnext_text%5D=&module_data%5Bpagination_pages%5D=2&module_data%5Bitems_layout%5D=dp-dfg-layout-list&module_data%5Bitems_skin%5D=dp-dfg-skin-default");
        URLaddressTemplate += PQString("&module_data%5Bthumb_width%5D=33%25&module_data%5Bitems_width%5D=20%25&module_data%5Bpopup_template%5D=default&module_data%5Bpopup_code%5D=&module_data%5Buse_overlay_icon%5D=on");
        URLaddressTemplate += PQString("&module_data%5Bhover_icon%5D=%26%23xe101%3B%7C%7Cdivi%7C%7C400&module_data%5Boverlay_icon_color%5D=%23FFFFFF&module_data%5Bhover_overlay_color%5D=rgba(0%2C0%2C0%2C0.64)");
        URLaddressTemplate += PQString("&module_data%5Bshow_search%5D=off&module_data%5Bsearch_position%5D=above&module_data%5Borderby_search%5D=on&module_data%5Brelevanssi%5D=off&module_data%5Bcache_on_page%5D=off");
        URLaddressTemplate += PQString("&module_data%5Bbg_items%5D=&module_data%5Badmin_label%5D=&module_data%5Bmodule_id%5D=&module_data%5Bmodule_class%5D=&module_data%5Bdpdfg_entry_title_level%5D=h2");
        URLaddressTemplate += PQString("&module_data%5Bread_more_button_icon%5D=&module_data%5Baction_button_icon%5D=&module_data%5Bthe_ID%5D=48162&module_data%5Bthe_author%5D=7&module_data%5Bseed%5D=7251");
        URLaddressTemplate += PQString("&module_data%5Bquery_context%5D=ajax&module_data%5Bconditional_tags%5D%5Bis_user_logged_in%5D=off&module_data%5Bconditional_tags%5D%5Bis_front_page%5D=off");
        URLaddressTemplate += PQString("&module_data%5Bconditional_tags%5D%5Bis_singular%5D=on&module_data%5Bconditional_tags%5D%5Bis_archive%5D=off&module_data%5Bconditional_tags%5D%5Bis_search%5D=off");
        URLaddressTemplate += PQString("&module_data%5Bconditional_tags%5D%5Bis_tax%5D=off&module_data%5Bconditional_tags%5D%5Bis_author%5D=off&module_data%5Bconditional_tags%5D%5Bis_date%5D=off");
        URLaddressTemplate += PQString("&module_data%5Bconditional_tags%5D%5Bis_post_type%5D=off&module_data%5Bquery_var%5D%5Bs%5D=&module_data%5Bquery_var%5D%5Byear%5D=0&module_data%5Bquery_var%5D%5Bmonthnum%5D=0");
        URLaddressTemplate += PQString("&module_data%5Bquery_var%5D%5Bday%5D=0&module_data%5Bquery_var%5D%5Bpost_type%5D=&module_data%5Bactive_filter%5D=&vb=off");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = fhParam2.toUInt();
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 17:  // Village


    case 1000:  // Batesville
    {
        switch(providerKey)
        {
        // One website is different from all the others
        // Now more current and uses JSON
        case 15089:
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            URLbase = URL;
            validateURL(HTTP, hostname, URLbase);

            //https://parkplacefuneralhome-15089.meaningfulfunerals.net/obituaries/obit_json?fh_id=15089&page_count=20&page_number=3&search_field=&sort_by=deathDate&sort_direction=desc&_=1586796739012
            //https://parkplacefuneralhome-15089.meaningfulfunerals.net/obituaries/obit_json?fh_id=15089&page_count=20&page_number=1&search_field=&sort_by=deathDate&sort_direction=desc&_=1607740187235

            URLaddressTemplate = URLbase + PQString("/obituaries/obit_json?fh_id=") + PQString(providerKey);
            URLaddressTemplate += PQString("&page_count=20&page_number=") + paramPlaceholder + PQString("&search_field=&sort_by=deathDate&sort_direction=desc");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;

            determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
            createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

            break;

        default:
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            URLbase = URL;
            validateURL(HTTP, hostname, URLbase);

            URLext = PQString("/obituaries/paging?fh_id=") + PQString(providerKey);
            URLext += PQString("&sort_by=all&page_number=") + paramPlaceholder;
            URLaddressTemplate = URLbase + URLext;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;

            determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);

            // Initial download just to determine total number of recs and pages
            createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

            if (flowParameters.initialSetup)
            {
                www->download(downloadRequest);
                while(www->processingDownload()){};
                if(www->lastDownloadSuccessful())
                {
                    sourceFile.setSourceFile(downloadRequest.outputs.downloadFileName);
                    if (sourceFile.moveTo("max-page"))
                        flowParameters.endingPosition = static_cast<unsigned int>(sourceFile.readNextBetween(BRACKETS).asNumber());
                }
            }
            else
                flowParameters.endingPosition = 4;
        }
    }
        break;

    case 1002:  // BrowseAll
    {
        // Single download pulls everything
        daysOfOverlap = 90;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = URL;
        validateURL(HTTP, hostname, URLbase);

        baseURL = URLbase;
        URLaddressTemplate = URLbase + PQString("/fh/home/home.cfm?&fh_id=") + PQString(providerKey);

        URLparams.numParams = 0;
        flowParameters.currentPosition = 0;
        flowParameters.endingPosition = 0;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 1002

    case 1003:  // DignityMemorial  - Extra query required
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        // Something is amiss with "start" and "size" parameters (problem is with provider), but coded to work
        // i.e., with start = 320 and size = 10, output is obits 0 through 330

        //PQString startDate("1900-01-01");
        PQString startDate("2020-01-01");
        // Refine query to pull only as many summaries as necessary
        if (pageVariables.latestDODdate > QDate(1900,1,1))
        {
            startDate.clear();
            pageVariables.cutoffDate = pageVariables.latestDODdate.addDays(-daysOfOverlap);
            //pageVariables.cutoffDate = QDate(2020,1,1);
            startDate << pageVariables.cutoffDate.year() << "-";
            startDate << QString("%1").arg(pageVariables.cutoffDate.month(), 2, 10, QChar('0')) << "-";
            startDate << QString("%1").arg(pageVariables.cutoffDate.day(), 2, 10, QChar('0'));
        }

        URLext  = PQString("/obituaries?q=(and%20(and%20locationid:'") + PQString(providerKey);
        URLext += PQString("')%20%20%20(or%20cmicreationdate:['") + startDate;
        URLext += PQString("t00:00:00z',}))&start=") + paramPlaceholder;
        URLext += PQString("&size=10&filtergroup=cmicreationdate&filtervalue=['") + startDate;
        URLext += PQString("t00:00:00z',}&filterchecked=true");
        //URLext += PQString("t00:00:00z',}&filterchecked=true&fh='") + PQString(fhParam1);
        URLaddressTemplate = baseURL + URLext;

        int tempInt = 0;
        // This one parameter will cause all desired obits to be downloaded in a single shot
        URLparams.numParams = 1;
        URLparams.param1Type = ptInt;
        URLparams.IntParam1 = &tempInt;

        flowParameters.currentPosition = 0;
        flowParameters.endingPosition = 0;

        //determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        www->download(downloadRequest);
        while(www->processingDownload()){};
        if(www->lastDownloadSuccessful())
        {
            sourceFile.setSourceFile(downloadRequest.outputs.downloadFileName);
            if (sourceFile.consecutiveMovesTo(50, "countResults", "value="))
            {
                word = sourceFile.readNextBetween(QUOTES);
                tempInt = static_cast<int>(word.asNumber());
                if (tempInt > 10)
                    tempInt -= 10;
                else
                    tempInt = 0;

                /*// New coding March 2021

                //https://www.dignitymemorial.com/obituaries?q=(and%20(and%20locationid:'3600')%20%20%20(or%20cmicreationdate:['1900-01-01t00:00:00z',}))&start=0&size=10&filtergroup=cmicreationdate&filtervalue=['1900-01-01t00:00:00z',}&filterchecked=true&fh='foster's%20garden%20chapel%20funeral%20home
                //https://www.dignitymemorial.com/obituaries?q=(and%20(and%20locationid:'3152')%20%20%20(or%20cmicreationdate:['2000-01-01t00:00:00z',}))&start=12381&size=10&filtergroup=cmicreationdate&filtervalue=['2000-01-01t00:00:00z',}&filterchecked=true
                //https://www.dignitymemorial.com/obituaries?q=(and%20(and%20locationid:%273152%27))&start=0&filtergroup=null&filtervalue=null&filterchecked=false&fh=%27Kane-Jerrett%20Funeral%20Homes%27
                //https://www.dignitymemorial.com/en//obituaries/ObituariesSearch/More?varQuery=q%3D(and+(and+locationid%3A%273152%27)+(or+cmicreationdate%3A%5B%272020-01-01T00%3A00%3A00Z%27%2C%7D)+++)%26start%3D0%26size%3D10%26filtergroup%3Dcmicreationdate%26filtervalue%3D%5B%272020-01-01t00%3A00%3A00z%27%2C%7D%26filterchecked%3Dtrue%26grave%3Dfalse&grave=false

                URLext  = PQString("/en//obituaries/ObituariesSearch/More?varQuery=");
                URLext += PQString("q=(and+(and+locationid:'") + PQString(providerKey) + PQString("')+");
                URLext += PQString("(or+cmicreationdate:['") + startDate + PQString("T00:00:00Z',})+++)");
                URLext += PQString("&start=") + paramPlaceholder + PQString("&size=10");
                URLext += PQString("&filtergroup=cmicreationdate&filtervalue=['") + startDate + PQString("t00:00:00z',}&filterchecked=true&grave=false");
                URLaddressTemplate = baseURL + URLext;*/

                createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
            }
        }
    }
        break; // 1003

    case 1004:  // Foster
    {
        // Single download pulls everything
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = URL;
        URLbase += PQString("/");
        validateURL(HTTP, hostname, URLbase);

        baseURL = URLbase;
        URLaddressTemplate = URLbase + QString("obituary_view_all.php");

        URLparams.numParams = 0;
        flowParameters.currentPosition = 0;
        flowParameters.endingPosition = 0;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 1004

    case 1005:  // CFS
    {
        daysOfOverlap = 61;
        pageVariables.firstAvailableDate.setDate(fhFirstObit.year(), fhFirstObit.month(), 1);
        flowParameters.initialSetup = false;
        flowParameters.flowType = sequential;
        flowParameters.flowIncrement = -1;
        flowParameters.initialPosition = 0;

        if (flowParameters.initialSetup)
            pageVariables.cutoffDate = pageVariables.firstAvailableDate;
        else
            pageVariables.cutoffDate = pageVariables.latestDODdate.addDays(-daysOfOverlap);

        baseURL = URL;
        validateURL(HTTP, hostname, baseURL);

        // There are two very different approaches coded, old and new
        if ((providerKey > 90000) || (providerKey == 824) || (providerKey == 6192) || (providerKey == 7207))
        {
            // Old sites - no pagination
            switch (providerKey)
            {
            case 6192:
                URLbase = baseURL;

                URLaddressTemplate  = baseURL + QString("/pax/hzobitset?stidx=") + paramPlaceholder;
                URLaddressTemplate += QString("&num=6&sids=7600&lcity=0&sets=0&smn=0&ps=6&sm=0&udn=1&udob=0");

                flowParameters.flowType = startToEnd;
                flowParameters.currentPosition = 0;
                if (flowParameters.initialSetup)
                    flowParameters.endingPosition = 72;
                else
                    flowParameters.endingPosition = 6;
                flowParameters.flowIncrement = 6;

                URLparams.numParams = 1;
                URLparams.param1Type = ptUint;
                URLparams.UIparam1 = &flowParameters.currentPosition;
                break;

            default:
                URLbase = baseURL + QString("/obituaries/allobituaries.php");

                URLaddressTemplate  = URLbase;
                URLaddressTemplate += PQString("?sm=") + paramPlaceholder;
                URLaddressTemplate += PQString("&sy=") + paramPlaceholder;

                URLparams.numParams = 2;
                URLparams.param1Type = ptQTDmonth;
                URLparams.QTDdate1 = &pageVariables.queryTargetDate;
                URLparams.param2Type = ptQTDyear;
                URLparams.QTDdate2 = &pageVariables.queryTargetDate;
                break;
            }

            determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
            createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
        }
        else
        {
            // New sites - can have pagination for any given month
            URLbase = baseURL + QString("/pax/obitsrch");

            switch (fhSpecialCode)
            {
            case 0:
                URLaddressTemplate  = URLbase;
                URLaddressTemplate += PQString("?numlistings=25");
                URLaddressTemplate += PQString("&paginate=1");
                URLaddressTemplate += PQString("&pg=") + paramPlaceholder;
                URLaddressTemplate += PQString("&sids=") + PQString(providerKey);
                URLaddressTemplate += PQString("&tgt=obitlist");
                URLaddressTemplate += PQString("&typ=1");
                URLaddressTemplate += PQString("&ym=") + paramPlaceholder;

                URLparams.numParams = 2;
                URLparams.param1Type = ptUint;
                URLparams.UIparam1 = &flowParameters.currentPosition;
                URLparams.param2Type = ptQTDmask;
                URLparams.QTDdate2 = &pageVariables.queryTargetDate;
                URLparams.maskType2 = mtYYYYMM;
                break;

            case 1:
                // https://www.hannahfuneralhome.com/pax/obitsrch?ym=202105&txtsrch=0&term=&sids=11506&paginate=1&numlistings=10&showmiddlename=0&candlemode=0&typ=1&listcity=0&udn=1&alpha=0
                // https://www.hannahfuneralhome.com/pax/obitsrch?ym=202105&txtsrch=0&term=&sids=11519&paginate=1&numlistings=10&pg=0&showmiddlename=0&candlemode=0&typ=1&listcity=0&udn=1&alpha=0
                //                                                ym=202103&txtsrch=0&term=&sids=11506&paginate=1&numlistings=10&showmiddlename=0&candlemode=0&typ=1&listcity=0&udn=1&alpha=0

                URLaddressTemplate  = URLbase;
                URLaddressTemplate += PQString("?ym=") + paramPlaceholder;
                URLaddressTemplate += PQString("&txtsrch=0&term=");
                if (providerKey == 11172)
                    URLaddressTemplate += PQString("&sids=") + PQString("11172-243,11172-244,11172-245");
                else
                    URLaddressTemplate += PQString("&sids=") + PQString(providerKey);
                URLaddressTemplate += PQString("&paginate=1");
                URLaddressTemplate += PQString("&numlistings=10");
                URLaddressTemplate += PQString("&pg=") + paramPlaceholder;
                URLaddressTemplate += PQString("&showmiddlename=0&candlemode=0&typ=1&listcity=0&udn=1&alpha=0");

                URLparams.numParams = 1;
                URLparams.param1Type = ptQTDmask;
                URLparams.QTDdate1 = &pageVariables.queryTargetDate;
                URLparams.maskType1 = mtYYYYMM;
                URLparams.param2Type = ptUint;
                URLparams.UIparam2 = &flowParameters.currentPosition;
                break;

            case 2: //pg=2&term=&paginate=1&ym=0&showmiddlename=0&listcity=0&tgt=obitlist&numlistings=10&sids=2744&typ=1&txtsrch=0

                URLaddressTemplate  = URLbase;
                URLaddressTemplate += PQString("?pg=") + paramPlaceholder;
                URLaddressTemplate += PQString("&term=&paginate=1&ym=0&showmiddlename=0&listcity=0&tgt=obitlist&numlistings=10&sids=") + PQString(providerKey);
                URLaddressTemplate += PQString("&typ=1&txtsrch=0");

                URLparams.numParams = 1;
                URLparams.param1Type = ptUint;
                URLparams.UIparam1 = &flowParameters.currentPosition;
                break;
            }

            // First download is current month
            flowParameters.currentPosition = flowParameters.initialPosition;
            flowParameters.endingPosition = flowParameters.initialPosition;
            pageVariables.paginatedResult = false;

            determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
            createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

            downloadRequest.instructions.verb = QString("POST");
        }
    }
        break;

    case 1006:  // Frazer
    {
        daysOfOverlap = 61;
        pageVariables.firstAvailableDate.setDate(fhFirstObit.year(), fhFirstObit.month(), 1);
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        if (providerKey == 15)
        {
            URLbase = baseURL + QString("/obituaries/obituary-listings");
            URLaddressTemplate  = URLbase;
            URLaddressTemplate += PQString("?page=") + paramPlaceholder;
        }
        else
        {
            URLbase = baseURL + QString("/obituaries/obituary-listings");
            URLaddressTemplate  = URLbase;
            URLaddressTemplate += PQString("?CurrentPage=") + paramPlaceholder;
            URLaddressTemplate += PQString("&PageSize=10");
        }

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = fhParam2.toUInt();
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1007:  // Alternatives
    {
        // Historical - Loop through pages
        // Active - Last 90 days is always pulled in a single page
        daysOfOverlap = 90;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        if (flowParameters.initialSetup)
        {
            URLbase = baseURL + QString("/") + fhParam1 + QString("/obituaries/archives");
            URLaddressTemplate += PQString("?page=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = fhParam2.toUInt();
        }
        else
        {
            URLbase = baseURL + QString("/") + fhParam1 + QString("/obituaries");
            URLaddressTemplate  = URLbase;

            URLparams.numParams = 0;
            flowParameters.currentPosition = 0;
            flowParameters.endingPosition = 0;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1008:  // FuneralTech
    {
        if (fhSpecialCode == 3)
        {
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/obituaries/obituary-listings?page=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup)
            {
                flowParameters.endingPosition = fhParam2.toInt();
                daysOfOverlap = 261;
            }
            else
                flowParameters.endingPosition = 2; // 2

            // Unique situation where a very specific preflight OPTION request is required
            preflightRequest.instructions.verb = QString("OPTIONS");
            preflightRequest.instructions.Accept = QString("ALL");
            preflightRequest.instructions.Access_Control_Request_Method = QString("GET");
            preflightRequest.instructions.Access_Control_Request_Headers = QString("domainid");
            preflightRequest.instructions.Origin = baseURL.getString();
            preflightRequest.instructions.User_Agent = QString("STD");
            preflightRequest.instructions.Sec_Fetch_Mode = QString("cors");
            preflightRequest.instructions.Sec_Fetch_Site = QString("cross-site");
            preflightRequest.instructions.Sec_Fetch_Dest = QString("empty");
            preflightRequest.instructions.Referer = baseURL.getString() + QString("/");
            preflightRequest.instructions.Accept_Encoding = QString("br");
            preflightRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");
        }
        else
        {
            // This is old approach applicable to 99% of sites
            // https://ogden.funeraltechweb.com/tribute/current-services/index.html?page=0
            daysOfOverlap = 161;
            flowParameters.initialSetup = true;
            flowParameters.flowType = startToEnd;
            flowParameters.indexOffset = 1;
            flowParameters.flowIncrement = 1;

            if (fhParam1.size() == 0)
                baseURL = URL;
            else
                baseURL = fhParam1 + QString(".") + URL;
            validateURL(HTTP, hostname, baseURL);

            URLbase = baseURL + QString("/tribute/all-services/index.html");
            URLaddressTemplate  = URLbase;
            URLaddressTemplate += PQString("?page=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 0;
            if (flowParameters.initialSetup)
                flowParameters.endingPosition = fhParam2.toUInt();
            else
                flowParameters.endingPosition = 3;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

    }
        break;

    case 1009:  // WordPress
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.indexOffset = 1;
        flowParameters.flowIncrement = 1;

        URLbase = baseURL + QString("/category/we-remember/page/");
        URLaddressTemplate  = URLbase;
        URLaddressTemplate += paramPlaceholder + QString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        flowParameters.endingPosition = fhParam2.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1010:  // Frontrunner
    {
        // Very different approaches in use
        daysOfOverlap = 61;

        switch(fhSpecialCode)
        {
        case 0:
            // Historical - Loop through pages
            // Active - Last xx days is always pulled in a single page
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;
            flowParameters.indexOffset = 1;    // Needed because first page is 0
            flowParameters.flowIncrement = 10;

            URLparams.numParams = 3;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &nonConstantProviderKey;
            URLparams.param2Type = ptQS;
            URLparams.QSparam2 = &fhParam1;
            URLparams.param3Type = ptUint;
            URLparams.UIparam3 = &flowParameters.currentPosition;

            if (flowParameters.initialSetup)
            {
                URLbase = baseURL + QString("/runtime.php?SiteId=");

                URLaddressTemplate  = URLbase;
                URLaddressTemplate += paramPlaceholder + QString("&NavigatorId=") + paramPlaceholder + QString("&ListType=past&start=") + paramPlaceholder + QString("&StartLetter=ANY");
                flowParameters.currentPosition = 0;
                flowParameters.endingPosition = static_cast<unsigned int>(flowParameters.flowIncrement * fhParam2.toInt());
                break;
            }
            else
            {
                URLbase = baseURL + QString("/runtime.php?SiteId=");

                URLaddressTemplate  = URLbase;
                URLaddressTemplate += paramPlaceholder + QString("&NavigatorId=") + paramPlaceholder + QString("&ListType=current&start=") + paramPlaceholder;
                flowParameters.currentPosition = 0;
                flowParameters.endingPosition = 0;
                break;
            }
            break;

        case 1:
        case 2:
            // Read JSON data directly
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;
            flowParameters.currentPosition = 1;

            downloadRequest.instructions.verb = QString("POST");

            URLparams.numParams = 2;
            URLparams.param1Type = ptQS;
            URLparams.QSparam1 = &fhParam1;
            URLparams.param2Type = ptUint;
            URLparams.UIparam2 = &flowParameters.currentPosition;

            if (fhSpecialCode == 1)
            {
                URLaddressTemplate  = PQString("https://obituaries.frontrunnerpro.com/runtime/311039/ims/WF2/public/get-records-additional.php?getServiceType=shiva&guid=") + paramPlaceholder;
                URLaddressTemplate += PQString("&height=216.3&pageNum=") + paramPlaceholder;
                URLaddressTemplate += PQString("&rpp=15&sort=dod&template=below&type=all&wholeSite=true&width=181.65");
            }
            else
            {
                URLaddressTemplate  = PQString("https://obituaries.frontrunnerpro.com/runtime/311039/ims/WF2/public/get-records-additional.php?getServiceType=shiva&guid=") + paramPlaceholder;
                URLaddressTemplate += PQString("&height=216.3&pageNum=") + paramPlaceholder;
                URLaddressTemplate += PQString("&rpp=8&sort=dod&template=beside&type=all&forceToCurrent=true&includeAdditionalServices=true&wholeSite=true&width=181.65");
            }

            if (flowParameters.initialSetup)
                flowParameters.endingPosition = 20;
            else
                flowParameters.endingPosition = 5; //5

            break;

        case 3:
        {
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/obituaries/obituary-listings?page=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup)
            {
                flowParameters.endingPosition = fhParam2.toInt();
                daysOfOverlap = 391;
            }
            else
                flowParameters.endingPosition = 2; // 2

            // Unique situation where a very specific preflight OPTION request is required
            preflightRequest.instructions.verb = QString("OPTIONS");
            preflightRequest.instructions.Accept = QString("ALL");
            preflightRequest.instructions.Access_Control_Request_Method = QString("GET");
            preflightRequest.instructions.Access_Control_Request_Headers = QString("domainid");
            preflightRequest.instructions.Origin = baseURL.getString();
            preflightRequest.instructions.User_Agent = QString("STD");
            preflightRequest.instructions.Sec_Fetch_Mode = QString("cors");
            preflightRequest.instructions.Sec_Fetch_Site = QString("cross-site");
            preflightRequest.instructions.Sec_Fetch_Dest = QString("empty");
            preflightRequest.instructions.Referer = baseURL.getString() + QString("/");
            preflightRequest.instructions.Accept_Encoding = QString("br");
            preflightRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");
        }
            break;

        case 4:
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            URLbase = PQString("https://memorials.hpmcgarry.ca/sitemap.php?pageNum=");
            URLaddressTemplate  = URLbase + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup)
                flowParameters.endingPosition = 50;
            else
                flowParameters.endingPosition = 1;
            break;

        case 5:
            if (flowParameters.initialSetup)
            {
                URLparams.numParams = 4;
                URLparams.param1Type = ptUint;
                URLparams.UIparam1 = &nonConstantProviderKey;
                URLparams.param2Type = ptUint;
                URLparams.UIparam2 = &nonConstantProviderKey;
                URLparams.param3Type = ptQS;
                URLparams.QSparam3 = &fhParam1;
                URLparams.param4Type = ptUint;
                URLparams.UIparam4 = &flowParameters.currentPosition;

                URLbase = baseURL;

                URLaddressTemplate  = URLbase + PQString("/runtime/") + paramPlaceholder + PQString("/runtime.php?SiteId=") + paramPlaceholder;
                URLaddressTemplate += PQString("&NavigatorId=") + paramPlaceholder + QString("&ListType=past&start=") + paramPlaceholder + QString("&StartLetter=ANY");
                flowParameters.currentPosition = 0;
                flowParameters.endingPosition = static_cast<unsigned int>(flowParameters.flowIncrement * fhParam2.toInt());
            }
            else
            {
                URLparams.numParams = 3;
                URLparams.param1Type = ptUint;
                URLparams.UIparam1 = &nonConstantProviderKey;
                URLparams.param2Type = ptUint;
                URLparams.UIparam2 = &nonConstantProviderKey;
                URLparams.param3Type = ptQS;
                URLparams.QSparam3 = &fhParam1;

                URLbase = baseURL;

                URLaddressTemplate  = URLbase + PQString("/runtime/") + paramPlaceholder + PQString("/runtime.php?SiteId=") + paramPlaceholder;
                URLaddressTemplate += PQString("&NavigatorId=") + paramPlaceholder;
                flowParameters.currentPosition = 0;
                flowParameters.endingPosition = static_cast<unsigned int>(flowParameters.flowIncrement * fhParam2.toInt());
            }
            break;

        case 6:
            /*flowParameters.initialSetup = false;
            flowParameters.flowType = monthly;
            flowParameters.monthCurrent = 1;
            flowParameters.monthEnd = 12;
            flowParameters.currentMonth = PQString::MONTHS.at(flowParameters.monthCurrent);
            pageVariables.usePubDateForCutOff = true;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/page/") + paramPlaceholder + PQString("/");
            URLaddressTemplate += PQString("?s=") + paramPlaceholder + PQString("&et_pb_searchform_submit=et_search_proccess&et_pb_include_posts=yes&et_pb_include_pages=yes&sort=dod");

            URLparams.numParams = 2;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;
            URLparams.param2Type = ptQS;
            URLparams.QSparam2 = &flowParameters.currentMonth;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 100;*/

            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/?et_blog");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup)
                flowParameters.endingPosition = 100;
            else
                flowParameters.endingPosition = 3;
            break;

        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1011:  // FuneralOne
    {
        switch(providerKey)
        {
        default:
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            //https://www.kennedyfh.com/obituaries/api/search.json?orderBy=DeathDate&pageSize=12&pageNumber=2
            //https://www.kennedyfh.com/obituaries/api/search.json?orderBy=DeathDate&pageNumber=1&pageSize=12
            URLbase = baseURL + PQString("/obituaries/");

            URLaddressTemplate  = URLbase;
            URLaddressTemplate += PQString("api/search.json?");
            URLaddressTemplate += PQString("orderBy=DeathDate");
            URLaddressTemplate += PQString("&pageNumber=") + paramPlaceholder;
            URLaddressTemplate += PQString("&pageSize=12");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup)
                flowParameters.endingPosition = fhParam2.toInt();
            else
                flowParameters.endingPosition = 5;

            determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
            createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

            downloadRequest.instructions.verb = QString("POST");
            downloadRequest.instructions.Accept = QString("application/json, text/javascript, */*; q=0.01");
            downloadRequest.instructions.ContentTypeHeader = QString("application/x-www-form-urlencoded; charset=UTF-8");
            downloadRequest.instructions.X_Requested_With = QString("XMLHttpRequest");
            downloadRequest.instructions.Origin = baseURL.getString();
            downloadRequest.instructions.Sec_Fetch_Mode = QString("cors");
            downloadRequest.instructions.Sec_Fetch_Site = QString("same-origin");
            downloadRequest.instructions.Sec_Fetch_Dest = QString("empty");
            //downloadRequest.instructions.Accept_Encoding = QString("gzip, deflate, br");
            //downloadRequest.instructions.Accept_Encoding = QString("br");
            downloadRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");
            downloadRequest.instructions.Referer = URLbase.getString();

            break;

        case 449:
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase;
            URLaddressTemplate += PQString("/obituaries?ede02939_page=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup)
                flowParameters.endingPosition = 4;
            else
                flowParameters.endingPosition = 2;

            determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
            createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

            break;

        case 4332:
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            downloadRequest.instructions.verb = QString("POST");

            URLbase = baseURL;
            URLaddressTemplate  = URLbase;
            URLaddressTemplate += PQString("/obituaries/ObitSearchList/") + paramPlaceholder;
            URLaddressTemplate += PQString("?filter=&sortby=&showing=&location=&targetNew=false");
            //URLaddressTemplate += PQString("/obituaries/#!/Sort:DateOfDeath/Page:") + paramPlaceholder;
            //https://www.steckleygooderham.com/obituaries/ObitSearchList/2?filter=&filterSort=0&letter=&locationId=&showing=&targetNew=false
            //URLaddressTemplate += PQString("/obituaries/ObitSearchList/") + paramPlaceholder;
            //URLaddressTemplate += PQString("?filter=&filterSort=0&letter=&locationId=&showing=&targetNew=false");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 4;

            determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
            createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

            break;

        }
    }
        break;

    case 1012:  // WebStorm
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        //URLaddressTemplate  = URLbase + QString("obituaries-pre.php");
        //URLaddressTemplate  = URLbase + QString("obituaries-all-chronological.php");
        URLaddressTemplate  = URLbase + PQString("/obituaries/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 8;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1013:  // GasLamp
    {
        // Can loop through recent to old

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL + QString("/");
        URLaddressTemplate  = URLbase + QString("obituaries/?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 1;
        else
            flowParameters.endingPosition = fhParam2.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1014:  // ClickTributes
    {
        // Can loop through recent to old

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL + QString("/");
        URLaddressTemplate  = URLbase + QString("obituaries/?pg=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 2;
        else
            flowParameters.endingPosition = fhParam2.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1015:  // Connelly McKinley
    {
        // Can loop through recent to old

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL + QString("/");
        URLaddressTemplate  = URLbase + QString("obituaries/page/") + paramPlaceholder + QString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 4;
        else
            flowParameters.endingPosition = fhParam2.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1016:  // Arbor Memorial
    {
        switch(providerKey)
        {
        // Oddbal sites
        case 29:
        case 33:
        case 47:
        case 98:
        case 99:
        case 100:
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;
            flowParameters.flowIncrement = 10;

            // https://rideaumemorial.sharingmemories.ca/memlist.html?start=740&limit=10&searchText=&page=75
            // https://maisongarneau.sharingmemories.ca/memlist.html?start=10&limit=10&sort=dod&dir=DESC&searchText=&memorialTypeIds=0,4&scope=&vendorId=178&enabled=1&livingMemorials=false&surnameOnly=false&startsWith=false
            // https://maisongarneau.sharingmemories.ca/memlist.html?start=10&limit=10&sort=dod&dir=DESC&searchText=&memorialTypeIds=0,4&scope=&vendorId=178&enabled=1&livingMemorials=false&surnameOnly=false&startsWith=false

            //Request URL: https://funerairearmstrong.sharingmemories.ca/memlist.html?start=10&limit=10&sort=dod&dir=DESC&searchText=&memorialTypeIds=0,4&scope=&vendorId=177&enabled=1&livingMemorials=false&surnameOnly=false&startsWith=false
            //             https://mountroyalcem.permavita.com/memlist.html?start=630&limit=10&sort=dod&dir=DESC&searchText=&memorialTypeIds=0,4&scope=&vendorId=109&enabled=1&livingMemorials=false&surnameOnly=false&startsWith=false
            URLaddressTemplate = baseURL + PQString("/memlist.html?start=") + paramPlaceholder;
            URLaddressTemplate += PQString("&limit=10&sort=dod&dir=DESC&searchText=&memorialTypeIds=0,4&scope=&vendorId=") + fhParam2;
            URLaddressTemplate += PQString("&enabled=1&livingMemorials=false&surnameOnly=false&startsWith=false");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 0;
            if (flowParameters.initialSetup)
                flowParameters.endingPosition = fhParam1.toUInt();
            else
                flowParameters.endingPosition = 50;

            break;

        default:
            // Can loop through recent to old
            // Download retrieves JSON data

            daysOfOverlap = 61;

            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;
            flowParameters.flowIncrement = 100;

            // https://searchg2.crownpeak.net/arbormemorial-live-realtime/select?q=&q.alt=*&echoParams=explicit&fl=*,score&defType=edismax&wt=json&start=
            //          10&rows=10&spellcheck=true&spellcheck.collate=true&spellcheck.collateExtendedResults=true&fq=custom_s_template%3A13418&fq=custom_s_branchkey%3Akelly&sort=custom_dt_deceased_death_date%20desc,custom_s_deceased_name%20asc&hl=true&hl.fl=*&hl.snippets=3&hl.simple.pre=%3Cb%3E&hl.simple.post=%3C/b%3E&f.title.hl.fragsize=50000&f.url.hl.fragsize=50000&json.wrf=searchg2_8899221425779675&qf=

            URLaddressTemplate =  PQString("https://searchg2.crownpeak.net/arbormemorial-live-realtime/select?q=&q.alt=*&echoParams=explicit&fl=*,score&defType=edismax&wt=json&start=") + paramPlaceholder;
            URLaddressTemplate += PQString("&rows=") + paramPlaceholder;
            URLaddressTemplate += PQString("&spellcheck=true&spellcheck.collate=true&spellcheck.collateExtendedResults=true&fq=custom_s_template%3A13418&fq=custom_s_branchkey%3A") + paramPlaceholder;
            URLaddressTemplate += PQString("&sort=custom_dt_deceased_death_date%20desc,custom_s_deceased_name%20asc&hl=true&hl.fl=*&hl.snippets=3&hl.simple.pre=%3Cb%3E");
            URLaddressTemplate += PQString("&hl.simple.post=%3C/b%3E&f.title.hl.fragsize=50000&f.url.hl.fragsize=50000&json.wrf=searchg2_") + fhParam2 + PQString("&qf=");

            URLparams.numParams = 3;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;
            URLparams.param2Type = ptInt;
            URLparams.IntParam2 = &flowParameters.flowIncrement;
            URLparams.param3Type = ptQS;
            URLparams.QSparam3 = &fhParam1;

            flowParameters.currentPosition = 0;
            flowParameters.endingPosition = 99;
            break;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1017:  // SiteWyze
    {
        // Can loop through recent to old

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL + QString("/");
        URLaddressTemplate  = URLbase + QString("?pg=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 2;
        else
            flowParameters.endingPosition = fhParam2.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1017

    case 1018:  // Thinking Thru
    {
        // Single download pulls everything current due to low frequency
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = URL;
        validateURL(HTTP, hostname, URLbase);

        baseURL = URLbase;
        URLaddressTemplate = URLbase + QString("/obituaries/");

        URLparams.numParams = 0;
        flowParameters.currentPosition = 0;
        flowParameters.endingPosition = 0;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 1018

    case 1019:  // Codesign
    {
        // Can loop through recent to old

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL + QString("/");
        URLaddressTemplate  = URLbase + QString("category/obituaries/page/") + paramPlaceholder + QString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 3;
        else
            flowParameters.endingPosition = fhParam2.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1020:  // Shape5
    {
        // Can loop through recent to old

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 5;

        URLbase = baseURL + QString("/");
        URLaddressTemplate  = URLbase + QString("index.php/en/memorials?start=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 1;
        else
            flowParameters.endingPosition = fhParam2.toUInt() * static_cast<unsigned int> (flowParameters.flowIncrement);

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1021:  // Tribute Archive
    {
        // Instead of running individual cities and/or papers, can run Canada-wide with single pass
        // Extra DB lookup is used to assign the proper code

        // Can loop through recent to old

        daysOfOverlap = 61;  // Used in relation to publishDate
        pageVariables.usePubDateForCutOff = true;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.obitsPerPage = 240;

        URLbase = baseURL;

        switch(providerKey)
        {
        /*case 1:
            URLaddressTemplate  = URLbase + PQString("/obituaries/obituaries/search?sort_by=date&order=desc&search_type=advanced&ap_c=") + paramPlaceholder;
            URLaddressTemplate += PQString("&filter_date=anytime&limit=") + paramPlaceholder;
            URLaddressTemplate += PQString("&p=") + paramPlaceholder;

            URLparams.numParams = 3;
            URLparams.param1Type = ptQS;
            URLparams.QSparam1 = &fhParam1;
            URLparams.param2Type = ptUint;
            URLparams.UIparam2 = &flowParameters.obitsPerPage;
            URLparams.param3Type = ptUint;
            URLparams.UIparam3 = &flowParameters.currentPosition;
            break;

        case 2:
            URLaddressTemplate  = URLbase + PQString("/obituaries/obituaries/search?Waterfall=0&limit=") + paramPlaceholder;
            URLaddressTemplate += PQString("&p=") + paramPlaceholder;

            URLparams.numParams = 2;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.obitsPerPage;
            URLparams.param2Type = ptUint;
            URLparams.UIparam2 = &flowParameters.currentPosition;
            break;

        case 3:
            URLaddressTemplate  = URLbase + PQString("/obituaries/obituaries/search?limit=") + paramPlaceholder;
            URLaddressTemplate += PQString("&p=") + paramPlaceholder;

            URLparams.numParams = 2;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.obitsPerPage;
            URLparams.param2Type = ptUint;
            URLparams.UIparam2 = &flowParameters.currentPosition;
            break;

        case 4:
            URLaddressTemplate  = URLbase + PQString("/obituaries/obituaries/search?p=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;
            break;*/

        case 9999:
            URLaddressTemplate  = URLbase + PQString("/obituaries/obituaries/search?filter_date=past30days&sort_by=date&order=desc&p=") + paramPlaceholder;
            //URLaddressTemplate += PQString("&ckprm=1");
            //URLaddressTemplate  = URLbase + PQString("/obituaries/obituaries/search?filter_date=pastyear&sort_by=date&order=desc&p=") + paramPlaceholder;
            URLaddressTemplate += PQString("&ckprm=1");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;
            break;

        default:
            URLaddressTemplate  = URLbase + PQString("/obituaries/obituaries/search?Waterfall=0&limit=") + paramPlaceholder;
            URLaddressTemplate += PQString("&p=") + paramPlaceholder;
            URLaddressTemplate += PQString("&ckprm=1");

            URLparams.numParams = 2;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.obitsPerPage;
            URLparams.param2Type = ptUint;
            URLparams.UIparam2 = &flowParameters.currentPosition;
            break;
        }

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == false)
        {
            if (providerID == 9999)
                flowParameters.endingPosition = 9;
            else
                flowParameters.endingPosition = 3;
        }
        else
        {
            flowParameters.endingPosition = 40;
            flowParameters.obitsPerPage = 240;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1022:  // YAS
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase;
        URLaddressTemplate += PQString("/obituaries-life-stories/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = fhParam2.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1023:  // WFFH
    {
        // JSON approach - single listing
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = URL;
        validateURL(HTTP, hostname, URLbase);

        URLaddressTemplate = URLbase + PQString("/ajax/dataService.asmx/getStories?_=1591468800336");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

    }   // end case 1023
        break;

    case 1024:  // FHW Solutions
    {
        daysOfOverlap = 61;
        pageVariables.firstAvailableDate.setDate(fhFirstObit.year(), fhFirstObit.month(), 1);
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        switch(providerKey)
        {
        case 1:
            flowParameters.flowIncrement = 1;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/obituaries/") + paramPlaceholder + PQString("/");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
            break;

        case 53:
            // JSON approach
            flowParameters.flowIncrement = 50;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/fhws/cfc/obituary.cfc?method=getObituariesSearch&client_id=") + paramPlaceholder;
            URLaddressTemplate += PQString("&search=&sortBy=0&startAt=") + paramPlaceholder;

            URLparams.numParams = 2;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &nonConstantProviderKey;
            URLparams.param2Type = ptUint;
            URLparams.UIparam2 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = fhParam1.toUInt();
            break;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1025:  // Crew
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL + QString("/obituaries/");
        URLaddressTemplate  = URLbase + QString("page/");
        URLaddressTemplate += paramPlaceholder + QString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 5;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // Crew

    case 1026:  // Jelly
    {
        // Sophisticated JSON swapping site where only initial screen is readable (i.e. no history could be obtained)

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL + QString("/obituaries/");
        URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition  = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1026 Jelly

    case 1027:  // Nirvana
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL + QString("/obituaries/");
        URLaddressTemplate  = URLbase + QString("page/");
        URLaddressTemplate += paramPlaceholder + QString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1027

    case 1028:  // Bergetti
    {
        // Very low volume
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL + QString("/category/obituaries/");
        URLaddressTemplate  = URLbase;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1028 Bergetti

    case 1029:  // PacificByte
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL + QString("/obituaries/");
        URLaddressTemplate  = URLbase + QString("?pg=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 15;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1029  PacificByte

    case 1030:  // ROI
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/online-tributes/page/")+ paramPlaceholder + QString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 10;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1030  ROI

    case 1031:  // Vernon
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        //URLaddressTemplate  = URLbase + QString("/latest-news/page/")+ paramPlaceholder + QString("/");
        URLaddressTemplate  = URLbase + QString("/latest-news/");

        URLparams.numParams = 0;
        //URLparams.numParams = 1;
        //URLparams.param1Type = ptUint;
        //URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1031  Vernon

    case 1032:  // Gustin
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL + QString("/obituaries/");
        URLaddressTemplate  = URLbase + QString("?f=obits&pagenum=")+ paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 5;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1032  Gustin

    case 1033:  // Ashlean
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        // https://voyagefuneralhomes.com/wp-admin/admin-ajax.php
        // pid=5314+elementor&wid=3fab224&cpid=&page_num=2&curr_url=https%253A%252F%252Fvoyagefuneralhomes.com%252Fcategory%252Fobituaries%252F&action=ae_post_data&fetch_mode=paged&nonce=075f69e0a3
        // pid=5314+elementor&wid=3fab224&cpid=&page_num=2&curr_url=https%253A%252F%252Fvoyagefuneralhomes.com%252Fcategory%252Fobituaries%252F&action=ae_post_data&fetch_mode=paged&nonce=b8f349b4ab

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/wp-admin/admin-ajax.php?pid=5314+elementor&wid=3fab224&cpid=&page_num=") + paramPlaceholder;
        URLaddressTemplate += PQString("&curr_url=https%253A%252F%252Fvoyagefuneralhomes.com%252Fcategory%252Fobituaries%252F&action=ae_post_data&fetch_mode=paged&nonce=b8f349b4ab");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 7; // 10

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");
    }
        break; // 1033 Ashlean

    case 1034:  // Halifax
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1034 Halifax

    case 1035:  // Specialty
    {
        daysOfOverlap = 61;
        pageVariables.firstAvailableDate.setDate(fhFirstObit.year(), fhFirstObit.month(), 1);
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        baseURL = URL;
        if (baseURL.right(1) != PQString("/"))
            baseURL += PQString("/");

        validateURL(HTTP, hostname, baseURL);

        if (flowParameters.initialSetup)
        {
            if (providerKey == 31)
                baseURL += PQString("Obituaries/");
        }
        else
        {
            if (providerKey == 31)
                baseURL += PQString("Obituaries/");
            else
            {
                switch(fhSpecialCode)
                {
                case 0:
                    baseURL += PQString("obituaries/");
                    break;

                case 1:
                case 2:
                    baseURL += PQString("");
                    break;
                }
            }
        }

        URLbase = baseURL;

        switch(fhSpecialCode)
        {
        case 0:
            URLaddressTemplate = URLbase + PQString("search?first=&last=&dod=");
            break;

        case 1:
            URLaddressTemplate = URLbase + PQString("obitsearch?first=&last=&dod=");
            break;

        case 2:
            URLaddressTemplate = URLbase + PQString("obituaries");
            break;

        }

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 1036:  // ReneBabin
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1036 ReneBabin

    case 1037:  // Smiths
    {
        // Only pull current from main page
        daysOfOverlap = 90;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1037 Smiths

    case 1038:  // SeaBreeze
    {
        // Very unique process to retrieve obits
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        // https://peifuneralcoops.com/setpage.php?page=Central_Queens.php&set=Notices&type=recent
        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/setpage.php?page=") + fhParam1 + PQString(".php&set=Notices&type=");
        if (flowParameters.initialSetup)
            URLaddressTemplate += PQString("archived");
        else
            URLaddressTemplate += PQString("recent");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1038 SeaBreeze

    case 1039:  // RedSands
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/category/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1039 RedSands

    case 1040:  // AdGraphics
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituary_notices.html");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1040 AdGraphics

    case 1041:  // Websites
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + fhParam1;

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1041 Websites

    case 1042:  // MPG
    {
        daysOfOverlap = 61;
        pageVariables.firstAvailableDate.setDate(fhFirstObit.year(), fhFirstObit.month(), 1);
        flowParameters.initialSetup = false;
        flowParameters.flowType = dateRange;
        flowParameters.flowIncrement = -7;

        if (flowParameters.initialSetup)
            pageVariables.cutoffDate = pageVariables.firstAvailableDate;
        else
            pageVariables.cutoffDate = pageVariables.latestDODdate.addDays(-daysOfOverlap);
        pageVariables.queryEndDate = globals->today;
        pageVariables.queryStartDate = pageVariables.queryEndDate.addDays(flowParameters.flowIncrement + 1);

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/en-CA/Find-Services?fromDate=") + paramPlaceholder + PQString("&toDate=") + paramPlaceholder;
        // 01%2F17%2F2021&toDate=01%2F24%2F2021
        //https://www.mountpleasantgroup.com/en-CA/Find-Services.aspx?fromDate=09%2F04%2F2021&toDate=09%2F12%2F2021

        URLparams.numParams = 2;
        URLparams.param1Type = ptQTDmask;
        URLparams.QTDdate1 = &pageVariables.queryStartDate;
        URLparams.maskType1 = mtMMsDDsYYYY;
        URLparams.param2Type = ptQTDmask;
        URLparams.QTDdate2 = &pageVariables.queryEndDate;
        URLparams.maskType2 = mtMMsDDsYYYY;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

    }   // 1042  MPG
        break;

    case 1043:  // MediaHosts
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries.php");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1043 MediaHosts

    case 1044:  // DragonFly
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1044 DragonFly

    case 1045:  // MyFavourite
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1045 MyFavorite

    case 1046:  // Coop
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        // https://www.fco-cfo.coop/en/death-notices/?page=4
        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/en/death-notices/?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1046 Coop

    case 1047:  // EverTech
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/?term=&pg=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 10;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1047 EverTech

    case 1048:  // Maclean
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/death-notices/obituaries/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1048 Maclean

    case 1049:  // MCG
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;

        if (providerKey == 3)
        {
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/funerals/");

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }
        else
        {
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/obituaries/cwp-funerals/0/") + paramPlaceholder + PQString("/");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup == true)
                flowParameters.endingPosition = fhParam1.toUInt();
            else
                flowParameters.endingPosition = 2;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1049 MCG

    case 1051:  // WebSolutions
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/en/obituaries?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1051 WebSolutions

    case 1052:  // Citrus  (see also 2152 MLBW)
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/necrologies/");
        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        www->download(downloadRequest);
        while(www->processingDownload()){};
        if(www->lastDownloadSuccessful())
        {
            sourceFile.setSourceFile(downloadRequest.outputs.downloadFileName);
            if (sourceFile.consecutiveMovesTo(300, "JetEngineSettings", "ajaxlisting", ":"))
            {
                URLbase = sourceFile.readNextBetween(QUOTES);
                URLbase.JSONsimplify();
                URLaddressTemplate = URLbase;
                URLaddressTemplate += PQString("&action=jet_engine_ajax&handler=get_listing&query%5Bpost_status%5D=publish&query%5Bfound_posts%5D=1281&query%5Bmax_num_pages%5D=129");
                URLaddressTemplate += PQString("&query%5Bpost_type%5D=necrologies&query%5Borderby%5D=&query%5Border%5D=DESC&query%5Bpaged%5D=0&query%5Bposts_per_page%5D=10&page_settings%5Bpost_id%5D=1882");
                URLaddressTemplate += PQString("&page_settings%5Bqueried_id%5D=5513%7CWP_Post&page_settings%5Belement_id%5D=3d3bc47&page_settings%5Bpage%5D=") + paramPlaceholder;
                URLaddressTemplate += PQString("&listing_type=elementor&isEditMode=false");

                downloadRequest.instructions.verb = QString("POST");

                URLparams.numParams = 1;
                URLparams.param1Type = ptUint;
                URLparams.UIparam1 = &flowParameters.currentPosition;

                flowParameters.currentPosition = 1;
                flowParameters.endingPosition = 8;

                determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
                createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
            }
        }
     }
        break; // 1052 Citrus

    case 1054:  // TNours
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avisdedeces?ccm_paging_p=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1054 TNours

    case 1056:  // Sandfire
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1056 Sandfire

    case 1057:  // Carve
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;
        downloadRequest.instructions.verb = QString("POST");

        URLbase = baseURL;
        //URLaddressTemplate  = URLbase + PQString("/obits.php?p=") + paramPlaceholder;
        URLaddressTemplate  = URLbase + PQString("/obituaries/?nocache=1659219487");
        URLaddressTemplate += PQString("?action=jet_engine_ajax&handler=get_listing&page_settings%5Bpost_id%5D=880&page_settings%5Bqueried_id%5D=4295%7CWP_Post");
        URLaddressTemplate += PQString("&page_settings%5Belement_id%5D=eb4ac1f&page_settings%5Bpage%5D=1&listing_type=elementor&isEditMode=false");

        URLparams.numParams = 0;
        //URLparams.param1Type = ptUint;
        //URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 0;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1057 Carve

    case 1058:  // SRS
    {
        // Historical and active same listing - Loop page by page
        // Same as 2006 - Pierson's

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;

        //https://www.bardal.ca/ObituariesHelper/ObituariesListPagedItems?testId=obituariesListPageItemsForm?PageSize=5&FirstPageObituariesCount=0&ObDomainId=46d8bada-f71d-4f38-910c-7cf71f14690c&CurrentPage=1&SearchText=&SortingColumn=13&Dates=0&SelectedLocationId=&HasNextPage=True&X-Requested-With=XMLHttpRequest
        //https://www.bardal.ca/ObituariesHelper/ObituariesListPagedItems?testId=obituariesListPageItemsForm?PageSize=5&FirstPageObituariesCount=0&ObDomainId=46d8bada-f71d-4f38-910c-7cf71f14690c&CurrentPage=2&SearchText=&SortingColumn=3&Dates=0&SelectedLocationId=&HasNextPage=True&X-Requested-With=XMLHttpRequest

        URLaddressTemplate  = URLbase + PQString("/ObituariesHelper/ObituariesListPagedItems?testId=obituariesListPageItemsForm?");
        URLaddressTemplate += PQString("PageSize=5&FirstPageObituariesCount=0&ObDomainId=") + paramPlaceholder + PQString("&CurrentPage=") + paramPlaceholder;
        URLaddressTemplate += PQString("&SearchText=&SortingColumn=3&Dates=0&SelectedLocationId=&HasNextPage=True");

        URLparams.numParams = 2;
        URLparams.param1Type = ptQS;
        URLparams.QSparam1 = &fhParam1;
        URLparams.param2Type = ptUint;
        URLparams.UIparam2 = &flowParameters.currentPosition;

        if (flowParameters.initialSetup)
        {
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = fhParam2.toUInt();
        }
        else
        {
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 3;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");
        downloadRequest.instructions.X_Requested_With = QString("XMLHttpRequest");

    }
        break;  // SRS

    case 1059:  // BrandAgent
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1059 BrandAgent

    case 1060:  // EP Media
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/category/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1060 EP Media

    case 1061:  // Linkhouse
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/death-notice/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1061 Linkhouse

    case 1063:  // LinkWeb
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries.php");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1063 LinkWeb

    case 1064:  // Josh Pascoe
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 10;

        //        ttps://www.mgdalyfuneralhome.com/index.php?option=com_mtree&task=listcats&cat_id=0&Itemid=26&limit=10&limitstart=0
        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/index.php?option=com_mtree&task=listcats&cat_id=0&Itemid=26&limit=10&limitstart=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 10;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1064 Josh Pascoe

    case 1065:  // Chad Simpson
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/services");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1065 Chad Simpson

    case 1066:  // Marketing Images
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituary.php?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1066 Marketing Images

    case 1067:  // Esite
    {
        switch(providerKey)
        {
        case 1:
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = lastValue;
            flowParameters.flowIncrement = -1;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/loadmore?startid=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 9999;
            if (flowParameters.initialSetup == true)
                flowParameters.endingPosition = fhParam1.toUInt();
            else
                flowParameters.endingPosition = 9999;
            break;

        case 2:
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;
            flowParameters.flowIncrement = 1;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/obituaries?page=") + paramPlaceholder + PQString("&searchstring=");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup == true)
                flowParameters.endingPosition = 5;
            else
                flowParameters.endingPosition = 1;
            break;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1067 Esite

    case 1068:  // SquareSpace
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries-and-notices");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1068 SquareSpace

    case 1069:  // Eggs
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1069 Eggs

    case 1070:  // MFH
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1070 MFH

    case 1071:  // Oneil
    {
        daysOfOverlap = 30;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        //http://www.legacy.com/funerals/Oneil-London/obituary-search.aspx?daterange=30&lastname=smith&countryid=2&stateid=47&affiliateid=3092
        URLbase = baseURL;
        URLaddressTemplate  = URLbase + paramPlaceholder + PQString("/obituary-search.aspx?daterange=30&countryid=2&stateid=") + paramPlaceholder;
        URLaddressTemplate += PQString("&affiliateid=") + paramPlaceholder;

        URLparams.numParams = 3;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        URLparams.param1Type = ptQS;
        URLparams.QSparam1 = &fhParam1;
        URLparams.param2Type = ptQS;
        URLparams.QSparam2 = &fhParam2;
        URLparams.param3Type = ptUint;
        URLparams.UIparam3 = &nonConstantProviderKey;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1071 Oneil

    case 1073:  // Back2Front
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/040~Funeral_Details/");

        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1071 Oneil

    case 1075:  // InView
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/category/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1075 InView

    case 1077:  // RKD
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/wp-admin/admin-ajax.php?");
        URLaddressTemplate += PQString("action=jet_smart_filters&provider=jet-engine%2Fdefault&defaults%5Bpost_status%5D%5B%5D=publish&defaults%5Bpost_type%5D=acf-death-notices&defaults%5Bposts_per_page%5D=40&defaults%5Bpaged%5D=1");
        URLaddressTemplate += PQString("&defaults%5Bignore_sticky_posts%5D=1&defaults%5Border%5D=DESC&defaults%5Borderby%5D=meta_value_num&defaults%5Bmeta_key%5D=death_date&defaults%5Bmeta_type%5D=DATE");
        URLaddressTemplate += PQString("&defaults%5Btax_query%5D%5B0%5D%5Btaxonomy%5D=notice-category&defaults%5Btax_query%5D%5B0%5D%5Bfield%5D=slug&defaults%5Btax_query%5D%5B0%5D%5Bterms%5D%5B%5D=death-notice");
        URLaddressTemplate += PQString("&defaults%5Btax_query%5D%5B0%5D%5Bterms%5D%5B%5D=both&defaults%5Btax_query%5D%5B0%5D%5Boperator%5D=IN&defaults%5Btax_query%5D%5B1%5D%5Btaxonomy%5D=display-type");
        URLaddressTemplate += PQString("&defaults%5Btax_query%5D%5B1%5D%5Bfield%5D=slug&defaults%5Btax_query%5D%5B1%5D%5Bterms%5D%5B%5D=display&defaults%5Btax_query%5D%5B1%5D%5Boperator%5D=IN&defaults%5Btax_query%5D%5Brelation%5D=AND");
        URLaddressTemplate += PQString("&settings%5Blisitng_id%5D=1426&settings%5Bcolumns%5D=4&settings%5Bcolumns_tablet%5D=3&settings%5Bcolumns_mobile%5D=2&settings%5Bpost_status%5D%5B%5D=publish&settings%5Buse_random_posts_num%5D=");
        URLaddressTemplate += PQString("&settings%5Bposts_num%5D=40&settings%5Bmax_posts_num%5D=9&settings%5Bnot_found_message%5D=No+data+was+found&settings%5Bis_masonry%5D=&settings%5Bequal_columns_height%5D=yes&settings%5Buse_load_more%5D=");
        URLaddressTemplate += PQString("&settings%5Bload_more_id%5D=&settings%5Bload_more_type%5D=click&settings%5Buse_custom_post_types%5D=&settings%5Bhide_widget_if%5D=&settings%5Bcarousel_enabled%5D=&settings%5Bslides_to_scroll%5D=1");
        URLaddressTemplate += PQString("&settings%5Barrows%5D=true&settings%5Barrow_icon%5D=fa+fa-angle-left&settings%5Bdots%5D=&settings%5Bautoplay%5D=true&settings%5Bautoplay_speed%5D=5000&settings%5Binfinite%5D=true");
        URLaddressTemplate += PQString("&settings%5Beffect%5D=slide&settings%5Bspeed%5D=500&settings%5Binject_alternative_items%5D=&settings%5Bscroll_slider_enabled%5D=&settings%5Bscroll_slider_on%5D%5B%5D=desktop");
        URLaddressTemplate += PQString("&settings%5Bscroll_slider_on%5D%5B%5D=tablet&settings%5Bscroll_slider_on%5D%5B%5D=mobile&props%5Bfound_posts%5D=6181&props%5Bmax_num_pages%5D=155&props%5Bpage%5D=3&paged=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 3;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");
    }
        break; // 1077 RKD

    case 1078:  // SDP
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 525;
            flowParameters.flowType = sequential;
            flowParameters.flowIncrement = -1;
            pageVariables.cutoffDate = pageVariables.firstAvailableDate;

            URLaddressTemplate  = URLbase + PQString("/en/avis-deces-archives/?fmois=") + paramPlaceholder + PQString("&fannee=") + paramPlaceholder;

            URLparams.numParams = 2;
            URLparams.param1Type = ptQTDmonth;
            URLparams.QTDdate1 = &pageVariables.queryTargetDate;
            URLparams.param2Type = ptQTDyear;
            URLparams.QTDdate2 = &pageVariables.queryTargetDate;
        }
        else
        {
            pageVariables.cutoffDate = pageVariables.latestDODdate.addDays(-daysOfOverlap);
            URLaddressTemplate  = URLbase + PQString("/en/avis-deces");

            URLparams.numParams = 0;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1078 SDP

    case 1079:  // Globalia
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/?pg=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1079 Globalia

    case 1080:  // Vortex
    {
        switch(providerKey)
        {
        case 1:
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/en/obituary.html");

            URLparams.numParams = 0;
            flowParameters.endingPosition = 1;

            break;

        case 3:
            daysOfOverlap = 61;

            pageVariables.firstAvailableDate.setDate(fhFirstObit.year(), fhFirstObit.month(), 1);
            flowParameters.initialSetup = false;
            flowParameters.flowType = sequential;
            flowParameters.flowIncrement = -1;
            flowParameters.numBlankDownloadLimit = 90;

            if (flowParameters.initialSetup)
                pageVariables.cutoffDate = pageVariables.firstAvailableDate;
            else
                pageVariables.cutoffDate = pageVariables.latestDODdate.addDays(-daysOfOverlap);

            URLbase = baseURL;

            URLaddressTemplate  = baseURL + QString("/avis-de-deces/?_obituary_name=&_obituary_firstname=&_obituary_date=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptQTDmask;
            URLparams.QTDdate1 = &pageVariables.queryTargetDate;
            URLparams.maskType1 = mtYYYYhMMhDD;

            break;

        default:
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/avis-deces/archives/?lettre=&motCle=&page=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup == true)
                flowParameters.endingPosition = 8;
            else
                flowParameters.endingPosition = 2;
            break;

        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1080 Vortex

    case 1081:  // Elegant - Girl
    {
        flowParameters.initialSetup = false;

        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 61;
            flowParameters.flowType = sequential;
            flowParameters.flowIncrement = -1;
            pageVariables.queryTargetDate = pageVariables.queryTargetDate.addDays(-10);

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/") + paramPlaceholder + PQString("/") + paramPlaceholder + PQString("/");

            URLparams.numParams = 2;
            URLparams.param1Type = ptQTDyear;
            URLparams.QTDdate1 = &pageVariables.queryTargetDate;
            URLparams.param2Type = ptQTDmonth;
            URLparams.QTDdate2 = &pageVariables.queryTargetDate;
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/category/avis-de-deces/");

            URLparams.numParams = 0;
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1081 Elegant

    case 1082:  // YellowPages
    {
        switch(providerKey)
        {
        case 1:
            // Only a single listing showing obits from last 30 days
            flowParameters.initialSetup = false;
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase;

            URLparams.numParams = 0;

            determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
            createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
            break;

        case 2:
            // Only a single listing showing obits from last 30 days
            flowParameters.initialSetup = false;
            daysOfOverlap = 61;
            flowParameters.flowType = startToEnd;
            flowParameters.flowIncrement = 14;

            //http://www.complexegendron.ca/php/index.php?start=14
            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/php/index.php?start=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 0;
            if (flowParameters.initialSetup)
                flowParameters.endingPosition = 140;
            else
                flowParameters.endingPosition = 0;
            determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
            createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
            break;
        }

    }
        break; // 1082 YellowPages

    case 1083:  // Shooga
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/category/avis-de-deces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1083 Shooga

    case 1084:  // NBL
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/deces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1084 NBL

    case 1085:  // WPBakery
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1085 WPBakery

    case 1086:  // Imago
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-deces/?pg=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 10;
        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1086 Imago

    case 1088:  // Ubeo
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;

        switch (providerKey)
        {
        case 1:
            flowParameters.flowType = startToEnd;
            flowParameters.flowIncrement = 10;

            //https://www.royetgiguere.com/ajax/recuperer_avis.php?limit=115&abrege=1&nombre_avis=5&maison_id=0
            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/ajax/recuperer_avis.php?");
            URLaddressTemplate += PQString("limit=") + paramPlaceholder;
            URLaddressTemplate += PQString("&abrege=1&nombre_avis=10&maison_id=0");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 0;
            if (flowParameters.initialSetup == true)
                flowParameters.endingPosition = fhParam1.toUInt();
            else
                flowParameters.endingPosition = 125;

            downloadRequest.instructions.verb = QString("POST");

            break;

        case 2:
            flowParameters.flowType = singleListing;

            URLbase = baseURL;

            if (flowParameters.initialSetup)
                URLaddressTemplate  = URLbase + PQString("/archives-avis-de-deces/");
            else
                URLaddressTemplate  = URLbase + PQString("/avis-de-deces/");

            URLparams.numParams = 0;
            flowParameters.currentPosition = 0;
            flowParameters.endingPosition = 0;
            break;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1088 Ubeo

    case 1090:  // Morin
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 1;

        URLbase = baseURL;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 599;
            pageVariables.cutoffDate = pageVariables.firstAvailableDate;

            URLaddressTemplate  = URLbase + PQString("/2021/page/") + paramPlaceholder + PQString("/?cat=29");

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 16;
        }
        else
        {
            pageVariables.cutoffDate = pageVariables.latestDODdate.addDays(-daysOfOverlap);

            URLaddressTemplate  = URLbase + PQString("/avis-deces/page/") + paramPlaceholder + PQString("/");

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 2;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1090 Morin

    case 1091:  // Taiga
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 1;
        pageVariables.cutoffDate = pageVariables.latestDODdate.addDays(-daysOfOverlap);

        // https://www.charronetfils.com/fr/avis/index.php?p=1&search=
        URLbase = baseURL;
        if (flowParameters.initialSetup)
            URLaddressTemplate  = URLbase + PQString("/fr/avis/archives.php?p=") + paramPlaceholder + PQString("&search=&yearid=2020");
        else
            URLaddressTemplate  = URLbase + PQString("/fr/avis/index.php?p=") + paramPlaceholder + PQString("&search=");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1091 Taiga

    case 1093:  // PubliWeb
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 1;
        pageVariables.cutoffDate = pageVariables.latestDODdate.addDays(-daysOfOverlap);

        URLbase = baseURL;
        if (providerKey == 1)
            //URLaddressTemplate  = URLbase + PQString("/AvisDeces/Index?page=") + paramPlaceholder + PQString("&mm=0&p=0");
            URLaddressTemplate  = URLbase + PQString("/avis-de-deces?page=") + paramPlaceholder;
        else
            URLaddressTemplate  = URLbase + PQString("/avis_deces_liste.asp?struid=&page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
        {
            if (providerKey == 1)
                flowParameters.endingPosition = 9;
            else
                flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1093 Publiweb

    case 1094:  // Magnus
    {
        // Unique construct where template is changed below after first download
        // Authentication key is captured during first download through "tempID"

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        pageVariables.numericDateOrder = doDMY;

        flowParameters.flowType = startToEnd;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 100;
        else
            flowParameters.endingPosition = 8;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/fr/avis-de-deces");
        URLparams.numParams = 0;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.Accept = QString("text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
        downloadRequest.instructions.Sec_Fetch_Mode = QString("navigate");
        downloadRequest.instructions.Sec_Fetch_Site = QString("same-origin");
        downloadRequest.instructions.Sec_Fetch_Dest = QString("document");
        //downloadRequest.instructions.Accept_Encoding = QString("br");
        downloadRequest.instructions.Accept_Encoding = QString("br");
        downloadRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");
        downloadRequest.instructions.Referer = URLaddressTemplate.getString();
    }
        break;

    case 1095:  // Soleweb
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/?s=&paged=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1095 Soleweb

    case 1096:  // Voyou
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        // fullname=&days=1&context=Site_Ajax&action=search&page=3&maxpp=&name=
        //
        URLbase = baseURL;

        if (providerKey < 3)
        {
            if (flowParameters.initialSetup == true)
                URLaddressTemplate  = URLbase + PQString("/ajax.html?fullname=&days=2&context=Site_Ajax&action=search&page=") + paramPlaceholder + PQString("&maxpp=&name=");
            else
                URLaddressTemplate  = URLbase + PQString("/ajax.html?fullname=&days=1&context=Site_Ajax&action=search&page=") + paramPlaceholder + PQString("&maxpp=&name=");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup == true)
                flowParameters.endingPosition = fhParam1.toUInt();
            else
                flowParameters.endingPosition = 2;

            downloadRequest.instructions.verb = QString("POST");
        }
        else
        {
            URLaddressTemplate  = URLbase + PQString("/wp/wp-admin/admin-ajax.php?action=filter_deces&afp_nonce=f6d3cc0c65&paged=") + paramPlaceholder + PQString("&search=");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup == true)
                flowParameters.endingPosition = 20;
            else
                flowParameters.endingPosition = 4;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

    }
        break; // 1096 Voyou

    case 1097:  // Scrypta
    {
        if (providerKey == 1)
        {
            // not currently working - default to short term fix below
            daysOfOverlap = 61;
            pageVariables.firstAvailableDate.setDate(fhFirstObit.year(), fhFirstObit.month(), 1);
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            //URLaddressTemplate  = URLbase + PQString("/fr/necrologie-avis-de-deces?page=") + paramPlaceholder+ PQString("&number_items=24&sort=d_dod%2Fdesc");
            URLaddressTemplate  = URLbase + PQString("/fetch_front_end_obituaries?reference_id=17b948bb-9f11-11ec-8ba8-02f387f5c980&type=list&current_page=") + paramPlaceholder+ PQString("&number_items_per_page=24&filters[sort]=d_dod%2Fdesc&action=null");

            // Initial download sets stage
            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 16;
            pageVariables.cutoffDate = pageVariables.latestDODdate.addDays(-daysOfOverlap);

            downloadRequest.instructions.verb = QString("POST");
            downloadRequest.instructions.Accept = QString("application/json, text/javascript, */*; q=0.01");
            //downloadRequest.instructions.ContentTypeHeader = QString("multipart/form-data");
            downloadRequest.instructions.ContentTypeHeader = QString("application/x-www-form-urlencoded; charset=UTF-8");
            downloadRequest.instructions.X_Requested_With = QString("XMLHttpRequest");
            downloadRequest.instructions.Origin = baseURL.getString();
            downloadRequest.instructions.Sec_Fetch_Mode = QString("cors");
            downloadRequest.instructions.Sec_Fetch_Site = QString("same-origin");
            downloadRequest.instructions.Sec_Fetch_Dest = QString("empty");
            //downloadRequest.instructions.Accept_Encoding = QString("br");
            downloadRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");
            downloadRequest.instructions.Referer = URLbase.getString() + QString("/fr/necrologie-avis-de-deces?page=1&number_items=24&sort=d_dod%2Fdesc");

        }
        else
        {
            daysOfOverlap = 261;
            pageVariables.firstAvailableDate.setDate(fhFirstObit.year(), fhFirstObit.month(), 1);
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/fetch_front_end_obituaries");

            URLparams.numParams = 0;

            QByteArray ba;
            QHttpPart part1, part2, part3, part4, part5, part6;
            part1.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"reference_id\""));
            part1.setBody("17b948bb-9f11-11ec-8ba8-02f387f5c980");
            part2.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"type\""));
            part2.setBody("list");
            part3.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"filters[sort]\""));
            part3.setBody("d_dod/desc");
            part4.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"number_items_per_page\""));
            part4.setBody("24");
            part5.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"action\""));
            part5.setBody("null");
            part6.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"current_page\""));

            for (unsigned int i = flowParameters.currentPosition; i <= flowParameters.endingPosition; i++)
            {
                multiPart = new QHttpMultiPart;
                multiPart->append(part1);
                multiPart->append(part2);
                multiPart->append(part3);
                multiPart->append(part4);
                multiPart->append(part5);
                ba.setNum(i);
                part6.setBody(ba);
                multiPart->append(part6);

                downloadRequest.instructions.multiPartList.append(multiPart);
                multiPart = nullptr;  // List will be deleted individually after reply received
            }

            downloadRequest.instructions.verb = QString("MULTIPART");
            downloadRequest.instructions.Accept = QString("application/json, text/javascript, */*; q=0.01");
            downloadRequest.instructions.ContentTypeHeader = QString("multipart/form-data");
            downloadRequest.instructions.X_Requested_With = QString("XMLHttpRequest");
            downloadRequest.instructions.Origin = baseURL.getString();
            downloadRequest.instructions.Sec_Fetch_Mode = QString("cors");
            downloadRequest.instructions.Sec_Fetch_Site = QString("same-origin");
            downloadRequest.instructions.Sec_Fetch_Dest = QString("empty");
            //downloadRequest.instructions.Accept_Encoding = QString("br");
            downloadRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");
            downloadRequest.instructions.Referer = URLbase.getString() + QString("/fr/necrologie-avis-de-deces?page=") + QString::number(flowParameters.currentPosition) + QString("&number_items=24&sort=d_dod/desc");
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1097 Scrypta

    case 1098:  // Jaimonsite
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces-services-funeraires?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 2;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1098 Jaimsonsite

    case 1099:  // Saguenay
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis");

        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1099 Saguenay

    case 1100:  // Lithium
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 12;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/php/ajax_loadnecro.php?nombreaffiche=") + paramPlaceholder;
        URLaddressTemplate += PQString("&fieldName=&limit=&fieldDate=");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 540;
        else
            flowParameters.endingPosition = 36;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");

    }
        break; // 1100 Lithium

    case 1101:  // Cameleon
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/fr/avis-de-deces/");

        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1101 Cameleon

    case 1102:  // LogiAction
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 1;

        URLbase = baseURL;
        if (providerID == 1)
            URLaddressTemplate  = URLbase + PQString("/avisdeces/?_pgn=") + paramPlaceholder;
        else
            URLaddressTemplate  = URLbase + PQString("/?_pgn=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 22;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1102 LogiAction

    case 1103:  // BLsolutions
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 1;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/category/avis-de-deces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 26;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1103 BLsolutions

    case 1104:  // Torapro
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 1;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/en/obituaries/page/") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 11;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1104 Torapro

    case 1105:  // Axanta
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 1;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/necrologie/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 20;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1105 Axanta

    case 1106:  // ADN
    {
        /*daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        pageVariables.firstAvailableDate.setDate(2019,12,27);
        pageVariables.cutoffDate = pageVariables.firstAvailableDate;
        flowParameters.flowType = dateRange;
        flowParameters.flowIncrement = -1;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/recherche/date/");
        URLaddressTemplate += paramPlaceholder + PQString("-") + paramPlaceholder + PQString("-") + paramPlaceholder;
        URLaddressTemplate += PQString("/page-1");

        pageVariables.queryEndDate.setDate(2020,2,7);
        pageVariables.queryStartDate.setDate(2020,2,7);

        URLparams.numParams = 3;
        URLparams.param1Type = ptQTDday;
        URLparams.QTDdate1 = &pageVariables.queryStartDate;
        URLparams.param2Type = ptQTDmonth;
        URLparams.QTDdate2 = &pageVariables.queryStartDate;
        URLparams.param3Type = ptQTDyear;
        URLparams.QTDdate3 = &pageVariables.queryStartDate;*/

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 1;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/page-") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 8;
        else
            flowParameters.endingPosition = 4;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1106 ADN

    case 1107:  // B367
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 1;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 53;
        else
            flowParameters.endingPosition = 5;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1107 B367

    case 1108:  // Tomahawk
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/?pg=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 50;
        else
            flowParameters.endingPosition = 5;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1108 Tomahawk

    case 1110:  // Caza
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 10;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces-en-ligne/?start=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 400;
        else
            flowParameters.endingPosition = 40;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1110 Caza

    case 1111:  // Tegara
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-deces?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 20;
        else
            flowParameters.endingPosition = 5;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1111 Tegara

    case 1112:  // NMedia
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 100;
        QByteArray payload;

        URLbase = baseURL; //
        URLaddressTemplate  = URLbase + PQString("/Service/InventoryService.asmx/GetProductsByFacets?_Facets&Search=&OrderBy_PropertyName0&IdentifierGuid=17d538cf-a248-d961-d652-4a1db6d995a4&IsDescOrdertrue&Index10&Count10fr-CAc00cfbe6-2a56-4d03-bdab-d7e0746b1968");

        payload  = QString("{\"args\":{\"FacetsConfiguration\":{\"Facets\":[{\"IdentifierGuidType\":3,\"IsExpandableSection\":false,\"IsExpandedByDefault\":true,\"IsMultipleSelection\":false,\"IsMutuallyExclusive\":true,\"IsResetSelection\":false,").toUtf8();
        payload += QString("\"IsRequired\":false,\"IsCheckBoxSelection\":false,\"IsProportionalSize\":false,\"ShowResultCount\":false,\"IsDropDown\":true,\"Width\":0,\"Height\":0,\"ValueFormat\":null,\"DropDownWaterMark\":\"Tous les complexes\",").toUtf8();
        payload += QString("\"DropDownWaterMarkResourceKey\":null,\"IsVerticalAlignement\":true,\"HideIfNoResult\":false,\"CustomText\":null,\"CustomTextResourceKey\":null,\"OrderedCustomFieldValueGuids\":[\"c6fcf35d-54f6-2ca5-842e-fa5e97afad11\",").toUtf8();
        payload += QString("\"39a7b55c-2463-1b60-90d0-77651b90cd99\",\"d4e34184-98cb-1384-0e90-9ce36ff8bba0\",\"fa1dff5e-16c0-24f1-646f-f61d9be797f6\"],\"Range\":null,\"IsDescOrder\":false,\"IsSortOrder\":false,").toUtf8();
        payload += QString("\"ParentTagGuid\":\"00000000-0000-0000-0000-000000000000\",\"ClassificationGuid\":\"00000000-0000-0000-0000-000000000000\",\"PropertyName\":0,\"IdentifierGuid\":\"3a04e611-8d55-a133-2737-a71c3a3b83dc\"},").toUtf8();
        payload += QString("{\"IdentifierGuidType\":3,\"IsExpandableSection\":false,\"IsExpandedByDefault\":true,\"IsMultipleSelection\":false,\"IsMutuallyExclusive\":true,\"IsResetSelection\":false,\"IsRequired\":false,").toUtf8();
        payload += QString("\"IsCheckBoxSelection\":false,\"IsProportionalSize\":false,\"ShowResultCount\":false,\"IsDropDown\":true,\"Width\":0,\"Height\":0,\"ValueFormat\":null,\"DropDownWaterMark\":\"Année\",\"DropDownWaterMarkResourceKey\":null,").toUtf8();
        payload += QString("\"IsVerticalAlignement\":true,\"HideIfNoResult\":false,\"CustomText\":null,\"CustomTextResourceKey\":null,\"OrderedCustomFieldValueGuids\":[\"31437352-d792-eacd-9912-924856406ec0\",\"f0a83c9b-729a-09c5-4d16-030d62440de6\",").toUtf8();
        payload += QString("\"c2f91fdf-e176-2d78-9797-7964aaac7f4d\",\"c7cf8744-ee30-9dd6-977b-e44663c09b4d\",\"e48746bb-c539-67f3-52ac-aa81a02c09d0\",\"4371e341-7e7b-395f-c39e-51ac55b981f6\",\"3349231f-9f6e-9eed-43b2-ee35eb6504ee\",").toUtf8();
        payload += QString("\"0deb842b-ca78-5c83-3769-34e4df052660\",\"30f1f192-3850-2d31-45d1-4feebf975635\",\"a7e36124-210e-aafe-1091-2118116ac8f3\"],\"Range\":null,\"IsDescOrder\":false,\"IsSortOrder\":false,").toUtf8();
        payload += QString("\"ParentTagGuid\":\"00000000-0000-0000-0000-000000000000\",\"ClassificationGuid\":\"00000000-0000-0000-0000-000000000000\",\"PropertyName\":0,\"IdentifierGuid\":\"fa298514-fc42-ea42-8789-637d27dc7497\"},").toUtf8();
        payload += QString("{\"IdentifierGuidType\":3,\"IsExpandableSection\":false,\"IsExpandedByDefault\":true,\"IsMultipleSelection\":false,\"IsMutuallyExclusive\":true,\"IsResetSelection\":false,\"IsRequired\":false,").toUtf8();
        payload += QString("\"IsCheckBoxSelection\":false,\"IsProportionalSize\":false,\"ShowResultCount\":false,\"IsDropDown\":true,\"Width\":0,\"Height\":0,\"ValueFormat\":null,\"DropDownWaterMark\":\"Mois\",\"DropDownWaterMarkResourceKey\":null,").toUtf8();
        payload += QString("\"IsVerticalAlignement\":true,\"HideIfNoResult\":false,\"CustomText\":null,\"CustomTextResourceKey\":null,\"OrderedCustomFieldValueGuids\":[\"fb511438-8d6b-f213-b5b2-9145e11bdebb\",").toUtf8();
        payload += QString("\"ac45173c-0771-c015-0422-a1e80749f19e\",\"fb2c0d9f-5524-268c-8e3f-ce0998512ad3\",\"03dc9ae0-ec02-f1b4-5d8e-8d8f2a4ef15d\",\"d32918e4-651a-95c5-4582-473163c97638\",\"fabab403-8642-9635-8ddb-2dc112a72439\",").toUtf8();
        payload += QString("\"cdc91ac9-3173-0171-0d40-fe7d51b6e3ed\",\"3ebc7d1a-1a52-781a-af8f-bee95eb68a4b\",\"8dc1c5bd-29b6-d751-8488-01d0d585415b\",\"a36e3575-53fd-ccb7-7470-812b3e9cf61d\",\"9daf4406-01f8-aa92-ab7f-56807c956728\",").toUtf8();
        payload += QString("\"e3b6dddc-d45f-7253-1896-4700983ac8ca\"],\"Range\":null,\"IsDescOrder\":false,\"IsSortOrder\":false,\"ParentTagGuid\":\"00000000-0000-0000-0000-000000000000\",").toUtf8();
        payload += QString("\"ClassificationGuid\":\"00000000-0000-0000-0000-000000000000\",\"PropertyName\":0,\"IdentifierGuid\":\"0929ca2f-f855-3d41-77f4-754975621ace\"}],\"Tags\":[\"2a61180d-37c8-2cf9-4505-4f4596ae6de2\"],").toUtf8();
        payload += QString("\"CatalogGuid\":\"00000000-0000-0000-0000-000000000000\",\"RedirectionZoneHierarchyId\":0,\"GroupByFamily\":false,\"GroupingCustomFieldGuids\":[],\"GetOnlyProductsWithUrl\":false,\"IsRedirectOnSelect\":true,").toUtf8();
        payload += QString("\"HasGroupedQueryZone\":false,\"DateFilter\":0},\"CustomFieldGuids\":[\"92e07d8c-a292-4f21-1c9d-215bec4dbcb8\",\"d967506e-7ba5-26e4-6c40-1dd6b2c466b3\",\"4414d72d-0b7a-a13e-d92a-166cec1040b5\",").toUtf8();
        payload += QString("\"17d538cf-a248-d961-d652-4a1db6d995a4\",\"407b7048-899b-2a2e-e230-7ae2d781f3f9\",\"e165c184-157a-1bae-454b-208ff2d84047\",\"e1e5eebc-a3dc-9c0f-845f-6dadc41c353e\",").toUtf8();
        payload += QString("\"f2fec77b-53a8-484f-1526-0b7638c241a8\",\"a291054d-d746-7fcd-9d5f-99b7ea74304b\"],\"CultureId\":\"fr-CA\",\"CurrencyId\":1,\"CurrencyIsoCode\":\"CAD\",\"Facets\":[],\"Search\":\"\",").toUtf8();
        payload += QString("\"OrderBy\":{\"PropertyName\":0,\"IdentifierGuid\":\"17d538cf-a248-d961-d652-4a1db6d995a4\"},\"IsDescOrder\":true,\"Index\":0,\"Count\":100}}").toUtf8();

        URLparams.numParams = 0;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 2400;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");
        downloadRequest.instructions.payload = payload;
        downloadRequest.instructions.Accept = QString("application/json, text/javascript, */*; q=0.01");
        downloadRequest.instructions.ContentTypeHeader = QString("application/json; charset=UTF-8");
        downloadRequest.instructions.X_Requested_With = QString("XMLHttpRequest");
        downloadRequest.instructions.Origin = baseURL.getString();
        downloadRequest.instructions.Sec_Fetch_Mode = QString("cors");
        downloadRequest.instructions.Sec_Fetch_Site = QString("same-origin");
        downloadRequest.instructions.Sec_Fetch_Dest = QString("empty");
        //downloadRequest.instructions.Accept_Encoding = QString("br");
        downloadRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");
        downloadRequest.instructions.Referer = URLbase.getString() + QString("/avis-deces");
    }
        break; // 1112 NMedia

    case 1113:  // Webs
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-d-c-s-obituary");
        //             https://www.jaguilbault.com/avis-de-d-c-s-obituary

        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.Accept = QString("text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
        downloadRequest.instructions.Sec_Fetch_Mode = QString("navigate");
        downloadRequest.instructions.Sec_Fetch_Site = QString("same-origin");
        downloadRequest.instructions.Sec_Fetch_Dest = QString("document");
        //downloadRequest.instructions.Accept_Encoding = QString("br");
        downloadRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");
        downloadRequest.instructions.Referer = baseURL.getString() + QString("/");
    }
        break; // 1113 Webs

    case 1114:  // Descary
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/en/category/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 73;
        else
            flowParameters.endingPosition = 7;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1114 Descary

    case 1115:  // Tonik
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/fr/deces/avis_deces?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 50;
        else
            flowParameters.endingPosition = 5;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1115 Tonik

    case 1116:  // Kaleidos
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis_deces.html?pN_4979=") + paramPlaceholder;
        URLaddressTemplate += PQString("&ajax=listeAvis4979");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 5;
        else
            flowParameters.endingPosition = 0;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1116 Kaleidos

    case 1117:  // Gemini
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        pageVariables.usePubDateForCutOff = true;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/category/avis-de-deces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 13;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1117 Gemini

    case 1118:  // Alias
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/") + paramPlaceholder + PQString("#avis");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 22;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1118 Alias

    case 1119:  // Cible
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/wp-admin/admin-ajax.php?action=ajax-GetListeAvisDeces&lang=fr");

        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1119 Cible

    case 1121:  // District4Web
    {
        if (providerKey == 2)
        {
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup)
                flowParameters.endingPosition = 4;
            else
                flowParameters.endingPosition = 1;
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/avis-de-deces/");

            URLparams.numParams = 0;
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1121 District4Web

    case 1122:  // Cake
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avisdeces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 25;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1122 Cake

    case 1123:  // J27
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 20;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/en/obituaries.html?option=com_ajax&module=minifrontpagepro&format=raw&start=") + paramPlaceholder + PQString("&Itemid=432&module_title=AVIS%20FULL");
        //URLaddressTemplate  = URLbase + PQString("/index.php/en/obituaries?start=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 60;
        else
            flowParameters.endingPosition = 0;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1123 J27

    case 1124:  // NetRevolution
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/?jet-smart-filters=jet-engine/avis&jet_paged=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 27;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1124 NetRevolution

    case 1125:  // ImageXpert
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/accueil?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 51;
        else
            flowParameters.endingPosition = 5;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1125 ImageXpert

    case 1126:  // Reactif
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/?pagination=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 5;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1126 Reactif

    case 1127:  // Boite
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avisdedeces/?f=obits&pagenum=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 6;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1127 Boite

    case 1128:  // Orage
    {
        flowParameters.initialSetup = false;

        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 61;
            flowParameters.flowType = alphabetical;

            URLbase = baseURL + PQString("/fr/");
            URLaddressTemplate  = URLbase + PQString("jardin.php?char=") +  paramPlaceholder;
            URLaddressTemplate += PQString("&ob=datefin_nouvelle&od=DESC");

            URLparams.numParams = 1;
            URLparams.param1Type = ptQS;
            URLparams.QSparam1 = &flowParameters.currentLetter;

            flowParameters.letterCurrent = 65;
            flowParameters.letterEnd = 90;
            flowParameters.currentLetter = QString(QChar(flowParameters.letterCurrent));
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/fr/avis.php");

            URLparams.numParams = 0;
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1128 Orage

    case 1129:  // Kerozen
    {
        if (providerKey == 2)
        {
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/liste/deces/");

            URLparams.numParams = 0;
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/avis/page/") + paramPlaceholder + PQString("/");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup == true)
                flowParameters.endingPosition = 24;
            else
                flowParameters.endingPosition = 2;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1129 Kerozen

    case 1130:  // Inovision
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 5;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1130 Inovision

    case 1131:  // FRM
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        if (providerKey <= 100)
        {
            URLbase = baseURL;
            switch(providerKey)
            {
            case 3:
                URLaddressTemplate  = URLbase + PQString("/category/obituaries/page/") + paramPlaceholder + PQString("/");
                break;

            case 10:
                URLaddressTemplate  = URLbase + PQString("/avis-de-deces/page/") + paramPlaceholder + PQString("/");
                break;

            case 14:
                URLaddressTemplate  = URLbase + PQString("/obituary/page/") + paramPlaceholder + PQString("/");
                break;

            default:
                URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");
                break;
            }

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup == true)
                flowParameters.endingPosition = fhParam2.toUInt();
            else
                flowParameters.endingPosition = 3;
        }
        else
        {
            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/page/") + paramPlaceholder + PQString("/?s&cat=3&post_type=post");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup == true)
                flowParameters.endingPosition = fhParam2.toUInt();
            else
                flowParameters.endingPosition = 2;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1131 FRM

    case 1132:  // Passage Coop
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;
        pageVariables.usePubDateForCutOff = true;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/en/recent-obituaries/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1132 Passage Coop

    case 1134:  // JBCote
    {
        if (providerKey == 3)
        {
            // Strange setup - need for second parameter driven run to be determined after first download

            daysOfOverlap = 0;
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/cgi-bin/coop-avis.cgi");

            URLparams.numParams = 0;
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 2;
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.initialSetup = false;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/en/death-notice?page=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup == true)
                flowParameters.endingPosition = fhParam1.toUInt();
            else
                flowParameters.endingPosition = 2;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        if (flowParameters.initialSetup)
            pageVariables.cutOffID = QString("3858");
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1134 JBCote

    case 1135:  // Blackcreek
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;

        URLbase = baseURL;

        if (flowParameters.initialSetup)
        {
            flowParameters.flowType = startToEnd;

            URLaddressTemplate  = URLbase + PQString("/past-services/page/") + paramPlaceholder + PQString("/");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 52;
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLaddressTemplate  = URLbase + PQString("/current-services/");

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1135 Blackcreek

    case 1136:  // CityMax
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries.html");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1136 CityMac

    case 1137:  // SYGIF
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/views/ajax?view_name=avis_de_d_c_s&view_display_id=page&view_args=&view_path=avis-de-deces&view_base_path=avis-de-deces&view_dom_id=2a1c5a35af1c3424c1fd08e1ae42bb1a");
        URLaddressTemplate += PQString("&pager_element=0&page=") + paramPlaceholder;
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=facebook-jssdk&ajax_html_ids%5B%5D=twitter-wjs&ajax_html_ids%5B%5D=omega-media-query-dummy&ajax_html_ids%5B%5D=skip-link&ajax_html_ids%5B%5D=page");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=section-header&ajax_html_ids%5B%5D=zone-header-wrapper&ajax_html_ids%5B%5D=zone-header&ajax_html_ids%5B%5D=region-logo&ajax_html_ids%5B%5D=block-block-1");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=region-menup&ajax_html_ids%5B%5D=block-system-main-menu&ajax_html_ids%5B%5D=zone-slideshow-wrapper&ajax_html_ids%5B%5D=zone-slideshow");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=region-slideshow&ajax_html_ids%5B%5D=block-block-8&ajax_html_ids%5B%5D=zone-fastlink3-wrapper&ajax_html_ids%5B%5D=zone-fastlink3");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=region-fastlink-1&ajax_html_ids%5B%5D=section-content&ajax_html_ids%5B%5D=zone-content-wrapper&ajax_html_ids%5B%5D=zone-content");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=region-content&ajax_html_ids%5B%5D=main-content&ajax_html_ids%5B%5D=page-title&ajax_html_ids%5B%5D=block-views-exp-avis-de-d-c-s-page");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=views-exposed-form-avis-de-d-c-s-page&ajax_html_ids%5B%5D=edit-title-wrapper&ajax_html_ids%5B%5D=edit-title&ajax_html_ids%5B%5D=edit-submit-avis-de-d-c-s");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=block-system-main&ajax_html_ids%5B%5D=region-aside&ajax_html_ids%5B%5D=block-text-resize-0&ajax_html_ids%5B%5D=text_resize_decrease");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=text_resize_reset&ajax_html_ids%5B%5D=text_resize_increase&ajax_html_ids%5B%5D=text_resize_clear&ajax_html_ids%5B%5D=block-views-liens-utiles-block-1");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=block-views-liens-utiles-block&ajax_html_ids%5B%5D=block-views-colonne-de-droite-block&ajax_html_ids%5B%5D=views_slideshow_cycle_main_colonne_de_droite-block");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=views_slideshow_cycle_teaser_section_colonne_de_droite-block&ajax_html_ids%5B%5D=views_slideshow_cycle_div_colonne_de_droite-block_0");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=views_slideshow_cycle_div_colonne_de_droite-block_1&ajax_html_ids%5B%5D=views_slideshow_cycle_div_colonne_de_droite-block_2");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=widget_pager_bottom_colonne_de_droite-block&ajax_html_ids%5B%5D=views_slideshow_pager_field_item_bottom_colonne_de_droite-block_0");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=views_slideshow_pager_field_item_bottom_colonne_de_droite-block_1&ajax_html_ids%5B%5D=views_slideshow_pager_field_item_bottom_colonne_de_droite-block_2");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=twitter-widget-0&ajax_html_ids%5B%5D=section-footer&ajax_html_ids%5B%5D=zone-footer-wrapper&ajax_html_ids%5B%5D=zone-footer&ajax_html_ids%5B%5D=region-video");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=block--block--9&ajax_html_ids%5B%5D=region-menufooter-1&ajax_html_ids%5B%5D=block-footer-sitemap-footer-sitemap&ajax_html_ids%5B%5D=footer-sitemap");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=region-menufooter-2&ajax_html_ids%5B%5D=block-menu-menu-menu-secondaire&ajax_html_ids%5B%5D=region-federation&ajax_html_ids%5B%5D=block--block--6");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=region-copyright&ajax_html_ids%5B%5D=block-block-7&ajax_html_ids%5B%5D=fb-root&ajax_html_ids%5B%5D=cboxOverlay&ajax_html_ids%5B%5D=colorbox");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=cboxWrapper&ajax_html_ids%5B%5D=cboxTopLeft&ajax_html_ids%5B%5D=cboxTopCenter&ajax_html_ids%5B%5D=cboxTopRight&ajax_html_ids%5B%5D=cboxMiddleLeft");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=cboxContent&ajax_html_ids%5B%5D=cboxTitle&ajax_html_ids%5B%5D=cboxCurrent&ajax_html_ids%5B%5D=cboxPrevious&ajax_html_ids%5B%5D=cboxNext");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=cboxSlideshow&ajax_html_ids%5B%5D=cboxLoadingOverlay&ajax_html_ids%5B%5D=cboxLoadingGraphic&ajax_html_ids%5B%5D=cboxMiddleRight&ajax_html_ids%5B%5D=cboxBottomLeft");
        URLaddressTemplate += PQString("&ajax_html_ids%5B%5D=cboxBottomCenter&ajax_html_ids%5B%5D=cboxBottomRight&ajax_html_ids%5B%5D=rufous-sandbox&ajax_page_state%5Btheme%5D=cfbsl");
        URLaddressTemplate += PQString("&ajax_page_state%5Btheme_token%5D=Hjx8NM4WudgIiWsa_g6h3HME2aFgSjbyuXKUCCdLeLM&ajax_page_state%5Bcss%5D%5Bmodules%2Fsystem%2Fsystem.base.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bmodules%2Fsystem%2Fsystem.menus.css%5D=1&ajax_page_state%5Bcss%5D%5Bmodules%2Fsystem%2Fsystem.messages.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bmodules%2Fsystem%2Fsystem.theme.css%5D=1&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fmodules%2Fviews_slideshow%2Fviews_slideshow.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bmodules%2Fcomment%2Fcomment.css%5D=1&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fmodules%2Fdate%2Fdate_api%2Fdate.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fmodules%2Fdate%2Fdate_popup%2Fthemes%2Fdatepicker.1.7.css%5D=1&ajax_page_state%5Bcss%5D%5Bmodules%2Ffield%2Ftheme%2Ffield.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fmodules%2Ffooter_sitemap%2Ffooter_sitemap.css%5D=1&ajax_page_state%5Bcss%5D%5Bmodules%2Fnode%2Fnode.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bmodules%2Fuser%2Fuser.css%5D=1&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fmodules%2Fviews%2Fcss%2Fviews.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fmodules%2Fckeditor%2Fcss%2Fckeditor.css%5D=1&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fmodules%2Fcolorbox%2Fstyles%2Fdefault%2Fcolorbox_style.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fmodules%2Fctools%2Fcss%2Fctools.css%5D=1&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fmodules%2Ftext_resize%2Ftext_resize.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fmodules%2Fviews_slideshow%2Fcontrib%2Fviews_slideshow_cycle%2Fviews_slideshow_cycle.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fthemes%2Fomega%2Falpha%2Fcss%2Falpha-reset.css%5D=1&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fthemes%2Fomega%2Falpha%2Fcss%2Falpha-mobile.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fthemes%2Fomega%2Falpha%2Fcss%2Falpha-alpha.css%5D=1&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fthemes%2Fomega%2Fomega%2Fcss%2Fformalize.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fthemes%2Fomega%2Fomega%2Fcss%2Fomega-text.css%5D=1&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fthemes%2Fomega%2Fomega%2Fcss%2Fomega-branding.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fthemes%2Fomega%2Fomega%2Fcss%2Fomega-menu.css%5D=1&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fthemes%2Fomega%2Fomega%2Fcss%2Fomega-forms.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fthemes%2Fomega%2Fomega%2Fcss%2Fomega-visuals.css%5D=1&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fthemes%2Fcfbsl%2Fcss%2Fglobal.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bie%3A%3Anormal%3A%3Asites%2Fall%2Fthemes%2Fcfbsl%2Fcss%2Fcfbsl-alpha-default.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bie%3A%3Anormal%3A%3Asites%2Fall%2Fthemes%2Fcfbsl%2Fcss%2Fcfbsl-alpha-default-normal.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bie%3A%3Anormal%3A%3Asites%2Fall%2Fthemes%2Fomega%2Falpha%2Fcss%2Fgrid%2Falpha_default%2Fnormal%2Falpha-default-normal-12.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bnarrow%3A%3Asites%2Fall%2Fthemes%2Fcfbsl%2Fcss%2Fcfbsl-alpha-default.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bnarrow%3A%3Asites%2Fall%2Fthemes%2Fcfbsl%2Fcss%2Fcfbsl-alpha-default-narrow.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fthemes%2Fomega%2Falpha%2Fcss%2Fgrid%2Falpha_default%2Fnarrow%2Falpha-default-narrow-12.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bnormal%3A%3Asites%2Fall%2Fthemes%2Fcfbsl%2Fcss%2Fcfbsl-alpha-default.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bnormal%3A%3Asites%2Fall%2Fthemes%2Fcfbsl%2Fcss%2Fcfbsl-alpha-default-normal.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bcss%5D%5Bsites%2Fall%2Fthemes%2Fomega%2Falpha%2Fcss%2Fgrid%2Falpha_default%2Fnormal%2Falpha-default-normal-12.css%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bjs%5D%5B0%5D=1&ajax_page_state%5Bjs%5D%5B1%5D=1&ajax_page_state%5Bjs%5D%5Bmisc%2Fjquery.js%5D=1&ajax_page_state%5Bjs%5D%5Bmisc%2Fjquery.once.js%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bjs%5D%5Bmisc%2Fdrupal.js%5D=1&ajax_page_state%5Bjs%5D%5Bmisc%2Fjquery.cookie.js%5D=1&ajax_page_state%5Bjs%5D%5Bmisc%2Fjquery.form.js%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Fmodules%2Fviews_slideshow%2Fjs%2Fviews_slideshow.js%5D=1&ajax_page_state%5Bjs%5D%5Bmisc%2Fajax.js%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Fmodules%2Fboost_captcha%2Fboost_captcha.js%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bjs%5D%5Bpublic%3A%2F%2Flanguages%2Ffr_tcO7SRNFvjlhT3oPoSJxyjgto-ks3S4_PnO5d22yyT4.js%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Flibraries%2Fcolorbox%2Fjquery.colorbox-min.js%5D=1&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Fmodules%2Fcolorbox%2Fjs%2Fcolorbox.js%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Fmodules%2Fcolorbox%2Fstyles%2Fdefault%2Fcolorbox_style.js%5D=1&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Fmodules%2Fviews%2Fjs%2Fbase.js%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bjs%5D%5Bmisc%2Fprogress.js%5D=1&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Fmodules%2Fviews_load_more%2Fviews_load_more.js%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Fmodules%2Fviews%2Fjs%2Fajax_view.js%5D=1&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Fmodules%2Ftext_resize%2Ftext_resize.js%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Flibraries%2Fjquery.cycle%2Fjquery.cycle.all.js%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Fmodules%2Fviews_slideshow%2Fcontrib%2Fviews_slideshow_cycle%2Fjs%2Fviews_slideshow_cycle.js%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Fmodules%2Fgoogle_analytics%2Fgoogleanalytics.js%5D=1");
        URLaddressTemplate += PQString("&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Fthemes%2Fomega%2Fomega%2Fjs%2Fjquery.formalize.js%5D=1&ajax_page_state%5Bjs%5D%5Bsites%2Fall%2Fthemes%2Fomega%2Fomega%2Fjs%2Fomega-mediaqueries.js%5D=1");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 25;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");

    }
        break; // 1137 SYGIF

    case 1138:  // PortNeuf
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 35;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1138 PortNeuf

    case 1139:  // Burke
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/runtime/69/default.php?branchId=all&page=") + paramPlaceholder;
        URLaddressTemplate += PQString("&type=all&sort=DateOfDeath&sortdir=DESC&sortLabel=Date+of+Death&start=&end=&datetype=ServiceDate&tpl=colchester&SiteId=&NavigatorId=&nowrap=1&widget=Services&search=&listclass=home&rpp=18");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 22;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1139 Burke

    case 1140:  // Canadian
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 26;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/") + fhParam1 + PQString(".html?start=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam2.toInt() * flowParameters.flowIncrement;
        else
            flowParameters.endingPosition = static_cast<int>(fhParam2.toInt() / 10) * flowParameters.flowIncrement;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1140 Canadian

    case 1141:  // Ballamedia
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 70;

        // https://baygardens.permavita.com/api/list/EFJOd2h19vDRFFsa97?searchText=&sort=dod&dir=desc&limit=10&start=100
        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/api/list/EFJOd2h19vDRFFsa97?searchText=&sort=dod&dir=desc&limit=70&start=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 700;
        else
            flowParameters.endingPosition = 0;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.Accept = QString("text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
        downloadRequest.instructions.Sec_Fetch_Mode = QString("navigate");
        downloadRequest.instructions.Sec_Fetch_Site = QString("none");
        downloadRequest.instructions.Sec_Fetch_Dest = QString("document");
        downloadRequest.instructions.Accept_Encoding = QString("br");
        downloadRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");

    }
        break; // 1141 Ballamedia

    case 1142:  // Jac
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/index.php");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1142 Jac

    case 1143:  // Ministry
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        if (flowParameters.initialSetup)
            URLaddressTemplate  = URLbase + PQString("/archive-notices.php");
        else
            URLaddressTemplate  = URLbase + PQString("/deathnoticesc5.php");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1143 Ministry

    case 1144:  // Multinet
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL + PQString("/cgi-bin/");
        URLaddressTemplate  = URLbase + PQString("rfcc.cgi");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1144 Multinet

    case 1145:  // PropulC
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/?-page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 15;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1145 PropulC

    case 1146:  // Nexion
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        if (flowParameters.initialSetup)
            URLaddressTemplate  = URLbase + PQString("/avis-deces/");
        else
            URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1146 Nexion

    case 1147:  // LCProduction
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis/?f=obits&pagenum=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 13;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1147 LCProduction

    case 1148:  // Absolu
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowIncrement = 10;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces?start=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 550;
        else
            flowParameters.endingPosition = 50;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1148 Absolu

    case 1149:  // Suite B
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        if (flowParameters.initialSetup)
            URLaddressTemplate  = URLbase + PQString("/defunt/?find=all");
        else
            URLaddressTemplate  = URLbase + PQString("/defunt/?find=all");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1149 Suite B

    case 1150:  // Map
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces?pg=") + paramPlaceholder + PQString("&alpha=&lieu=0&by=dt");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 47;
        else
            flowParameters.endingPosition = 4;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1150 Map

    case 1151:  // iClic
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;
        pageVariables.usePubDateForCutOff = true;

        URLbase = baseURL;
        //URLaddressTemplate  = URLbase + PQString("/fr/avis-de-deces?p=") + paramPlaceholder;
        URLaddressTemplate  = URLbase + PQString("/_api/cloud-data/v1/wix-data/collections/query");
        URLaddressTemplate += PQString("?{\"collectionName\":\"Blog/Posts\",\"dataQuery\":{\"filter\":{},\"sort\":[{\"fieldName\":\"publishedDate\",\"order\":\"DESC\"}],\"paging\":{\"offset\":0,\"limit\":200}},");
        URLaddressTemplate += PQString("\"options\":{},\"includeReferencedItems\":[],\"segment\":\"LIVE\",\"appId\":\"5a3a8fca-371a-4c20-9ea0-16b976f5732d\"}");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 1;
        else
            flowParameters.endingPosition = 1;

        downloadRequest.instructions.verb = QString("POST");
        downloadRequest.instructions.Accept = QString("application/json, text/plain, */*");
        downloadRequest.instructions.ContentTypeHeader = QString("application/json");
        downloadRequest.instructions.Referer = QString("https://www.maisonfuneraireroussin.com/_partials/wix-thunderbolt/dist/clientWorker.709d549b.bundle.min.js");
        downloadRequest.instructions.Authorization  = QString("wixcode-pub.b4caa853ce7efd748e53715837b639eda8052cca.eyJpbnN0YW5jZUlkIjoiM2JhOTU2YzQtMDk4My00NTdhLWJlOWYtZGQxMzNlOWQ4YjVlIiwiaHRtbFNpdGVJZCI6Ij");
        downloadRequest.instructions.Authorization += QString("NmNDBmOTRjLTBjZWUtNGI5MC1iYTQ3LWZiNDQ0M2RlZTI1MiIsInVpZCI6bnVsbCwicGVybWlzc2lvbnMiOm51bGwsImlzVGVtcGxhdGUiOmZhbHNlLCJzaWduRGF0ZSI6MTY3NTMwODYyN");
        downloadRequest.instructions.Authorization += QString("DQ1NiwiYWlkIjoiNWRlNmE5YWEtNDBjMS00YWY2LWE2M2EtNWQzZTA1ZTZiYzI5IiwiYXBwRGVmSWQiOiJDbG91ZFNpdGVFeHRlbnNpb24iLCJpc0FkbWluIjpmYWxzZSwibWV0YVNpdGVJ");
        downloadRequest.instructions.Authorization += QString("ZCI6IjUzNjRhZGM5LWRjMTMtNDM5ZS1iZTAxLTBkZjFlMWE4NWU5YSIsImNhY2hlIjpudWxsLCJleHBpcmF0aW9uRGF0ZSI6bnVsbCwicHJlbWl1bUFzc2V0cyI6IlNob3dXaXhXaGlsZUx");
        downloadRequest.instructions.Authorization += QString("vYWRpbmcsQWRzRnJlZSxIYXNEb21haW4iLCJ0ZW5hbnQiOm51bGwsInNpdGVPd25lcklkIjoiZDRkNzRhN2ItODY4Ny00NGI3LWI0MGMtZDJlNWVlMDg5ZTM0IiwiaW5zdGFuY2VUeXBlIj");
        downloadRequest.instructions.Authorization += QString("oicHViIiwic2l0ZU1lbWJlcklkIjpudWxsLCJwZXJtaXNzaW9uU2NvcGUiOm51bGx9");

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1151 iClic

    case 1154:  // Bouille
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 10;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1154 Bouille

    case 1155:  // Techlogical
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 30;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1155 Techlogical

    case 1156:  // GyOrgy
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1156 GyOrgy

    case 1157:  // GemWebb
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/category/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 22;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1157 GemWebb

    case 1158:  // RedChair
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1158 RedChair

    case 1159:  // ExtremeSurf
    {
        flowParameters.initialSetup = false;

        URLbase = baseURL;

        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 691;
            flowParameters.flowType = startToEnd;

            URLaddressTemplate  = URLbase + PQString("/past-services/?page_id_all=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 4;
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLaddressTemplate  = URLbase + PQString("/funeral-notices/");

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1159 ExtremeSurf

    case 1160:  // Cahoots
    {
        flowParameters.initialSetup = false;

        URLbase = baseURL;

        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 691;
            flowParameters.flowType = startToEnd;

            URLaddressTemplate  = URLbase + PQString("/blogs/obituaries?page=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 12;
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLaddressTemplate  = URLbase;

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1160 Cahoots

    case 1161:  // Tride
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 39;
        else
            flowParameters.endingPosition = 4;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1161 Tride

    case 1162:  // Jensii
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/scheduled-services.htm");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1162 Jensii

    case 1163:  // InterWeb
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/?f=obits&pagenum=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 25;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1163 InterWeb

    case 1164:  // Brown
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis/?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 8;
        else
            flowParameters.endingPosition = 5;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1164 Brown

    case 1165:  // Tukio
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        switch (providerKey)
        {
        case 1:
            //https://manage.tukioswebsites.com/obitapi/ajax_obits/305/obit_page_obits/standard?sq=&page=4&screen=&filter=
            //https://manage.tukioswebsites.com/obitapi/ajax_obits/553/obit_page_obits/modern?sq=&page=2&screen=&filter=
            // https://manage2.tukioswebsites.com305&page=2&per_page=10
            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/api/legacy/obituaries?fhid=") + fhParam1 + PQString("&page=") + paramPlaceholder + PQString("&per_page=10");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup == true)
                flowParameters.endingPosition = 4;
            else
                flowParameters.endingPosition = 2;

            break;

        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            // https://manage2.tukioswebsites.com/api/v1/obituaries?siteAlias=dc4dc470&page=2&per_page=10
            // https://manage2.tukioswebsites.com/api/v1/obituaries?siteAlias=a804057b&page=2&q=&per_page=10&include_services=0&veterans_only=0
            // manage2.tukioswebsites.com/api/v1/obituaries?siteAlias=00aceaf8&page=4&per_page=10
            URLbase = baseURL;
            if ((providerKey == 6) || (providerKey == 7) || (providerKey == 8))
                URLaddressTemplate  = URLbase + PQString("/api/v1/obituaries?siteAlias=") + fhParam2 + PQString("&page=") + paramPlaceholder + PQString("&per_page=10");
            else
                URLaddressTemplate  = URLbase + PQString("/api/v1/obituaries?siteAlias=") + fhParam2 + PQString("&page=") + paramPlaceholder + PQString("&q=&per_page=10&include_services=0&veterans_only=0");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            if (flowParameters.initialSetup == true)
                flowParameters.endingPosition = 5;
            else
                flowParameters.endingPosition = 2;

            break;
        }

        // Unique situation where a very specific preflight OPTION request is required
        preflightRequest.instructions.verb = QString("OPTIONS");
        preflightRequest.instructions.Accept = QString("ALL");
        preflightRequest.instructions.Access_Control_Request_Method = QString("GET");
        preflightRequest.instructions.Access_Control_Request_Headers = QString("authorization");
        preflightRequest.instructions.Origin = baseURL.getString();
        preflightRequest.instructions.User_Agent = QString("STD");
        preflightRequest.instructions.Sec_Fetch_Mode = QString("cors");
        preflightRequest.instructions.Sec_Fetch_Site = QString("cross-site");
        preflightRequest.instructions.Sec_Fetch_Dest = QString("empty");
        preflightRequest.instructions.Referer = baseURL.getString() + QString("/");
        preflightRequest.instructions.Accept_Encoding = QString("br");
        preflightRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");

        // Preflight Request
        downloadRequest.instructions.Authorization = QString("Bearer k9PMgGzdKda2PGocioyUBzAtVwFj7FsKZlpxORi6");
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
        preflightRequest.instructions.url = downloadRequest.instructions.url;
        www->download(preflightRequest);
        while(www->processingDownload()){};

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1165 Tukio

    case 1166:      // webCemeteries
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/upcoming-burials/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1166 webCemeteries

    case 1167:      // Etincelle
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/") + fhParam1 + PQString("/avis-de-deces");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 1167 Etincelle

    case 1168:      // ObitAssistant
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        //https://www.obituary-assistant.com/api/rest/obituaries?page=2&_=1674929206917

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/api/rest/obituaries?page=") + paramPlaceholder + PQString("&_=") + fhParam1;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam2.toInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        // Unique situation where a very specific preflight OPTION request is required
        preflightRequest.instructions.verb = QString("OPTIONS");
        preflightRequest.instructions.Accept = QString("ALL");
        preflightRequest.instructions.Access_Control_Request_Method = QString("GET");
        preflightRequest.instructions.Access_Control_Request_Headers = QString("authorization");
        preflightRequest.instructions.Origin = baseURL.getString();
        preflightRequest.instructions.User_Agent = QString("STD");
        preflightRequest.instructions.Sec_Fetch_Mode = QString("cors");
        preflightRequest.instructions.Sec_Fetch_Site = QString("cross-site");
        preflightRequest.instructions.Sec_Fetch_Dest = QString("empty");
        preflightRequest.instructions.Referer = baseURL.getString() + QString("/");
        //preflightRequest.instructions.Accept_Encoding = QString("br");
        preflightRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");

        downloadRequest.instructions.Accept = QString("application/json;charset=utf-8");
        downloadRequest.instructions.Authorization = QString("Basic d2lsbGlhbXM6SnZEb0lRcUNaWA==");

        // Preflight Request
        preflightRequest.instructions.url = downloadRequest.instructions.url;
        www->download(preflightRequest);
        while(www->processingDownload()){};
    }
        break; // 1168 ObitAssistant

    case 2000:  // Mike Purdy
    {
        // Historical - Loop through pages of annual obits
        // Active - Last x years
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = -1;

        URLbase = baseURL + QString("/obituaries-");

        URLaddressTemplate  = URLbase;
        URLaddressTemplate += paramPlaceholder + QString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        if (flowParameters.initialSetup)
        {
            flowParameters.currentPosition = 2016;
            flowParameters.endingPosition = fhParam2.toUInt();
        }
        else
        {
            flowParameters.currentPosition = 2016;
            flowParameters.endingPosition = fhParam2.toUInt();
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 2001:  // Bow River
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = -1;

        URLbase = baseURL + QString("/index.php?f=obits&c_orderby=date&c_alpha=&c_page=");
        URLaddressTemplate  = URLbase;
        URLaddressTemplate += paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        if (flowParameters.initialSetup)
        {
            flowParameters.currentPosition = fhParam2.toUInt();
            flowParameters.endingPosition = 1;
        }
        else
        {
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);

        // Download once to get last page number
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
        www->download(downloadRequest);
        while(www->processingDownload()){};
        if(www->lastDownloadSuccessful())
        {
            sourceFile.setSourceFile(downloadRequest.outputs.downloadFileName);
            if (sourceFile.moveTo("table class=\"obit_idx\""))
            {
                if (sourceFile.moveBackwardTo("alpha=\""))
                {
                    word = sourceFile.readNextBetween(BRACKETS);
                    flowParameters.currentPosition = static_cast<unsigned int>(word.asNumber());

                    createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
                }
            }
        }
    }
        break;

    case 2002:  // Serenity
    {
        // Historical and active same listing - Loop page by page
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;

        URLaddressTemplate  = URLbase + QString("/obituary/page/");
        URLaddressTemplate += paramPlaceholder + QString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        if (flowParameters.initialSetup)
        {
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = fhParam2.toUInt();
        }
        else
        {
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 2;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 2003:  // McInnis
    {
        // Historical and active same listing - Loop page by page
        // Only pull URL at this point
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        if (flowParameters.initialSetup)
        {
            //baseURL.dropLeft(5);
            //URLbase = PQString("http") + baseURL;
            //baseURL.dropLeft(8);
            URLbase = baseURL;
        }
        else
            URLbase = baseURL;

        //URLaddressTemplate  = URLbase + QString("/obituaries/page/");
        //URLaddressTemplate += paramPlaceholder + QString("/");
        URLaddressTemplate  = URLbase + QString("/tribute/all-services/index.html?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        if (flowParameters.initialSetup)
        {
            flowParameters.currentPosition = 0;
            flowParameters.endingPosition = fhParam2.toUInt();
        }
        else
        {
            flowParameters.currentPosition = 0;
            flowParameters.endingPosition = 9;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 2004:  // Sturgeon
    {
        // Historical and active same listing - Loop page by page
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 5;

        URLbase = baseURL;

        URLaddressTemplate  = URLbase + QString("/index.php/en/memorials?start=");
        URLaddressTemplate += paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        if (flowParameters.initialSetup)
        {
            flowParameters.currentPosition = 0;
            flowParameters.endingPosition = fhParam2.toUInt();
        }
        else
            flowParameters.currentPosition = 0;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 2005:  // CornerStone
    {
        // Historical and active same listing - Loop page by page
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;

        URLaddressTemplate  = URLbase + QString("/obituaries/10/all/") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        if (flowParameters.initialSetup)
        {
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 35;
        }
        else
        {
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 2;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 2006:  // Pierson's
    {
        // Historical and active same listing - Loop page by page
        // Same as 1058 SRS
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;

        //URLaddressTemplate  = URLbase + QString("/obituary?sort_dir=desc&sort=date_death&page=") + paramPlaceholder;
        URLaddressTemplate  = URLbase + PQString("/ObituariesHelper/ObituariesListPagedItems?testId=obituariesListPageItemsForm?");
        URLaddressTemplate += PQString("PageSize=5&FirstPageObituariesCount=0&ObDomainId=b2521000-3ac8-4e26-a929-bfd4bb7d18a0&CurrentPage=") + paramPlaceholder;
        URLaddressTemplate += PQString("&SearchText=&SortingColumn=13&Dates=0&SelectedLocationId=&HasNextPage=True");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        if (flowParameters.initialSetup)
        {
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = fhParam2.toUInt();
        }
        else
        {
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 3;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");
        downloadRequest.instructions.X_Requested_With = QString("XMLHttpRequest");

    }
        break;

    case 2007:  // Trinity
    {
        // Historical and active same listing - Loop page by page
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;

        URLaddressTemplate  = URLbase + QString("/obituary/page/") + paramPlaceholder + QString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        if (flowParameters.initialSetup)
        {
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 60;
        }
        else
        {
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 10;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 2008:  // CelebrateLife
    {
        // Single download pulls everything
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        baseURL = URLbase;
        URLaddressTemplate = URLbase + QString("/obituaries.html");

        URLparams.numParams = 0;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2008

    case 2009:  // Funks
    {
        // Single download pulls all current obituaries
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate = URLbase + QString("/obituraries/");  // Normal processing
        //URLaddressTemplate = URLbase + QString("/obituary-archive-2020/");   // For one of the archives

        URLparams.numParams = 0;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2009

    case 2010:  // WowFactor
    {
        // Single download pulls all current obituaries in the current year
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate = URLbase + QString("/obituary/obituary-") + paramPlaceholder + QString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        flowParameters.currentPosition = globals->today.year();
        flowParameters.endingPosition = pageVariables.cutoffDate.year();

        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2010

    case 2011:  // Dalmeny
    {
        // Can loop through recent to old

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL + QString("/");
        URLaddressTemplate  = URLbase + QString("obituaries/page/") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 4;
        else
            flowParameters.endingPosition = fhParam2.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 2012:  // Hansons
    {
        // Only recent deaths available - all one page

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL + QString("/");
        URLaddressTemplate  = URLbase + QString("obituaries/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 2014:  // Martens
    {
        daysOfOverlap = 61;
        pageVariables.firstAvailableDate.setDate(fhFirstObit.year(), fhFirstObit.month(), 1);
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        if (flowParameters.initialSetup)
            pageVariables.cutoffDate = pageVariables.firstAvailableDate;
        else
            pageVariables.cutoffDate = pageVariables.latestDODdate.addDays(-daysOfOverlap);

        URLbase = baseURL ;
        URLaddressTemplate  = URLbase + QString("/obituaries/?f=obits&pagenum=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 2;
        else
            flowParameters.endingPosition = 9;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
}
        break;

    case 2015:  // Shine
    {
        // Only recent deaths available - all one page
        // Obits are temporary and get converted into PDFs over year

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;

    case 2016:  // Simply
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/category/recently-passed/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 2;
        else
            flowParameters.endingPosition = fhParam1.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2016 Simply

    case 2017:  // McCall
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/obituaries/page/") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 6;
        else
            flowParameters.endingPosition = 60;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2017 McCall

    case 2018:  // Care
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/obituaries/?f=obits&pagenum=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 1;
        else
            flowParameters.endingPosition = 10;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2018 Care

    case 2019:  // Ancient
    {
        // Only recent deaths available - all one page

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;
        pageVariables.usePubDateForCutOff = true;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // Ancient

    case 2020:  // Amherst
    {
        // Only recent deaths available - all one page

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/memorial.html");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2020 Amherst

    case 2022:  // Heritage
    {
        // Only recent deaths available - all one page

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        // First download obtains parameters
        URLbase = baseURL + PQString("/obituaries/");

        PQString param1, param2;
        initialRequest.instructions.url = URLbase;
        initialRequest.outputs.downloadFileName = QString("tempWebPage.htm");

        www->download(initialRequest);
        while(www->processingDownload()){};
        if(www->lastDownloadSuccessful())
        {
            sourceFile.setSourceFile(initialRequest.outputs.downloadFileName);
            sourceFile.setGlobalVars(*globals);
            sourceFile.beg();

            if (sourceFile.moveTo("data-vc-post-id="))
            {
                param1 = sourceFile.readNextBetween(QUOTES).getString();
                if (sourceFile.moveTo("data-vc-public-nonce="))
                    param2 = sourceFile.readNextBetween(QUOTES).getString();
            }
        }

        URLaddressTemplate  = baseURL + PQString("/wp-admin/admin-ajax.php?");
        /*URLaddressTemplate += PQString("action=vc_get_vc_grid_data&vc_action=vc_get_vc_grid_data&tag=vc_basic_grid&data%5Bvisible_pages%5D=5&data%5Bpage_id%5D=2635&data%5Bstyle%5D=pagination");
        URLaddressTemplate += PQString("&data%5Baction%5D=vc_get_vc_grid_data&data%5Bshortcode_id%5D=1635354215831-df1a1f6e-9836-10&data%5Bitems_per_page%5D=12&data%5Bauto_play%5D=false&data%5Bgap%5D=30");
        URLaddressTemplate += PQString("&data%5Bspeed%5D=-1000&data%5Bloop%5D=&data%5Banimation_in%5D=&data%5Banimation_out%5D=&data%5Barrows_design%5D=vc_arrow-icon-arrow_01_left&data%5Barrows_color%5D=blue");
        URLaddressTemplate += PQString("&data%5Barrows_position%5D=inside&data%5Bpaging_design%5D=pagination_square&data%5Bpaging_color%5D=black&data%5Btag%5D=vc_basic_grid&vc_post_id=") + param1 + PQString("&_vcnonce=") + param2;*/

        // 2635&_vcnonce=f3693976c6
        URLaddressTemplate += PQString("action=vc_get_vc_grid_data&vc_action=vc_get_vc_grid_data&tag=vc_basic_grid&data%5Bvisible_pages%5D=5&data%5Bpage_id%5D=2635&data%5Bstyle%5D=pagination&data%5Baction%5D=vc_get_vc_grid_data");
        URLaddressTemplate += PQString("&data%5Bshortcode_id%5D=1635354215831-df1a1f6e-9836-10&data%5Bitems_per_page%5D=12&data%5Bauto_play%5D=false&data%5Bgap%5D=30&data%5Bspeed%5D=-1000&data%5Bloop%5D=&data%5Banimation_in%5D=");
        URLaddressTemplate += PQString("&data%5Banimation_out%5D=&data%5Barrows_design%5D=vc_arrow-icon-arrow_01_left&data%5Barrows_color%5D=blue&data%5Barrows_position%5D=inside&data%5Bpaging_design%5D=pagination_square");
        URLaddressTemplate += PQString("&data%5Bpaging_color%5D=black&data%5Btag%5D=vc_basic_grid&vc_post_id=") + param1 + PQString("&_vcnonce=") + param2;

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");
        downloadRequest.instructions.Accept = QString("text/html, */*; q=0.01");
        downloadRequest.instructions.ContentTypeHeader = QString("application/x-www-form-urlencoded; charset=UTF-8");
        downloadRequest.instructions.X_Requested_With = QString("XMLHttpRequest");
        downloadRequest.instructions.Origin = baseURL.getString();
        downloadRequest.instructions.Sec_Fetch_Mode = QString("cors");
        downloadRequest.instructions.Sec_Fetch_Site = QString("same-origin");
        downloadRequest.instructions.Sec_Fetch_Dest = QString("empty");
        //downloadRequest.instructions.Accept_Encoding = QString("br");
        downloadRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");
        downloadRequest.instructions.Referer = URLbase.getString();
    }
        break;  // 2022 Heritage

    case 2023:  // Koru
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        downloadRequest.instructions.verb = QString("POST");

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/wp-admin/admin-ajax.php") + PQString("?");
        URLaddressTemplate += PQString("action=trx_addons_item_pagination&nonce=f71dc9cba5&params=a%3A91%3A%7Bs%3A3%3A%22cat%22%3Bs%3A3%3A%22177%22%3Bs%3A7%3A%22columns%22%3Bi%3A3%3Bs%3A14%3A%22columns_tablet%22%3Bs%3A0%3A%22%22%3Bs%3A14%3A%22");
        URLaddressTemplate += PQString("columns_mobile%22%3Bs%3A0%3A%22%22%3Bs%3A5%3A%22count%22%3Bi%3A12%3Bs%3A6%3A%22offset%22%3Bi%3A0%3Bs%3A7%3A%22orderby%22%3Bs%3A9%3A%22post_date%22%3Bs%3A5%3A%22order%22%3Bs%3A4%3A%22");
        URLaddressTemplate += PQString("desc%22%3Bs%3A3%3A%22ids%22%3Bs%3A0%3A%22%22%3Bs%3A6%3A%22slider%22%3Bb%3A0%3Bs%3A13%3A%22slider_effect%22%3Bs%3A5%3A%22slide%22%3Bs%3A17%3A%22slider_pagination%22%3Bs%3A4%3A%22none%22%3Bs%3A22%3A%22");
        URLaddressTemplate += PQString("slider_pagination_type%22%3Bs%3A7%3A%22bullets%22%3Bs%3A24%3A%22slider_pagination_thumbs%22%3Bi%3A0%3Bs%3A15%3A%22slider_controls%22%3Bs%3A4%3A%22none%22%3Bs%3A12%3A%22slides_space%22%3Bi%3A0%3Bs%3A15%3A%22");
        URLaddressTemplate += PQString("slides_centered%22%3Bs%3A0%3A%22%22%3Bs%3A15%3A%22slides_overflow%22%3Bs%3A0%3A%22%22%3Bs%3A18%3A%22slider_mouse_wheel%22%3Bs%3A0%3A%22%22%3Bs%3A15%3A%22slider_autoplay%22%3Bs%3A1%3A%221%22%3Bs%3A11%3A%22");
        URLaddressTemplate += PQString("slider_loop%22%3Bs%3A1%3A%221%22%3Bs%3A16%3A%22slider_free_mode%22%3Bs%3A0%3A%22%22%3Bs%3A5%3A%22title%22%3Bs%3A0%3A%22%22%3Bs%3A11%3A%22title_align%22%3Bs%3A4%3A%22none%22%3Bs%3A11%3A%22title_style%22%3Bs%3A7%3A%22");
        URLaddressTemplate += PQString("default%22%3Bs%3A9%3A%22title_tag%22%3Bs%3A4%3A%22none%22%3Bs%3A11%3A%22title_color%22%3Bs%3A0%3A%22%22%3Bs%3A12%3A%22title_color2%22%3Bs%3A0%3A%22%22%3Bs%3A18%3A%22gradient_direction%22%3Bs%3A1%3A%220%22%3Bs%3A18%3A%22");
        URLaddressTemplate += PQString("title_border_color%22%3Bs%3A0%3A%22%22%3Bs%3A18%3A%22title_border_width%22%3Ba%3A3%3A%7Bs%3A4%3A%22unit%22%3Bs%3A2%3A%22px%22%3Bs%3A4%3A%22size%22%3Bi%3A0%3Bs%3A5%3A%22sizes%22%3Ba%3A0%3A%7B%7D%7Ds%3A14%3A%22");
        URLaddressTemplate += PQString("title_bg_image%22%3Ba%3A3%3A%7Bs%3A3%3A%22url%22%3Bs%3A0%3A%22%22%3Bs%3A2%3A%22id%22%3Bs%3A0%3A%22%22%3Bs%3A4%3A%22size%22%3Bs%3A0%3A%22%22%3B%7Ds%3A6%3A%22title2%22%3Bs%3A0%3A%22%22%3Bs%3A12%3A%22");
        URLaddressTemplate += PQString("title2_color%22%3Bs%3A0%3A%22%22%3Bs%3A19%3A%22title2_border_color%22%3Bs%3A0%3A%22%22%3Bs%3A19%3A%22title2_border_width%22%3Ba%3A3%3A%7Bs%3A4%3A%22unit%22%3Bs%3A2%3A%22px%22%3Bs%3A4%3A%22");
        URLaddressTemplate += PQString("size%22%3Bi%3A0%3Bs%3A5%3A%22sizes%22%3Ba%3A0%3A%7B%7D%7Ds%3A15%3A%22title2_bg_image%22%3Ba%3A3%3A%7Bs%3A3%3A%22url%22%3Bs%3A0%3A%22%22%3Bs%3A2%3A%22id%22%3Bs%3A0%3A%22%22%3Bs%3A4%3A%22");
        URLaddressTemplate += PQString("size%22%3Bs%3A0%3A%22%22%3B%7Ds%3A8%3A%22subtitle%22%3Bs%3A0%3A%22%22%3Bs%3A14%3A%22subtitle_align%22%3Bs%3A4%3A%22none%22%3Bs%3A17%3A%22subtitle_position%22%3Bs%3A5%3A%22above%22%3Bs%3A11%3A%22");
        URLaddressTemplate += PQString("description%22%3Bs%3A0%3A%22%22%3Bs%3A4%3A%22link%22%3Bs%3A0%3A%22%22%3Bs%3A10%3A%22link_style%22%3Bs%3A7%3A%22default%22%3Bs%3A10%3A%22link_image%22%3Bs%3A0%3A%22%22%3Bs%3A9%3A%22link_text%22%3Bs%3A0%3A%22%22%3Bs%3A10%3A%22");
        URLaddressTemplate += PQString("new_window%22%3Bi%3A0%3Bs%3A5%3A%22typed%22%3Bs%3A0%3A%22%22%3Bs%3A13%3A%22typed_strings%22%3Bs%3A0%3A%22%22%3Bs%3A10%3A%22typed_loop%22%3Bs%3A1%3A%221%22%3Bs%3A12%3A%22typed_cursor%22%3Bs%3A1%3A%221%22%3Bs%3A17%3A%22");
        URLaddressTemplate += PQString("typed_cursor_char%22%3Bs%3A1%3A%22_%22%3Bs%3A11%3A%22typed_color%22%3Bs%3A0%3A%22%22%3Bs%3A11%3A%22typed_speed%22%3Bs%3A1%3A%226%22%3Bs%3A11%3A%22typed_delay%22%3Bs%3A1%3A%221%22%3Bs%3A2%3A%22id%22%3Bs%3A0%3A%22%22%3Bs%3A5%3A%22");
        URLaddressTemplate += PQString("class%22%3Bs%3A0%3A%22%22%3Bs%3A9%3A%22className%22%3Bs%3A0%3A%22%22%3Bs%3A3%3A%22css%22%3Bs%3A0%3A%22%22%3Bs%3A4%3A%22type%22%3Bs%3A7%3A%22classic%22%3Bs%3A8%3A%22featured%22%3Bs%3A5%3A%22image%22%3Bs%3A17%3A%22");
        URLaddressTemplate += PQString("featured_position%22%3Bs%3A3%3A%22top%22%3Bs%3A10%3A%22thumb_size%22%3Bs%3A5%3A%22large%22%3Bs%3A11%3A%22tabs_effect%22%3Bs%3A4%3A%22fade%22%3Bs%3A12%3A%22hide_excerpt%22%3Bs%3A0%3A%22%22%3Bs%3A13%3A%22");
        URLaddressTemplate += PQString("hide_bg_image%22%3Bs%3A0%3A%22%22%3Bs%3A15%3A%22icons_animation%22%3Bs%3A0%3A%22%22%3Bs%3A9%3A%22no_margin%22%3Bs%3A0%3A%22%22%3Bs%3A8%3A%22no_links%22%3Bs%3A0%3A%22%22%3Bs%3A10%3A%22pagination%22%3Bs%3A5%3A%22");
        URLaddressTemplate += PQString("pages%22%3Bs%3A4%3A%22page%22%3Bi%3A1%3Bs%3A13%3A%22posts_exclude%22%3Bs%3A0%3A%22%22%3Bs%3A9%3A%22post_type%22%3Bs%3A12%3A%22cpt_services%22%3Bs%3A8%3A%22taxonomy%22%3Bs%3A18%3A%22cpt_services_group%22%3Bs%3A5%3A%22");
        URLaddressTemplate += PQString("popup%22%3Bi%3A0%3Bs%3A9%3A%22more_text%22%3Bs%3A9%3A%22Read+more%22%3Bs%3A11%3A%22count_extra%22%3Ba%3A3%3A%7Bs%3A4%3A%22unit%22%3Bs%3A2%3A%22px%22%3Bs%3A4%3A%22size%22%3Bi%3A12%3Bs%3A5%3A%22");
        URLaddressTemplate += PQString("sizes%22%3Ba%3A0%3A%7B%7D%7Ds%3A13%3A%22columns_extra%22%3Ba%3A3%3A%7Bs%3A4%3A%22unit%22%3Bs%3A2%3A%22px%22%3Bs%3A4%3A%22size%22%3Bi%3A3%3Bs%3A5%3A%22sizes%22%3Ba%3A0%3A%7B%7D%7Ds%3A20%3A%22");
        URLaddressTemplate += PQString("columns_tablet_extra%22%3Ba%3A3%3A%7Bs%3A4%3A%22unit%22%3Bs%3A2%3A%22px%22%3Bs%3A4%3A%22size%22%3Bs%3A0%3A%22%22%3Bs%3A5%3A%22sizes%22%3Ba%3A0%3A%7B%7D%7Ds%3A20%3A%22columns_mobile_extra%22%3Ba%3A3%3A%7Bs%3A4%3A%22");
        URLaddressTemplate += PQString("unit%22%3Bs%3A2%3A%22px%22%3Bs%3A4%3A%22size%22%3Bs%3A0%3A%22%22%3Bs%3A5%3A%22sizes%22%3Ba%3A0%3A%7B%7D%7Ds%3A12%3A%22offset_extra%22%3Ba%3A3%3A%7Bs%3A4%3A%22unit%22%3Bs%3A2%3A%22px%22%3Bs%3A4%3A%22");
        URLaddressTemplate += PQString("size%22%3Bi%3A0%3Bs%3A5%3A%22sizes%22%3Ba%3A0%3A%7B%7D%7Ds%3A18%3A%22slides_space_extra%22%3Ba%3A3%3A%7Bs%3A4%3A%22unit%22%3Bs%3A2%3A%22px%22%3Bs%3A4%3A%22size%22%3Bi%3A0%3Bs%3A5%3A%22sizes%22%3Ba%3A0%3A%7B%7D%7Ds%3A24%3A%22");
        URLaddressTemplate += PQString("gradient_direction_extra%22%3Ba%3A3%3A%7Bs%3A4%3A%22unit%22%3Bs%3A2%3A%22px%22%3Bs%3A4%3A%22size%22%3Bi%3A0%3Bs%3A5%3A%22sizes%22%3Ba%3A0%3A%7B%7D%7Ds%3A17%3A%22typed_speed_extra%22%3Ba%3A3%3A%7Bs%3A4%3A%22");
        URLaddressTemplate += PQString("unit%22%3Bs%3A2%3A%22px%22%3Bs%3A4%3A%22size%22%3Bi%3A6%3Bs%3A5%3A%22sizes%22%3Ba%3A0%3A%7B%7D%7Ds%3A17%3A%22typed_delay_extra%22%3Ba%3A3%3A%7Bs%3A4%3A%22unit%22%3Bs%3A2%3A%22px%22%3Bs%3A4%3A%22");
        URLaddressTemplate += PQString("size%22%3Bi%3A1%3Bs%3A5%3A%22sizes%22%3Ba%3A0%3A%7B%7D%7Ds%3A10%3A%22link_extra%22%3Ba%3A4%3A%7Bs%3A3%3A%22url%22%3Bs%3A0%3A%22%22%3Bs%3A11%3A%22is_external%22%3Bs%3A0%3A%22%22%3Bs%3A8%3A%22");
        URLaddressTemplate += PQString("nofollow%22%3Bs%3A0%3A%22%22%3Bs%3A17%3A%22custom_attributes%22%3Bs%3A0%3A%22%22%3B%7Ds%3A16%3A%22link_image_extra%22%3Ba%3A3%3A%7Bs%3A3%3A%22url%22%3Bs%3A0%3A%22%22%3Bs%3A2%3A%22id%22%3Bs%3A0%3A%22%22%3Bs%3A4%3A%22");
        URLaddressTemplate += PQString("size%22%3Bs%3A0%3A%22%22%3B%7Ds%3A13%3A%22show_subtitle%22%3Bs%3A0%3A%22%22%3Bs%3A6%3A%22scheme%22%3Bs%3A0%3A%22%22%3Bs%3A11%3A%22color_style%22%3Bs%3A7%3A%22default%22%3Bs%3A22%3A%22");
        URLaddressTemplate += PQString("mouse_helper_highlight%22%3Bs%3A0%3A%22%22%3Bs%3A2%3A%22sc%22%3Bs%3A11%3A%22sc_services%22%3B%7D&page=") + paramPlaceholder + PQString("&filters_active=all");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2023 Koru

    case 2024:  // Kowalchuk
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/obituaries-2/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 1;
        else
            flowParameters.endingPosition = 10;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2024 Kowalchuk

    case 2025:  // Loehmer
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/tributes-and-obituaries/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2025 Loehmer

    case 2026:  // Doyle
    {
        flowParameters.initialSetup = false;

        if (flowParameters.initialSetup == false)
        {
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/current-services");

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }
        else
        {
            daysOfOverlap = 181;
            flowParameters.flowType = startToEnd;
            flowParameters.flowIncrement = 20;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + QString("/obituaries/2021?start=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 0;
            flowParameters.endingPosition = 80;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2026 Doyle

    case 2027:  // Ethical
    {
        initialRequest.instructions.url = QString("https://www.ethicaldeathcare.com/_api/v2/dynamicmodel");
        initialRequest.instructions.Accept = QString("*/*");
        initialRequest.instructions.ContentTypeHeader = QString("application/json; charset=UTF-8");
        initialRequest.instructions.Sec_Fetch_Site = QString("same-origin");
        initialRequest.instructions.Sec_Fetch_Mode = QString("cors");
        initialRequest.instructions.Sec_Fetch_Dest = QString("empty");
        initialRequest.instructions.Referer = baseURL.getString() + QString("/");
        //initialRequest.instructions.Accept_Encoding = QString("br");
        initialRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");
        initialRequest.outputs.downloadFileName = QString("tempWebPage.htm");

        www->download(initialRequest);
        while(www->processingDownload()){};
        if(www->lastDownloadSuccessful())
        {
            sourceFile.setSourceFile(initialRequest.outputs.downloadFileName);
            sourceFile.setGlobalVars(*globals);
            sourceFile.beg();

            if (sourceFile.consecutiveMovesTo(50, "14bcded7-0066-7c35-14d7-466cb3f09103", "instance", ":"))
            {
                downloadRequest.instructions.Instance = sourceFile.readNextBetween(QUOTES).getString();
                downloadRequest.instructions.Authorization = downloadRequest.instructions.Instance;
            }
            else
            {
                downloadRequest.instructions.Instance = QString("iVvup2ZIpbVsol04S6G6BGXEw17V3RcXKTJ4IMCdDVQ.eyJpbnN0YW5jZUlkIjoiNTllNDZkMjctMjk3MC00ODEwLWI2MWQtYzFlZGQyOTk5ZjU4IiwiYXBwRGVmSWQiOiIxNGJjZGVkNy0wMDY2LTdjMzUtMTRkNy00NjZjYjNmMDkxMDMiLCJtZXRhU2l0ZUlkIjoiMDFlYWM0YjAtYTU5Yy00NjJlLThhN2YtMmRlNDI0YmIxNmNkIiwic2lnbkRhdGUiOiIyMDIyLTA1LTE1VDE2OjI0OjA2LjI4M1oiLCJkZW1vTW9kZSI6ZmFsc2UsImFpZCI6Ijk0ODJiNWNmLWZlZDEtNDFkNC1iODc1LWYzODdhNzI1YWE0OCIsImJpVG9rZW4iOiI1ODBlYTk5Ny04Y2VjLTBlM2UtM2M2Mi1lYzA5ZjYyMjg5OTUiLCJzaXRlT3duZXJJZCI6IjFjYzg1Mzg1LTZhMWUtNDI1Zi1iNTkwLWE5MGJjMjUwYWZmMCJ9");
                downloadRequest.instructions.Authorization = QString("iVvup2ZIpbVsol04S6G6BGXEw17V3RcXKTJ4IMCdDVQ.eyJpbnN0YW5jZUlkIjoiNTllNDZkMjctMjk3MC00ODEwLWI2MWQtYzFlZGQyOTk5ZjU4IiwiYXBwRGVmSWQiOiIxNGJjZGVkNy0wMDY2LTdjMzUtMTRkNy00NjZjYjNmMDkxMDMiLCJtZXRhU2l0ZUlkIjoiMDFlYWM0YjAtYTU5Yy00NjJlLThhN2YtMmRlNDI0YmIxNmNkIiwic2lnbkRhdGUiOiIyMDIyLTA1LTE1VDE2OjI0OjA2LjI4M1oiLCJkZW1vTW9kZSI6ZmFsc2UsImFpZCI6Ijk0ODJiNWNmLWZlZDEtNDFkNC1iODc1LWYzODdhNzI1YWE0OCIsImJpVG9rZW4iOiI1ODBlYTk5Ny04Y2VjLTBlM2UtM2M2Mi1lYzA5ZjYyMjg5OTUiLCJzaXRlT3duZXJJZCI6IjFjYzg1Mzg1LTZhMWUtNDI1Zi1iNTkwLWE5MGJjMjUwYWZmMCJ9");
            }
        }

        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 40;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/_api/communities-blog-node-api/_api/posts?offset=") + paramPlaceholder;
        URLaddressTemplate += PQString("&size=40&pinnedFirst=true&excludeContent=true");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 80;
        else
            flowParameters.endingPosition = 200;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.Accept = QString("application/json, text/plain, */*");
        downloadRequest.instructions.Referer = QString("https://www.ethicaldeathcare.com/_partials/wix-thunderbolt/dist/clientWorker.1b71ed5d.bundle.min.js");
    }
        break; // 2027 Ethical

    case 2029:  // Direct
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2029 Direct

    case 2030:  // SMC
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/obituaries?e6b1c5dc_page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 38;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2030 SMC

    case 2031:  // Belevedere
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        if (flowParameters.initialSetup)
            URLaddressTemplate  = URLbase + QString("/api/api.php?page=")  + paramPlaceholder + PQString("&endpoint=memorials");
        else
            URLaddressTemplate  = URLbase + QString("/api/api.php?page=")  + paramPlaceholder + PQString("&endpoint=memorials");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 50;
        else
            flowParameters.endingPosition = 4;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");
        downloadRequest.instructions.Origin = URLbase.getString();
        downloadRequest.instructions.X_Origin = URLbase.getString();
        downloadRequest.instructions.Referer = URLbase.getString() + QString("/death_notices");
        downloadRequest.instructions.Sec_Fetch_Site = QString("same-origin");
        downloadRequest.instructions.Sec_Fetch_Mode = QString("cors");
        downloadRequest.instructions.Sec_Fetch_Dest = QString("empty");
        downloadRequest.instructions.Accept_Encoding = QString("br");
        downloadRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");
        //downloadRequest.instructions.ContentTypeHeader = QString("application/x-www-form-urlencoded; charset=UTF-8");
    }
        break; // 2031 Belvedere

    case 2032:  // Davidson
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/obituaries/cwp-funerals/0/")  + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 11;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2032 Davidson

    case 2033:  // Carnell
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/obituaries/page/")  + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 18;
        else
            flowParameters.endingPosition = 6;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

    }   // 2033 Carnell
        break;

    case 2035:  // JOsmond
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2035 JOsmond

    case 2036:  // Tivahost
    {
         daysOfOverlap = 61;
         flowParameters.initialSetup = false;
         flowParameters.flowType = startToEnd;

         URLbase = baseURL;
         URLaddressTemplate  = URLbase + QString("/obituaries/?paging_page_702_l1107121578=")  + paramPlaceholder;

         URLparams.numParams = 1;
         URLparams.param1Type = ptUint;
         URLparams.UIparam1 = &flowParameters.currentPosition;

         flowParameters.currentPosition = 0;
         if (flowParameters.initialSetup)
             flowParameters.endingPosition = 5;
         else
             flowParameters.endingPosition = 1;

         determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
         createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2036 Tivahost

    case 2037:  // KMF
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries?offset") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptQS;
        URLparams.QSparam1 = &flowParameters.lastValueParam;

        flowParameters.lastValueParam = QString("=");

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 25;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2037 KMF

    case 2038:  // AMG
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        if (flowParameters.initialSetup)
            URLaddressTemplate  = URLbase + PQString("/condolences/list.aspx");
        else
            URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2038 AMG

    case 2039:  // Orillia
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/obituaries/?f=obits&pagenum=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 2;
        else
            flowParameters.endingPosition = fhParam1.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2039  Orillia

    case 2040:  // OSM
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 20;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/obituariesnoticess34.php?command=showall&offset=") + paramPlaceholder + QString("&currentFeed=2");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 1;
        else
            flowParameters.endingPosition = fhParam1.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2040  OSM

    case 2041:  // Alcock
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2041 Alcock

    case 2042:  // Abstraact
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/deathnotices/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2042 Abstract

    case 2043:  // Beechwood
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/en/views/ajax?_wrapper_format=drupal_ajax");
        URLaddressTemplate += PQString("&view_name=services&view_display_id=block_5&view_args=&view_path=%2Fnode%2F128&view_base_path=services%2Fexport.csv");
        URLaddressTemplate += PQString("&view_dom_id=217ab857ed846154d9072c04fe4c91fb6bf78f4b9d10f4fb78b1b4bd1c1231cd&pager_element=0&field_death_date_value=All&page=") + paramPlaceholder;
        URLaddressTemplate += PQString("&_drupal_ajax=1&ajax_page_state%5Btheme%5D=beechwood&ajax_page_state%5Btheme_token%5D=");
        URLaddressTemplate += PQString("&ajax_page_state%5Blibraries%5D=beechwood%2Fglobal-styling%2Cbeechwood%2Fglobal-stylingv2%2Cbeechwood%2Fjquery-cookie");
        URLaddressTemplate += PQString("%2Cbeechwood%2Fslick%2Cbeechwood_flower_store%2Fbeechwood_flower_store.commerce%2Cbetter_exposed_filters%2Fgeneral%2Cbootstrap%2Fpopover%2Cbootstrap");
        URLaddressTemplate += PQString("%2Ftooltip%2Ccommerce_stripe%2Fstripe%2Ccore%2Fhtml5shiv%2Cfontawesome%2Ffontawesome.svg.shim%2Csystem%2Fbase%2Cviews%2Fviews.ajax%2Cviews%2Fviews.module");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == false)
            flowParameters.endingPosition = 10;
        else
            flowParameters.endingPosition = fhParam1.toUInt();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");

    }
        break;  // 2043 Beechwood

    case 2044:  // Benjamins
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/MonthView.aspx");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2044 Benjamins

    case 2045:  // Berthiaume
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/Funerailles");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2045 Berthiaume

    case 2046:  // Blenhein
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        //URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");
        // https://blenheimcommunityfuneralhome.com/obituaries/
        // https://blenheimcommunityfuneralhome.com/obituaries/
        URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;
        //URLparams.param1Type = ptUint;
        //URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        /*if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;*/

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2046 Blenhein

    case 2047:  // Brenneman
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;
        //flowParameters.flowType = startToEnd;
        //flowParameters.flowIncrement = -1;

        URLbase = baseURL;
        //URLaddressTemplate  = URLbase + PQString("/current-funerals/categories/") + paramPlaceholder;
        URLaddressTemplate  = URLbase + PQString("/current-funerals");

        URLparams.numParams = 0;
        flowParameters.currentPosition = 0;
        flowParameters.endingPosition = 0;

        /*URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = globals->today.year();
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
        {
            QDate tempDate(globals->today.addDays(-daysOfOverlap));
            flowParameters.endingPosition = tempDate.year();
        }*/

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2047 Brenneman  https://www.cardinalfuneralhomes.com/page/2/?s=a

    case 2048:  // Cardinal
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/page/") + paramPlaceholder + QString("/?s=a");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 6;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2048 Cardinal

    case 2049:  // Carson
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 10;
        downloadRequest.instructions.Accept = QString("application/json, text/javascript, */*; q=0.01");

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/api/public/") + fhParam2 + QString("-list?skip=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 20;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2049 Carson

    case 2050:  // Turner
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + QString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2050 Turner

    case 2052:  // Eagleson
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/obituaries/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2052 Eagleson

    case 2053:  // FirstMemorial
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/ottawa-obituaries/page/") + paramPlaceholder + PQString("/?gclid=Cj0KCQiAz9ieBhCIARIsACB0oGLawVRbL_FK3a0Kpbn4xR9TLzJknh44C8TuCCNhYHI-MPf_lxZ0gDUaAqxlEALw_wcB");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2053 FirstMemorial

    case 2054:  // Haine
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/current.html");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2054 Haine

    case 2056:  // RHB
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;

        if (flowParameters.initialSetup)
            URLaddressTemplate  = URLbase + QString("/cms/index.php/obituaries/archive/");
        else
            URLaddressTemplate  = URLbase + QString("/cms/index.php/obituaries/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2056 RHB

    case 2057:  // Rhody
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/life_story/page/") + paramPlaceholder + QString("/?et_blog");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2057 Rhody

    case 2058:  // Simpler
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituary/ajax/record-request.php?pageNum=") + paramPlaceholder + QString("&recordDate=");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2058 Simpler

    case 2059:  // Steadman
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/obituaries/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2059 Steadman

    case 2060:  // Steeles
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/records/?selectfieldshow=");
        if (flowParameters.initialSetup)
            URLaddressTemplate += PQString("last-180");
        else
            URLaddressTemplate += PQString("last-30");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");

    }
        break; // 2060 Steeles

    case 2061:  // Bridge
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/cremate/index.php/obituaries/page-") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2061 Bridge

    case 2062:  // McCormack
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2062 McCormack

    case 2063:  // Brunet
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        //URLaddressTemplate  = URLbase + PQString("/index.php/?inSite=1");
        //URLaddressTemplate  = PQString("http://archive.brunetfuneralhome.ca/index.php/#top");
        URLaddressTemplate  = URLbase + PQString("/obituaries/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2063 Brunet

    case 2065:  // TurnerFamily
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2065 TurnerFamily

    case 2066:  // VanHeck
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/category/general/page/") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = fhParam1.toUInt();
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2066 VanHeck

    case 2067:  // TBK
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2067 TBK

    case 2068:  // Whelan
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/services.html");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2068 Whelan

    case 2069:  // Aeterna
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/en/death-notices/?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 87;
        else
            flowParameters.endingPosition = 8;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2069 Aeterna

    case 2070:  // Actuel
    {
        flowParameters.initialSetup = false;

        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 599;
            flowParameters.flowType = alphabetical;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/en/obituary-find-a-person/?letter=") + paramPlaceholder + PQString("&pg=") + paramPlaceholder;

            URLparams.numParams = 2;
            URLparams.param1Type = ptQS;
            URLparams.QSparam1 = &flowParameters.currentLetter;
            URLparams.param2Type = ptUint;
            URLparams.UIparam2 = &flowParameters.currentPosition;


            flowParameters.letterCurrent = 97;
            flowParameters.letterEnd = 122;
            flowParameters.currentLetter = QString(QChar(flowParameters.letterCurrent));
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;

            pageVariables.paginatedResult = false;
            flowParameters.lastValueParam = QString("1");
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/en/obituaries-montreal/?pg=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 5;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2070 Actuel

    case 2071:  // Dupuis
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/mod/act_p/AvisAct.php?");
        URLaddressTemplate += PQString("a=updateAvis&filter%5Bfullname%5D=all&filter%5Bsalon%5D=all&filter%5Byear%5D=all&filter%5Bpage%5D=") + paramPlaceholder;
        URLaddressTemplate += PQString("&module%5BContentAvis%5D=pagination");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 28;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");

    }
        break; // 2071 Dupus

    case 2072:  // HGDivision
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/fr/Necrologies/Index?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 135;
        else
            flowParameters.endingPosition = 10;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");

    }
        break; // 2072 HGDivision

    case 2073:  // Jacques
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/uploads/ajax/mort.php?tri=DESC&page=") + paramPlaceholder;
        URLaddressTemplate += PQString("&triElement=dateDeces&recherche=&rechercheDate=&pageLargeur=2543&retourVersNecro=necro");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 12;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");

    }
        break; // 2073 Jacques

    case 2074:  // Joliette
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        pageVariables.usePubDateForCutOff = true;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 60;
        else
            flowParameters.endingPosition = 6;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        downloadRequest.instructions.verb = QString("POST");

    }
        break; // 2074 Joliette

    case 2075:  // Rajotte
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/avis_de_deces");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2075 Rajotte

    case 2076:  // BM
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/necrologie?filtre=recent");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2076 BM

    case 2077:  // Jodoin
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 14;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2077 Jodoin

    case 2078:  // Fournier
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/deces/recherche?mois=&annee=&salon=&afficher=50&page=") + paramPlaceholder + PQString("&motclef=");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 10;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2078 Fournier

    case 2079:  // Desnoyer
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 30;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2079 Desnoyer

    case 2080:  // Desrosiers
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 65;
        else
            flowParameters.endingPosition = 6;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2080 Desrosiers

    case 2081:  // MontPetit
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/fr/avis-deces?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 6;
        else
            flowParameters.endingPosition = 0;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2081 MontPetit

    case 2082:  // Parent
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        flowParameters.flowIncrement = 20;

        URLbase = baseURL;
        //URLaddressTemplate  = URLbase + PQString("/avis-de-deces/?f=obits&pagenum=") + paramPlaceholder;
        URLaddressTemplate  = URLbase + PQString("/index.php/avis-de-deces?start=") + paramPlaceholder;
        //https://www.parentst-hilaire.com/index.php/avis-de-deces?start=20

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 0;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 20;
        else
            flowParameters.endingPosition = 0;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2082 Parent

    case 2083:  // RichardPhilibert
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        pageVariables.usePubDateForCutOff = true;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/blog.php?pn=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 72;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2083 RichardPhilibert

    case 2084:  // Kane
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + QString("/en/funeral.php");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2084 Kane

    case 2085:  // Gaudet
    {
        flowParameters.initialSetup = false;

        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 599;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + QString("/avis-de-deces-archives.php?p=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 8;
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + QString("/avis-de-deces.php");

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break;  // 2085 Gaudet

    case 2087:  // NouvelleVie
    {
        flowParameters.initialSetup = false;

        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 599;
            flowParameters.flowType = alphabetical;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/avis-de-deces-recherche.php?lettre=") + paramPlaceholder;

            URLparams.numParams = 2;
            URLparams.param1Type = ptQS;
            URLparams.QSparam1 = &flowParameters.currentLetter;
            URLparams.param2Type = ptUint;
            URLparams.UIparam2 = &flowParameters.currentPosition;


            flowParameters.letterCurrent = 65;
            flowParameters.letterEnd = 90;
            flowParameters.currentLetter = QString(QChar(flowParameters.letterCurrent));
            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/avis-de-deces.php");

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2087 NouvelleVie

    case 2088:  // Santerre
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        pageVariables.usePubDateForCutOff = true;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 23;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2088 Santerre

    case 2089:  // Shields
    {
        flowParameters.initialSetup = false;

        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 599;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/Funerailles");

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase;

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2089 Shields

    case 2090:  // Gamache
    {
        flowParameters.initialSetup = false;

        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 599;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/fr/avis/archives/+") + paramPlaceholder + PQString("+");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 26;
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/fr/avis/deces/");

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2090 Gamache

    case 2091:  // Landry
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/") +fhParam1 + PQString("/avis_de_deces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = fhParam2.toUInt();
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2091 Landry

    case 2092:  // St.Louis
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 52;
        else
            flowParameters.endingPosition = 5;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2092 St.Louis

    case 2093:  // McGerrigle
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/category/avis-de-deces-obituaries/");

        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2093 McGerrigle

    case 2094:  // Paperman
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/en/funerals");

        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2094 Paperman

    case 2095:  // Poissant
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/pages/") + paramPlaceholder + PQString("#page");
        //http://poissantetfils.ca/fr/avis-de-deces/pages/2#page

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 65;
        else
            flowParameters.endingPosition = 5;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2095 Poissant

    case 2096:  // Legare
    {
        flowParameters.initialSetup = false;

        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 399;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/site/archive/") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 2022;
            flowParameters.endingPosition = 2022;
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/avis_deces");

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2096 Legare

    case 2097:  // Longpre
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/");

        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2097 Longpre

    case 2098:  // Lanaudiere
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        pageVariables.usePubDateForCutOff = true;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/inmemoriam/funerailles/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 37;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2098 Lanaudiere

    case 2100:  // Voluntas
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 20;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2100 Voluntas

    case 2101:  // Wilbrod
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        pageVariables.usePubDateForCutOff = true;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/fr/avis-de-deces?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 35;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2101 Wilbrod

    case 2102:  // Hodges
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries?offset") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptQS;
        URLparams.QSparam1 = &flowParameters.lastValueParam;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 15;
        else
            flowParameters.endingPosition = 1;

        flowParameters.lastValueParam=QString("=");

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2102 Hodges

    case 2103:  // Bergeron
    {
        flowParameters.initialSetup = false;

        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 999;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/archive.php?page=") + paramPlaceholder;

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 17;
            flowParameters.endingPosition = 45;
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase;

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2103 Bergeron

    case 2104:  // Passage
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/index.asp");

        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2104 Passage

    case 2105:  // Granit
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/necrologie.php");

        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2105 Granit

    case 2106:  // Affordable
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 24;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2106 Affordable

    case 2107:  // LFC
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/fr/trouver-un-avis/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2107 LFC

    case 2108:  // Life Transitions
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/runtime/390728/default.php?branchId=all&page=") + paramPlaceholder;
        URLaddressTemplate += PQString("&type=current&sort=DateOfDeath&sortdir=DESC&sortLabel=Date+of+Death&start=&end=&datetype=ServiceDate&tpl=fsg&SiteId=&NavigatorId=&nowrap=1&widget=Services&search=&listclass=&rpp=5");;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 9;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2108 Life Transitions

    case 2109:  // Davis
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/#memorials");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2109 Davis

    case 2110:  // MacLark
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/condolences/page/") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 8;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // MacLark

    case 2111:  // Fallis
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituary-notices?p=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 10;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2111 Fallis

    case 2112:  // Timiskaming
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/en/death-notices/?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 93;
        else
            flowParameters.endingPosition = 4;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2112 Timiskaming

    case 2113:  // Garrett
    {
        flowParameters.initialSetup = false;

        if (flowParameters.initialSetup)
        {
            daysOfOverlap = 599;
            flowParameters.flowType = startToEnd;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/index.php?nxs-webmethod-queryparameter=true");
            URLaddressTemplate += PQString("&action=nxs_ajax_webmethods&webmethod=lazyloadblog&postcontainerid=702&postid=702&paging_page=") + paramPlaceholder;
            URLaddressTemplate += PQString("&placeholderid=l1107121578");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 4;

            downloadRequest.instructions.verb = QString("POST");
        }
        else
        {
            daysOfOverlap = 61;
            flowParameters.flowType = singleListing;

            URLbase = baseURL;
            URLaddressTemplate  = URLbase + PQString("/obituaries/");

            URLparams.numParams = 0;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 1;
        }

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2113 Garrett

    case 2114:  // Smith
    {
        flowParameters.initialSetup = false;

        daysOfOverlap = 61;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/smith/death-notices?filter_year=")  + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        flowParameters.currentPosition = pageVariables.cutoffDate.year();
        flowParameters.endingPosition = globals->today.year();

        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2114 Smith

    case 2115:  // Picard
    {
        flowParameters.initialSetup = false;

        daysOfOverlap = 61;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-deces.php?annee=")  + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        flowParameters.currentPosition = pageVariables.cutoffDate.year();
        //flowParameters.currentPosition = globals->today.year();
        flowParameters.endingPosition = globals->today.year();

        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2115 Picard

    case 2116:  // Richelieu
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  8;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2116 Richelieu

    case 2117:  // Roy
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2117 Roy

    case 2118:  // Charlevoix
    {
        flowParameters.initialSetup = false;

        daysOfOverlap = 61;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avisdeces.php?page=")  + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 19;
        else
            flowParameters.endingPosition = 5;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2118 Charlevoix

    case 2119:  // Aurora
    {
        flowParameters.initialSetup = false;

        daysOfOverlap = 61;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/fr/") + fhParam1 + PQString("/")  + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 29;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2119 Aurora

    case 2120:  // Montcalm
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/listedeces/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2120 Montcalm

    case 2122:  // Laurent
    {
        flowParameters.initialSetup = false;

        daysOfOverlap = 61;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL + PQString("/fr/avis-de-deces");
        //URLaddressTemplate  = URLbase + PQString("/deces/page/") + paramPlaceholder + PQString("/");
        URLaddressTemplate  = URLbase + PQString("/?page=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 27;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2122 Laurent

    case 2123:  // Eternal
    {
        flowParameters.initialSetup = false;

        daysOfOverlap = 61;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/?p=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 2;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2123 Eternal

    case 2124:  // Ruel
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        if (flowParameters.initialSetup)
            URLaddressTemplate  = URLbase + PQString("/avis-de-deacutecegraves-anteacuterieurs.html");
        else
            URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2124 Ruel

    case 2125:  // Hamel
    {
        flowParameters.initialSetup = false;

        daysOfOverlap = 61;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/?f=obits&pagenum=") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 28;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2125 Hamel

    case 2126:  // CremAlt
    case 2129:  // Forest
    case 2137:  // Tricity
    {
        flowParameters.initialSetup = false;

        daysOfOverlap = 61;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries--condolences/previous/") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 3;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2126 CremAlt, Forest, TriCity

    case 2127:  // London
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/condolences/?count=-1");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2127 London

    case 2128:  // Dryden
    {
        flowParameters.initialSetup = false;

        daysOfOverlap = 61;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries-grid/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 8;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2128 Dryden

    case 2131:  // Lampman
    {
        flowParameters.initialSetup = false;

        daysOfOverlap = 61;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries-tributes/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 8;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2131 Lampman

    case 2132:  // ecoPassages
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;
        pageVariables.usePubDateForCutOff = true;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries?offset") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptQS;
        URLparams.QSparam1 = &flowParameters.lastValueParam;

        flowParameters.lastValueParam = QString("=");

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 10;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2132 ecoPassages

    case 2133:  // Peaceful
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2133 Peaceful

    case 2134:  // Ranger
    {
        flowParameters.initialSetup = false;

        daysOfOverlap = 61;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/category/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 155;
        else
            flowParameters.endingPosition = 15;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2134 Ranger

    case 2135:  // People
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        if (flowParameters.initialSetup)
            URLaddressTemplate  = URLbase + PQString("/archives/");
        else
            URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2135 People

    case 2136:  // Whitcroft
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        if (flowParameters.initialSetup)
            URLaddressTemplate  = URLbase + PQString("/2020%20Death%20Notices.htm");
        else
            URLaddressTemplate  = URLbase + PQString("/Death%20Notices.html");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2136 Whitcroft

    /*case 2137:  // TriCity
    {
        flowParameters.initialSetup = false;

        daysOfOverlap = 61;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries--condolences/archives/") + paramPlaceholder + PQString("-") + paramPlaceholder;
        //URLaddressTemplate  = URLbase + PQString("/obituaries--condolences/");

        URLparams.numParams = 2;
        URLparams.param1Type = ptJulian;
        URLparams.Julian1 = &flowParameters.currentPosition;
        URLparams.maskType1 = mtMM;
        URLparams.param2Type = ptJulian;
        URLparams.Julian2 = &flowParameters.currentPosition;
        URLparams.maskType2 = mtYYYY;

        flowParameters.currentPosition = globals->today.toJulianDay();
        flowParameters.endingPosition = globals->today.toJulianDay();

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2137 True*/

    case 2138:  // LegacyCardstrom
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase;

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2138 LegacyCardstrom

    case 2139:  // LegacyCardstrom
    {
        daysOfOverlap = 361;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        QDate switchOver(globals->today.addDays(-7));
        int yyyy = switchOver.year();
        if (flowParameters.initialSetup)
            URLaddressTemplate  = URLbase + PQString("/obituaries/2020");
        else
            //URLaddressTemplate  = URLbase + PQString("/obituaries/") + PQString(yyyy) + PQString("-obituaries");
            URLaddressTemplate  = URLbase + PQString("/obituaries/") + PQString(yyyy);

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2139 LegacyCardstrom

    case 2140:  // Arimathea
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries?offset") + paramPlaceholder;

        URLparams.numParams = 1;
        URLparams.param1Type = ptQS;
        URLparams.QSparam1 = &flowParameters.lastValueParam;

        flowParameters.lastValueParam = QString("=");

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup == true)
            flowParameters.endingPosition = 15;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2140 Arimathea

    case 2141:  // GFournier
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/deces/recherche?mois=0&annee=0&salon=0&afficher=50&page=") + paramPlaceholder + PQString("&motclef=");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 10;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2141 GFournier

    case 2143:  // Harmonia
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/?s=date&p=1&nb=all");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2139 LegacyCardstrom

    case 2144:  // Omega
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        if (flowParameters.initialSetup)
            URLaddressTemplate  = URLbase + PQString("/archives-des-décès");
        else
            //URLaddressTemplate  = URLbase + PQString("/avis-de-deces");
            URLaddressTemplate  = URLbase + PQString("/copie-de-avis-de-décès");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2144 Omega

    case 2145:  // HeritageWP
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2145 HeritageWP

    case 2146:  // Ouellet
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        if (flowParameters.initialSetup)
            URLaddressTemplate  = URLbase + PQString("/about.html");
        else
            URLaddressTemplate  = URLbase + PQString("/home.html");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2146 Ouellet

    case 2147:  // HommageNB
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 88;
        else
            flowParameters.endingPosition = 7;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2147 Hommage NB

    case 2148:  // Drake
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 60;
        else
            flowParameters.endingPosition = 3;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2148 Drake

    case 2149:  // CityLine
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/tribute");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 2;
        else
            flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2149 CityLine

    case 2150:  // Komitas
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
        {
            flowParameters.endingPosition = 9;
            daysOfOverlap = 391;
        }
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2150 Komitas

    case 2151:  // Driftwood
    {
        flowParameters.initialSetup = false;
        pageVariables.usePubDateForCutOff = true;

        daysOfOverlap = 61;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/category/obituaries/page/") + paramPlaceholder + PQString("/");

        URLparams.numParams = 1;
        URLparams.param1Type = ptUint;
        URLparams.UIparam1 = &flowParameters.currentPosition;

        flowParameters.currentPosition = 1;
        if (flowParameters.initialSetup)
            flowParameters.endingPosition = 40;
        else
            flowParameters.endingPosition = 2;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2151 Driftwood

    case 2152:  // MLBW  (see also 1052 Citrus)
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = startToEnd;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/avis-de-deces/");
        URLparams.numParams = 0;
        flowParameters.currentPosition = 1;
        flowParameters.endingPosition = 1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

        www->download(downloadRequest);
        while(www->processingDownload()){};
        if(www->lastDownloadSuccessful())
        {
            sourceFile.setSourceFile(downloadRequest.outputs.downloadFileName);

            if (sourceFile.consecutiveMovesTo(300, "JetEngineSettings", "ajaxlisting", ":"))
            {
                URLbase = sourceFile.readNextBetween(QUOTES);
                URLbase.JSONsimplify();
                URLaddressTemplate = URLbase;
            }

            URLaddressTemplate = URLbase;
            URLaddressTemplate += PQString("&action=jet_engine_ajax&handler=listing_load_more&query%5Bpost_status%5D%5B%5D=publish&query%5Bpost_type%5D=avis-de-deces&query%5Bposts_per_page%5D=15&query%5Bpaged%5D=1");
            URLaddressTemplate += PQString("&query%5Bignore_sticky_posts%5D=1&query%5Bsuppress_filters%5D=false&query%5Bjet_smart_filters%5D=jet-engine%2Flisting&widget_settings%5Blisitng_id%5D=560");
            URLaddressTemplate += PQString("&widget_settings%5Bposts_num%5D=15&widget_settings%5Bcolumns%5D=3&widget_settings%5Bcolumns_tablet%5D=2&widget_settings%5Bcolumns_mobile%5D=1&widget_settings%5Bis_archive_template%5D=");
            URLaddressTemplate += PQString("&widget_settings%5Bpost_status%5D%5B%5D=publish&widget_settings%5Buse_random_posts_num%5D=&widget_settings%5Bmax_posts_num%5D=9");
            URLaddressTemplate += PQString("&widget_settings%5Bnot_found_message%5D=Aucun+avis+de+d%C3%A9c%C3%A8s+trouv%C3%A9.&widget_settings%5Bis_masonry%5D=false&widget_settings%5Bequal_columns_height%5D=");
            URLaddressTemplate += PQString("&widget_settings%5Buse_load_more%5D=yes&widget_settings%5Bload_more_id%5D=load-more&widget_settings%5Bload_more_type%5D=click&widget_settings%5Buse_custom_post_types%5D=");
            URLaddressTemplate += PQString("&widget_settings%5Bhide_widget_if%5D=&widget_settings%5Bcarousel_enabled%5D=&widget_settings%5Bslides_to_scroll%5D=1&widget_settings%5Barrows%5D=true");
            URLaddressTemplate += PQString("&widget_settings%5Barrow_icon%5D=fa+fa-angle-left&widget_settings%5Bdots%5D=&widget_settings%5Bautoplay%5D=true&widget_settings%5Bautoplay_speed%5D=5000");
            URLaddressTemplate += PQString("&widget_settings%5Binfinite%5D=true&widget_settings%5Bcenter_mode%5D=&widget_settings%5Beffect%5D=slide&widget_settings%5Bspeed%5D=500");
            URLaddressTemplate += PQString("&widget_settings%5Binject_alternative_items%5D=&widget_settings%5Bscroll_slider_enabled%5D=&widget_settings%5Bscroll_slider_on%5D%5B%5D=desktop");
            URLaddressTemplate += PQString("&widget_settings%5Bscroll_slider_on%5D%5B%5D=tablet&widget_settings%5Bscroll_slider_on%5D%5B%5D=mobile&widget_settings%5Bcustom_query%5D=false");
            URLaddressTemplate += PQString("&widget_settings%5Bcustom_query_id%5D=&widget_settings%5B_element_id%5D=listing&page_settings%5Bpost_id%5D=false&page_settings%5Bqueried_id%5D=false");
            URLaddressTemplate += PQString("&page_settings%5Belement_id%5D=false&page_settings%5Bpage%5D=") + paramPlaceholder + PQString("&listing_type=false&isEditMode=false");
            URLaddressTemplate += PQString("");

            downloadRequest.instructions.verb = QString("POST");

            URLparams.numParams = 1;
            URLparams.param1Type = ptUint;
            URLparams.UIparam1 = &flowParameters.currentPosition;

            flowParameters.currentPosition = 1;
            flowParameters.endingPosition = 4;

            createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
        }
    }
        break; // 2152 MLBW

    case 2153:  // Sproing
    {
        daysOfOverlap = 61;
        flowParameters.initialSetup = false;
        flowParameters.flowType = singleListing;

        URLbase = baseURL;
        URLaddressTemplate  = URLbase + PQString("/obituaries");

        URLparams.numParams = 0;

        flowParameters.currentPosition = 1;
        flowParameters.endingPosition =  1;

        determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
        break; // 2153 Sproing

    default:
        qDebug() << "Attempt to retrieve URLs for unknown/uncoded provider";
        return;
    }

    /*******************************/
    /*      Real  query(ies)       */
    /*******************************/

    OQString cleanString;
    flowParameters.keepDownloading = true;
    if (globals->runStatus != 2)
        flowParameters.initialSetup = false;  // Needed in case I forgot to flip switch from last setup
    if (!pageVariables.cutoffDate.isValid())
        pageVariables.cutoffDate = pageVariables.latestDODdate.addDays(-daysOfOverlap);

    while (flowParameters.keepDownloading)
    {
        www->download(downloadRequest);
        while(www->processingDownload()){};
        if(www->lastDownloadSuccessful())
        {
            sourceFile.setSourceFile(downloadRequest.outputs.downloadFileName);
            sourceFile.setGlobalVars(*globals);
            sourceFile.beg();
            temp.setGlobalVars(*globals);
            temp.setContentLanguage(lang);

            switch(providerID)
            {
            case 1:
            {

                while (sourceFile.consecutiveMovesTo(100, "PersonCardNameLink", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

                break;
            }

            case 2:
            {
                // Only URL, DOB and DOD typically available
                sourceFile.moveTo("ul class=\"obitlist\"");

                while (sourceFile.consecutiveMovesTo(100, "<h2>", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo(QString("class=\"uppercase\""), QString("class=\"summary\""), 0))
                    {
                        string = sourceFile.readNextBetween(BRACKETS).getString();
                        if (string.left(10) == QString("Passed on "))
                            pageVariables.ucDOD = string.right(string.length() - 10);
                        else
                        {
                            if (string.left(8) == QString("Born on "))
                                pageVariables.ucDOB = string.right(string.length() - 8);
                            else
                                pageVariables.ucDOBandDOD = string;
                        }
                    }

                    if (sourceFile.conditionalMoveTo(QString("published in the Winnipeg Free Press on: "), QString("href="), 0))
                    {
                        temp = sourceFile.getUntil("<");
                        numTemp = temp.countFrequency(QString(","));
                        if (numTemp > 1)
                        {
                            unsigned position = temp.findPosition(PQString(","), -1, 0, 2);
                            pageVariables.ucPubDate = temp.right(temp.getLength() - position - 1);
                        }
                        else
                            pageVariables.ucPubDate = temp;
                    }


                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

                break;
            }

            case 6:  // Cooperative Funeraire
            {
                while (sourceFile.moveTo("class=\"notice-in-grid\""))
                {
                    QString tempString;

                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    sourceFile.moveBackwardTo("href=");

                    tempString = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars().getString();
                    if (tempString.left(4).compare("http") == 0)
                        pageVariables.webAddress = tempString;
                    else
                    {
                        if (providerKey == 111)
                            pageVariables.webAddress = URLbase + PQString(tempString);
                        else
                            pageVariables.webAddress = URLbase + PQString(tempString);
                    }
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("</h3>", "</li>", 1))
                    {
                        if (providerKey == 111)
                        {
                            temp = sourceFile.getUntil("<br");
                            temp.cleanUpEnds();
                            pageVariables.pcKey = temp.getString();
                            pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);
                        }
                        else
                        {
                            sourceFile.moveTo("<br");
                            pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                        }
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 6 Cooperative Funeraire
                break;

            case 8: // Domaine Funeraire
            {
                while(sourceFile.consecutiveMovesTo(25, "class=\"card\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read YOB - YOD
                    if (sourceFile.conditionalMoveTo("<span>", "</p>"))
                        pageVariables.ucVariable = sourceFile.getUntil("</p>");

                    // Read Location and DOD
                    if (sourceFile.conditionalMoveTo(" | ", "</div>"))
                    {
                        sourceFile.moveBackwardTo(">");
                        temp = sourceFile.getUntil(" | ");
                        temp.cleanUpEnds();
                        pageVariables.pcKey = temp.getString();

                        pageVariables.ucDOD = sourceFile.getUntil("<");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (((pageVariables.includeRecord) || flowParameters.initialSetup) && ((record.yod == 0) || (record.yod > 2020)))
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // case 8 Domaine Funeraire

            case 11:  // GreatWest
            {
                switch(providerKey)
                {
                case 18:
                    while(sourceFile.moveTo("<div class=\"media-left\">"))
                    {
                        record.clear();
                        pageVariables.reset();

                        sourceFile.moveBackwardTo("href=", 200);
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        sourceFile.moveTo("</div>");
                        if (sourceFile.conditionalMoveTo("datetime=\"", "<div class=\"media-left\">", 0))
                            pageVariables.ucPubDate = sourceFile.getUntil("T");

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                case 21:
                    while(sourceFile.moveTo("class=\"card card-block\""))
                    {
                        record.clear();
                        pageVariables.reset();

                        sourceFile.moveBackwardTo("href=");
                        URLext = sourceFile.readNextBetween(DOUBLE_QUOTES);
                        pageVariables.webAddress = URLbase + URLext;
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOD
                        if (sourceFile.conditionalMoveTo("class=\"card-text\"", "href=", 0))
                            pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                        else
                            sourceFile.forward(200);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                case 20:
                case 26:
                    while(sourceFile.moveTo("class=\"media media-bordered\""))
                    {
                        record.clear();
                        pageVariables.reset();

                        sourceFile.moveBackwardTo("href=");
                        URLext = sourceFile.readNextBetween(DOUBLE_QUOTES);
                        pageVariables.webAddress = URLbase + URLext;
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOD
                        if (sourceFile.conditionalMoveTo("Date of death: ", "href="))
                        {
                            pageVariables.ucDOD = sourceFile.getUntil(" Date ");

                            if (sourceFile.conditionalMoveTo("Date of birth: ", "href="))
                            {
                                pageVariables.ucDOB = sourceFile.getUntil(" Age: ");
                                pageVariables.ageAtDeath = sourceFile.getUntil(" ").asNumber();
                            }
                        }

                        // Read DOP
                        if (sourceFile.conditionalMoveTo("datetime=\"", "href=", 0))
                            pageVariables.ucPubDate = sourceFile.getUntil("T");

                        sourceFile.moveTo("</a>");

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                default:
                    while(sourceFile.moveTo("class=\"media media-bordered\""))
                    {
                        record.clear();
                        pageVariables.reset();

                        sourceFile.moveBackwardTo("href=");
                        URLext = sourceFile.readNextBetween(DOUBLE_QUOTES);
                        pageVariables.webAddress = URLbase + URLext;
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOP
                        if (sourceFile.conditionalMoveTo("datetime=\"", "href=", 0))
                            pageVariables.ucPubDate = sourceFile.getUntil("T");
                        else
                            sourceFile.forward(200);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // case 11 GreatWest

            case 12:  // ObitTree
            {
                while(sourceFile.consecutiveMovesTo(100, "class=\"obituaries-list\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    if (sourceFile.conditionalMoveTo("itemprop=\"deathDate\"", "class=\"obituaries-list\"", 0))
                    {
                        sourceFile.moveTo("datetime=", 25);
                        pageVariables.ucDOD = sourceFile.readNextBetween(QUOTES);
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
                sourceFile.beg();
                if (!sourceFile.moveTo(">Next Page<"))
                    flowParameters.keepDownloading = false;

            }
                break;

            case 13:  // Echovita
            {
                while(sourceFile.consecutiveMovesTo(50, "text-name-obit-in-list", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = baseURL + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("<span class=", "</div>"))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Age at Death
                    if (sourceFile.conditionalMoveTo("<span class=", "</div>"))
                    {
                        temp = sourceFile.readNextBetween(BRACKETS);
                        temp.cleanUpEnds();
                        temp.removeBookEnds(PARENTHESES);
                        int index = temp.findPosition(" ");
                        if (index > 0)
                        {
                            int age = temp.left(index).asNumber();
                            if ((age > 0) && (age < 125))
                                pageVariables.ageAtDeath = age;
                        }
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break;  // Echovita

            case 14:  // Saltwire
            {
                while(sourceFile.consecutiveMovesTo(100, "class=\"sw-obit-list__item\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    pageVariables.currentPubDate = QDate::fromJulianDay(flowParameters.currentPosition);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break;  // Saltwire

            case 15:    // Black Media
            {
                if (fhSpecialCode == 1)
                {
                    while(sourceFile.consecutiveMovesTo(100, "class=\"large-3 columns obituary\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        if (sourceFile.conditionalMoveTo("class=\"author-meta", "class=\"large-3 columns obituary\"", 0))
                        {
                            if (sourceFile.moveTo("title=", 150))
                                pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    QDate anotherRecord;
                    dateFormat = QString("yyyy-MM-dd");
                    int index;
                    int consecutiveExcludedCount = 0;
                    bool keepGoing = true;
                    sourceFile.beg();

                    while (keepGoing && sourceFile.loadValue(QString("publish_time"), anotherRecord, dateFormat))
                    {
                        record.clear();
                        pageVariables.reset();

                        pageVariables.currentPubDate = anotherRecord;
                        sourceFile.loadValue(QString("url"), pageVariables.webAddress);
                        pageVariables.webAddress = HTTP + QString("//") + pageVariables.webAddress;
                        index = pageVariables.webAddress.findPosition(fhURLidDivider);
                        if (index >= 0)
                            pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                        else
                            pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter("LAST", "/").getString();

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.webAddress.getString().contains("u201"))
                            pageVariables.includeRecord = false;
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        {
                            records.append(record);
                            consecutiveExcludedCount = 0;
                        }
                        else
                            consecutiveExcludedCount++;

                        if (consecutiveExcludedCount >= 5)
                            keepGoing = false;
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break;  // Black Media

            case 16:  // Aberdeen
            {
                sourceFile.JSONsimplify();
                sourceFile.unescapeJSONformat();
                sourceFile.unQuoteHTML();

                while(sourceFile.consecutiveMovesTo(25, "class=\"entry-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.moveTo("class=\"published\""))
                        pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break;  // Aberdeen

            case 17:  // Village
            {

            }
                break;  // Village


            case 1000:
            {
                switch(providerKey)
                {
                case 15089:
                {
                    // JSON approach
                    int anotherRecord;
                    dateFormat = QString("MM/dd/yyyy");
                    sourceFile.beg();

                    while (sourceFile.loadValue(QString("show_cards"), anotherRecord, false))
                    {
                        record.clear();
                        pageVariables.reset();

                        sourceFile.loadValue(QString("obit_path"), pageVariables.webAddress);
                        sourceFile.loadValue(QString("birth_date"), pageVariables.currentDOB, dateFormat);
                        sourceFile.loadValue(QString("o_id"), pageVariables.ID);
                        sourceFile.loadValue(QString("death_date"), pageVariables.currentDOD, dateFormat);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }

                    updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
                    break;
                }

                default:
                    // Obits are listed from most recent to older
                    while(sourceFile.consecutiveMovesTo(75, "obit-item", "a href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        URLext = sourceFile.readNextBetween(DOUBLE_QUOTES);
                        URLtemp = URLext;
                        URLtemp.dropLeft(10);  // Drop "/obituary/"
                        /*if (!URLtemp.isNumeric())
                        {
                            if (sourceFile.conditionalMoveTo(QString("images/obituaries/"), QString("</a>"), 0))
                            {
                                URLtemp = sourceFile.getUntil(QString("."), 30);
                                numTemp = static_cast<unsigned int>(URLtemp.extractFirstNumber());
                                if (numTemp > 0)
                                {
                                    URLext = URLext.left(10);
                                    URLext += PQString(numTemp);
                                }
                            }
                        }*/

                        pageVariables.webAddress = URLbase + URLext;

                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                        /*if (pageVariables.sequentialID)
                        {
                            word = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                            if (word.isNumeric())
                                pageVariables.ID = word.getString();
                        }*/

                        // Read DOB and DOD
                        if (sourceFile.conditionalMoveTo("lifespan", "obit-item", 1))
                            pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }

                    updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
                }
            }
                break; // Case 1000

            case 1002:
            {
                // Obits are listed from most recent to older

                while(sourceFile.moveTo("class=\"rsidecoltext\" href="))
                {
                    record.clear();
                    pageVariables.reset();

                    URLext = sourceFile.readNextBetween(DOUBLE_QUOTES);
                    pageVariables.webAddress = URLbase + URLext;

                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    if (sourceFile.consecutiveMovesTo(50, "class=\"rsidecoltext\"", "d."))
                        pageVariables.ucDOD = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // case 1002

            case 1003:
            {
                PQString tempString;

                // Determine base URL and setup

                if (sourceFile.moveTo("property=\"og:locale\" content="))
                {
                    /*PQString langCode = sourceFile.readNextBetween(DOUBLE_QUOTES);
                    if (langCode == PQString("en"))
                        lang = english;
                    else
                    {
                        if (langCode == PQString("es"))
                            lang = spanish;
                        else
                        {
                            if (langCode == PQString("fr"))
                                lang = french;
                            else
                                lang = language_unknown;
                        }
                    }*/

                    if (sourceFile.moveTo("property=\"og:url\" content="))
                    {
                        URLbase = sourceFile.readNextBetween(DOUBLE_QUOTES);
                        URLbase.removeEnding("obituaries");
                    }
                }

                // Obits are listed from most recent to older

                while(sourceFile.consecutiveMovesTo(100, "DM", "-link", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    tempString = sourceFile.readNextBetween(DOUBLE_QUOTES);
                    if (tempString.left(4) != PQString("http"))
                        pageVariables.webAddress = URLbase  + tempString;
                    else
                        pageVariables.webAddress = tempString;
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOB and DOD
                    sourceFile.moveTo("screen-title-date");
                    if (sourceFile.conditionalMoveTo("<span", "</div>", 1))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);
                    pageVariables.numericDateOrder = doMDY;

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // case 1003

            case 1004:  // FOSTER
            {
                if(sourceFile.moveTo("All Obituaries"))
                {
                    while (sourceFile.consecutiveMovesTo(20, "&bull", "a href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        URLext = sourceFile.readNextBetween(QUOTES);
                        pageVariables.webAddress = baseURL + URLext;
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Next group of information may or may not have "maiden" information
                        sourceFile.moveTo("</a>");
                        word = sourceFile.getWord(true, DIVIDER, false);
                        if (word.removeBookEnds(PARENTHESES))
                        {
                            word.dropLeft(8);  // "Maiden: "
                            word.replace(QString(" "), QString(""));
                            pageVariables.maidenName = word;
                            word = sourceFile.getWord(true, DIVIDER, false);
                        }

                        // We should now be positioned at end of "&nbsp"
                        // Read DOB and DOD
                        if (sourceFile.peekAtNext(1) == OQString(";"))
                            sourceFile.forward(1);
                        pageVariables.ucDOBandDOD = sourceFile.getUntil("<");

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }

                    updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
                }
            }
               break;  // 1004

            case 1005:  // CFS
            {
                if ((providerKey > 90000) || (providerKey == 824) || (providerKey == 6192) || (providerKey == 7207))
                {
                    // Old sites - URL and DOD are available

                    switch (providerKey)
                    {
                    case 6192:
                        sourceFile.moveTo("<div class=\"row\">");
                        sourceFile.moveTo("<div class=\"row\">");
                        for (int i = 1; i<= 6; i++)
                        {
                            sourceFile.consecutiveMovesTo(100, "class=\"hzobittile", "href=");

                            record.clear();
                            pageVariables.reset();

                            // Read extension for each deceased, starts with "../"
                            pageVariables.webAddress = baseURL + sourceFile.readNextBetween(QUOTES);
                            pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                            if (sourceFile.consecutiveMovesTo(100, "class=\"obitdate\"", "<small"))
                            {
                                temp = sourceFile.readNextBetween(BRACKETS);
                                if (temp.getLength() == 4)
                                    pageVariables.yod = static_cast<unsigned int>(temp.asNumber());
                                else
                                    pageVariables.ucDOD = temp;
                            }

                            // Process
                            process(record, pageVariables, lang);
                            if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                                records.append(record);
                        }
                        break;

                    case 7207:
                        while (sourceFile.consecutiveMovesTo(50, "class=\"obitlist-title\"", "href="))
                        {
                            record.clear();
                            pageVariables.reset();

                            // Read extension for each deceased, starts with "../"
                            pageVariables.webAddress = baseURL + sourceFile.readNextBetween(QUOTES);
                            pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                            // Process
                            process(record, pageVariables, lang);
                            if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                                records.append(record);
                        }
                        break;

                    default:
                        while (sourceFile.consecutiveMovesTo(100, "class=\"stdA\"", "href="))
                        {
                            record.clear();
                            pageVariables.reset();

                            // Read extension for each deceased, starts with "../"
                            URLext = sourceFile.readNextBetween(QUOTES);
                            URLext.dropLeft(2);
                            pageVariables.webAddress = baseURL + URLext;
                            pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                            // Get DOD
                            if (sourceFile.conditionalMoveTo("td align=\"right\"", "class=\"stdA\"", 2))
                            {
                                if (sourceFile.moveTo(">"))
                                {
                                    sourceFile.backward(1);
                                    pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                                    pageVariables.ucDOD.cleanUpEnds();
                                    // Field starts with day of week, then date
                                    pageVariables.ucDOD.removeLeading(" ");
                                    word = pageVariables.ucDOD.getWord();
                                    pageVariables.ucDOD.dropLeft(word.getLength() + 1);
                                }
                            }

                            // Process
                            process(record, pageVariables, lang);
                            if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                                records.append(record);
                        }
                        break;
                    }

                    updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
                }
                else
                {
                    // New sites - Only URL is readily available

                    if (!pageVariables.paginatedResult && sourceFile.moveTo("Page 1 of "))
                    {
                        pageVariables.paginatedResult = true;
                        OQString pageString = sourceFile.getWord(false, BRACKETS);
                        flowParameters.endingPosition = static_cast<unsigned int>(pageString.asNumber());
                    }
                    sourceFile.beg();

                    while (sourceFile.consecutiveMovesTo(20, "class=\"obitlist-title\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        // Weird use of a different (purchased?) for a handful of providerKeys
                        URLext = sourceFile.readNextBetween(QUOTES);
                        if (URLext.left(4) == PQString("http"))
                            pageVariables.webAddress = URLext;
                        else
                            pageVariables.webAddress = baseURL + URLext;
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        if (sourceFile.conditionalMoveTo("class=\"obit-city", "</p>", 0))
                        {
                            temp = sourceFile.readNextBetween(BRACKETS);
                            pageVariables.pcKey = temp.extractFirstWord();
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }

                    updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
                }
            }   // end case 1005
                break;

            case 1006:  // Frazer
            {
                while (sourceFile.consecutiveMovesTo(200, "class=\"obituary-info\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    URLext = sourceFile.readNextBetween(QUOTES);
                    string = URLext.right(15).getString();
                    if (string == QString("celebrationWall"))
                    {
                        URLext.dropRight(15);  // celebrationWall
                        URLext += QString("obituaryInfo");
                    }
                    pageVariables.webAddress = baseURL + URLext;
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read dates
                    if (sourceFile.conditionalMoveTo("class=\"dates\"", "class=\"link\"", 2))
                    {
                        sourceFile.forward(1);
                        pageVariables.ucDOBandDOD = sourceFile.getUntil("</div>");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1006
                break;

            case 1007:  // Alternatives
            {
                if (flowParameters.initialSetup)
                    sourceFile.moveTo("Search Archives by Name");
                else
                    sourceFile.moveTo("Obituaries & Condolences");

                while (sourceFile.conditionalMoveTo("a href=", "id=\"footer\"", 2))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    URLext = sourceFile.readNextBetween(QUOTES);
                    pageVariables.webAddress = baseURL + URLext;
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read dates
                    if (sourceFile.conditionalMoveTo("class=\"obit-dod\"", "</div> </div>", 2))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
                if (flowParameters.keepDownloading == false)
                    sortRecords(records);

            }   // end case 1007
                break;

            case 1008:  // Funeral Tech
            {
                if (fhSpecialCode == 3)
                {
                    if (sourceFile.moveTo("window.API.domainId ="))
                        followUpRequest.instructions.DomainID = sourceFile.readNextBetween(QUOTES).getString();
                    int index = downloadRequest.instructions.url.findPosition(PQString("page="));
                    PQString pageNumber = downloadRequest.instructions.url.right(downloadRequest.instructions.url.getLength() - index - 5);
                    PQString followUpURLAddress = PQString("https://api.secure.tributecenteronline.com/ClientApi/obituaries/GetObituariesExtended?searchTerm=&sortingColumn=3&dates=0&servingLocationId=0&selectedAreaId=0&pageSize=10&pageNumber=") + pageNumber;
                    //PQString followUpURLAddress = PQString("https://api.secure.tributecenteronline.com/ClientApi/obituaries/GetLocationAreas");
                    followUpRequest.instructions.qUrl.clear();
                    followUpRequest.instructions.url = followUpURLAddress.getString();
                    followUpRequest.outputs.downloadFileName = QString("tempWebPage.htm");
                    preflightRequest.instructions.qUrl.clear();
                    preflightRequest.instructions.url = followUpURLAddress.getString();
                    sourceFile.close();

                    // Preflight Request
                    www->download(preflightRequest);
                    while(www->processingDownload()){};
                    if(www->lastDownloadSuccessful())
                    {
                        www->download(followUpRequest);
                        while(www->processingDownload()){};
                        if(www->lastDownloadSuccessful())
                        {
                            sourceFile.setSourceFile(downloadRequest.outputs.downloadFileName);
                            sourceFile.setGlobalVars(*globals);
                            sourceFile.beg();
                            pageVariables.numericDateOrder = doYMD;
                            QString firstname, lastname, fullname;

                            while (sourceFile.moveTo("<ExtendedObituaryModel>"))
                            {
                                record.clear();
                                pageVariables.reset();
                                firstname.clear();
                                lastname.clear();

                                if (sourceFile.conditionalMoveTo("<BirthDate>", "<ExtendedObituaryModel>", 0))
                                    pageVariables.ucDOB = sourceFile.getUntil("T");
                                if (sourceFile.conditionalMoveTo("<DeathDate>", "<ExtendedObituaryModel>", 0))
                                    pageVariables.ucDOD = sourceFile.getUntil("T");
                                if (sourceFile.conditionalMoveTo("<FirstName>", "<ExtendedObituaryModel>", 0))
                                    firstname = sourceFile.getUntil("<").getString();
                                if (sourceFile.conditionalMoveTo("<Id>", "<ExtendedObituaryModel>", 0))
                                    pageVariables.ID = sourceFile.getUntil("<").getString();
                                if (sourceFile.conditionalMoveTo("<LastName>", "<ExtendedObituaryModel>", 0))
                                    lastname = sourceFile.getUntil("<").getString();

                                fullname = firstname + QString(" ") + lastname;
                                fullname.replace("\"", "");
                                fullname.replace(" ", "-");
                                pageVariables.webAddress = URLbase + PQString("/obituaries/") + PQString(fullname) + PQString("?obId=") + pageVariables.ID + PQString("#");
                                        //https://www.lessardstephens.com/obituaries/Arthur-Mousley?obId=23593669#

                                // Read DOB where provided
                                if (sourceFile.conditionalMoveTo("obitBirthDate", "NEW RECORD TEMPLATE", 0))
                                    pageVariables.ucDOB = sourceFile.readNextBetween(BRACKETS);

                                // Read DOD where provided
                                if (sourceFile.conditionalMoveTo("obitDeathDate", "NEW RECORD TEMPLATE", 0))
                                    pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                                // Process
                                process(record, pageVariables, lang);
                                if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                                    records.append(record);
                            }
                        }
                    }
                }
                else
                {
                    sourceFile.moveTo("full-list-container");

                    while (sourceFile.consecutiveMovesTo(750, "tribute-row", "deceased-name", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        URLext = sourceFile.readNextBetween(QUOTES);
                        pageVariables.webAddress = baseURL + URLext;
                        if (providerKey == 65)
                            pageVariables.webAddress.replace("service-information", "obituary", Qt::CaseSensitive);
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOD
                        if (sourceFile.conditionalMoveTo("deceased-date-of-death", "tribute-buttons", 2))
                        {
                            if (sourceFile.conditionalMoveTo("Date of Death:", "</div>", 2))
                            {
                                if (sourceFile.moveTo("span"), 100)
                                    pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                            }
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1008
                break;

            case 1009:  // WordPress
            {
                // Only ID and URL available (ignore yyyy - yyyy)
                sourceFile.moveTo("We Remember");

                while (sourceFile.moveTo("article id=\"post-"))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.ID = sourceFile.getUntil("\"").getString();

                    // Read extension for each deceased
                    sourceFile.consecutiveMovesTo(750, "entry-title", "a href=");
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1009
                break;

            case 1010:  // FrontRunner
            {
                // Various fundamental differences in approach
                switch(fhSpecialCode)
                {
                case 0:
                case 5:
                    if (flowParameters.initialSetup)
                        sourceFile.moveTo(">Past Services<");
                    else
                    {
                        if (sourceFile.moveTo("Page 1 of "))
                        {
                            OQString temp = sourceFile.getUntil("<");
                            flowParameters.endingPosition = static_cast<unsigned int>(10 * temp.asNumber());
                        }
                        else
                            flowParameters.endingPosition = 0;

                        sourceFile.beg();
                        sourceFile.moveTo(">Current Services<");
                    }

                    //while (sourceFile.moveTo(">Obituary/Death Notice<"))
                    while (sourceFile.moveTo(">Book of Memories<"))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        //sourceFile.moveBackwardTo("a href=");
                        sourceFile.moveTo("a href=");
                        URLext = sourceFile.readNextBetween(QUOTES);
                        pageVariables.webAddress = baseURL + URLext;
                        if (providerKey == 196216)
                            pageVariables.webAddress.replace("obituary.php", "index.php", Qt::CaseSensitive);
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOB where provided
                        if (sourceFile.conditionalMoveTo("obitBirthDate", "NEW RECORD TEMPLATE", 0))
                            pageVariables.ucDOB = sourceFile.readNextBetween(BRACKETS);

                        // Read DOD where provided
                        if (sourceFile.conditionalMoveTo("obitDeathDate", "NEW RECORD TEMPLATE", 0))
                            pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                    }
                    break;

                case 1:
                case 2:
                    // JSON approach
                    bool success;
                    dateFormat = QString("MMMM dd, yyyy");

                    sourceFile.loadValue(QString("success"), success);
                    if (success)
                    {
                        while (sourceFile.loadValue(QString("id"), tempID, false))
                        {
                            record.clear();
                            pageVariables.reset();

                            pageVariables.ID = tempID;
                            sourceFile.loadValue(QString("dod"), pageVariables.currentDOD, dateFormat);
                            sourceFile.loadValue(QString("dob"), pageVariables.currentDOB, dateFormat);
                            sourceFile.loadValue(QString("url"), pageVariables.webAddress);
                            if (pageVariables.webAddress.right(9) != PQString("index.php"))
                                pageVariables.webAddress += PQString("index.php");
                            //if (pageVariables.webAddress.right(13) != PQString("obituary.php"))
                            //    pageVariables.webAddress += PQString("obituary.php");

                            // Address weird issue coming directly from download
                            if (pageVariables.currentDOB == QDate(1969,12,31))
                                pageVariables.currentDOB = QDate(1900,1,1);

                            // Process
                            process(record, pageVariables, lang);
                            if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                                records.append(record);
                        }
                    }
                    break;

                case 3:
                {
                    if (sourceFile.moveTo("window.API.domainId ="))
                        followUpRequest.instructions.DomainID = sourceFile.readNextBetween(QUOTES).getString();
                    int index = downloadRequest.instructions.url.findPosition(PQString("page="));
                    PQString pageNumber = downloadRequest.instructions.url.right(downloadRequest.instructions.url.getLength() - index - 5);
                    PQString followUpURLAddress = PQString("https://api.secure.tributecenteronline.com/ClientApi/obituaries/GetObituariesExtended?searchTerm=&sortingColumn=3&dates=0&servingLocationId=0&selectedAreaId=0&pageSize=10&pageNumber=") + pageNumber;
                    //PQString followUpURLAddress = PQString("https://api.secure.tributecenteronline.com/ClientApi/obituaries/GetLocationAreas");
                    followUpRequest.instructions.qUrl.clear();
                    followUpRequest.instructions.url = followUpURLAddress.getString();
                    followUpRequest.outputs.downloadFileName = QString("tempWebPage.htm");
                    preflightRequest.instructions.qUrl.clear();
                    preflightRequest.instructions.url = followUpURLAddress.getString();
                    sourceFile.close();

                    // Preflight Request
                    www->download(preflightRequest);
                    while(www->processingDownload()){};
                    if(www->lastDownloadSuccessful())
                    {
                        www->download(followUpRequest);
                        while(www->processingDownload()){};
                        if(www->lastDownloadSuccessful())
                        {
                            sourceFile.setSourceFile(downloadRequest.outputs.downloadFileName);
                            sourceFile.setGlobalVars(*globals);
                            sourceFile.beg();
                            pageVariables.numericDateOrder = doYMD;
                            QString firstname, lastname, fullname;

                            while (sourceFile.moveTo("<ExtendedObituaryModel>"))
                            {
                                record.clear();
                                pageVariables.reset();
                                firstname.clear();
                                lastname.clear();

                                if (sourceFile.conditionalMoveTo("<BirthDate>", "<ExtendedObituaryModel>", 0))
                                    pageVariables.ucDOB = sourceFile.getUntil("T");
                                if (sourceFile.conditionalMoveTo("<DeathDate>", "<ExtendedObituaryModel>", 0))
                                    pageVariables.ucDOD = sourceFile.getUntil("T");
                                if (sourceFile.conditionalMoveTo("<FirstName>", "<ExtendedObituaryModel>", 0))
                                    firstname = sourceFile.getUntil("<").getString();
                                if (sourceFile.conditionalMoveTo("<Id>", "<ExtendedObituaryModel>", 0))
                                    pageVariables.ID = sourceFile.getUntil("<").getString();
                                if (sourceFile.conditionalMoveTo("<LastName>", "<ExtendedObituaryModel>", 0))
                                    lastname = sourceFile.getUntil("<").getString();

                                fullname = firstname + QString(" ") + lastname;
                                fullname.replace("\"", "");
                                fullname.replace(" ", "-");
                                pageVariables.webAddress = URLbase + PQString("/obituaries/") + PQString(fullname) + PQString("?obId=") + pageVariables.ID + PQString("#");
                                        //https://www.lessardstephens.com/obituaries/Arthur-Mousley?obId=23593669#

                                // Read DOB where provided
                                if (sourceFile.conditionalMoveTo("obitBirthDate", "NEW RECORD TEMPLATE", 0))
                                    pageVariables.ucDOB = sourceFile.readNextBetween(BRACKETS);

                                // Read DOD where provided
                                if (sourceFile.conditionalMoveTo("obitDeathDate", "NEW RECORD TEMPLATE", 0))
                                    pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                                // Process
                                process(record, pageVariables, lang);
                                if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                                    records.append(record);
                            }
                        }
                    }
                }
                    break;

                case 4:
                    while (sourceFile.consecutiveMovesTo(50, "<li>", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        if (sourceFile.moveTo("class=\"dateofdeath\""))
                            pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                case 6:
                    while (sourceFile.consecutiveMovesTo(1250, "article id=\"post-", "class=\"entry-title\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES);
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOB - DOD
                        if (sourceFile.conditionalMoveTo("class=\"post-content-inner\"", "article id=\"post-", 0))
                        {
                            sourceFile.moveTo("<p>");
                            temp = sourceFile.getUntil("<");
                            temp.beg();
                            if (temp.consecutiveMovesTo(50, " - ", ", ", " "))
                                pageVariables.ucDOBandDOD = temp.left(temp.getPosition());
                            else
                            {
                                temp.beg();
                                if (temp.consecutiveMovesTo(50, " ~ ", ", ", " "))
                                    pageVariables.ucDOBandDOD = temp.left(temp.getPosition());
                            }
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }

                    /*while (sourceFile.consecutiveMovesTo(1250, "article id=\"post-", "class=\"entry-title\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES);
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOP
                        if (sourceFile.conditionalMoveTo("class=\"published\"", "article id=\"post-", 0))
                            pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }

                    sourceFile.beg();
                    if (sourceFile.moveTo("No results match your search."))
                    {
                        flowParameters.currentPosition = 0;
                        flowParameters.monthCurrent++;
                        flowParameters.currentMonth = PQString::MONTHS.at(flowParameters.monthCurrent);
                    }
                    else
                    {
                        sourceFile.beg();
                        if (!sourceFile.consecutiveMovesTo(500, "class=\"pagination clearfix\"", "Older Entries"))
                        {
                            flowParameters.currentPosition = 0;
                            flowParameters.monthCurrent++;
                            flowParameters.currentMonth = PQString::MONTHS.at(flowParameters.monthCurrent);
                        }
                    }*/

                    break;
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1010
                break;

            case 1011:  // FuneralOne
            {
                switch (providerKey)
                {
                case 449:
                    while (sourceFile.consecutiveMovesTo(30, "class=\"obit-image-container\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = baseURL + sourceFile.readNextBetween(QUOTES);
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOD
                        if (sourceFile.conditionalMoveTo("class=\"obit-date-text\"", "obit-image-container", 0))
                             pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }

                    break;

                case 4332:
                    // Old approach
                    // URL and DOD typically available

                    if (flowParameters.initialSetup)
                    {
                        PQString manuallyCreatedFile;

                        switch(providerKey)
                        {
                        case 21661:
                            manuallyCreatedFile = PQString("LyleReeves.txt");
                            break;

                        case 449:
                            manuallyCreatedFile = PQString("C:\\Obits\\Batch Read\\Steckley History.txt");
                            break;

                        case 4332:
                            manuallyCreatedFile = PQString("\\Obits\\Batch Read\\Haycock History.txt");
                            break;
                        }

                        sourceFile.close();
                        sourceFile.clear();
                        sourceFile.setSourceFile(manuallyCreatedFile);
                        sourceFile.setGlobalVars(*globals);
                        temp.setGlobalVars(*globals);
                        temp.setContentLanguage(lang);
                    }

                    sourceFile.beg();

                    while (sourceFile.consecutiveMovesTo(25, "class=\"tribute\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = baseURL + sourceFile.readNextBetween(QUOTES);;
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOD
                        if (sourceFile.conditionalMoveTo("class=\"tribute__dates\"", "View Details", 2))
                             pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                default:
                    // New JSON format
                    bool success;
                    dateFormat = QString("MM/dd/yyyy");

                    sourceFile.beg();
                    sourceFile.loadValue(QString("success"), success);
                    if (success)
                    {
                        while (sourceFile.loadValue(QString("tributeId"), tempID, false))
                        {
                            record.clear();
                            pageVariables.reset();

                            pageVariables.ID = tempID;
                            sourceFile.loadValue(QString("birthDate"), pageVariables.currentDOB, dateFormat);
                            sourceFile.loadValue(QString("deathDate"), pageVariables.currentDOD, dateFormat);
                            sourceFile.loadValue(QString("tributeUrl"), URLtemp);
                            pageVariables.webAddress = URLbase + URLtemp + PQString("/#!/Obituary");
                            sourceFile.loadValue(QString("locationCity"), pageVariables.pcKey);

                            // Process
                            process(record, pageVariables, lang);
                            if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                                records.append(record);
                        }
                    }

                    break;
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1011
                break;

            case 1012:  // WebStorm
            {
                while (sourceFile.consecutiveMovesTo(200, "<!-- info -->", "class=\"obit_name", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES);
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("<p class=", "<!-- mobile name and date -->", 0))
                       pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1012
                break;

            case 1013:  // GasLamp
            {
                while (sourceFile.consecutiveMovesTo(100, "ct-gallery-itemInner-content", "a href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    URLext = sourceFile.readNextBetween(QUOTES);
                    pageVariables.webAddress = URLbase + URLext;
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1013
                break;

            case 1014:  // ClickTributes
            {
                while (sourceFile.consecutiveMovesTo(1000, "ct-obituary-list-view", "ct-obituary-content", "a href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOB and DOD where provided
                    if (sourceFile.conditionalMoveTo("class=\"ct-dates\"", "ct-obituary-foot", 0))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1014
                break;

            case 1015:  // Connelly McKinley
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"entry-title-link\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1015
                break;

            case 1016:  // Arbor Memorial
            {
                switch(providerKey)
                {
                case 29:
                case 33:
                case 47:
                case 98:
                case 99:
                case 100:
                    // Odd ball sites
                    sourceFile.beg();

                    while (!sourceFile.isEOS() && sourceFile.moveTo("http://schema.org/Person"))
                    {
                        record.clear();
                        pageVariables.reset();

                        if (sourceFile.consecutiveMovesTo(500, "class=\"memlistInfo\"", "itemprop=\"url\"", "href="))
                        {
                            pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                            pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                            if (sourceFile.conditionalMoveTo("birthDate", "</table>", 0))
                                pageVariables.ucDOB = sourceFile.readNextBetween(BRACKETS);

                            if (sourceFile.conditionalMoveTo("deathDate", "</table>", 0))
                                pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                default:
                    // Reading from a JSON file
                    OQString tempString;

                    if (flowParameters.initialSetup == true)
                    {
                        if (sourceFile.moveTo("\"numFound\":"))
                        {
                            tempString = sourceFile.getUntil(QString(","));
                            flowParameters.endingPosition = tempString.getString().toUInt();
                        }
                    }

                    while (!sourceFile.isEOS() && sourceFile.moveTo("\"id\":"))
                    {
                        record.clear();
                        pageVariables.reset();

                        pageVariables.ID = sourceFile.readNextBetween(QUOTES).getString();

                        sourceFile.moveTo("\"custom_dt_deceased_birth_date\"");
                        tempString = sourceFile.readNextBetween(QUOTES);
                        index = tempString.findPosition(PQString("T"));
                        if (index == 10)
                            pageVariables.ucDOB = tempString.left(10);

                        sourceFile.moveTo("\"custom_dt_deceased_death_date\"");
                        tempString = sourceFile.readNextBetween(QUOTES);
                        index = tempString.findPosition(PQString("T"));
                        if (index == 10)
                            pageVariables.ucDOD = tempString.left(10);

                        sourceFile.moveTo("\"custom_s_url\":");
                        URLext = sourceFile.readNextBetween(QUOTES);
                        pageVariables.webAddress = baseURL + URLext;

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1016
                break;

            case 1017:  // SiteWyze
            {
                while (sourceFile.moveTo("\'announcement-item"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    sourceFile.moveTo("href=");
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD where provided
                    if (sourceFile.conditionalMoveTo(" - ", "announcement-sep", 0))
                        pageVariables.ucDOD = sourceFile.getUntil("</a>");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1017
                break;

            case 1018:  // Thinking Thru
            {
                // No permament URL exists

                while (sourceFile.moveTo("<!–– hereerereer ––>"))
                {
                    record.clear();
                    pageVariables.reset();

                    if (sourceFile.moveTo("<strong>"))
                    {
                        sourceFile.backward(1);
                        URLext = sourceFile.readNextBetween(BRACKETS);
                        URLext.replace(QString(" "), QString(""));
                        URLext.replace(QString("\""), QString(""));
                        URLext += QString(".dnd"); // Do not download
                    }
                    pageVariables.webAddress = downloadRequest.instructions.url + PQString("DND") + URLext;
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    sourceFile.moveTo("greentext");
                    pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1018
                break;

            case 1019:  // Codesign
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"elementor-post__title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"elementor-post-date\"", "class=\"elementor-post__title\"", 0))
                        pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1019
                break;

            case 1020:  // Shape5
            {
                while (sourceFile.consecutiveMovesTo(300, "page-header", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = baseURL + sourceFile.readNextBetween(QUOTES);
                    string = pageVariables.webAddress.getString();

                    // Pull ID out of address
                    index = string.lastIndexOf("/");
                    int index2 = string.indexOf("-", index);
                    pageVariables.ID = string.mid(index + 1, index2 - (index + 1));

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1020
                break;

            case 1021:  // Tribute Archive
            {
                RECORD checkRecord;

                while (sourceFile.consecutiveMovesTo(500, ".listview-summary .print-only", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = baseURL + sourceFile.readNextBetween(QUOTES);
                    string = pageVariables.webAddress.getString();

                    // Pull ID out of address
                    index = string.lastIndexOf("-");                    
                    if (index >= 0)
                        pageVariables.ID = string.right(string.length() - index - 1);
                    if ((index == -1) || (pageVariables.ID.length() != 10))
                    {
                        index = string.lastIndexOf("/");
                        if (index >= 0)
                            pageVariables.ID = string.right(string.length() - index - 1);
                    }

                    // Check for dates
                    if (sourceFile.conditionalMoveTo("<div class=\"dates\">", "<p class=\"content\">", 0))
                    {
                        unstructuredContent tempUC;
                        tempUC = sourceFile.getUntilEarliestOf("</div>", "<span class=\"category\">", 0);
                        tempUC.removeHTMLtags();
                        pageVariables.ucVariable = tempUC;
                    }

                    if (sourceFile.conditionalMoveTo("<p class=\"content\">", ".listview-summary .print-only", 0))
                    {
                        unstructuredContent tempUC;
                        QList<QDate> dateList;
                        OQString cleanString;
                        unsigned int numDates;
                        tempUC = sourceFile.getUntil("</p>");
                        index = tempUC.findPosition(PQString("<br />"), -1);
                        if (index >= 0)
                            tempUC.dropRight(tempUC.getLength() - static_cast<unsigned int>(index));
                        tempUC.simplify();
                        tempUC.replace(QString("("), QString(" "));
                        tempUC.replace(QString(")"), QString(" "));
                        numDates = tempUC.pullOutDates(language_unknown, dateList, 3, cleanString, false);
                        if ((numDates == 2) || ((numDates == 3) && (dateList[1] == dateList[2])))
                        {
                            if (dateList[0] < dateList[1])
                            {
                                pageVariables.currentDOB = dateList[0];
                                pageVariables.currentDOD = dateList[1];
                            }
                            else
                            {
                                pageVariables.currentDOB = dateList[1];
                                pageVariables.currentDOD = dateList[0];
                            }

                            double age = elapse(pageVariables.currentDOB, pageVariables.currentDOD);
                            if (age < 20)
                            {
                                // Material risk of error
                                pageVariables.currentDOB.setDate(0,0,0);
                                pageVariables.currentDOD.setDate(0,0,0);
                            }
                        }
                    }

                    // Pick off publisher and publish date
                    if (sourceFile.conditionalMoveTo(QString("<div class=\"published\">"), QString(".listview-summary .print-only"), 0))
                    {
                        if (sourceFile.moveTo("</span>", 100))
                        {
                            unstructuredContent tempUC;
                            OQString tempString;
                            bool success;
                            tempUC = sourceFile.getUntil("</div");
                            tempUC.simplify();
                            index = tempUC.findPosition(PQString(" on "), 1);
                            tempString = tempUC.left(index);
                            if (globals->runStatus >= 200)
                            {
                                success = getIDandKey(globals, tempString.getString(), pageVariables.providerID, pageVariables.providerKey);
                                if (!success)
                                {
                                    pageVariables.providerID = 1021;
                                    OQStream tempStream = tempString;
                                    OQString firstWord = tempStream.getWord();
                                    if (firstWord == OQString("Cochrane"))
                                        pageVariables.providerKey = 55;
                                    else
                                        pageVariables.providerKey = 9999;
                                }
                            }
                            tempUC.dropLeft(index + 4);
                            pageVariables.ucPubDate = tempUC;
                        }
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((record.pubdate.year() > 1900) && (record.pubdate.year() < 2021))
                        pageVariables.includeRecord = false;
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
                if (records.size() > 0)
                {
                    checkRecord = records.last();
                    if ((record.pubdate.year() > 1900) && (record.pubdate.year() < 2020))
                        flowParameters.keepDownloading = false;
                }

            }   // end case 1021
                break;

            case 1022:  // YAS
            {
                while (sourceFile.consecutiveMovesTo(500, "article id=\"post", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Check for dates
                    if (sourceFile.conditionalMoveTo(QString("class=\"date\""), QString("article id=\"post"), 0))
                    {
                        unstructuredContent tempUC;
                        QList<QDate> dateList;
                        OQString cleanString;
                        unsigned int numDates;
                        tempUC = sourceFile.readNextBetween(BRACKETS);
                        tempUC.simplify();
                        numDates = tempUC.pullOutDates(language_unknown, dateList, 3, cleanString, false);
                        if (numDates == 1)
                            pageVariables.currentDOD = dateList[0];
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1022
                break;

            case 1023:  // WFFH
            {
                // JSON approach
                QString tempID, firstName, lastName, style;
                dateFormat = QString("M/d/yyyy");

                while (sourceFile.loadValue(QString("PersonId"), tempID, false))
                {
                    record.clear();
                    pageVariables.reset();
                    firstName.clear();
                    lastName.clear();
                    style.clear();

                    sourceFile.loadValue(QString("FirstName"), firstName);
                    sourceFile.loadValue(QString("LastName"), lastName);
                    sourceFile.loadValue(QString("Template"), style);
                    sourceFile.loadValue(QString("Deceased"), pageVariables.currentDOD, dateFormat);
                    sourceFile.loadValue(QString("Born"), pageVariables.currentDOB, dateFormat);
                    sourceFile.loadValue(QString("SafeFirstName"), firstName);
                    sourceFile.loadValue(QString("SafeLastName"), lastName);

                    pageVariables.ID = tempID;
                    pageVariables.webAddress = URLbase + QString("/lifestory/") + style + QString("/home.html?");
                    pageVariables.webAddress += QString("Year=") + QString::number(pageVariables.currentDOD.year()) + QString("&Month=") + QString::number(pageVariables.currentDOD.month()) + QString("&Day=") + QString::number(pageVariables.currentDOD.day());
                    pageVariables.webAddress += QString("&FirstName=") + firstName + QString("&LastName=") + lastName;

                    //http://coventryfuneralservices.ca/lifestory/garden02/home.html?Year=2020&Month=5&Day=31&FirstName=Sofie&LastName=Verklan

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1023
                break;

            case 1024:  // FHW Solutions
            {
                switch(providerKey)
                {
                case 1:
                    // URL, DOB and DOB typically available

                    while (sourceFile.consecutiveMovesTo(200, "<!-- info -->", "class=\"obit_name", "href="))
                    {
                        record.clear();
                        pageVariables.reset();
                        pageVariables.numericDateOrder = doDMY;

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES);
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        if (sourceFile.conditionalMoveTo("<p class=", "<!-- mobile name and date -->", 0))
                        {
                            sourceFile.moveTo(">");
                            temp = sourceFile.getUntil("</div>");
                            temp.removeHTMLtags();
                            pageVariables.ucDOBandDOD = temp.getString();
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                case 53:
                    // JSON approach
                    QString webAddress1, webAddress2, lastName;
                    dateFormat = QString("M/d/yyyy");

                    while (sourceFile.loadValue(QString("OBIT_TEXT"), lastName, false))
                    {
                        record.clear();
                        pageVariables.reset();
                        webAddress1.clear();
                        webAddress2.clear();

                        sourceFile.loadValue(QString("ID"), pageVariables.ID);
                        //sourceFile.loadValue(QString("CLIENT_SHARE_URL"), webAddress1);
                        sourceFile.loadValue(QString("BORN_DATE"), pageVariables.currentDOB, dateFormat);
                        sourceFile.loadValue(QString("OBIT_URL_REWRITE"), webAddress2);
                        sourceFile.loadValue(QString("DIED_DATE"), pageVariables.currentDOD, dateFormat);
                        pageVariables.webAddress = PQString("https://www.williamsfuneralhomeltd.com/obituaries?name=") + webAddress2;
                        //pageVariables.webAddress = webAddress1 + webAddress2 + QString("-obituary");

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1024
                break;

            case 1025:  // Crew
            {
                while (sourceFile.consecutiveMovesTo(200, "class=\"obit-item\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1025
                break;

            case 1026:  // Jelly
            {
                while (sourceFile.consecutiveMovesTo(100, "post-title", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    if (pageVariables.webAddress != PQString("{{ url }}"))
                    {
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Check for dates
                        if (sourceFile.conditionalMoveTo(QString("<p>"), QString("</div>"), 0))
                            pageVariables.ucDOBandDOD = sourceFile.getUntil("</p>");

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1026 Jelly
                break;

            case 1027:  // Nirvana
            {
                while (sourceFile.consecutiveMovesTo(1000, "<tr>", "<td><ul>", "<li><a href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Check for dates
                    if (sourceFile.conditionalMoveTo(QString("</td>-->"), QString("</tr>"), 0))
                    {
                        sourceFile.moveTo("<td>");
                        pageVariables.ucDOB = sourceFile.getUntil("</td>");
                        sourceFile.moveTo("<td>");
                        pageVariables.ucDOD = sourceFile.getUntil("</td>");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1027 Nirvana
                break;

            case 1028:  // Bergetti
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"post_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1028 Bergetti
                break;

            case 1029:  // Pacific Byte
            {
                while (sourceFile.consecutiveMovesTo(1000, "ct-obituary-list-view", "ct-obituary-content", "a href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    // https://nunespottinger.clicktributes.net/obituary/a-barbara-craig-fenwick-wilson/
                    tempID = sourceFile.readNextBetween(QUOTES).getString();
                    int index = tempID.indexOf(QString("profile="));
                    pageVariables.ID = tempID.mid(index + 8, tempID.length() - (index + 8));
                    pageVariables.webAddress = baseURL + PQString("/obituary/") + pageVariables.ID + PQString("/");

                    // Read DOB and DOD where provided
                    if (sourceFile.conditionalMoveTo("class=\"ct-dates\"", "ct-obituary-foot", 0))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1029
                break;

            case 1030:  // ROI
            {
                while (sourceFile.consecutiveMovesTo(20, "entry-title", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1030 ROI
                break;

            case 1031:  // Vernon
            {
                while (sourceFile.consecutiveMovesTo(250, "class=\"post-content-div\"", "class=\"title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Pull out dates
                    if (sourceFile.conditionalMoveTo("class=\"bd-post-content\">", "bd-post-footer", 0))
                    {
                        temp = sourceFile.getUntil(" at ", 100);
                        temp.replaceHTMLentities();
                        temp.simplify();
                        pageVariables.ucDOBandDOD = temp;
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1031 Vernon
                break;

            case 1032:  // Gustin
            {
                while (sourceFile.consecutiveMovesTo(200, "class=\"wpfh_obit\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Pull out DOD
                    if (sourceFile.conditionalMoveTo("wpfh_obit_date", "class=\"wpfh_obit\"", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1032 Gustin
                break;

            case 1033:  // Ashlean
            {
                while (sourceFile.consecutiveMovesTo(200, "data-widget_type=\\\"ae-post-title.default\\\"", "href=\\"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    tempID = sourceFile.readNextBetween(QUOTES).getString();
                    tempID.replace("\\/", "/");
                    tempID.chop(1);
                    pageVariables.webAddress = tempID;
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1033 Ashlean
                break;

            case 1034:  // Halifax
            {
                while (sourceFile.consecutiveMovesTo(20, "entry-title", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1034 Halifax
                break;

            case 1035:  // Specialty
            {
                int style = 0;
                if (sourceFile.moveTo("funeralListDeceased"))
                    style = 1;
                sourceFile.beg();
                if (sourceFile.moveTo("table class=\"obitList\""))
                    style = 2;
                if (style != 2)
                {
                    sourceFile.beg();
                    if (sourceFile.moveTo("class=\"obitList\""))
                        style = 3;
                }

                if (URLbase.right(1) != PQString("/"))
                    URLbase += PQString("/");

                // URL, DOB and DOB typically available
                sourceFile.beg();

                switch(style)
                {
                case 0:
                    while (sourceFile.moveTo("id=\"obit_"))
                    {
                        record.clear();
                        pageVariables.reset();

                        sourceFile.moveTo("href=", 150);
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        if (pageVariables.webAddress.left(1) == OQString("/"))
                            pageVariables.webAddress = baseURL + pageVariables.webAddress;
                        if (pageVariables.webAddress.right(1) == OQString("/"))
                            pageVariables.webAddress.dropRight(1);
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read dates
                        if (sourceFile.conditionalMoveTo("class=\"obit_dob\"", "obit_community", 0))
                            pageVariables.ucDOB = sourceFile.readNextBetween(BRACKETS);
                        if (sourceFile.conditionalMoveTo("class=\"obit_dod\"", "obit_community", 0))
                            pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                case 1:
                    while (sourceFile.consecutiveMovesTo(100, "funeralListDeceased", "id=\"obit_"))
                    {
                        record.clear();
                        pageVariables.reset();

                        sourceFile.moveTo("href=", 150);
                        temp = sourceFile.readNextBetween(QUOTES);
                        if (temp.left(4) == PQString("http"))
                        {
                            index = temp.findPosition(":");
                            temp.dropLeft(index + 1);
                            temp = HTTP + temp;
                            pageVariables.webAddress = temp.getString();
                        }
                        else
                        {
                            if (temp.left(1) == PQString("/"))
                                temp.dropLeft(1);
                            if (temp.left(11) == PQString("obituaries/"))
                                temp.dropLeft(11);
                            pageVariables.webAddress = URLbase + temp;
                        }
                        pageVariables.ID = OQString(pageVariables.webAddress.getString()).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read dates
                        if (sourceFile.conditionalMoveTo("id=\"details_dob\"", "details_community", 0))
                            pageVariables.ucDOB = sourceFile.readNextBetween(BRACKETS);
                        if (sourceFile.conditionalMoveTo("id=\"details_dod\"", "details_community", 0))
                            pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                case 2:
                    sourceFile.moveTo("table class=\"obitList\"");
                    while (sourceFile.consecutiveMovesTo(100, "<tr>", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        temp = sourceFile.readNextBetween(QUOTES);
                        if (temp.left(4) == PQString("http"))
                        {
                            index = temp.findPosition(":");
                            temp.dropLeft(index + 1);
                            temp = HTTP + temp;
                            pageVariables.webAddress = temp.getString();
                        }
                        else
                        {
                            pageVariables.webAddress = URLbase;
                            if (temp.left(1) == PQString("/"))
                                temp.dropLeft(1);
                            pageVariables.webAddress += temp;
                        }
                        pageVariables.ID = OQString(pageVariables.webAddress.getString()).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read dates
                        if (sourceFile.conditionalMoveTo("class=\"dob\"", "</tr>", 0))
                        {
                            sourceFile.conditionalMoveTo("span class=\"dob\"", "</tr>", 0);
                            pageVariables.ucDOB = sourceFile.readNextBetween(BRACKETS);
                        }
                        if (sourceFile.conditionalMoveTo("class=\"dod\"", "</tr>", 0))
                        {
                            sourceFile.conditionalMoveTo("span class=\"dob\"", "</tr>", 0);
                            pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                case 3:
                    sourceFile.moveTo("class=\"obitList\"");
                    while (sourceFile.conditionalMoveTo("href=", "<!-- END CONTENT -->", 0))
                    {
                        record.clear();
                        pageVariables.reset();

                        temp = sourceFile.readNextBetween(QUOTES);
                        if (temp.left(4) == PQString("http"))
                        {
                            index = temp.findPosition(":");
                            temp.dropLeft(index + 1);
                            temp = HTTP + temp;
                            pageVariables.webAddress = temp.getString();
                        }
                        else
                        {
                            pageVariables.webAddress = URLbase;
                            if (temp.left(1) == PQString("/"))
                                temp.dropLeft(1);
                            if (temp.left(11) == PQString("obituaries/"))
                                temp.dropLeft(11);
                            pageVariables.webAddress += temp;
                        }
                        pageVariables.ID = OQString(pageVariables.webAddress.getString()).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read dates
                        if (sourceFile.conditionalMoveTo("class=\"obit_dob\"", "</a>", 0))
                            pageVariables.ucDOB = sourceFile.readNextBetween(BRACKETS);
                        if (sourceFile.conditionalMoveTo("class=\"obit_dod\"", "</a>", 0))
                            pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1035
                break;

            case 1036:  // ReneBabin
            {
                while (sourceFile.consecutiveMovesTo(30, "class=\"post_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1036 ReneBabin
                break;

            case 1037:  // Smiths
            {
                while (sourceFile.consecutiveMovesTo(200, "<tr>", "javascript:pop("))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1037 Smiths
                break;

            case 1038:  // SeaBreeze
            {
                int index1, index2;
                index1 = URLaddress.findPosition(".com");
                temp = URLaddress.left(index1 + 4) + PQString("/mobi/");
                index1 = URLaddress.findPosition("=");
                index1++;
                index2 = URLaddress.findPosition("&", 1, index1);
                temp += URLaddress.middle(index1, index2 - index1);
                if (temp.left(4) != PQString("http"))
                    temp = URLbase + temp;

                while (sourceFile.moveTo("FORM name=\"notices"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.ID = sourceFile.getUntil("\"").getString();
                    pageVariables.POSTformRequest = QString("message=") + pageVariables.ID;
                    pageVariables.webAddress = temp.getString();

                    // https://peifuneralcoops.com/Central_Queens.php
                    // https://peifuneralcoops.com/mobi/Central_Queens.php || message=5165
                    // https://peifuneralcoops.com/setpage.php?page=Central_Queens.php&set=Notices&type=recent

                    // Pull out DOD
                    if (sourceFile.conditionalMoveTo("return formSubmit", "</tr>", 0))
                    {
                        sourceFile.moveTo("class=\"noticelist\"");
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1038 SeaBreeze
                break;

            case 1039:  // RedSands
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"entry-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Pull out DOD
                    if (sourceFile.moveTo(" &#8211; ", 100))
                        pageVariables.ucDOD = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1039 RedSands
                break;

            case 1040:  // AdGraphics
            {
                // Limited number of obits, all on same page
                sourceFile.moveTo("images/obituary_notices_band.jpg");

                while (sourceFile.moveTo("images/horz_grey.jpg"))
                {
                    record.clear();
                    pageVariables.reset();

                    sourceFile.backward(100);
                    sourceFile.moveBackwardTo("<img src=\"images/");

                    // Read extension for each deceased
                    pageVariables.ID = sourceFile.getUntil(".").getString();
                    pageVariables.webAddress = downloadRequest.instructions.url;
                    sourceFile.moveTo("images/horz_grey.jpg");
                    sourceFile.forward(100);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1040 AdGraphics
                break;

            case 1041:  // Websites
            {
                while (sourceFile.consecutiveMovesTo(50, "<li class=\"rpwe-li rpwe-clearfix\">", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Pull out DOD
                    if (sourceFile.consecutiveMovesTo(100, "class=\"rpwe-time published\"", ">"))
                        pageVariables.ucDOD = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1041 Websites
                break;

            case 1042:  // MPG
            {
                while (sourceFile.moveTo("onclick=\"RedirectToServiceDetails(&#39;"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.getUntil("&#39;");
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read YOB and YOD
                    if (sourceFile.consecutiveMovesTo(100, "class=\"fs_name\"", "</h", "<h"))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1042 MPG
                break;

            case 1043:  // MediaHosts
            {
                while (sourceFile.moveTo("class=\"card-title\""))
                {
                    record.clear();
                    pageVariables.reset();

                    // Pull out DOD
                    if (sourceFile.conditionalMoveTo("class=\"card-text lead\"", "href", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Read extension for each deceased
                    sourceFile.consecutiveMovesTo(200, "card-text", "href=");
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1043 MediaHosts
                break;

            case 1044:  // Dragonfly
            {
                while (sourceFile.moveTo("class=\"deteofdeath\""))
                {
                    record.clear();
                    pageVariables.reset();

                    // DOD
                    pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Read extension for each deceased
                    sourceFile.moveTo("href=");
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES);
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1044 Dragonfly
                break;

            case 1045:  // MyFavourite
            {
                while (sourceFile.consecutiveMovesTo(150, "class=\"obit-body\"", "<a href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.getUntil(">");
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    sourceFile.moveTo("<!-- .entry-header -->");
                    if (sourceFile.conditionalMoveTo(", ", "<", 0))
                        pageVariables.ucDOD = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1045 MyFavourite
                break;

            case 1046:  // Coop
            {
                sourceFile.moveTo("id=\"liste-avis\"");

                while (sourceFile.consecutiveMovesTo(25, "<li>", "href=\"/en/death-notices/"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    sourceFile.backward(19);
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.consecutiveMovesTo(100, "class=\"notice-name\"", "</h"))
                    {
                        sourceFile.moveTo("<br>");
                        pageVariables.ucDOD = sourceFile.getUntil("<");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1046 Coop
                break;

            case 1047:  // EverTech
            {
                while (sourceFile.moveTo(">View Memorial<"))
                {
                    record.clear();
                    pageVariables.reset();

                    sourceFile.moveBackwardTo("href=");

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read YOB and YOD
                    if (sourceFile.moveBackwardTo("widget-sub-title dfd-sub-title", 10000))
                    {
                        sourceFile.moveTo(">");
                        sourceFile.backward(1);
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);
                    }
                    else
                        sourceFile.moveTo("go to end of file");

                    sourceFile.moveTo(">View Memorial<");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1047 EverTech
                break;

            case 1048:  // Maclean
            {
                while (sourceFile.consecutiveMovesTo(100, "mailto:?", "body="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.getUntil("\"");
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOB and DOD
                    if (sourceFile.conditionalMoveTo("class=\"lifetime\"", "<!-- Reveal Modals end -->", 0))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1048 Maclean
                break;

            case 1049:  // MCG
            {
                if (providerKey == 3)
                {
                    while (sourceFile.consecutiveMovesTo(100, "_hlListing", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOD
                        if (sourceFile.conditionalMoveTo("Pass Date: ", "_hlListing", 0))
                        {
                            sourceFile.moveTo(" ");
                            pageVariables.ucDOD = sourceFile.getUntil("<");
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    while (sourceFile.consecutiveMovesTo(100, "class=\"cwp_list_item\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOD
                        if (sourceFile.conditionalMoveTo("Passing Date:", "cwp_list_item", 0))
                        {
                            sourceFile.moveTo("<br");
                            pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1049 MCG
                break;

            case 1051:  // WebSolutions
            {
                sourceFile.moveTo("id=\"small-screen\"");

                while (sourceFile.moveTo("class=\"card-body\""))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    sourceFile.moveBackwardTo("href=");
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    if (sourceFile.conditionalMoveTo("Date of Death:", ">View Obituary<", 1))
                    {
                        sourceFile.moveTo("</b");
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1051 WebSolutions
                break;

            case 1052:  // Citrus (see also 2152 MLBW)
            {
                //sourceFile.JSONsimplify();
                //sourceFile.unQuoteHTML();

                while (sourceFile.moveTo("jet-listing-dynamic-link__label"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    sourceFile.moveBackwardTo("href=");
                    temp = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    temp.JSONsimplify();
                    temp.unescapeJSONformat();
                    temp.removeEnding("\\");
                    temp.unQuoteHTML();
                    temp.replace("%3a",":", Qt::CaseInsensitive);
                    pageVariables.webAddress = temp;
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read YOB and YOD
                    if (sourceFile.moveTo("jet-listing-dynamic-field__content"))
                    {
                        temp = sourceFile.readNextBetween(BRACKETS);
                        temp.replace(" - ", "-");
                        temp.cleanUpEnds();
                        if (temp.getLength() == 9)
                        {
                            pageVariables.yob = temp.left(4).getString().toUInt();
                            pageVariables.yod = temp.right(4).getString().toUInt();
                        }
                        else
                        {
                            if (temp.getLength() == 4)
                            {
                                numTemp = temp.getString().toUInt();
                                if (numTemp < static_cast<unsigned int>(globals->today.year() - 1))
                                    pageVariables.yob = numTemp;
                                else
                                    pageVariables.yod = numTemp;
                            }
                        }
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                URLaddressTemplate = URLbase;
                URLaddressTemplate += PQString("&action=jet_engine_ajax&handler=listing_load_more&query%5Bpost_status%5D=publish&query%5Bfound_posts%5D=1281&query%5Bmax_num_pages%5D=129&query%5Bpost_type%5D=necrologies");
                URLaddressTemplate += PQString("&query%5Borderby%5D=&query%5Border%5D=DESC&query%5Bpaged%5D=0&query%5Bposts_per_page%5D=10&widget_settings%5Blisitng_id%5D=1909&widget_settings%5Bposts_num%5D=10");
                URLaddressTemplate += PQString("&widget_settings%5Bcolumns%5D=1&widget_settings%5Bcolumns_tablet%5D=1&widget_settings%5Bcolumns_mobile%5D=1&widget_settings%5Bis_archive_template%5D=yes");
                URLaddressTemplate += PQString("&widget_settings%5Bpost_status%5D%5B%5D=publish&widget_settings%5Buse_random_posts_num%5D=&widget_settings%5Bmax_posts_num%5D=9&widget_settings%5Bnot_found_message%5D=No+data+was+found");
                URLaddressTemplate += PQString("&widget_settings%5Bis_masonry%5D=false&widget_settings%5Bequal_columns_height%5D=&widget_settings%5Buse_load_more%5D=yes&widget_settings%5Bload_more_id%5D=");
                URLaddressTemplate += PQString("&widget_settings%5Bload_more_type%5D=scroll&widget_settings%5Buse_custom_post_types%5D=yes&widget_settings%5Bcustom_post_types%5D%5B%5D=necrologies");
                URLaddressTemplate += PQString("&widget_settings%5Bhide_widget_if%5D=empty_query&widget_settings%5Bcarousel_enabled%5D=&widget_settings%5Bslides_to_scroll%5D=1&widget_settings%5Barrows%5D=true");
                URLaddressTemplate += PQString("&widget_settings%5Barrow_icon%5D=fa+fa-angle-left&widget_settings%5Bdots%5D=&widget_settings%5Bautoplay%5D=true&widget_settings%5Bautoplay_speed%5D=5000&widget_settings%5Binfinite%5D=true");
                URLaddressTemplate += PQString("&widget_settings%5Bcenter_mode%5D=&widget_settings%5Beffect%5D=slide&widget_settings%5Bspeed%5D=500&widget_settings%5Binject_alternative_items%5D=&widget_settings%5Bscroll_slider_enabled%5D=");
                URLaddressTemplate += PQString("&widget_settings%5Bscroll_slider_on%5D%5B%5D=desktop&widget_settings%5Bscroll_slider_on%5D%5B%5D=tablet&widget_settings%5Bscroll_slider_on%5D%5B%5D=mobile&widget_settings%5Bcustom_query%5D=false");
                URLaddressTemplate += PQString("&widget_settings%5Bcustom_query_id%5D=&widget_settings%5B_element_id%5D=&page_settings%5Bpost_id%5D=false&page_settings%5Bqueried_id%5D=false&page_settings%5Belement_id%5D=false");
                URLaddressTemplate += PQString("&page_settings%5Bpage%5D=") + paramPlaceholder + PQString("&listing_type=false&isEditMode=false");

                createURLaddress(downloadRequest, URLaddressTemplate, URLparams);

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1052 Citrus
                break;

            case 1054:  // TNours
            {
                while (sourceFile.consecutiveMovesTo(35, "class=\"egaliser\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    if (sourceFile.conditionalMoveTo("class=\"button button-date\">", "class=\"egaliser\"", 0))
                    {
                        pageVariables.ucDOD = sourceFile.getUntil("</a>");
                        pageVariables.ucDOD.replace("<br>", " ", Qt::CaseSensitive);
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break;  // 1054 TNours

            case 1056:  // SandFire
            {
                while (sourceFile.consecutiveMovesTo(250, "class=\"obituary-list-item format-standard\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOB and DOD
                    if (sourceFile.conditionalMoveTo("class=\"meta-data\"", "obituary-list-item format-standard", 0))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1056 SandFire
                break;

            case 1057:  // Carve
            {
                // New JSON approach
                sourceFile.JSONsimplify();

                while (sourceFile.consecutiveMovesTo(150, "data-post-id=", "data-url="))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                /*
                // Issue with many obits never including YOD, just mmmm dd

                while (sourceFile.consecutiveMovesTo(50, "class=\"obituary_list_item\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Need to estimate YOD
                    numTemp = pageVariables.ID.toUInt();
                    if (numTemp <= 2136)
                        pageVariables.yod = 2019;
                    else
                    {
                        if (numTemp <= 2255)
                            pageVariables.yod = 2020;
                        else
                        {
                            if ((globals->today.month() >= 2) || (globals->today.day() >= 7))
                                pageVariables.yod = globals->today.year();
                            else
                                pageVariables.yod = globals->today.year() - 1;
                        }
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }*/

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1057 Carve
                break;

            case 1058:  // SRS
            {
                while (sourceFile.moveTo("obituary-info"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    if (sourceFile.moveTo("href="))
                    {
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    }

                    // Read available dates
                    if (sourceFile.conditionalMoveTo("class=\"dates\"", "class=\"description\""))
                        pageVariables.ucDOBandDOD = sourceFile.getUntil("</div>");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case SRS
                break;

            case 1059:  // BrandAgent
            {
                while (sourceFile.consecutiveMovesTo(100, "ptb_post_title ptb_entry_title", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    if (sourceFile.consecutiveMovesTo(200, "ptb_extra_post_date ptb_extra_post_meta", "datetime="))
                        pageVariables.ucDOD = sourceFile.readNextBetween(QUOTES);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1059 BrandAgent
                break;

            case 1060:  // EP Media
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"obituaries_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    index = pageVariables.ID.indexOf("-nee-");
                    if (index > 0)
                        pageVariables.maidenName = pageVariables.ID.right(pageVariables.ID.length() - index - 5);

                    // Read PubDate
                    if (sourceFile.conditionalMoveTo("class=\"post_info_date\"", "post_descr clearfix", 0))
                        pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    // Read DOB and DOD
                    if (sourceFile.conditionalMoveTo("class=\"post_descr clearfix\"", "<!-- /.post_content -->", 0))
                    {
                        sourceFile.moveTo("<p>");
                        pageVariables.ucDOBandDOD = sourceFile.getUntil("...");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1060 EP Media
                break;

            case 1061:  // Linkhouse
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"death-notice-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    if (sourceFile.conditionalMoveTo("class=\"post-date\"", "</section>", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1061 Linkhouse
                break;

            case 1063:  // LinkWeb
            {
                sourceFile.moveTo("</article>-->");

                while (sourceFile.consecutiveMovesTo(100, "class=\"uk-panel-title\"", "<p>", ", "))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read DOD
                    pageVariables.ucDOD = sourceFile.getUntil(" of ");

                    // Read extension for each deceased
                    sourceFile.moveTo("href=");
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1063 LinkWeb
                break;

            case 1064:  //  Josh Pascoe
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"dottedLine\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1064 Josh Pascoe
                break;

            case 1065:  // Chad Simpson
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"px-6 py-4 mx-auto\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOB and DOD
                    sourceFile.moveBackwardTo("</div>", 400);
                    sourceFile.backward(7);
                    sourceFile.moveBackwardTo("</div>", 400);
                    if (sourceFile.conditionalMoveTo("class=\"card-dates\">", "px-6 py-4 mx-auto", 0))
                    {
                        pageVariables.ucDOBandDOD = sourceFile.getUntil("</span>");
                        pageVariables.ucDOBandDOD.replaceHTMLentities();
                        pageVariables.ucDOBandDOD.removeHTMLtags(false);
                    }
                    sourceFile.moveTo("class=\"cardX");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1065 Chad Simpson
                break;

            case 1066:  //  Marketing Images
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"details\"", "<h", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1066 Marketing Images
                break;

            case 1067:  //  Esite
            {
                switch(providerKey)
                {
                case 1:
                    while (sourceFile.consecutiveMovesTo(75, "<div class=\"row\">", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                case 2:
                    while (sourceFile.moveTo("class=\"deteofdeath\""))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read DOD
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                        // Read extension for each deceased
                        sourceFile.moveTo("href=\"", 150);
                        pageVariables.webAddress = URLbase + sourceFile.getUntil("\">");
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1067 Esite
                break;

            case 1068:  // SquareSpace
            {
                while (sourceFile.consecutiveMovesTo(250, "summary-excerpt", "summary-excerpt-only", "p class="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read DOB and DOD
                    pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Read extension for each deceased
                    sourceFile.moveTo("href=");
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1068 SquareSpace
                break;

            case 1069:  // Eggs
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"posttitle\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1069 Eggs
                break;

            case 1070:  //  MFH
            {
                while (sourceFile.consecutiveMovesTo(75, "class=\"tribute-detail\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"date\"", "class=\"item tribute\"", 0))
                    {
                        sourceFile.moveTo(", ");
                        pageVariables.ucPubDate = sourceFile.getUntil("<");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1070 MFH
                break;

            case 1071:  // Oneil
            {
                while (sourceFile.consecutiveMovesTo(250, "class=\"obitName\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    temp = sourceFile.readNextBetween(QUOTES);
                    temp.replaceHTMLentities();
                    pageVariables.webAddress = temp.getString();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read YOB and YOD
                    QString datesRemoved, justDates;
                    temp = sourceFile.readNextBetween(BRACKETS);
                    temp.splitComponents(datesRemoved, justDates);
                    pageVariables.ucDOBandDOD = justDates;

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1071 Oneil
                break;

            case 1073:  // Back2Front
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"nameOfDeceased\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read YOB and YOD
                    if (sourceFile.consecutiveMovesTo(100, "app.tbl.name.date_of_birth", ">", " "))
                        pageVariables.yob = sourceFile.getUntil("<").asNumber();
                    if (sourceFile.consecutiveMovesTo(100, "app.tbl.name.date_of_death", ">", " "))
                        pageVariables.yod = sourceFile.getUntil("<").asNumber();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1073 Back2Front
                break;

            case 1075:  // InView
            {
                while (sourceFile.consecutiveMovesTo(100, "class='slide-entry-title entry-title", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1075 InView
                break;

            case 1077:  // RKD
            {
                while (sourceFile.consecutiveMovesTo(75, "class=\\\"elementor-heading-title elementor-size-default\\\"", "href=\\"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.webAddress.unescapeJSONformat();
                    pageVariables.webAddress.dropRight(1);
                    pageVariables.webAddress.replace("http:", "https:", Qt::CaseSensitive);
                    pageVariables.webAddress.replace("%3a", ":", Qt::CaseInsensitive);
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    if (sourceFile.conditionalMoveTo("Date of Death:<\\/strong", "<\\/section>", 2))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1077 RKD
                break;

            case 1078:  // SDP
            {
                if (sourceFile.moveTo(">Here are the recent obituaries.<"))
                {
                    while (sourceFile.consecutiveMovesTo(150, "class=\"fltlft content20", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        if (sourceFile.conditionalMoveTo("<br /", "class=\"fltlft content20", 0))
                            pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    sourceFile.beg();
                    if (sourceFile.moveTo(">Here are the obituaries archives.<"))
                    {
                        while (sourceFile.consecutiveMovesTo(150, "class=\"fltlft content80 archivesListe\"", "href="))
                        {
                            record.clear();
                            pageVariables.reset();

                            // Read extension for each deceased
                            pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                            pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                            if (sourceFile.conditionalMoveTo("|", "</div>", 0))
                                pageVariables.ucDOD = sourceFile.getUntil("<");

                            // Process
                            process(record, pageVariables, lang);
                            if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                                records.append(record);
                        }
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1078 SDP
                break;

            case 1079:  // Globalia
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"more-avis\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1079 Globalia
                break;

            case 1080:  // Vortex
            {
                switch(providerKey)
                {
                case 1:
                    //  URL only
                    sourceFile.moveTo("id=\"lettreA\"");

                    while (sourceFile.conditionalMoveTo("<li>", "</main>", 2))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        sourceFile.moveTo("<a href=");
                        pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                case 3:
                    while (sourceFile.consecutiveMovesTo(100, "class=\"obituary-link overlay-link\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        pageVariables.currentDOD = pageVariables.queryTargetDate;

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                default:
                    //  URL and DOD
                    sourceFile.moveTo("class=\"listeAvis ibfix");

                    int i = 0;
                    while ((sourceFile.consecutiveMovesTo(20, "<li>", "<a href=\"")) && (i < 12))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + PQString("/") + sourceFile.getUntil("?");
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // DOD
                        if (sourceFile.conditionalMoveTo("datetime=", "</li>", 0))
                            pageVariables.ucDOD = sourceFile.readNextBetween(QUOTES);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);

                        i++;
                    }
                    break;

                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1080 Vortex
                break;

            case 1081:  // Elegant
            {
                while (sourceFile.consecutiveMovesTo(75, "class=\"post_content clearfix\"", "class=\"post_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1081 Elegant
                break;

            case 1082:  // YellowPages
            {
                switch(providerKey)
                {
                case 1:
                    // Same URL applies to temporary obit (one month only)
                    while (sourceFile.moveTo("<table id=\"notice"))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = downloadRequest.instructions.url;
                        pageVariables.ID = sourceFile.getUntil("\"").getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                case 2:
                    // Same URL applies to temporary obit (one month only)
                    while (sourceFile.consecutiveMovesTo(100, "class=\"page-header\"", "itemprop=\"name\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read YOB and YOD
                        if (sourceFile.conditionalMoveTo(" - ", "</div>"))
                        {
                            temp = sourceFile.getUntil("<");
                            temp.simplify();
                            temp.cleanUpEnds();
                            numTemp = temp.left(4).asNumber();
                            if ((numTemp > 1900) && (numTemp <= static_cast<unsigned int>(globals->today.year())))
                                pageVariables.yob = numTemp;
                            numTemp = temp.right(4).asNumber();
                            if ((numTemp >= static_cast<unsigned int>(globals->today.year() - 1)))
                                pageVariables.yod = numTemp;
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;
                }


                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1082 YellowPages
                break;

            case 1083:  // Shooga
            {
                //  Only URL
                sourceFile.beg();

                while (sourceFile.consecutiveMovesTo(100, "class=\"blog-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1083 Shooga
                break;

            case 1084:  // NBL
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"titre-deces\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOPublish
                    if (sourceFile.conditionalMoveTo("<li class=\"text-left\"", "</li>", 0))
                        pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1084 NBL
                break;

            case 1085:  // WPBakery
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"entry-title", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                     // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1085 WPBakery
                break;

            case 1086:  // Imago
            {
                while (sourceFile.consecutiveMovesTo(100, "entry-date published updated", "datetime="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read PubDate
                    pageVariables.ucPubDate = sourceFile.getUntil("T");

                    // YOB - YOD  age
                    sourceFile.moveTo("class=\"deces-date\"");
                    pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);
                    index = pageVariables.ucVariable.findPosition("(");
                    if (index > 0)
                    {
                        temp = pageVariables.ucVariable.right(pageVariables.ucVariable.getLength() - index);
                        temp.removeBookEnds(PARENTHESES);
                        temp.removeEnding(" ans");
                        pageVariables.ageAtDeath = temp.asNumber();

                        pageVariables.ucVariable.dropRight(pageVariables.ucVariable.getLength() - index);
                    }

                    // Read extension for each deceased
                    sourceFile.moveTo("href=");
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1086 Imago
                break;

            case 1088:  // Ubeo
            {
                switch(providerKey)
                {
                case 1:
                    while (sourceFile.consecutiveMovesTo(50, "class=\"margB_3 for_638\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                case 2:
                    while (sourceFile.consecutiveMovesTo(100, "class=\"avis_btn\"", "class=\"btn_more\" href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1088 Ubeo
                break;

            case 1090:  // Morin
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"post_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1090 Morin
                break;

            case 1091:  // Taiga
            {                   
                if (providerKey == 1)
                {
                    pageVariables.numericDateOrder = doDMY;

                    while (sourceFile.consecutiveMovesTo(40, "class=\"media-heading\"", "href=\""))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + sourceFile.getUntil("&");
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOB and DOD
                        if (sourceFile.consecutiveMovesTo(100, "class=\"media-dates\"", "</i"))
                            pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    while (sourceFile.consecutiveMovesTo(30, "class=\"btn btn-", "href=\""))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + sourceFile.getUntil("&p");
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOB and DOD
                        if (sourceFile.moveBackwardTo("Décédé(e) le  ", 1000))
                        {
                            pageVariables.ucDOD = sourceFile.getUntil("<");
                            pageVariables.numericDateOrder = doDMY;
                        }
                        sourceFile.moveTo("<div class=\"row\">");

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1091 Taiga
                break;

            case 1093:  // PubliWeb
            {
                if (providerKey == 1)
                {
                    while (sourceFile.consecutiveMovesTo(100, "class=\"item\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOD
                        if (sourceFile.moveTo("class=\"date\""))
                            pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    while (sourceFile.consecutiveMovesTo(100, "class=\"blanc11pt\"", "javascript:document.location.href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1093 PubliWeb
                break;

            case 1094:  // Magnus
            {
                if (flowParameters.currentPosition == 1)
                {
                    while (sourceFile.consecutiveMovesTo(100, "class=\"aside\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOB and DOD
                        if (sourceFile.conditionalMoveTo("class=\"small dates\"", "class=\"aside\"", 0))
                        {
                            pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);
                            pageVariables.ucDOBandDOD.replace(" | ", " - ");
                            pageVariables.ucDOBandDOD.replace(".", "/");
                            pageVariables.numericDateOrder = doDMY;
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                sourceFile.beg();
                PQString token, followUpURL, pageNumber;
                if (sourceFile.consecutiveMovesTo(100, "name=\"__RequestVerificationToken\"", "value="))
                {

                    token = sourceFile.readNextBetween(QUOTES);
                    pageNumber = PQString(flowParameters.currentPosition + 1);

                    followUpRequest.instructions.verb = QString("POST");
                    followUpURL  = URLbase + PQString("/Api/ObituaryListNextPage?__RequestVerificationToken=") + token;
                    followUpURL += PQString("&Form.AdvancedSearchBy=&Form.Page=") + pageNumber;
                    followUpURL += PQString("&Form.Name=&Form.Death=");

                    followUpRequest.instructions.qUrl.clear();
                    followUpRequest.instructions.url = followUpURL.getString();
                    followUpRequest.outputs.downloadFileName = QString("tempTempWebPage.htm");

                    followUpRequest.instructions.Accept = QString("ALL");
                    followUpRequest.instructions.Sec_Fetch_Mode = QString("cors");
                    followUpRequest.instructions.Sec_Fetch_Site = QString("same-origin");
                    followUpRequest.instructions.Sec_Fetch_Dest = QString("empty");
                    followUpRequest.instructions.Accept_Encoding = QString("br");
                    followUpRequest.instructions.Accept_Language = QString("en-US,en;q=0.9");
                    followUpRequest.instructions.Origin = baseURL.getString();
                    followUpRequest.instructions.Referer = URLaddressTemplate.getString();
                    followUpRequest.instructions.X_Requested_With = QString("XMLHttpRequest");

                    sourceFile.close();

                    www->download(followUpRequest);
                    while(www->processingDownload()){};
                    if(www->lastDownloadSuccessful())
                    {
                        sourceFile.setSourceFile(followUpRequest.outputs.downloadFileName);
                        sourceFile.setGlobalVars(*globals);
                        sourceFile.beg();

                        while (sourceFile.consecutiveMovesTo(100, "class=\"aside\"", "href="))
                        {
                            record.clear();
                            pageVariables.reset();

                            // Read extension for each deceased
                            pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                            pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                            // Read DOB and DOD
                            if (sourceFile.conditionalMoveTo("class=\"small dates\"", "class=\"aside\"", 0))
                            {
                                pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);
                                pageVariables.ucDOBandDOD.replace(" | ", " - ");
                                pageVariables.ucDOBandDOD.replace(".", "/");
                                pageVariables.numericDateOrder = doDMY;
                            }

                            // Process
                            process(record, pageVariables, lang);
                            if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                                records.append(record);
                        }
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1904 Magnus
                break;

            case 1095:  // Soleweb
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"entry-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1095 Soleweb
                break;

            case 1096:  // Voyou
            {
                if (providerKey < 3)
                {
                    sourceFile.JSONsimplify();

                    while (sourceFile.consecutiveMovesTo(125, "class=\"death-item-wrapper", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.webAddress.unescapeJSONformat();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                        if (pageVariables.ID.right(5) == QString(".html"))
                            pageVariables.ID.chop(5);

                       // Read DOD
                       if (sourceFile.consecutiveMovesTo(500, "class=\"dead-txt\">", "<br"))
                            pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                       // Process
                       process(record, pageVariables, lang);
                       if ((pageVariables.webAddress.right(13) == PQString("ceremonie.htm")) || (pageVariables.webAddress.right(13) == PQString("xposition.htm")))
                           pageVariables.includeRecord = false;
                       if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                           records.append(record);
                    }
                 }
                 else
                 {
                    PQString tempString;

                     while (sourceFile.consecutiveMovesTo(125, "class=\"vy_archive_container_item_info", "href="))
                     {
                         record.clear();
                         pageVariables.reset();

                         // Read extension for each deceased
                         tempString = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                         if (tempString.left(4) != PQString("http"))
                             pageVariables.webAddress = URLbase + tempString;
                         else
                             pageVariables.webAddress = tempString;
                         pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                         // Read DOD
                         if (sourceFile.conditionalMoveTo("class=\"vy_text uk-panel", "class=\"vy_archive_container_item_info", 0))
                         {
                             if (sourceFile.moveTo("Décédé", 50))
                             {
                                 sourceFile.moveTo(" le ", 15);
                                 pageVariables.ucDOD = sourceFile.getUntil("<");
                             }
                         }

                         // Process
                         process(record, pageVariables, lang);
                         if (pageVariables.webAddress.findPosition("diffusion") > 0)
                             pageVariables.includeRecord = false;
                         if ((pageVariables.webAddress.right(13) == PQString("ceremonie.htm")) || (pageVariables.webAddress.right(13) == PQString("xposition.htm")))
                             pageVariables.includeRecord = false;
                         if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                             records.append(record);
                     }
                 }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1096 Voyou
                break;

            case 1097:  // Scrypta
            {
                if (providerKey == 1)
                {
                    // new approach is JSON
                    sourceFile.unescapeJSONformat();

                    while (sourceFile.consecutiveMovesTo(75, "class=\"TGO01-card__text\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOD
                        if (sourceFile.moveTo("class=\"date subtitle caps color--5\""))
                        {
                            pageVariables.ucDOD = sourceFile.getUntil("<");
                            pageVariables.ucDOD.cleanUpEnds();
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    while (sourceFile.consecutiveMovesTo(75, "class=\"obi_name\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOD
                        if (sourceFile.consecutiveMovesTo(25, "class=\"obi_date\"", "le "))
                            pageVariables.ucDOD = sourceFile.getUntil("<");

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1097 Scrypta
                break;

            case 1098:  // Jaimonsite
            {
                while (sourceFile.consecutiveMovesTo(150, "data-hook=\"product-item-root\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    if (sourceFile.conditionalMoveTo("data-hook=\"product-item-ribbon\"", "</div>"))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1098 Jaimonsite
                break;

            case 1099:  // Saguenay
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"grid-item\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    if (sourceFile.conditionalMoveTo("class=\"info\"", "</article>"))
                    {
                        sourceFile.moveTo("<h", 50);
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1099 Saguenay
                break;

            case 1100:  // Lithium
            {
                while (sourceFile.consecutiveMovesTo(250, "class=\"columnsnecro avis-info\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    if (sourceFile.conditionalMoveTo("<p", "</div></div>"))
                    {
                        temp = sourceFile.readNextBetween(BRACKETS);
                        if (temp.getString().contains("-"))
                            pageVariables.ucDOBandDOD = temp;
                        else
                            pageVariables.ucDOD = temp;
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1100 Lithium
                break;

            case 1101:  // Cameleon
            {
                while (sourceFile.moveTo("<div class=\"table\">"))
                {
                    if (sourceFile.moveBackwardTo("href=", 500))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                    sourceFile.moveTo("</a>");
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1101 Cameleon
                break;

            case 1102:  // LogiAction
            {
                switch (providerKey)
                {
                case 1:
                    while (sourceFile.consecutiveMovesTo(50, "<li>", "https://www.complexefunerairejacquescouture.com/avis-de-deces/"))
                    {
                        record.clear();
                        pageVariables.reset();

                        sourceFile.moveBackwardTo("href=");

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Potentially pick off YOB and YOD
                        temp = pageVariables.webAddress.right(11);
                        if (temp.removeBookEnds(PARENTHESES))
                            pageVariables.ucVariable = temp;

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;

                case 2:
                    while (sourceFile.consecutiveMovesTo(500, "<div class=\"client\">", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Potentially pick off YOB and YOD
                        temp = pageVariables.webAddress.right(11);
                        if (temp.removeBookEnds(PARENTHESES))
                            pageVariables.ucVariable = temp;

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                    break;
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1102 LogiAction
                break;

            case 1103:  // BLsolutions
            {
                while (sourceFile.consecutiveMovesTo(25, "class=\"post_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Potentially pick off YOB and YOD
                    temp = pageVariables.webAddress.right(10);
                    temp.dropRight(1);
                    if (temp.getString().indexOf("-") == 4)
                        pageVariables.ucVariable = temp;

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1103 BLsolutions
                break;

            case 1104:  // Torapro
            {
                while (sourceFile.consecutiveMovesTo(500, "<article id=\"post-", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1104 Torapro
                break;

            case 1105:  // Axanta
            {
                while (sourceFile.moveTo("class=\"obituary-wrapper\""))
                {
                    record.clear();
                    pageVariables.reset();

                    sourceFile.moveBackwardTo("href=");

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOB - DOD
                    if (sourceFile.moveTo("class=\"entry-date\""))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1105 Axanta
                break;

            case 1106:  // ADN
            {
                while (sourceFile.moveTo("class=\"obituary__items\""))
                {
                    record.clear();
                    pageVariables.reset();

                    sourceFile.moveBackwardTo("href=");

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.consecutiveMovesTo(100, "class=\"obituary__date\"", "décédé", " le "))
                        pageVariables.ucDOD = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1106 ADN
                break;

            case 1107:  // B367
            {
                while (sourceFile.consecutiveMovesTo(150, "class=\"obituary-list-item", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // YOB - YOD
                    if (sourceFile.conditionalMoveTo("<span", "class=\"obituary-list-item", 0))
                    {
                        sourceFile.moveTo(">");
                        pageVariables.ucVariable = sourceFile.getUntil("<");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1107 B367
                break;

            case 1108:  // Tomahawk
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"boite-ombre\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Dates
                    if (sourceFile.conditionalMoveTo("class=\"deces-date\"", "</article>", 0))
                    {
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);
                        int index = pageVariables.ucVariable.getString().indexOf("(");
                        if (index > 0)
                            pageVariables.ucVariable.dropRight(pageVariables.ucVariable.getLength() - index + 1);
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1108 Tomahawk
                break;

            case 1110:  // Caza
            {
                while (sourceFile.consecutiveMovesTo(500, "class=\"content typography\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Dates
                    if (sourceFile.moveBackwardTo("<p>"))
                        pageVariables.ucVariable = sourceFile.getUntil(" |");

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1110 Caza
                break;

            case 1111:  // Tegara
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"info-defunt\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Dates
                    if (sourceFile.moveTo("class=\"infos-carte\""))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    if (sourceFile.moveTo("class=\"ann-e\""))
                        pageVariables.ucVariable = sourceFile.readNextBetween(QUOTES);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1111 Tegara
                break;

            case 1112:  // NMedia
            {
                //  JSON format, but not one read the usual way
                sourceFile.unescapeJSONformat();
                sourceFile.JSONsimplify();

                while (sourceFile.moveTo("17d538cf"))
                {
                    record.clear();
                    pageVariables.reset();

                    // DOD
                    sourceFile.moveBackwardTo("\"Value\":\"");
                    temp = sourceFile.getUntil("\"");
                    pageVariables.ucDOD = temp.left(10);

                    // YOB
                    sourceFile.moveTo("f2fec77b");
                    sourceFile.moveBackwardTo("\"Value\":\"");
                    pageVariables.yob = sourceFile.getUntil("\"").asNumber();

                    // Read extension for each deceased
                    sourceFile.moveTo("\"ProductModelUrl\":\"");
                    pageVariables.webAddress = sourceFile.getUntil("\"");
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1112 NMedia
                break;

            case 1113:  // Webs
            {
                sourceFile.moveTo("<ul");

                while (sourceFile.consecutiveMovesTo(200, "<li class=", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1113 Webs
                break;

            case 1114:  // Descary
            {
                while (sourceFile.consecutiveMovesTo(200, "class=\"post_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1114 Descary
                break;

            case 1115:  // Tonik
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"listAvis text-center\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.moveTo("class=\"date\""))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1115 Tonik
                break;

            case 1116:  // Kaleidos
            {
                int index1, index2;

                while (sourceFile.consecutiveMovesTo(25, "class=\"range-avis\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    index1 = pageVariables.webAddress.getString().indexOf("/Avis");
                    index1 = pageVariables.webAddress.getString().indexOf("/", index1 + 1);
                    index2 = pageVariables.webAddress.getString().indexOf("/", index1 + 1);
                    pageVariables.ID = pageVariables.webAddress.getString().mid(index1 + 1, index2 - index1 - 1);

                    // DOD
                    if (sourceFile.moveTo("class=\"ville-date-avis\""))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1116 Kaleidos
                break;

            case 1117:  // Gemini
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"omc-blog-one-heading\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOP
                    if (sourceFile.moveTo(">Publié le</b>"))
                        pageVariables.ucPubDate = sourceFile.getUntil(" |");

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1117 Gemini
                break;

            case 1118:  // Alias
            {
                while (sourceFile.consecutiveMovesTo(50, ">Date du décès<", ": "))
                {
                    record.clear();
                    pageVariables.reset();

                    // DOD
                    pageVariables.ucDOD = sourceFile.getUntil("<");

                    // Read extension for each deceased
                    sourceFile.moveTo("href=");
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1118 Alias
                break;

            case 1119:  // Cible
            {
                sourceFile.JSONsimplify();
                dateFormat = QString("yyyy-MM-dd");
                QString tempString;

                while (sourceFile.loadValue(QString("id"), tempString, false))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.ID = tempString;
                    sourceFile.loadValue(QString("death_date"), pageVariables.currentDOD, dateFormat);
                    sourceFile.loadValue(QString("birthdate"), pageVariables.currentDOB, dateFormat);
                    pageVariables.webAddress = URLbase + PQString("/avis-de-deces/details/?id=") + pageVariables.ID;

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1119 Cible
                break;

            case 1121:  // District4Web
            {
                if (providerKey == 2)
                {
                    while (sourceFile.consecutiveMovesTo(100, "class=\"entry-content\"", "content="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    while (sourceFile.consecutiveMovesTo(100, "class=\"fusion-rollover-title-link\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        sourceFile.moveTo("href=");
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1121 District4Web
                break;

            case 1122:  // Cake
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"inner box l-fade__in\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOB - DOD
                    if (sourceFile.consecutiveMovesTo(25, "<p", ">"))
                        pageVariables.ucDOBandDOD = sourceFile.getUntil(",");

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1122 Cake
                break;

            case 1123:  // J27
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"mfp_infinity_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1123 J27
                break;

            case 1124:  // NetRevolution
            {
                while (sourceFile.consecutiveMovesTo(250, "class=\"elementor-row\"", "data-raven-link="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.moveTo("class=\"jet-listing-dynamic-field__content\""))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1124 NetRevolution
                break;

            case 1125:  // ImageXpert
             {
                 while (sourceFile.consecutiveMovesTo(25, "property=\"dc:title\"", "href="))
                 {
                     record.clear();
                     pageVariables.reset();

                     // Read extension for each deceased
                     pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                     pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                     // Process
                     process(record, pageVariables, lang);
                     if (pageVariables.includeRecord || flowParameters.initialSetup)
                         records.append(record);
                 }

                 updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

             }   // end case 1125 ImageXpert
                 break;

            case 1126:  // Reactif
             {
                 while (sourceFile.consecutiveMovesTo(25, "class=\"sericeimg\"", "href="))
                 {
                     record.clear();
                     pageVariables.reset();

                     // Read extension for each deceased
                     pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                     pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                     // YOB - YOD
                     temp = pageVariables.webAddress.right(10);
                     temp.dropRight(1);
                     if (temp[4] == QChar('-'))
                         pageVariables.ucVariable = temp;

                     // Process
                     process(record, pageVariables, lang);
                     if (pageVariables.includeRecord || flowParameters.initialSetup)
                         records.append(record);
                 }

                 updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

             }   // end case 1126 Reactif
                 break;

            case 1127:  // Boite
             {
                 while (sourceFile.consecutiveMovesTo(25, "class=\"wpfh_obit_image\"", "href="))
                 {
                     record.clear();
                     pageVariables.reset();

                     // Read extension for each deceased
                     pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                     pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                     // Process
                     process(record, pageVariables, lang);
                     if (pageVariables.includeRecord || flowParameters.initialSetup)
                         records.append(record);
                 }

                 updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

             }   // end case 1127 Boite
                 break;

            case 1128:  // Orage
             {
                 while (sourceFile.consecutiveMovesTo(25, "class=\"fiche\"", "href="))
                 {
                     record.clear();
                     pageVariables.reset();

                     // Read extension for each deceased
                     pageVariables.webAddress = URLbase + PQString("/fr/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                     pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                     // DOD
                     if (sourceFile.conditionalMoveTo("<spa", "</div>", 0))
                         pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                     // Process
                     process(record, pageVariables, lang);
                     if (pageVariables.includeRecord || flowParameters.initialSetup)
                         records.append(record);
                 }

                 updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

             }   // end case 1128 Orage
                 break;

            case 1129:  // Kerozen
             {
                if (providerKey == 2)
                {
                    while (sourceFile.consecutiveMovesTo(100, "class=\"liste\"", "href="))
                     {
                         record.clear();
                         pageVariables.reset();

                         // Read extension for each deceased
                         pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                         pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                         if (sourceFile.conditionalMoveTo("Défunt | ", "class=\"liste\"", 0))
                             pageVariables.ucDOD = sourceFile.getUntil("<");

                         // Process
                         process(record, pageVariables, lang);
                         if (pageVariables.includeRecord || flowParameters.initialSetup)
                             records.append(record);
                     }
                }
                else
                {
                    while (sourceFile.moveTo("class=\"avis--date\""))
                     {
                         record.clear();
                         pageVariables.reset();

                         // YOB - YOD
                         pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                         if (sourceFile.conditionalMoveTo("Décédé(e) le ", "href=", 0))
                             pageVariables.ucDOD = sourceFile.getUntil("<");

                         // Read extension for each deceased
                         sourceFile.moveTo("href=");
                         pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                         pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                         // Process
                         process(record, pageVariables, lang);
                         if (pageVariables.includeRecord || flowParameters.initialSetup)
                             records.append(record);
                     }
                }

                 updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

             }   // end case 1128 Kerozen
                 break;

            case 1130:  // Inovision
             {
                 while (sourceFile.consecutiveMovesTo(100, "class=\"text-block\"", "href"))
                 {
                     record.clear();
                     pageVariables.reset();

                     // Read extension for each deceased
                     pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                     pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                     // DOD
                     if (sourceFile.moveTo("class=\"date\"", 150))
                         pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                     // Process
                     process(record, pageVariables, lang);
                     if (pageVariables.includeRecord || flowParameters.initialSetup)
                         records.append(record);
                 }

                 updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

             }   // end case 1130 Inovision
                 break;

            case 1131:  //  FRM
            {
                if (providerKey <= 100)
                {
                    switch(providerKey)
                    {
                    case 3:
                        while (sourceFile.consecutiveMovesTo(50, "class=\"entry-title\"", "href="))
                        {
                            record.clear();
                            pageVariables.reset();

                            // Read extension for each deceased
                            pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                            pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                            // Process
                            process(record, pageVariables, lang);
                            if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                                records.append(record);
                        }
                        break;

                    /*case 10:
                        while (sourceFile.consecutiveMovesTo(100, "class=\"entry-content\"", "content="))
                        {
                            record.clear();
                            pageVariables.reset();

                            // Read extension for each deceased
                            pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                            pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                            // Process
                            process(record, pageVariables, lang);
                            if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                                records.append(record);
                        }
                        break;*/

                    default:
                        while (sourceFile.consecutiveMovesTo(50, "class=\"entry-title-link\"", "href="))
                        {
                            record.clear();
                            pageVariables.reset();

                            // Read extension for each deceased
                            pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                            pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                            // Some sites have YOB-YOD tagged on to end of webaddress
                            temp = pageVariables.ID.right(9);
                            if (temp[4] == "-")
                            {
                                pageVariables.yob = temp.left(4).asNumber();
                                pageVariables.yod = temp.right(4).asNumber();
                            }

                            if (sourceFile.moveTo("class=\"entry-time\"", 500))
                                pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                            // Process
                            process(record, pageVariables, lang);
                            if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                                records.append(record);
                        }
                        break;
                    }
                }
                else
                {
                    while (sourceFile.consecutiveMovesTo(50, "class=\"post\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        if (sourceFile.conditionalMoveTo("<small", "</div>", 0))
                        {
                            sourceFile.moveTo(", ");
                            pageVariables.ucPubDate = sourceFile.getUntil("<");
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1131 FRM
                break;

            case 1132:  // Passage Coop
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"c p4 py2\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOP
                    if (sourceFile.conditionalMoveTo("class=\"date\"", "class=\"c p4 py2\"", 0))
                        pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1132 Passage Coop
                break;

            case 1134:  // JBCote
            {
                if (providerKey == 3)
                {
                    while (sourceFile.moveTo("Lire la fiche complète..."))
                    {
                        record.clear();
                        pageVariables.reset();

                        sourceFile.moveBackwardTo("href=");

                        // Read web address for each deceased
                        pageVariables.webAddress = URLbase + PQString("/cgi-bin/") + sourceFile.readNextBetween(QUOTES);
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                        sourceFile.moveTo("</tr>");

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }

                    if (pageVariables.ID > pageVariables.cutOffID)
                        URLaddressTemplate = URLbase + PQString("/cgi-bin/coop-avis.cgi?new_fiche=") + pageVariables.cutOffID + PQString("&last_fiche=") + pageVariables.ID + PQString("&last_line=") + pageVariables.cutOffID;
                }
                else
                {
                    while (sourceFile.consecutiveMovesTo(250, "class=\"obituaries-announcement\"", "href=\""))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = sourceFile.getUntil("?");
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Read DOD
                        if (sourceFile.conditionalMoveTo("<!--", "class=\"link\"", 1))
                        {
                            sourceFile.moveTo("<p");
                            pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                            pageVariables.numericDateOrder = doYMD;
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1134 JBCote
                break;

            case 1135:  // Blackcreek
            {
                while (sourceFile.consecutiveMovesTo(100, "<div class=\"blog-thumb right\">", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOP
                    if (sourceFile.conditionalMoveTo("Young Funeral Home</a>  | ", "<article", 0))
                        pageVariables.ucPubDate = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1135 Blackcreek
                break;

            case 1136:  // CityMax
            {
                // All obits are within a single page (no individual pages)
                OQStream tempStream;
                sourceFile.moveTo("section-webpage-multipage");

                while (sourceFile.moveTo("<h2"))
                {
                    record.clear();
                    pageVariables.reset();

                    tempStream = sourceFile.readNextBetween(BRACKETS);
                    tempID = tempStream.getString();
                    pageVariables.webAddress = downloadRequest.instructions.url;
                    pageVariables.ID = OQString(tempID).convertToID();

                    // Read dates
                    sourceFile.moveTo("<p", 50);
                    pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1136 CityMax

            case 1137:  // SYGIF
            {
                sourceFile.JSONsimplify();
                sourceFile.unescapeJSONformat();
                sourceFile.unQuoteHTML();
                OQString currentUcDOD;

                sourceFile.consecutiveMovesTo(100, "xsd:dateTime", "content=\"");
                currentUcDOD = sourceFile.getUntil("T");

                while (sourceFile.consecutiveMovesTo(50, "class=\"field-content nom\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Set DOD - Already read
                    pageVariables.ucDOD = currentUcDOD;

                    // Read web address
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("xsd:dateTime", "field-content nom", 0))
                    {
                        sourceFile.moveTo("content=\"");
                        currentUcDOD = sourceFile.getUntil("T");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1137 SYGIF

            case 1138:  // PortNeuf
            {
                pageVariables.numericDateOrder = doMDY;

                while (sourceFile.moveTo("category-avis-de-deces"))
                {
                    record.clear();
                    pageVariables.reset();

                    if (sourceFile.moveTo("class=\"date_label\"", 100))
                        pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    if (sourceFile.moveTo("href="))
                    {
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1138 PortNeuf

            case 1139:  // Burke
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"service-listing\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.consecutiveMovesTo(100, "date-text", "<h"))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1139 Burke

            case 1140:  // Canadian
            {
                QString tempString, yyyy, mmm, dd;
                int index;

                while (sourceFile.consecutiveMovesTo(50, "class=\"btn btn-secondary\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhParam1 + QString("/"), fhURLidDivider).getString();

                    // Pull out DOD from webaddress
                    tempString = pageVariables.webAddress.getString();
                    tempString.chop(5);
                    yyyy = tempString.right(4);
                    tempString.chop(5);
                    index = tempString.lastIndexOf("-");
                    dd = tempString.right(tempString.length() - index - 1);
                    tempString.chop(tempString.length() - index);
                    index = tempString.lastIndexOf("-");
                    mmm = tempString.right(tempString.length() - index - 1);
                    pageVariables.ucDOD = PQString(mmm).proper() + QString(" ") + dd + QString(", ") + yyyy;

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1140 Canadian

            case 1141:  // BallaMedia
            {
                while (sourceFile.moveTo("\"siteURL\":"))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1141 BallaMedia

            case 1142:  // Jac
            {
                while (sourceFile.consecutiveMovesTo(100, "<tr>", "<td", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1142 Jac

            case 1143:  // Ministry
            {
                // Limited number of obits, all on same page

                if (flowParameters.initialSetup)
                {
                    while (sourceFile.consecutiveMovesTo(20, "<br />", "<br />"))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        sourceFile.forward(5);
                        sourceFile.moveTo("<br />");
                        sourceFile.moveBackwardTo(":");
                        sourceFile.moveBackwardTo(">");
                        temp = sourceFile.getUntil("<");
                        temp.removeHTMLtags();
                        temp.replaceHTMLentities();
                        pageVariables.ID = temp.convertToID();
                        pageVariables.webAddress = downloadRequest.instructions.url;

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    while (sourceFile.moveTo("/photos/custom/"))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        temp = sourceFile.getUntil(".jp");
                        temp.removeHTMLtags();
                        temp.replaceHTMLentities();
                        pageVariables.ID = temp.convertToID();
                        pageVariables.webAddress = downloadRequest.instructions.url;

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 1143 Ministry
                break;

            case 1144:  // Multinet
            {
                // Limited number of obits, all on same page

                while (sourceFile.consecutiveMovesTo(50, "class=\"caption offset-top-10\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // location
                    sourceFile.moveTo("<br");
                    pageVariables.pcKey = sourceFile.readNextBetween(BRACKETS).getString();

                    // YOB-YOD
                    sourceFile.moveTo("br", 5);
                    pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1144 Ministry

            case 1145:  // PropulC
            {
                unstructuredContent lastDOD;

                while (sourceFile.moveToEarliestOf("class='title -smaller'", "class='title -bigger'"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    temp = sourceFile.get();
                    if (temp == ">")
                    {
                        sourceFile.moveTo("- ");
                        lastDOD = sourceFile.getUntil(" ");
                    }
                    else
                    {
                        sourceFile.moveTo("href=");
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    }
                    pageVariables.ucDOD = lastDOD;

                    // YOB-YOD
                    sourceFile.moveTo("</div");
                    pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord || flowParameters.initialSetup) && (pageVariables.ID.length() > 0))
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1145 PropulC

            case 1146:  // Nexion
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"avis\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // YOB-YOD
                    if (sourceFile.conditionalMoveTo("class=\"avis-date\"", "class=\"avis\"", 0))
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1146 Nexion

            case 1147:  // LCProduction
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"wpfh_obit_inner\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("Décédé(e) le <class=\"wpfh_obit_date\"", "class=\"wpfh_obit_inner\"", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1147 LCProduction

            case 1148:  // Absolu
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"avis-deces-resume\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("class=\"date\"", "class=\"avis-deces-resume\"", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1148 Absolu

            case 1149:  // Suite B
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"colonne\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("class=\"date\"", "class=\"colonne\"", 0))
                    {
                        temp = sourceFile.readNextBetween(BRACKETS);
                        if ((temp.getLength() == 9) && (temp[4] == "-"))
                            pageVariables.ucVariable = temp;
                        else
                            pageVariables.ucDOD = temp;
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.webAddress.getLength() < 25)
                        pageVariables.includeRecord = false;
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1149 SuiteB

            case 1150:  // Map
            {
                while (sourceFile.moveTo("<div class='deces'>"))
                {
                    record.clear();
                    pageVariables.reset();

                    // YOB-YOD
                    if (sourceFile.conditionalMoveTo("class='yspan'", "print_avis.php?id=", 0))
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                    // Read extension for each deceased
                    sourceFile.moveTo("print_avis.php?id=");
                    sourceFile.moveBackwardTo("../..", 100);
                    pageVariables.webAddress = URLbase + sourceFile.getUntil("'");
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1150 Map

            case 1151:  // iClic
            {
                // New format is JSON
                while (sourceFile.moveTo("postPageUrl:"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();

                    //DOP
                    sourceFile.consecutiveMovesTo(50, "publishedDate:", "$date", "\"");
                    pageVariables.ucPubDate = sourceFile.getUntil("T");

                    sourceFile.moveTo("slug:");
                    pageVariables.ID = sourceFile.readNextBetween(QUOTES).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1151 iClic

            case 1154:  // Bouille
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOB-DOD
                    if (sourceFile.conditionalMoveTo("class=\"date\"", "class=\"title\"", 0))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1154 Bouille

            case 1155:  // Techlogical
            {
                while (sourceFile.consecutiveMovesTo(100, "<li>", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("class=\"fusion-inline-sep\"", "<ul class=\"slides\">", 0))
                    {
                        sourceFile.moveBackwardTo("<span>");
                        pageVariables.ucDOD = sourceFile.getUntil("<");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1155 Techlogical

            case 1156:  // GyOrgy
            {
                sourceFile.moveTo(">RECENT OBITUARIES<");

                while (sourceFile.consecutiveMovesTo(100, "<li>", "data-testid=\"linkElement\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1156 GyOrgy

            case 1157:  // GemWebb
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"fl-post-feed-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1157 GemWebb

            case 1158:  // RedChair
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"uael-post__title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1158 RedChair

            case 1159:  // ExtremeSurf
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"cs-post-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("<li><time", "</article>", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1159 ExtremeSurf

            case 1160:  // Cahoots
            {
                if (flowParameters.initialSetup)
                {
                    while (sourceFile.consecutiveMovesTo(100, "class=\"article__title h3\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // DOD
                        if (sourceFile.conditionalMoveTo("datetime=\"", "class=\"article__title h3\"", 0))
                            pageVariables.ucDOD = sourceFile.getUntil("T");

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    while (sourceFile.consecutiveMovesTo(25, "<header>", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read extension for each deceased
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // DOD
                        if (sourceFile.conditionalMoveTo("datetime=\"", "class=\"article__title h3\"", 0))
                            pageVariables.ucDOD = sourceFile.getUntil("T");

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1160 Cahoots

            case 1161:  // Tride
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"obit-content", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOB, DOD and Age
                    if (sourceFile.conditionalMoveTo("Born:", "</div>", 0))
                    {
                        sourceFile.moveTo("</span");
                        pageVariables.ucDOB = sourceFile.readNextBetween(BRACKETS);
                    }

                    if (sourceFile.conditionalMoveTo("Died:", "</div>", 0))
                    {
                        sourceFile.moveTo("</span");
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                    }

                    if (sourceFile.conditionalMoveTo("Age:", "</div>", 0))
                    {
                        sourceFile.moveTo("</span");
                        pageVariables.ageAtDeath = sourceFile.readNextBetween(BRACKETS).asNumber();
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1161 Tride

            case 1162:  // Jensii
            {
                while (sourceFile.consecutiveMovesTo(50, "<div class=\"t\">", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOB and DOD
                    if (sourceFile.conditionalMoveTo("class=\"nservice\">", "<div class=\"t\">", 0))
                    {
                        temp = sourceFile.getUntil("</p>");
                        temp.removeHTMLtags();
                        pageVariables.ucDOBandDOD = temp;
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1162 Jensii

            case 1163:  // InterWeb
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"wpfh_obit_thumbnail_name\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.consecutiveMovesTo(200, "class=\"wpfh_obit_thumbnail_dates\"", "href=", ">", 0))
                        pageVariables.ucDOD = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1163 InterWeb

            case 1164:  // Brown
            {
                while (sourceFile.consecutiveMovesTo(150, "class=\"grid-x\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("<p>", "d-avis-txt"))
                        pageVariables.ucDOD = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1164 Brown

            case 1165:  // Tukio
            {
                // JSON approach
                QString anotherRecord;
                dateFormat = QString("MMMM dd, yyyy");
                sourceFile.beg();

                while (sourceFile.loadValue(QString("id"), anotherRecord, false))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.ID = anotherRecord;
                    sourceFile.loadValue(QString("public_url"), pageVariables.webAddress);
                    sourceFile.loadValue(QString("formatted_date_of_birth"), pageVariables.currentDOB, dateFormat);
                    sourceFile.loadValue(QString("formatted_date_of_death"), pageVariables.currentDOD, dateFormat);
                    sourceFile.loadValue(QString("age"), pageVariables.ageAtDeath);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1165 Tukio

            case 1166:  // webCemeteries
            {
                pageVariables.numericDateOrder = doMDY;
                while (sourceFile.consecutiveMovesTo(100, "class=\"decedent-name\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.consecutiveMovesTo(100, "Born:", "decedent-date-info"))
                        pageVariables.ucDOB = sourceFile.readNextBetween(BRACKETS);
                    if (sourceFile.consecutiveMovesTo(100, "Died:", "decedent-date-info"))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    if (sourceFile.conditionalMoveTo("Age", "class=\"basic-information\"", 0))
                    {
                        if(sourceFile.moveTo("decedent-date-info"))
                            pageVariables.ageAtDeath = sourceFile.readNextBetween(BRACKETS).asNumber();
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1166 webCemeteries

            case 1167:  // Etincelle
            {
                OQString extension;
                bool includeRec;

                while (sourceFile.consecutiveMovesTo(1000, "article class=\"avis\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    extension = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    includeRec = extension.removeLeading(".");
                    pageVariables.webAddress = URLbase + extension;
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (!includeRec)
                        pageVariables.includeRecord = false;
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1167 Etincelle

            case 1168:  // ObitAssistant
            {
                pageVariables.numericDateOrder = doMDY;

                while (sourceFile.consecutiveMovesTo(30, "VISITOR_NAME", "DATE"))
                {
                    record.clear();
                    pageVariables.reset();

                    // DOB
                    sourceFile.consecutiveMovesTo(25, "BORN_DATE", ":");
                    pageVariables.ucDOB = sourceFile.readNextBetween(QUOTES);

                    // ID
                    sourceFile.consecutiveMovesTo(25, "OBIT_URL_REWRITE", ":");
                    pageVariables.ID = sourceFile.readNextBetween(QUOTES).getString();
                    pageVariables.webAddress = PQString("https://www.williamsfuneralhomeltd.com/obituaries?name=") + pageVariables.ID;

                    // DOD
                    sourceFile.consecutiveMovesTo(25, "DIED_DATE", ":");
                    pageVariables.ucDOD = sourceFile.readNextBetween(QUOTES);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 1168 ObitAssistant

            case 2000:  // Mike Purdy
            {
                sourceFile.moveTo("Permanent Link to Obituaries");

                while (sourceFile.consecutiveMovesTo(50, "class=\"title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read dates as a single field
                    if (sourceFile.conditionalMoveTo("class=\"excerpt\"", "class=\"listing-item\"", 2))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 2000
                break;

            case 2001:  // BowRiver
            {
                sourceFile.moveTo("class=\"obit_idx\"");

                while (sourceFile.moveTo("class=\"obit_link\" href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    URLext = sourceFile.readNextBetween(QUOTES);
                    pageVariables.webAddress = baseURL + URLext;
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    if (sourceFile.consecutiveMovesTo(50, "obit_link_date", ">"))
                    {
                        sourceFile.backward(1);
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 2001
                break;

            case 2002:  // Serenity
            {
                sourceFile.moveTo("listing-group");

                while (sourceFile.moveTo("article-header"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read DOB
                    if (sourceFile.conditionalMoveTo("date-of-birth\"", "date-of-death", 0))
                        pageVariables.ucDOB = sourceFile.readNextBetween(BRACKETS);

                    // Read DOD
                    if (sourceFile.conditionalMoveTo("date-of-death", "</header>", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Read extension for each deceased
                    sourceFile.consecutiveMovesTo(100, "<address>", "a href=");
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 2002
                break;

            case 2003:  // McInnis
            {
                int index1;
                QString tempString;

                while (sourceFile.moveTo("class=\"deceased-name\">"))
                {
                    record.clear();
                    pageVariables.reset();

                    /*// Read ID
                    pageVariables.ID = sourceFile.getWord().getString();

                    // Read extension for each deceased
                    if (sourceFile.consecutiveMovesTo(200, "entry-title-link", "href="))
                         pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();

                    // Read DOB and DOD where available
                    if (sourceFile.conditionalMoveTo("entry-content", "entry-meta, 0"))
                    {
                        sourceFile.moveTo("<p>");
                        tempString = sourceFile.getNext(121).getString();
                        index1 = tempString.indexOf('\n');
                        if ((index1 >= 0) && (index1 <= 60))
                        {
                            pageVariables.ucVariable = tempString.left(index1);
                            index2 = tempString.indexOf('\n', index1 + 1);
                            if ((index2 >= 0) && ((index2 - (index1 + 1)) <= 60))
                            {
                                pageVariables.ucVariable += OQString(" ");
                                pageVariables.ucVariable += OQString(tempString.mid(index1 + 1, index2 - (index1 + 1)));
                            }
                        }
                    }*/

                    // Read extension for each deceased
                    if (sourceFile.moveTo("href=", 100))
                    {
                        tempString = sourceFile.readNextBetween(QUOTES).getString();
                        pageVariables.webAddress = URLbase + tempString;
                    }

                    // Read ID
                    index1 = tempString.indexOf(QString("/"), 17);
                    pageVariables.ID = tempString.mid(17, index1 - 17);

                    // Read DOD where available
                    if (sourceFile.conditionalMoveTo("deceased-date-of-death", "deceased-name", 0))
                    {
                        if (sourceFile.consecutiveMovesTo(200, "<span>", ", "))
                            pageVariables.ucDOD = sourceFile.getUntil("<");
                    }


                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 2003
                break;

            case 2004:  // Sturgeon
            {
                while (sourceFile.consecutiveMovesTo(1000, "class=\"page-header\"", "itemprop=\"name\"", "a href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read extension for each deceased
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 2004
                break;

            case 2005:  // CornerStone
            {
                while (sourceFile.moveTo("column four-fifths info"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read available dates
                    if (sourceFile.conditionalMoveTo("<h4", "<a href=>", 0))
                    {
                        temp = sourceFile.readNextBetween(BRACKETS);
                        singleChar = temp.right(1);
                        if (singleChar == OQString("-"))
                        {
                            if (sourceFile.conditionalMoveTo("<h4", "<a href=>", 0))
                            {
                                temp += OQString(" ");
                                temp += sourceFile.readNextBetween(BRACKETS);
                                pageVariables.ucDOBandDOD = temp;
                            }
                            else
                            {
                                temp.dropRight(1);
                                pageVariables.ucDOB = temp;
                            }
                        }
                        else
                        {
                            pageVariables.ucDOD = temp;
                        }
                    }

                    // Read extension for each deceased
                    sourceFile.moveTo("a href=");
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 2005
                break;

            case 2006:  // Pierson's
            {
                while (sourceFile.moveTo("obituary-info"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    if (sourceFile.moveTo("href="))
                    {
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    }

                    // Read available dates
                    if (sourceFile.conditionalMoveTo("class=\"dates\"", "class=\"description\""))
                    {
                        temp = sourceFile.getUntil("</div>");
                        pageVariables.ucDOBandDOD = temp;
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 2006
                break;

            case 2007:  // Trinity
            {
                while (sourceFile.consecutiveMovesTo(25, "class=\"obituary-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read webaddress and ID
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 2007
                break;

            case 2008:  // CelebrateLife
            {
                sourceFile.moveTo("id=\"wsite-content\"");

                while (sourceFile.moveTo("wsite-content-title"))
                {
                    record.clear();
                    pageVariables.reset();

                    if (sourceFile.conditionalMoveTo("class=\"paragraph\"", "a href=", 0))
                    {
                        // Read DOB and DOD
                        sourceFile.moveTo(">");
                        pageVariables.ucDOBandDOD = sourceFile.getUntil("<");

                        // Read webaddress
                        if (sourceFile.moveTo("a href="))
                        {
                            pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                            pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                        }
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 2008
                break;

            case 2009:  // Funks
            {
                sourceFile.moveTo("<p>Obituaries</p>");     // Normal processing
                //sourceFile.moveTo(">This page has the following sub pages.<");     // One of the archives

                while (sourceFile.conditionalMoveTo("href=", "******", 0))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read webaddress
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 2009
                break;

            case 2010:  // WowFactor
            {
                while (sourceFile.consecutiveMovesTo(500, "<!-- .et_pb_portfolio_item -->", "id=\"post", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read webaddress
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 2010
                break;

            case 2011:  // Dalmeny
            {
                while (sourceFile.moveTo("class=\"post_title\""))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read webaddress
                    if (sourceFile.moveTo("href="))
                    {
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 2011
                break;

            case 2012:  // Hansons
            {
                while (sourceFile.moveTo("class=\"post_title\""))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read webaddress
                    if (sourceFile.moveTo("href="))
                    {
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break;

            case 2014:  // Martens
            {
                while (sourceFile.consecutiveMovesTo(30, "class=\"wpfh_obit_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read webaddress
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    sourceFile.moveTo("class=\"wpfh_obit_date\"");
                    pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break;

            case 2015:  // Shine
            {
                sourceFile.consecutiveMovesTo(200, ">Current Services<", "class=\"wpb_wrapper\"", ">");

                while (sourceFile.consecutiveMovesTo(250, "class=\"wpb_text_column wpb_content_element", "class=\"wpb_wrapper\"", "<p", ">"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read DOD
                    sourceFile.backward(1);
                    pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Read URL
                    sourceFile.consecutiveMovesTo(100, "class=\"wpb_wrapper\"", "href=");
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break;

            case 2016:  // Simple
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"post-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    //sourceFile.consecutiveMovesTo(100, "class=\"wpb_wrapper\"", "href=");
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("Passed Away: ", "</div>", 0))
                        pageVariables.ucDOD = sourceFile.getUntil("<");

                    if (sourceFile.conditionalMoveTo("Obituary added: ", "</div?", 0))
                        pageVariables.ucPubDate = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2016 Simple

            case 2017:  // McCall
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"ob-content\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.moveTo("Date Posted: "))
                        pageVariables.ucPubDate = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2017 McCall

            case 2018:  // Care
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"wpfh_obit_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.moveTo("class=\"wpfh_obit_date\"", 150))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2018 Care

            case 2019:  // Ancient
            {
                pageVariables.numericDateOrder = doMDY;

                while (sourceFile.consecutiveMovesTo(75, "class=\"blog-title-link blog-link\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = PQString("https:") + sourceFile.readNextBetween(QUOTES);
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.moveTo("class=\"date-text\"", 150))
                        pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2019 Ancient

            case 2020:  // Amherst
            {
                sourceFile.moveTo(">Amherst Memorial Pages<");

                while (sourceFile.consecutiveMovesTo(20, "<p>", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2020 Amherst

            case 2022:  // Heritage
            {
                while (sourceFile.consecutiveMovesTo(50, "vc_btn3-color-juicy-pink", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2022 Heritage

            case 2023:  // Koru
            {
                sourceFile.JSONsimplify();
                sourceFile.unescapeJSONformat();
                sourceFile.unQuoteHTML();

                while (sourceFile.consecutiveMovesTo(50, "class=\"sc_services_item_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2023 Koru

            case 2024:  // Kowalchuk
            {
                while (sourceFile.consecutiveMovesTo(25, "class=\"entry-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if(sourceFile.consecutiveMovesTo(50, "class=\"entry-date published\"", ">"))
                        pageVariables.ucDOD = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2024 Kowalchuk

            case 2025:  // Loehmer
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"row kl-list\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if(sourceFile.conditionalMoveTo("Passed on ", "class=\"row kl-list\""))
                        pageVariables.ucDOD = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2025 Loehmer

            case 2026:  // Doyles
            {
                if (flowParameters.initialSetup)
                {
                    while (sourceFile.consecutiveMovesTo(100, "<td", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read URL
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    while (sourceFile.consecutiveMovesTo(50, "item_title item_title__", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read URL
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2026 Doyles

            case 2027:  // Ethical
            {
                // URL and DOP  **** DOUBLE DOWNLOAD REQUIRED ****
                // JSON approach
                QString altID, tempString;
                PQString newURL;
                qSourceFile tempSourceFile;
                bool keepGoing = true;

                unstructuredContent tempStream;
                dateFormat = QString("yyyy-MM-dd");

                while (keepGoing && sourceFile.loadValue(QString("_id"), altID, false))
                {
                    record.clear();
                    pageVariables.reset();
                    tempString.clear();
                    keepGoing = false;

                    sourceFile.loadValue(QString("seoSlug"), tempString);
                    newURL = URLbase + PQString("/post/") + tempString;
                    followUpRequest.instructions.url = newURL.getString();
                    followUpRequest.outputs.downloadFileName = QString("tempTempWebPage.htm");
                    www->download(followUpRequest);
                    while(www->processingDownload()){};
                    if(www->lastDownloadSuccessful())
                    {
                        tempSourceFile.setSourceFile(followUpRequest.outputs.downloadFileName);
                        tempSourceFile.setGlobalVars(*globals);

                        if (tempSourceFile.moveTo(">OBITUARIES<"))
                        {
                            while (tempSourceFile.moveTo("<a data-testid=\"linkElement\" href="))
                            {
                                pageVariables.webAddress = tempSourceFile.readNextBetween(QUOTES);
                                pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                                // Process
                                process(record, pageVariables, lang);
                                if (pageVariables.includeRecord || flowParameters.initialSetup)
                                    records.append(record);
                            }
                        }
                        keepGoing = false;
                    }

                    altID.clear();

                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2027 Ethical

            case 2029:  // Direct
            {
                while (sourceFile.consecutiveMovesTo(150, "et_pb_column_1_4", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if(sourceFile.conditionalMoveTo("<p>", "<!-- .et_pb_text -->", 0))
                        pageVariables.ucDOBandDOD = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2029 Direct

            case 2030:  // SMC
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"collection-item _100", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2030 SMC

            case 2031:  // Belvedere
            {
                // JSON approach
                QString altID;
                dateFormat = QString("yyyy-MM-dd");

                while (sourceFile.loadValue(QString("_id"), altID, false))
                {
                    record.clear();
                    pageVariables.reset();

                    sourceFile.loadValue(QString("name"), pageVariables.ID);
                    sourceFile.loadValue(QString("death"), pageVariables.currentDOD, dateFormat);
                    sourceFile.loadValue(QString("birth"), pageVariables.currentDOB, dateFormat);
                    pageVariables.webAddress = URLbase + PQString("/memorial/") + pageVariables.ID;

                    altID.clear();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord || flowParameters.initialSetup) && (pageVariables.ID.length() > 1))
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2031 Belvedere

            case 2032:  // Davidson
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"cwp_list_item\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("Passing Date:", "class=\"cwp_list_item\"", 0))
                    {
                        sourceFile.consecutiveMovesTo(100, "<br>", ", ");
                        pageVariables.ucDOD = sourceFile.getUntil("<");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2032 Davidson

            case 2033:  // Carnell
            {
                sourceFile.moveTo("class=\"archiveObit\"");

                while (sourceFile.consecutiveMovesTo(50, "</a>", "<h3>", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("</h3><h6", "</p>"))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2033 Carnell

            case 2035:  // JOsmond
            {
                while (sourceFile.consecutiveMovesTo(150, "class=\"obituary-list-item", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"meta-data\"", "class=\"obituary-list-item", 0))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2035 JOsmond

            case 2036:  // Tivahost
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"nxs-title nxs-applylinkvarcolor", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES);
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"nxs-blog-meta\"", "class=\"nxs-title nxs-applylinkvarcolor", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2036 Tivahost

            case 2037:  // KMF
            {
                while (sourceFile.moveTo("<!--POST HEADER-->"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    if (sourceFile.consecutiveMovesTo(100, "class=\"entry-title\"", "href="))
                    {
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    }

                    // Read DOP
                    if (sourceFile.consecutiveMovesTo(50, "class=\"published\"", "datetime="))
                    {
                        pageVariables.ucPubDate = sourceFile.readNextBetween(QUOTES);
                        pageVariables.numericDateOrder = doYMD;
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                sourceFile.beg();
                sourceFile.moveTo("<!--PAGINATION-->");
                sourceFile.consecutiveMovesTo(100, "class=\"older-posts", "offset=");
                flowParameters.lastValueParam = QString("=") + sourceFile.getUntil("\"").getString();

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2037 KMF

            case 2038:  // AMG
            {
                if (flowParameters.initialSetup)
                {
                    while (sourceFile.consecutiveMovesTo(100, "class=\"list-item\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read URL
                        pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    while (sourceFile.consecutiveMovesTo(100, "class=\"person\"", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read URL
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.webAddress.replace("listing.", "obituary.", Qt::CaseSensitive);
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2038 AMG

            case 2039:  // Orillia
            {
                while (sourceFile.consecutiveMovesTo(25, "class=\"wpfh_obit_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    sourceFile.moveTo("class=\"wpfh_obit_date\"");
                    pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2039 Orillia

            case 2040:  // OSM
            {
                while (sourceFile.consecutiveMovesTo(250, "class=\"news_story_link\"", "ID="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.ID = sourceFile.getUntil("&").getString();
                    pageVariables.webAddress = URLbase + QString("/obituariesnoticess34.php?command=viewArticle&ID=") + pageVariables.ID;

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2040 OSM

            case 2041:  // Alcock
            {
                QString excluded = QString("about-us|about-1|account|biodegradable-urns|cart-page|checkout|contact|fullscreen-page|fullscreen-page-1|grief-and-healing|home|jewelry|keepsake-urns");
                excluded += QString("|links|obituaries|organizations|our-staff|planning-ahead|pre-arrange-now|price-list|product-page|regular-urns|search-results|services|shop");
                QStringList excludedList = excluded.split("|");
                //sourceFile.moveTo(".\\/copy-of-obituaries");

                while (sourceFile.consecutiveMovesTo(100, "MazNVa rYiAuL", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES);
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord || flowParameters.initialSetup) && !excludedList.contains(pageVariables.ID))
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2041 Alcock

            case 2042:  // Abstract
            {
                sourceFile.moveTo(">Death Notices<");

                while (sourceFile.consecutiveMovesTo(15, "<h2>", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    pageVariables.yod = pageVariables.webAddress.middle(pageVariables.webAddress.getString().indexOf(".ca/") + 4, 4).asNumber();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2042 Abstract

            case 2043:  // Beechwood
            {
                int count = 0;
                if (flowParameters.currentPosition == 0)
                    count = 1;

                while ((count < 8) && (sourceFile.moveTo("href=")))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    URLext = sourceFile.getUntil("\\u0022\\u003E\\n ");
                    URLext.dropLeft(6);
                    URLext.replace("\\/", "/");
                    pageVariables.webAddress = URLbase + URLext;
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    count++;

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2043 Beechwood

            case 2044:  // Benjamins
            {
                sourceFile.moveTo(">SERVICES<");

                while (sourceFile.consecutiveMovesTo(25, "month_view_name", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2044 Benjamins

            case 2045:  // Berthiaume
            {
                lang = french;

                while (sourceFile.consecutiveMovesTo(100, "tr style=", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    if (sourceFile.conditionalMoveTo("align='left'", "</tr>", 0))
                    {
                        sourceFile.moveTo(" ");
                        pageVariables.ucDOD = sourceFile.getUntil("<");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2045 Berthiaume

            case 2046:  // Blenheim
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"obit-item\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOB and DOD
                    if (sourceFile.conditionalMoveTo("class=\"obit-dates\"", "</a>", 0))
                        pageVariables.ucDOBandDOD = sourceFile.getUntil("</p>");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2046 Blenheim

            case 2047:  // Brenneman
            {
                while (sourceFile.consecutiveMovesTo(50, "so9KdE TBrkhx", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOB and DOD
                    //if (sourceFile.moveTo("<br /"))
                    //    pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2047 Brenneman

            case 2048:  // Brenneman
            {
                while (sourceFile.consecutiveMovesTo(250, "search-result", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    if (sourceFile.conditionalMoveTo("class=\"obit-date\"", "<!--/search-result-->", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord || flowParameters.initialSetup) && (pageVariables.ID.left(1) != QString("?")))
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2048 Brenneman

            case 2049:  // Carson
            {
                // URL and DOD
                // JSON approach
                QString webAddressExt;
                dateFormat = QString("yyyy-MM-dd");

                while (sourceFile.loadValue(QString("Id"), webAddressExt, false))
                {
                    record.clear();
                    pageVariables.reset();

                    sourceFile.loadValue(QString("ShortId"), pageVariables.ID, false);
                    sourceFile.loadValue(QString("DateOfDeath"), pageVariables.currentDOD, dateFormat);
                    pageVariables.webAddress = URLbase + QString("/obituaries/details/") + pageVariables.ID;
                    webAddressExt.clear();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2049 Carson

            case 2050:  // Turner
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"entry-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"published\"", "class=\"comments-number\"", 0))
                        pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    // Read DOB - DOD
                    sourceFile.moveTo("<p>");
                    if (sourceFile.moveTo("<img", 10))
                        sourceFile.moveTo(">");
                    pageVariables.ucDOBandDOD = sourceFile.getUntil("</p>");

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2050 Turner

            case 2052:  // Eagleson
            {
                while (sourceFile.consecutiveMovesTo(150, "<!-- Left Content -->", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOB - DOD
                    sourceFile.moveTo("class=\"obit-date\">");
                    pageVariables.ucDOBandDOD = sourceFile.getUntil("-<");

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2052 Eagleson

            case 2053:  // FirstMemorial
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"inner-orbituary\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read Dates
                    if (sourceFile.conditionalMoveTo("class=\"equal-height-date\"", "class=\"inner-orbituary\"", 0))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2053 FirstMemorial

            case 2054:  // Haine
            {
                // All obits are within a single page (no individual pages)
                OQStream tempStream;
                OQString word;

                while (sourceFile.moveTo("<div class=\"plain\">"))
                {
                    record.clear();
                    pageVariables.reset();

                    sourceFile.conditionalMoveTo("<b>", "</div>", 0);
                    tempStream = sourceFile.getUntil("<");
                    tempID = tempStream.getString();

                    // Read URL
                    word = tempStream.getWord();
                    if (word.removeEnding(",") && word.isAllCaps())
                    {
                        pageVariables.webAddress = downloadRequest.instructions.url;
                        pageVariables.ID = OQString(tempID).convertToID();

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2054 Haine

            case 2056:  // RHB
            {
                if (flowParameters.initialSetup)
                {
                    while (sourceFile.consecutiveMovesTo(100, "class='monthsPage pageNode  pageId", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read URL
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    sourceFile.moveTo("<!-- Begin Main Content -->");

                    while (sourceFile.consecutiveMovesTo(10, "<li>", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read URL
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2056 RHB

            case 2057:  // Rhody
            {
                // All obits are within a single page (no individual pages)

                while (sourceFile.consecutiveMovesTo(250, "article id=\"post-", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2057 Rhody

            case 2058:  // Simpler
            {
                // JSON approach
                QString webAddressExt;
                dateFormat = QString("MMMM dd, yyyy");

                while (sourceFile.loadValue(QString("id"), tempID, false))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.ID = tempID;
                    sourceFile.loadValue(QString("dod"), pageVariables.currentDOD, dateFormat);
                    sourceFile.loadValue(QString("url"), webAddressExt);
                    pageVariables.webAddress = URLbase + PQString(webAddressExt).unescapeJSONformat();
                    webAddressExt.clear();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2058 Simpler

            case 2059:  // Steadman
            {
                if (flowParameters.initialSetup)
                {
                    sourceFile.moveTo(">Obituaries<");

                    while (sourceFile.consecutiveMovesTo(10, "<h", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read URL
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // DOB and DOD
                        if (sourceFile.conditionalMoveTo("<p", "</div>", 1))
                            pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    sourceFile.moveTo(">Recent Obituaries<");

                    while (sourceFile.consecutiveMovesTo(10, "<li>", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read URL
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2059 Steadman

            case 2060:  // Steeles
            {
                while (sourceFile.consecutiveMovesTo(25, "class=\"view-more\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2060 Steeles

            case 2061:  // Bridge
            {
                while (sourceFile.consecutiveMovesTo(30, "class=\"page-header\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2061 Bridge

            case 2062:  // McCormack
            {
                //QString excluded = QString("after-a-loss|blue-water-page|contact-us|explanation-of-pricing|history|no-notice|obituaries|observer-online|online-registration-form|our-facilities|testimonials|under-construction");
                //QStringList excludedList = excluded.split("|");

                while (sourceFile.consecutiveMovesTo(300, "data-testid=\"mesh-container-content\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    pageVariables.ID.chop(QString("mccormackfuneralhome").length());

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord || flowParameters.initialSetup) && pageVariables.webAddress.getString().contains("obituarylist-1"))
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2062 McCormack

            case 2063:  // Brunet
            {
                pageVariables.numericDateOrder = doYMD;

                while (sourceFile.consecutiveMovesTo(100, "class=\"obituary ", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Check get list of IDs
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"obituary__date\"", "class=\"obituary ", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2063 Brunet

            case 2065:  // TurnerFamily
            {
                while (sourceFile.consecutiveMovesTo(25, "class=\"entry-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"published\"", "comments-number", 0))
                        pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);


                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2065 TurnerFamily

            case 2066:  // VanHeck
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"entry-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("itemprop=\"datePublished\"", "<!-- .entry-meta -->", 0))
                        pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2066 VanHeck

            case 2067:  // TBK
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"blogs-item-link\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("Passing Date:<", "class=\"blogs-item-link\"", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2067 TBK

            case 2068:  // Whelan
            {
                // All a single page
                sourceFile.moveTo("<!-- NEW Notices -->");

                //while (sourceFile.consecutiveMovesTo(50, "<p style=\"margin-bottom: 1em;\">", "<b"))
                while (sourceFile.moveTo("<p style=\"margin-bottom: 1em;\">"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    sourceFile.moveToEarliestOf("<b", "<strong");
                    temp = sourceFile.readNextBetween(BRACKETS);
                    pageVariables.ID = temp.convertToID();
                    pageVariables.webAddress = downloadRequest.instructions.url;

                    if (sourceFile.conditionalMoveTo("<b>", "</p>", 0))
                    {
                        sourceFile.backward(1);
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2068 Whelan

            case 2069:  // Aeterna
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"list-article grid__row death__notice-container js-obituary\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"slider__death-infoYear capitalize\"", "</div>", 0))
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);
                    if (sourceFile.conditionalMoveTo("class=\"slider__death-infoDate capitalize\"", "</div>", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2069 Aeterna

            case 2070:  // Actuel
            {
                while (sourceFile.moveTo("class=\"date-avis\""))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read YOB - YOD
                    pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                    // Read URL
                    if (sourceFile.consecutiveMovesTo(50, "class=\"more-avis\"", "href="))
                    {
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                sourceFile.beg();
                if (sourceFile.consecutiveMovesTo(500, "class=\"pagination\"", "class=\"last\"", "/en/obituary-find-a-person/", "&pg="))
                {
                    pageVariables.paginatedResult = true;
                    flowParameters.endingPosition = sourceFile.getUntil("\"").asNumber();
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2070 Actuel

            case 2071:  // Dupuis
            {
                sourceFile.JSONsimplify();

                while (sourceFile.consecutiveMovesTo(50, "class=\"avis new\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    temp = sourceFile.readNextBetween(QUOTES);
                    temp.removeLeading("//salonsdupuis.com");
                    pageVariables.webAddress = URLbase + temp;

                    tempID = temp.getString().right(10);
                    if (tempID[5] == QString("-"))
                    {
                        temp.dropRight(10);
                        pageVariables.ucVariable = OQString(tempID.mid(1, 9));
                    }
                    temp.dropLeft(15);
                    pageVariables.ID = temp.getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2071 Dupuis

            case 2072:  // HGDivision
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"obit-content clearfix\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"obit-date\"", "class=\"margin-wrapper\"", 0))
                    {
                        sourceFile.consecutiveMovesTo(50, "le: ", "</spa");
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2072 HGDivision

            case 2073:  // Jacques
            {
                pageVariables.numericDateOrder = doYMD;
                QString target;

                if (flowParameters.initialSetup)
                    target = QString("class=\"btn-group\"");
                else
                    target = QString("class=\"ct-gallery-itemInner-image ct-gallery--hover\"");

                while (sourceFile.consecutiveMovesTo(30, target, "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOB and DOD
                    sourceFile.moveTo("class=\"ct-u-colorGrey ct-u-displayBlock ct-fw-700 ct-fs-i\"");
                    pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2073 Jacques

            case 2074:  // Joliette
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"blog-shortcode-post-title entry-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    sourceFile.moveTo("<p>");
                    pageVariables.ucVariable = sourceFile.getUntil(" ");
                    if ((pageVariables.ucVariable.getLength() != 9) || (pageVariables.ucVariable[4] != QString("-")))
                    {
                        pageVariables.ucVariable = sourceFile.getWord();
                        if ((pageVariables.ucVariable.getLength() != 9) || (pageVariables.ucVariable[4] != QString("-")))
                            pageVariables.ucVariable.clear();
                    }

                    if (sourceFile.moveTo("class=\"fusion-inline-sep\""))
                    {
                        sourceFile.moveBackwardTo("<span>");
                        pageVariables.ucPubDate = sourceFile.getUntil("<");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2074 Joliette

            case 2075:  // Rajotte
            {
                // All obits are within a single page (no individual pages)
                OQStream tempStream;
                OQString word;

                while (sourceFile.consecutiveMovesTo(100, "twitter-share-button", "data-url="))
                {
                    record.clear();
                    pageVariables.reset();
                    pageVariables.numericDateOrder = doYMD;

                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES);
                    pageVariables.ID = pageVariables.webAddress.right(4).getString();

                    /*
                    // Dates
                    temp = sourceFile.getUntil(" ");
                    QDate qdate = temp.readDateField(doYMD);
                    if (qdate.isValid())
                    {
                        if ((qdate.month() != 1) || (qdate.day() != 1))
                            pageVariables.ucDOB = temp;
                    }

                    sourceFile.moveTo("/ ", 20);
                    pageVariables.ucDOD = sourceFile.getUntilEarliestOf(" ", "\"");

                    sourceFile.moveTo("php/form_share.php?id=");
                    pageVariables.ID = sourceFile.getUntil("&").getString();
                    pageVariables.webAddress = downloadRequest.instructions.url;*/

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2075 Rajotte

            case 2076:  // BM
            {
                while (sourceFile.moveTo("class=\"necro_Name\""))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    sourceFile.moveBackwardTo("href=");
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    sourceFile.moveTo("</div>");

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2076 BM

            case 2077:  // Jodoin
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"photo\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("| Décédé(e) le ", "class=\"photo\"", 0))
                        pageVariables.ucDOD = sourceFile.getUntil("<");

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2077 Jodoin

            case 2078:  // Fournier
            {
                while (sourceFile.consecutiveMovesTo(350, "<div class=\"article-wrap\">", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"date\"", "</article>", 0))
                    {
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                        pageVariables.ucDOD.fixBasicErrors();
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2078 Fournier

            case 2079:  // Desnoyer
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"fusion-rollover-link\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2079 Desnoyer

            case 2080:  // Desrosiers
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"entry-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.consecutiveMovesTo(100, "class=\"entry-date\"", "</i"))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2080 Desrosiers

            case 2081:  // Montpetit
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"date-display-single\"", "content=\""))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.ucDOD = sourceFile.getUntil("T");

                    // Read URL
                    sourceFile.consecutiveMovesTo(100, "class=\"views-field views-field-title\"", "href=");
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2081 MontPetit

            case 2082:  // Parent
            {
                while (sourceFile.moveTo("class=\"card 1\""))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    sourceFile.moveBackwardTo("<a", 100);
                    sourceFile.moveTo("href=");
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    sourceFile.moveTo("</a>");

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2082 Parent

            case 2083:  // RichardPhilibert
            {
                sourceFile.moveTo("class=\"entry main-entry\"");

                while (sourceFile.consecutiveMovesTo(100, "class=\"date\"", "href=\"#\""))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    // Read URL
                    sourceFile.consecutiveMovesTo(100, "class=\"entry-title\"", "href=");
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2083 RichardPhilibert

            case 2084:  // Kane
            {
                sourceFile.moveTo("class=\"obituaries_list\"");

                while (sourceFile.consecutiveMovesTo(75, "<li", "<span>", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + PQString("/en/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    sourceFile.moveTo("<em");
                    temp = sourceFile.readNextBetween(BRACKETS);
                    if (temp.asNumber() > 0)
                        pageVariables.yob = temp.asNumber();
                    sourceFile.moveTo("<em");
                    temp = sourceFile.readNextBetween(BRACKETS);
                    if (temp.asNumber() > 0)
                        pageVariables.yod = temp.asNumber();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2084 Kane

            case 2085:  // Gaudet
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"avis\"", "<img"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    sourceFile.moveBackwardTo("href=");
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    int index = pageVariables.webAddress.getString().indexOf("&p=");
                    if (index != -1)
                        pageVariables.webAddress.dropRight(pageVariables.webAddress.getLength() - index);
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("</h2", "</a>", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2085 Gaudet

            case 2087:  // NouvelleVie
            {
                while (sourceFile.moveTo("avis-de-deces-detail.php?id="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    sourceFile.moveBackwardTo("href=");
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    sourceFile.moveTo("class=\"dates\"");
                    pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);
                    pageVariables.ucVariable.removeBookEnds(PARENTHESES);
                    sourceFile.moveTo("</a>");

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord || flowParameters.initialSetup) && ((pageVariables.yod == 1900) || (pageVariables.yod >= 2019)))
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2087 NouvelleVie

            case 2088:  // Santerre
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"post_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.moveTo("class=\"post_info_date\""))
                        pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2088 Santerre

            case 2089:  // Shields
            {
                if (flowParameters.initialSetup == true)
                {
                    while (sourceFile.consecutiveMovesTo(25, "<td align=\"left\">", "href="))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read URL
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        if (sourceFile.conditionalMoveTo("<td align='left'>", "</tr>", 0))
                        {
                            sourceFile.moveTo(" ");
                            pageVariables.ucDOD = sourceFile.getUntil("<");
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    while (sourceFile.consecutiveMovesTo(200, "<strong>", "Date du d", "s :", "</strong> "))
                    {
                        record.clear();
                        pageVariables.reset();

                        sourceFile.moveTo(" ");
                        pageVariables.ucDOD = sourceFile.getUntil("<");

                        // Read URL
                        sourceFile.moveTo("href=");
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2089 Shields

            case 2090:  // Gamache
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"image_wrapper\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.moveTo("</a></h4"))
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2090 Gamache

            case 2091:  // Landry
            {
                while (sourceFile.consecutiveMovesTo(500, "<!-- article -->", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2091 Landry

            case 2092:  // St.Louis
            {
                pageVariables.numericDateOrder = doYMD;

                while (sourceFile.consecutiveMovesTo(100, "class=\"content flex-grow-1\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.getUntil(" ");
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOD
                    sourceFile.moveTo("datetime=");
                    pageVariables.ucDOD = sourceFile.getUntil(">");

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2092 St.Louis

            case 2093:  // McGerrigle
            {
                while (sourceFile.consecutiveMovesTo(300, "<article id=\"post", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2093 McGerrigle

            case 2094:  // Paperman
            {
                while (sourceFile.consecutiveMovesTo(50, "class='vevent'", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2094 Paperman

            case 2095:  // Poissont
            {
                while (sourceFile.consecutiveMovesTo(50, "deces-liste-nom", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(DOUBLE_QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("<h3", "</div>", 0))
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2095 Poissont

            case 2096:  // Legare
            {
                while (sourceFile.consecutiveMovesTo(50, "<div id=\"txtDefunt\">", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("<p class=\"bodyDefunt\"><p>", "</div>"))
                        pageVariables.ucVariable = sourceFile.getUntil("</p>");

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2096 Legare

            case 2097:  // Longpre
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"entry-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"post-content-inner\"><p", "</article>"))
                        pageVariables.ucVariable = sourceFile.getUntil("</p>");

                    if (sourceFile.moveTo("<p", 10))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2097 Longpre

            case 2098:  // Lanaudiere
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"post_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"post_info_date\"", "</div>"))
                        pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2098 Lanaudiere

            case 2100:  // Volontos
            {
                while (sourceFile.consecutiveMovesTo(150, "article class=\"post-", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"passing\"", "</article>", 1))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2100 Volontos

            case 2101:  // Wilbrod
            {
                pageVariables.numericDateOrder = doYMD;

                while (sourceFile.consecutiveMovesTo(150, "<section>", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"date\"", "</section>", 1))
                        pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2101 Wilbrod

            case 2102:  // Hodges
            {
                while (sourceFile.moveTo("<!--POST HEADER-->"))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read DOP
                    if (sourceFile.consecutiveMovesTo(50, "class=\"published\"", "datetime="))
                    {
                        pageVariables.ucPubDate = sourceFile.readNextBetween(QUOTES);
                        pageVariables.numericDateOrder = doYMD;
                    }

                    // Read URL
                    if (sourceFile.consecutiveMovesTo(100, "class=\"entry-title\"", "href="))
                    {
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                sourceFile.beg();
                sourceFile.moveTo("<!--PAGINATION-->");
                sourceFile.consecutiveMovesTo(100, "class=\"next\"", "offset=");
                flowParameters.lastValueParam = QString("=") + sourceFile.getUntil("\"").getString();

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2102 Hodges

            case 2103:  // Bergeron
            {
                if (flowParameters.initialSetup)
                {
                    while (sourceFile.consecutiveMovesTo(50, "class=\"btn\"", "Avis de décès complet"))
                    {
                        record.clear();
                        pageVariables.reset();

                        // Read URL
                        sourceFile.moveBackwardTo("href=");
                        pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        if (sourceFile.moveBackwardTo("</h", 3000))
                            pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                        sourceFile.moveTo("<div class=\"fiche\">");

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    while (sourceFile.consecutiveMovesTo(50, "<h3><span>", "</h"))
                    {
                        record.clear();
                        pageVariables.reset();

                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                        // Read URL
                        sourceFile.moveTo("data-href=");
                        pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2103 Bergeron

            case 2104:  // Passage
            {
                pageVariables.numericDateOrder = doMDY;

                while (sourceFile.consecutiveMovesTo(50, "<td>", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.consecutiveMovesTo(300, "</span>", "<span", ">"))
                    {
                        sourceFile.backward(1);
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2104 Passage

            case 2105:  // Granit
            {
                while (sourceFile.consecutiveMovesTo(75, "<div id=\"box_avis\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("class=\"annee_nais_deces\"", "<article", 0))
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2105 Granit

            case 2106:  // Affordable
            {
                while (sourceFile.consecutiveMovesTo(75, "class=\"entry-image-link\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2106 Affordable

            case 2107:  // LFC
            {
                while (sourceFile.moveTo("class=\"announced-text\""))
                {
                    record.clear();
                    pageVariables.reset();

                    if (sourceFile.conditionalMoveTo("<span", "href=", 0))
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                    // Read URL
                    sourceFile.moveTo("href=");
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord || flowParameters.initialSetup) && ((pageVariables.yod == 0) || pageVariables.yod >= 2020))
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2107 LFC

            case 2108:  // Life Transitions
            {
                while (sourceFile.consecutiveMovesTo(50, "target=\"_blank\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    temp = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.webAddress = temp;
                    temp.dropRight(10); // /index.php
                    pageVariables.ID = temp.readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("date-text", "target=\"_blank\"", 0))
                    {
                        sourceFile.moveTo("<h");
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2108 Transitions

            case 2109:  // Davis
            {
                sourceFile.moveTo("option value=");
                sourceFile.forward(1);
                while (sourceFile.moveTo("option value="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2109 Davis

            case 2110:  // MacLark
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"profile-image\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase +  sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOB - DOD
                    if (sourceFile.conditionalMoveTo("class=\"small text-muted\"", "</item>", 0))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2110 MacLark

            case 2111:  // Fallis
            {
                while (sourceFile.consecutiveMovesTo(350, "class=\"padded center\"", "class=\"bold\">", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase +  sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOB - DOD
                    if (sourceFile.conditionalMoveTo("<div", "class=\"padded center\"", 0))
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2111 Fallis

            case 2112:  // Timiskaming
            {
                while (sourceFile.moveTo("class=\"notice-infos\""))
                {
                    record.clear();
                    pageVariables.reset();

                    sourceFile.moveBackwardTo("<li>");
                    sourceFile.moveTo("href=");

                    // Read URL
                    pageVariables.webAddress = URLbase +  sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("class=\"notice-location\"", "</div>", 0))
                    {
                        sourceFile.moveBackwardTo("<br>", 100);
                        sourceFile.backward(6);
                        sourceFile.moveBackwardTo("<br", 100);
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2112 Timiskaming

            case 2113:  // Garrett
            {
                if (flowParameters.initialSetup)
                    sourceFile.JSONsimplify();

                while (sourceFile.conditionalMoveTo("class=\"nxs-title nxs-applylinkvarcolor", "<!-- END content -->", 2))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    sourceFile.moveTo("href=", 50);
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars().unescapeJSONformat();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2113 Garrett

            case 2114:  // Smith
            {
                while (sourceFile.consecutiveMovesTo(15, "<td>", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("<em", "<td><a href=", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2114 Smith

            case 2115:  // Picard
            {
                while (sourceFile.moveTo("<div class=\"hidden\""))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    sourceFile.moveBackwardTo("<h3");
                    temp = sourceFile.readNextBetween(BRACKETS);
                    pageVariables.webAddress = downloadRequest.instructions.url;
                    pageVariables.ID = temp.convertToID();
                    sourceFile.moveTo("</div>");

                    // YOB-YOD
                    if (temp.right(1) == PQString(")"))
                    {
                        temp = temp.right(11);
                        if (temp.removeBookEnds(PARENTHESES))
                            pageVariables.ucVariable = temp;
                    }
                    else
                    {
                        temp = temp.right(9);
                        if (temp.middle(4,1) == PQString("-"))
                            pageVariables.ucVariable = temp;
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2115 Picard

            case 2116:  // Richelieu
            {
                while (sourceFile.consecutiveMovesTo(300, "<div class=\"news-list\">", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2116 Richelieu

            case 2117:  // Roy
            {
                while (sourceFile.moveTo("!important;\"><a href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // YOB - YOD
                    if (sourceFile.conditionalMoveTo("</p>", "!important;\"><a href=", 0))
                    {
                        sourceFile.moveBackwardTo("<p", 25);
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2117 Roy

            case 2118:  // Charlevoix
            {
                while (sourceFile.consecutiveMovesTo(75, "id=\"middle\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // YOB - YOD
                    if (sourceFile.conditionalMoveTo("</a><br", "</div>", 0))
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; //  2118 Charlevoix

            case 2119:  // Aurora
            {
                while (sourceFile.consecutiveMovesTo(400, "class=\"profile-details", "</div>"))
                {
                    record.clear();
                    pageVariables.reset();

                    // DOD
                    sourceFile.moveBackwardTo("<br", 50);
                    pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Read URL
                    sourceFile.consecutiveMovesTo(30, "class=card-body", "href=");
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    int index1 = pageVariables.webAddress.getString().lastIndexOf("/", -2);
                    int index2 = pageVariables.webAddress.getString().lastIndexOf("/", -static_cast<int>(pageVariables.webAddress.getLength() - index1 + 1));
                    index2++;
                    pageVariables.ID = pageVariables.webAddress.middle(index2, index1 - index2).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; //  2119 Aurora

            case 2120:  // Montcalm
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"post_title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // YOB - YOD
                    temp = pageVariables.webAddress.right(9);
                    temp.dropRight(1);
                    if (temp.isNumeric())
                        pageVariables.ucVariable = temp.left(4) + PQString("-") + temp.right(4);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; //  2120 Montcalm

            case 2122:  // Laurent
            {
                while (sourceFile.moveTo("class=\"avis\""))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    if (sourceFile.moveBackwardTo("href=", 50))
                    {
                        pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // DOD
                        if (sourceFile.conditionalMoveTo("Décédé(e) le ", "class=\"avis\"", 0))
                            pageVariables.ucDOD = sourceFile.getUntil("<");
                        else
                            sourceFile.moveTo("</a>");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; //  2122 Laurent

            case 2123:  // Eternal
            {
                while (sourceFile.consecutiveMovesTo(12, "<li>", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // YOB - YOD
                    if (sourceFile.conditionalMoveTo("class=\"annee\"", "</li>", 0))
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                    if (sourceFile.conditionalMoveTo("décédé le ", "</li>", 0))
                    {
                        sourceFile.getWord();
                        pageVariables.ucDOD = sourceFile.getUntil(("<"));
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; //  2123 Eternal

            case 2124:  // Ruel
            {
                if (flowParameters.initialSetup)
                {
                    sourceFile.moveTo("class=\"main-wrap\"");

                    while (sourceFile.consecutiveMovesTo(100, "class=\"wsite-content-title\"", ">"))
                    {
                        record.clear();
                        pageVariables.reset();

                        if (sourceFile.conditionalMoveTo("<font", "</h"))
                            sourceFile.moveTo(">");

                        // Read URL
                        pageVariables.webAddress = downloadRequest.instructions.url;
                        temp = sourceFile.getUntil("<");
                        pageVariables.ID = temp.convertToID();

                        // YOB - YOD
                        if (temp.middle(temp.getLength() - 5, 1) == PQString("-"))
                            pageVariables.ucVariable = temp.right(9);

                        if (temp.middle(temp.getLength() - 6, 1) == PQString("-"))
                        {
                            temp = temp.right(11);
                            temp.removeBookEnds(PARENTHESES);
                            pageVariables.ucVariable = temp;
                        }

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                }
                else
                {
                    sourceFile.moveTo("wsite-nav-377535599353310750");
                    while (sourceFile.conditionalMoveTo("href=", "wsite-nav-149061948690503014", 0))
                    {
                        pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES);
                        pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                        // Process
                        process(record, pageVariables, lang);
                        if (pageVariables.includeRecord || flowParameters.initialSetup)
                            records.append(record);
                    }
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; //  2124 Ruel

            case 2125:  // Hamel
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"wpfh_obit_thumbnail_dates\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; //  2125 Hamel

            case 2126:  // CremAlt
            case 2129:  // Forest
            case 2137:  // Tricity
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"blog-title-link", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = HTTP + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("class=\"date-text\"", "</div>", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; //  2126 CremAlt  2129 Dryden

            case 2127:  // London
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"listing\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; //  2127 London

            case 2128:  // Dryden
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"post-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("class=\"meta-data\"", "class=\"post-title\"", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; //  2128 Dryden

            case 2130:  // JasonSmith
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"post-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("class=\"meta-data\"", "class=\"post-title\"", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; //  2130 JasonSmith

            case 2131:  // Lampman
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"single-post-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; //  2131 Lampman

            case 2132:  // ecoPassages
            {
                while (sourceFile.moveTo("class=\"blog-date\""))
                {
                    record.clear();
                    pageVariables.reset();
                    pageVariables.numericDateOrder = doMD20Y;

                    // Read DOP
                    pageVariables.ucPubDate = sourceFile.readNextBetween(BRACKETS);

                    // Read URL
                    sourceFile.consecutiveMovesTo(100, "class=\"blog-title\"", "href=");
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOB - DOD
                    if (sourceFile.conditionalMoveTo("class=\"blog-excerpt-wrapper\"", "class=\"blog-date\"", 0))
                    {
                        sourceFile.consecutiveMovesTo(100, "<p", ">");
                        pageVariables.ucDOBandDOD = sourceFile.getUntil("<");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                sourceFile.beg();
                sourceFile.moveTo("class=\"blog-list-pagination\"");
                sourceFile.conditionalMoveTo("class=\"older\"", "</svg>");
                sourceFile.moveTo("offset=", 150);
                flowParameters.lastValueParam = QString("=") + sourceFile.getUntilEarliestOf("\"","&").getString();

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2132 ecoPassages

            case 2133:  // Peaceful
            {
                while (sourceFile.consecutiveMovesTo(75, "class=\"mega-post-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Read DOP
                    if (sourceFile.conditionalMoveTo("class=\"mega-post-date\"", "class=\"mega-post-title\"", 0))
                    {
                        sourceFile.consecutiveMovesTo(75, "</i>", ", ");
                        pageVariables.ucPubDate = sourceFile.getUntil("<");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2133 Peaceful

            case 2134:  // Ranger
            {
                while (sourceFile.consecutiveMovesTo(75, "class=\"post-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2134 Ranger

            case 2135:  // People
            {
                while (sourceFile.consecutiveMovesTo(15, "<li>", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    pageVariables.ID.chop(4);  //.htm

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2135 People

            case 2136:  // Whitcroft
            {
                QList<QDate> dateList;
                int numDates;
                OQString cleanString;

                while (sourceFile.consecutiveMovesTo(25, "<td>", "href="))
                {
                    record.clear();
                    pageVariables.reset();
                    dateList.clear();
                    cleanString.clear();

                    // Read URL
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    temp = sourceFile.readNextBetween(BRACKETS);
                    numDates = temp.pullOutEnglishDates(dateList, 1, cleanString, false, false);
                    if (numDates == 1)
                        pageVariables.currentDOD = dateList[0];

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.webAddress.getString().indexOf("Notices.htm") > 0)
                        pageVariables.includeRecord = false;
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2136 Whitcroft

            case 2138:  // LegacyCardstrom
            {
                while (sourceFile.consecutiveMovesTo(150, "class=\"wsite-caption\"", "class=\"paragraph\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();
                    pageVariables.ID.chop(5);   // .html

                    // DOD
                    if (sourceFile.moveTo("<br /", 50))
                        pageVariables.ucDOBandDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2138 Cardstrom

            case 2139:  // Weibe
            {
                while (sourceFile.consecutiveMovesTo(25, "class=\"list-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2139 Weibe

            case 2140:  // Arimathea
            {
                while (sourceFile.consecutiveMovesTo(400, "class=\"entry-header\"", "class=\"dt-published", "datetime="))
                {
                    record.clear();
                    pageVariables.reset();
                    pageVariables.numericDateOrder = doYMD;

                    // Read DOD
                    pageVariables.ucDOD = sourceFile.readNextBetween(QUOTES);

                    // Read URL
                    sourceFile.moveTo("href=");
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                sourceFile.beg();
                sourceFile.moveTo("class=\"pagination");
                sourceFile.conditionalMoveTo("class=\"older\"", "</nav>");
                sourceFile.moveTo("offset=", 150);
                flowParameters.lastValueParam = QString("=") + sourceFile.getUntilEarliestOf("\"","&").getString();

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2140 Arimathea

            case 2141:  // GFournier
            {
                while (sourceFile.consecutiveMovesTo(25, "class=\"article-infobox\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("class=\"date\"", "</div>", 0))
                    {
                        sourceFile.moveTo(" ");
                        sourceFile.moveTo(" ");
                        pageVariables.ucDOD = sourceFile.getUntil("<");
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2141 GFournier

            case 2143:  // Harmonia
            {
                while (sourceFile.consecutiveMovesTo(25, "<td>", "<div>", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("class=\"date\"", "</div>", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2143 Harmonia

            case 2144:  // Omega
            {
                QRegularExpression rxTarget;
                rxTarget.setPattern(">\\d{4}-\\d{4}<");
                index = 0;
                index = sourceFile.getString().indexOf(rxTarget, index);

                while (index != -1)
                {
                    record.clear();
                    pageVariables.reset();

                    // YOB - YOD
                    sourceFile.beg();
                    sourceFile.forward(index + 1);
                    pageVariables.ucVariable = sourceFile.getUntil("<");

                    // Read URL
                    sourceFile.backward(10);
                    sourceFile.moveBackwardTo("</p>");
                    sourceFile.backward(10);
                    sourceFile.moveBackwardTo(">");
                    pageVariables.webAddress = downloadRequest.instructions.url;
                    pageVariables.ID = sourceFile.getUntil("<").convertToID();
                    index = sourceFile.getString().indexOf(rxTarget, index + 10);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2144 Omega

            case 2145:  // HeritageWP
            {
                while (sourceFile.consecutiveMovesTo(75, "class=\"media-responsive post-meta\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("style=\"font-weight: normal;\"", "class=\"content\"", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2145 HeritageWP

            case 2146:  // Ouellet
            {
                while (sourceFile.consecutiveMovesTo(15, "<p>", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = URLbase + PQString("/") + sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    temp = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider);
                    temp.removeEnding(".html");
                    pageVariables.ID = temp.getString();

                    // DOD
                    if (sourceFile.moveTo("target=", 10))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2146 Ouellet

            case 2147:  // HommageNB
            {
                while (sourceFile.consecutiveMovesTo(100, "class=\"well-default\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // YOB - YOD
                    if (sourceFile.conditionalMoveTo("title=\"Obituaries Title\"", "class=\"well-default\"", 0))
                    {
                        sourceFile.moveTo("</h");
                        temp = sourceFile.readNextBetween(BRACKETS);
                        temp.cleanUpEnds();
                        temp = temp.right(13);
                        if (temp.removeBookEnds(PARENTHESES))
                            pageVariables.ucVariable = temp.getString();
                    }

                    // Age at Death
                    if (sourceFile.conditionalMoveTo("class=\"obituaries-age\"", "class=\"well-default\"", 0))
                    {
                        if(sourceFile.moveTo("(Age "))
                            pageVariables.ageAtDeath = static_cast<int>(sourceFile.getUntil(")").asNumber());
                    }

                    // DOD
                    if (sourceFile.conditionalMoveTo("Date de décès <p>(", "class=\"well-default\"", 0))
                        pageVariables.ucDOD = sourceFile.getUntil(")");

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2147 HommageNB

            case 2148:  // Drake
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"entry-title-link\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2148 Drake

            case 2149:  // CityLine
            {
                while (sourceFile.consecutiveMovesTo(100, "<!-- <div class='catelinenolink'></div><br />-->", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // YOB - YOD
                    if (sourceFile.conditionalMoveTo("class='tributepostdata'", "<!-- <div class='catelinenolink'></div><br />-->", 0))
                        pageVariables.ucVariable = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                URLaddressTemplate = URLbase + PQString("/template/modules/tribute/frontend/ajax/loadmore.php?lastcount=20&datacat=");
                downloadRequest.instructions.verb = QString("POST");
                createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2149 CityLine

            case 2150:  // Komitas
            {
                while (sourceFile.consecutiveMovesTo(150, "class=\"obt-block-main\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("<span", "class=\"obt-block-main\"", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2150 Komitas

            case 2151:  // Driftwood
            {
                while (sourceFile.consecutiveMovesTo(50, "class=\"entry-title td-module-title\"", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // Publish Date
                    if (sourceFile.conditionalMoveTo("datetime=\"", "class=\"entry-title td-module-title\"", 0))
                        pageVariables.ucPubDate = sourceFile.getUntil("T");

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2151 Driftwood

            case 2152:  // MLBW (see also 1052 Citrus)
            {
                sourceFile.JSONsimplify();
                sourceFile.unQuoteHTML();

                while (sourceFile.consecutiveMovesTo(1000, "jet-listing-grid__item", "data-raven-link="))
                {
                    record.clear();
                    pageVariables.reset();

                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();;
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    if (sourceFile.conditionalMoveTo("1309f6a", "jet-listing-grid__item", 0))
                    {
                        sourceFile.consecutiveMovesTo(200, "<span", ">");
                        sourceFile.backward(1);
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);
                    }

                    if (sourceFile.conditionalMoveTo("d0d63de", "jet-listing-grid__item", 0))
                    {
                        sourceFile.consecutiveMovesTo(200, "<span", ">");
                        sourceFile.backward(1);
                        pageVariables.pcKey = sourceFile.readNextBetween(BRACKETS).getString();
                    }

                    // Process
                    process(record, pageVariables, lang);
                    if ((pageVariables.includeRecord) || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);

            }   // end case 2152 MLBW
                break;

            case 2153:  // Sproing
            {
                while (sourceFile.consecutiveMovesTo(150, "js-img-obj-fit__container", "href="))
                {
                    record.clear();
                    pageVariables.reset();

                    // Read URL
                    pageVariables.webAddress = sourceFile.readNextBetween(QUOTES).replaceIneligibleURLchars();
                    pageVariables.ID = OQString(pageVariables.webAddress).readURLparameter(fhURLid, fhURLidDivider).getString();

                    // DOD
                    if (sourceFile.conditionalMoveTo("small dod-2", "js-img-obj-fit__container", 0))
                        pageVariables.ucDOD = sourceFile.readNextBetween(BRACKETS);

                    // Process
                    process(record, pageVariables, lang);
                    if (pageVariables.includeRecord || flowParameters.initialSetup)
                        records.append(record);
                }

                updateFlowParameters(flowParameters, pageVariables, downloadRequest, URLaddressTemplate, URLparams);
            }
                break; // 2153 Sproing


            default:
                qDebug() << "Attempt to retrieve URLs for unknown/uncoded provider";
                return;

            } // end of switch

        } // end of processing downloaded file
        else
        {
            if (www->redirectURL.getLength() > 0)
            {
                URLaddress = www->redirectURL;
                downloadRequest.instructions.qUrl = www->redirectURL.getString();
            }
            else
                flowParameters.keepDownloading = false;
        }

        // Failsafe coding to prevent useless looping
        if (records.size() == flowParameters.numRecordsRead)
        {
            flowParameters.numBlankDownloads++;
            if (flowParameters.numBlankDownloads >= flowParameters.numBlankDownloadLimit)
                flowParameters.keepDownloading = false;
        }
        else
        {
            flowParameters.numBlankDownloads = 0;
            flowParameters.numRecordsRead = records.size();
        }

    } // end of downloading loop

    // Write output to appropriate file
    outputFile += PQString(" Back To ");
    outputFile += lastDate;
    outputFile += PQString(".obitURLlist");

    if (records.size() > 0)
    {
        removeDuplicates(records);
        numRecsFound = records.size();
        globals->output = new QFile((globals->batchDirectory + PQString("\\") + outputFile).getString());
        if(globals->output->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream outputStream(globals->output);
            QString divider("|");

            while (records.size() > 0)
            {
                record = records.takeFirst();
                outputStream << record.providerID << divider << record.providerKey << divider << record.url << divider << record.POSTformRequest << divider;
                outputStream << record.dob.toString("yyyyMMdd") << divider << record.yob << divider << record.dod.toString("yyyyMMdd") << divider << record.yod << divider;
                outputStream << record.maidenNames << divider << record.ID << divider << record.pubdate.toString("yyyyMMdd") << divider << record.ageAtDeath << divider;
                outputStream << record.pcKey << divider << Qt::endl;
            }
        }
    }
}

void MINER::sortRecords(QList<RECORD> &records)
{
    int numRecs = records.size();

    for (int i = 0; i < (numRecs - 1); i++)
    {
        for (int j = 0; j < (numRecs - 1 - i); j++)
        {
            if (records[j].dod.isValid() && !records[j+1].dod.isValid())
                records.move(j, j+1);
            else
            {
                if (records[j].dod.isValid() && records[j+1].dod.isValid() && (records[j].dod < records[j+1].dod))
                    records.move(j, j+1);
            }
        }
    }
}

void MINER::removeDuplicates(QList<RECORD> &records)
{
    int numRecs = records.size();

    for (int i = 0; i < (numRecs - 1); i++)
    {
        for (int j = 0; j < (numRecs - 1 - i); j++)
        {
            if (records[j].url < records[j+1].url)
                records.move(j, j+1);
        }
    }

    int i = 0;
    while (i < (records.size() - 1))
    {
        if ((records[i].url.compare(records[i+1].url) == 0) && (records[i].ID.compare(records[i+1].ID) == 0))
            records.removeAt(i + 1);
        else
            i++;
    }
}

void MINER::process(RECORD &record, PAGEVARIABLES &pageVariables, LANGUAGE &lang)
{
    QDate tempDate;
    QList<QDate> dateList;
    OQString cleanString;
    QUrl checkURL;
    unsigned int numDates;
    bool validDOD;
    databaseSearches dbSearch;

    pageVariables.webAddress.removeEnding(COMMA);
    checkURL = pageVariables.webAddress.getString();
    if (!checkURL.isValid())
    {
        pageVariables.includeRecord = false;
        return;
    }

    record.providerID = pageVariables.providerID;
    record.providerKey = pageVariables.providerKey;
    record.url = pageVariables.webAddress.getString();
    record.maidenNames = pageVariables.maidenName.getString();
    record.ID = pageVariables.ID.left(85);
    record.yob = pageVariables.yob;
    record.yod = pageVariables.yod;
    record.ageAtDeath = pageVariables.ageAtDeath;
    record.POSTformRequest = pageVariables.POSTformRequest;
    record.pcKey = pageVariables.pcKey;

    // Read in published date if available
    if (pageVariables.ucPubDate.getLength() > 0)
    {
        pageVariables.ucPubDate.cleanUpEnds();
        pageVariables.ucPubDate.setContentLanguage(lang);
        pageVariables.ucPubDate.setGlobalVars(*globals);
        tempDate = pageVariables.ucPubDate.readDateField(pageVariables.numericDateOrder);
        if (tempDate.isValid())
            pageVariables.currentPubDate = tempDate;
    }
    if (pageVariables.currentPubDate.isValid() && (pageVariables.currentPubDate > QDate(1900,1,1)))
        record.pubdate = pageVariables.currentPubDate;

    // Decide whether or not to include record
    if (record.url.right(20) == QString("Margaret-Holzleitner"))
    {
        int dummyBreak = 0;
        dummyBreak++;
    }

    if (dbSearch.deceasedRecordExists(record.providerID, record.providerKey, record.ID, record.url, record.pubdate))
    {
        pageVariables.includeRecord = false;
        return;
    }

    // If proceeding, fill in remaining info
    if ((pageVariables.ucDOBandDOD.getLength() > 0) || (pageVariables.ucVariable.getLength() > 0))
    {
        if (pageVariables.ucDOBandDOD.getLength() > 0)
        {
            pageVariables.ucDOBandDOD.replaceHTMLentities();
            pageVariables.ucDOBandDOD.removeHTMLtags();
            pageVariables.ucDOBandDOD.unQuoteHTML();
            pageVariables.ucDOBandDOD.cleanUpEnds();
            pageVariables.ucDOBandDOD.simplify();
            pageVariables.ucDOBandDOD.fixBasicErrors();
            pageVariables.ucDOBandDOD.setContentLanguage(lang);
            pageVariables.ucDOBandDOD.setGlobalVars(*globals);

            if ((pageVariables.ucDOBandDOD.getLength() == 9) && (pageVariables.ucDOBandDOD.middle(4,1) == QString("-")))
            {
                record.yob = pageVariables.ucDOBandDOD.left(4).getString().toUInt();
                record.yod = pageVariables.ucDOBandDOD.right(4).getString().toUInt();
                numDates = 0;
            }
            else
            {
                // Read DOB and DOD - Default is for alpha dates
                if (pageVariables.numericDateOrder == doNULL)
                    numDates = pageVariables.ucDOBandDOD.pullOutDates(lang, dateList, 3, cleanString, true);
                else
                {
                    numDates = 0;
                    DATES tempDates = pageVariables.ucDOBandDOD.readNumericDates(pageVariables.numericDateOrder);
                    if (tempDates.hasDateInfo())
                    {
                        if (tempDates.potentialDOB.isValid())
                        {
                            dateList.append(tempDates.potentialDOB);
                            numDates++;
                        }

                        if (tempDates.potentialDOD.isValid())
                        {
                            dateList.append(tempDates.potentialDOD);
                            numDates++;
                        }
                    }
                }
            }
        }
        else
        {
            // ucVariable has unstructed data instead
            pageVariables.ucVariable.replaceHTMLentities();
            pageVariables.ucVariable.removeHTMLtags();
            pageVariables.ucVariable.unQuoteHTML();
            pageVariables.ucVariable.cleanUpEnds();
            pageVariables.ucVariable.simplify();
            pageVariables.ucVariable.fixBasicErrors();
            pageVariables.ucVariable.removeBookEnds();
            pageVariables.ucVariable.setContentLanguage(lang);
            pageVariables.ucVariable.setGlobalVars(*globals);

            // Read DOB and DOD - Default is for alpha dates
            numDates = pageVariables.ucVariable.pullOutDates(lang, dateList, 2, cleanString, false);
        }

        if ((numDates >= 1) && (numDates <= 2))
        {
            if (numDates == 1)
            {
                if (pageVariables.alphabeticalListing)
                {
                    if (dateList[0] < pageVariables.firstAvailableDate)
                        pageVariables.currentDOB = dateList[0];
                    else
                        pageVariables.currentDOD = dateList[0];
                }
                else
                {
                    if (dateList[0].daysTo(pageVariables.priorDODdate) < 60)
                        pageVariables.currentDOD = dateList[0];
                    else
                        pageVariables.currentDOB = dateList[0];
                }
            }
            else
            {
                pageVariables.currentDOB = dateList[0];
                pageVariables.currentDOD = dateList[1];
            }
        }
        else
        {
            if ((numDates == 0) || (numDates >= 9900))
            {
                DATES dates;
                dates = pageVariables.ucVariable.readYears();
                if ((dates.potentialYOB > 0) && (dates.potentialYOD > 0))
                {
                    record.yob = dates.potentialYOB;
                    record.yod = dates.potentialYOD;
                }
                else
                {
                    // Look for December 1961 - March 2021
                    OQString temp;
                    if (pageVariables.ucDOBandDOD.getLength() > 0)
                        temp = pageVariables.ucDOBandDOD;
                    else
                    {
                        if (pageVariables.ucVariable.getLength() > 0)
                            temp = pageVariables.ucVariable;
                    }
                    temp.removeWrittenMonths();
                    dates = unstructuredContent(temp).readYears();
                    if ((dates.potentialYOB > 0) && (dates.potentialYOD > 0))
                    {
                        record.yob = dates.potentialYOB;
                        record.yod = dates.potentialYOD;
                    }
                }
            }
        }
    }    

    // Process DOB
    if (pageVariables.ucDOB.getLength() > 0)
    {
        pageVariables.ucDOB.replaceHTMLentities();
        pageVariables.ucDOB.removeHTMLtags();
        pageVariables.ucDOB.unQuoteHTML();
        pageVariables.ucDOB.cleanUpEnds();
        pageVariables.ucDOB.simplify();
        pageVariables.ucDOB.fixBasicErrors();
        pageVariables.ucDOB.setContentLanguage(lang);
        pageVariables.ucDOB.setGlobalVars(*globals);

        tempDate = pageVariables.ucDOB.readDateField(pageVariables.numericDateOrder);
        if (tempDate.isValid())
            pageVariables.currentDOB = tempDate;
    }

    // Process DOD
    if (pageVariables.ucDOD.getLength() > 0)
    {
        pageVariables.ucDOD.replaceHTMLentities();
        pageVariables.ucDOD.removeHTMLtags();
        pageVariables.ucDOD.unQuoteHTML();
        pageVariables.ucDOD.cleanUpEnds();
        pageVariables.ucDOD.simplify();
        pageVariables.ucDOD.fixBasicErrors();
        pageVariables.ucDOD.setContentLanguage(lang);
        pageVariables.ucDOD.setGlobalVars(*globals);

        tempDate = pageVariables.ucDOD.readDateField(pageVariables.numericDateOrder);
        if (tempDate.isValid())
            pageVariables.currentDOD = tempDate;
    }

    // Will capture data read in directly from JSON file
    if (pageVariables.currentDOB.isValid())
    {
        record.dob = pageVariables.currentDOB;
        if (pageVariables.currentDOB != QDate(1900,1,1))
            record.yob = pageVariables.currentDOB.year();
        else
        {
            if (record.yob == 0)
                record.yob = 1900;
        }
        pageVariables.yob = record.yob;
    }

    if (pageVariables.currentDOD.isValid() && (pageVariables.currentDOD <= globals->today))
    {
        record.dod = pageVariables.currentDOD;
        if (pageVariables.currentDOD != QDate(1900,1,1))
            record.yod = pageVariables.currentDOD.year();
        else
        {
            if (record.yod == 0)
                record.yod = 1900;
        }
        pageVariables.yod = record.yod;

        if (pageVariables.currentDOD > QDate(1900,1,1))
            pageVariables.priorDODdate = pageVariables.currentDOD;

        if (pageVariables.currentDOD > pageVariables.latestDODdate)
        {
            pageVariables.latestDODdate = pageVariables.currentDOD;
            pageVariables.lastDate = pageVariables.currentDOD.toString("yyyyMMdd");
        }
    }

    if ((record.ageAtDeath == 0) && pageVariables.currentDOB.isValid() && pageVariables.currentDOD.isValid() && (pageVariables.currentDOB > QDate(1900,1,1)) && (pageVariables.currentDOD <= globals->today))
    {
        record.ageAtDeath = static_cast<unsigned int>(elapse(pageVariables.currentDOB, pageVariables.currentDOD));
        if ((record.ageAtDeath < 0) || (record.ageAtDeath > 125))
            record.ageAtDeath = 0;
    }

    // Determine whether or not to include the record
    validDOD = pageVariables.currentDOD.isValid() && (pageVariables.currentDOD > QDate(1900,1,1));
    pageVariables.includeRecord = false;

    if (validDOD && (pageVariables.currentDOD >= pageVariables.cutoffDate))
        pageVariables.includeRecord = true;
    else
    {
        if (pageVariables.sequentialID)
        {
            if((record.ID.toInt() > pageVariables.cutOffID.toInt()))
                pageVariables.includeRecord = true;
        }
        else
        {
            if (!validDOD)
                pageVariables.includeRecord = true;
        }
    }

    // Deal with problematic records that always appear
    if ((record.ID == QString("archives")) || (record.ID.length() == 0))
    {
        PQString errorMsg;
        errorMsg << "Rejected record: " << pageVariables.webAddress.getString();
        globals->logMsg(ErrorConnection, errorMsg);
        pageVariables.includeRecord = false;
    }
}

void MINER::updateFlowParameters(FLOWPARAMETERS &flowParameters, PAGEVARIABLES &pageVariables, DOWNLOADREQUEST &downloadRequest, PQString &URLaddressTemplate, URLPARAMS &URLparams)
{
    // Update parameters first, then determine whether or not to continue
    // Some providers (eg. 1005) have a startToEnd loop with an overall sequential approach - handled with paginated field

    // Update parameters
    switch (flowParameters.flowType)
    {
    case startToEnd:
        flowParameters.currentPosition = static_cast<unsigned int>(static_cast<int>(flowParameters.currentPosition) + flowParameters.flowIncrement);
        break;

    case sequential:
        if (pageVariables.paginatedResult)
        {
            flowParameters.currentPosition = static_cast<unsigned int>(static_cast<int>(flowParameters.currentPosition) + 1);

            if ((flowParameters.currentPosition + flowParameters.indexOffset) >= flowParameters.endingPosition)
            {
                flowParameters.currentPosition = flowParameters.initialPosition;
                flowParameters.endingPosition = flowParameters.initialPosition;
                pageVariables.paginatedResult = false;
                pageVariables.queryTargetDate = pageVariables.queryTargetDate.addMonths(flowParameters.flowIncrement);
            }
        }
        else
        {
            if (pageVariables.providerID == 1080)
                pageVariables.queryTargetDate = pageVariables.queryTargetDate.addDays(flowParameters.flowIncrement);
            else
                pageVariables.queryTargetDate = pageVariables.queryTargetDate.addMonths(flowParameters.flowIncrement);
        }

        break;

    case singleListing:
        // Nothing to do here
        break;

    case dateRange:
        pageVariables.queryStartDate = pageVariables.queryStartDate.addDays(flowParameters.flowIncrement);
        pageVariables.queryEndDate   = pageVariables.queryEndDate.addDays(flowParameters.flowIncrement);
        break;

    case lastValue:
        flowParameters.currentPosition = pageVariables.ID.toUInt();
        break;

    case alphabetical:
        if (pageVariables.paginatedResult)
        {
            flowParameters.currentPosition = static_cast<unsigned int>(static_cast<int>(flowParameters.currentPosition) + 1);

            if ((flowParameters.currentPosition + flowParameters.indexOffset) > flowParameters.endingPosition)
            {
                flowParameters.currentPosition = 1;
                flowParameters.endingPosition = 1;
                flowParameters.letterCurrent++;
                pageVariables.paginatedResult = false;
                flowParameters.currentLetter = QString(QChar(flowParameters.letterCurrent));
            }
        }
        else
        {
            flowParameters.letterCurrent++;
            flowParameters.currentLetter = QString(QChar(flowParameters.letterCurrent));
        }
        break;

    case monthly:
        flowParameters.currentPosition = static_cast<unsigned int>(static_cast<int>(flowParameters.currentPosition) + 1);

        if ((flowParameters.currentPosition + flowParameters.indexOffset) > flowParameters.endingPosition)
        {
            flowParameters.currentPosition = 1;
            flowParameters.monthCurrent++;
            if (flowParameters.monthCurrent <= 12)
                flowParameters.currentMonth = PQString::MONTHS.at(flowParameters.monthCurrent);
        }
        break;
    }

    // Determine whether or not to continue
    bool stopDownloading = true;

    if (flowParameters.initialSetup)
    {
        switch (flowParameters.flowType)
        {
        case startToEnd:            
            if (flowParameters.flowIncrement > 0)
                stopDownloading = ((flowParameters.currentPosition + flowParameters.indexOffset) > flowParameters.endingPosition);
            else
                stopDownloading = ((flowParameters.currentPosition + flowParameters.indexOffset) < flowParameters.endingPosition);
            if (pageVariables.currentDOD > QDate(1900,1,1))
                stopDownloading = stopDownloading || (pageVariables.currentDOD < pageVariables.cutoffDate);
            break;

        case sequential:
            stopDownloading = (pageVariables.queryTargetDate < pageVariables.firstAvailableDate);
            break;

        case singleListing:
            stopDownloading = true;
            break;

        case dateRange:
            stopDownloading = (pageVariables.queryEndDate < pageVariables.firstAvailableDate);
            break;

        case lastValue:
            stopDownloading = (flowParameters.currentPosition < flowParameters.endingPosition);
            break;

        case alphabetical:
            stopDownloading = (flowParameters.letterCurrent > flowParameters.letterEnd);
            break;

        case monthly:
            stopDownloading = (flowParameters.monthCurrent > flowParameters.monthEnd);
            break;
        }
    }
    else
    {
        if (pageVariables.usePubDateForCutOff)
        {
            if (pageVariables.currentPubDate.isValid() && (pageVariables.currentPubDate >= pageVariables.cutoffDate))
                stopDownloading = false;

            switch(flowParameters.flowType)
            {
            case monthly:
                stopDownloading = (flowParameters.monthCurrent > flowParameters.monthEnd);
                break;

            default:
                break;
            }
        }
        else
        {
            switch (flowParameters.flowType)
            {
            case startToEnd:
                if (flowParameters.flowIncrement > 0)
                    stopDownloading = ((flowParameters.currentPosition + flowParameters.indexOffset) > flowParameters.endingPosition);
                else
                    stopDownloading = ((flowParameters.currentPosition + flowParameters.indexOffset) < flowParameters.endingPosition);
                if (!stopDownloading && (pageVariables.currentDOD > QDate(1900,1,1)))
                    stopDownloading = pageVariables.currentDOD < pageVariables.cutoffDate;
                if (!stopDownloading && PQString(pageVariables.ID).isNumeric() && PQString(pageVariables.cutOffID).isNumeric())
                    stopDownloading = pageVariables.ID < pageVariables.cutOffID;
                break;

            case sequential:
                stopDownloading = (pageVariables.queryTargetDate < pageVariables.firstAvailableDate);
                break;

            case singleListing:
                stopDownloading = true;
                break;

            case dateRange:
                stopDownloading = (pageVariables.queryEndDate < pageVariables.cutoffDate);
                break;

            case lastValue:
                stopDownloading = (flowParameters.currentPosition < flowParameters.endingPosition);
                break;

            case alphabetical:
                stopDownloading = (flowParameters.letterCurrent > flowParameters.letterEnd);
                break;

            case monthly:
                stopDownloading = (flowParameters.monthCurrent > flowParameters.monthEnd);
                break;
            }

            if (!stopDownloading && (flowParameters.flowType != singleListing))
            {
                if (pageVariables.cutoffDate.isValid() && (pageVariables.currentDOD > pageVariables.latestDODdate) && (pageVariables.cutoffDate != globals->today))
                    stopDownloading = stopDownloading || (pageVariables.currentDOD < pageVariables.cutoffDate);

                if (!stopDownloading && pageVariables.cutoffDate.isValid() && (pageVariables.cutoffDate != globals->today))
                {
                    stopDownloading = stopDownloading || (pageVariables.queryTargetDate < pageVariables.cutoffDate);
                    if (pageVariables.currentDOD > QDate(1900,1,1))
                        stopDownloading = stopDownloading || (pageVariables.currentDOD < pageVariables.cutoffDate);
                }
            }
        }
    }

    // Update parameters
    if (stopDownloading)
        flowParameters.keepDownloading = false;
    else
    {
        flowParameters.keepDownloading = true;
        createURLaddress(downloadRequest, URLaddressTemplate, URLparams);
    }
}

void MINER::determineCutOffDate(const int &daysOfOverlap, FLOWPARAMETERS &flowParameters, PAGEVARIABLES &pageVariables)
{
    databaseSearches dbSearch;

    if (flowParameters.initialSetup)
    {
        pageVariables.cutoffDate = pageVariables.firstAvailableDate;
        pageVariables.cutOffID = QString("0");
    }
    else
    {
        if (pageVariables.usePubDateForCutOff)
        {
            pageVariables.cutoffDate = dbSearch.getLastPubDate(globals, (PROVIDER)pageVariables.providerID, pageVariables.providerKey);
            pageVariables.cutoffDate = pageVariables.cutoffDate.addDays(-daysOfOverlap);

            if ((!pageVariables.cutoffDate.isValid()) || (pageVariables.cutoffDate < QDate(2019,1,1)))
            {
                pageVariables.usePubDateForCutOff = false;
                determineCutOffDate(daysOfOverlap, flowParameters, pageVariables);
            }
        }
        else
        {
            if (pageVariables.latestDODdate.isValid() && (pageVariables.latestDODdate > QDate(1900,1,1)))
                pageVariables.cutoffDate = pageVariables.latestDODdate.addDays(-daysOfOverlap);
            else
                pageVariables.cutoffDate = pageVariables.firstAvailableDate;

            if (pageVariables.sequentialID)
            {
                pageVariables.cutOffID = dbSearch.IDforLastDeathPreceding(pageVariables.cutoffDate, pageVariables.providerID, pageVariables.providerKey, globals);
                if ((pageVariables.cutOffID.length() == 0) || (pageVariables.cutOffID.left(1) == QString("9")))  // Must exclude 9s for double recs with made up ID
                    pageVariables.cutOffID = QString("0");
            }
        }
    }
}

