#include "../Include/matchRecord.h"

MATCHRESULT::MATCHRESULT() : bestScore(0), bestNetScore(0), bestID(0), bestNetID(0)
{

}

MATCHRESULT::~MATCHRESULT()
{

}

void MATCHRESULT::clear()
{
    matchRecord.clear();
    bestScore = 0;
    bestNetScore = 0;
    bestID = 0;
    bestNetID = 0;
}

void MATCHRESULT::update(unsigned int id, int score, int numInconsistency, bool newInfo)
{
    if (this->alreadyContains(id))
    {
        // Update criteria based on net score
        if ((score - inconsistencyCost * numInconsistency) > getNetScore(id))
            setScore(id, score, numInconsistency, newInfo);
        else;
            // Leave current scores
    }
    else
        addScore(id, score, numInconsistency, newInfo);

}

int MATCHRESULT::getBestScore() const
{
    return bestScore;
}

int MATCHRESULT::getBestNetScore() const
{
    return bestNetScore;
}

unsigned int MATCHRESULT::getBestID() const
{
    return bestID;
}

unsigned int MATCHRESULT::getBestNetID() const
{
    return bestNetID;
}

int MATCHRESULT::getScore(unsigned int id) const
{
    int result = -1;
    int i = 0;
    bool matched = false;

    while (!matched && (i < matchRecord.size()))
    {
        if (id == matchRecord.at(i).recordID)
        {
            matched = true;
            result = matchRecord.at(i).score;
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

    while (!matched && (i < matchRecord.size()))
    {
        if (id == matchRecord.at(i).recordID)
        {
            matched = true;
            result = matchRecord.at(i).netScore;
        }
        i++;
    }

    return result;
}

void MATCHRESULT::setScore(unsigned int id, int score, int numInconsistency, bool newInfo)
{
    // Nothing is updated if ID is not matched

    int i = 0;
    bool matched = false;
    int netScore;

    while (!matched && (i < matchRecord.size()))
    {
        if (id == matchRecord.at(i).recordID)
        {
            matched = true;
            matchRecord[i].score = score;
            matchRecord[i].netScore = netScore = score - inconsistencyCost * numInconsistency;
            matchRecord[i].inconsistencyCount = numInconsistency;
            matchRecord[i].newInfoAvailable = newInfo;

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

void MATCHRESULT::addScore(unsigned int id, int score, int numInconsistency, bool newInfo)
{
    int netScore = score - inconsistencyCost * numInconsistency;

    MATCHRECORD newRecord;
    newRecord.recordID = id;
    newRecord.score = score;
    newRecord.netScore = netScore;
    newRecord.inconsistencyCount = numInconsistency;
    newRecord.newInfoAvailable = newInfo;

    matchRecord.append(newRecord);

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

bool MATCHRESULT::alreadyContains(unsigned int id) const
{
    int i = 0;
    bool matched = false;

    while (!matched && (i < matchRecord.size()))
    {
        if (id == matchRecord.at(i).recordID)
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

    while (!matched && (i < matchRecord.size()))
    {
        if (id == matchRecord.at(i).recordID)
        {
            matched = true;
            result = matchRecord.at(i).newInfoAvailable;
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

    int numRecs = matchRecord.size();

    // Loop through all records
    for (i = 1; i < numRecs; i++)
    {

        // Select the hole position of the number to be inserted
        holePosition = i;

       // Select the value to be inserted.
       recordToInsert = matchRecord.at(i);
       valueToInsert = recordToInsert.netScore;

       // Shift all previously sorted numbers right until the same or higher number is encountered
       while ((holePosition > 0) && (matchRecord.at(i-1).netScore < valueToInsert))
       {
          matchRecord[holePosition] = matchRecord[holePosition-1];
          holePosition--;
       }

       // Insert the new record at its proper position in the sorted list
       if (holePosition != i)
       {
          matchRecord[holePosition] = recordToInsert;
       }
    }
}

int match(const dataRecord &dr, MATCHRESULT &matchScore, bool removeAccents)
{
    // Results are coded by strength of match, where
    // -1 = some type of error encountered
    //  0 = nothing matched
    //  4 = good match (likely match - four good items with no inconsistencies)
    //  5 = solid match with five good items (DOB, DOD, gender, lastName, firstName) and no inconsistencies
    //  +17 if match all three of lastName, firstName and DOB (i.e., KEY in database)
    //  netScore = score less 2 per inconsistency

    GLOBALVARS gv = dr.globals;

    int currentScore, i, j, bonusPts;
    int DBnumLast, DBnumFirst, numLast, numFirst, inconsistencyCount;
    unsigned int gender, DBgender, deceasedNumber;
    bool success, newInfoAvailable;

    PQString DOBdateSQL, DODdateSQL;
    QDate DOB, DOD;
    QString firstName, lastName, altLastName1, altLastName2, altLastName3, altLastName4, nameAKA1, nameAKA2, middleNames;
    QString DBfirstName, DBnameAKA1, DBnameAKA2, DBmiddleNames;
    QString comma(", ");
    QStringList middleNameList, DBmiddleNameList;
    QSqlQuery query;
    QSqlError error;
    PQString errorMessage;

    DOB = dr.getDOB();
    DOD = dr.getDOD();
    if (removeAccents)
    {
        lastName = dr.getLastName().getUnaccentedString();
        altLastName1 = dr.getLastNameAlt1().getUnaccentedString();
        altLastName2 = dr.getLastNameAlt2().getUnaccentedString();
        altLastName3 = dr.getLastNameAlt3().getUnaccentedString();
        firstName = dr.getFirstName().getUnaccentedString();
        nameAKA1 = dr.getFirstNameAKA1().getUnaccentedString();
        nameAKA2 = dr.getFirstNameAKA2().getUnaccentedString();
        middleNames = dr.getMiddleNames().getUnaccentedString();
    }
    else
    {
        lastName = dr.getLastName().getString();
        altLastName1 = dr.getLastNameAlt1().getString();
        altLastName2 = dr.getLastNameAlt2().getString();
        altLastName3 = dr.getLastNameAlt3().getString();
        firstName = dr.getFirstName().getString();
        nameAKA1 = dr.getFirstNameAKA1().getString();
        nameAKA2 = dr.getFirstNameAKA2().getString();
        middleNames = dr.getMiddleNames().getString();
    }
    if (middleNames.size() > 0)
        middleNameList = middleNames.split(' ', QString::SkipEmptyParts);
    gender = static_cast<unsigned int>(dr.getGender());

    if (lastName.size() == 0)
        numLast = 0;
    else
    {
        if (altLastName1.size() == 0)
            numLast = 1;
        else
        {
            if (altLastName2.size() == 0)
                numLast = 2;
            else
            {
                if (altLastName3.size() == 0)
                    numLast = 3;
                else
                    numLast = 4;
            }
        }
    }

    // Strategy is to use least ambiguous available data first

    /******************************************************/
    /*       FIRST PASS - DOB, DOD and Last Name          */
    /******************************************************/

    if (DOB.isValid() && DOD.isValid())
    {
        // Retrieve any records with a matching DOB, DOD and last name
        DOBdateSQL << DOB.toString("yyyy/MM/dd") << QString(" 0:0:0");
        DODdateSQL << DOD.toString("yyyy/MM/dd") << QString(" 0:0:0");

        success = query.prepare("SELECT firstName, nameAKA1, nameAKA2, middleNames, lastName, altLastName1, altLastName2, altLastName3, gender, deceasedNumber "
                                "FROM deceased WHERE DOB = :DOB AND DOD = :DOD AND lastName = :lastName");
        query.bindValue(":DOB", QVariant(DOBdateSQL.getString()));
        query.bindValue(":DOD", QVariant(DODdateSQL.getString()));

        for (j = 0; j < numLast; j++)  // Loop through each possible last name in the record being queried
        {
            switch(j)
            {
            case 0:
                query.bindValue(":lastName", QVariant(lastName));
                bonusPts = 20;
                break;

            case 1:
                query.bindValue(":lastName", QVariant(altLastName1));
                bonusPts = 0;
                break;

            case 2:
                query.bindValue(":lastName", QVariant(altLastName2));
                bonusPts = 0;
                break;

            case 3:
                query.bindValue(":lastName", QVariant(altLastName3));
                bonusPts = 0;
                break;

            }

            success = query.exec();
            if (success)
                success = query.next();
            else
            {
                error = query.lastError();

                errorMessage << QString("SQL error trying to match record using DOB, DOD and last name: ") << lastName << comma << firstName;
                gv.logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
                currentScore = -1;
            }

            if (success)    // Check all other fields for the record
            {
                for (i = 0; i < query.size(); i++)
                {
                    currentScore = 3;
                    inconsistencyCount = 0;
                    newInfoAvailable = false;

                    DBnumLast = 0;
                    DBnumFirst = 0;
                    numFirst = 0;

                    if (removeAccents)
                    {
                        DBfirstName = PQString(query.value(0).toString()).getUnaccentedString();
                        DBnameAKA1 = PQString(query.value(1).toString()).getUnaccentedString();
                        DBnameAKA2 = PQString(query.value(2).toString()).getUnaccentedString();
                        DBmiddleNames = PQString(query.value(3).toString()).getUnaccentedString();
                    }
                    else
                    {
                        DBfirstName = query.value(0).toString();
                        DBnameAKA1 = query.value(1).toString();
                        DBnameAKA2 = query.value(2).toString();
                        DBmiddleNames = query.value(3).toString();
                    }
                    if (DBmiddleNames.size() > 0)
                        DBmiddleNameList = DBmiddleNames.split(' ', QString::SkipEmptyParts);
                    else
                        DBmiddleNameList.clear();
                    DBgender = query.value(8).toUInt();
                    deceasedNumber = query.value(9).toUInt();

                    if ((gender > 0) && (DBgender > 0))
                    {
                        if (gender == DBgender)
                            currentScore++;
                        else
                            inconsistencyCount++;
                    }

                    if (firstName.size() > 0)
                    {
                        numFirst++;
                        if((firstName == DBfirstName) || (firstName == DBnameAKA1) || (firstName == DBnameAKA2))
                        {
                            currentScore++;
                            currentScore += bonusPts;       // Needed to identify pure SQL key match
                            if (firstName == DBfirstName)   // Needed to identify indirect SQL key match
                                currentScore += 17;
                        }
                        // No inconsistency counted if no match is found
                        // TO DO: Database with name variations
                    }

                    if (nameAKA1.size() > 0)
                    {
                        numFirst++;
                        if((nameAKA1 == DBfirstName) || (nameAKA1 == DBnameAKA1) || (nameAKA1 == DBnameAKA2))
                            currentScore++;
                        // No inconsistency counted if no match is found
                    }

                    if (nameAKA2.size() > 0)
                    {
                        numFirst++;
                        if((nameAKA2 == DBfirstName) || (nameAKA2 == DBnameAKA1) || (nameAKA2 == DBnameAKA2))
                            currentScore++;
                        // No inconsistency counted if no match is found
                    }

                    if (middleNames.size() > 0)
                    {
                        // Limit additional score to 1
                        bool matchFound = false;
                        while (!matchFound && (i < middleNameList.size()))
                        {
                            while (!matchFound && (j < DBmiddleNameList.size()))
                            {
                                if (middleNameList[i] == DBmiddleNameList[j])
                                    matchFound = true;
                                j++;
                            }
                            i++;
                        }

                        if (matchFound)
                            currentScore++;
                    }

                    // Determine if new record has more information than is recorded in DB
                    if (query.value(4).toString().size() > 0)
                        DBnumLast++;
                    if (query.value(5).toString().size() > 0)
                        DBnumLast++;
                    if (query.value(6).toString().size() > 0)
                        DBnumLast++;
                    if (query.value(7).toString().size() > 0)
                        DBnumLast++;

                    if (DBfirstName.size() > 0)
                        DBnumFirst++;
                    if (DBnameAKA1.size() > 0)
                        DBnumFirst++;
                    if (DBnameAKA2.size() > 0)
                        DBnumFirst++;

                    if ((numLast > DBnumLast) || (middleNameList.size() > DBmiddleNameList.size()) || (numFirst > DBnumFirst) || ((gender > 0) && (DBgender == 0)))
                        newInfoAvailable = true;

                    // Assessment
                    matchScore.update(deceasedNumber, currentScore, inconsistencyCount, newInfoAvailable);

                    query.next();
                }
            }

        } // end of loop through last name and alternates

        query.clear();

    } // end of query using DOB, DOD and last name

    if (matchScore.getBestScore() >= 5)
        return matchScore.getBestScore();

    /******************************************************/
    /*         SECOND PASS - DOB and Last Name            */
    /******************************************************/

    if (DOB.isValid())
    {
        // Retrieve any records with a matching DOB and last name
        DOBdateSQL << DOB.toString("yyyy/MM/dd") << QString(" 0:0:0");

        success = query.prepare("SELECT firstName, nameAKA1, nameAKA2, middleNames, lastName, altLastName1, altLastName2, altLastName3, gender, deceasedNumber "
                                "FROM deceased WHERE DOB = :DOB AND lastName = :lastName");
        query.bindValue(":DOB", QVariant(DOBdateSQL.getString()));

        for (j = 0; j < numLast; j++)  // Loop through each possible last name in the record being queried
        {
            switch(j)
            {
            case 0:
                query.bindValue(":lastName", QVariant(lastName));
                bonusPts = 20;
                break;

            case 1:
                query.bindValue(":lastName", QVariant(altLastName1));
                bonusPts = 0;
                break;

            case 2:
                query.bindValue(":lastName", QVariant(altLastName2));
                bonusPts = 0;
                break;

            case 3:
                query.bindValue(":lastName", QVariant(altLastName3));
                bonusPts = 0;
                break;
            }

            success = query.exec();
            if (success)
                success = query.next();
            else
            {
                error = query.lastError();

                errorMessage << QString("SQL error trying to match record using DOB and last name: ") << lastName << comma << firstName;
                gv.logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
                currentScore = -1;
            }

            if (success)    // Check all other fields for the record
            {
                for (i = 0; i < query.size(); i++)
                {
                    currentScore = 2;
                    inconsistencyCount = 0;
                    newInfoAvailable = false;

                    DBnumLast = 0;
                    DBnumFirst = 0;
                    numFirst = 0;

                    if (removeAccents)
                    {
                        DBfirstName = PQString(query.value(0).toString()).getUnaccentedString();
                        DBnameAKA1 = PQString(query.value(1).toString()).getUnaccentedString();
                        DBnameAKA2 = PQString(query.value(2).toString()).getUnaccentedString();
                        DBmiddleNames = PQString(query.value(3).toString()).getUnaccentedString();
                    }
                    else
                    {
                        DBfirstName = query.value(0).toString();
                        DBnameAKA1 = query.value(1).toString();
                        DBnameAKA2 = query.value(2).toString();
                        DBmiddleNames = query.value(3).toString();
                    }

                    if (DBmiddleNames.size() > 0)
                        DBmiddleNameList = DBmiddleNames.split(' ', QString::SkipEmptyParts);
                    else
                        DBmiddleNameList.clear();
                    DBgender = query.value(8).toUInt();
                    deceasedNumber = query.value(9).toUInt();

                    if ((gender > 0) && (DBgender > 0))
                    {
                        if (gender == DBgender)
                            currentScore++;
                        else
                            inconsistencyCount++;
                    }

                    if (firstName.size() > 0)
                    {
                        numFirst++;
                        if((firstName == DBfirstName) || (firstName == DBnameAKA1) || (firstName == DBnameAKA2))
                        {
                            currentScore++;
                            currentScore += bonusPts;       // Needed to identify pure SQL key match
                            if (firstName == DBfirstName)   // Needed to identify indirect SQL key match
                                currentScore += 17;
                        }
                        // No inconsistency counted if no match is found
                        // TO DO: Database with name variations
                    }

                    if (nameAKA1.size() > 0)
                    {
                        numFirst++;
                        if((nameAKA1 == DBfirstName) || (nameAKA1 == DBnameAKA1) || (nameAKA1 == DBnameAKA2))
                            currentScore++;
                        // No inconsistency counted if no match is found
                    }

                    if (nameAKA2.size() > 0)
                    {
                        numFirst++;
                        if((nameAKA2 == DBfirstName) || (nameAKA2 == DBnameAKA1) || (nameAKA2 == DBnameAKA2))
                            currentScore++;
                        // No inconsistency counted if no match is found
                    }

                    if (middleNames.size() > 0)
                    {
                        // Limit additional score to 1
                        bool matchFound = false;
                        while (!matchFound && (i < middleNameList.size()))
                        {
                            while (!matchFound && (j < DBmiddleNameList.size()))
                            {
                                if (middleNameList[i] == DBmiddleNameList[j])
                                    matchFound = true;
                                j++;
                            }
                            i++;
                        }

                        if (matchFound)
                            currentScore++;
                    }

                    // Determine if new record has more information than is recorded in DB
                    if (query.value(4).toString().size() > 0)
                        DBnumLast++;
                    if (query.value(5).toString().size() > 0)
                        DBnumLast++;
                    if (query.value(6).toString().size() > 0)
                        DBnumLast++;
                    if (query.value(7).toString().size() > 0)
                        DBnumLast++;

                    if (DBfirstName.size() > 0)
                        DBnumFirst++;
                    if (DBnameAKA1.size() > 0)
                        DBnumFirst++;
                    if (DBnameAKA2.size() > 0)
                        DBnumFirst++;

                    if ((numLast > DBnumLast) || (middleNameList.size() > DBmiddleNameList.size()) || (numFirst > DBnumFirst) ||
                        ((gender > 0) && (DBgender == 0)) || DOD.isValid())
                        newInfoAvailable = true;

                    // Assessment
                    matchScore.update(deceasedNumber, currentScore, inconsistencyCount, newInfoAvailable);

                    query.next();
                }
            }

        } // end of loop through last name and alternates

        query.clear();

    } // end of query using DOB and last name

    if (matchScore.getBestScore() >= 5)
        return matchScore.getBestScore();

    /******************************************************/
    /*         THIRD PASS - DOD and Last Name            */
    /******************************************************/

    if (DOD.isValid())
    {
        // Retrieve any records with a matching DOD and last name
        DODdateSQL << DOD.toString("yyyy/MM/dd") << QString(" 0:0:0");

        success = query.prepare("SELECT firstName, nameAKA1, nameAKA2, middleNames, lastName, altLastName1, altLastName2, altLastName3, gender, deceasedNumber "
                                "FROM deceased WHERE DOD = :DOD AND lastName = :lastName");
        query.bindValue(":DOD", QVariant(DODdateSQL.getString()));

        for (j = 0; j < numLast; j++)  // Loop through each possible last name in the record being queried
        {
            switch(j)
            {
            case 0:
                query.bindValue(":lastName", QVariant(lastName));
                break;

            case 1:
                query.bindValue(":lastName", QVariant(altLastName1));
                break;

            case 2:
                query.bindValue(":lastName", QVariant(altLastName2));
                break;

            case 3:
                query.bindValue(":lastName", QVariant(altLastName3));
                break;
            }

            success = query.exec();
            if (success)
                success = query.next();
            else
            {
                error = query.lastError();

                errorMessage << QString("SQL error trying to match record using DOD and last name: ") << lastName << comma << firstName;
                gv.logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
                currentScore = -1;
            }

            if (success)    // Check all other fields for the record
            {
                for (i = 0; i < query.size(); i++)
                {
                    currentScore = 2;
                    inconsistencyCount = 0;
                    newInfoAvailable = false;

                    DBnumLast = 0;
                    DBnumFirst = 0;
                    numFirst = 0;

                    if (removeAccents)
                    {
                        DBfirstName = PQString(query.value(0).toString()).getUnaccentedString();
                        DBnameAKA1 = PQString(query.value(1).toString()).getUnaccentedString();
                        DBnameAKA2 = PQString(query.value(2).toString()).getUnaccentedString();
                        DBmiddleNames = PQString(query.value(3).toString()).getUnaccentedString();
                    }
                    else
                    {
                        DBfirstName = query.value(0).toString();
                        DBnameAKA1 = query.value(1).toString();
                        DBnameAKA2 = query.value(2).toString();
                        DBmiddleNames = query.value(3).toString();
                    }

                    if (DBmiddleNames.size() > 0)
                        DBmiddleNameList = DBmiddleNames.split(' ', QString::SkipEmptyParts);
                    else
                        DBmiddleNameList.clear();
                    DBgender = query.value(8).toUInt();
                    deceasedNumber = query.value(9).toUInt();

                    if ((gender > 0) && (DBgender > 0))
                    {
                        if (gender == DBgender)
                            currentScore++;
                        else
                            inconsistencyCount++;
                    }

                    if (firstName.size() > 0)
                    {
                        numFirst++;
                        if((firstName == DBfirstName) || (firstName == DBnameAKA1) || (firstName == DBnameAKA2))
                            currentScore++;
                        // No inconsistency counted if no match is found
                        // TO DO: Database with name variations
                    }

                    if (nameAKA1.size() > 0)
                    {
                        numFirst++;
                        if((nameAKA1 == DBfirstName) || (nameAKA1 == DBnameAKA1) || (nameAKA1 == DBnameAKA2))
                            currentScore++;
                        // No inconsistency counted if no match is found
                    }

                    if (nameAKA2.size() > 0)
                    {
                        numFirst++;
                        if((nameAKA2 == DBfirstName) || (nameAKA2 == DBnameAKA1) || (nameAKA2 == DBnameAKA2))
                            currentScore++;
                        // No inconsistency counted if no match is found
                    }

                    if (middleNames.size() > 0)
                    {
                        // Limit additional score to 1
                        bool matchFound = false;
                        while (!matchFound && (i < middleNameList.size()))
                        {
                            while (!matchFound && (j < DBmiddleNameList.size()))
                            {
                                if (middleNameList[i] == DBmiddleNameList[j])
                                    matchFound = true;
                                j++;
                            }
                            i++;
                        }

                        if (matchFound)
                            currentScore++;
                    }

                    // Determine if new record has more information than is recorded in DB
                    if (query.value(4).toString().size() > 0)
                        DBnumLast++;
                    if (query.value(5).toString().size() > 0)
                        DBnumLast++;
                    if (query.value(6).toString().size() > 0)
                        DBnumLast++;
                    if (query.value(7).toString().size() > 0)
                        DBnumLast++;

                    if (DBfirstName.size() > 0)
                        DBnumFirst++;
                    if (DBnameAKA1.size() > 0)
                        DBnumFirst++;
                    if (DBnameAKA2.size() > 0)
                        DBnumFirst++;

                    if ((numLast > DBnumLast) || (middleNameList.size() > DBmiddleNameList.size()) || (numFirst > DBnumFirst) ||
                        ((gender > 0) && (DBgender == 0)) || DOB.isValid())
                        newInfoAvailable = true;

                    // Assessment
                    matchScore.update(deceasedNumber, currentScore, inconsistencyCount, newInfoAvailable);

                    query.next();
                }
            }

        } // end of loop through last name and alternates

        query.clear();

    } // end of query using DOD and last name

    if (matchScore.getBestScore() >= 4)
        return matchScore.getBestScore();

    /******************************************************/
    /*      FOURTH PASS - Last and First Name Only        */
    /******************************************************/

    success = query.prepare("SELECT firstName, nameAKA1, nameAKA2, middleNames, lastName, altLastName1, altLastName2, altLastName3, gender, deceasedNumber "
                            "FROM deceased WHERE lastName = :lastName AND (firstName = :firstName OR nameAKA1 = :firstName OR nameAKA2 = :firstname)");
    query.bindValue(":firstName", QVariant(firstName));

    for (j = 0; j < numLast; j++)  // Loop through each possible last name in the record being queried
    {
        switch(j)
        {
        case 0:
            query.bindValue(":lastName", QVariant(lastName));
            break;

        case 1:
            query.bindValue(":lastName", QVariant(altLastName1));
            break;

        case 2:
            query.bindValue(":lastName", QVariant(altLastName2));
            break;

        case 3:
            query.bindValue(":lastName", QVariant(altLastName3));
            break;

        }

        success = query.exec();
        if (success)
            success = query.next();
        else
        {
            error = query.lastError();

            errorMessage << QString("SQL error trying to match record using last name only: ") << lastName << comma << firstName;
            gv.logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
            currentScore = -1;
        }

        if (success)    // Check all other fields for the record
        {
            for (i = 0; i < query.size(); i++)
            {
                currentScore = 2;
                inconsistencyCount = 0;
                newInfoAvailable = false;

                DBnumLast = 0;
                DBnumFirst = 0;
                numFirst = 1;

                if (removeAccents)
                {
                    DBfirstName = PQString(query.value(0).toString()).getUnaccentedString();
                    DBnameAKA1 = PQString(query.value(1).toString()).getUnaccentedString();
                    DBnameAKA2 = PQString(query.value(2).toString()).getUnaccentedString();
                    DBmiddleNames = PQString(query.value(3).toString()).getUnaccentedString();
                }
                else
                {
                    DBfirstName = query.value(0).toString();
                    DBnameAKA1 = query.value(1).toString();
                    DBnameAKA2 = query.value(2).toString();
                    DBmiddleNames = query.value(3).toString();
                }

                if (DBmiddleNames.size() > 0)
                    DBmiddleNameList = DBmiddleNames.split(' ', QString::SkipEmptyParts);
                else
                    DBmiddleNameList.clear();
                DBgender = query.value(8).toUInt();
                deceasedNumber = query.value(9).toUInt();

                if ((gender > 0) && (DBgender > 0))
                {
                    if (gender == DBgender)
                        currentScore++;
                    else
                        inconsistencyCount++;
                }

                if (nameAKA1.size() > 0)
                {
                    numFirst++;
                    if((nameAKA1 == DBfirstName) || (nameAKA1 == DBnameAKA1) || (nameAKA1 == DBnameAKA2))
                        currentScore++;
                    // No inconsistency counted if no match is found
                }

                if (nameAKA2.size() > 0)
                {
                    numFirst++;
                    if((nameAKA2 == DBfirstName) || (nameAKA2 == DBnameAKA1) || (nameAKA2 == DBnameAKA2))
                        currentScore++;
                    // No inconsistency counted if no match is found
                }

                if (middleNames.size() > 0)
                {
                    // Limit additional score to 1
                    bool matchFound = false;
                    while (!matchFound && (i < middleNameList.size()))
                    {
                        while (!matchFound && (j < DBmiddleNameList.size()))
                        {
                            if (middleNameList[i] == DBmiddleNameList[j])
                                matchFound = true;
                            j++;
                        }
                        i++;
                    }

                    if (matchFound)
                        currentScore++;
                }

                // Determine if new record has more information than is recorded in DB
                if (query.value(4).toString().size() > 0)
                    DBnumLast++;
                if (query.value(5).toString().size() > 0)
                    DBnumLast++;
                if (query.value(6).toString().size() > 0)
                    DBnumLast++;
                if (query.value(7).toString().size() > 0)
                    DBnumLast++;

                if (DBfirstName.size() > 0)
                    DBnumFirst++;
                if (DBnameAKA1.size() > 0)
                    DBnumFirst++;
                if (DBnameAKA2.size() > 0)
                    DBnumFirst++;

                if ((numLast > DBnumLast) || (middleNameList.size() > DBmiddleNameList.size()) || (numFirst > DBnumFirst) ||
                    ((gender > 0) && (DBgender == 0)) || DOD.isValid() || DOB.isValid())
                    newInfoAvailable = true;

                // Assessment
                matchScore.update(deceasedNumber, currentScore, inconsistencyCount, newInfoAvailable);

                query.next();
            }
        }

    } // end of loop through last name and alternates

    query.clear();


    return matchScore.getBestScore();
}
