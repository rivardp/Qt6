// qUnstructuredContent.h

#ifndef UNSTRUCTUREDCONTENT_H
#define UNSTRUCTUREDCONTENT_H

#include <QDate>
#include <QSqlQuery>
#include <QSqlError>

#include "../Include/Actuary/PQDate.h"
#include "../Include/PMySQL/databaseSearches.h"
#include "../Include/PMySQL/funeralHomeSQL.h"
#include "../Include/Penser/OQStream.h"
#include "../Include/Unstructured/qMakeDate.h"
#include "../UpdateFuneralHomes/Include/dataStructure.h"
#include "../UpdateFuneralHomes/Include/dataRecord.h"
#include "../UpdateFuneralHomes/Include/matchRecord.h"
#include "../UpdateFuneralHomes/Include/globalVars.h"

class readObit;

class unstructuredContent :public OQStream{

public:

	unstructuredContent();
    unstructuredContent(const OQString source);
    unstructuredContent(const unstructuredContent &orig);
	~unstructuredContent();

    void determineLanguageAndGender(OQStream &justInitialNames);
    GENDER getGender() const;
    LANGUAGE getLanguage() const;
    unsigned int getNumMaleWords() const;
    unsigned int getNumFemaleWords() const;

    DATES readNumericDates(DATEORDER dateOrder = doNULL);
    DATES readDates(LANGUAGE lang, bool limitWords = false);
    DATES readYears(bool reliable = true);
    DATES readDOBandDOD(bool reliable = true);
    bool  sentenceKeyWordsAndDates(const LANGUAGE lang, DATES &dates);
    DATES contentKeyWordsAndDates(QList<QString> &firstNameList, LANGUAGE lang, unsigned int maxSentences = 100);
    void  sentencePullBackToBackDates(const LANGUAGE lang, DATES &dates);
    DATES contentPullBackToBackDates(LANGUAGE lang);
    void  sentenceReadBornYear(const LANGUAGE lang, DATES &dates);
    DATES contentReadBornYear(LANGUAGE lang);
    unsigned int sentenceReadAgeAtDeath(bool updateDirectly = true, bool preRead = false);
    unsigned int sentenceReadNakedAgeAtDeath(bool updateDirectly = true);
    unsigned int sentenceReadAgeNextBirthday();
    unsigned int sentenceReadUnbalancedYOB();
    unsigned int contentReadAgeAtDeath(unsigned int maxSentences);
    unsigned int contentReadAgeAtDeath(QList<QString> firstName);
    unsigned int contentReadAgeNextBirthday(unsigned int maxSentences);
    void readAgeAtDeathPostNumChecks(bool &numFound, bool &contextError, bool eventCheck, int &num, int &altNum, OQString peek1, OQString peek2, OQString peek3, LANGUAGE lang);
    void readDateOfService(QDate &DOSD, LANGUAGE lang);
    void sentenceReadDateOfService(QDate &DOSD, LANGUAGE lang);
    void contentReadSpouseName(LANGUAGE lang = language_unknown);
    bool sentenceReadSpouseName(LANGUAGE lang);
    bool sentenceReadYearsMarried(LANGUAGE lang);
    bool clean(LANGUAGE lang = language_unknown);
    bool truncateAfterParentReference(LANGUAGE lang = language_unknown, bool firstSentence = false);
    bool truncateAfterChildReference(LANGUAGE lang = language_unknown, bool firstSentence = false);
    bool truncateAfterSiblingReference(LANGUAGE lang = language_unknown, bool firstSentence = false);
    bool truncateAfterRelativeReference(LANGUAGE lang = language_unknown, bool firstSentence = false);
    bool truncateAfterRelationshipWords(LANGUAGE lang = language_unknown, bool firstSentence = false);
    bool truncateAfterMiscWords(LANGUAGE lang = language_unknown, bool firstSentence = false);
    bool truncateAfter(QList<QString> targetPhrases, bool firstSentence = false);
    bool truncateAfter(QString cutOffText, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    bool removeBornToFiller(LANGUAGE lang = language_unknown);
    bool removeSpouseForFiller(LANGUAGE lang = language_unknown);
    bool removeAtTheHospitalFiller(LANGUAGE lang = language_unknown);
    bool removeDates(LANGUAGE lang = language_unknown);
    bool removeYears();
    bool removeTrailingLocation();
    bool reverseUncapitalizedSentenceStarts();
    bool extractLocationIfFirst(GLOBALVARS &gv);
    bool stripOutAndProcessDates();
    void removeCelebration();
    void removeLeadingJunk();

    void enhanceWith(unstructuredContent uc);
    void splitComponents(QString &datesRemoved, QString &justDates);

    void compressCompoundNames(QString &string, LANGUAGE lang = language_unknown);
    void compressCompoundNames(OQString &string, LANGUAGE lang = language_unknown);
    void compressCompoundNames(OQStream &string, LANGUAGE lang = language_unknown);
    void compressCompoundNames(unstructuredContent &string, LANGUAGE lang = language_unknown);

    QDate readDateField(DATEORDER dateOrder = doNULL);
    PQString processAllNames();
    QList<NAMEINFO> readAlternates(unsigned int endPoints, bool applyAdjacentChecks = true, GENDER workingGender = genderUnknown);
    bool nameAndGenderConsistent(QString name, GENDER gender);
    bool isJustDates();
    bool isJustNames(bool justSavedNames = false);
    bool isJustSavedNames();
    void splitIntoSentences();
    void splitIntoSentences(QList<QString> &firstWordList, QStringList &stopWords);
    void addSentence(OQString &newSentence);
    OQStream getSentence(const LANGUAGE &language = language_unknown, const int sentenceNum = 0);                                                                                               // Overrides OQStream class method
    OQStream getSentence(bool &realSentenceEncountered, const QStringList &stopWords, const LANGUAGE &lang = language_unknown, const int sentenceNum = 0);                                      // Overrides OQStream class method
    OQStream getSentence(const QList<QString> &firstNames, bool &realSentenceEncountered, const QStringList &stopWords, const LANGUAGE &lang = language_unknown, const int sentenceNum = 0);    // Overrides OQStream class method
    OQStream getNextRealSentence(bool restrictNamesToDR = false, unsigned int minWordCount = 6);
    void prepareNamesToBeRead(bool removeRecognized = true);
    void pickOffNames();
    void checkForDates();
    void fixHyphenatedNames();
    void fixOneLargeQuoteBlock();
    void prepareStructuredNames();
    void processStructuredYears();
    void processDateField(DATEFIELD dateField, DATEORDER dateOrder = doNULL);
    void processYearField(DATEFIELD dateField);
    void processAgeAtDeath();
    void processGender();

    bool genderRelationalReferences(unsigned int &maleCount, unsigned int &femaleCount, bool avoidDoubleCount = false);
    void removeAllKnownNames();
    void removeErroneousComma();
    void insertRequiredComma();
    void readFirstNameFirst(QList<NAMESTATS> &nameStatsList);
    void readLastNameFirst(QList<NAMESTATS> &nameStatsList);
    bool validateNameOrder(bool initialCheck = true);
    bool validateNameOrder(QList<NAMESTATS> &nameStatsList, bool initialCheck = true);
    void validateNameOrder2();
    void extraNameProcessing();

    bool lookupUncapitalizedNames(OQString name);
    bool listContainsLocationWords(QList<QString> &list);
    bool areAllLocationWords();
    void parseDateFromAndTo(const OQString &word, MAKEDATE &md, DATEORDER dateOrder = doNULL);

    DATES sentencePullOutDates(const LANGUAGE lang, const unsigned int maxSentences);
    unsigned int pullOutDates(const LANGUAGE lang, QList<QDate> &dateList, unsigned int maxDates, OQString &cleanString, bool limitWords, bool serviceDate = false, int forceYear = 0);
    unsigned int pullOutEnglishDates(QList<QDate> &dateList, unsigned int maxDates, OQString &cleanString, bool limitWords, bool serviceDate, int forceYear = 0);
    unsigned int pullOutFrenchDates(QList<QDate> &dateList, unsigned int maxDates, OQString &cleanString, bool limitWords, bool serviceDate, int forceYear = 0);
    unsigned int pullOutSpanishDates(QList<QDate> &dateList, unsigned int maxDates, OQString &cleanString, bool limitWords, bool serviceDate, int forceYear = 0);
    bool pullBackToBackEnglishDates(QList<QDate> &dateList);
    bool pullBackToBackFrenchDates(QList<QDate> &dateList);
    bool pullBackToBackSpanishDates(QList<QDate> &dateList);
    QDate fillInDate(const LANGUAGE lang, OQString &cleanString, const int year);

    int getNumPages();
    int getNumSentences() const;
    int getSentenceNum(int index) const;
    int getSentenceStartPosition(int sentenceNum) const;

    OQStream getJustNames(const NAMESKNOWN nk);

    int countFrequency(QString word, Qt::CaseSensitivity = Qt::CaseInsensitive) const;
    int countFrequencyFirst(QString word, Qt::CaseSensitivity = Qt::CaseInsensitive) const;
    int countFrequency(unstructuredContent *uc, QString word, Qt::CaseSensitivity = Qt::CaseInsensitive) const;
    int countFrequencyFirst(unstructuredContent *uc, QString word, Qt::CaseSensitivity = Qt::CaseInsensitive) const;
    int countFirstNames();

    void copyKeyVariablesFrom(const unstructuredContent &uc);
    void clear();
    void clearString();
    void clearSentenceStartPositions();
    void setItsString(const OQString &rhs);

    void SaltWireCleanUp();

    unstructuredContent& operator= (const OQString &rhs);
    unstructuredContent& operator= (const unstructuredContent &orig);

    void setGlobalVars(GLOBALVARS &gv);
    void setContentLanguage(LANGUAGE lang);

protected:
    LANGUAGE contentLanguage;
    GENDER contentGender;

    unsigned int numEnglishDates;
	unsigned int numFrenchDates;
	unsigned int numSpanishDates;
    unsigned int numMaleWords;
    unsigned int numFemaleWords;

    QList<int> sentenceStartPositions;
    int lastSentenceStartPosition;
    bool realSentenceEncountered;

    static GLOBALVARS *globals;
    MAKEDATE dateToday;
};

#endif
