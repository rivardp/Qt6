// databaseSearches.cpp

#include "databaseSearches.h"
#include "../UpdateFuneralHomes/Include/dataRecord.h"

int databaseSearches::surnameLookup(const QString name, GLOBALVARS *globals)
{
    QSqlQuery query;
    QSqlError error;

    bool success;
    int numMatches = 0;
    PQString errorMessage;

    success = query.prepare("SELECT COUNT(*) FROM death_audits.deceased WHERE lastName = :lastName");
    query.bindValue(":lastName", QVariant(name));
    success = query.exec();
    if (!success || (query.size() > 1))
    {
        error = query.lastError();
        errorMessage << QString("SQL problem looking up: ") << name << " in deceased database";
        globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
        return false;
    }
    else
    {
        if (query.size() == 1)
        {
            query.next();
            numMatches = query.value(0).toInt();
        }
    }

    query.clear();

    return numMatches;
}

bool databaseSearches::areUniquelySurnames(QList<OQString> &listOfNames, GLOBALVARS *gv, GENDER gender)
{
    if (listOfNames.size() == 0)
        return false;

    bool valid = true;
    QString word;

    while (valid && !listOfNames.isEmpty())
    {
        word = listOfNames.takeFirst().getString();
        valid = (surnameLookup(word, gv) > 0);
        if (valid)
            valid = !givenNameLookup(word, gv, gender);
    }

    return valid;
}

bool databaseSearches::areAllNames(QList<OQString> &listOfNames, GLOBALVARS *gv)
{
    if (listOfNames.size() == 0)
        return false;

    bool valid = true;
    QString word;

    while (valid && !listOfNames.isEmpty())
    {
        word = listOfNames.takeFirst().getString();
        valid = givenNameLookup(word, gv);
        if (!valid)
            valid = nicknameLookup(word, gv);
    }

    return valid;
}

bool databaseSearches::givenNameLookup(const QString name, GLOBALVARS *globals, GENDER gender)
{
    int maleCount = 0;
    int femaleCount = 0;

    return givenNameLookup(name, globals, maleCount, femaleCount, gender);
}

bool databaseSearches::givenNameLookup(const QString name, GLOBALVARS *globals, int &maleCount, int &femaleCount, GENDER gender)
{
    QSqlQuery query;
    QSqlError error;

    bool success;
    int numMatches;
    double malePct;
    PQString errorMessage;
    bool result = false;

    success = query.prepare("SELECT maleCount, femaleCount, malePct FROM death_audits.firstnames WHERE name = :name");
    query.bindValue(":name", QVariant(name));
    success = query.exec();
    if (!success || (query.size() > 1))
    {
        error = query.lastError();
        errorMessage << QString("SQL problem looking up: ") << name << " in firstnames database - " << globals->globalDr->getURL();
        globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
        return false;
    }
    else
    {
        if (query.size() == 1)
        {
            query.next();
            maleCount = query.value(0).toInt();
            femaleCount = query.value(1).toInt();
            malePct = query.value(2).toDouble();
        }
        else
        {
            maleCount = 0;
            femaleCount = 0;
            return false;
        }
    }

    query.clear();

    numMatches = maleCount + femaleCount;
    switch(gender)
    {
    case Male:
        if (((numMatches < 100) && (maleCount > 0)) || ((numMatches >= 100) && (malePct > 0.005)))
            result = true;
        break;

    case Female:
        if (((numMatches < 100) && (femaleCount > 0)) || ((numMatches >= 100) && (malePct < 0.995)))
            result = true;
        break;

    case genderUnknown:
        if (numMatches >= 1)
            result = true;

        break;
    }

    return result;
}

bool databaseSearches::nicknameLookup(const QString name, GLOBALVARS *gv)
{
    QSqlQuery query;
    QSqlError error;

    bool success;
    PQString errorMessage;

    success = query.prepare("SELECT nickname FROM death_audits.nicknames WHERE nickname = :nickname");
    query.bindValue(":nickname", QVariant(name));
    success = query.exec();
    if (!success)
    {
        error = query.lastError();
        errorMessage << QString("SQL problem looking up: ") << name << " in nicknames database";
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
        return false;
    }
    else
    {
        if (query.size() == 0)
            return false;
        else
            return true;
    }
}

bool databaseSearches::pureNickNameLookup(const QString name, GLOBALVARS *gv)
{
    QSqlQuery query;
    QSqlError error;

    bool success;
    bool isPure = false;
    QString formalNames;
    PQString errorMessage;

    success = query.prepare("SELECT formalNames FROM death_audits.nicknames WHERE nickname = :nickname");
    query.bindValue(":nickname", QVariant(name));
    success = query.exec();
    if (!success || (query.size() > 1))
    {
        error = query.lastError();
        errorMessage << QString("SQL problem looking up: ") << name << " in nicknames database";
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
        return false;
    }
    else
    {
        if (query.size() == 1)
        {
            query.next();
            formalNames = query.value(0).toString();
            if (formalNames.size() == 0)
                isPure = true;
        }
    }

    query.clear();

    return isPure;
}

