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

#include "../UpdateFuneralHomes/Include/dataStructure.h"
#include "../Internet/qWebStructures.h"
#include "../UpdateFuneralHomes/Include/globalVars.h"
#include "../PostalCodes/postalCodeInfo.h"
#include "../PayAlertEnterprise/groupConfig.h"

class databaseSearches
{
public:
    int  surnameLookup(const QString name, GLOBALVARS *gv) const;
    bool areUniquelySurnames(QList<QString> listOfNames, GLOBALVARS *gv, GENDER gender = Female) const;
    bool areAllNames(QList<QString> listOfNames, GLOBALVARS *gv) const;
    bool areAllLocations(QList<QString> listOfNames, GLOBALVARS *gv) const;
    bool givenNameLookup(const QString name, GLOBALVARS *gv, GENDER gender = genderUnknown) const;
    bool givenNameLookup(const QString name, GLOBALVARS *gv, int &maleCount, int &femaleCount, GENDER gender = genderUnknown) const;
    bool nicknameLookup(const QString name, GLOBALVARS *gv) const;
    void nameStatLookup(QString name, GLOBALVARS *gv, NAMESTATS &nameStats, GENDER gender = genderUnknown, bool skipClear = false, bool corrected = true) const;
    void removeLowConviction(QList<QString> &nameList, GLOBALVARS *gv) const;
    bool validRecordExists(downloadOutputs &outputs, GLOBALVARS *gv) const;
    bool deceasedRecordExists(const unsigned int &providerID, const unsigned int &providerKey, const QString &deceasedID, const QString &url, const QDate &pubDate) const;

    QString IDforLastDeathPreceding(const QDate &DOD, const unsigned int providerID, const unsigned int providerKey, GLOBALVARS *gv) const;
    QDate getLastPubDate(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey) const;
    QString lookupPostalCode(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey) const;                // looks in postalcode database
    QString pcLookup(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey, QString location) const;      // looks in funeralhomelocation database
    POSTALCODE_INFO pcLookupPlaces(GLOBALVARS *gv, PROVIDER provID, unsigned int provKey, QString place) const;   // looks in postalcodeplaces, and then funeralhome data if multiples exist

    bool fillInPostalCodeInfo(GLOBALVARS *gv, POSTALCODE_INFO &pci, QString pc) const;
    bool savePostalCodeInfo(int deceasedNumber, int providerID, int providerKey, POSTALCODE_INFO &pc) const;
    POSTALCODE_INFO getPostalCodeInfo(int deceasedNumber, GLOBALVARS *gv) const;

    // The functions below are all called by, or rely on, dataRecord members
    bool pureNickNameLookup(const QString name, GLOBALVARS *gv) const;
    double genderLookup(dataRecord *dr, GLOBALVARS *gv) const;
    double genderLookup(QList<QString> &listOfNames, GLOBALVARS *gv) const;
    dataRecord readRecord(unsigned int deceasedID, QString lastName, GLOBALVARS *gv) const;
    dataRecord readMonitoredRecord(unsigned int deceasedID, QString lastName, GLOBALVARS *gv) const;
    QList<QString> getURLlist(unsigned int deceasedID, GLOBALVARS *gv) const;
    bool updateLastObit(dataRecord &dr, GLOBALVARS *gv) const;

    QList<GROUPCONFIG> getAllGroupConfigs(unsigned int clientCode) const;

private:

};

#endif

#ifndef DATA_RECORD_H
#include "../UpdateFuneralHomes/Include/dataRecord.h"
#endif
