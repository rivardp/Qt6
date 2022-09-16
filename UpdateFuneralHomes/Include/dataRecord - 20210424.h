// dataRecord.h
//

#ifndef DATA_RECORD_H
#define DATA_RECORD_H

#include <QDate>
#include <QList>

#include "../UpdateFuneralHomes/Include/dataStructure.h"
#include "../Include/PMySQL/databaseSearches.h"
#include "../Include/Actuary/PQDate.h"
#include "../Include/Penser/OQStream.h"
#include "../UpdateFuneralHomes/Include/globalVars.h"
#include "../UpdateFuneralHomes/Include/tableLookup.h"
#include "../UpdateFuneralHomes/Include/providers.h"

bool pureNickNameLookup(const QString name, GLOBALVARS *gv);

class dataRecord
{

public:
	dataRecord();
	~dataRecord();

    void setFamilyName(const PQString &name);
    void setFamilyNames(QList<OQString> &nameList);
    void setFirstName(const PQString &name, const unsigned int position = 0);
    void setFirstNames(const PQString &name, const bool forceLoad = false);
    void setMiddleNames(const PQString &name);
    void setMiddleNameUsedAsFirstName(const PQString &name);
    void setParentsLastName(const PQString &pln);
    void setMaidenNames(const PQString &mn);
    void addMaidenName(const PQString &mn);
    void setPrefix(const PQString &pfx);
    void setSuffix(const PQString &sfx);
    void setGender(const GENDER sex, const bool forceOverride = false);
    void setWorkingGender(const GENDER sex);
    void setDOB(const QDate &dob, const bool forceOverride = false, const bool fullyCredible = false);
    void setMinDOB(const QDate &dob);
    void setMaxDOB(const QDate &dob);
    void setMinMaxDOB();
    void setDOD(const QDate &dod, const bool forceOverride = false, const bool fullyCredible = false);
    void setDOBandDOD(const DATES &dates, const bool forceOverride = false);
    void setURL(const PQString &url);
    void setAltURL(const PQString &url);
    void setAltURL2(const PQString &url);
    void setLanguage(const LANGUAGE lang);
    void setAlternates(const QList<NAMEINFO> &nameInfoList, bool bestOf = false);
    void setAlternates(const NAMEINFO &nameInfo, bool bestOf = false);
    void setDate(const QDate &day);
    void setYOB(const unsigned int yob, const bool forceOverride = false, const bool fullyCredible = false);
    void setYOD(const unsigned int yod, const bool forceOverride = false, const bool fullyCredible = false);
    void setAgeAtDeath(const unsigned int num, const bool fullyCredible = false, const bool override = false);
    void setProvider(const PROVIDER &pvdr);
    void setAltProvider(const PROVIDER &pvdr);
    void setAltProvider2(const PROVIDER &pvdr);
    void setProviderKey(const unsigned int pk);
    void setAltProviderKey(const unsigned int pk);
    void setAltProviderKey2(const unsigned int pk);
    void setDeceasedNumber(const unsigned int dn);
    void setCycle(const unsigned int num);
    void setUnmatched(const unsigned int num);
    void setPriorUnmatched(const unsigned int num);
    void setPreviouslyLoaded(const unsigned int num);
    void setTitle(const PQString &title);
    void setTitleKey(const PQString &titleKey);
    void setID(const PQString &id);
    void setAltID(const PQString &id);
    void setAltID2(const PQString &id);
    void setFirstLoaded(const QDate & date);
    void setLastUpdated(const QDate &date);
    void setNeeEtAlEncountered(const bool flag);
    void setAgeNextReference(const bool flag);
    void setMaleHypentated(const bool flag);
    void setSingleYear(const unsigned int year);
    void setPotentialFirstName(const QString name);
    void setSpouseName(const QString name);
    void storeContent(PQString *content, unsigned int fieldType);
    void setPublishDate(const QDate &date);
    void setAltPublishDate(const QDate &date);
    void setAltPublishDate2(const QDate &date);
    void setDOS(const QDate &date);
    void setCommentDate(const QDate &date);
    void setDatesLocked(bool dl);
    void setProv(PROVINCE prov);
    void attemptToMakeRoomFor(const PQString &name);

    void clear();
    void clearDOB();
    void clearLastNames();
    void clearFirstNames();
    void removeUnnecessaryInitials();
    void standardizeBadRecord();
    int runDateValidations();

    void createFirstNameList(QList<QString> &resultList) const;
    void createMiddleNameList(QList<QString> &resultList) const;
    void sortFirstNames();