bool databaseSearches::nickNameInList(const OQString name, QList<QString> &listOfNames, GLOBALVARS *gv)
{
    bool isNickName = false;
    PQString errMsg;

    for (int i = 0; i < listOfNames.size(); i++)
        isNickName = isNickName || name.isInformalVersionOf(listOfNames.at(i), errMsg);

    if (errMsg.getLength() > 0)
        gv->logMsg(ErrorRecord, errMsg);

    return isNickName;
}

void databaseSearches::nameStatLookup(QString name, GLOBALVARS *globals, NAMESTATS &nameStats, GENDER gender, bool skipClear)
{
    if (!skipClear)
        nameStats.clear();

    if (name.size() == 1)
    {
        nameStats.name = name;
        nameStats.isLikelyGivenName = true;
        return;
    }

    QSqlQuery query;
    QSqlError error;

    bool success;
    unsigned int count = 0;
    double factor = 1;
    PQString errorMessage;

    success = query.prepare("SELECT maleCount, femaleCount, malePct FROM death_audits.firstnames WHERE name = :name");
    query.bindValue(":name", QVariant(name));
    success = query.exec();

    if (success)
    {
        nameStats.name = name;

        if (query.size() == 1)
        {
            query.next();
            nameStats.maleCount = query.value(0).toUInt();
            nameStats.femaleCount = query.value(1).toUInt();
            nameStats.malePct = query.value(2).toDouble();
            nameStats.determineCredibility();

            switch (gender)
            {
            case Male:
                count = nameStats.maleCount;
                factor = 1;
                break;

            case Female:
                count = nameStats.femaleCount;
                factor = 1;
                break;

            case genderUnknown:
                count = nameStats.maleCount + nameStats.femaleCount;
                factor = 0.5;
                break;
            }

            if (count > 0)
                nameStats.isGivenName = true;
        }

        nameStats.surnameCount = static_cast<unsigned int>(surnameLookup(name, globals));
        nameStats.isSurname = (nameStats.surnameCount > 0);

        if ((count > 0) && (nameStats.surnameCount == 0))
            nameStats.isLikelyGivenName = true;

        if ((count == 0) && (nameStats.surnameCount > 0))
            nameStats.isLikelySurname = true;

        if ((count > 0) && (nameStats.surnameCount > 0))
        {
            // There are approximately 1.25 first names (combined) for every last name
            double ratio = nameStats.surnameCount * 1.25 * factor / count;
            if (ratio > 3)
                nameStats.isLikelySurname = true;
            if (ratio < 0.333)
                nameStats.isLikelyGivenName = true;
        }

        if ((count == 0) && (nameStats.surnameCount == 0))
            nameStats.isPureNickName = pureNickNameLookup(name, globals);
    }
    else
    {
        error = query.lastError();
        errorMessage << QString("SQL problem looking up nameStats for: ") << name << " in firstnames database - " << globals->globalDr->getURL();
        globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }

    query.clear();

}

bool databaseSearches::validRecordExists(downloadOutputs &outputs, GLOBALVARS *globals)
{
    bool validRecordAlready, success;

    QSqlQuery query;
    QSqlError error;
    PQString errorMessage;

    //success = query.prepare("SELECT cycle FROM death_audits.deceased WHERE providerID = :providerID AND providerKey = :providerKey AND ID = :ID");
    success = query.prepare("SELECT deceasedNumber "
                            "FROM death_audits.deceasedidinfo "
                            "WHERE providerID = :providerID AND providerKey = :providerKey AND ID = :ID "
                            "GROUP BY deceasedNumber");
    query.bindValue(":providerID", QVariant(outputs.providerID));
    query.bindValue(":providerKey", QVariant(outputs.providerKey));
    query.bindValue(":ID", QVariant(outputs.ID));
    success = query.exec();

    if (success)
    {
        if (query.size() == 0)
            validRecordAlready = false;
        else
        {
            query.next();
            validRecordAlready = !query.value(0).toBool();
        }
    }
    else
    {
        validRecordAlready = false;
        error = query.lastError();
        errorMessage << QString("SQL problem confirming a valid record for: ");
        errorMessage << outputs.providerID << "-" << outputs.providerKey  << "-" << outputs.ID;
        errorMessage << " in deceased database";
        globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }

    query.clear();


    return validRecordAlready;
}

void databaseSearches::removeLowConviction(QList<QString> &nameList, GLOBALVARS *globals)
{
    int position = 0;
    double threshold = 0.80;
    QString name;
    NAMESTATS nameStats;

    QSqlQuery query;
    QSqlError error;

    bool success;
    PQString errorMessage;

    while (position < nameList.size())
    {
        name = nameList.at(position);
        if (name.length() < 2)
            nameList.removeAt(position);
        else
        {
            success = query.prepare("SELECT maleCount, femaleCount, malePct FROM death_audits.firstnames WHERE name = :name");
            query.bindValue(":name", QVariant(name));
            success = query.exec();

            if (success)
            {
                if (query.size() == 1)
                {
                    query.next();
                    nameStats.maleCount = query.value(0).toUInt();
                    nameStats.femaleCount = query.value(1).toUInt();
                    nameStats.malePct = query.value(2).toDouble();
                    if ((nameStats.malePct >= threshold) || (nameStats.malePct <= (1.0 - threshold)))
                        position++;
                    else
                        nameList.removeAt(position);
                }
                else    // Name not found in database
                    nameList.removeAt(position);
            }
            else
            {
                error = query.lastError();
                errorMessage << QString("SQL problem looking up gender for: ") << name << " in firstnames database - " << globals->globalDr->getURL();
                globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));

                nameList.removeAt(position);
            }
        }

        query.clear();
    }
}

