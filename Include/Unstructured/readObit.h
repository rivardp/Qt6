// readObit.h
//

#ifndef READOBIT_H
#define READOBIT_H


class dataRecord;

#include <QFile>
#include <QDate>

#include "../UpdateFuneralHomes/Include/dataStructure.h"
#include "../UpdateFuneralHomes/Include/dataRecord.h"
#include "../UpdateFuneralHomes/Include/tableLookup.h"
#include "../UpdateFuneralHomes/Include/providers.h"
#include "../UpdateFuneralHomes/Include/mostCommonName.h"
#include "../Include/Actuary/PQDate.h"
#include "../Include/PMySQL/databaseSearches.h"
#include "../Include/Penser/OQString.h"
#include "../Include/Penser/OQStream.h"
#include "../Include/Unstructured/qUnstructuredContent.h"
#include "keyWords.h"

class readObit:public OQStream
{
public:
    readObit();
    ~readObit();
	
    void operator = (QFile &file);
    bool setSource(QFile &file);
    PQString getSource() const;

    void setGlobalVars(GLOBALVARS &gv);
    void setGodReference(bool bl);
    bool hasGodReference() const;
    void setHyphenatedNameFlag(bool flag);
    bool hasHyphenatedName() const;
    void setNumMaleWords(int numWords);
    void setNumFemaleWords(int numWords);
    int getNumMaleWords() const;
    int getNumFemaleWords() const;
    QStringList getStopWords();
    QStringList getLocationWords(PROVIDER provider, unsigned int providerKey);

    void clear();

    void read();
    void readLanguage();
    void readInObitTitle();
    void readInObitAddress();
    void readInCustomAddress();
    void readInObitDates();
    void readInObitText();
    void processObitText(bool UndoBadSentence, bool insertPeriods);
    void readLookupFields(const SEARCHPARAM &sp, const fieldNames &fn);
    void readMessages();
    void readPublishDate();
    void readServiceDate();
    void readTitleInfo();
    void readStructuredContent();
    void readUnstructuredContent(bool UseFirstDateAsDOD);
    void readParentsLastName();
    void readStyle();
    void readGender();
    void cleanAndExtract();
    void replaceProblematicChars();
    void addressUnknownGender();
    void processMaidenNames();
    void validateJustInitialNames();
    void removeProblematicWordsFromJIN();
    void finalNameCleanup();
    int  runNameValidations();
    int  runGenderValidation();
    int  runRecordValidation();
    void determinePotentialFirstName();
    void runStdProcessing(unstructuredContent &uc, bool insertPeriods);
    void addressMissingGender();
    void genderBySentencesStartingWithPronouns();
    void genderByRelationshipWords();
    void genderByTitle();
    void genderByAltName();
    void genderByUnionReference();
    void genderByNameLookup();

    unsigned int countFrequency(QString word, Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive) const;
    unsigned int countFrequencyFirst(QString word, Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive) const;

    void parseContentFromTo(PString &content, std::vector<PString*> &wordVector);

    bool processNewDateInfo(DATES &dates, unsigned int pass);
    bool fillInDatesStructured(unstructuredContent &uc, bool reliable = true);
    void fillInDatesFirstPass();
    void fillInDatesSecondPass();
    void fillInDatesThirdPass(unstructuredContent &uc);
    void fillInDatesFourthPass();
    void fillInDatesFifthPass();
    void fillInDatesSixthPass();
    void fillInDatesSeventhPass();
    void fillInDatesSeventhPassFollowUp();
    void fillInDatesEighthPass();
    void fillInDatesNinthPass();
    void fillInDatesTenthPass();
    void assignFirstDateToDOD();

    bool missingDateInfo() const;
    bool isLocation(OQString word);

    void saveStructuredNamesProcessed(QString namesProcessed);
    unstructuredContent getStructuredNamesProcessed() const;
    bool runAlternateNameProcessing1();
    bool runAlternateNameProcessing1b();
    bool reorderNames();

    unstructuredContent* getUCaddress();
    unstructuredContent* getJustInitialNamesAddress();
    GLOBALVARS *globals;

private:
    PQString filename;
    unstructuredContent uc;
    unstructuredContent ucCleaned;
    unstructuredContent ucFillerRemoved;                // With dates and "born...to" filler eliminated
    unstructuredContent ucFillerRemovedAndTruncated;    // With dates and "born...to" filler eliminated
    unstructuredContent justInitialNamesUC;             // All words until a "key" word or "divider" is encountered
    unstructuredContent structuredNamesProcessed;        // The UC used for the structured name read
    mostCommonName mcn;                                 // List of first words of each sentence (to find most common name)
    QList<NAMESTATS> nameStatsList;
    bool godReference;
    bool hyphenatedLastName;
    int numMaleWords;
    int numFemaleWords;

    unsigned int style;     // Used to differentiate between different webpage styles for the same provider

};

#endif  // READOBIT_H

