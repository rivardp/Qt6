#include "Include/dataMiningBPL.h"

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
    bool LegacyRun = false;
    bool noHTMLfile;

    // First step is to create a list of .obitList or .obitLegacy files found in the Read Batch directory
    QList<QString> fileList;
    QDir directory;

    // Select Type of run if
    if (!globals->batchRunFlag)
    {
        int choice;
        std::cout << "Is this a Legacy Run?  (1 = Yes) " ;
        std::cin >> choice;
        if (choice == 1)
            LegacyRun = true;
    }

    if (LegacyRun)
    {
        QStringList fileNames = QFileDialog::getOpenFileNames(nullptr,
                                                tr("Select Files"), "C:\\Obits\\Providers\\Legacy\\JSON Dumps 20230802", tr("Obit Files (*.obitLegacy)"));
        directory.setCurrent(globals->readDirectory.getString());
        fileList = fileNames;
    }
    else
    {
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

    QString fullLine, HTMLfileName, target;
    QString providerString, keyString, idString;
    QString baseClientDirectory, recordName;
    PQString URL;

    QFile* inputFile;
    QString exportFile("CSVtoBeLoaded.csv");

    unsigned int processCount = 0;

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
    QList<dataRecord> drDoubles;
    while (iterFile.hasNext())
    {
        currentFile = iterFile.next();
        obitListFile = new QFile;
        obitListFile->setFileName(currentFile);
        obitListFile->open(QFile::ReadOnly | QFile::Text);

        if (LegacyRun)
        {
            OQStream LegacyStream;
            QString residual;
            unstructuredContent tempUC;
            tempUC.setGlobalVars(*globals);

            while (fillLegacyStream(LegacyStream, residual, obitListFile))
            {
                processCount++;
                obit.clear();
                dr.clear();
                dr.setDate(globals->today);
                dr.setProvider(Legacy3000);

                if(loadDr(dr, LegacyStream))
                {
                    LegacyStream.clear();

                    obit.read();
                    processDefdMessages();
                    if (dr.wi.doubleMemorialFlag == 10)
                        dr.drDoubles(drDoubles, dr.getObitSnippet());
                    dr.xport(exportFile);

                    unsigned int freq = 10;
                    if ((processCount % freq) == 0)
                        qDebug() << processCount << " records processed";
                }
            }
        }
        else
        {
            fullLine = obitListFile->readLine();
            while (!fullLine.isNull())
            {
                processCount++;
                obit.clear();
                dr.clear();
                dr.setDate(globals->today);
                fullLine.replace("\n","");

                if(loadDr(dr, fullLine, providerString, keyString, idString))
                {
                    noHTMLfile = (dr.getProvider() >= 3000);

                    // Create full file names
                    baseClientDirectory = globals->baseDirectory.getString() + QString("\\") + providerString + QString(" ") + keyString + QString("\\");
                    recordName = providerString + QString(" ") + keyString + QString(" ") + idString;
                    if (noHTMLfile)
                        HTMLfileName = QString("C:\\Obits\\Providers\\Dummy HTML File.txt");
                    else
                        HTMLfileName = baseClientDirectory + QString("Obituaries\\") + recordName + QString(".htm");


                    // Read in the HTML file
                    inputFile = new QFile;
                    inputFile->setFileName(HTMLfileName);
                    if(obit.setSource(*inputFile))
                    {
                        obit.read();
                        processDefdMessages();
                        if (dr.wi.doubleMemorialFlag == 10)
                            dr.drDoubles(drDoubles, dr.getObitSnippet());
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
            }
        }

        obitListFile->close();
        obitListFile->rename(currentFile.append(".processed"));

        while (drDoubles.size() > 0){
            drDoubles.takeFirst().xport(exportFile);
        }

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

bool MINER::fillLegacyStream(OQStream &LegacyStream, QString &residual, QFile* obitListFile)
{
    int index;
    long int bufferSize = 524288;
    QString target("\",\"communities\"");
    bool streamLoaded = false;

    index = residual.indexOf(target);
    if ((index == -1) && !obitListFile->atEnd())
        residual += QString::fromUtf8(obitListFile->read(bufferSize));
    residual.replace(QChar(8211), QChar(45));
    index = residual.indexOf(target);
    if (index > 0)
    {
        LegacyStream = OQStream(residual.left(index + target.length()));
        residual = residual.right(residual.length() - (index + target.length()));
        streamLoaded = true;
    }


    return streamLoaded;
}

void MINER::logNewFH(QString &tempString, GLOBALVARS *gv)
{
    OQString url(tempString);
    url.removeLeading("https://");
    url.removeLeading("http://");
    url.removeLeading("www.");
    QString URL = url.getString();
    int index = URL.indexOf("/");
    if (index >= 0)
        URL = URL.left(index);
    bool alreadyKnown = true;
    databaseSearches dbSearch;
    QString pc;

    if (URL.length() > 0)
        alreadyKnown = dbSearch.FHrecordExists(URL, pc, gv);

    if (alreadyKnown)
    {
        // Funeral home location more precise than any newspaper
        gv->globalDr->setPostalCode(pc);
    }
    else
    {
        PQString msg("Unknown funeral home: ");
        msg += URL;
        globals->logMsg(ActionRequired, msg);
    }
}

bool MINER::loadDr(dataRecord &dr, OQStream &LegacyStream)
{
    QString tempString;
    QList<QString> middles, newMiddles, lastNames, firstNames;
    bool includesInitial = false;
    unstructuredContent tempUC;
    databaseSearches dbSearch;
    NAMESTATS nameStats;
    GENDER gender = genderUnknown;
    bool firstNameAllCaps = false;
    bool lastNameAllCaps = true;
    bool hyphenatedName = false;

    // ID
    LegacyStream.loadStringValue("personId", tempString, false);
    dr.setID(tempString);

    // yyyy - yyyy
    LegacyStream.loadStringValue("fromToYears", tempString, true);
    if (tempString.length() > 0)
    {
        tempUC = tempString;
        tempUC.processStructuredYears();
    }

    // Publish Date
    LegacyStream.loadStringValue("datePostedFrom", tempString, true);
    {
        tempUC = tempString.left(10);
        tempUC.processDateField(dfDOP, doYMD);
    }

    // First Name
    LegacyStream.loadStringValue("firstName", tempString, true);
    if (tempString.length() > 0)
    {
        tempUC = OQString(tempString);
        tempUC.removeInternalPeriods();
        tempUC.removeEnding(".");
        firstNameAllCaps = tempUC.isAllCaps();
        if (tempUC.countWords() == 1)
            dr.setFirstNames(tempUC.proper().getString());
        else
        {
            tempUC.removeAllSuffixPrefix();
            if (tempUC.countWords() == 1)
                dr.setFirstNames(tempUC.proper().getString());
            else
            {
                firstNames.clear();
                firstNames = tempUC.getString().split(" ");
                for (int i = 0; i < firstNames.size(); i++)
                {
                    if (firstNames.at(i).left(1) == "(")
                    {
                        tempUC = OQString(firstNames.at(i));
                        tempUC.removeBookEnds(PARENTHESES);
                        firstNames[i] = tempUC.getString();
                        if (tempUC.lower() == OQString("and"))
                            dr.wi.doubleMemorialFlag = 10;
                    }
                }
                while (firstNames.size() > 0){
                    dr.setFirstNames(OQString(firstNames.takeFirst()).proper());
                }
            }
        }
    }

    // Last Name
    LegacyStream.loadStringValue("lastName", tempString, true);
    if (tempString.length() > 0)
    {
        lastNames.clear();
        tempUC = OQString(tempString);
        lastNameAllCaps = tempUC.isAllCaps() && !tempUC.isInitial();
        tempUC = tempUC.proper();
        tempUC.prepareStructuredNames();
        if (tempUC.isHyphenated())
        {
            hyphenatedName = true;
            if (dr.getGender() == Male)
                dr.setMaleHypentated(true);
            lastNames = tempUC.getString().split("-");
        }
        else
            lastNames = tempUC.getString().split(" ");
        while (lastNames.size() > 0){
            dr.setFamilyName(OQString(lastNames.takeFirst()).proper());
        }
    }

    // Fix flipping of first and last
    if (firstNameAllCaps && !lastNameAllCaps && (globals->globalDr->getLastNameAlt1().getLength() == 0) && (globals->globalDr->getFirstNameAKA1().getLength() == 0))
    {
        QString tempFirst, tempLast;
        QString name = globals->globalDr->getLastName().getString();
        dbSearch.nameStatLookup(name, globals, nameStats, gender);
        if (nameStats.isLikelyGivenName)
        {
            tempFirst = name;
            name = globals->globalDr->getFirstName().getString();
            dbSearch.nameStatLookup(name, globals, nameStats, gender);
            if (nameStats.isLikelySurname)
            {
                tempLast = name;
                globals->globalDr->clearNames();
                globals->globalDr->setFirstNames(tempFirst);
                globals->globalDr->setFamilyName(tempLast);
            }
        }
    }

    // Middlenames
    LegacyStream.loadStringValue("middleName", tempString, true);
    if (tempString.length() > 0)
    {
        if (tempString.contains(" and ", Qt::CaseInsensitive))
            globals->globalDr->wi.doubleMemorialFlag = 10;
        else
        {
            if (tempString.left(4).toLower() == QString("and "))
                globals->globalDr->wi.doubleMemorialFlag = 10;
        }

        middles.clear();
        newMiddles.clear();
        middles = tempString.split(" ");
        while (middles.size() > 0)
        {
            tempUC = OQString(middles.takeFirst());
            tempUC.removeEnding(".");
            newMiddles.append(tempUC.proper().getString());
            if (tempUC.getLength() == 1)
                includesInitial = true;
        }
        dr.setMiddleNames(newMiddles.join(" "));
    }

    // Nickname
    LegacyStream.loadStringValue("nickName", tempString, true);
    if (tempString.length() > 0)
    {
        tempUC = OQString(tempString);
        tempUC.removeBookEnds(QUOTES);
        tempUC.removeBookEnds(PARENTHESES);
        dr.setFirstNames(tempUC.proper());
    }
    else
    {
        if ((dr.getFirstName().getLength() == 1) && (dr.getMiddleNames().countWords() == 2) && includesInitial)
        {
            middles.clear();
            newMiddles.clear();
            dr.getMiddleNameList(middles);
            tempString = middles.takeFirst();
            if (tempString == dr.getFirstName().getString())
            {
                tempString = middles.takeFirst();
                dr.setMiddleNameUsedAsFirstName(tempString);
                dr.setFirstNames(tempString);
                dr.clearMiddleNames();
                dr.setMiddleNames(tempString);
            }
            else
            {
                dr.setFirstNames(tempString);
                dr.clearMiddleNames();
                dr.setMiddleNames(tempString);
            }
        }
    }

    double unisex = dbSearch.genderLookup(&dr, globals);
    if (unisex >= 0.9)
    {
        gender = Male;
        if (hyphenatedName)
            dr.setMaleHypentated(true);
    }
    else
    {
        if (unisex <= 0.1)
            gender = Female;
    }

    // Maiden Name
    LegacyStream.loadStringValue("maidenName", tempString, true);  // often represents nickname
    if (tempString.length() > 0)
    {
        tempUC = tempString;
        tempUC.removeLeadingNeeEtAl();
        tempString= tempUC.getString();
        tempUC.compressCompoundNames(tempString);
        tempString.replace("/", " ");
        tempString.replace(",", " ");
        tempString.replace("  ", " ");

        lastNames = tempString.split(" ");
        if (lastNames.size() == 1)
        {
            QString name = tempUC.proper().getString();
            dbSearch.nameStatLookup(name, globals, nameStats, gender);
            if (!nameStats.isPureNickName)
            {
                bool isNickName = nameStats.isLikelyGivenName || dr.isANameVersion(name, true);
                if (isNickName)
                    dr.setFirstNames(name);
                else
                    dr.setMaidenNames(name);
            }
        }
        else
        {
            bool maidenSet = false;
            while (lastNames.size() > 0)
            {
                tempString = OQString(lastNames.takeFirst()).proper().getString();
                if (!maidenSet)
                {
                    dr.setMaidenNames(tempString);
                    maidenSet = true;
                }
                else
                    dr.setFamilyName(tempString);
            }
        }
    }

    // Age
    LegacyStream.loadStringValue("age", tempString, true);
    if (tempString.length() > 0)
        dr.setAgeAtDeath(tempString.toInt());

    // ProviderKey
    LegacyStream.loadStringValue("sourceId", tempString, true);
    if (tempString.length() > 0)
        dr.setProviderKey(tempString.toUInt());

    // URL
    LegacyStream.loadStringValue("websiteUrl", tempString, true);
    if (tempString.length() > 0)
        logNewFH(tempString, globals);

    // Snippet
    LegacyStream.loadStringValue("obitSnippet", tempString, true);
    if (tempString.length() > 0)
    {
        QRegularExpression targetS;
        targetS.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);
        targetS.setPattern("  ([A-Z])");

        tempString.replace("   ", "<br>");
        tempString.replace(targetS, "<br>\\1");
        tempString.replace("...", "");

        targetS.setPattern("Sunrise: (January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})(<br>Sunset: )(January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})");
        tempString.replace(targetS, "(\\1 \\2 - \\4 \\5)");

        targetS.setPattern("<br>(January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})(.*)<br>(January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})(.*)<br>");
        tempString.replace(targetS, "(\\1 \\2 - \\4 \\5) ");

        targetS.setPattern("<br>(.*)(January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})<br>(.*)(January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})<br>");
        tempString.replace(targetS, "(\\2 \\3 - \\5 \\6) ");

        QRegularExpression targetI;
        targetI.setPatternOptions(QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
        targetI.setPattern("\\bloving\\b");
        tempString.replace(targetI, "");
        tempString.replace("  ", " ");
        targetI.setPattern("\\bin memory of\\b");
        tempString.replace(targetI, "");
        targetI.setPattern("\\bin memory\\b");
        tempString.replace(targetI, "");
        tempString.replace("  ", " ");

        dr.setObitSnippet(tempString);
    }

    // URL
    LegacyStream.loadStringValue("href", tempString, true);
    dr.setURL(tempString);

    // Fixes
    if ((dr.getFirstName() == PQString("In")) && ((dr.getLastName() == PQString("Of")) || dr.getLastName() == PQString("Memory")))
    {
        dr.clearNames();
        databaseSearches dbSearch;
        NAMESTATS nameStats;
        QString fullName;
        OQStream tempStream(dr.getObitSnippet());
        unstructuredContent tempUC;
        bool keepGoing = true;
        QString word;

        if (tempStream.moveTo("<", 30))
        {
            tempUC = tempStream.readNextBetween(BRACKETS);
            int numWords = tempUC.countWords();
            if ((numWords >= 2) && (numWords <= 6))
                keepGoing = false;
        }

        if (keepGoing)
        {
            tempStream.beg();
            while (keepGoing && !tempStream.isEOS())
            {
                word = tempStream.getWord().getString();
                if (!((word.toLower() == QString("in")) || (word.toLower() == QString("loving")) || (word.toLower() == QString("memory")) || (word.toLower() == QString("of"))))
                {
                    dbSearch.nameStatLookup(word, globals, nameStats, gender);
                    if (nameStats.isGivenName || nameStats.isSurname)
                    {
                        fullName += word;
                        fullName += " ";
                    }
                    else
                        keepGoing = false;
                }
            }
            tempUC = fullName;
        }

        tempUC.cleanUpEnds();
        tempUC.prepareStructuredNames();
        globals->globalObit->processStructuredNames();
    }

    // Check for a single name only in the structured fields
    int totalNames = 0;
    OQString savedName;
    OQString name = globals->globalDr->getLastName();
    if (name.getLength() > 0)
    {
        savedName = name;
        totalNames++;

        name = globals->globalDr->getLastNameAlt1();
        if (name.getLength() > 0)
        {
            totalNames++;
            name = globals->globalDr->getLastNameAlt1();

            if (name.getLength() > 0)
            {
                totalNames++;
                name = globals->globalDr->getLastNameAlt2();
            }
        }
    }

    name = globals->globalDr->getFirstName();
    if (name.getLength() > 0)
    {
        if (totalNames == 0)
            savedName = name;
        totalNames++;

        name = globals->globalDr->getFirstNameAKA1();
        if (name.getLength() > 0)
            totalNames++;
    }

    name = globals->globalDr->getMiddleNames();
    if (name.getLength() > 0)
    {
        QList<QString> middles = name.getString().split(" ");
        if (totalNames == 0)
            savedName = middles.at(0);
        totalNames += middles.size();
    }

    if (totalNames == 1)
    {
        // Cannot rely on predefined field descriptions
        dbSearch.nameStatLookup(savedName.getString(), globals, nameStats, gender);
        globals->globalDr->clearNames();
        if (nameStats.isLikelySurname)
            globals->globalDr->setFamilyName(savedName);
        else
        {
            if (nameStats.isLikelyGivenName)
                globals->globalDr->setFirstName(savedName);
            else
                globals->globalDr->setFamilyName(savedName);
        }
    }

    return true;
}

bool MINER::loadDr(dataRecord &dr, QString &fullLine, QString &providerString, QString &keyString, QString &idString)
{
    QString divider("||");
    int breakpoint, start, length;
    PROVIDER provider;
    QString tempString, stringDate;
    QDate qdate, nullDate;
    PQString URL;
    int year;
    bool found = false;

    databaseSearches dbSearch;
    POSTALCODE_INFO pcInfo;
    QString pcLocation;
    nullDate.setDate(1900,1,1);

    start = 0;
    breakpoint = fullLine.indexOf(divider, start);
    if (breakpoint > 0)
    {
        found = true;

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
        breakpoint = fullLine.indexOf(divider, start);
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

        // 13. Full Name
        start = breakpoint + 2;
        breakpoint = fullLine.indexOf(divider, start);
        length = breakpoint - start;
        tempString = fullLine.mid(start, length);
        if (tempString.length() > 0)
            dr.setRawFullName(tempString);

        // 14. Obit Snippet
        start = breakpoint + 2;
        breakpoint = fullLine.lastIndexOf(divider);
        length = breakpoint - start;
        tempString = fullLine.mid(start, length);
        if (tempString.length() > 0)
            dr.setObitSnippet(tempString);
    }

    return found;
}
