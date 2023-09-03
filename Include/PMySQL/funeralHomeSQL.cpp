#include "funeralHomeSQL.h"

void fhRecordStructure::clear()
{
    QDate qNullDate;

    listerID = 0;
    listerKey = 0;
    providerID = Undefined;
    providerKey = 0;
    fhName.clear();
    fhCity.clear();
    fhProvince.clear();
    fhPostalCode.clear();
    fhHTTP.clear();
    fhWWW.clear();
    fhURL.clear();
    fhURLforUpdate.clear();
    fhSearchCode = 0;
    fhRunStatus = 0;
    fhLastUpdate = qNullDate;
    fhLastRun = qNullDate;
    fhSpecialCode = 0;
}

bool fhRecordStructure::lookupFHID(unsigned int providerID)
{
    bool result;

    QSqlQuery query;
    bool success;

    success = query.prepare("SELECT providerKey FROM death_audits.funeralHomeData WHERE providerID = :providerID AND providerKey = :providerKey");
    if (!success)
        qDebug() << "Invalid query";

    query.bindValue(":providerID", QVariant(providerID));
    query.bindValue(":providerKey", QVariant(providerKey));
    success = query.isValid();
    success = query.exec();
    if (query.size() != 1)
        result = false;
    else
        result = true;

    return result;

}

bool fhRecordStructure::addRecordToFH()
{
    bool success;

    QSqlQuery query;

    query.prepare("INSERT INTO death_audits.funeralHomeData(providerID, providerKey, fhName, fhCity, fhProvince, fhPostalCode, fhRunStatus, fhURL, fhLastUpdate) "
                  "VALUES (:providerID, :providerKey, :funeralHomeName, :funeralHomeCity, :funeralHomeProvince, :funeralHomePostalCode, :funeralHomeRunStatus, "
                          ":funeralHomeHTTP, :funeralHomeWWW, :funeralHomeURL, :funeralHomeLastUpdate)");
    query.bindValue(":providerID", QVariant(providerID));
    query.bindValue(":providerKey", QVariant(providerKey));
    query.bindValue(":funeralHomeName", QVariant(fhName));
    query.bindValue(":funeralHomeCity", QVariant(fhCity));
    query.bindValue(":funeralHomeProvince", QVariant(fhProvince));
    query.bindValue(":funeralHomePostalCode", QVariant(fhPostalCode));
    query.bindValue(":funeralHomeRunStatus", QVariant(fhRunStatus));
    query.bindValue(":funeralHomeHTTP", QVariant(fhHTTP));
    query.bindValue(":funeralHomeWWW", QVariant(fhWWW));
    query.bindValue(":funeralHomeURL", QVariant(fhURL));
    query.bindValue(":funeralHomeLastUpdate", QVariant(globals->todaySQL.getString()));
    success = query.exec();

    if(!success)
    {
        QSqlError error = query.lastError();
        PQString errorMessage;
        errorMessage << QString("Malformed INSERT query for new funeral home ID: ") << providerKey << QString(" - ");
        errorMessage << getLastExecutedQuery(query);
        globals->logMsg(ErrorSQL, errorMessage, error.type());
    }

    return success;
}

