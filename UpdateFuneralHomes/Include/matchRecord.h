// matchRecord.h

#ifndef MATCHRECORD_H
#define MATCHRECORD_H

#include <QDate>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include "../Include//Penser/OQStream.h"
#include "../UpdateFuneralHomes/Include/dataStructure.h"
#include "../UpdateFuneralHomes/Include/dataRecord.h"
#include "../Include/PMySQL/databaseSearches.h"
//#include "../Include/Penser/QtUtilities.h"
#include "../PostalCodes/postalCodeInfo.h"

#define mPREFIX         1
#define mSUFFIX         2
#define mMIDDLENAMES    4
#define mMINDOB         8
#define mMAXDOB         16
#define mYOB            32
#define mYOD            64
#define mGENDER         128
#define mFIRSTNAME      256
#define mFNAMEAKA2      512
#define mFNAMEAKA1      1024
#define mFNAME          2048
#define mDOD            4096
#define mDOB            8192
#define mSNAME          16384
#define mLNAMEALT3      32768
#define mLNAMEALT2      65536
#define mLNAMEALT1      131072
#define mLNAME          262144
#define cDOB            524288
#define mAGEDEATH       1048576
#define mSPOUSENAME     2097152
#define mPC             4194304

#define inconsistencyCost 2


//                         DOB dob DOD LastName AltLastName FirstName AltFirstName
// mkKeyDOD                 X       X     X                     X
// mkAltKeyDOD              X       X                X          X
// mkKeyDOBDODnameAlt       X       X     X                                X
// mkAltKeyDOBDODnameAlt    X       X                X                     X
// mkKey                    X             X                     X
// mkAltKey                 X                        X          X
// mkDODdobName                 X   X     X                     X
// mkDODdobAltName              X   X                X          X

// mkDODname                        X     X                     X
// mkDODaltName                     X                X          X
// mkDOBnameAlt             X             X                                X
// mkDOBaltNameAlt          X                        X                     X
// mKDODnameAlt                     X     X                                X
// mkDODaltNameAlt                  X                X                     X

// mkCdobName                   X         X                     X
// mkCdobAltName                X                    X          X
// mkNoKey                                X                     X
// mkNoKeyAltName                                    X          X
// mkNoKeyAlt                             X                                X
// mkNoKeyAltNameAlt                                 X                     X


enum MATCHKEY {mkNone = 0,
               mkNoKeyAltNameAlt, mkNoKeyAlt, mkNoKeyAltName, mkNoKey, mkCdobAltName, mkCdobName,
               mkDODaltNameAlt, mkDODnameAlt, mkDOBaltNameAlt, mkDOBnameAlt, mkDODaltName, mkDODname,
               mkDODdobAltName, mkDODdobName, mkAltKey, mkKey, mkAltKeyDOBDODnameAlt, mkKeyDOBDODnameAlt, mkAltKeyDOD, mkKeyDOD,
               mkID};

static QList<QString> MatchTypes = QList<QString> () << "No match"
                                                     << "NoKeyAltNameAlt match" << "NoKeyAlt Match" << "NoKeyAltName match" << "NoKey match" << "CdobName match" << "CdobAltName match"
                                                     << "DODaltNameAlt match" << "DODnameAlt match" << "DOBaltNameAlt match" << "DOBnameAlt match" << "DODaltName match" << "DODname match"
                                                     << "DODdobAltName match" << "DODdobName match" << "AltKey match" << "Key match"
                                                     << "AltKeyDOBDODnameAlt match" << "KeyDOBDODnameAlt match" << "AltKeyDOD match" << "KeyDOD match" << "KeyID match";

class MATCHRECORD
{
public:

    MATCHRECORD();

    unsigned int recordID;
    QString recordLastName;
    unsigned int dataCount;
    int score;
    int netScore;
    unsigned int inconsistencyCount;
    int inconsistencyScore;
    int rankScore;
    bool newInfoAvailable;
    bool sufficientlyMatched;
    bool matchedFHinfo;
    bool isAnalyzed;
    bool sameDataProvided;
    bool sufficientNameMatch;
    bool closeDOBrange;
    int potentialNumDataMatches;
    int numNameMatches;
    double pcDistance;
    enum MATCHKEY matchKey;

    bool matchedLNAME;
    bool matchedLNAMEALT;
    bool matchedDOB;
    bool matchedFNAME;
    bool matchedFIRSTNAME;
    bool matchedSNAME;
    bool matchedDOD;
    bool matchedGENDER;
    bool matchedDODinfo;
    bool matchedDOBinfo;
    bool matchedPREFIX;
    bool matchedSUFFIX;
    bool matchedYOB;
    bool matchedYOD;
    bool matchedAGEDEATH;
    bool matchedIDnumber;
    bool matchedSPOUSE;

