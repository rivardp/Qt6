#include "readCSV.h"
#include "globals.h"

void readLine(QFile &filename, dataRecord &dr, GLOBALVARS gv)
{
    // CSV format   Last Name,Last Alt1,Last Alt2,Last Alt3,
    //              First Name,First Alt1,First Alt2,Middle Names,
    //              Prefix,Suffix,Gender#
    //              DOB,DOD,Min DOB,Max DOB,YOB,YOD,Age at Death,
    //              URL,Last Updated,Lang #,Language,
    //              Provider,Provider Key,ID
    //              Service Date, Publish Date, Spouse Name
    //              Flags

    PQString word;
    PQString name;
    int num, y, m, d;
    QString comma(",");
    char buffer[1024];

    qint64 lineLength = filename.readLine(buffer, sizeof(buffer));
    if (lineLength != -1)
    {
        // The line is available in buf
        PQStream line(buffer);

        // Clear anything left in the prior dr record
        dr.clear();

        // Last names
        for (unsigned int i = 0; i < 4; i++)
        {
            name = line.getUntil(comma);
            if (name.getLength() > 0)
                dr.setFamilyName(name);
        }

        // First names
        for (unsigned int i = 0; i < 3; i++)
        {
            name = line.getUntil(comma);
            if (name.getLength() > 0)
                dr.setFirstNames(name, true);
        }

        // Middle names
        name = line.getUntil(comma);
        if (name.getLength() > 0)
            dr.setMiddleNames(name);

        // Prefix and suffix
        word = line.getUntil(comma);
        if (word.getLength() > 0)
            dr.setPrefix(word);
        word = line.getUntil(comma);
        if (word.getLength() > 0)
            dr.setSuffix(word);

        // Gender
        num = static_cast<int>(line.getUntil(comma).asNumber());
        dr.setGender(GENDER(num));

        // DOB
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if (num > 0)
        {
            y = static_cast<int>(num / 10000);
            m = static_cast<int>((num - y * 10000) / 100);
            d = static_cast<int>((num - y * 10000 - m * 100));
            QDate tempDate(y,m,d);
            if (tempDate.isValid() && (tempDate <= gv.today))
                dr.setDOB(tempDate);
        }

        // DOD
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if (num > 0)
        {
            y = static_cast<int>(num / 10000);
            m = static_cast<int>((num - y * 10000) / 100);
            d = static_cast<int>((num - y * 10000 - m * 100));
            QDate tempDate(y,m,d);
            if (tempDate.isValid() && (tempDate <= gv.today))
                dr.setDOD(tempDate);
        }

        // min DOB
        num = static_cast<int>(line.getUntil(comma).asNumber());
        bool minDOBset = false;
        if ((num > 0) && !(dr.getMinDOB().isValid() && (dr.getMinDOB() > QDate(1875,1,1))))
        {
            y = static_cast<int>(num / 10000);
            m = static_cast<int>((num - y * 10000) / 100);
            d = static_cast<int>((num - y * 10000 - m * 100));
            QDate tempDate(y,m,d);
            if (tempDate.isValid() && (tempDate <= gv.today))
            {
                dr.setMinDOB(tempDate);
                minDOBset = true;
            }
        }

        // max DOB
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if ((num > 0) && minDOBset)
        {
            y = static_cast<int>(num / 10000);
            m = static_cast<int>((num - y * 10000) / 100);
            d = static_cast<int>((num - y * 10000 - m * 100));
            QDate tempDate(y,m,d);
            if (tempDate.isValid() && (tempDate <= gv.today))
                dr.setMaxDOB(tempDate);
        }

        // YOB
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if ((num > 0) && (dr.getYOB() == 0))
            dr.setYOB(static_cast<unsigned int>(num));

        // YOD
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if ((num > 0) && (dr.getYOD() == 0) && (dr.getYOD() <= static_cast<unsigned int>(gv.today.year())))
            dr.setYOD(static_cast<unsigned int>(num));

        // Age at death
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if (num > 0)
            dr.setAgeAtDeath(static_cast<unsigned int>(num));

        // URL
        word = line.getUntil(comma);
        if (word.getLength() > 0)
            dr.setURL(word);

        // Last updated
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if (num > 0)
        {
            y = static_cast<int>(num / 10000);
            m = static_cast<int>((num - y * 10000) / 100);
            d = static_cast<int>((num - y * 10000 - m * 100));
            dr.setLastUpdated(QDate(y,m,d));
        }

        // Language
        num = static_cast<int>(line.getUntil(comma).asNumber());
        dr.setLanguage(LANGUAGE(num));
        word = line.getUntil(comma);  // Skip past written language

        // Provider
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if (num > 0)
            dr.setProvider(PROVIDER(num));

        // Provider key
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if (num > 0)
            dr.setProviderKey(static_cast<unsigned int>(num));

        // ID
        word = line.getUntil(comma);  // Skip past written language
        if (word.getLength() > 0)
            dr.setID(word);

        // Service Date
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if (num > 0)
        {
            y = static_cast<int>(num / 10000);
            m = static_cast<int>((num - y * 10000) / 100);
            d = static_cast<int>((num - y * 10000 - m * 100));
            dr.setDOS(QDate(y,m,d));
        }

        // Publish Date
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if (num > 0)
        {
            y = static_cast<int>(num / 10000);
            m = static_cast<int>((num - y * 10000) / 100);
            d = static_cast<int>((num - y * 10000 - m * 100));
            dr.setPublishDate(QDate(y,m,d));
        }

        // Spouse Name
        word = line.getUntil(comma);  //
        if (word.getLength() > 0)
            dr.setSpouseName(word.getString());

        // Warning information flags and fields (only used for checking - not for load
        for (int i = 0; i < 10; i++)
            num = static_cast<int>(line.getUntil(comma).asNumber());

        // Potentially usuable info for checking - not loaded
        word = line.getUntil(comma);  //

        // Postal Code
        word = line.getUntil(comma);  //
        if (word.getLength() > 0)
        {
            QString temp = word.getString();
            dr.setPostalCode(temp);
        }

        // numUnmatched - only used for CSV checking for now
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if (num > 0)
            dr.setUnmatched(static_cast<unsigned int>(num));

        // priorUnmatched - only used for CSV checking for now
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if (num > 0)
            dr.setPriorUnmatched(static_cast<unsigned int>(num));

        // previouslyLoaded - control when to load record and first name
        num = static_cast<int>(line.getUntil(comma).asNumber());
        if (num > 0)
            dr.setPreviouslyLoaded(static_cast<unsigned int>(num));
    }
}

