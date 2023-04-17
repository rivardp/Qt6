#include "Include/dataMiningBP.h"

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

    // From QtCreator, no arguments provided and value is set by choice
    // For production version, a single argument of "0" will be provided

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

   processObits();

   quit();
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

void MINER::processObits()
{
    // First step is to create a list of .obitList files found in the Read Batch directory
    QList<QString> fileList;
    QDir directory;
    directory.setCurrent(globals->readDirectory.getString());
    directory.setFilter(QDir::Files);
    directory.setNameFilters(QStringList()<<"*.obitList");

    QFileInfoList list = directory.entryInfoList();
    QFileInfo fileInfo;
    for (int i = 0; i < list.size(); ++i)
    {
        fileInfo = list.at(i);
        fileList.append(fileInfo.fileName());
    }

    readObit obit;
    globals->globalObit = &obit;
    globals->uc = obit.getUCaddress();
    globals->justInitialNamesUC = obit.getJustInitialNamesAddress();
    globals->structuredNamesProcessed = obit.getStructuredNamesAddress();
    dataRecord dr;
    globals->globalDr = &dr;
    obit.setGlobalVars(*globals);
    dr.setGlobals(*globals);

    QString providerString, keyString, idString;
    QString fullLine, HTMLfileName, tempString, target, stringDate;
    QString baseClientDirectory, recordName;
    QString divider("||");
    PQString URL;
    QDate qdate, nullDate;
    int breakpoint, start, length;
    unsigned int year;
    PROVIDER provider;

    POSTALCODE_INFO pcInfo;
    databaseSearches dbSearch;
    QString pcLocation;

    QFile* inputFile;
    QString exportFile("CSVtoBeLoaded.csv");

    unsigned int processCount = 0;
    nullDate.setDate(1900,1,1);

    // Loop through and read one .obitList file at a time, processing it by:
    //   1. Reading the file name and the original HTML
    //   2. Process the file
    //   3. Export the file to a CSV file for checking
    //   4. Eventually, upload to the database

    // Loop through each .obitList file and read records one at a time
    // Each record has fullPathName||PDF structure, where fullPathName includes C:\Obits\Providers\PROVIDER PROVIDERKEY\PDFs\filename.pdf
    QListIterator<QString> iterFile(fileList);
    QFile *obitListFile;
    QString currentFile;
    while (iterFile.hasNext())
    {
        currentFile = iterFile.next();
        obitListFile = new QFile;
        obitListFile->setFileName(currentFile);
        obitListFile->open(QFile::ReadOnly | QFile::Text);
        fullLine = obitListFile->readLine();
        while (!fullLine.isNull())
        {
            processCount++;
            obit.clear();
            dr.clear();
            dr.setDate(globals->today);
            fullLine.replace("\n","");
            start = 0;
            breakpoint = fullLine.indexOf(divider, start);
            if (breakpoint > 0)
            {
                // Pull key information from the structure of the filename

                // 1. Provider ID
                providerString = fullLine.mid(start, breakpoint - start);
                provider = static_cast<PROVIDER>(providerString.toUInt());
                dr.setProvider(provider);

                // 2. Provider Key
                start = breakpoint + 2;
                breakpoint = fullLine.indexOf(divider, start);
                keyString = fullLine.mid(start, breakpoint - start);
                dr.setProviderKey(keyString.toUInt());

                // 3. ID
                start = breakpoint + 2;
                breakpoint = fullLine.indexOf(divider, start);
                idString = fullLine.mid(start, breakpoint - start);
                dr.setID(idString);

                // 4. URL
                start = breakpoint + 2;
                breakpoint = fullLine.indexOf(divider, start);
                URL = PQString(fullLine.mid(start, breakpoint - start));
                dr.setURL(URL);

                // 5. DOB
                start = breakpoint + 2;
                breakpoint = fullLine.indexOf(divider, start);
                stringDate = fullLine.mid(start, breakpoint - start);
                qdate.setDate(stringDate.left(4).toInt(), stringDate.mid(4,2).toInt(), stringDate.right(2).toInt());
                if (qdate != nullDate)
                    dr.setDOB(qdate);

                // 6. YOB
                start = breakpoint + 2;
                breakpoint = fullLine.indexOf(divider, start);
                tempString = fullLine.mid(start, breakpoint - start);
                year = tempString.toUInt();
                if (year == 1900)
                    year = 0;
                dr.setYOB(year, true);

                // 7. DOD
                start = breakpoint + 2;
                breakpoint = fullLine.indexOf(divider, start);
                stringDate = fullLine.mid(start, breakpoint - start);
                qdate.setDate(stringDate.left(4).toInt(), stringDate.mid(4,2).toInt(), stringDate.right(2).toInt());
                if (qdate != nullDate)
                    dr.setDOD(qdate);

                // 8. YOD
                start = breakpoint + 2;
                breakpoint = fullLine.indexOf(divider, start);
                tempString = fullLine.mid(start, breakpoint - start);
                year = tempString.toUInt();
                if (year == 1900)
                    year = 0;
                dr.setYOD(year, true);

                // Fix for earlier mistakes
                if (dr.getDOB().isValid() && dr.getDOD().isValid() && (dr.getDOB() != QDate(1900,1,1)) && (dr.getDOD() != QDate(1900,1,1)) && (dr.getDOB() > dr.getDOD()))
                {
                    dr.setDOD(dr.getDOB());
                    dr.setDOB(qdate);
                }

                // 9. Maiden names (may be NULL)
                start = breakpoint + 2;
                breakpoint = fullLine.indexOf(divider, start);
                length = breakpoint - start;
                if (length > 0)
                {
                    tempString = fullLine.mid(start, length);
                    OQString oTempString(tempString);
                    oTempString.fixBasicErrors();
                    dr.setMaidenNames(oTempString.getString());
                }

                // 10. Publish Date
                start = breakpoint + 2;
                breakpoint = fullLine.indexOf(divider, start);
                length = breakpoint - start;
                stringDate = fullLine.mid(start, length);
                qdate.setDate(stringDate.left(4).toInt(), stringDate.mid(4,2).toInt(), stringDate.right(2).toInt());
                if (qdate.isValid() && (qdate > QDate(1900,1,1)))
                    dr.setPublishDate(qdate);

                // 11. Age At Death
                start = breakpoint + 2;
                breakpoint = fullLine.indexOf(divider, start);
                length = breakpoint - start;
                dr.setAgeAtDeath(fullLine.mid(start, length).toUInt());

                // 12. Postal Code
                start = breakpoint + 2;
                breakpoint = fullLine.lastIndexOf(divider);
                length = breakpoint - start;
                tempString = fullLine.mid(start, length);
                if (tempString.length() > 0)
                {
                    pcLocation.clear();
                    pcLocation = dbSearch.pcLookup(globals, globals->globalDr->getProvider(), globals->globalDr->getProviderKey(), tempString);
                    pcInfo.clear();
                    if (pcLocation.length() > 0)
                    {
                        dbSearch.fillInPostalCodeInfo(globals, pcInfo, pcLocation);
                        if (pcInfo.isValid())
                            dr.setPostalCode(pcInfo);
                    }

                    if (!pcInfo.isValid())
                    {
                        pcInfo = dbSearch.pcLookupPlaces(globals, globals->globalDr->getProvider(), globals->globalDr->getProviderKey(), tempString);
                        if (pcInfo.isValid())
                            dr.setPostalCode(pcInfo);
                    }
                }

                // Create full file names
                baseClientDirectory = globals->baseDirectory.getString() + QString("\\") + providerString + QString(" ") + keyString + QString("\\");
                recordName = providerString + QString(" ") + keyString + QString(" ") + idString;
                HTMLfileName = baseClientDirectory + QString("Obituaries\\") + recordName + QString(".htm");

                // Read in the HTML file
                inputFile = new QFile;
                inputFile->setFileName(HTMLfileName);
                if(obit.setSource(*inputFile))
                {
                    obit.read();
                    processDefdMessages();
                    dr.xport(exportFile);
                }
                inputFile->close();

                unsigned int freq = 10;
                if ((processCount % freq) == 0)
                        qDebug() << processCount << " records processed";
            }
            else
            {
                PQString errMsg;
                errMsg << "Invalid record encountered in file: " << HTMLfileName;
                globals->logMsg(ErrorRunTime, errMsg);
            }
            fullLine = obitListFile->readLine();
        }   // End of while loop within each file

        obitListFile->close();
        obitListFile->rename(currentFile.append(".processed"));

    }   // End of while loop going through all the files

    if (globals->batchRunFlag)
    {
        QFile *file = new QFile(globals->batchRunFileName);
        QTextStream *outputStream = nullptr;

        if(file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            outputStream = new QTextStream(file);
            *outputStream << QString("BatchProcess completed running and exited normally at ") << QDateTime::currentDateTime().toString("h:mm") << Qt::endl;
        }

        delete outputStream;
        file->close();
    }
}