    bool consistentDOB;
    bool inconsistentDOB;
    bool inconsistentDOD;
    bool inconsistentAgeAtDeath;
    bool inconsistentSpouseName;
    bool inconsistentLNAME;
    bool consistentPC;
    bool inconsistentPC;

    bool newRecord;

    void clear();
    void setID(unsigned int id);
    void setLastName(QString lastName);
    void setMatchKey(MATCHKEY mk);
    void setSameFH(bool matched);
    void setSufficientlyMatched(bool sufficient);
    MATCHKEY getMatchKey() const;
    bool isNewRecord() const;
    bool hasNoMajorInconsistencies() const;
    void addToScore(int points);
    void addToInconsistencyScore(int points);
    unsigned int countItems(int score);
    unsigned int countDataItems(int score);
    void analyze();

    void compareGenders(unsigned int recGender, unsigned int DBgender);
    void compareLastNames(QStringList &recLastNameList, QStringList &DBlastNameList);
    void compareFirstNames(QStringList &recFirstNameList, QStringList &DBfirstNameList);
    void compareMiddleNames(QStringList &recMiddleNameList, QStringList &DBmiddleNameList);
    void compareDOBinfo(DATEINFO &recDateInfo, DATEINFO &DBdateInfo);
    void compareDODinfo(DATEINFO &recDateInfo, DATEINFO &DBdateInfo);
    void compareDOBDODinfo(DATEINFO &recDOBInfo, DATEINFO &DBDOBInfo, DATEINFO &recDODInfo, DATEINFO &DBDODInfo);
    void compareAgeAtDeath(unsigned int recDateAge, unsigned int DBdateAge);
    void compareIDs(QString recID, QString DBid, QString DBaltID, QString DBaltID2);
    void compareSpouseName(QString recSpouseName, QString DBspouseName);
    void comparePostalCodes(POSTALCODE_INFO &pc1, POSTALCODE_INFO &pc2);

    void setRankScore(int score);
    void analyzeRank();

};

class MATCHRESULT
{
private:
    QList<MATCHRECORD> matchRecordList;
    enum MATCHKEY bestMatchKey;
    int bestScore;
    int bestNetScore;
    int bestRankScore;
    unsigned int bestID;
    unsigned int bestNetID;
    unsigned int bestMatchKeyID;
    QString bestLastName;


public:
    MATCHRESULT();

    void clear();
    void sort();
    void update(unsigned int id, int score, int numInconsistency, int scoreInconsistency, bool newInfo, QString lastName);
    void setScore(unsigned int id, int score, int numInconsistency, int scoreInconsistency, bool newInfo, QString lastName);
    void addScore(unsigned int id, int score, int numInconsistency, int scoreInconsistency, bool newInfo, QString lastName);
    void update(MATCHRECORD &mr);
    void setScore(MATCHRECORD &mr);
    void addScore(MATCHRECORD &mr);
    void addRankScore(MATCHRECORD &mr);
    int getScore(unsigned int id) const;
    int getNetScore(unsigned int id) const;
    int getBestScore() const;
    int getBestNetScore() const;
    int getBestRankScore() const;
    QString getBestLastName() const;
    MATCHKEY getBestMatch() const;
    MATCHKEY getMatchKey(unsigned int id) const;
    QString getLastName(unsigned int id) const;
    unsigned int getBestID() const;
    unsigned int getBestNetID() const;
    unsigned int getBestMatchKeyID() const;
    bool alreadyContains(unsigned int id) const;
    bool isNewInfoAvailable(unsigned int id) const;
    bool isSufficientlyClose(unsigned int id) const;
    bool fromSameFH(unsigned int id) const;
};


MATCHKEY match(const dataRecord &dr, MATCHRESULT &matchScore, bool removeAccents = true);
int initialCheckRank(const dataRecord &dr, dataRecord &bestMatch, MATCHRESULT &matchScore, bool reverseRun = false);
void updateList(QStringList &listName, PQString string, bool removeAccents = true, bool isMiddleNameList = false);

double compareNames(QList<QString> &list1, QList<QString> &list2, bool countMultiple = false);
void addNameVariations(QList<QString> &nameList, QList<QString> &expandedNameList);
int calcProximityScore(POSTALCODE_INFO &pc1, POSTALCODE_INFO &pc2);

#endif