QString databaseSearches::IDforLastDeathPreceding(const QDate &DOD, const unsigned int providerID, const unsigned int providerKey, GLOBALVARS *gv)
{
    QSqlQuery query;
    QSqlError error;
    PQString errorMessage;
    QString result;
    QDate targetDOD;

    bool success;
    success = query.prepare("SELECT MAX(d.DOD), MAX(ft.ID) FROM "
                            "death_audits.deceased d INNER JOIN "
                            "(SELECT dts.deceasedNumber, dts.ID FROM death_audits.deceasedidinfo dts WHERE dts.providerID = :providerID AND dts.providerKey = :providerKey ORDER BY dts.deceasedNumber) "
                            "AS ft USING(deceasedNumber) "
                            "WHERE d.DOD <= :DOD");
    query.bindValue(":providerID", QVariant(providerID));
    query.bindValue(":providerKey", QVariant(providerKey));
    query.bindValue(":DOD", QVariant(DOD));
    success = query.exec();

    if (success)
    {
        if (query.size() == 0)
            result = QString("0");
        else
        {
            query.next();
            targetDOD = query.value(0).toDate();

            query.clear();
            success = query.prepare("SELECT MAX(d.DOD), MAX(ft.ID) FROM "
                                    "death_audits.deceased d INNER JOIN "
                                    "(SELECT dts.deceasedNumber, dts.ID FROM death_audits.deceasedidinfo dts WHERE dts.providerID = :providerID AND dts.providerKey = :providerKey ORDER BY dts.deceasedNumber) "
                                    "AS ft USING(deceasedNumber) "
                                    "WHERE d.DOD = :DOD");
            query.bindValue(":providerID", QVariant(providerID));
            query.bindValue(":providerKey", QVariant(providerKey));
            query.bindValue(":DOD", QVariant(targetDOD));
            success = query.exec();

            if (success)
            {
                query.next();
                result = query.value(1).toString();
            }
        }
    }
    else
    {
        result = QString("0");
        error = query.lastError();
        errorMessage << QString("SQL problem finding ID for specified DOD for: ");
        errorMessage << providerID << "-" << providerKey;
        errorMessage << " in deceased database";
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }

    query.clear();

    return result;
}

QDate databaseSearches::getLastPubDate(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey)
{
    QSqlQuery query;
    QSqlError error;

    bool success;
    bool errorEncountered = false;

    QDate result(1900,1,1);
    QDate result1(1900,1,1);

    if ((provID == TributeArchive) && (provKey == 9999))
    {
        success = query.prepare("SELECT MAX(GREATEST(d.DOD, ft.publishDate)) FROM death_audits.deceased d INNER JOIN "
                                "(SELECT dts.deceasedNumber, dts.publishDate FROM death_audits.deceasedidinfo dts WHERE dts.providerID = :providerID ORDER BY dts.deceasedNumber) "
                                "AS ft USING(deceasedNumber)");
        query.bindValue(":providerID", QVariant(provID));

        success = query.exec();
        if (success && (query.size() == 1))
        {
            query.next();
            result1 = query.value(0).toDate();
            if (result1.isValid())
                result = result1;
        }
        else
            errorEncountered = true;
    }
    else
    {
        success = query.prepare("SELECT MAX(GREATEST(d.DOD, ft.publishDate)) FROM death_audits.deceased d INNER JOIN "
                                "(SELECT dts.deceasedNumber, dts.publishDate FROM death_audits.deceasedidinfo dts WHERE dts.providerID = :providerID AND dts.providerKey = :providerKey ORDER BY dts.deceasedNumber) "
                                "AS ft USING(deceasedNumber)");
        query.bindValue(":providerID", QVariant(provID));
        query.bindValue(":providerKey", QVariant(provKey));

        success = query.exec();
        if (success && (query.size() == 1))
        {
            query.next();
            result1 = query.value(0).toDate();
            if (result1.isValid())
                result = result1;
            else
                result = QDate(2019,12,1);
        }
        else
            errorEncountered = true;
    }

    if (errorEncountered)
    {
        PQString errorMessage;

        error = query.lastError();
        errorMessage << QString("SQL problem finding lastest publishDate for: ");
        errorMessage << provID << "-" << provKey;
        errorMessage << " in deceased database";
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }

    return result;
}

QString databaseSearches::lookupPostalCode(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey)
{
    QSqlQuery query;
    QSqlError error;

    bool success;
    bool errorEncountered = false;

    QString result;

    success = query.prepare("SELECT fhPostalCode FROM death_audits.funeralhomedata WHERE providerID = :providerID AND providerKey = :providerKey "
                                                                                   "AND ((fhRunStatus = 1) OR (fhRunStatus = 2))");
    query.bindValue(":providerID", QVariant(provID));
    query.bindValue(":providerKey", QVariant(provKey));

    success = query.exec();
    if (success && (query.size() == 1))
    {
        query.next();
        result = query.value(0).toString();
    }
    else
        errorEncountered = true;

    if (errorEncountered)
    {
        PQString errorMessage;

        error = query.lastError();
        errorMessage << QString("SQL problem finding postal code for: ");
        errorMessage << provID << "-" << provKey;
        errorMessage << " in funeralHomeData database";
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }

    return result;
}

