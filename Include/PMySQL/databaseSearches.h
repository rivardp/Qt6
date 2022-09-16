// databaseSearches.h

#ifndef DATABASE_SEARCHES_FORWARD_H
#include "../Include/PMySQL/databaseSearchesForward.h"
#endif

#ifndef DATA_RECORD_FORWARD_H
#include "../UpdateFuneralHomes/Include/dataRecordForward.h"
#endif

#ifndef DATABASE_SEARCHES_H
#define DATABASE_SEARCHES_H

#include <QDate>
#include <QSqlQuery>
#include <QSqlError>

#include "../Include/Penser/PQString.h"
#include "../UpdateFuneralHomes/Include/dataStructure.h"
#include "../Internet/qWebStructures.h"
#include "../UpdateFuneralHomes/Include/globalVars.h"
#include "../PostalCodes/postalCodeInfo.h"
#include "../PostalCodes/postalcodes.h"

class databaseSearches
{
public:
    int  surnameLookup(const QString name, GLOBALVARS *gv);
    bool areUniquelySurnames(QList<OQString> &listOfNames, GLOBALVARS *gv, GENDER gender = Female);
    bool areAllNames(QList<OQString> &listOfNames, GLOBALVARS *gv);
    bool givenNameLookup(const QString name, GLOBALVARS *gv, GENDER gender = genderUnknown);
    bool givenNameLookup(const QString name, GLOBALVARS *gv, int &maleCount, int &femaleCount, GENDER gender = genderUnknown);
    bool nicknameLookup(const QString name, GLOBALVARS *gv);
    bool nickNameInList(const OQString name, QList<QString> &listOfNames, GLOBALVARS *gv);
    void nameStatLookup(QString name, GLOBALVARS *gv, NAMESTATS &nameStats, GENDER gender = genderUnknown, bool skipClear = false);
    void removeLowConviction(QList<QString> &nameList, GLOBALVARS *gv);
    bool validRecordExists(downloadOutputs &outputs, GLOBALVARS *gv);
    bool deceasedRecordExists(const unsigned int &providerID, const unsigned int &providerKey, const QString &deceasedID, const QString &url, const QDate &pubDate);

    QString IDforLastDeathPreceding(const QDate &DOD, const unsigned int providerID, const unsigned int providerKey, GLOBALVARS *gv);
    QDate getLastPubDate(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey);
    QString lookupPostalCode(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey);                // looks in postalcode hash
    QString pcLookup(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey, QString location);      // looks in funeralhomelocation database
    POSTALCODE_INFO pcLookupPlaces(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey, QString place);   // looks in postalcodeplaces, and then funeralhome data if multiples exist

    bool savePostalCodeInfo(int deceasedNumber, int providerID, int providerKey, POSTALCODE_INFO &pc);
    POSTALCODE_INFO getPostalCodeInfo(int deceasedNumber, GLOBALVARS *gv);

    // The functions below are all called by, or rely on, dataRecord members
    bool pureNickNameLookup(const QString name, GLOBALVARS *gv);
    double genderLookup(dataRecord *dr, GLOBALVARS *gv);
    double genderLookup(QList<QString> &listOfNames, GLOBALVARS *gv);
    dataRecord readRecord(unsigned int deceasedID, QString lastName, GLOBALVARS *gv);
    dataRecord readMonitoredRecord(unsigned int deceasedID, QString lastName, GLOBALVARS *gv);
    QList<QString> getURLlist(unsigned int deceasedID, GLOBALVARS *gv);
    bool updateLastObit(dataRecord &dr, GLOBALVARS *gv);

private:

};

#endif

#ifndef DATA_RECORD_H
#include "../UpdateFuneralHomes/Include/dataRecord.h"
#endif
