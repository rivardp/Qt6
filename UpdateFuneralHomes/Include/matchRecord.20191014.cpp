#include "../Include/matchRecord.h"


MATCHRECORD::MATCHRECORD() :  recordID(0), score(0), netScore(0), inconsistencyCount(0), inconsistencyScore(0), newInfoAvailable(false), isAnalyzed(false),
                              matchedLNAME(false), matchedLNAMEALT(false), matchedDOB(false), matchedFNAME(false),
                              matchedDOD(false), matchedGENDER(false), matchedDODinfo(false), matchedDOBinfo(false),
                              matchedPREFIX(false), matchedSUFFIX(false)
{
}

void MATCHRECORD::clear()
{
    recordID = 0;
    dataCount = 0;
    score = 0;
    netScore = 0;
    inconsistencyCount = 0;
    inconsistencyScore = 0;
    newInfoAvailable = false;
    isAnalyzed = false;

    matchedLNAME = false;
    matchedLNAMEALT = false;
    matchedDOB = false;
    matchedFNAME = false;
    matchedDOD = false;
    matchedGENDER = false;
    matchedDODinfo = false;
    matchedDOBinfo = false;
    matchedPREFIX = false;
    matchedSUFFIX = false;
}

void MATCHRECORD::setID(unsigned int id)
{
    recordID = id;
}

void MATCHRECORD::setMatchKey(MATCHKEY mk)
{
    matchKey = mk;
}

void MATCHRECORD::addToScore(int points)
{
    score = score | points;
    isAnalyzed = false;
}

void MATCHRECORD::addToInconsistencyScore(int points)
{
    inconsistencyScore = inconsistencyScore | points;
    isAnalyzed = false;
}

unsigned int MATCHRECORD::countItems(int score)
{
    unsigned int result = 0;

    if (score & mPREFIX)
        result++;
    if (score & mSUFFIX)
        result++;
    if (score & mMIDDLENAMES)
        result++;
    if (score & mMINDOB)
        result++;
    if (score & mMAXDOB)
        result++;
    if (score & mYOB)
        result++;
    if (score & mYOD)
        result++;
    if (score & mGENDER)
        result++;
    if (score & mDOD)
        result++;
    if (score & mFNAMEAKA2)
        result++;
    if (score & mFNAMEAKA1)
        result++;
    if (score & mFNAME)
        result++;
    if (score & mDOB)
        result++;
    if (score & mDOD)
        result++;
    if (score & mLNAMEALT3)
        result++;
    if (score & mLNAMEALT2)
        result++;
    if (score & mLNAMEALT1)
        result++;
    if (score & mLNAME)
        result++;

    return result;
}

unsigned int MATCHRECORD::countDataItems(int score)
{
    unsigned int count = 0;

    while (score > 0)
    {
        count += score & 1;
        score >>= 1;
    }

    return count;
}

void MATCHRECORD::analyze()
{
    if (isAnalyzed)
        return;

    matchKey = mkNone;

    inconsistencyCount = countDataItems(inconsistencyScore);
    dataCount = countDataItems(score) + inconsistencyCount;

    matchedLNAME = ((score & mLNAME) == mLNAME);
    matchedLNAMEALT = ((score & mLNAMEALT1) == mLNAMEALT1) || ((score & mLNAMEALT2) == mLNAMEALT2) || ((score & mLNAMEALT3) == mLNAMEALT3);
    matchedDOB = ((score & mDOB) == mDOB);
    matchedFNAME = ((score & mFNAME) == mFNAME) || ((score & mFNAMEAKA1) == mFNAMEAKA1) || ((score & mFNAMEAKA2) == mFNAMEAKA2);
    matchedDOD = ((score & mDOD) == mDOD);
    matchedGENDER = ((score & mGENDER) == mGENDER);
    matchedDODinfo = ((score & mYOD) == mYOD);
    matchedDOBinfo = ((score & mMINDOB) == mMINDOB);    // This is intentional to capture a number of types of matches
    matchedPREFIX = ((score & mPREFIX) == mPREFIX);
    matchedSUFFIX = ((score & mSUFFIX) == mSUFFIX);

    if (matchedDOB && matchedLNAME && matchedFNAME)
        matchKey = mkKey;
    else
    {
        if (matchedDOB && matchedLNAMEALT && matchedFNAME)
            matchKey = mkAltKey;
        else
        {
            // TODO: Make this more sophisticated
            if ((dataCount - inconsistencyCount) >= 5)
                matchKey = mkNoKey;
            else
            {
                matchKey = mkNone;
                newInfoAvailable = false;
            }
        }
    }

    isAnalyzed = true;
}