QString databaseSearches::pcLookup(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey, QString location)
{
    QSqlQuery query;
    QSqlError error;

    bool success;
    bool errorEncountered = false;

    QString result;

    success = query.prepare("SELECT postalCode FROM death_audits.funeralhomelocation WHERE providerID = :providerID AND providerKey = :providerKey "
                                                                                    "AND location = :location");
    query.bindValue(":providerID", QVariant(provID));
    query.bindValue(":providerKey", QVariant(provKey));
    query.bindValue(":location", QVariant(location));

    success = query.exec();
    if (success && (query.size() == 1))
    {
        query.next();
        result = query.value(0).toString();
    }
    else
        errorEncountered = true;

    if (errorEncountered)
    {
        PQString errorMessage;

        if (success && (query.size() == 0))
        {
            errorMessage << "No postal code entry for :";
            errorMessage << provID << "-" << provKey << "-" << location;
            errorMessage << " in funeralHomeLocation database";
            gv->logMsg(ErrorSQL, errorMessage);
        }
        else
        {
            error = query.lastError();
            errorMessage << QString("SQL problem finding postal code based on location for: ");
            errorMessage << provID << "-" << provKey << "-" << location;
            errorMessage << " in funeralHomeLocation database";
            gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
        }
    }

    return result;
}

POSTALCODE_INFO databaseSearches::pcLookupPlaces(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey, QString location)
{
    QSqlQuery queryA, queryB;
    QSqlError errorA, errorB;

    bool success;
    bool errorEncounteredA = false;
    bool errorEncounteredB = false;

    QString result, tempResult;
    double distance, tempDistance;
    POSTALCODE_INFO pcInfoFH, pcInfoPlace, pcResult;
    PostalCodes *pcDatabase;

    success = queryA.prepare("SELECT postalCode, location, province, prov, provinceEnum, timezone, latitude, longitude FROM death_audits.postalcodeplaces WHERE location = :location");
    queryA.bindValue(":location", QVariant(location));

    success = queryA.exec();
    if (success)
    {
        if (queryA.size() == 1)
        {
            queryA.next();
            POSTALCODE_INFO temp(queryA.value(0).toString(), queryA.value(1).toString(), queryA.value(2).toString(), queryA.value(3).toString(), queryA.value(4).toInt(), queryA.value(5).toInt(), queryA.value(6).toDouble(), queryA.value(7).toDouble());
            if (temp.isValid())
                pcResult = temp;
        }
        else
        {
            success = queryB.prepare("SELECT fhPostalCode FROM death_audits.funeralhomedata WHERE providerID = :providerID and providerKey = :providerKey");
            queryB.bindValue(":providerID", QVariant(provID));
            queryB.bindValue(":providerKey", QVariant(provKey));

            success = queryB.exec();
            if (success && (queryB.size() == 1))
            {
                queryB.next();
                tempResult = queryB.value(0).toString();
            }
            else
                errorEncounteredB = true;

            if (!errorEncounteredB)
            {
                distance = 9999;
                pcDatabase = new PostalCodes();
                pcInfoFH = pcDatabase->lookup(tempResult);
                if (pcInfoFH.isValid())
                {
                    while (queryA.next())
                    {
                        tempResult = queryA.value(0).toString();
                        pcInfoPlace = pcDatabase->lookup(tempResult);
                        if (pcInfoPlace.isValid())
                        {
                            tempDistance = pcInfoFH.distanceTo(pcInfoPlace);
                            if (tempDistance < distance)
                                pcResult = pcInfoPlace;
                        }
                    }
                }
                delete pcDatabase;
                pcDatabase = nullptr;
            }
        }
    }
    else
        errorEncounteredA = true;

    if (errorEncounteredA)
    {
        PQString errorMessage;

        errorA = queryA.lastError();
        errorMessage << QString("SQL problem finding postal code based on place for: ");
        errorMessage << provID << "-" << provKey << "-" << location;
        errorMessage << " in postalcodeplaces database";
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(errorA.type()));
    }

    if (errorEncounteredB)
    {
        PQString errorMessage;

        errorB = queryB.lastError();
        errorMessage << QString("SQL problem finding postal code based on place for: ");
        errorMessage << provID << "-" << provKey << "-" << location;
        errorMessage << " in funeralhomedata database";
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(errorB.type()));
    }

    return pcResult;
}

QList<QString> databaseSearches::getURLlist(unsigned int deceasedID, GLOBALVARS *gv)
{
    QList<QString> urlList;

    QSqlQuery query;
    QSqlError error;
    PQString errorMessage;
    bool success;

    success = query.prepare("SELECT url FROM death_audits.deceasedidinfo WHERE deceasedNumber = :deceasedNumber");

    query.bindValue(":deceasedNumber", QVariant(deceasedID));

    success = query.exec();

    if (!success || (query.size() == 0))
    {
        error = query.lastError();
        errorMessage << QString("SQL problem trying to obtain urls for deceasedID: ") << deceasedID << " in monitored database";
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }
    else
    {
        while (query.next())
        {
            urlList.append(query.value(0).toString());
        }
    }

    return urlList;
}