bool loadRecord(const dataRecord &dr, MATCHKEY &matchKeyResult, bool &newRec)
{
    GLOBALVARS gv = dr.globals;
    dataRecord drCopy = dr;
    dataRecord drDB;
    databaseSearches dbSearch;
    POSTALCODE_INFO pcInfo;

    QSqlQuery query, overrideQuery;
    QSqlError error;

    bool success, newInfo, sameFH, sufficientlyMatched, forceNewDeceasedID, forceInfoOverride, acceptableOverride, deleteRecord;

    bool updateExistingRecord = false;
    bool DBrecordRead = false;
    bool loaded = true;
    bool newRecord = false;
    unsigned int lastDeceasedNum = 0;
    double weight;
    MATCHRESULT matchResult;

    PQString errorMessage, dateSQL, firstLoaded, lastUpdated;
    PQString comma(", ");
    QString blank("");
    QString exportFile("MatchedCSV.csv");
    QDate defaultDate(1,1,1);

    /***************************************/
    /*   DETERMINE TYPE OF LOAD REQUIRED   */
    /***************************************/

    unsigned int runCode = dr.getPreviouslyLoaded();

    switch (runCode)
    {
    case 0:
        // Normal - new record
        forceInfoOverride = false;
        forceNewDeceasedID = false;
        acceptableOverride = false;
        deleteRecord = false;
        break;

    case 1:
        // Normal - new record, but only because original record in DB-deceased was deleted
        forceInfoOverride = false;
        forceNewDeceasedID = false;
        acceptableOverride = false;
        deleteRecord = false;
        break;

    case 100:
        // High match score, but confirmed to be a new record
        forceNewDeceasedID = true;
        forceInfoOverride = false;
        acceptableOverride = false;
        deleteRecord = false;
        break;

    case 101:
        // New information available confirmed, so force override of info
        forceNewDeceasedID = false;
        forceInfoOverride = true;
        acceptableOverride = true;
        deleteRecord = false;
        break;

    case 900:
        // Bad record not representing a real obit - Load ID to avoid processing again
        forceInfoOverride = false;
        forceNewDeceasedID = false;
        acceptableOverride = false;
        deleteRecord = false;
        break;

    default:
        // For now, runCode is existing deceasedID
        // Delete existing record(s) and load new one(s)
        forceNewDeceasedID = false;
        forceInfoOverride = false;
        acceptableOverride = false;
        deleteRecord = true;
        break;

    }

    /*********************************/
    /*    DELETE WHERE NECESSARY     */
    /*********************************/

    // Delete existing record then execute normal load after
    if (deleteRecord)
    {
        success = query.prepare("DELETE FROM deceased WHERE deceasedNumber = :deceasedNumber");
        query.bindValue(":deceasedNumber", QVariant(runCode));
        success = query.exec();
        query.clear();
    }

    /*********************************/
    /*   START NORMAL LOAD ROUTINE   */
    /*********************************/

    // Pull lastDeceasedNum from database
    query.prepare("SELECT MAX(deceasedNumber) FROM deceased");
    success = query.exec();
    query.next();

    if (!success || (query.size() != 1))
    {
        error = query.lastError();

        errorMessage << QString("SQL problem retrieving number of last deceased member while attempting to load: ") << dr.getLastName() << comma << dr.getFirstName();
        gv.logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }
    else
    {
        lastDeceasedNum = query.value(0).toUInt();
    }

    // First step is to search for a match to avoid having duplicate records loaded
    matchKeyResult = match(dr, matchResult);
    MATCHKEY temp = matchResult.getBestMatch();
    newInfo = matchResult.isNewInfoAvailable(matchResult.getBestMatchKeyID());
    sameFH = matchResult.fromSameFH(matchResult.getBestMatchKeyID());
    sufficientlyMatched = matchResult.isSufficientlyClose(matchResult.getBestMatchKeyID());

    if ((matchKeyResult >= mkNoKeyAltNameAlt) && !forceNewDeceasedID)
    {
        drDB = dbSearch.readRecord(matchResult.getBestMatchKeyID(), matchResult.getBestLastName(), &gv);
        DBrecordRead = true;
        drDB.wi.outstandingErrMsg = static_cast<int>(matchKeyResult);
        drCopy.wi.outstandingErrMsg = static_cast<int>(matchKeyResult);
        if (newInfo)
        {
            drDB.wi.validated = 1;
            drCopy.wi.validated = 1;
        }
        drDB.setGlobals(gv);
        drCopy.setGlobals(gv);
    }

    // Additional output for testMode
    if (testMode)
    {
        errorMessage << QString("Result of: ") << dr.getLastName() << comma << dr.getFirstName();
        errorMessage << QString("  - Match result: ") << MatchTypes.at(matchKeyResult);
        if ((matchKeyResult >= mkNoKeyAltNameAlt) && newInfo)
            errorMessage << QString("  - New information available! ");
        gv.logMsg(AuditListing, errorMessage);

        // Quit processing (i.e., don't load)
        return false;
    }

    // Determine what to do with any type of match to an existing record
    // 1. First time match (i.e., no 100 or 101 coded), record won't load and error message will be generated
    // 2. Expected match (i.e., 100 or 101 coded), determine how to update, if at all

    if ((matchKeyResult >= mkNoKey) && !forceNewDeceasedID)
    {
        if (matchResult.isSufficientlyClose(matchResult.getBestMatchKeyID()))
            acceptableOverride = true;
        else
            firstLoaded = gv.todaySQL;

        updateExistingRecord = forceInfoOverride || (acceptableOverride && (newInfo || !sameFH || sufficientlyMatched));
        if (updateExistingRecord)
        {
            // At least some of the existing DB records will be updated
            // Need to pull existing data from DB to compare and update
            if (!DBrecordRead)
                drDB = dbSearch.readRecord(matchResult.getBestMatchKeyID(), matchResult.getBestLastName(), &gv);
            drCopy.updateToBestOf(drDB);
            lastUpdated = gv.todaySQL;

            if (forceNewDeceasedID)
                firstLoaded = gv.todaySQL;
            /*else
            {
                errorMessage << QString(" Match Type: ") << MatchTypes.at(matchKeyResult);
                errorMessage << QString("  - Additional information available for: ") << dr.getLastName() << comma << dr.getFirstName();
                errorMessage << QString("  - Deceased ID (dr): ") << dr.getID();
                errorMessage << QString("  - Deceased ID (db): ") << matchResult.getBestID();
                errorMessage << QString("  - Provider info: ") << dr.getProvider() << dr.getProviderKey();
                gv.logMsg(ActionRequired, errorMessage);
            }*/
        }
        else
        {
            if (sameFH && !newInfo)
            {
                // Do nothing
                return false;
            }
            else
            {
                // Export for manual review
                errorMessage << QString(" Match Type: ") << MatchTypes.at(matchKeyResult);
                errorMessage << QString("  - Matched record for: ") << dr.getLastName() << comma << dr.getFirstName();
                errorMessage << QString("  - Deceased ID (dr): ") << dr.getID();
                errorMessage << QString("  - Deceased ID (db): ") << matchResult.getBestID();
                errorMessage << QString("  - Provider info: ") << dr.getProvider() << dr.getProviderKey();
                gv.logMsg(ActionRequired, errorMessage);

                drDB.xport(exportFile);
                drCopy.xport(exportFile);

                return false;
            }
        }
    }
    else
    {
        firstLoaded = gv.todaySQL;
    }

    // Determine number of records required per distinct deceasedID (one per last name variation)
    QList<QString> lastNames;
    QString tempName;
    //QString cycleLastName, cycleDOB;
    lastNames.append(drCopy.getLastName().getString());
    tempName = drCopy.getLastNameAlt1().getString();
    if (tempName.size() > 0)
    {
        lastNames.append(tempName);
        tempName = drCopy.getLastNameAlt2().getString();
        if (tempName.size() > 0)
        {
            lastNames.append(tempName);
            tempName = drCopy.getLastNameAlt3().getString();
            if (tempName.size() > 0)
            {
                lastNames.append(tempName);
            }
        }
    }
    unsigned int numRecs = static_cast<unsigned int>(lastNames.size());

    // Pad the QList with blanks to create four records
    for (unsigned int i = numRecs; i < 4; i++)
        lastNames.append(blank);

    // Load record(s)
    query.prepare("INSERT INTO deceased (lastName, firstName, middleNames, altLastName1, altLastName2, altLastName3, nameAKA1, nameAKA2, "
                  "prefix, suffix, gender, DOB, minDOB, maxDOB, DOD, YOB, YOD, ageAtDeath, spouseName, DOS, "
                  "language, weight, firstLoaded, lastUpdated, deceasedNumber) "
                  "VALUES (:lastName, :firstName, :middleNames, :altLastName1, :altLastName2, :altLastName3, :nameAKA1, :nameAKA2, "
                  ":prefix, :suffix, :gender, :DOB, :minDOB, :maxDOB, :DOD, :YOB, :YOD, :ageAtDeath, :spouseName, :DOS, "
                  ":language, :weight, :firstLoaded, :lastUpdated, :deceasedNumber)");
    query.bindValue(":firstName", QVariant(drCopy.getFirstName().getString()));
    query.bindValue(":middleNames", QVariant(drCopy.getMiddleNames().getString()));
    query.bindValue(":nameAKA1", QVariant(drCopy.getFirstNameAKA1().getString()));
    query.bindValue(":nameAKA2", QVariant(drCopy.getFirstNameAKA2().getString()));
    query.bindValue(":prefix", QVariant(drCopy.getPrefix().getString()));
    query.bindValue(":suffix", QVariant(drCopy.getSuffix().getString()));
    query.bindValue(":gender", QVariant(static_cast<int>(drCopy.getGender())));
    query.bindValue(":spouseName", QVariant(drCopy.getSpouseName()));

    dateSQL.clear();
    if (drCopy.getDOB().isValid())
        dateSQL << drCopy.getDOB().toString("yyyy/MM/dd") << QString(" 0:0:0");
    else
        dateSQL << defaultDate.toString("yyyy/MM/dd") << QString(" 0:0:0");
    query.bindValue(":DOB", QVariant(dateSQL.getString()));
    //cycleDOB = dateSQL.getString();

    dateSQL.clear();
    if (drCopy.getMinDOB().isValid())
        dateSQL << drCopy.getMinDOB().toString("yyyy/MM/dd") << QString(" 0:0:0");
    query.bindValue(":minDOB", QVariant(dateSQL.getString()));

    dateSQL.clear();
    if (drCopy.getMaxDOB().isValid())
        dateSQL << drCopy.getMaxDOB().toString("yyyy/MM/dd") << QString(" 0:0:0");
    query.bindValue(":maxDOB", QVariant(dateSQL.getString()));

    dateSQL.clear();
    if (drCopy.getDOD().isValid())
        dateSQL << drCopy.getDOD().toString("yyyy/MM/dd") << QString(" 0:0:0");
    query.bindValue(":DOD", QVariant(dateSQL.getString()));

    if (drCopy.getYOB() == 0)
        query.bindValue(":YOB", QVariant(QVariant::Int));
    else
        query.bindValue(":YOB", QVariant(drCopy.getYOB()));

    if (drCopy.getYOD() == 0)
        query.bindValue(":YOD", QVariant(QVariant::Int));
    else
        query.bindValue(":YOD", QVariant(drCopy.getYOD()));

    query.bindValue(":ageAtDeath", QVariant(drCopy.getAgeAtDeath()));

    dateSQL.clear();
    if (drCopy.getDOS().isValid())
        dateSQL << drCopy.getDOS().toString("yyyy/MM/dd") << QString(" 0:0:0");
    query.bindValue(":DOS", QVariant(dateSQL.getString()));

    query.bindValue(":language", QVariant(static_cast<int>(drCopy.getLanguage())));
    query.bindValue(":firstLoaded", QVariant(firstLoaded.getString()));
    if (lastUpdated.getLength() > 0)
        query.bindValue(":lastUpdated", QVariant(lastUpdated.getString()));
    else
        query.bindValue(":lastUpdated", QVariant(firstLoaded.getString()));
    if (!updateExistingRecord && !deleteRecord)
    {
        lastDeceasedNum++;
        drCopy.setDeceasedNumber(lastDeceasedNum);
    }
    if (deleteRecord)
        query.bindValue(":deceasedNumber", QVariant(runCode));
    else
        query.bindValue(":deceasedNumber", QVariant(drCopy.getDeceasedNumber()));

    /*******************************************************************/
    /*          Update fhLastObit in funeralhomedata database          */
    /*******************************************************************/

    dbSearch.updateLastObit(drCopy, &gv);

    /*******************************************************************/
    /*  Update deceasedidinfo - One record per deceasedNumber to add   */
    /*******************************************************************/

    loadSourceID(drCopy.getSourceID(), gv);

    /*******************************************************************/
    /*  Update deceasedlocation - One record per deceasedNumber to add */
    /*******************************************************************/

    pcInfo = drCopy.getPostalCodeInfo();
    dbSearch.savePostalCodeInfo(drCopy.getDeceasedNumber(), drCopy.getProvider(), drCopy.getProviderKey(), pcInfo);

    /*******************************************************************/
    /* Update deceased - Up to max 4 records per deceasedNumber to add */
    /*******************************************************************/

    for (unsigned int i = 1; i <= numRecs; i++)
    {
        // Set last names for record
        switch(i)
        {
        case 1:
            query.bindValue(":lastName", QVariant(lastNames.value(0)));
            query.bindValue(":altLastName1", QVariant(lastNames.value(1)));
            query.bindValue(":altLastName2", QVariant(lastNames.value(2)));
            query.bindValue(":altLastName3", QVariant(lastNames.value(3)));
            //cycleLastName = lastNames.value(0);

            drCopy.clearLastNames();
            drCopy.setFamilyName(lastNames.value(0));
            drCopy.setFamilyName(lastNames.value(1));
            drCopy.setFamilyName(lastNames.value(2));
            drCopy.setFamilyName(lastNames.value(3));
            break;

        case 2:
            query.bindValue(":lastName", QVariant(lastNames.value(1)));
            query.bindValue(":altLastName1", QVariant(lastNames.value(0)));
            query.bindValue(":altLastName2", QVariant(lastNames.value(2)));
            query.bindValue(":altLastName3", QVariant(lastNames.value(3)));
            //cycleLastName = lastNames.value(1);

            drCopy.clearLastNames();
            drCopy.setFamilyName(lastNames.value(1));
            drCopy.setFamilyName(lastNames.value(0));
            drCopy.setFamilyName(lastNames.value(2));
            drCopy.setFamilyName(lastNames.value(3));
            break;

        case 3:
            query.bindValue(":lastName", QVariant(lastNames.value(2)));
            query.bindValue(":altLastName1", QVariant(lastNames.value(0)));
            query.bindValue(":altLastName2", QVariant(lastNames.value(1)));
            query.bindValue(":altLastName3", QVariant(lastNames.value(3)));
            //cycleLastName = lastNames.value(2);

            drCopy.clearLastNames();
            drCopy.setFamilyName(lastNames.value(2));
            drCopy.setFamilyName(lastNames.value(0));
            drCopy.setFamilyName(lastNames.value(1));
            drCopy.setFamilyName(lastNames.value(3));
            break;

        case 4:
            query.bindValue(":lastName", QVariant(lastNames.value(3)));
            query.bindValue(":altLastName1", QVariant(lastNames.value(0)));
            query.bindValue(":altLastName2", QVariant(lastNames.value(1)));
            query.bindValue(":altLastName3", QVariant(lastNames.value(2)));
            //cycleLastName = lastNames.value(3);

            drCopy.clearLastNames();
            drCopy.setFamilyName(lastNames.value(3));
            drCopy.setFamilyName(lastNames.value(0));
            drCopy.setFamilyName(lastNames.value(1));
            drCopy.setFamilyName(lastNames.value(2));
            break;
        }

        // Set weight for record
        if (drCopy.getPreviouslyLoaded() == 900)
            weight = 0;
        else
        {
            weight = int(100.00 / numRecs) / 100.00;
            if (i == numRecs)
                weight = 1.00 - (i - 1) * weight;
        }
        query.bindValue(":weight", QVariant(weight));

        // If record flagged as override, determine if this particular lastName/deceasedNumber combination is a new record or an UPDATE
        newRecord = true;

        if (updateExistingRecord)
        {
            /********************************/
            /*   FORCE OVERRIDE SITUATION   */
            /********************************/

            success = overrideQuery.prepare("SELECT firstLoaded FROM death_audits.deceased WHERE deceasedNumber = :deceasedNumber AND lastName = :lastName");

            overrideQuery.bindValue(":lastName", QVariant(lastNames.value(i - 1)));
            overrideQuery.bindValue(":deceasedNumber", QVariant(drCopy.getDeceasedNumber()));

            success = overrideQuery.exec();
            success = success & (overrideQuery.size() == 1);                

            if (success)
            {
                // If success, we are updating a deceasedID and not creating a new one
                overrideQuery.next();
                QDate fl = overrideQuery.value(0).toDate();
                drCopy.setFirstLoaded(fl);

                newRecord = false;
            }
            else
            {
                errorMessage.clear();
                errorMessage << "New record assumed on override for " << drCopy.getLastName() << ", " << drCopy.getFirstName();
                gv.logMsg(ErrorRecord, errorMessage);
            }
        }

        if (!newRecord)
        {
            updateRecord(drCopy, weight, gv.todaySQL);
        }
        else
        {
            /***************************/
            /*  NORMAL LOAD PROCEDURE  */
            /***************************/

            success = query.exec();

            if (!success)
            {
                errorMessage.clear();
                errorMessage << QString("SQL error trying to load record: ") << dr.getLastName() << comma << dr.getFirstName();

                error = query.lastError();
                if (error.isValid())
                {
                    QString DBerrorDesc = error.databaseText();
                    QString DriverErrorDesc = error.driverText();
                    errorMessage << QString(" - ") << DBerrorDesc;
                }
                QString lastQ = query.lastQuery();

                gv.logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
            }
            query.next();

            loaded = loaded && success;

        }
    }  // end of for loop for each lastName variation

    if (loaded && newRecord)
        dbSearch.updateLastObit(drCopy, &gv);

    newRec = newRecord;

    return loaded;
}

