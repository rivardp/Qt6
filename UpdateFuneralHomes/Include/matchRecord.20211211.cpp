#include "../Include/matchRecord.h"


MATCHRECORD::MATCHRECORD() :  recordID(0), score(0), netScore(0), inconsistencyCount(0), inconsistencyScore(0), rankScore(0),
                              newInfoAvailable(false), isAnalyzed(false),
                              matchedLNAME(false), matchedLNAMEALT(false), matchedDOB(false), matchedFNAME(false),
                              matchedDOD(false), matchedGENDER(false), matchedDODinfo(false), matchedDOBinfo(false),
                              matchedPREFIX(false), matchedSUFFIX(false)
{
}

void MATCHRECORD::clear()
{
    recordID = 0;
    recordLastName.clear();
    dataCount = 0;
    score = 0;
    netScore = 0;
    inconsistencyCount = 0;
    inconsistencyScore = 0;
    rankScore = 0;
    newInfoAvailable = false;
    matchedFHinfo = false;
    sufficientlyMatched = false;
    isAnalyzed = false;
    sameDataProvided = true;
    sufficientNameMatch = false;
    closeDOBrange = false;
    potentialNumDataMatches = 0;
    numNameMatches = 0;

    matchedLNAME = false;
    matchedLNAMEALT = false;
    matchedDOB = false;
    matchedFNAME = false;
    matchedFIRSTNAME = false;
    matchedDOD = false;
    matchedGENDER = false;
    matchedDODinfo = false;
    matchedDOBinfo = false;
    matchedPREFIX = false;
    matchedSUFFIX = false;
    matchedYOB = false;
    matchedYOD = false;
    matchedAGEDEATH = false;
    matchedSPOUSE = false;

    consistentDOB = false;

    inconsistentDOB = false;
    inconsistentDOD = false;
    inconsistentAgeAtDeath = false;
    inconsistentSpouseName = false;
}

void MATCHRECORD::setID(unsigned int id)
{
    recordID = id;
}

void MATCHRECORD::setLastName(QString lastName)
{
    recordLastName = lastName;
}

void MATCHRECORD::setMatchKey(MATCHKEY mk)
{
    matchKey = mk;
}

void MATCHRECORD::setSameFH(bool matched)
{
    matchedFHinfo = matched;
}

void MATCHRECORD::setSufficientlyMatched(bool sufficient)
{
    sufficientlyMatched = sufficient;
}

MATCHKEY MATCHRECORD::getMatchKey() const
{
    return matchKey;
}