bool databaseSearches::deceasedRecordExists(const unsigned int &providerID, const unsigned int &providerKey, const QString &deceasedID, const QString &url, const QDate &pubDate)
{
    QSqlQuery query;
    QSqlError error;
    PQString pubDateSQL;

    Q_UNUSED(url);

    bool success;
    bool result = false;

    if (pubDate.isValid() && (pubDate > QDate(1900,1,1)))
    {
        pubDateSQL << pubDate.toString("yyyy/MM/dd") << QString(" 0:0:0");

        success = query.prepare("SELECT deceasedNumber FROM death_audits.deceasedidinfo WHERE ((providerID = :providerID) AND (providerKey = :providerKey) AND (ID = :deceasedID) AND (publishDate >= :publishDate))");
        query.bindValue(":providerID", QVariant(providerID));
        query.bindValue(":providerKey", QVariant(providerKey));
        query.bindValue(":deceasedID", QVariant(deceasedID));
        query.bindValue(":publishDate", QVariant(pubDateSQL.getString()));
    }
    else
    {
        success = query.prepare("SELECT deceasedNumber FROM death_audits.deceasedidinfo WHERE ((providerID = :providerID) AND (providerKey = :providerKey) AND (ID = :deceasedID))");
        query.bindValue(":providerID", QVariant(providerID));
        query.bindValue(":providerKey", QVariant(providerKey));
        query.bindValue(":deceasedID", QVariant(deceasedID));
    }

    success = query.exec();

    if (success && (query.size() > 0))
        result = true;

    return result;
}

dataRecord databaseSearches::readRecord(unsigned int deceasedID, QString lastName, GLOBALVARS *gv)
{
    // Load separated into two queries, one to "deceased" and one to "deceasedidinfo"

    dataRecord record;
    QSqlQuery query;
    QSqlError error;
    PQString errorMessage;
    bool success;

    QString tempString;
    QDate tempDate;
    int tempInt;
    unsigned int tempUInt;
    unsigned int currentYear = static_cast<unsigned int>(gv->today.year());

    record.clear();
    record.setGlobals(*gv);

    success = query.prepare("SELECT lastName, altLastName1, altLastName2, altLastName3, firstName, nameAKA1, nameAKA2, middlenames, "
                            "suffix, prefix, gender, DOB, DOD, YOB, YOD, ageAtDeath, minDOB, maxDOB, lastUpdated "
                            "FROM death_audits.deceased WHERE deceasedNumber = :deceasedNumber AND lastName = :lastName");

    query.bindValue(":deceasedNumber", QVariant(deceasedID));
    query.bindValue(":lastName", QVariant(lastName));

    success = query.exec();

    if (!success || (query.size() != 1))
    {
        error = query.lastError();
        errorMessage << QString("SQL problem trying to read DB record for ") << lastName << " - " << deceasedID << " in deceased database";
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }
    else
    {
        query.next();
        record.setFamilyName(PQString(query.value(0).toString()));

        tempString = query.value(1).toString();
        if (tempString.size() > 0)
            record.setFamilyName(PQString(tempString));

        tempString = query.value(2).toString();
        if (tempString.size() > 0)
            record.setFamilyName(PQString(tempString));

        tempString = query.value(3).toString();
        if (tempString.size() > 0)
            record.setFamilyName(PQString(tempString));

        tempString = query.value(4).toString();
        if (tempString.size() > 0)
            record.setFirstName(tempString, 1);

        tempString = query.value(5).toString();
        if (tempString.size() > 0)
            record.setFirstName(tempString, 2);

        tempString = query.value(6).toString();
        if (tempString.size() > 0)
            record.setFirstName(tempString, 3);

        tempString = query.value(7).toString();
        if (tempString.size() > 0)
            record.setMiddleNames(tempString);

        tempString = query.value(8).toString();
        if (tempString.size() > 0)
            record.setSuffix(PQString(tempString));

        tempString = query.value(9).toString();
        if (tempString.size() > 0)
            record.setPrefix(PQString(tempString));

        tempInt = query.value(10).toInt();
        record.setGender(static_cast<GENDER>(tempInt));

        tempDate = query.value(11).toDate();
        if (tempDate.isValid() && (tempDate != QDate(1,1,1)))
            record.setDOB(tempDate);

        tempDate = query.value(12).toDate();
        if (tempDate.isValid())
            record.setDOD(tempDate);

        tempUInt = query.value(13).toUInt();
        if ((tempUInt >= 1875) && (tempUInt <= currentYear))
            record.setYOB(tempUInt);

        tempUInt = query.value(14).toUInt();
        if ((tempUInt >= 1875) && (tempUInt <= currentYear))
            record.setYOD(tempUInt);

        tempUInt = query.value(15).toUInt();
        if ((tempUInt >= 0) && (tempUInt <= 130))
            record.setAgeAtDeath(tempUInt);

        tempDate = query.value(16).toDate();
        if (tempDate.isValid())
            record.setMinDOB(tempDate);

        tempDate = query.value(17).toDate();
        if (tempDate.isValid())
            record.setMaxDOB(tempDate);

        //record.setCycle(query.value(18).toUInt());

        tempDate = query.value(18).toDate();
        if (tempDate.isValid())
            record.setLastUpdated(tempDate);
    }

    query.clear();

    success = query.prepare("SELECT url, providerID, providerKey, ID, publishDate "
                            "FROM death_audits.deceasedidinfo "
                            "WHERE deceasedNumber = :deceasedNumber");

    query.bindValue(":deceasedNumber", QVariant(deceasedID));

    success = query.exec();

    if (!success || (query.size() == 0))
    {
        error = query.lastError();
        errorMessage << QString("SQL problem trying to read DB record for ") << lastName << " - " << deceasedID << " in deceased database";
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }
    else
    {
        // Load the first match if it exists
        query.next();

        record.setURL(query.value(0).toString());
        tempUInt = query.value(1).toUInt();
        record.setProvider(static_cast<PROVIDER>(tempUInt));
        record.setProviderKey(query.value(2).toUInt());
        record.setID(query.value(3).toString());
        tempDate = query.value(4).toDate();
        if (tempDate.isValid())
            record.setPublishDate(tempDate);
    }

    POSTALCODE_INFO pcInfo = getPostalCodeInfo(deceasedID, gv);
    record.setPostalCode(pcInfo);

    record.setDeceasedNumber(deceasedID);

    return record;
}