void MATCHRECORD::compareGenders(unsigned int recGender, unsigned int DBgender)
{
    if ((recGender > 0) && (DBgender > 0))
    {
        if (recGender == DBgender)
            addToScore(mGENDER);
        else
            addToInconsistencyScore(mGENDER);
    }

    if ((recGender > 0) && (DBgender == 0))
        newInfoAvailable = true;
}

void MATCHRECORD::compareLastNames(QStringList &recLastNameList, QStringList &DBlastNameList)
{
    // Positions are determined by reference to the DB record (i.e., the database key)

    bool matched;

    for (int i = 0; i < recLastNameList.size(); i++)
    {
        matched = false;

        for (int j = 0; j < DBlastNameList.size(); j++)
        {
            if (DBlastNameList[j] == recLastNameList[i])
            {
                matched = true;

                switch(j)
                {
                case 0:
                    addToScore(mLNAME);
                    break;

                case 1:
                    addToScore(mLNAMEALT1);
                    break;

                case 2:
                    addToScore(mLNAMEALT2);
                    break;

                case 3:
                    addToScore(mLNAMEALT3);
                    break;
                }
            }
        }

        if (!matched)
            newInfoAvailable = true;
    }
}

void MATCHRECORD::compareFirstNames(QStringList &recFirstNameList, QStringList &DBfirstNameList)
{
    // Positions are determined by reference to the DB record (i.e., the database key)
    // TODO:  More sophisticated comparison grouping nicknames with their formal names, along with counting of non-matches

    bool matched;

    for (int i = 0; i < recFirstNameList.size(); i++)
    {
        matched = false;

        for (int j = 0; j < DBfirstNameList.size(); j++)
        {
            if (recFirstNameList[i].size() == 1)
            {
                if (DBfirstNameList[j].left(1) == recFirstNameList[i])
                {
                    addToScore(mFNAME);
                    matched = true;
                }
            }
            else
            {
                if (DBfirstNameList[j] == recFirstNameList[i])
                {
                    addToScore(mFNAME);
                    matched = true;
                }
            }
        }

        if (!matched)
            newInfoAvailable = true;
    }
}

void MATCHRECORD::compareMiddleNames(QStringList &recMiddleNameList, QStringList &DBmiddleNameList)
{
    // For consistency, positions are determined by reference to the DB record

    bool matched;

    for (int i = 0; i < recMiddleNameList.size(); i++)
    {
        matched = false;

        for (int j = 0; j < DBmiddleNameList.size(); j++)
        {
            if (recMiddleNameList[i].size() == 1)
            {
                if (DBmiddleNameList[j].left(1) == recMiddleNameList[i])
                {
                    addToScore(mMIDDLENAMES);
                    matched = true;
                }
            }
            else
            {
                if (DBmiddleNameList[j] == recMiddleNameList[i])
                {
                    addToScore(mMIDDLENAMES);
                    matched = true;
                }
            }
        }

        if (!matched)
            newInfoAvailable = true;
    }
}

