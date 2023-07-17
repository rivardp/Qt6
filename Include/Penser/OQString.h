// OQString.h - PQString with some added functionality for Obits
// 


#ifndef OQSTRING_H
#define OQSTRING_H

#include <QString>
#include <QSqlQuery>
#include <QUrl>
#include <QDate>

#include "../../UpdateFuneralHomes/Include/enums.h"

struct monthInfo{
    QString monthAlpha;
    unsigned int monthNumeric;
    monthInfo() : monthAlpha(""), monthNumeric(0) {}
    monthInfo(QString mthAlpha, unsigned int mthNumeric) : monthAlpha(mthAlpha), monthNumeric(mthNumeric) {}
};

struct dayOfWeekInfo{
    QString dayOfWeekAlpha;
    unsigned int dayOfWeekNumeric;
    dayOfWeekInfo() : dayOfWeekAlpha(""), dayOfWeekNumeric(0) {}
    dayOfWeekInfo(QString dowAlpha, unsigned int dowNumeric) : dayOfWeekAlpha(dowAlpha), dayOfWeekNumeric(dowNumeric) {}
};

#include "PQString.h"
#include <QListIterator>

class OQString :public PQString
{
public:
	// Constructors
    OQString();
    OQString(const char* const);			// Convert character array to PQString
    OQString(const PQString &);
    OQString(const QString &);
    OQString(const OQString &);
    OQString(const QChar& singleChar);		// created for get()
    OQString(const std::string &);
    OQString(const std::wstring &);

	// Destructor
    ~OQString();

    // Operators
    // The ones commented out below inherit from PQString
    // QChar & operator[](unsigned int offset);
    // QChar operator[](unsigned int offset) const;
    OQString operator+(const OQString &rhs);
    OQString operator+(const PQString &rhs);
    OQString operator+(const QString &rhs);
    OQString operator+(const QChar &rhs);
    void operator+=(const OQString &rhs);
    // void operator+=(const PQString &rhs);
    // void operator+=(const QString &rhs);
    // void operator+=(const QChar &rhs);
    OQString& operator= (const OQString &rhs);
    OQString& operator= (const PQString &rhs);
    OQString& operator= (const QString &rhs);
    OQString& operator= (const QChar &rhs);
    bool operator== (const OQString &rhs);
    // bool operator== (const PQString &rhs);
    // bool operator== (const QString &rhs);
    // bool operator== (const QChar &rhs);
    bool operator!= (const OQString &rhs);
    // bool operator!= (const PQString &rhs);
    // bool operator!= (const QString &rhs);
    // bool operator!= (const QChar &rhs);

	// Methods
    OQString readURLparameter(QString param1, QString param2 = QString(""));
    OQString pullOutWord(unsigned int startingPosition);
    OQString pullOutSentence(unsigned int startingPosition);
    QString convertToID();
    QString convertFromID();
    bool removeOrdinal(LANGUAGE language = language_unknown);
    bool removeLeadingNeeEtAl(LANGUAGE language = language_unknown);
    bool removeLeadingNeeEtAl(OQString nextWord, LANGUAGE language = language_unknown);
    bool removeLeadingAKA();
    bool removeInternalPeriods();
    bool removeSpousalReference(const LANGUAGE lang, bool firstSentence = false);
    void removeWrittenMonths();
    void removeAllSuffixPrefix();
    void removeIntroductions();
    void removeSuffixPrefix(const QList<QString> &list);
    void removeStrings(const QList<QString> &list);
    void conditionalBreaks();
    void fixBasicErrors(bool onlyContainsNames = false);
    void fixDateFormats();
    bool fixHyphenatedSaint();
    bool fixParentheses();
    //bool fixQuotes();
    //bool fixQuotes(QList<QString> &listOfFirstWords, GENDER gender = genderUnknown, LANGUAGE lang = language_unknown); // No longer in use
    //unsigned int standardizeQuotes();           // Returns number of quotes read in
    //unsigned int standardizeQuotes(QList<QString> &listOfFirstWords, GENDER gender = genderUnknown, LANGUAGE lang = language_unknown); // No longer in use
    //bool checkQuotes(unsigned int numQuotes);   // Looks for potential errors if uneven number of quotes exist
    void standardizeQuotes(); // Rewritten in this form March 26, 2023
    QList<OQString> createList() const;
    OQString& tidyUp();

