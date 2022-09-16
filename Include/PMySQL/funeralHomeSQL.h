#ifndef FUNERALHOMESQL_H
#define FUNERALHOMESQL_H

#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QString>
#include <QDate>
#include <QDebug>

#include "../Penser/PQString.h"
#include "../UpdateFuneralHomes/Include/globalVars.h"
#include "../UpdateFuneralHomes/Include/dataStructure.h"
#include "pMySQL.h"

// TABLE NAME:  funeralHomeData
// PRIMARY KEY: providedID, fhID
// COLUMNS
//  - fhID as INT NOT NULL
//  - providerID as INT NOT NULL
//  - fhName VARCHAR(100) NOT NULL
//  - fhProvince VARCHAR(25) NOT NULL
//  - fhSearchCode as INT
//  - fhRunStatus as INT  (1=Normal, 2=First time)
//  - fhURL as VARCHAR(50)
//  - fhLastUpdate as DATE
//  - fhLastRun as DATE

class fhRecordStructure
{
public:
    // The record elements
    unsigned int listerID;
    unsigned int listerKey;
    PROVIDER providerID;
    unsigned int providerKey;
    QString fhName;
    QString fhCity;
    QString fhProvince;
    QString fhPostalCode;
    QString fhHTTP;
    QString fhWWW;
    QString fhURL;
    QString fhURLforUpdate;
    unsigned int fhSearchCode;
    unsigned int fhRunStatus;
    QDate fhLastUpdate;
    QDate fhLastRun;
    unsigned int fhSpecialCode;

    // Methods
    void clear();
    bool lookupFHID(unsigned int provider);
    bool addRecordToFH();
    bool compareRecordToFH();
    void setGlobalVars(GLOBALVARS *gv);

private:
    GLOBALVARS *globals;
};

unsigned int lookupFHspecialCode(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey);
void setRunTypes(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey, int runTypes);
int readRunTypes(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey);
bool getIDandKey(GLOBALVARS *gv, const QString name, unsigned int &providerID, unsigned int &providerKey);
QDate getLastUpdateDate(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey);


#endif // FUNERALHOMESQL_H