void MATCHRECORD::compareDOBinfo(DATEINFO &recDateInfo, DATEINFO &DBdateInfo)
{
    // Check for match
    if (recDateInfo.exactDate.isValid() && DBdateInfo.exactDate.isValid())
    {
        if (recDateInfo.exactDate == DBdateInfo.exactDate)
            addToScore(mDOB);
        else
            addToInconsistencyScore(mDOB);
    }
    else
    {
        if (recDateInfo.exactDate.isValid() && !DBdateInfo.exactDate.isValid())
        {
            if (DBdateInfo.minDate.isValid())
            {
                if ((recDateInfo.exactDate >= DBdateInfo.minDate) && (recDateInfo.exactDate <= DBdateInfo.maxDate))
                {
                    addToScore(mDOB);
                    newInfoAvailable = true;
                }
                else
                    addToInconsistencyScore(mDOB);
            }
            else
            {
                newInfoAvailable = true;
            }
        }
        else
        {
            if (!recDateInfo.exactDate.isValid() && !DBdateInfo.exactDate.isValid() && recDateInfo.minDate.isValid())
            {
                if (DBdateInfo.minDate.isValid())
                {
                    if ((recDateInfo.minDate >= DBdateInfo.minDate) && (recDateInfo.maxDate <= DBdateInfo.maxDate))
                    {
                        addToScore(mMINDOB);
                        addToScore(mMAXDOB);
                    }
                    else
                    {
                        addToInconsistencyScore(mMINDOB);
                        addToInconsistencyScore(mMAXDOB);
                    }
                }
                else
                {
                    newInfoAvailable = true;
                }
            }
        }
    }
}

void MATCHRECORD::compareDODinfo(DATEINFO &recDateInfo, DATEINFO &DBdateInfo)
{
    // Check for match
    if (recDateInfo.exactDate.isValid() && DBdateInfo.exactDate.isValid())
    {
        if (recDateInfo.exactDate == DBdateInfo.exactDate)
            addToScore(mDOD);
        else
            addToInconsistencyScore(mDOD);
    }
    else
    {
        if ((recDateInfo.year > 0) && (DBdateInfo.year > 0))
        {
            if (recDateInfo.year == DBdateInfo.year)
                addToScore(mYOD);
            else
                addToInconsistencyScore(mYOD);
        }
    }

    // Check for any new information
    if (recDateInfo.exactDate.isValid() && !DBdateInfo.exactDate.isValid())
        newInfoAvailable = true;
    else
    {
        if ((recDateInfo.year > 0) && (DBdateInfo.year == 0))
            newInfoAvailable = true;
    }
}

MATCHRESULT::MATCHRESULT() : bestScore(0), bestNetScore(0), bestID(0), bestNetID(0), bestMatchKeyID(0)
{
    matchRecordList.clear();
    bestMatchKey = mkNone;
}

void MATCHRESULT::clear()
{
    matchRecordList.clear();
    bestMatchKey = mkNone;
    bestScore = 0;
    bestNetScore = 0;
    bestID = 0;
    bestNetID = 0;
    bestMatchKeyID = 0;
}

void MATCHRESULT::update(unsigned int id, int score, int numInconsistency, int scoreInconsistency, bool newInfo)
{
    if (this->alreadyContains(id))
    {
        // Update criteria based on net score
        if ((score - inconsistencyCost * numInconsistency) > getNetScore(id))
            setScore(id, score, numInconsistency, scoreInconsistency, newInfo);
        else;
            // Leave current scores
    }
    else
        addScore(id, score, numInconsistency, scoreInconsistency, newInfo);

}

void MATCHRESULT::update(MATCHRECORD &mr)
{
    if (this->alreadyContains(mr.recordID))
    {
        // Update criteria based on net score
        if (mr.matchKey > getMatchKey(mr.recordID))
            setScore(mr);
        else;
            // Leave current scores
    }
    else
        addScore(mr);

}

int MATCHRESULT::getBestScore() const
{
    return bestScore;
}

int MATCHRESULT::getBestNetScore() const
{
    return bestNetScore;
}

MATCHKEY MATCHRESULT::getBestMatch() const
{
    return bestMatchKey;
}

unsigned int MATCHRESULT::getBestID() const
{
    return bestID;
}

unsigned int MATCHRESULT::getBestNetID() const
{
    return bestNetID;
}

unsigned int MATCHRESULT::getBestMatchKeyID() const
{
    return bestMatchKeyID;
}

int MATCHRESULT::getScore(unsigned int id) const
{
    int result = -1;
    int i = 0;
    bool matched = false;

    while (!matched && (i < matchRecordList.size()))
    {
        if (id == matchRecordList.at(i).recordID)
        {
            matched = true;
            result = matchRecordList.at(i).score;
        }
        i++;
    }

    return result;
}

int MATCHRESULT::getNetScore(unsigned int id) const
{
    int result = -1;
    int i = 0;
    bool matched = false;

    while (!matched && (i < matchRecordList.size()))
    {
        if (id == matchRecordList.at(i).recordID)
        {
            matched = true;
            result = matchRecordList.at(i).netScore;
        }
        i++;
    }

    return result;
}