    bool isPrefix(LANGUAGE lang = language_unknown) const;
    bool isFullPrefix(LANGUAGE lang = language_unknown) const;
    bool isAbbreviatedPrefix(LANGUAGE lang = language_unknown) const;
    bool isSuffix(LANGUAGE lang = language_unknown) const;
    bool isSuffixKeep(LANGUAGE lang = language_unknown) const;
    bool isSuffixDrop(LANGUAGE lang = language_unknown) const;
    bool isSuffixAllCaps(LANGUAGE lang = language_unknown) const;
    bool isTitle(LANGUAGE lang = language_unknown, GENDER gender = genderUnknown) const;
    bool isMaleTitle(LANGUAGE lang = language_unknown) const;
    bool isFemaleTitle(LANGUAGE lang = language_unknown) const;
    bool isEnglishTitle(GENDER gender = genderUnknown) const;
    bool isFrenchTitle(GENDER gender = genderUnknown) const;
    bool isSpanishTitle(GENDER gender = genderUnknown) const;
    bool isSaint() const;
    bool isGodReference() const;
    bool isSpousalReference(const LANGUAGE lang = language_unknown) const;
    bool isAbbreviation() const;
    bool isProvAbbreviation() const;
    bool isCompoundName(LANGUAGE lang = language_unknown) const;
    bool isUncapitalizedName() const;
    bool isSingleVan() const;
    bool isNoVowelName() const;
    bool isHyphenatedName() const;
    bool isDegree() const;
    bool isAltNameIndicator() const;
    bool isNeeEtAl() const;
    bool isNeeEtAl(OQString nextWord) const;
    bool isEndOfBlockTag() const;
    bool isLocation() const;
    bool isInitial() const;

    bool isAnd() const;
    bool isOrdinal(const LANGUAGE lang = language_unknown) const;
    bool isOrdinalNumber(const LANGUAGE lang = language_unknown) const;
    unsigned int getOrdinalNumber(const LANGUAGE lang = language_unknown) const;
    QList<QString> getBrotherReferences(const LANGUAGE lang = language_unknown) const;
    QList<QString> getParentReferences(const LANGUAGE lang = language_unknown, GENDER gender = genderUnknown) const;
    QList<QString> getSpousalReferences(const LANGUAGE lang = language_unknown, GENDER gender = genderUnknown) const;
    QList<QString> getSiblingReferences(const LANGUAGE lang = language_unknown, GENDER gender = genderUnknown) const;
    QList<QString> getChildReferences(const LANGUAGE lang = language_unknown, GENDER gender = genderUnknown) const;
    QList<QString> getRelativeReferences(const LANGUAGE lang = language_unknown) const;
    QList<QString> getRelationshipWords(const LANGUAGE lang = language_unknown, GENDER gender = genderUnknown) const;
    QList<QString> getMiscWords(const LANGUAGE lang = language_unknown) const;
    QList<QString> convertToQStringList(const QList<OQString> OQlist) const;

    bool isInformalVersionOf(const QString &formalname, PQString &errMsg) const;
    bool isFormalVersionOf(const QString &nickname, PQString &errMsg) const;
    bool isAGivenName(PQString &errMsg) const;
    bool isALastName(PQString &errMsg) const;
    bool isNickNameRelativeToList(QList<QString> &listOfNames);

    unsigned int isWrittenMonth(const LANGUAGE &lang = language_unknown, const bool fullWordsOnly = false);
    unsigned int isWrittenMonthAbbreviation(const LANGUAGE &lang = language_unknown);
    unsigned int isWrittenDayOfWeek(const LANGUAGE &lang = language_unknown);
    bool isPureDateRange(LANGUAGE lang = language_unknown) const;
    bool isRecognized(LANGUAGE lang = language_unknown) const;
    bool isRecognizedFirstWord() const;
    bool isFoundIn(QList<QString> &wordList, int sortOrder = 0) const;

    bool isEnglish() const;
    bool isEnglishMale() const;
    bool isEnglishFemale() const;
    bool isFrench() const;
    bool isFrenchMale() const;
    bool isFrenchFemale() const;
    bool isSpanish() const;
    bool isSpanishMale() const;
    bool isSpanishFemale() const;