bool loadSourceID(const SOURCEID &sid, GLOBALVARS gv)
{
    QSqlQuery query, overrideQuery;
    QSqlError error;
    PQString errorMessage, dateSQL;
    bool success;

    if ((sid.deceasedNumber == 0) || (sid.provider == (PROVIDER)0) || (sid.providerKey == 0) || (sid.URL.getLength() == 0))
    {
        errorMessage << "An unexpected load sourceID error occurred with: " << sid.provider << "-" << sid.providerKey << "-" << sid.ID;
        gv.logMsg(ErrorRecord, errorMessage);
        return false;
    }

    dateSQL.clear();
    if (sid.publishDate.isValid())
        dateSQL << sid.publishDate.toString("yyyy/MM/dd") << QString(" 0:0:0");

    success = query.prepare("INSERT INTO death_audits.deceasedidinfo (deceasedNumber, providerID, providerKey, ID, url, publishDate) "
                            "VALUES(:deceasedNumber, :providerID, :providerKey, :ID, :url, :publishDate)");

    query.bindValue(":deceasedNumber", QVariant(sid.deceasedNumber));
    query.bindValue(":providerID", QVariant(sid.provider));
    query.bindValue(":providerKey", QVariant(sid.providerKey));
    query.bindValue(":ID", QVariant(sid.ID.getString()));
    query.bindValue(":url", QVariant(sid.URL.getString()));
    query.bindValue(":publishDate", QVariant(dateSQL.getString()));

    success = query.exec();

    //
    if (!success)
    {
        // Likely a duplicate entry due to a new publish date - Update entry
        query.clear();
        success = query.prepare("UPDATE death_audits.deceasedidinfo SET publishDate = :publishDate "
                                "WHERE deceasedNumber = :deceasedNumber AND providerID = :providerID AND providerKey = :providerKey AND ID = :ID and url = :url");

        query.bindValue(":deceasedNumber", QVariant(sid.deceasedNumber));
        query.bindValue(":providerID", QVariant(sid.provider));
        query.bindValue(":providerKey", QVariant(sid.providerKey));
        query.bindValue(":ID", QVariant(sid.ID.getString()));
        query.bindValue(":url", QVariant(sid.URL.getString()));
        query.bindValue(":publishDate", QVariant(dateSQL.getString()));

        success = query.exec();
    }

    return success;
}