MATCHKEY MATCHRESULT::getMatchKey(unsigned int id) const
{
    MATCHKEY result = mkNone;
    int i = 0;
    bool matched = false;

    while (!matched && (i < matchRecordList.size()))
    {
        if (id == matchRecordList.at(i).recordID)
        {
            matched = true;
            result = matchRecordList.at(i).matchKey;
        }
        i++;
    }

    return result;
}

void MATCHRESULT::setScore(unsigned int id, int score, int numInconsistency, int scoreInconsistency, bool newInfo)
{
    // Nothing is updated if ID is not matched

    int i = 0;
    bool matched = false;
    int netScore;

    while (!matched && (i < matchRecordList.size()))
    {
        if (id == matchRecordList.at(i).recordID)
        {
            matched = true;
            matchRecordList[i].score = score;
            matchRecordList[i].netScore = netScore = score - inconsistencyCost * numInconsistency;
            matchRecordList[i].inconsistencyCount = static_cast<unsigned int>(numInconsistency);
            matchRecordList[i].inconsistencyScore = scoreInconsistency;
            matchRecordList[i].newInfoAvailable = newInfo;

            if (score > bestScore)
            {
                bestScore = score;
                bestID = id;
            }

            if (netScore > bestNetScore)
            {
                bestNetScore = netScore;
                bestNetID = id;
            }
        }

        i++;
    }
}

void MATCHRESULT::setScore(MATCHRECORD &mr)
{
    // Nothing is updated if ID is not matched

    int i = 0;
    bool matched = false;
    int netScore;

    while (!matched && (i < matchRecordList.size()))
    {
        if (mr.recordID == matchRecordList.at(i).recordID)
        {
            matched = true;
            if (!mr.isAnalyzed)
                mr.analyze();

            matchRecordList[i].score = mr.score;
            matchRecordList[i].netScore = netScore = mr.score - inconsistencyCost * static_cast<int>(mr.inconsistencyCount);
            matchRecordList[i].inconsistencyCount = mr.inconsistencyCount;
            matchRecordList[i].inconsistencyScore = mr.inconsistencyScore;
            matchRecordList[i].newInfoAvailable = mr.newInfoAvailable;
            matchRecordList[i].matchKey = mr.matchKey;

            if (mr.score > bestScore)
            {
                bestScore = mr.score;
                bestID = mr.recordID;
            }

            if (netScore > bestNetScore)
            {
                bestNetScore = netScore;
                bestNetID = mr.recordID;
            }

            if (mr.matchKey > bestMatchKey)
            {
                bestMatchKey = mr.matchKey;
                bestMatchKeyID = mr.recordID;
            }
        }

        i++;
    }
}

void MATCHRESULT::addScore(unsigned int id, int score, int numInconsistency, int scoreInconsistency, bool newInfo)
{
    int netScore = score - inconsistencyCost * numInconsistency;

    MATCHRECORD newRecord;
    newRecord.recordID = id;
    newRecord.score = score;
    newRecord.netScore = netScore;
    newRecord.inconsistencyCount = static_cast<unsigned int>(numInconsistency);
    newRecord.inconsistencyScore = scoreInconsistency;
    newRecord.newInfoAvailable = newInfo;

    matchRecordList.append(newRecord);

    if (score > bestScore)
    {
        bestScore = score;
        bestID = id;
    }

    if (netScore > bestNetScore)
    {
        bestNetScore = netScore;
        bestNetID = id;
    }
}

void MATCHRESULT::addScore(MATCHRECORD &mr)
{
    if (!mr.isAnalyzed)
        mr.analyze();

    matchRecordList.append(mr);

    if (mr.score > bestScore)
    {
        bestScore = mr.score;
        bestID = mr.recordID;
    }

    if (mr.netScore > bestNetScore)
    {
        bestNetScore = mr.netScore;
        bestNetID = mr.recordID;
    }

    if (mr.matchKey > bestMatchKey)
    {
        bestMatchKey = mr.matchKey;
        bestMatchKeyID = mr.recordID;
    }
}