    bool isBirthWord(LANGUAGE language = language_unknown) const;
    bool isAgeWord(LANGUAGE language = language_unknown) const;
    bool isDeathWord(LANGUAGE language = language_unknown) const;
    bool isServiceWord(LANGUAGE language = language_unknown) const;
    bool isDeathIndicator(LANGUAGE language = language_unknown) const;
    bool isOtherPersonReference(LANGUAGE language = language_unknown) const;
    bool isMiscKeeperWord(LANGUAGE language = language_unknown) const;
    bool isProblematicFirstName() const;
    bool isPronoun() const;
    bool isAboriginalName(OQString nextWord, bool firstCall = true) const;
    bool areParentWords(LANGUAGE lang = language_unknown, GENDER gender = genderUnknown) const;
    bool isProvince() const;

    bool isGenderWord(GENDER gender = genderUnknown, LANGUAGE language = language_unknown) const;
    bool isMaleGenderWord(LANGUAGE language = language_unknown) const;
    bool isFemaleGenderWord(LANGUAGE language = language_unknown) const;
    bool hasDualMeaning(LANGUAGE language = language_unknown) const;

    bool areRelationshipWords(GENDER gender = genderUnknown, LANGUAGE language = language_unknown) const;
    bool areEnglishRelationshipWords() const;
    bool areEnglishMaleRelationshipWords() const;
    bool areEnglishFemaleRelationshipWords() const;
    bool areEnglishUnisexRelationshipWords() const;
    bool areFrenchRelationshipWords() const;
    bool areFrenchMaleRelationshipWords() const;
    bool areFrenchFemaleRelationshipWords() const;
    bool areFrenchUnisexRelationshipWords() const;
    bool areSpanishRelationshipWords() const;
    bool areSpanishMaleRelationshipWords() const;
    bool areSpanishFemaleRelationshipWords() const;
    bool areSpanishUnisexRelationshipWords() const;

    bool followedByPrecedingIndicators(OQString &peek1, OQString &peek2, OQString &peek3, LANGUAGE language = language_unknown) const;

    QUrl asQUrl();

protected:
    friend class dataRecord;
    friend class mostCommonName;

    static QList<monthInfo> monthsEnglishFull;
    static QList<monthInfo> monthsFrenchFull;
    static QList<monthInfo> monthsSpanishFull;
    static QList<monthInfo> monthsEnglishAbbreviated;
    static QList<monthInfo> monthsFrenchAbbreviated;
    static QList<monthInfo> monthsSpanishAbbreviated;

    static QList<dayOfWeekInfo> daysOfWeekEnglish;
    static QList<dayOfWeekInfo> daysOfWeekFrench;
    static QList<dayOfWeekInfo> daysOfWeekSpanish;

    static QList<QString> uncapitalizedNames;
    static QList<QString> singleVans;
    static QList<QString> hyphenatedNameBeginnings;
    static QList<QString> aboriginalNames;
    static QList<QString> problematicAboriginalNames;
    static QList<QString> noVowelNames;
    static QList<QString> prefixesAbbreviatedEnglish;
    static QList<QString> prefixesAbbreviatedFrench;
    static QList<QString> prefixesAbbreviatedSpanish;
    static QList<QString> prefixesFullEnglish;
    static QList<QString> prefixesFullFrench;
    static QList<QString> prefixesFullSpanish;
    static QList<QString> suffixesKeepEnglish;
    static QList<QString> suffixesKeepFrench;
    static QList<QString> suffixesKeepSpanish;
    static QList<QString> suffixesDropEnglish;
    static QList<QString> suffixesDropFrench;
    static QList<QString> suffixesDropSpanish;
    static QList<QString> suffixesDegree;
    static QList<QString> maleTitlesEnglish;
    static QList<QString> femaleTitlesEnglish;
    static QList<QString> maleTitlesFrench;
    static QList<QString> femaleTitlesFrench;
    static QList<QString> maleTitlesSpanish;
    static QList<QString> femaleTitlesSpanish;
    static QList<QString> saints;
    static QList<QString> godReferences;

    static QList<QString> ordinalsEnglish;
    static QList<QString> ordinalsFrench;
    static QList<QString> ordinalsSpanish;