bool fhRecordStructure::compareRecordToFH()
{
    bool success;
    bool changes = false;
    QString tempString;

    QSqlQuery query;

    query.prepare("SELECT fhName, fhCity, fhProvince, fhPostalCode, fhHTTP, fhWWW, fhURL FROM death_audits.funeralHomeData \
                   WHERE providerID = :providerID AND providerKey = :providerKey");
    query.bindValue(":provider", providerID);
    query.bindValue(":providerKey", providerKey);
    success = query.exec();
    query.next();

    if (query.size() != 1)
    {
        PQString errorMessage;
        errorMessage << QString("Comparing of funeral home data failed: ") << providerID << " - " << providerKey;
        errorMessage << QString(" - ") << fhName;
        globals->logMsg(ErrorSQL, errorMessage);
    }
    else
    {
        tempString = query.value(0).toString();
        if (tempString != fhName)
        {
            changes = true;
            PQString errorMessage;
            errorMessage << QString("Change in funeral home NAME for: ") << providerKey;
            errorMessage << QString(" - From ") << tempString << QString(" to ") << fhName;
            globals->logMsg(ActionRequired, errorMessage);
        }

        tempString = query.value(1).toString();
        if (tempString != fhCity)
        {
            changes = true;
            PQString errorMessage;
            errorMessage << QString("Change in funeral home CITY for: ") << providerKey;
            errorMessage << QString(" - From ") << tempString << QString(" to ") << fhCity;
            globals->logMsg(ActionRequired, errorMessage);
        }

        tempString = query.value(2).toString();
        if (tempString != fhProvince)
        {
            changes = true;
            PQString errorMessage;
            errorMessage << QString("Change in funeral home PROVINCE for: ") << providerKey;
            errorMessage << QString(" - From ") << tempString << QString(" to ") << fhProvince;
            globals->logMsg(ActionRequired, errorMessage);
        }

        tempString = query.value(3).toString();
        if (tempString != fhPostalCode)
        {
            changes = true;
            PQString errorMessage;
            errorMessage << QString("Change in funeral home PostalCode for: ") << providerKey;
            errorMessage << QString(" - From ") << tempString << QString(" to ") << fhURL;
            globals->logMsg(ActionRequired, errorMessage);
        }

        tempString = query.value(4).toString();
        if (tempString != fhHTTP)
        {
            changes = true;
            PQString errorMessage;
            errorMessage << QString("Change in funeral home HTTP for: ") << providerKey;
            errorMessage << QString(" - From ") << tempString << QString(" to ") << fhHTTP;
            globals->logMsg(ActionRequired, errorMessage);
        }

        tempString = query.value(5).toString();
        if (tempString != fhWWW)
        {
            changes = true;
            PQString errorMessage;
            errorMessage << QString("Change in funeral home WWW for: ") << providerKey;
            errorMessage << QString(" - From ") << tempString << QString(" to ") << fhWWW;
            globals->logMsg(ActionRequired, errorMessage);
        }

        tempString = query.value(6).toString();
        if (tempString != fhURL)
        {
            changes = true;
            PQString errorMessage;
            errorMessage << QString("Change in funeral home URL for: ") << providerKey;
            errorMessage << QString(" - From ") << tempString << QString(" to ") << fhURL;
            globals->logMsg(ActionRequired, errorMessage);
        }
    }

    // If everything matches, update the lastUpdateDate
    if (!changes)
    {
        success = query.prepare("UPDATE death_audits.funeralHomeData SET fhLastUpdate = :funeralHomeLastUpdate WHERE providerID = :providerID AND providerKey = :providerKey");
        query.bindValue(":providerID", QVariant(providerID));
        query.bindValue(":providerKey", QVariant(providerKey));
        query.bindValue(":funeralHomeLastUpdate", QVariant(globals->todaySQL.getString()));
        success = query.exec();

        if (!success)
        {
//            QString errorMsg = query.lastError().databaseText() + " - " + query.lastError().driverText();
            QSqlError error = query.lastError();
            PQString errorMessage;
            errorMessage << QString("Updating of fhLastDate failed for: ") << providerKey;
            errorMessage << QString(" - ") << fhName;
            globals->logMsg(ErrorSQL, errorMessage, error.type());
        }
    }

    return changes;
}

void fhRecordStructure::setGlobalVars(GLOBALVARS *gv)
{
    globals = gv;
}