dataRecord databaseSearches::readMonitoredRecord(unsigned int memberID, QString lastName, GLOBALVARS *gv)
{
    dataRecord record;
    QSqlQuery query;
    QSqlError error;
    PQString errorMessage;
    bool success;

    QString tempString;
    QDate tempDate;
    int tempInt;

    record.clear();
    record.setGlobals(*gv);

    success = query.prepare("SELECT lastName, firstName, nameAKA1, nameAKA2, middlenames, gender, DOB, minDOB, maxDOB, DOD, deceasedNumber "
                            "FROM death_audits.deceased WHERE deceasedNumber = :memberID AND lastName = :lastName");

    /*// SELECT MAX(L.DOB), MIN(L.DOB), MAX(R.ID) FROM death_audits.deceased L INNER JOIN death_audits.deceasedidinfo R ON L.deceasedNumber = R.deceasedNumber WHERE R.providerID = 1038 AND R.providerKey = 5;
    success = query.prepare("SELECT L.lastName, L.firstName, L.nameAKA1, L.nameAKA2, L.middlenames, L.gender, L.DOB, L.minDOB, L.maxDOB, L.DOD, L.deceasedNumber, R.url "
                            "FROM death_audits.deceased L INNER JOIN death_audits.deceasedidinfo R ON L.deceasedNumber = R.deceasedNumber "
                            "WHERE L.deceasedNumber = :memberID AND L.lastName = :lastName");
    success = query.prepare("SELECT d.lastName, d.firstName, d.nameAKA1, d.nameAKA2, d.middlenames, d.gender, d.DOB, d.minDOB, d.maxDOB, d.DOD, d.deceasedNumber, ft.url FROM "
                            "death_audits.deceased d INNER JOIN "
                            "(SELECT dts.deceasedNumber, dts.ID FROM death_audits.deceasedidinfo dts WHERE dts.providerID = :providerID AND dts.providerKey = :providerKey AND dts.DOD = :DOD ORDER BY dts.deceasedNumber) "
                            "AS ft USING(deceasedNumber)");*/

    query.bindValue(":memberID", QVariant(memberID));
    query.bindValue(":lastName", QVariant(lastName));

    success = query.exec();

    if (!success || (query.size() != 1))
    {
        error = query.lastError();
        errorMessage << QString("SQL problem trying to read 'monitored' record for ") << lastName << " - " << memberID << " in monitored database";
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }
    else
    {
        query.next();
        record.setFamilyName(PQString(query.value(0).toString()));

        tempString = query.value(1).toString();
        if (tempString.size() > 0)
            record.setFirstName(tempString, 1);

        tempString = query.value(2).toString();
        if (tempString.size() > 0)
            record.setFirstName(tempString, 2);

        tempString = query.value(3).toString();
        if (tempString.size() > 0)
            record.setFirstName(tempString, 3);

        tempString = query.value(4).toString();
        if (tempString.size() > 0)
            record.setMiddleNames(tempString);

        tempInt = query.value(5).toInt();
        record.setGender(static_cast<GENDER>(tempInt));

        tempDate = query.value(6).toDate();
        if (tempDate.isValid() && (tempDate != QDate(1,1,1)))
            record.setDOB(tempDate);

        tempDate = query.value(7).toDate();
        if (tempDate.isValid() && (tempDate != QDate(1,1,1)))
            record.setMinDOB(tempDate);

        tempDate = query.value(8).toDate();
        if (tempDate.isValid() && (tempDate != QDate(1,1,1)))
            record.setMaxDOB(tempDate);

        tempDate = query.value(9).toDate();
        if (tempDate.isValid() && (tempDate != QDate(1,1,1)))
            record.setDOD(tempDate);

        record.setDeceasedNumber(query.value(10).toUInt());
    }

    return record;
}