bool MATCHRESULT::alreadyContains(unsigned int id) const
{
    int i = 0;
    bool matched = false;

    while (!matched && (i < matchRecordList.size()))
    {
        if (id == matchRecordList.at(i).recordID)
            matched = true;
        i++;
    }

    return matched;
}

bool MATCHRESULT::isNewInfoAvailable(unsigned int id) const
{
    bool result = false;
    int i = 0;
    bool matched = false;

    while (!matched && (i < matchRecordList.size()))
    {
        if (id == matchRecordList.at(i).recordID)
        {
            matched = true;
            result = matchRecordList.at(i).newInfoAvailable;
        }
        i++;
    }

    return result;
}

void MATCHRESULT::sort()
{
    // Sorts based on highest to lowest netScore
    // Uses insertion sort due to expected high number of repeat scores for longer lists

    int valueToInsert;
    MATCHRECORD recordToInsert;
    int holePosition;
    int i;

    int numRecs = matchRecordList.size();

    // Loop through all records
    for (i = 1; i < numRecs; i++)
    {

        // Select the hole position of the number to be inserted
        holePosition = i;

       // Select the value to be inserted.
       recordToInsert = matchRecordList.at(i);
       valueToInsert = recordToInsert.netScore;

       // Shift all previously sorted numbers right until the same or higher number is encountered
       while ((holePosition > 0) && (matchRecordList.at(i-1).netScore < valueToInsert))
       {
          matchRecordList[holePosition] = matchRecordList[holePosition-1];
          holePosition--;
       }

       // Insert the new record at its proper position in the sorted list
       if (holePosition != i)
       {
          matchRecordList[holePosition] = recordToInsert;
       }
    }
}