unsigned int lookupFHspecialCode(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey)
{
    QSqlQuery query;
    QSqlError error;

    bool success;
    unsigned int result = 0;

    query.prepare("SELECT fhSpecialCode from death_audits.funeralhomedata WHERE providerID = :providerID AND providerKey = :providerKey AND "
                                                                        "(fhRunStatus = 1 OR fhRunStatus = 2 OR fhRunStatus = 100 OR fhRunStatus = 101)");
    query.bindValue(":providerID", QVariant(provID));
    query.bindValue(":providerKey", QVariant(provKey));
    success = query.exec();
    if (success && (query.size() == 1))
    {
        query.next();
        result = query.value(0).toUInt();
    }
    else
    {
        error = query.lastError();
        PQString errorMessage;
        errorMessage << QString("SQL problem retrieving fhURL from :") << provID << " - " << provKey;
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }

    query.clear();

    return result;
}

void setRunTypes(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey, int runTypes)
{
    QSqlQuery query;
    QSqlError error;

    bool success;

    query.prepare("UPDATE death_audits.funeralhomedata SET fhRunTypes = :runTypes WHERE providerID = :providerID AND providerKey = :providerKey");
    query.bindValue(":providerID", QVariant(provID));
    query.bindValue(":providerKey", QVariant(provKey));
    query.bindValue(":runTypes", QVariant(runTypes));
    success = query.exec();
    if (!success)
    {
        error = query.lastError();
        PQString errorMessage;
        errorMessage << QString("SQL problem setting fhRunType for:") << provID << " - " << provKey;
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }

    query.clear();
}

int readRunTypes(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey)
{
    QSqlQuery query;
    QSqlError error;

    bool success;
    unsigned int result = 0;

    success = query.prepare("SELECT fhRunTypes from death_audits.funeralhomedata WHERE providerID = :providerID AND providerKey = :providerKey AND "
                                                                        "(fhRunStatus = 1 OR fhRunStatus = 2 OR fhRunStatus = 100 OR fhRunStatus = 101)");
    query.bindValue(":providerID", QVariant(provID));
    query.bindValue(":providerKey", QVariant(provKey));
    success = query.exec();
    if (success && (query.size() == 1))
    {
        query.next();
        result = query.value(0).toInt();
    }
    else
    {
        error = query.lastError();
        PQString errorMessage;
        errorMessage << QString("SQL problem retrieving fhRunType from:") << provID << " - " << provKey;
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }

    query.clear();

    return result;
}

QDate getLastUpdateDate(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey)
{
    QSqlQuery query;
    QSqlError error;

    bool success;
    QDate result;

    success = query.prepare("SELECT fhLastRun from death_audits.funeralhomedata WHERE providerID = :providerID AND providerKey = :providerKey AND "
                                                                        "(fhRunStatus = 1 OR fhRunStatus = 2 OR fhRunStatus = 100 OR fhRunStatus = 101)");
    query.bindValue(":providerID", QVariant(provID));
    query.bindValue(":providerKey", QVariant(provKey));
    success = query.exec();
    if (success && (query.size() == 1))
    {
        query.next();
        result = query.value(0).toDate();
    }
    else
    {
        error = query.lastError();
        PQString errorMessage;
        errorMessage << QString("SQL problem retrieving fhRunType from:") << provID << " - " << provKey;
        gv->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }

    query.clear();
    if (!result.isValid())
        result = QDate(1900,1,1);

    return result;
}


bool getIDandKey(GLOBALVARS *gv, const QString name, unsigned int &providerID, unsigned int &providerKey)
{
    QSqlQuery query;
    QSqlError error;

    bool success;

    success = query.prepare("SELECT providerID, providerKey from death_audits.funeralhomedata WHERE fhName = :fhName AND (fhRunStatus = 1 OR fhRunStatus = 2 OR fhRunStatus = 100)");
    query.bindValue(":fhName", QVariant(name));

    success = query.exec();
    if (success && (query.size() == 1))
    {
        query.next();
        providerID = query.value(0).toUInt();
        providerKey = query.value(1).toUInt();
    }
    else
    {
        success = false;
        error = query.lastError();
        PQString errorMessage;
        errorMessage << QString("Unable to find unique match for fhName = ") << name;
        gv->logMsg(ErrorRunTime, errorMessage, static_cast<int>(error.type()));
    }

    query.clear();

    return success;
}