double databaseSearches::genderLookup(dataRecord *dr, GLOBALVARS *globals)
{
    QList<QString> names;
    dr->pullUniqueGivenNames(names);

    return genderLookup(names, globals);
}

double databaseSearches::genderLookup(QList<QString> &listOfNames, GLOBALVARS *globals)
{
    double result, conviction, convictionShortfall, convictionTopUp;
    double probabilityWrongMale, probabilityWrongFemale, probGood;
    double firstWeight;  // Used when more than just first name exists

    NAMESTATS nameStats;

    // Order of names is first, middle, then AKA
    // Higher credibility accorded to earlier names
    removeLowConviction(listOfNames, globals);

    switch (listOfNames.size())
    {
    case 0:
        result = 0.5;
        probabilityWrongMale = 0.50;
        probabilityWrongFemale = 0.50;
        break;

    case 1:
        nameStatLookup(listOfNames.at(0), globals, nameStats);
        if ((nameStats.maleCount + nameStats.femaleCount) == 0)
            nameStats.malePct = 0.5;
        conviction = 0.5 - nameStats.malePct;
        switch (nameStats.credibility)
        {
        case zero:
            result = 0.5 - conviction * 0.000;
            break;

        case low:
            result = 0.5 - conviction * 0.800;
            break;

        case medium:
            result = 0.5 - conviction * 0.875;
            break;

        case high:
            result = 0.5 - conviction * 0.950;
            break;

        case veryHigh:
            result = 0.5 - conviction * 1.000;
            break;
        }

        probabilityWrongMale   = 1.0 - nameStats.malePct;
        probabilityWrongFemale = nameStats.malePct;

        break;  // case 1

    default:
        nameStatLookup(listOfNames.at(0), globals, nameStats);
        if ((nameStats.maleCount + nameStats.femaleCount) == 0)
            nameStats.malePct = 0.5;
        switch (nameStats.credibility)
        {
        case zero:
        case low:
        case medium:
            firstWeight = 0.70;
            break;

        case high:
            firstWeight = 0.80;
            break;

        case veryHigh:
            firstWeight = 0.90;
            break;
        }
        conviction = firstWeight * (0.5 - nameStats.malePct);
        probabilityWrongMale   = 1.0 - nameStats.malePct;
        probabilityWrongFemale = nameStats.malePct;

        for (int i = 1; i < listOfNames.size(); i++)
        {
            nameStatLookup(listOfNames.at(i), globals, nameStats);
            conviction += ((1.0 - firstWeight) / (listOfNames.size() - 1)) * (0.5 - nameStats.malePct);

            probabilityWrongMale   = probabilityWrongMale * (1.0 - nameStats.malePct);
            probabilityWrongFemale = probabilityWrongFemale * nameStats.malePct;

        }

        // Increase conviction if probability of a wrong guess is small as a result of multiple names
        if (conviction <= 0)
        {
            probGood = 1.0 - probabilityWrongMale;
            convictionShortfall = 1.0 + conviction / 0.50;
        }
        else
        {
            probGood = 1.0 - probabilityWrongFemale;
            convictionShortfall = 1.0 - conviction / 0.50;
        }
        convictionTopUp = (convictionShortfall * probGood * probGood) * 0.5;
        convictionTopUp = convictionTopUp * 0.5;    // Maximum improvement is half the shortfall
        if (conviction <= 0)
            conviction = conviction - convictionTopUp;
        else
            conviction = conviction + convictionTopUp;

        result = 0.5 - conviction * 1.000;

        break;  // case 2+
    }

    return result;
}

bool databaseSearches::updateLastObit(dataRecord &dr, GLOBALVARS *gv)
{
    QSqlQuery query;
    QSqlError error;
    PQString errorMessage, dateSQL;
    bool success;
    QDate firstObit, lastObit, recDOD;

    bool updated = false;
    recDOD = dr.getDOD();
    if (!recDOD.isValid() || (recDOD > gv->today))
        return updated;

    success = query.prepare("SELECT fhFirstObit, fhLastObit FROM death_audits.funeralhomedata "
                            "WHERE providerID = :providerID AND providerKey = :providerKey AND ((fhRunStatus = 1) OR (fhRunStatus = 2))");

    query.bindValue(":providerID", QVariant(dr.getProvider()));
    query.bindValue(":providerKey", QVariant(dr.getProviderKey()));

    success = query.exec();

    if (!success || (query.size() != 1))
    {
        error = query.lastError();
        errorMessage << QString("SQL problem trying to update fhLastObit for ") << dr.getProvider() << " - " << dr.getProviderKey() << " in deceased database";
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }
    else
    {
        query.next();
        firstObit = query.value(0).toDate();
        lastObit = query.value(1).toDate();

        if (recDOD > lastObit)
        {
            query.clear();
            dateSQL << recDOD.toString("yyyy/MM/dd") << QString(" 0:0:0");

            success = query.prepare("UPDATE death_audits.funeralhomedata SET fhLastObit = :fhLastObit "
                                    "WHERE providerID = :providerID AND providerKey = :providerKey AND ((fhRunStatus = 1) OR (fhRunStatus = 2))");

            query.bindValue(":fhLastObit", QVariant(dateSQL.getString()));
            query.bindValue(":providerID", QVariant(dr.getProvider()));
            query.bindValue(":providerKey", QVariant(dr.getProviderKey()));

            success = query.exec();
            updated = success;
        }

        if ((recDOD > QDate(1900,1,1)) && (recDOD < firstObit))
        {
            query.clear();
            dateSQL << recDOD.toString("yyyy/MM/dd") << QString(" 0:0:0");

            success = query.prepare("UPDATE death_audits.funeralhomedata SET fhFirstObit = :fhFirstObit "
                                    "WHERE providerID = :providerID AND providerKey = :providerKey AND ((fhRunStatus = 1) OR (fhRunStatus = 2))");

            query.bindValue(":fhFirstObit", QVariant(dateSQL.getString()));
            query.bindValue(":providerID", QVariant(dr.getProvider()));
            query.bindValue(":providerKey", QVariant(dr.getProviderKey()));

            success = query.exec();
            updated = success;
        }
    }

    return updated;
}

