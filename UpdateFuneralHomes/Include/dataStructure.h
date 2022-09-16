// dataStructure.h
//

#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

#include "../../UpdateFuneralHomes/Include/enums.h"
#include "../Include/Actuary/PQDate.h"
#include "../Include/Penser/OQString.h"
#include "providers.h"

struct NAMESTATS{

    QString name;

    unsigned int maleCount;
    unsigned int femaleCount;
    double malePct;
    CREDIBILITY credibility;

    unsigned surnameCount;

    bool isInitial;
    bool inParentheses;
    bool inQuotes;
    bool hasComma;
    bool isSurname;
    bool isGivenName;
    bool isLikelySurname;
    bool isLikelyGivenName;
    bool isNickName;
    bool isPureNickName;
    int frequencyTotal;
    int frequencyFirst;

    NAMESTATS(): maleCount(0), femaleCount(0), malePct(0.5), credibility(zero), surnameCount(0),
                 isInitial(false), inParentheses(false), inQuotes(false), hasComma(false),
                 isSurname(false), isGivenName(false), isLikelySurname(false),
                 isLikelyGivenName(false), isNickName(false), isPureNickName(false),
                 frequencyTotal(-1), frequencyFirst(-1) {}
    void clear()
    {
        name.clear();
        maleCount = 0;
        femaleCount = 0;
        malePct = 0.5;
        credibility = zero;
        surnameCount = 0;
        isInitial = false;
        inParentheses = false;
        inQuotes = false;
        hasComma = false;
        isSurname = false;
        isGivenName = false;
        isLikelySurname = false;
        isLikelyGivenName = false;
        isNickName = false;
        isPureNickName = false;
        frequencyTotal = -1;
        frequencyFirst = -1;
    }
    void determineCredibility();

};


struct NAMEINFO{
    PQString name;
    NAMETYPE type;
    unsigned int numWords;
    unsigned int position;
    bool hadBookEnds;
    bool hadComma;
    bool hadNeeEtAl;
    bool hadFormerly;
    bool priorNeeEtAlEncountered;
    bool hadAKAindicator;
    bool matched;  // not currently used
    bool nameStatsRun;
    NAMESTATS nameStats;

    NAMEINFO() : numWords(0), position(0), hadBookEnds(false), hadComma(false), hadNeeEtAl(false), hadFormerly(false),
        priorNeeEtAlEncountered(false), hadAKAindicator(false), matched(false), nameStatsRun(false) {}
    void clear(){
        nameStatsRun = false;
        name.clear();
        type = ntUnknown;
        position = 0;
        numWords = 0;
        hadBookEnds = false;
        hadComma = false;
        hadNeeEtAl = false;
        hadFormerly = false;
        priorNeeEtAlEncountered = false;
        hadAKAindicator = false;

        matched = false;
        nameStats.clear();
    }
};

struct DATEINFO{
    QDate exactDate;
    QDate minDate;
    QDate maxDate;
    unsigned int year;

    DATEINFO(): year(0) {
        exactDate.setDate(0,0,0);
        minDate.setDate(0,0,0);
        maxDate.setDate(0,0,0);
    }

    void clear(){
        exactDate.setDate(0,0,0);
        minDate.setDate(0,0,0);
        maxDate.setDate(0,0,0);
        year = 0;
    }
};

struct SEARCHPARAM {
    PQString Provider;		// Name of website developer
    PROVIDER provider;		// enum code for Provider
    PQString client;			// Third party who hired provider
    PQString version;		// Used to differentiate different platforms
    SEARCHTYPE searchType;
	
    PQString tagType;		// Keyword following "<"
    PQString tagID;			// First qualifier name after tagType
    PQString tagContents;	// TagID criteria
    PQString tagTarget;		// If another qualifier provided, pull wording between next set of "", otherwise pull wording after ">"
	
    PQString tableLookupValue;			// Search key to be used, where next field in "quotes" will be searched in the Lookup Table
	unsigned int tableLookupDelimiter;  // The delimiter within which values are read - usually QUOTES

	/******************************************************************/
	/* EXAMPLE                                                        */
	/* <meta name="description" content="Free Web tutorials">         */
	/*                                                                */
	/* tagType = "meta*                                               */
	/* tagID   = "name"                                               */
	/* tagContents = "description"                                    */
	/*                                                                */
	/******************************************************************/
};

struct DATES {
    QDate potentialDOB;
    QDate potentialDOD;
    int potentialYOB;
    int potentialYOD;
    bool fullyCredible;

    DATES(): potentialYOB(0), potentialYOD(0), fullyCredible(false) {}

    bool hasDateInfo(){return (potentialDOB.isValid() || potentialDOD.isValid() || (potentialYOB > 0) || (potentialYOD > 0));}
};

struct PUBLISHER {
    PQString name;
    PQString city;
    PQString province;
    PQString country;

    void clear(){
        name.clear();
        city.clear();
        province.clear();
        country.clear();
    }
};

class CONTENTREAD
{
public:
    CONTENTREAD();
    ~CONTENTREAD();

    OQString allContent;
    OQString firstBlock;
    OQString secondBlock;
    OQString thirdBlock;

    OQString select(const PQString &targetWord);

    void cleanUpEnds();
    void clear();
};

class WARNINGINFO
{
public:
    WARNINGINFO();
    ~WARNINGINFO();

    int dateAgeError;
    int ageFlag;
    int dateFlag;
    int genderFlag;
    int nameReversalFlag;
    int nameFlagGeneral;
    int doubleMemorialFlag;
    int bilingualFlag;
    int futureUse;
    int outstandingErrMsg;
    int memorialFlag;
    int validated;

    bool nameWarningException;

    QString spouseFirtName;
    QString checkParentsName;
    QString checkInclName;
    QString checkExclName;
    QString confirmTreatmentName;
    QString confirmMiddleName;

    void clear();
    void resetNonDateValidations();

};

class FUNERALHOME
{
public:
    FUNERALHOME();
    ~FUNERALHOME();

    int listerKey;
    int listerID;
    int providerID;
    int providerKey;

    QString fhName;
    QString fhCity;
    QString fhProvince;
    QString fhPostalCode;
    QString fhURL;
    QString fhURLparam1;
    QString fhURLparam2;
    QString fhHTTP;
    QString fhWWW;

    int fhRunStatus;

    void clear();
};



#endif