MATCHKEY match(const dataRecord &dr, MATCHRESULT &matchScore, bool removeAccents)
{
    // OLD APPROACH
    // Results are coded by strength of match, where
    // -1 = some type of error encountered
    //  0 = nothing matched
    //  4 = good match (likely match - four good items with no inconsistencies)
    //  5 = solid match with five good items (DOB, DOD, gender, lastName, firstName) and no inconsistencies
    //  +17 if match all three of lastName, firstName and DOB (i.e., KEY in database)
    //  netScore = score less 2 per inconsistency

    // NEW APPROACH
    // Used matchKey to determine strength of match

    GLOBALVARS gv = dr.globals;

    int i, queryPass, lastNamePass;
    unsigned int recGender, DBgender, deceasedNumber;
    bool success;
    bool execute = false;
    MATCHRECORD matchRecord;

    PQString DOBdateSQL, DODdateSQL;
    QDate recDOB, recDOD;
    QString comma(", ");
    QStringList recFirstNameList, DBfirstNameList, recMiddleNameList, DBmiddleNameList, recLastNameList, DBlastNameList;
    DATEINFO recInfoDOB, recInfoDOD, DBinfoDOB, DBinfoDOD;

    QSqlQuery query;
    QSqlError error;
    PQString errorMessage;

    // Set up the dr.data for comparisons
    recDOB = dr.getDOB();
    recInfoDOB.exactDate = recDOB;
    recInfoDOB.year = static_cast<unsigned int>(recInfoDOB.exactDate.year());
    recInfoDOB.minDate = dr.getMinDOB();
    recInfoDOB.maxDate = dr.getMaxDOB();
    recDOD = dr.getDOD();
    recInfoDOD.exactDate = recDOD;
    recInfoDOD.year = static_cast<unsigned int>(recInfoDOD.exactDate.year());

    recGender = static_cast<unsigned int>(dr.getGender());

    updateList(recLastNameList, dr.getLastName(), removeAccents);
    updateList(recLastNameList, dr.getLastNameAlt1(), removeAccents);
    updateList(recLastNameList, dr.getLastNameAlt2(), removeAccents);
    updateList(recLastNameList, dr.getLastNameAlt3(), removeAccents);

    updateList(recFirstNameList, dr.getFirstName(), removeAccents);
    updateList(recFirstNameList, dr.getFirstNameAKA1(), removeAccents);
    updateList(recFirstNameList, dr.getFirstNameAKA2(), removeAccents);
    // TODO: Database with name variations

    updateList(recMiddleNameList, dr.getMiddleNames(), removeAccents, true);


    // Strategy is to use least ambiguous available data first

    /*******************************************************/
    /*    FIRST QUERY PASS  - DOB, DOD and Last Name       */
    /*    SECOND QUERY PASS - DOB and Last Name            */
    /*    THIRD QUERY PASS  - DOD and Last Name            */
    /*    FOURTH QUERY PASS - Last and First Name Only     */
    /*******************************************************/

    for (queryPass = 1; queryPass <= 4; queryPass++)
    {
        query.clear();
        DOBdateSQL.clear();
        DODdateSQL.clear();

        // Prepare query excluding lastName
        switch (queryPass)
        {
        case 1:
            if (recDOB.isValid() && recDOD.isValid())
                execute = true;
            else
                execute = false;

            if (execute)
            {
                DOBdateSQL << recDOB.toString("yyyy/MM/dd") << QString(" 0:0:0");
                DODdateSQL << recDOD.toString("yyyy/MM/dd") << QString(" 0:0:0");

                success = query.prepare("SELECT firstName, nameAKA1, nameAKA2, middleNames, lastName, altLastName1, altLastName2, altLastName3, "
                                        "gender, deceasedNumber, DOB, DOD, minDOB, maxDOB "
                                        "FROM deceased WHERE DOB = :DOB AND DOD = :DOD AND lastName = :lastName");
                query.bindValue(":DOB", QVariant(DOBdateSQL.getString()));
                query.bindValue(":DOD", QVariant(DODdateSQL.getString()));
            }
            break;

        case 2:
            if (recDOB.isValid())
                execute = true;
            else
                execute = false;

            if (execute)
            {
                DOBdateSQL << recDOB.toString("yyyy/MM/dd") << QString(" 0:0:0");

                success = query.prepare("SELECT firstName, nameAKA1, nameAKA2, middleNames, lastName, altLastName1, altLastName2, altLastName3, "
                                        "gender, deceasedNumber, DOB, DOD, minDOB, maxDOB "
                                        "FROM deceased WHERE DOB = :DOB AND lastName = :lastName");
                query.bindValue(":DOB", QVariant(DOBdateSQL.getString()));
            }
            break;

        case 3:
            if (recDOD.isValid())
                execute = true;
            else
                execute = false;

            if (execute)
            {
                DODdateSQL << recDOD.toString("yyyy/MM/dd") << QString(" 0:0:0");

                success = query.prepare("SELECT firstName, nameAKA1, nameAKA2, middleNames, lastName, altLastName1, altLastName2, altLastName3, "
                                        "gender, deceasedNumber, DOB, DOD, minDOB, maxDOB "
                                        "FROM deceased WHERE DOD = :DOD AND lastName = :lastName");
                query.bindValue(":DOD", QVariant(DODdateSQL.getString()));
            }
            break;

        case 4:
            execute = true;
            if (execute)
            {
                success = query.prepare("SELECT firstName, nameAKA1, nameAKA2, middleNames, lastName, altLastName1, altLastName2, altLastName3, "
                                        "gender, deceasedNumber, DOB, DOD, minDOB, maxDOB "
                                        "FROM deceased WHERE lastName = :lastName AND (firstName = :firstName OR nameAKA1 = :firstName OR nameAKA2 = :firstname)");
                query.bindValue(":firstName", QVariant(recFirstNameList.at(0)));
            }
            break;
        }

        // Prepare lastName component of query
        if (execute)
        {
            for (lastNamePass = 0; lastNamePass < recLastNameList.size(); lastNamePass++)  // Loop through each possible last name in the record being queried
            {
                switch(lastNamePass)
                {
                case 0:
                    query.bindValue(":lastName", QVariant(recLastNameList.at(0)));
                    break;

                case 1:
                    query.bindValue(":lastName", QVariant(recLastNameList.at(1)));
                    break;

                case 2:
                    query.bindValue(":lastName", QVariant(recLastNameList.at(2)));
                    break;

                case 3:
                    query.bindValue(":lastName", QVariant(recLastNameList.at(3)));
                    break;
                }

                // Execute query
                success = query.exec();

                if (!success)
                {
                    error = query.lastError();

                    errorMessage << QString("SQL error trying to match record: ") << recLastNameList.at(0) << comma << recFirstNameList.at(0);
                    gv.logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
                }
                else
                {
                    // Check and rank match score for each record

                    for (i = 0; i < query.size(); i++)
                    {
                        matchRecord.clear();
                        DBfirstNameList.clear();
                        DBmiddleNameList.clear();
                        DBlastNameList.clear();
                        DBinfoDOB.clear();
                        DBinfoDOD.clear();

                        query.next();

                        // Step 1:  Read the remaining DB record data
                        updateList(DBlastNameList, PQString(query.value(4).toString()), removeAccents);
                        updateList(DBlastNameList, PQString(query.value(5).toString()), removeAccents);
                        updateList(DBlastNameList, PQString(query.value(6).toString()), removeAccents);
                        updateList(DBlastNameList, PQString(query.value(7).toString()), removeAccents);

                        updateList(DBfirstNameList, PQString(query.value(0).toString()), removeAccents);
                        updateList(DBfirstNameList, PQString(query.value(1).toString()), removeAccents);
                        updateList(DBfirstNameList, PQString(query.value(2).toString()), removeAccents);

                        updateList(DBmiddleNameList, PQString(query.value(3).toString()), removeAccents, true);

                        DBgender = query.value(8).toUInt();
                        deceasedNumber = query.value(9).toUInt();

                        DBinfoDOB.exactDate = query.value(10).toDate();
                        DBinfoDOB.year = static_cast<unsigned int>(DBinfoDOB.exactDate.year());
                        DBinfoDOB.minDate = query.value(12).toDate();
                        DBinfoDOB.maxDate = query.value(13).toDate();
                        DBinfoDOD.exactDate = query.value(11).toDate();
                        DBinfoDOD.year = static_cast<unsigned int>(recInfoDOD.exactDate.year());

                        // Step 2 - Run generic comparisons
                        matchRecord.compareLastNames(recLastNameList, DBlastNameList);
                        matchRecord.compareFirstNames(recFirstNameList, DBfirstNameList);
                        matchRecord.compareMiddleNames(recMiddleNameList, DBmiddleNameList);
                        matchRecord.compareGenders(recGender, DBgender);

                        // Step 3:  Run pass specific comparisons
                        switch(queryPass)
                        {
                        case 1:
                            matchRecord.addToScore(mDOB);
                            matchRecord.addToScore(mDOD);
                            break;

                        case 2:
                            matchRecord.addToScore(mDOB);
                            matchRecord.compareDODinfo(recInfoDOD, DBinfoDOD);
                            break;

                        case 3:
                            matchRecord.addToScore(mDOD);
                            matchRecord.compareDOBinfo(recInfoDOB, DBinfoDOB);
                            break;

                        case 4:
                            matchRecord.compareDOBinfo(recInfoDOB, DBinfoDOB);
                            matchRecord.compareDODinfo(recInfoDOD, DBinfoDOD);
                           break;

                        }

                        // Step 4 - Adjust mkKey score if one of the alternate last names was used in the query/match
                        matchRecord.analyze();
                        if ((matchRecord.matchKey == mkKey) && (lastNamePass != 0))
                            matchRecord.setMatchKey(mkAltKey);

                        // Step 5 - Store results
                        matchRecord.setID(deceasedNumber);
                        matchScore.update(matchRecord);
                    }

                    // Decide whether or not to keep going
                    if (matchScore.getBestMatch() >= mkAltKey)
                        return matchScore.getBestMatch();
                }
            }
        }
    } // end of queryPass

    return matchScore.getBestMatch();
}

void updateList(QStringList &listName, PQString string, bool removeAccents, bool isMiddleNameList)
{
    if (isMiddleNameList)
    {
        QStringList tempList;

        if (string.getLength() > 0)
        {
            if (removeAccents)
                tempList = string.getUnaccentedString().split(' ', QString::SkipEmptyParts);
            else
                tempList = string.getString().split(' ', QString::SkipEmptyParts);
        }

        if (tempList.size() > 0)
            listName.append(tempList);
    }
    else
    {
        if (string.getLength() > 0)
        {
            if (removeAccents)
                listName.append(string.getUnaccentedString());
            else
                listName.append(string.getString());
        }
    }
}