bool databaseSearches::savePostalCodeInfo(int deceasedNumber, int providerID, int providerKey, POSTALCODE_INFO &pc)
{
    QSqlQuery query, queryAdd, queryUpdate;
    QSqlError error;
    bool success;
    int existingID, existingKey;

    bool updated = false;

    success = query.prepare("SELECT providerID, providerKey FROM death_audits.deceasedlocation WHERE deceasedNumber = :deceasedNumber");

    query.bindValue(":deceasedNumber", QVariant(deceasedNumber));

    success = query.exec();

    if (success || (query.size() != 1))
    {
        if (query.size() == 0)
        {
            // New Record
            success = queryAdd.prepare("INSERT INTO death_audits.deceasedlocation (deceasedNumber, postalCode, city, province, prov, latitude, longitude, providerID, providerKey) "
                                       "VALUES (:deceasedNumber, :postalCode, :city, :province, :prov, :latitude, :longitude, :providerID, :providerKey)");

            queryAdd.bindValue(":deceasedNumber", QVariant(deceasedNumber));
            queryAdd.bindValue(":postalCode", QVariant(pc.getPostalCode()));
            queryAdd.bindValue(":city", QVariant(pc.getCity()));
            queryAdd.bindValue(":province", QVariant(pc.getProvince()));
            queryAdd.bindValue(":prov", QVariant(pc.getProv()));
            queryAdd.bindValue(":latitude", QVariant(pc.getLatitude()));
            queryAdd.bindValue(":longitude", QVariant(pc.getLongitude()));
            queryAdd.bindValue(":providerID", QVariant(providerID));
            queryAdd.bindValue(":providerKey", QVariant(providerKey));

            updated = queryAdd.exec();
        }
        else
        {
            // Determine if new postal code is expected to be more accurate
            // Funeral home postal codes are favoured over newspapers or online
            bool update = false;

            query.next();
            existingID = query.value(0).toInt();
            existingKey = query.value(1).toInt();
            if (existingID == 1021)
                existingID = 1;
            if (providerID > existingID)
                update = true;

            if (update)
            {
                success = queryUpdate.prepare("UPDATE death_audits.deceasedlocation "
                                              "SET postalCode = :postalCode, city = :city, province = :province, prov = :prov, "
                                              "    latitude = :latitude, longitude = :longitude, providerID = :providerID, providerKey = :providerKey "
                                              "WHERE deceasedNumber = :deceasedNumber");

                queryUpdate.bindValue(":deceasedNumber", QVariant(deceasedNumber));
                queryUpdate.bindValue(":postalCode", QVariant(pc.getPostalCode()));
                queryUpdate.bindValue(":city", QVariant(pc.getCity()));
                queryUpdate.bindValue(":province", QVariant(pc.getProvince()));
                queryUpdate.bindValue(":prov", QVariant(pc.getProv()));
                queryUpdate.bindValue(":latitude", QVariant(pc.getLatitude()));
                queryUpdate.bindValue(":longitude", QVariant(pc.getLongitude()));
                queryUpdate.bindValue(":providerID", QVariant(providerID));
                queryUpdate.bindValue(":providerKey", QVariant(providerKey));

                updated = queryUpdate.exec();
            }
        }
    }

    return updated;
}

POSTALCODE_INFO databaseSearches::getPostalCodeInfo(int deceasedNumber, GLOBALVARS *gv)
{
    QSqlQuery query;
    QSqlError error;
    PQString errorMessage;
    bool success;
    POSTALCODE_INFO result;
    PostalCodes pcDatabase;
    QString pc;

    success = query.prepare("SELECT postalCode, city, province, prov, latitude, longitude, providerID, providerKey FROM death_audits.deceasedlocation WHERE deceasedNumber = :deceasedNumber");

    query.bindValue(":deceasedNumber", QVariant(deceasedNumber));

    success = query.exec();

    if (!success || (query.size() != 1))
    {
        error = query.lastError();
        errorMessage << QString("Problem trying to retrieve postal code info for ") << deceasedNumber << " in deceasedlocation database";
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }
    else
    {
        query.next();
        pc = query.value(0).toString();
        result = pcDatabase.lookup(pc);
    }

    return result;
}