bool validate(QString &name)
{
    bool valid = true;

    if (name.right(1) == ".")
        name.chop(1);

    if (name.size() < 2)
        valid = false;
    else
    {
        if (name.mid(1,1) == ".")
            valid = false;
    }

    return valid;
}

void updateFirstNameDB(const dataRecord &dr)
{
    // Don't do anything if gender is unknown
    if (dr.getGender() == genderUnknown)
        return;

    GLOBALVARS gv = dr.globals;
    databaseSearches dbSearch;

    QSqlQuery query;
    QSqlError error;

    bool success;

    // Gather the list of all first names
    QList<QString> firstNames;
    QString tempName, tempWord;
    PQString errorMessage;
    PQStream tempStream;

    tempName = dr.getFirstName().getString();
    if (validate(tempName))
        firstNames.append(tempName);

    tempName = dr.getFirstNameAKA1().getString();
    if (validate(tempName))
        firstNames.append(tempName);

    tempName = dr.getFirstNameAKA2().getString();
    if (validate(tempName))
        firstNames.append(tempName);

    tempName = dr.getMiddleNames().getString();
    if (tempName.size() > 1)
    {
        tempStream = PQString(tempName);
        tempWord = tempStream.getWord().getString();
        while (validate(tempWord))
        {
            // For middle names, only add to the name count if the name is already a firstname and not a surname
            bool isSurname = dbSearch.surnameLookup(tempWord, &gv);
            bool isGivenName = dbSearch.givenNameLookup(tempWord, &gv);
            if (isGivenName && !isSurname)
                firstNames.append(tempWord);
            tempWord = tempStream.getWord().getString();
        }
    }
    int numRecs = firstNames.size();

    // Load name into database and update statistics
    unsigned int maleCount, femaleCount;
    double malePct, totalCount;

    for (int i = 0; i < numRecs; i++)
    {
        query.prepare("SELECT name, maleCount, femaleCount FROM firstnames WHERE name = :name");
        query.bindValue(":name", QVariant(firstNames.value(i)));
        success = query.exec();
        query.next();

        if (!success || (query.size() > 1))
        {
            error = query.lastError();

            errorMessage << QString("SQL error trying to determine if firstnames already contains: ") << firstNames.value(i);
            gv.logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
        }
        else
        {
            if (query.size() == 0)  // new name to be loaded
            {
                maleCount = femaleCount = 0;
                if (dr.getGender() == Male)
                    maleCount++;
                else
                    femaleCount++;
                totalCount = double(maleCount + femaleCount);
                malePct = maleCount / totalCount;

                query.prepare("INSERT INTO firstnames(name, maleCount, femaleCount, malePct) VALUES (:name, :maleCount, :femaleCount, :malePct)");
                query.bindValue(":name", QVariant(firstNames.value(i)));
                query.bindValue(":maleCount", QVariant(maleCount));
                query.bindValue(":femaleCount", QVariant(femaleCount));
                query.bindValue(":malePct", QVariant(malePct));
                success = query.exec();
                query.next();

                if (!success)
                {
                    error = query.lastError();

                    errorMessage << QString("SQL error trying to add new first name: ") << firstNames.value(i);
                    gv.logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
                }
            }
            else    // name exists
            {
                // Pull current counts from record matched
                maleCount = query.value(1).toUInt();
                femaleCount = query.value(2).toUInt();
                if (dr.getGender() == Male)
                    maleCount++;
                else
                    femaleCount++;
                totalCount = double(maleCount + femaleCount);
                malePct = maleCount / totalCount;

                query.prepare("UPDATE firstnames SET maleCount = :maleCount, femaleCount = :femaleCount, malePct = :malePct WHERE name = :name");
                query.bindValue(":name", QVariant(firstNames.value(i)));
                query.bindValue(":maleCount", QVariant(maleCount));
                query.bindValue(":femaleCount", QVariant(femaleCount));
                query.bindValue(":malePct", QVariant(malePct));
                success = query.exec();
                query.next();

                if (!success)
                {
                    error = query.lastError();

                    errorMessage << QString("SQL error trying to update first name: ") << firstNames.value(i);
                    gv.logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
                }
            }
        }
    }
}