void MINER::processDefdMessages()
{
    QString word, target;
    OQString tempWord;
    DEFDERRORMESSAGES defdMsg;
    int start, end;
    bool ignore;

    while (globals->defdErrorMessages.size() > 0)
    {
        defdMsg = globals->defdErrorMessages.takeFirst();
        target = QString("Most common first word ");
        start = defdMsg.msg.indexOf(target);

        if (start != -1)
        {
            // Most common first name errMsg
            start += target.length();
            target = QString(" was rejected");
            end = defdMsg.msg.indexOf(target);
            tempWord = PQString(defdMsg.msg.mid(start, end - start));
            tempWord.removeBookEnds(QUOTES);
            word = tempWord.getString();

            ignore = false;
            if (globals->globalDr->isAFirstName(tempWord))
                ignore = true;
            if (!ignore && tempWord.isRecognizedFirstWord())
                ignore = true;

            if (!ignore)
            {
                globals->logMsg(defdMsg.mt, defdMsg.msg, defdMsg.msgInfo);
                if (globals->globalDr->getFirstName().left(1).lower() == tempWord.left(1).lower())
                    globals->globalDr->wi.outstandingErrMsg = 1;
            }
        }
        else
        {
            // Potential good information message
            if (((globals->globalDr->getYOB() > 0) || (globals->globalDr->getMinDOB() != QDate(1875,1,1))) && globals->globalDr->getDOD().isValid())
                ignore = true;
            else
                ignore = false;

            if (!ignore)
            {
                globals->logMsg(defdMsg.mt, defdMsg.msg, defdMsg.msgInfo);
                globals->globalDr->wi.outstandingErrMsg = 1;
            }
        }
    }
}