bool MATCHRECORD::isNewRecord() const
{
    return newRecord;
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
    if (score & cDOB)
        result++;
    if (score & mAGEDEATH)
        result++;
    if (score & mSPOUSENAME)
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
    newRecord = true;

    inconsistencyCount = countDataItems(inconsistencyScore);
    dataCount = countDataItems(score) + inconsistencyCount;

    matchedLNAME = ((score & mLNAME) == mLNAME);
    matchedLNAMEALT = ((score & mLNAMEALT1) == mLNAMEALT1) || ((score & mLNAMEALT2) == mLNAMEALT2) || ((score & mLNAMEALT3) == mLNAMEALT3);
    matchedDOB = ((score & mDOB) == mDOB);
    matchedFNAME = ((score & mFNAME) == mFNAME);
    matchedFIRSTNAME = ((score & mFIRSTNAME) == mFIRSTNAME);
    matchedSNAME = ((score & mFNAME) == mFNAME) || ((score & mFNAMEAKA1) == mFNAMEAKA1) || ((score & mFNAMEAKA2) == mFNAMEAKA2);
    matchedDOD = ((score & mDOD) == mDOD);
    matchedGENDER = ((score & mGENDER) == mGENDER);
    matchedDODinfo = ((score & mYOD) == mYOD);
    matchedDOBinfo = ((score & mMINDOB) == mMINDOB);    // This is intentional to capture a number of types of matches
    matchedPREFIX = ((score & mPREFIX) == mPREFIX);
    matchedSUFFIX = ((score & mSUFFIX) == mSUFFIX);
    matchedAGEDEATH = ((score & mAGEDEATH) == mAGEDEATH);
    matchedYOB = ((score & mYOB) == mYOB);
    matchedYOD = ((score & mYOD) == mYOD);
    matchedSPOUSE = ((score & mSPOUSENAME) == mSPOUSENAME);

    consistentDOB = ((score & cDOB) == cDOB);

    inconsistentDOB = ((inconsistencyScore & mDOB) == mDOB) || ((inconsistencyScore & mYOB) == mYOB);
    inconsistentDOD = ((inconsistencyScore & mDOD) == mDOD) || ((inconsistencyScore & mYOD) == mYOD);
    inconsistentAgeAtDeath = ((inconsistencyScore & mAGEDEATH) == mAGEDEATH);
    inconsistentSpouseName = ((inconsistencyScore & mSPOUSENAME) == mSPOUSENAME);
    inconsistentLNAME = ((inconsistencyScore & mLNAME) == mLNAME);

    if (!inconsistentDOB && !inconsistentDOD && !inconsistentAgeAtDeath)
    {
        if (matchedDOB && matchedLNAME && matchedFNAME)
        {
            if (matchedDOD)
                matchKey = mkKeyDOD;
            else
                matchKey = mkKey;
        }
        else
        {
            if (matchedDOB && matchedLNAMEALT && matchedFNAME)
            {
                if (matchedDOD)
                    matchKey = mkAltKeyDOD;
                else
                    matchKey = mkAltKey;
            }
            else
            {
                if ((matchedDOB || consistentDOB) && matchedFIRSTNAME)
                {
                    if (matchedDOB)
                    {
                        if (matchedLNAME)
                        {
                            if (matchedDOD)
                                matchKey = mkKeyDOBDODnameAlt;
                            else
                                matchKey = mkDOBnameAlt;
                        }
                        else
                        {
                            if (matchedLNAMEALT)
                            {
                                if (matchedDOD)
                                    matchKey = mkAltKeyDOBDODnameAlt;
                                else
                                    matchKey = mkDOBaltNameAlt;
                            }
                        }
                    }
                    else
                    {
                        if (matchedLNAME)
                        {
                            if (matchedDOD)
                                matchKey = mkDODdobName;
                            else
                                matchKey = mkDODdobAltName;
                        }
                        else
                        {
                            if (matchedLNAMEALT)
                            {
                                if (matchedDOD)
                                    matchKey = mkCdobName;
                                else
                                    matchKey = mkCdobAltName;
                            }
                        }
                    }
                }
                else
                {
                    if (matchedDOD)
                    {
                        if (matchedLNAME)
                        {
                            if (matchedFNAME)
                                matchKey = mkDODname;
                            else
                            {
                                if (matchedFIRSTNAME)
                                    matchKey = mkDODnameAlt;
                            }
                        }
                        else
                        {
                            if (matchedLNAMEALT)
                            {
                                if (matchedFNAME)
                                    matchKey = mkDODaltName;
                                else
                                {
                                    if (matchedFIRSTNAME)
                                        matchKey = mkDODaltNameAlt;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (matchedLNAME)
                        {
                            if (matchedFNAME)
                                matchKey = mkNoKey;
                            else
                            {
                                if (matchedFIRSTNAME)
                                    matchKey = mkNoKeyAlt;
                            }
                        }
                        else
                        {
                            if (matchedLNAMEALT)
                            {
                                if (matchedFNAME)
                                    matchKey = mkNoKeyAltName;
                                else
                                {
                                    if (matchedFIRSTNAME)
                                        matchKey = mkNoKeyAltNameAlt;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Final handling
    if (inconsistentLNAME)
        matchKey = mkNone;

    if (matchKey >= mkDODname)
    {
        newRecord = false;
        sufficientlyMatched = true;
    }
    else
    {
        if ((matchKey >= mkDODaltNameAlt) && (inconsistencyScore == 0))
            newRecord = false;
        else
        {
            if ((matchKey >= mkDODaltName) && (inconsistencyScore == 0) && matchedAGEDEATH)
                newRecord = false;
        }

        if ((newRecord && (potentialNumDataMatches >= 6) && sameDataProvided) || matchedIDnumber)
        {
            sufficientlyMatched = true;
            newRecord = false;
        }

        if (newRecord && (matchKey >= mkNoKey) && (inconsistencyScore == 0))
        {
            if (sufficientNameMatch)
            {
                sufficientlyMatched = true;
                newRecord = false;
            }
            else
            {
                if ((numNameMatches >= 3) && closeDOBrange)
                {
                    sufficientlyMatched = true;
                    newRecord = false;
                }
            }
        }

        if (!sufficientlyMatched && (inconsistencyScore > 0))
        {
            newRecord = true;
            matchKey = mkNone;
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
        {
            addToInconsistencyScore(mGENDER);
            sameDataProvided = false;
        }
    }

    if ((recGender > 0) && (DBgender == 0))
        newInfoAvailable = true;

    if (((recGender > 0) && (DBgender == 0)) || ((recGender == 0) && (DBgender > 0)))
        sameDataProvided = false;

    if ((recGender > 0) || (DBgender >0))
        potentialNumDataMatches++;
}

void MATCHRECORD::compareLastNames(QStringList &recLastNameList, QStringList &DBlastNameList)
{
    // Positions are determined by reference to the DB record (i.e., the database key)

    bool matched;
    int numMatches = 0;
    int maxMatches;

    if (recLastNameList.size() >= DBlastNameList.size())
        maxMatches = DBlastNameList.size();
    else
        maxMatches = recLastNameList.size();

    for (int i = 0; i < recLastNameList.size(); i++)
    {
        matched = false;

        for (int j = 0; j < DBlastNameList.size(); j++)
        {
            if (DBlastNameList[j] == recLastNameList[i])
            {
                matched = true;
                numMatches++;

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

    if (DBlastNameList.size() > recLastNameList.size())
        potentialNumDataMatches += DBlastNameList.size();
    else
        potentialNumDataMatches += recLastNameList.size();

    if ((DBlastNameList.size() != recLastNameList.size()) || (numMatches != DBlastNameList.size()))
        sameDataProvided = false;

    if ((DBlastNameList.size() > numMatches) && (recLastNameList.size() > numMatches))
        addToInconsistencyScore(mLNAME);
    else
        numNameMatches += numMatches;

    if ((numMatches >= 2) && (numMatches == maxMatches))
        sufficientNameMatch = true;    
}

void MATCHRECORD::compareFirstNames(QStringList &recFirstNameList, QStringList &DBfirstNameList)
{
    // Positions are determined by reference to the DB record (i.e., the database key)
    // TODO:  More sophisticated comparison grouping nicknames with their formal names, along with counting of non-matches

    bool matched;
    int numMatches = 0;
    OQString tempName;
    PQString errorMessage;

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
                    numMatches++;
                }
            }
            else
            {
                if (DBfirstNameList[j] == recFirstNameList[i])
                {
                    addToScore(mFNAME);
                    matched = true;
                    numMatches++;
                }
                else
                {
                    tempName = DBfirstNameList[j];
                    if (tempName.isFormalVersionOf(recFirstNameList[i], errorMessage) || tempName.isInformalVersionOf(recFirstNameList[i], errorMessage))
                        addToScore(mFIRSTNAME);
                }
            }
        }

        if (!matched)
            newInfoAvailable = true;
    }

    if (DBfirstNameList.size() > recFirstNameList.size())
        potentialNumDataMatches += DBfirstNameList.size();
    else
        potentialNumDataMatches += recFirstNameList.size();

    if ((DBfirstNameList.size() != recFirstNameList.size()) || (DBfirstNameList.size() != numMatches))
        sameDataProvided = false;

    sufficientNameMatch = sufficientNameMatch && (numMatches >= 1);

    if (numMatches >= 1)
        numNameMatches += 1;
}

void MATCHRECORD::compareMiddleNames(QStringList &recMiddleNameList, QStringList &DBmiddleNameList)
{
    // For consistency, positions are determined by reference to the DB record

    bool matched;
    int numMatches = 0;

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
                    numMatches++;
                }
            }
            else
            {
                if (DBmiddleNameList[j] == recMiddleNameList[i])
                {
                    addToScore(mMIDDLENAMES);
                    matched = true;
                    numMatches++;
                }
            }
        }

        if (!matched)
            newInfoAvailable = true;
    }

    if (DBmiddleNameList.size() > recMiddleNameList.size())
        potentialNumDataMatches += DBmiddleNameList.size();
    else
        potentialNumDataMatches += recMiddleNameList.size();

    if ((DBmiddleNameList.size() != recMiddleNameList.size()) || (DBmiddleNameList.size() != numMatches))
        sameDataProvided = false;

    if ((DBmiddleNameList.size() > numMatches) && (recMiddleNameList.size() > numMatches))
        addToInconsistencyScore(mMIDDLENAMES);
    else
        numNameMatches += numMatches;
}

void MATCHRECORD::compareDOBinfo(DATEINFO &recDateInfo, DATEINFO &DBdateInfo)
{
    double DBdateRange, recDateRange;

    // Check for match
    if (recDateInfo.exactDate.isValid() && DBdateInfo.exactDate.isValid())
    {
        if (recDateInfo.exactDate == DBdateInfo.exactDate)
            addToScore(mDOB);
        else
        {
            addToInconsistencyScore(mDOB);
            sameDataProvided = false;
        }
    }
    else
    {
        if ((DBdateInfo.year > 0) && (recDateInfo.year > 0))
        {
            if (DBdateInfo.year == recDateInfo.year)
                addToScore(mYOB);
            else
            {
                addToInconsistencyScore(mYOB);
                sameDataProvided = false;
            }
        }

        if (DBdateInfo.minDate.isValid() && DBdateInfo.maxDate.isValid() && (DBdateInfo.minDate > QDate(1875,1,1)))
            DBdateRange = elapse(DBdateInfo.minDate, DBdateInfo.maxDate);
        else
            DBdateRange = 99;

        if (recDateInfo.minDate.isValid() && recDateInfo.maxDate.isValid() && (recDateInfo.minDate > QDate(1900,1,1)))
            recDateRange = elapse(recDateInfo.minDate, recDateInfo.maxDate);
        else
            recDateRange = 99;

        if (recDateInfo.exactDate.isValid() && !DBdateInfo.exactDate.isValid())
        {
            if (DBdateRange < 2.0)
            {
                if ((recDateInfo.exactDate >= DBdateInfo.minDate) && (recDateInfo.exactDate <= DBdateInfo.maxDate))
                {
                    addToScore(cDOB);
                    newInfoAvailable = true;
                }
                else
                {
                    addToInconsistencyScore(mDOB);
                    sameDataProvided = false;
                }
            }
            else
                newInfoAvailable = true;
        }
        else
        {
            if (!recDateInfo.exactDate.isValid() && (DBdateInfo.exactDate.isValid() && (DBdateInfo.exactDate > QDate(1875,1,1))))
            {
                if (recDateRange < 2.0)
                {
                    if ((DBdateInfo.exactDate >= recDateInfo.minDate) && (DBdateInfo.exactDate <= recDateInfo.maxDate))
                        addToScore(cDOB);
                    else
                    {
                        addToInconsistencyScore(mDOB);
                        sameDataProvided = false;
                    }
                }
            }
            else
            {
                if ((DBdateRange < 2.0) && (recDateRange < 2.0))
                {
                    if ((recDateInfo.minDate >= DBdateInfo.minDate) && (recDateInfo.maxDate <= DBdateInfo.maxDate))
                    {
                        addToScore(mMINDOB);
                        addToScore(mMAXDOB);
                    }
                    else
                    {
                        if (((recDateInfo.minDate == DBdateInfo.minDate) && (DBdateRange <= 1) && (recDateRange <= 1)) ||
                            ((recDateInfo.maxDate == DBdateInfo.maxDate) && (DBdateRange <= 1) && (recDateRange <= 1)))
                        {
                            addToScore(mMINDOB);
                            addToScore(mMAXDOB);
                        }
                        else
                        {
                            addToInconsistencyScore(mMINDOB);
                            addToInconsistencyScore(mMAXDOB);
                            sameDataProvided = false;
                        }
                    }
                }
                else
                {
                    if (recDateRange < 2.0)
                    {
                        newInfoAvailable = true;
                    }
                }
            }
        }
    }

    if ((recDateInfo.minDate > DBdateInfo.maxDate) || (recDateInfo.maxDate < DBdateInfo.minDate))
    {
        addToInconsistencyScore(mDOB);
        addToInconsistencyScore(mMINDOB);
        addToInconsistencyScore(mMAXDOB);
        sameDataProvided = false;
    }
    else
    {
        if ((DBdateInfo.maxDate.year() - recDateInfo.minDate.year()) <= 1)
            closeDOBrange = true;
    }

    if (recDateInfo.exactDate.isValid() || DBdateInfo.exactDate.isValid())
        potentialNumDataMatches++;
    else
    {
        if ((recDateInfo.year > 0) || (DBdateInfo.year > 0))
            potentialNumDataMatches++;
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
        {
            addToInconsistencyScore(mDOD);
            sameDataProvided = false;
        }
    }
    else
    {
        if ((recDateInfo.year > 0) && (DBdateInfo.year > 0))
        {
            if (recDateInfo.year == DBdateInfo.year)
                addToScore(mYOD);
            else
            {
                addToInconsistencyScore(mYOD);
                sameDataProvided = false;
            }
        }
        else
        {
            if ((recDateInfo.exactDate != DBdateInfo.exactDate) || (recDateInfo.year != DBdateInfo.year))
                sameDataProvided = false;
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

    if (recDateInfo.exactDate.isValid() || DBdateInfo.exactDate.isValid())
        potentialNumDataMatches++;
    else
    {
        if ((recDateInfo.year > 0) || (DBdateInfo.year > 0))
            potentialNumDataMatches++;
    }
}

void MATCHRECORD::compareDOBDODinfo(DATEINFO &recDOBInfo, DATEINFO &DBDOBInfo, DATEINFO &recDODInfo, DATEINFO &DBDODInfo)
{
    // Catch inconsistency where maxDOB can be interpreted as maxDOD
    if (recDODInfo.exactDate.isValid() && !DBDODInfo.exactDate.isValid() && (!DBDOBInfo.exactDate.isValid() || (DBDOBInfo.exactDate == QDate(1,1,1))) && (DBDOBInfo.minDate == QDate(1875,1,1)))
    {
        if (recDODInfo.exactDate > DBDOBInfo.maxDate)
        {
            sameDataProvided = false;
            addToInconsistencyScore(mDOD);
        }
    }

    if (DBDODInfo.exactDate.isValid() && !recDODInfo.exactDate.isValid() && (!recDOBInfo.exactDate.isValid() || (recDOBInfo.exactDate == QDate(1,1,1))) && (recDOBInfo.minDate == QDate(1875,1,1)))
    {
        if (DBDODInfo.exactDate > recDOBInfo.maxDate)
        {
            sameDataProvided = false;
            addToInconsistencyScore(mDOD);
        }
    }
}

void MATCHRECORD::compareSpouseName(QString recSpouseName, QString DBspouseName)
{
    if ((recSpouseName.length() == 0) || (DBspouseName.length() == 0))
        return;

    if (recSpouseName.toLower() == DBspouseName.toLower())
        addToScore(mSPOUSENAME);
    else
    {
        OQString name(recSpouseName);
        PQString errMsg;
        if (name.isFormalVersionOf(DBspouseName, errMsg) || name.isInformalVersionOf(DBspouseName, errMsg))
            addToScore(mSPOUSENAME);
        else
            addToInconsistencyScore(mSPOUSENAME);
    }
}

MATCHRESULT::MATCHRESULT() : bestScore(0), bestNetScore(0), bestID(0), bestNetID(0), bestMatchKeyID(0)
{
    matchRecordList.clear();
    bestLastName.clear();
    bestMatchKey = mkNone;
}

void MATCHRESULT::clear()
{
    matchRecordList.clear();
    bestLastName.clear();
    bestMatchKey = mkNone;
    bestScore = 0;
    bestNetScore = 0;
    bestRankScore = 0;
    bestID = 0;
    bestNetID = 0;
    bestMatchKeyID = 0;
}

void MATCHRESULT::update(unsigned int id, int score, int numInconsistency, int scoreInconsistency, bool newInfo, QString lastName)
{
    if (this->alreadyContains(id))
    {
        // Update criteria based on net score
        if ((score - inconsistencyCost * numInconsistency) > getNetScore(id))
            setScore(id, score, numInconsistency, scoreInconsistency, newInfo, lastName);
        else;
            // Leave current scores
    }
    else
        addScore(id, score, numInconsistency, scoreInconsistency, newInfo, lastName);

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

int MATCHRESULT::getBestRankScore() const
{
    return bestRankScore;
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

QString MATCHRESULT::getBestLastName() const
{
    return bestLastName;
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

QString MATCHRESULT::getLastName(unsigned int id) const
{
    QString result;
    int i = 0;
    bool matched = false;

    while (!matched && (i < matchRecordList.size()))
    {
        if (id == matchRecordList.at(i).recordID)
        {
            matched = true;
            result = matchRecordList.at(i).recordLastName;
        }
        i++;
    }

    return result;
}

void MATCHRESULT::setScore(unsigned int id, int score, int numInconsistency, int scoreInconsistency, bool newInfo, QString lastName)
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
            matchRecordList[i].recordLastName = lastName;

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
                bestLastName = mr.recordLastName;
            }
        }

        i++;
    }
}

void MATCHRESULT::addScore(unsigned int id, int score, int numInconsistency, int scoreInconsistency, bool newInfo, QString lastName)
{
    int netScore = score - inconsistencyCost * numInconsistency;

    MATCHRECORD newRecord;
    newRecord.recordID = id;
    newRecord.score = score;
    newRecord.netScore = netScore;
    newRecord.inconsistencyCount = static_cast<unsigned int>(numInconsistency);
    newRecord.inconsistencyScore = scoreInconsistency;
    newRecord.newInfoAvailable = newInfo;
    newRecord.recordLastName = lastName;

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
        bestLastName = mr.recordLastName;
    }
}

void MATCHRESULT::addRankScore(MATCHRECORD &mr)
{
    matchRecordList.append(mr);

    if (mr.rankScore > bestRankScore)
    {
        bestRankScore = mr.rankScore;
        bestID = mr.recordID;
        bestLastName = mr.recordLastName;
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

bool MATCHRESULT::isSufficientlyClose(unsigned int id) const
{
    bool result = false;
    int i = 0;
    bool matched = false;

    while (!matched && (i < matchRecordList.size()))
    {
        if (id == matchRecordList.at(i).recordID)
        {
            matched = true;
            result = matchRecordList.at(i).sufficientlyMatched;
        }
        i++;
    }

    return result;
}

bool MATCHRESULT::fromSameFH(unsigned int id) const
{
    bool result = false;
    int i = 0;
    bool matched = false;

    while (!matched && (i < matchRecordList.size()))
    {
        if (id == matchRecordList.at(i).recordID)
        {
            matched = true;
            result = matchRecordList.at(i).matchedFHinfo;
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
    // Used matchKey to determine strength of match

    GLOBALVARS gv = dr.globals;

    int i, queryPassStart, queryPassEnd, queryPass, lastNamePass;
    unsigned int recGender, DBgender, deceasedNumber, recAgeAtDeath, DBageAtDeath;
    bool success;
    bool execute = false;
    bool matchedID = false;
    MATCHRECORD matchRecord;

    PQString DOBdateSQL, DODdateSQL;
    QDate recDOB, recDOD;
    QString comma(", ");
    QString passLastName;
    QStringList recFirstNameList, DBfirstNameList, recMiddleNameList, DBmiddleNameList, recLastNameList, DBlastNameList;
    DATEINFO recInfoDOB, recInfoDOD, DBinfoDOB, DBinfoDOD;

    QSqlQuery query;
    QSqlError error;
    PQString errorMessage;

    // Setup all the provider identification information
    unsigned int recProviderID, recProviderKey;
    QString recID, DBid;
    recProviderID = static_cast<unsigned int>(dr.getProvider());
    recProviderKey = dr.getProviderKey();
    recID = dr.getID().getString();

    // Before comparing data, check if a perfect ID match exists
    success = query.prepare("SELECT deceasedNumber FROM death_audits.deceasedidinfo "
                            "WHERE providerID = :providerID AND providerKey = :providerKey AND ID = :ID");
    query.bindValue(":providerID", QVariant(recProviderID));
    query.bindValue(":providerKey", QVariant(recProviderKey));
    query.bindValue(":ID", QVariant(recID));

    success = query.exec();
    if (success && (query.size() > 0))
    {
        // Only a single match will be attempted
        query.next();
        deceasedNumber = query.value(0).toUInt();
        matchedID = true;
        queryPassStart = 0;
        queryPassEnd = 0;
    }
    else
    {
        // Up to four queries will be run
        deceasedNumber = 0;
        queryPassStart = 1;
        queryPassEnd = 4;
    }

    // Set up the dr.data for comparisons
    recDOB = dr.getDOB();
    recInfoDOB.exactDate = recDOB;
    recInfoDOB.year = dr.getYOB();
    recInfoDOB.minDate = dr.getMinDOB();
    recInfoDOB.maxDate = dr.getMaxDOB();
    recDOD = dr.getDOD();
    recInfoDOD.exactDate = recDOD;
    recInfoDOD.year = dr.getYOD();

    recGender = static_cast<unsigned int>(dr.getGender());
    recAgeAtDeath = dr.getAgeAtDeath();

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
    /*    ZERO QUERY PASS   - ID                           */
    /*    FIRST QUERY PASS  - DOB, DOD and Last Name       */
    /*    SECOND QUERY PASS - DOB and Last Name            */
    /*    THIRD QUERY PASS  - DOD and Last Name            */
    /*    FOURTH QUERY PASS - Last and First Name Only     */
    /*******************************************************/

    for (queryPass = queryPassStart; queryPass <= queryPassEnd; queryPass++)
    {
        query.clear();
        DOBdateSQL.clear();
        DODdateSQL.clear();

        // Prepare query excluding lastName
        switch (queryPass)
        {
        case 0:
            success = query.prepare("SELECT firstName, nameAKA1, nameAKA2, middleNames, lastName, altLastName1, altLastName2, altLastName3, "
                                    "gender, deceasedNumber, DOB, DOD, minDOB, maxDOB, YOB, YOD, ageAtDeath "
                                    "FROM death_audits.deceased "
                                    "WHERE (deceasedNumber = :deceasedNumber)");
            query.bindValue(":deceasedNumber", QVariant(deceasedNumber));

            execute = true;
            break;

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
                                        "gender, deceasedNumber, DOB, DOD, minDOB, maxDOB, YOB, YOD, ageAtDeath "
                                        "FROM death_audits.deceased WHERE lastName = :lastName AND DOB = :DOB AND DOD = :DOD");
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
                                        "gender, deceasedNumber, DOB, DOD, minDOB, maxDOB, YOB, YOD, ageAtDeath "
                                        "FROM death_audits.deceased WHERE lastName = :lastName AND DOB = :DOB");
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
                                        "gender, deceasedNumber, DOB, DOD, minDOB, maxDOB, YOB, YOD, ageAtDeath "
                                        "FROM death_audits.deceased WHERE lastName = :lastName AND DOD = :DOD");
                query.bindValue(":DOD", QVariant(DODdateSQL.getString()));
            }
            break;

        case 4:
            execute = true;
            if (execute)
            {
                success = query.prepare("SELECT firstName, nameAKA1, nameAKA2, middleNames, lastName, altLastName1, altLastName2, altLastName3, "
                                        "gender, deceasedNumber, DOB, DOD, minDOB, maxDOB, YOB, YOD, ageAtDeath "
                                        "FROM death_audits.deceased WHERE lastName = :lastName AND (firstName = :firstName OR nameAKA1 = :firstName OR nameAKA2 = :firstname)");
                query.bindValue(":firstName", QVariant(recFirstNameList.at(0)));
            }
            break;
        }

        // Prepare lastName component of query
        if (execute)
        {
            for (lastNamePass = 0; lastNamePass < recLastNameList.size(); lastNamePass++)  // Loop through each possible last name in the record being queried
            {
                passLastName = recLastNameList.at(lastNamePass);
                query.bindValue(":lastName", QVariant(passLastName));

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
                        DBinfoDOB.year = query.value(14).toUInt();
                        DBinfoDOB.minDate = query.value(12).toDate();
                        DBinfoDOB.maxDate = query.value(13).toDate();
                        DBinfoDOD.exactDate = query.value(11).toDate();
                        DBinfoDOD.year = query.value(15).toUInt();

                        DBageAtDeath = query.value(16).toUInt();

                        // Step 2 - Run generic comparisons
                        matchRecord.compareLastNames(recLastNameList, DBlastNameList);
                        matchRecord.compareFirstNames(recFirstNameList, DBfirstNameList);
                        matchRecord.compareMiddleNames(recMiddleNameList, DBmiddleNameList);
                        matchRecord.compareGenders(recGender, DBgender);
                        matchRecord.compareAgeAtDeath(recAgeAtDeath, DBageAtDeath);
                        //matchRecord.compareIDs(recID, DBid, DBaltID, DBaltID2);

                        // Step 3:  Run pass specific comparisons
                        switch(queryPass)
                        {
                        case 0:
                            matchRecord.compareDOBinfo(recInfoDOB, DBinfoDOB);
                            matchRecord.compareDODinfo(recInfoDOD, DBinfoDOD);
                            matchRecord.compareDOBDODinfo(recInfoDOB, DBinfoDOB, recInfoDOD, DBinfoDOD);
                            break;

                        case 1:
                            matchRecord.addToScore(mDOB);
                            matchRecord.addToScore(mDOD);
                            matchRecord.addToScore(mYOB);
                            matchRecord.addToScore(mYOD);
                            matchRecord.potentialNumDataMatches += 2;
                            break;

                        case 2:
                            matchRecord.addToScore(mDOB);
                            matchRecord.addToScore(mYOB);
                            matchRecord.potentialNumDataMatches += 1;
                            matchRecord.compareDODinfo(recInfoDOD, DBinfoDOD);
                            break;

                        case 3:
                            matchRecord.addToScore(mDOD);
                            matchRecord.addToScore(mYOD);
                            matchRecord.potentialNumDataMatches += 1;
                            matchRecord.compareDOBinfo(recInfoDOB, DBinfoDOB);
                            break;

                        case 4:
                            matchRecord.compareDOBinfo(recInfoDOB, DBinfoDOB);
                            matchRecord.compareDODinfo(recInfoDOD, DBinfoDOD);
                            matchRecord.compareDOBDODinfo(recInfoDOB, DBinfoDOB, recInfoDOD, DBinfoDOD);
                           break;

                        }

                        // Step 4 - Adjust mkKey score if one of the alternate last names was used in the query/match
                        if (matchedID)
                            matchRecord.setMatchKey(mkID);
                        else
                        {
                            matchRecord.analyze();
                            if ((matchRecord.getMatchKey() == mkKey) && (lastNamePass != 0))
                                matchRecord.setMatchKey(mkAltKey);
                        }

                        // Step 5 - Allow for likely matches in certain conditions to avoid recent obits from reappearing
                        if (!matchRecord.isNewRecord() && (matchRecord.getMatchKey() >= mkDODname))
                        {
                            if (dr.globals.today < dr.getDOD().addDays(60))
                                matchRecord.setSufficientlyMatched(true);
                        }

                        // Step 6 - Store results
                        matchRecord.setID(deceasedNumber);
                        matchRecord.setLastName(passLastName);
                        matchRecord.setSameFH(matchedID);
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

int rank(const dataRecord &dr, dataRecord &bestMatch, MATCHRESULT &matchScore)
{
    // matchKey result is only a secondary outcome

    GLOBALVARS gv = dr.globals;
    int i, rankingScore, addPoints, numPotential;
    bool success;
    MATCHRECORD matchRecord;
    databaseSearches dbSearch;

    PQString minDOBdateSQL, maxDOBdateSQL, noDOBdateSQL;
    QDate minDOBthreshold, maxDOBthreshold, deceasedDOB, tempDate, monitoredDOB, monitoredMinDOB, monitoredMaxDOB, monitoredDOD;
    QList<QString> monitoredFirstNames, monitoredMiddleNames, deceasedFirstNames, deceasedMiddleNames;
    QList<QString> expandedDeceasedFirstNames, expandedMonitoredFirstNames;
    QString spouseFirstName;
    OQStream middlenames;
    PQString word;
    GENDER monitoredGender, deceasedGender;
    unsigned int monitoredID;
    QDate missingDOB(1,1,1);
    bool DOBavailable, countMultiple;
    double numMatches;

    QSqlQuery query;
    QSqlError error;
    PQString errorMessage;

    deceasedDOB = missingDOB;
    minDOBthreshold = QDate(1875,1,1);
    maxDOBthreshold = QDate(2000,1,1);

    deceasedGender = dr.getGender();
    tempDate = dr.getDOB();
    if (tempDate.isValid() && (tempDate != missingDOB))
    {
        deceasedDOB = tempDate;
        if ((deceasedDOB.month() == 2) && (deceasedDOB.day() == 29))
        {
            minDOBthreshold = QDate(deceasedDOB.year() + 1, 3, 1);
            maxDOBthreshold = QDate(deceasedDOB.year() - 1, 2, 28);
        }
        else
        {
            minDOBthreshold = QDate(deceasedDOB.year() + 1, deceasedDOB.month(), deceasedDOB.day());
            maxDOBthreshold = QDate(deceasedDOB.year() - 1, deceasedDOB.month(), deceasedDOB.day());
        }
    }
    minDOBdateSQL << minDOBthreshold.toString("yyyy/MM/dd") << QString(" 0:0:0");
    maxDOBdateSQL << maxDOBthreshold.toString("yyyy/MM/dd") << QString(" 0:0:0");
    noDOBdateSQL << missingDOB.toString("yyyy/MM/dd") << QString(" 0:0:0");

    /*success = query.prepare("SELECT firstName, middleNames, gender, DOB, memberID "
                            "FROM monitored "
                            "WHERE lastName = :lastName AND "
                            "((DOB >= :minDOB AND DOB <= :maxDOB) OR (DOB = :noDOB))");*/
    success = query.prepare("SELECT firstName, nameAKA1, nameAKA2, middleNames, gender, DOB, minDOB, maxDOB, DOD, deceasedNumber "
                            "FROM death_audits.deceased "
                            "WHERE lastName = :lastName AND (:minDOB >= minDOB) AND (:maxDOB <= maxDOB)");
    query.bindValue(":lastName", QVariant(dr.getLastName().getString()));
    query.bindValue(":minDOB", QVariant(minDOBdateSQL.getString()));
    query.bindValue(":maxDOB", QVariant(maxDOBdateSQL.getString()));
    //query.bindValue(":noDOB", QVariant(noDOBdateSQL.getString()));

    success = query.exec();

    if (!success)
    {
        error = query.lastError();

        errorMessage << QString("SQL error trying to match monitored record: ") << dr.getLastName() << QString(" - ") << dr.getID();
        gv.logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }
    else
    {
        // Check and score ranking for each record

        // Initial one-time setup
        deceasedFirstNames.clear();
        deceasedMiddleNames.clear();
        dr.createFirstNameList(deceasedFirstNames);
        dr.createMiddleNameList(deceasedMiddleNames);

        // Repeated logic for each monitored record pulled
        numPotential = query.size();
        for (i = 0; i < numPotential; i++)
        {
            query.next();
            matchRecord.clear();
            monitoredFirstNames.clear();
            monitoredMiddleNames.clear();
            expandedMonitoredFirstNames.clear();
            middlenames.clear();
            rankingScore = 0;

            // Pull out the monitored record details
            monitoredFirstNames.append(PQString(query.value(0).toString()).getUnaccentedString());
            monitoredFirstNames.append(PQString(query.value(1).toString()).getUnaccentedString());
            monitoredFirstNames.append(PQString(query.value(2).toString()).getUnaccentedString());
            middlenames = PQString(query.value(3).toString()).getUnaccentedString();
            while (!middlenames.isEOS())
            {
                word = middlenames.getWord();
                word.cleanUpEnds();
                if (word.getLength() > 0)
                    monitoredMiddleNames.append(word.getString());
            }
            monitoredGender = static_cast<GENDER>(query.value(4).toInt());
            monitoredDOB = query.value(5).toDate();
            monitoredMinDOB = query.value(6).toDate();
            monitoredMaxDOB = query.value(7).toDate();
            monitoredDOD = query.value(8).toDate();
            monitoredID = query.value(9).toUInt();

            // Assess DOB score
            DOBavailable = deceasedDOB.isValid() && (deceasedDOB != missingDOB);
            if (DOBavailable)
            {
                if (deceasedDOB == monitoredMinDOB)
                    rankingScore += 500;
                if (deceasedDOB == monitoredMaxDOB)
                    rankingScore += 500;
                if (deceasedDOB == minDOBthreshold)
                    rankingScore += 250;
                if (deceasedDOB == maxDOBthreshold)
                    rankingScore += 250;

                if (rankingScore < 1000)
                {
                    long long rangeOfError = monitoredMaxDOB.toJulianDay() - monitoredMinDOB.toJulianDay();

                    if (deceasedDOB > monitoredMaxDOB)
                        rangeOfError += deceasedDOB.toJulianDay() - monitoredMaxDOB.toJulianDay();

                    if (deceasedDOB < monitoredMinDOB)
                        rangeOfError +=  - monitoredMinDOB.toJulianDay() - deceasedDOB.toJulianDay();

                    if (rangeOfError == 0)
                        rankingScore += 1000;
                    else
                    {

                        if (rangeOfError <= 31)
                            rankingScore += 500;
                        else
                        {
                            if (rangeOfError <=366)
                                rankingScore += 250;
                            else
                            {
                                if (rangeOfError <= 1000)
                                    rankingScore += 100;
                            }
                        }
                    }
                }
            }

            // Score gender match
            if ((deceasedGender != genderUnknown) && (monitoredGender != genderUnknown))
            {
                if (deceasedGender == monitoredGender)
                    rankingScore += 100;
                else
                    rankingScore += -50;
            }

            // Score first name match
            countMultiple = false;
            numMatches = compareNames(deceasedFirstNames, monitoredFirstNames, countMultiple);
            if (numMatches < 1)
            {
                if (expandedDeceasedFirstNames.size() == 0)
                    addNameVariations(deceasedFirstNames, expandedDeceasedFirstNames);
                addNameVariations(monitoredFirstNames, expandedMonitoredFirstNames);
                numMatches = 0.75 * compareNames(expandedDeceasedFirstNames, expandedMonitoredFirstNames, countMultiple);
            }
            addPoints = static_cast<int>(numMatches * 750);
            if (addPoints > 0)
                rankingScore += addPoints;
            else
            {
                // Look at middle name used a first name
                numMatches = 0.33 * compareNames(expandedDeceasedFirstNames, monitoredMiddleNames, countMultiple);
                addPoints = static_cast<int>(numMatches * 750);
                rankingScore += addPoints;
            }

            // Score middle name match
            countMultiple = true;
            numMatches = compareNames(deceasedMiddleNames, monitoredMiddleNames, countMultiple);
            addPoints = static_cast<int>(numMatches * 375);
            rankingScore += addPoints;

            // Store results
            matchRecord.setID(monitoredID);
            matchRecord.setLastName(dr.getLastName().getString());
            matchRecord.setRankScore(rankingScore);
            matchScore.addRankScore(matchRecord);
        }
    }

    if (matchScore.getBestRankScore() > 0)
        bestMatch = dbSearch.readMonitoredRecord(matchScore.getBestID(), matchScore.getBestLastName(), &gv);

    return matchScore.getBestRankScore();
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

void MATCHRECORD::compareAgeAtDeath(unsigned int recDateAge, unsigned int DBdateAge)
{
    if ((recDateAge > 0) && (DBdateAge > 0))
    {
        if (recDateAge == DBdateAge)
            addToScore(mAGEDEATH);
        else
            addToInconsistencyScore(mAGEDEATH);
    }

    if ((recDateAge > 0) || (DBdateAge > 0))
    {
        potentialNumDataMatches++;
        if (recDateAge != DBdateAge)
            sameDataProvided = false;
    }
}

void MATCHRECORD::compareIDs(QString recID, QString DBid, QString DBaltID, QString DBaltID2)
{
    unsigned int recIDnum, DBidNum, DBaltIDnum, DBaltID2num;

    recIDnum = recID.toUInt();
    if (recIDnum > 0)
    {
        DBidNum = DBid.toUInt();
        DBaltIDnum = DBaltID.toUInt();
        DBaltID2num = DBaltID2.toUInt();

        if ((recIDnum == DBidNum) || (recIDnum == DBaltIDnum) || (recIDnum == DBaltID2num))
            matchedIDnumber = true;
    }
}

void MATCHRECORD::setRankScore(int score)
{
    rankScore = score;
}

void MATCHRECORD::analyzeRank()
{

}


double compareNames(QList<QString> &list1, QList<QString> &list2, bool countMultiple)
{
    bool matched = false;
    bool keepGoing = true;
    double result = 0;
    int i,j;
    int minMatches = 0;
    int actualMatches = 0;

    if (countMultiple)
    {
        if (list1.size() > list2.size())
            minMatches = list2.size();
        else
            minMatches = list1.size();
    }

    i = 0;
    while (keepGoing && (i < list1.size()))
    {
        j = 0;
        while (keepGoing && (j < list2.size()))
        {
            if ((list1.at(i).size() == 1) || (list2.at(j).size() == 1))
            {
                matched = (list1.at(i).left(1).compare(list2.at(j).left(1), Qt::CaseInsensitive) == 0);
                if (matched)
                {
                    result += 0.5;
                    actualMatches++;
                    if (list1.at(i).size() == 1)
                        j = list2.size();
                }
            }
            else
            {
                matched = (list1.at(i).compare(list2.at(j), Qt::CaseInsensitive) == 0);
                if (matched)
                {
                    result += 1;
                    actualMatches++;
                    if (!countMultiple)
                        keepGoing = false;
                    else
                        j = list2.size();
                }
            }
            j++;
        }
        i++;
    }

    if (!countMultiple && (result > 1))
        result = 1;

    if (countMultiple && (actualMatches < minMatches))
        result = -minMatches;

    return result;
}

void addNameVariations(QList<QString> &nameList, QList<QString> &expandedNameList)
{
   QString name, newName;

    QSqlQuery query;
    bool success;
    QStringList listOfNames;
    QString unparsedList;
    int i, startingSize;

    i = 0;
    startingSize = nameList.size();

    while (i < startingSize)
    {
        name = nameList.at(i);
        expandedNameList.append(name);

        // Add in nicknames first
        success = query.prepare("SELECT altNames FROM firstnames WHERE name = :name");
        query.bindValue(":name", QVariant(name));
        success = query.exec();
        if (success && (query.size() == 1))
        {
            query.next();
            unparsedList = query.value(0).toString();
            listOfNames = unparsedList.split(" ", QString::SkipEmptyParts);
            while (listOfNames.size() > 0)
            {
                newName = listOfNames.takeFirst();
                if (newName.size() > 0)
                    expandedNameList.append(PQString(newName).getUnaccentedString());
            }
        }
        query.clear();

        // Add in formal names
        success = query.prepare("SELECT formalNames FROM nicknames WHERE nickname = :name");
        query.bindValue(":name", QVariant(name));
        success = query.exec();
        if (success && (query.size() == 1))
        {
            query.next();
            unparsedList = query.value(0).toString();
            listOfNames = unparsedList.split(" ", QString::SkipEmptyParts);
            while (listOfNames.size() > 0)
            {
                newName = listOfNames.takeFirst();
                if (newName.size() > 0)
                    expandedNameList.append(PQString(newName).getUnaccentedString());
            }
        }
        query.clear();

        i++;
    }

    // Sort names
    bool ascending = true;
    bool caseSensitive = false;
    bool removeDuplicates = true;
    sort(expandedNameList, ascending, caseSensitive, removeDuplicates);

    return;
}