void updateDrToBestOf(dataRecord &dr, PQString &middlenames, PQString &nameAKA1, PQString &nameAKA2, GENDER &gender)
{
    PQString tempName;
    PQString tempNAME;
    QList<PQString> nameList;
    databaseSearches dbSearch;

    // Deal with easy case of Middlenames - Will automatically just add new names
    dr.setMiddleNames(middlenames);

    // Deal with first names
    tempName = dr.getFirstName();
    tempNAME = tempName;
    nameList.append(tempNAME);

    tempName = dr.getFirstNameAKA1();
    if (tempName.getLength() > 0)
    {
        tempNAME = tempName;
        nameList.append(tempNAME);

        tempName = dr.getFirstNameAKA2();
        if (tempName.getLength() > 0)
        {
            tempNAME = tempName;
            nameList.append(tempNAME);
        }
    }

    if (nameAKA1.getLength() > 0)
    {
        bool matched = false;
        int i = 0;
        while (!matched && (i < nameList.size()))
        {
            matched = (nameAKA1 == nameList.at(i));
            i++;
        }
        if (!matched)
            nameList.append(nameAKA1);
    }

    if (nameAKA2.getLength() > 0)
    {
        bool matched = false;
        int i = 0;
        while (!matched && (i < nameList.size()))
        {
            matched = (nameAKA2 == nameList.at(i));
            i++;
        }
        if (!matched)
            nameList.append(nameAKA2);
    }

    // Remove pure nicknames
    int i = 0;
    while(i < nameList.size())
    {
        if (dbSearch.pureNickNameLookup(nameList.at(i).getString(), &dr.globals))
            nameList.removeAt(i);
        else
            i++;
    }

    // Re-sort
    int listSize = nameList.size();
    OQString OQname;
    PQString errMsg;
    if ( listSize > 1)
    {
        for (int i = 0; i < listSize; i++)
        {
            for (int j = i+1; j < listSize; j++)
            {
                OQname = nameList.at(i);
                if (OQname.isInformalVersionOf(nameList.at(j).getString(), errMsg))
                {
                    tempNAME = nameList.takeAt(j);
                    nameList.insert(i, tempNAME);
                }
            }
        }
    }

    if (nameList.size() > 2)
    {
        PQString errMsg;
        errMsg << "Loss of first name in 'best of' merge for: " << dr.getID();
        dr.globals.logMsg(ErrorRecord, errMsg);

        while (nameList.size() > 2)
            nameList.removeLast();
    }
    if (nameList.size() > 0)
    {
        dr.setFirstName(nameList.takeFirst(), 2);
        if (nameList.size() > 0)
            dr.setFirstName(nameList.takeFirst(), 3);
    }

    if (gender != genderUnknown)
    {
        if (gender != dr.getGender())
        {
            PQString errMsg;
            errMsg << "Mismatched genders in 'best of' merge for: " << dr.getID();
            dr.globals.logMsg(ErrorRecord, errMsg);
        }
    }
}