    static QList<QString> altNameIndicators;
    static QList<QString> precedingIndicators;
    static QList<QString> changedContextIndicators;
    static QList<QString> neeEtAlwords;
    static QList<QString> endOfBlockTags;

    static const int MAXWORDLENGTH = 14;
    static QList<QString> ignoreWordsEnglish1;
    static QList<QString> ignoreWordsEnglish2;
    static QList<QString> ignoreWordsEnglish3;
    static QList<QString> ignoreWordsEnglish4;
    static QList<QString> ignoreWordsEnglish5;
    static QList<QString> ignoreWordsEnglish6;
    static QList<QString> ignoreWordsEnglish7;
    static QList<QString> ignoreWordsEnglish8;
    static QList<QString> ignoreWordsEnglish9;
    static QList<QString> ignoreWordsEnglish10;
    static QList<QString> ignoreWordsEnglish11;
    static QList<QString> ignoreWordsEnglish12;
    static QList<QString> ignoreWordsEnglish13;
    static QList<QString> ignoreWordsEnglish14;

    static QList<QString> ignoreWordsFrench1;
    static QList<QString> ignoreWordsFrench2;
    static QList<QString> ignoreWordsFrench3;
    static QList<QString> ignoreWordsFrench4;
    static QList<QString> ignoreWordsFrench5;
    static QList<QString> ignoreWordsFrench6;
    static QList<QString> ignoreWordsFrench7;
    static QList<QString> ignoreWordsFrench8;
    static QList<QString> ignoreWordsFrench9;
    static QList<QString> ignoreWordsFrench10;
    static QList<QString> ignoreWordsFrench11;
    static QList<QString> ignoreWordsFrench12;
    static QList<QString> ignoreWordsFrench13;
    static QList<QString> ignoreWordsFrench14;

    static QList<QString> ignoreWordsSpanish1;
    static QList<QString> ignoreWordsSpanish2;
    static QList<QString> ignoreWordsSpanish3;
    static QList<QString> ignoreWordsSpanish4;
    static QList<QString> ignoreWordsSpanish5;
    static QList<QString> ignoreWordsSpanish6;
    static QList<QString> ignoreWordsSpanish7;
    static QList<QString> ignoreWordsSpanish8;
    static QList<QString> ignoreWordsSpanish9;
    static QList<QString> ignoreWordsSpanish10;
    static QList<QString> ignoreWordsSpanish11;
    static QList<QString> ignoreWordsSpanish12;
    static QList<QString> ignoreWordsSpanish13;
    static QList<QString> ignoreWordsSpanish14;

    static QList<QString> ignoreFirstWords1;
    static QList<QString> ignoreFirstWords2;
    static QList<QString> ignoreFirstWords3;
    static QList<QString> ignoreFirstWords4;
    static QList<QString> ignoreFirstWords5;
    static QList<QString> ignoreFirstWords6;
    static QList<QString> ignoreFirstWords7;
    static QList<QString> ignoreFirstWords8;
    static QList<QString> ignoreFirstWords9;
    static QList<QString> ignoreFirstWords10;
    static QList<QString> ignoreFirstWords11;
    static QList<QString> ignoreFirstWords12;
    static QList<QString> ignoreFirstWords13;
    static QList<QString> ignoreFirstWords14;

    static QList<QString> genderWordsEnglishM;
    static QList<QString> genderWordsEnglishF;
    static QList<QString> genderWordsFrenchM;
    static QList<QString> genderWordsFrenchF;
    static QList<QString> genderWordsSpanishM;
    static QList<QString> genderWordsSpanishF;
    static QList<QString> pronouns;

    static QList<QString> relationshipWordsEnglishM;
    static QList<QString> relationshipWordsEnglishF;
    static QList<QString> relationshipWordsEnglishU;
    static QList<QString> relationshipWordsFrenchM;
    static QList<QString> relationshipWordsFrenchF;
    static QList<QString> relationshipWordsFrenchU;
    static QList<QString> relationshipWordsSpanishM;
    static QList<QString> relationshipWordsSpanishF;
    static QList<QString> relationshipWordsSpanishU;

    static QList<QString> miscWordsEnglish;
    static QList<QString> miscWordsFrench;
    static QList<QString> miscWordsSpanish;

