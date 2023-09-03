#include "Include/dataMiningBR3.h"

MINER::MINER(QObject *parent) : QObject(parent)
{
    // get the instance of the main application
    app = QCoreApplication::instance();

    // any other additional setup here
}

void MINER::execute()
{
    qDebug() << "Data mining has started";

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

    /**************************************/
    /*       Set Override Options         */
    /**************************************/

    if (globals->runStatus == 2)
    {
        int choice;
        std::cout << "Do you want to exclude existing html records from processing?  (0 = No) (1 = Yes) (2 = Simulate Release Version)" ;
        std::cin >> choice;
        if (choice == 1)
            globals->runStatus = 3;
        else
        {
            if (choice == 2)
                globals->runStatus = 1;
        }
    }



    /************************************/
    /*     Start Downloading Files      */
    /************************************/

    mineReadObits();

   quit();
}

void MINER::setDownloadThread(QThread *thread)
{
    downloadThread = thread;
}

void MINER::setDownloadWorker(DownloadWorkerBR3 *downloadworker)
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

void MINER::mineReadObits()
{
    bool followRedirects = true;    // Not necessary for this program as final HTML filenames are read in
    bool POSTrequests = false;      // Only used for exception sites;
    globals->noMajorIssues = true;

    // Determine if run should download and overwrite over any existing files
    bool forceOverwrite;
    if (globals->runStatus == 1)
        forceOverwrite = true;
    else
        forceOverwrite = false;

    // First step is to create a list of .obitURLlist files found in the Read Batch directory
    QList<QString> fileList;
    QDir directory;
    directory.setCurrent(globals->readDirectory.getString());
    directory.setFilter(QDir::Files);
    directory.setNameFilters(QStringList()<<"*.obitURLlist");

    QFileInfoList list = directory.entryInfoList();
    QFileInfo fileInfo;
    for (int i = 0; i < list.size(); ++i)
    {
        fileInfo = list.at(i);
        fileList.append(fileInfo.fileName());
    }

    PQString errorMsg;
    unsigned int temp = static_cast<unsigned int>(fileList.size());
    errorMsg << "Number of files found was: " << temp;
    globals->logMsg(ErrorRunTime, errorMsg);
    bool successfulFileOpen;
    PQString clientSubDir;

    // Loop through and read one .obitURLlist file at a time, processing it by:
    //   1. Determining the proper name for each individual .html file
    //   2. Saving each HTML file to the proper directory
    //   3. Creating a PDF of the file and saving it to the proper directory - NOT CURRENTLY CODED
    //   4. Adding the full path name of the file to a .obitList file
    //
    //   Processing of data is intentionally completed separately to allow multiple or re-reading of files
    //   while minimizing the number of actual download from the obit providers

    int numProcessed, numDownloaded, numIncluded;
    int index;
    QString fileExt, tempString;
    QString singleURLfileName, URLlistFileName, nextRecord;
    QFile *inputFile, *masterURLlist;
    databaseSearches dbSearch;

    QSqlQuery query;

    StringRECORD record;

    // Create masterlist of downloads - to be shuffled randomly in order to avoid hitting same site consecutively
    QList<DOWNLOADREQUEST> masterDownloadRequestList;
    DOWNLOADREQUEST downloadRequest;
    numProcessed = 0;
    numDownloaded = 0;
    numIncluded = 0;

    // Loop through each .obitURLlist file to add records to the master file
    QListIterator<QString> iterFile(fileList);
    while (iterFile.hasNext())
    {
        singleURLfileName = iterFile.next();
        inputFile = new QFile;
        inputFile->setFileName(singleURLfileName);
        successfulFileOpen = inputFile->open(QIODevice::ReadOnly);
        errorMsg.clear();
        if (successfulFileOpen)
        {
            errorMsg << "Was able to open file: " << singleURLfileName;
            globals->logMsg(AuditListing, errorMsg);
        }
        else
        {
            errorMsg << "Was NOT able to open file: " << singleURLfileName;
            globals->logMsg(ErrorRunTime, errorMsg);
        }

        QTextStream inputStream(inputFile);

        // Read in the list of providerIDs, providerKeys, URLs, DOBs, DODs, maidenNames and pubDate
        QList<StringRECORD> records;
        OQStream tempStream;
        nextRecord = inputStream.readLine();
        while(!nextRecord.isNull())
        {
            tempStream = nextRecord;

            record.providerID = tempStream.getUntil(QString("|"));
            record.providerKey = tempStream.getUntil(QString("|"));
            record.url = tempStream.getUntil(QString("|"));
            record.POSTformRequest = tempStream.getUntil(QString("|"));
            record.dob = tempStream.getUntil(QString("|"));
            record.yob = tempStream.getUntil(QString("|"));
            record.dod = tempStream.getUntil(QString("|"));
            record.yod = tempStream.getUntil(QString("|"));
            record.maidenNames = tempStream.getUntil(QString("|"));
            record.ID = tempStream.getUntil(QString("|"));
            record.pubDate = tempStream.getUntil(QString("|"));
            record.ageAtDeath = tempStream.getUntil(QString("|"));
            record.pcKey = tempStream.getUntil(QString("|"));
            record.fullName = tempStream.getUntil(QString("|"));
            record.snippet = tempStream.getUntil(QString("|"));

            records.append(record);
            nextRecord = inputStream.readLine();
        }
        inputFile->close();
        errorMsg.clear();
        errorMsg << "Number of records read was: " << QString::number(records.size());
        globals->logMsg(AuditListing, errorMsg);

        QString searchSingleParam1, searchSingleParam2;
        QVariant tempvar;
        bool success;
        OQString lastProviderID = QString("0");
        OQString lastProviderKey = QString("0");

        QListIterator<StringRECORD> iterURL(records);
        OQString deceasedID, outputName, outputFileName, PDFfileName;

        OQString space(" ");
        OQString backslash("\\");
        OQString HTMLext(".htm");
        OQString PDFext(".pdf");

        // Loop through each record in file
        iterURL.toFront();
        while (iterURL.hasNext() && globals->noMajorIssues)
        {
            record = iterURL.next();
            numProcessed++;
            success = true;
            globals->noMajorIssues = true;
            downloadRequest.clear();

            // Query the obit ID parameter for the provider / key combination
            if ((record.providerID != lastProviderID) || (record.providerKey != lastProviderKey))
            {
                lastProviderID = record.providerID;
                lastProviderKey = record.providerKey;

                // Setup directory structure
                clientSubDir = record.providerID + PQString(" ") + record.providerKey;
                globals->setupClientDirectoryStructure(clientSubDir);  // This sets up both reports and PDF directories

                query.prepare("SELECT fhURLid, fhURLidDivider FROM death_audits.funeralhomedata "
                              "WHERE providerID = :provider AND providerKey = :key AND (fhRunStatus = 1 OR fhRunStatus = 2 OR fhRunStatus = 100)");
                query.bindValue(":provider", QVariant(record.providerID.getString()));
                query.bindValue(":key", QVariant(record.providerKey.getString()));
                query.exec();

                if (query.size() != 1)
                {
                    PQString msg;
                    msg << "Could not retrive the deceased ID searchParam for:  " << record.providerID.getString() << PQString(" - ") << record.providerKey.getString();
                    globals->logMsg(ErrorSQL, msg);
                    success = false;
                }
                else
                {
                    query.first();
                    tempvar = query.value(0);
                    searchSingleParam1 = query.value(0).toString();
                    searchSingleParam2 = query.value(1).toString();
                    query.clear();
                    success = true;
                }
            }

            if (!success)
            {
                searchSingleParam1 = QString("LAST");
                searchSingleParam2 = QString("/");
            }

            // Determine Deceased ID if not already in record
            if (record.ID.getLength() == 0)
            {
                deceasedID = record.url.readURLparameter(searchSingleParam1, searchSingleParam2);

                // Maximum deceasedID field width in database is currently 55
                if (deceasedID.getLength() > 55)
                {
                    PQString errMsg;
                    errMsg << "Field width for ID in \"deceased\" database must be widened to: " << deceasedID;
                    globals->logMsg(ErrorRunTime, errMsg);
                }
            }
            else
                deceasedID = record.ID;

            // Special coding for DND sites (i.e, where individual URLs do not exist
            tempString = record.url.getString();
            index = tempString.lastIndexOf(QString("."));
            if (index == -1)
                fileExt = QString("htm");
            else
                fileExt = tempString.right(tempString.length() - (index + 1));
            if (fileExt == QString("dnd"))
            {
                index = tempString.indexOf(QString("DND"));
                record.url = tempString.left(index);
                deceasedID = tempString.mid(index + 3, tempString.length() - (index + 7));
            }

            // Create output information
            outputName = record.providerID + space + record.providerKey + space + deceasedID;
            outputFileName = globals->baseDirectory + backslash + clientSubDir + backslash + PQString("Obituaries") + backslash + outputName + HTMLext;
            //PDFfileName = globals->baseDirectory + backslash + clientSubDir + backslash + PQString("PDFs")+ backslash + outputName + PDFext;
            PDFfileName = PDFext;

            downloadRequest.outputs.HTMLfileName = outputFileName.getString();
            downloadRequest.outputs.PDFfileName = PDFfileName.getString();
            downloadRequest.outputs.ID = deceasedID.getString();
            downloadRequest.outputs.providerID = static_cast<unsigned int>(record.providerID.asNumber());
            downloadRequest.outputs.providerKey = static_cast<unsigned int>(record.providerKey.asNumber());
            downloadRequest.outputs.dob = record.dob.getString();
            downloadRequest.outputs.yob = record.yob.getString();
            downloadRequest.outputs.dod = record.dod.getString();
            downloadRequest.outputs.yod = record.yod.getString();
            downloadRequest.outputs.ageAtDeath = record.ageAtDeath.getString().toUInt();
            if (downloadRequest.outputs.ageAtDeath == 999)
                downloadRequest.outputs.ageAtDeath = 0;
            downloadRequest.outputs.maidenNames = record.maidenNames.getString();
            downloadRequest.outputs.pubDate = record.pubDate.getString();
            downloadRequest.outputs.pcKey = record.pcKey.getString();
            downloadRequest.outputs.fullName = record.fullName.getString();
            downloadRequest.outputs.snippet = record.snippet.getString();
            downloadRequest.outputs.addToQueue = true;
            downloadRequest.outputs.globals = globals;
            if (forceOverwrite)
                downloadRequest.outputs.forceOverrides = true;
            else
                downloadRequest.outputs.forceOverrides = false;
            if ((record.POSTformRequest.getLength() > 0) || ((downloadRequest.outputs.providerID == 1011) && (downloadRequest.outputs.providerKey != 449)))
                POSTrequests = true;
            else
                POSTrequests = false;

            // Create record instruction
            downloadRequest.instructions.originalOrder = numProcessed;
            downloadRequest.instructions.randomOrder = 00;
            downloadRequest.instructions.url = record.url;
            downloadRequest.instructions.POSTformRequest = record.POSTformRequest.getString();
            downloadRequest.instructions.followRedirects = followRedirects;
            if (POSTrequests)
                downloadRequest.instructions.verb = QString("POST");
            downloadRequest.instructions.providerID = record.providerID.getString().toUInt();
            downloadRequest.instructions.providerKey = record.providerKey.getString().toUInt();

            // Drop off record for providerIDs with no obit where record already exists in database
            if (downloadRequest.instructions.providerID >= 3000)
            {
                QString url = downloadRequest.instructions.url.getString();
                QDate pubDate = QDate::fromString(downloadRequest.outputs.pubDate, "yyyyMMdd");
                if (!dbSearch.deceasedRecordExists(downloadRequest.outputs.providerID, downloadRequest.outputs.providerKey, downloadRequest.outputs.ID, url, pubDate))
                    masterDownloadRequestList.append(downloadRequest);
            }
            else
                masterDownloadRequestList.append(downloadRequest);

        } // end of loop through individual file

        // If run was successful, rename file such that it is not reprocessed again and again
        if (globals->noMajorIssues == true)
            inputFile->rename(singleURLfileName.append(".read"));
        delete inputFile;

    } // end of loop through all files

    // Randomize order of downloads
    QRandomGenerator *generator = QRandomGenerator::global();
    QVector<quint32> vector;
    vector.resize(numProcessed);
    generator->fillRange(vector.data(), vector.size());
    for (int i = 0; i < numProcessed; i++){
        masterDownloadRequestList[i].instructions.randomOrder = vector.at(i);
    }
    vector.clear();
    std::sort(masterDownloadRequestList.begin(), masterDownloadRequestList.end(), [](const DOWNLOADREQUEST &dr1, const DOWNLOADREQUEST &dr2){return dr1.instructions.randomOrder < dr2.instructions.randomOrder;});
    // Process one URL at a time, where most of processing is within downloadAndProcess function
    // 1. Check if the file has already been read in - if so, skip the actual download but otherwise process
    // 2. Download html file and save to client specific directory (where necessary)

    for (int i = 0; i < numProcessed; i++)
    {
        // Skip download for 3000 level providerIDs where no obit exists
        if(masterDownloadRequestList[i].instructions.providerID >= 3000)
        {
            masterDownloadRequestList[i].outputs.createObitEntry(masterDownloadRequestList[i].instructions.url.getString());
            numIncluded++;
        }
        else
        {
            if((masterDownloadRequestList[i].instructions.providerID == 1) && (masterDownloadRequestList[i].instructions.providerKey > 100000))
            {
                // Three calls required
                DOWNLOADREQUEST followUpRequest = masterDownloadRequestList[i];
                QString ext = "?apssov2tk=UDZMSW83NGlsYUdRZUhndEE4OURWUTM5VXMxQk1mNjUvSCszUyt2bzRmandsSHdIR3o4aGRjZkdrdjJLZ2N6UERqaDljb2pjYXl5d1MxUzlVSC9jSEpGQUY1YmFjeU85M1VyVk1URlQ0VFk9";
                followUpRequest.instructions.url += PQString(ext);

                www->downloadAndProcess(masterDownloadRequestList[i]);
                while(www->processingDownload()){};
                if(www->lastDownloadSuccessful())
                {
                    www->downloadAndProcess(followUpRequest);
                    while(www->processingDownload()){};
                    // Third call handled below
                }

                www->delay(2);
            }

            www->downloadAndProcess(masterDownloadRequestList[i]);
            while(www->processingDownload()){};
            if(www->lastDownloadSuccessful())
            {
                if (!masterDownloadRequestList[i].outputs.fileAlreadyExists)
                {
                    numDownloaded++;
                    numIncluded++;
                }
                else
                {
                    if (globals->runStatus < 3)
                        numIncluded++;
                }
            }
            qDebug() << (i + 1) << " of " << numProcessed << " processed";
            qDebug() << " ";
        }
    }

    // Return back to original order
    std::sort(masterDownloadRequestList.begin(), masterDownloadRequestList.end(), [](const DOWNLOADREQUEST &dr1, const DOWNLOADREQUEST &dr2){return dr1.instructions.originalOrder < dr2.instructions.originalOrder;});

    // Create obitList if record was not already in the deceased database (used as batchProcess input file)
    PQString stringdate(globals->today.toString("yyyyMMdd"));
    errorMsg.clear();
    errorMsg << "Today's date is :" << stringdate;
    globals->logMsg(ErrorRunTime, errorMsg);
    masterURLlist = new QFile;
    URLlistFileName = "List of URLs on ";
    URLlistFileName.append(stringdate.getString());
    URLlistFileName.append(QString(".obitList"));
    masterURLlist->setFileName(URLlistFileName);
    successfulFileOpen = masterURLlist->open(QIODevice::WriteOnly | QIODevice::Text);

    errorMsg.clear();
    if (successfulFileOpen)
    {
        errorMsg << "Was able to open masterListFile: " << URLlistFileName;

        QTextStream outputStream(masterURLlist);
        for (int i = 0; i < numProcessed; i++)
        {
            if (masterDownloadRequestList[i].outputs.obitListEntry.size() > 0)
                outputStream << masterDownloadRequestList[i].outputs.obitListEntry << Qt::endl;
        }
    }
    else
        errorMsg << "Was NOT able to open masterListFile: " << URLlistFileName;
    globals->logMsg(ErrorRunTime, errorMsg);

    masterURLlist->close();
    masterURLlist->deleteLater();

    if (globals->batchRunFlag)
    {
        QFile *file = new QFile(globals->batchRunFileName);
        QTextStream *outputStream = nullptr;

        if(file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            outputStream = new QTextStream(file);
            *outputStream << QString("BatchRead completed running and exited normally at ") << QDateTime::currentDateTime().toString("h:mm") << Qt::endl;
            *outputStream << QString("Total downloads: ") << numDownloaded << Qt::endl;
            *outputStream << QString("Total included: ") << numIncluded << Qt::endl;
        }

        delete outputStream;
        file->close();
    }
}