bool updateRecord(const dataRecord &dr, const double weight, const PQString updateDate)
{
    // Only updates record if match lastname and deceasedNumber

    GLOBALVARS gv = dr.globals;

    QSqlQuery query;
    QSqlError error;

    PQString dateSQL, errorMessage;
    bool success;

    QDate defaultDate(1,1,1);

    QDate DOB = dr.getDOB();
    QDate DOD = dr.getDOD();
    QDate DOS = dr.getDOS();
    QDate minDOB = dr.getMinDOB();
    QDate maxDOB = dr.getMaxDOB();
    unsigned int YOB = dr.getYOB();
    unsigned int YOD = dr.getYOD();
    unsigned int ageAtDeath = dr.getAgeAtDeath();
    unsigned int deceasedNumber = dr.getDeceasedNumber();

    success = query.prepare("UPDATE deceased "
                            "SET firstName = :firstName, middleNames = :middleNames, altLastName1 = :altLastName1,  "
                                "altLastName2 = :altLastName2, altLastName3 = :altLastName3, nameAKA1 = :nameAKA1, "
                                "nameAKA2 = :nameAKA2, prefix = :prefix, suffix = :suffix, gender = :gender, "
                                "DOB = :DOB, minDOB = :minDOB, maxDOB = :maxDOB, DOD = :DOD, YOB = :YOB, YOD = :YOD, "
                                "ageAtDeath = :ageAtDeath, DOS = :DOS, weight = :weight, lastUpdated = :lastUpdated "
                                "WHERE lastName = :lastName AND deceasedNumber = :deceasedNumber");

    query.bindValue(":firstName", QVariant(dr.getFirstName().getString()));
    query.bindValue(":middleNames", QVariant(dr.getMiddleNames().getString()));
    query.bindValue(":altLastName1", QVariant(dr.getLastNameAlt1().getString()));
    query.bindValue(":altLastName2", QVariant(dr.getLastNameAlt2().getString()));
    query.bindValue(":altLastName3", QVariant(dr.getLastNameAlt3().getString()));
    query.bindValue(":nameAKA1", QVariant(dr.getFirstNameAKA1().getString()));
    query.bindValue(":nameAKA2", QVariant(dr.getFirstNameAKA2().getString()));
    query.bindValue(":prefix", QVariant(dr.getPrefix().getString()));
    query.bindValue(":suffix", QVariant(dr.getSuffix().getString()));
    query.bindValue(":gender", QVariant(static_cast<int>(dr.getGender())));

    dateSQL.clear();
    if (DOB.isValid())
        dateSQL << DOB.toString("yyyy/MM/dd") << QString(" 0:0:0");
    else
        dateSQL << defaultDate.toString("yyyy/MM/dd") << QString(" 0:0:0");
    query.bindValue(":DOB", QVariant(dateSQL.getString()));

    dateSQL.clear();
    if (minDOB.isValid())
        dateSQL << minDOB.toString("yyyy/MM/dd") << QString(" 0:0:0");
    query.bindValue(":minDOB", QVariant(dateSQL.getString()));

    dateSQL.clear();
    if (maxDOB.isValid())
        dateSQL << maxDOB.toString("yyyy/MM/dd") << QString(" 0:0:0");
    query.bindValue(":maxDOB", QVariant(dateSQL.getString()));

    dateSQL.clear();
    if (DOD.isValid())
        dateSQL << DOD.toString("yyyy/MM/dd") << QString(" 0:0:0");
    query.bindValue(":DOD", QVariant(dateSQL.getString()));

    if (YOB == 0)
        query.bindValue(":YOB", QVariant(QVariant::Int));
    else
        query.bindValue(":YOB", QVariant(YOB));

    if (YOD == 0)
        query.bindValue(":YOD", QVariant(QVariant::Int));
    else
        query.bindValue(":YOD", QVariant(YOD));

    query.bindValue(":ageAtDeath", QVariant(ageAtDeath));

    dateSQL.clear();
    if (DOS.isValid())
        dateSQL << DOS.toString("yyyy/MM/dd") << QString(" 0:0:0");
    query.bindValue(":DOS", QVariant(dateSQL.getString()));

    query.bindValue(":weight", QVariant(weight));
    query.bindValue(":lastUpdated", QVariant(updateDate.getString()));

    query.bindValue(":lastName", QVariant(dr.getLastName().getString()));
    query.bindValue(":deceasedNumber", QVariant(deceasedNumber));

    success = query.exec();

    if (!success)
    {
        error = query.lastError();
        int errorType = static_cast<int>(error.type());

        errorMessage << QString("Unable to UPDATE record for: ") << dr.getDeceasedNumber() << " - " << error.databaseText();
        gv.logMsg(ErrorRecord, errorMessage, errorType);
        return false;
    }

    return success;
}
