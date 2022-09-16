#include "Include/dataMiningFH.h"

MINER::MINER(QObject *parent) : QObject(parent)
{
    // get the instance of the main application
    app = QCoreApplication::instance();

    // any other additional setup here
}

void MINER::execute()
{
    qDebug() << "Data mining has started";

   mineFHdata();

   quit();
}

void MINER::mineFHdata()
{
    mineLegacyData();
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

void MINER::mineLegacyData()
{
    // Mining the funeral home data takes a number of steps
    // 1. Make list of all funeral homes province by province
    // 2. Visit legacy website for each funeral home to retrieve its URL address
    // 3. Determine through SQL query if it's a new or old website
    // 4. Visit website all pull list of obituaries (new = all after setup, last 7 days otherwise)
    // 5. Read information from list of obituaries to read
    // 6. Compare obituaries to existing database entries and add new ones

    PROVIDER listerID = Legacy;
    //PROVIDER listerID = static_cast<PROVIDER>(2); // AFHA

    unsigned int numFuneralHomes, page;
    unsigned int queryCount = 0;    // used to limit queries while testing
    unsigned int maxQueries = 99999;    // used to limit queries while testing
    bool allowRedirects = true;
    bool success;

    OQString tempFileName("tempWebPage.htm");
    OQString URLaddressTemplate, URLaddress, webAddress;
    OQString *newURLaddress;
    OQString province;

    std::vector<OQString*> pURLaddresses;

    qSourceFile sourceFile;

    QSqlQuery query;

    /********************/
    /*      STEP 1      */
    /********************/

    // Retrieve the initial URL to start the data mining process
    query.prepare("SELECT initialURL FROM listers WHERE listerID = :listerID");
    query.bindValue(":listerID", QVariant(listerID));
    query.exec();

    QString temp;
    QVariant tempvar;

    if (query.size() != 1)
    {
        PQString msg;
        msg << "Could not retrive initial URL for:  " << QString(listerID);
        globals->logMsg(ErrorSQL, msg);
        return;
    }
    else
    {
        query.first();
        tempvar = query.value(0);
        temp = query.value(0).toString();
        URLaddressTemplate = query.value(0).toString();
        query.clear();
    }

    // Create initial URL
    // Legacy parameters are "province" and page#

    // Retrieve valid provinces from database
    //success = query.prepare("SELECT group1 FROM listerdata WHERE listerID = :listerID AND group1 = 'British-Columbia'");
    success = query.prepare("SELECT group1 FROM listerdata WHERE listerID = :listerID");
    query.bindValue(":listerID", QVariant(listerID));
    query.exec();

    // Loop through one province at a time to list all funeral homes in the province
    while (query.next() && (queryCount < maxQueries))
    {
        province = OQString(query.value(0).toString());
        page = 1;
        createURLaddress(URLaddress, URLaddressTemplate, URLparam(province), URLparam(page));

        www->download(URLaddress, tempFileName, allowRedirects);
        while(www->processingDownload()){};
        if(www->lastDownloadSuccessful())
        {
            sourceFile.setSourceFile(tempFileName);
            numFuneralHomes = getNumFH(sourceFile, listerID);
            if (numFuneralHomes > 0)
            {
                unsigned int entriesPerPage = 10;
                unsigned int numRead = 0;
                unsigned int numReadThisPage = 0;
                bool keepGoing = true;

                while ((numRead < numFuneralHomes) && keepGoing && (queryCount < maxQueries))
                {
                    keepGoing = sourceFile.moveTo(QString("fhname"));
                    if (keepGoing)
                    {
                        keepGoing = sourceFile.moveTo(QString("href"));
                        if (keepGoing)
                        {
                            webAddress = sourceFile.readNextBetween(DOUBLE_QUOTES);
                            webAddress = webAddress.unQuoteHTML();
                            newURLaddress = new OQString(webAddress);
                            pURLaddresses.push_back(newURLaddress);
                            numRead++;
                            numReadThisPage++;
                            queryCount++;
                            if ((numReadThisPage == entriesPerPage) && (numRead < numFuneralHomes))
                            {
                                sourceFile.close();
                                numReadThisPage = 0;
                                page++;
                                createURLaddress(URLaddress, URLaddressTemplate, URLparam(province), URLparam(page));
                                www->download(URLaddress, tempFileName, allowRedirects);
                                while(www->processingDownload()){};
                                if(www->lastDownloadSuccessful())
                                    sourceFile.setSourceFile(tempFileName);
                                else
                                    keepGoing = false;
                            }
                        }
                    }
                }  // end while
            }
        }

    }  // next province

    /*********************/
    /*    STEPS 2 & 3    */
    /*********************/

    // Visit legacy website for each funeral home to retrieve its URL address
    // Determine through SQL query if it's a new or old website

    OQString *url;
    fhRecordStructure fhRS;
    fhRS.setGlobalVars(globals);
    bool found, changes;
    allowRedirects = true;
    PQString errorMessage;

    for ( auto i = pURLaddresses.begin(); i != pURLaddresses.end(); i++ )
    {
        url = *i;
        www->download(*url, tempFileName, allowRedirects);
        while(www->processingDownload()){};
        if(www->lastDownloadSuccessful())
        {
            sourceFile.setSourceFile(tempFileName);
            fhRS.clear();
            success = getFuneralHomeData(sourceFile, listerID, fhRS);
            if (success)
            {
                errorMessage.clear();
                errorMessage << fhRS.listerID << "," << fhRS.listerKey << "," << fhRS.fhName << "," << fhRS.fhCity << "," << fhRS.fhProvince << "," << fhRS.fhPostalCode << ", " << fhRS.fhHTTP << ", " << fhRS.fhWWW << ", " << fhRS.fhURL;
                globals->logMsg(AuditListing, errorMessage);
            }
            /*if (success)
            {
                found = fhRS.lookupFHID(listerID);
                if (found)
                {
                    changes = fhRS.compareRecordToFH();
                    if (changes)
                    {
                        errorMessage << QString("Funeral home information changed: ") << fhRS.listerID;
                        errorMessage << QString(" - ") << fhRS.fhName;
                        globals->logMsg(ErrorSQL, errorMessage);
                    }
                }
                else
                {
                    fhRS.fhRunStatus = static_cast<unsigned int>(2);
                    fhRS.fhLastUpdate = globals->today;
                    fhRS.addRecordToFH();
                    errorMessage << QString("New funeral home found:") << fhRS.listerID;
                    errorMessage << QString(" - ") << fhRS.fhName;
                    globals->logMsg(ActionRequired, errorMessage);
                }
                errorMessage.clear();
                errorMessage << fhRS.listerID << "," << fhRS.listerKey << "," << fhRS.fhName << "," << fhRS.fhCity << "," << fhRS.fhProvince << "," << fhRS.fhPostalCode << ", " << fhRS.fhHTTP << ", " << fhRS.fhWWW << ", " << fhRS.fhURL;
                globals->logMsg(AuditListing, errorMessage);
            }*/
        }
    }   // end of Steps 2 & 3 for loop
}

bool MINER::getFuneralHomeData(qSourceFile &sf, PROVIDER &provider, fhRecordStructure &fhRS)
{
    OQString tempString, nextEntry;
    int tempNum;
    unstructuredContent uc;
    bool success;
    QString qNull, string, numString;

    fhRS.providerID = static_cast<PROVIDER>(provider);
    fhRS.fhRunStatus = NULL;
    fhRS.fhSearchCode = NULL;

    switch (provider)
    {
    case Legacy:
        if (sf.moveTo("<title>"))
        {
            uc = sf.getUntil(" -");
            fhRS.fhName = uc.getString();
        }

        if (sf.consecutiveMovesTo(100, "og:url", "content="))
        {
            uc = sf.readNextBetween(QUOTES);
            string = uc.getString();
            tempNum = string.indexOf("fh-");
            fhRS.listerID = 1;
            fhRS.listerKey = string.right(string.length() - tempNum - 3).toUInt();
        }

        if (sf.consecutiveMovesTo(100, "data:locality", "content="))
        {
            uc = sf.readNextBetween(QUOTES);
            fhRS.fhCity = uc.getString();
        }

        if (sf.consecutiveMovesTo(100, "data:region", "content="))
        {
            uc = sf.readNextBetween(QUOTES);
            fhRS.fhProvince = uc.getString();
        }

        if (sf.consecutiveMovesTo(100, "data:postal_code", "content="))
        {
            uc = sf.readNextBetween(QUOTES);
            if (uc.getLength() == 6)
                uc = uc.left(3) + OQString(" ") + uc.right(3);
            fhRS.fhPostalCode = uc.getString();
        }

        if (sf.consecutiveMovesTo(100, "data:website", "content="))
        {
            tempString = sf.peekAtNext(5);
            if (tempString != OQString(":null"))
            {
                uc = sf.readNextBetween(QUOTES);
                uc.cleanUpEnds();
                tempString = uc.getString();

                if (tempString.left(6) == OQString("https:"))
                {
                    fhRS.fhHTTP = QString("https:");
                    tempString.dropLeft(6);
                }

                if (tempString.left(5) == OQString("http:"))
                {
                    fhRS.fhHTTP = QString("http:");
                    tempString.dropLeft(5);
                }

                if (tempString.left(2) == OQString("//"))
                    tempString.dropLeft(2);

                if (tempString.left(4) == OQString("www."))
                {
                    fhRS.fhWWW = QString("www.");
                    tempString.dropLeft(4);
                }

                fhRS.fhURL = tempString.getString();
            }
        }


/*        // Read Name
        success = sf.consecutiveMovesTo(2, QString("DisplayName"), QString(":"));
        if (success)
        {
            nextEntry = sf.peekAtNext(4);
            if (nextEntry == "null")
                fhRS.fhName = qNull;
            else
            {
                tempString = sf.readNextBetween(DOUBLE_QUOTES);
                tempString = tempString.unQuoteHTML();
                if (tempString.getLength() > 0)
                    fhRS.fhName = tempString.getString();
                else
                    fhRS.fhName = qNull;
            }
        }
        else
            sf.beg();

        // Read ID
        success = sf.consecutiveMovesTo(2, QString("FuneralHomeId"), QString(":"));
        if (success)
        {
            tempString = sf.getUntil(",");
            tempNum = static_cast<unsigned int>(tempString.asNumber());
            if (tempNum)
                fhRS.listerID = tempNum;
            else
                fhRS.listerID = NULL;
        }
        else
            sf.beg();

        // Read City
        success = sf.consecutiveMovesTo(2, QString("City"), QString(":"));
        if (success)
        {
            nextEntry = sf.peekAtNext(4);
            if (nextEntry == "null")
                fhRS.fhCity = qNull;
            else
            {
                tempString = sf.readNextBetween(DOUBLE_QUOTES);
                tempString = tempString.unQuoteHTML();
                if (tempString.getLength() > 0)
                    fhRS.fhCity = tempString.getString();
                else
                    fhRS.fhCity = qNull;
            }
        }
        else
            sf.beg();

        // Read Province
        success = sf.consecutiveMovesTo(2, QString("StateName"), QString(":"));
        if (success)
        {
            nextEntry = sf.peekAtNext(4);
            if (nextEntry == "null")
                fhRS.fhProvince = qNull;
            else
            {
                tempString = sf.readNextBetween(DOUBLE_QUOTES);
                tempString = tempString.unQuoteHTML();
                if (tempString.getLength() > 0)
                    fhRS.fhProvince = tempString.getString();
                else
                    fhRS.fhProvince = qNull;
            }
        }
        else
            sf.beg();

        // Read Postal Code
        success = sf.consecutiveMovesTo(2, QString("StateName"), QString(":"));
        if (success)
        {
            nextEntry = sf.peekAtNext(4);
            if (nextEntry == "null")
                fhRS.fhProvince = qNull;
            else
            {
                tempString = sf.readNextBetween(DOUBLE_QUOTES);
                tempString = tempString.unQuoteHTML();
                if (tempString.getLength() > 0)
                    fhRS.fhProvince = tempString.getString();
                else
                    fhRS.fhProvince = qNull;
            }
        }
        else
            sf.beg();

        // Read URL
        success = sf.consecutiveMovesTo(2, QString("WebsiteUrl"), QString(":"));
        if (success)
        {
            nextEntry = sf.peekAtNext(4);
            if (nextEntry == "null")
                fhRS.fhURL = qNull;
            else
            {
                tempString = sf.readNextBetween(DOUBLE_QUOTES);
                tempString = tempString.unQuoteHTML();
                tempString.removeLeading(QStringLiteral("http:"));
                tempString.removeLeading(QStringLiteral("//"));
                tempString.removeLeading(QStringLiteral("www."));
                tempString.removeEnding(QStringLiteral("/"));
                if (tempString.getLength() > 0)
                    fhRS.fhURL = tempString.getString();
                else
                    fhRS.fhURL = qNull;
            }
        }
        else
            sf.beg();*/

        break;  // end Legacy

    default:
        break;

    }

    success = (fhRS.listerID > 0) && (fhRS.providerID > 0);

    return success;

}

unsigned int MINER::getNumFH(qSourceFile &sf, PROVIDER &provider)
{
    OQString numberOfFuneralHomes;
    unsigned int numFuneralHomes = 0;
    bool success;

    switch (provider)
    {
    case Legacy:
        success = sf.consecutiveMovesTo(500, "Within 50 Miles", "SubHeader", ">");
        if (success)
        {
            numberOfFuneralHomes = sf.getWord();
            numFuneralHomes = static_cast<unsigned int>(numberOfFuneralHomes.extractFirstNumber());
        }
        break;

    default:
        break;

    }

    if (numFuneralHomes > 0)
        return numFuneralHomes;
    else
        return 0;

}

unsigned int MINER::getFuneralHomeID(qSourceFile &sf, PROVIDER &provider)
{
    OQString inputString;
    unsigned int funeralHomeID = 0;
    bool success;

    switch (provider)
    {
    case Legacy:
        success = sf.moveTo("FuneralHomeId");
        if (success)
        {
            inputString = sf.getNext(15);
            funeralHomeID = static_cast<unsigned int>(inputString.extractFirstNumber());
        }
        break;

    default:
        break;

    }

    if (funeralHomeID > 0)
        return funeralHomeID;
    else
        return 0;

}

QString MINER::getFuneralHomeURL(qSourceFile &sf, PROVIDER &provider)
{
    OQString url;
    bool success;

    switch (provider)
    {
    case Legacy:
        success = sf.consecutiveMovesTo(50,QString("WebsiteUrl"),QString(":"));
        if (success)
            url = sf.readNextBetween(DOUBLE_QUOTES);
        break;

    default:
        break;

    }

    return url.getString();
}