    static QList<QString> problematicFirstNames;
    static QList<QString> problematicFemaleMiddleNames;
    static QList<QString> problematicRoadReferences;
    static QList<QString> problematicBookEndContents;

    static QList<QString> locations;
    static QList<QString> routes;
    static QList<QString> provAbbreviations;
    static QList<QString> provLong;
    static QList<QString> otherAbbreviations;

    // The variables below contain repeats of ignoreWords above
    static QList<QString> deathWordsEnglish;
    static QList<QString> deathWordsFrench;
    static QList<QString> deathWordsSpanish;

    static QList<QString> birthWordsEnglish;
    static QList<QString> birthWordsFrench;
    static QList<QString> birthWordsSpanish;

    static QList<QString> ageWordsEnglish;
    static QList<QString> ageWordsFrench;
    static QList<QString> ageWordsSpanish;

    static QList<QString> serviceWordsEnglish;
    static QList<QString> serviceWordsFrench;
    static QList<QString> serviceWordsSpanish;

    static QList<QString> brotherWordsEnglish;
    static QList<QString> brotherWordsFrench;
    static QList<QString> brotherWordsSpanish;

    static QList<QString> parentWordsEnglishM;
    static QList<QString> parentWordsEnglishF;
    static QList<QString> parentWordsEnglishU;
    static QList<QString> parentWordsFrenchM;
    static QList<QString> parentWordsFrenchF;
    static QList<QString> parentWordsFrenchU;
    static QList<QString> parentWordsSpanishM;
    static QList<QString> parentWordsSpanishF;
    static QList<QString> parentWordsSpanishU;

    static QList<QString> childReferencesEnglishM;
    static QList<QString> childReferencesEnglishF;
    static QList<QString> childReferencesEnglishU;
    static QList<QString> childReferencesFrenchM;
    static QList<QString> childReferencesFrenchF;
    static QList<QString> childReferencesFrenchU;
    static QList<QString> childReferencesSpanishM;
    static QList<QString> childReferencesSpanishF;
    static QList<QString> childReferencesSpanishU;

    static QList<QString> siblingReferencesEnglishM;
    static QList<QString> siblingReferencesEnglishF;
    static QList<QString> siblingReferencesEnglishU;
    static QList<QString> siblingReferencesFrenchM;
    static QList<QString> siblingReferencesFrenchF;
    static QList<QString> siblingReferencesFrenchU;
    static QList<QString> siblingReferencesSpanishM;
    static QList<QString> siblingReferencesSpanishF;
    static QList<QString> siblingReferencesSpanishU;

    static QList<QString> spousalReferencesEnglishM;
    static QList<QString> spousalReferencesEnglishF;
    static QList<QString> spousalReferencesEnglishU;
    static QList<QString> spousalReferencesFrenchM;
    static QList<QString> spousalReferencesFrenchF;
    static QList<QString> spousalReferencesFrenchU;
    static QList<QString> spousalReferencesSpanishM;
    static QList<QString> spousalReferencesSpanishF;
    static QList<QString> spousalReferencesSpanishU;

    static QList<QString> relativeReferencesEnglish;
    static QList<QString> relativeReferencesFrench;
    static QList<QString> relativeReferencesSpanish;

    static QList<QString> passingReferencesEnglishM;
    static QList<QString> passingReferencesEnglishF;
    static QList<QString> passingReferencesEnglishU;
    static QList<QString> passingReferencesFrenchM;
    static QList<QString> passingReferencesFrenchF;
    static QList<QString> passingReferencesFrenchU;
    static QList<QString> passingReferencesSpanishM;
    static QList<QString> passingReferencesSpanishF;
    static QList<QString> passingReferencesSpanishU;

    static QList<QString> otherPersonReferenceWordsEnglish;
    static QList<QString> otherPersonReferenceWordsFrench;
    static QList<QString> otherPersonReferenceWordsSpanish;

    static QList<QString> otherMiscKeeperWordsEnglish;
    static QList<QString> otherMiscKeeperWordsFrench;
    static QList<QString> otherMiscKeeperWordsSpanish;

    static QList<QString> deathIndicatorsEnglish;
    static QList<QString> deathIndicatorsFrench;
    static QList<QString> deathIndicatorsSpanish;


};


#endif