    QString  getURL() const;
    QString  getAltURL() const;
    QString  getAltURL2() const;
    GENDER   getGender() const;
    GENDER   getWorkingGender() const;
    LANGUAGE getLanguage() const;
    PQString getFirstName() const;
    PQString getFirstNameAKA1() const;
    PQString getFirstNameAKA2() const;
    PQString getMiddleNames() const;
    PQString getMiddleNameUsedAsFirstName() const;
    PQString getParentsLastName() const;
    QList<PQString> getMaidenNames() const;
    QList<QString> getFirstNameList() const;
    QList<QString> getGivenNameList(GLOBALVARS *gv) const;
    PQString getLastName() const;
    PQString getLastNameAlt1() const;
    PQString getLastNameAlt2() const;
    PQString getLastNameAlt3() const;
    PQString getFullName() const;
    PQString getPrefix() const;
    PQString getSuffix() const;
    QDate getDOB() const;
    QDate getMinDOB() const;
    QDate getMaxDOB() const;
    QDate getDOD() const;
	unsigned int getYOB() const;
	unsigned int getYOD() const;
    int getDeemedYOD();
    unsigned int getAgeAtDeath() const;
    bool getDOBcredibility() const;
    bool getDODcredibility() const;
    bool getYOBcredibility() const;
    bool getYODcredibility() const;
    bool getAgeAtDeathCredibility() const;
    PROVINCE getProv() const;
    PROVIDER getProvider() const;
    PROVIDER getAltProvider() const;
    PROVIDER getAltProvider2() const;
    unsigned int getProviderKey() const;
    unsigned int getAltProviderKey() const;
    unsigned int getAltProviderKey2() const;
    PQString getID() const;
    PQString getAltID() const;
    PQString getAltID2() const;
    QDate getPublishDate() const;
    QDate getAltPublishDate() const;
    QDate getAltPublishDate2() const;
    QDate getDOS() const;
    QDate getCommentDate() const;
    QDate getFirstRecorded() const;
    unsigned int getDeceasedNumber() const;
    unsigned int getCycle() const;
    unsigned int getPreviouslyLoaded() const;
    NAMESKNOWN getNamesKnown() const;
    unsigned int getNumFamilyNames() const;
    unsigned int getNumFirstNames() const;
    bool getMiddleNameList(QList<QString> &mnl) const;
    PQString getTitle() const;
    PQString getTitleKey() const;
    bool getNeeEtAlEncountered() const;
    bool getAgeNextReference() const;
    bool getMaleHyphenated() const;
    unsigned int getSingleYear() const;
    QString getPotentialFirstName() const;
    QString getSpouseName() const;
    bool getDatesLocked() const;

    void xport(QString filename, int extraOptParam = -999);
    bool checkNames();
    void simplifyInitials(PQString &word);
    void adjustHyphenatedLastNames();
    void addSingleNamesToList(QList<PQString> &list, PQString fullName);
    void pullGivenNames(QList<QString> &names) const;
    void pullUniqueGivenNames(QList<QString> &names) const;
    void setGlobals(GLOBALVARS &gv);
    void incorporateParentsLastName();
    void removeFromLastNames(PQString &name);
    void removeFromMiddleNames(PQString &name);
    void removeFromFirstNames(PQString &name);
    void updateToBestOf(dataRecord &newSource);

    bool isAFirstName(const PQString &name) const;
    bool isALastName(const PQString &name) const;
    bool isAMiddleName(const PQString &name) const;
    bool isASuffix(const PQString &name) const;
    bool isAPrefix(const PQString &name) const;
    bool isAnInitial(const PQString &letter) const;

    bool hasFirstNameInitial() const;
    bool hasMiddleNameInitial() const;
    bool replaceFirstNameInitial(const PQString &name);
    bool replaceMiddleNameInitial(const PQString &name);

    NAMETYPE isAName(const PQString &name);
    NAMETYPE isASavedName(const PQString &name);
    bool isANickName(const OQString &name);
    bool isAFormalName(const OQString &name);
    bool isANameVersion(const OQString &name, const bool expandedCheck = false);
    bool isASimilarName(const OQString &name);
    unsigned int getNumSavedNames() const;
	bool missingDOB() const;
	bool missingDOD() const;

    dataRecord& operator= (const dataRecord &rhs);

    GLOBALVARS globals;
    WARNINGINFO wi;

private:
    PQString familyName;
    PQString familyNameAlt1;
    PQString familyNameAlt2;
    PQString familyNameAlt3;
    PQString firstName;
    PQString firstNameAKA1;
    PQString firstNameAKA2;
    PQString middleNames;
    PQString middleNameUsedAsFirstName;
    PQString parentsLastName;
    QList<PQString> maidenNames;
    PQString suffix;
    PQString prefix;
	GENDER  gender;
    GENDER  workingGender;
    QDate   DOB;
    QDate   DOD;
	unsigned int YOB;
	unsigned int YOD;
    int deemedYOD;
	unsigned int ageAtDeath;
    QDate minDOB;
    QDate maxDOB;
    bool DOBfullyCredible;
    bool DODfullyCredible;
    bool YOBfullyCredible;
    bool YODfullyCredible;
    bool ageAtDeathFullyCredible;
    bool datesLocked;
    PROVINCE prov;
    PQString URL;
    PQString altURL;
    PQString altURL2;
    QDate     firstRecorded;
    QDate     lastUpdated;
	LANGUAGE  language;
    PROVIDER  provider;
    unsigned int providerKey;
    PROVIDER  altProvider;
    unsigned int altProviderKey;
    PROVIDER  altProvider2;
    unsigned int altProviderKey2;
    PQString ID;
    PQString altID;
    PQString altID2;
    QDate publishDate;
    QDate altPublishDate;
    QDate altPublishDate2;
    QDate DOS;
    QDate commentDate;
    unsigned int deceasedNumber;    // Only used to update existing records
    unsigned int cycle;             // Only used to update existing records
    unsigned int numUnmatched;      // Only used to export zeros as final column
    unsigned int priorUnmatched;    // Only used for reconciliation checking
    unsigned int previouslyLoaded;  // 0 =  no, 1 = Yes, 999 = Skip

    PQString title;
    PQString titleKey;

    bool neeEtAlEncountered;
    bool ageNextReference;
    bool maleHyphenated;
    unsigned int singleYear;
    QString potentialGivenName;
    QString spouseName;

};

class SOURCEID
{
public:
    PROVIDER provider;
    unsigned int providerKey;
    PQString URL;
    PQString ID;
    QDate publishDate;

    bool operator ==(SOURCEID const& newSource) const;
    SOURCEID& operator =(SOURCEID newSource);
};

#endif
