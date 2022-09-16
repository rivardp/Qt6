// mostCommonName.cpp

#include "mostCommonName.h"


mostCommonName::mostCommonName() :  nameFreq(0), SetupCompleted(false)
{

}

mostCommonName::~mostCommonName()
{

}

void mostCommonName::cleanUpList()
{
    if (listOfFirstWords.size() == 0)
        return;

    int i = 0;
    int lastPosition = listOfFirstWords.size() - 1;
    OQString tempWord;

    while (i <= lastPosition)
    {
        tempWord = OQString(listOfFirstWords.at(i));
        tempWord.removeEnding(PUNCTUATION);
        tempWord.removeBookEnds(QUOTES);
        tempWord.removeBookEnds(PARENTHESES);
        tempWord.removeLeading(PARENTHESES);
        tempWord.removeLeading(QUOTES);
        if (!tempWord.isAlpha())
        {
            listOfFirstWords.removeAt(i);
            lastPosition--;
        }
        else
        {
            listOfFirstWords[i] = tempWord.getString();
            i++;
        }
    }
}

void mostCommonName::sortOnWords()
{
    if (listOfFirstWords.size() <= 1)
        return;

    int i;
    int lastPosition = listOfFirstWords.size() - 1;
    QString tempWord;

    while (lastPosition > 0)
    {
        i = 0;
        while (i < lastPosition)
        {
            if (listOfFirstWords[i].compare(listOfFirstWords[i+1]) > 0)
            {
                tempWord = listOfFirstWords[i];
                listOfFirstWords[i] = listOfFirstWords[i+1];
                listOfFirstWords[i+1]= tempWord;
            }
            i++;
        }
        lastPosition--;
    }
}

void mostCommonName::sortOnFrequency()
{
    // Sorts from largest to smallest

    if (frequencyCounts.size() <= 1)
        return;

    unsigned int tempCount;
    int i;
    int lastPosition = frequencyCounts.size() - 1;
    QString tempWord;

    while (lastPosition > 0)
    {
        i = 0;
        while (i < lastPosition)
        {
            if (frequencyCounts[i] < frequencyCounts[i+1])
            {
                tempCount = frequencyCounts[i];
                frequencyCounts[i] = frequencyCounts[i+1];
                frequencyCounts[i+1]= tempCount;

                tempWord = listOfFirstWords[i];
                listOfFirstWords[i] = listOfFirstWords[i+1];
                listOfFirstWords[i+1]= tempWord;
            }
            i++;
        }
        lastPosition--;
    }

    SetupCompleted = true;
}

void mostCommonName::consolidateWords()
{
    int i;
    int numRecs = listOfFirstWords.size();
    if (numRecs == 0)
        return;

    frequencyCounts.clear();
    for (i = 0; i < numRecs; i++)
        frequencyCounts.append(1);

    i = 0;
    while (i < (listOfFirstWords.size() - 1))
    {
        if (listOfFirstWords[i] == listOfFirstWords[i+1])
        {
            frequencyCounts[i] = frequencyCounts[i] + frequencyCounts[i+1];
            frequencyCounts.removeAt(i+1);
            listOfFirstWords.removeAt(i+1);
        }
        else
            i++;
    }
}

NAMEINFO mostCommonName::readMostCommonName(GLOBALVARS *gv)
{
    // This function looks at the first word of each sentence
    // Reads through listOfFirstWords, which was previously filled in and partially cleansed of common words
    // Initial sorting and processing is assumed to have occurred as part of startsWithGenderWord function called earlier
    // Objective is to find a the person's commonly used name, as opposed to formal name

    databaseSearches dbSearch;
    NAMEINFO nameInfo;
    QString potentialName;
    OQString word;
    bool isGivenName, isCommonlyUsedName, isFormalName;
    int i;

    int origSize = listOfFirstWords.size();
    if (origSize == 0)
        return nameInfo;

    if (!SetupCompleted)
    {
        cleanUpList();
        sortOnWords();
        consolidateWords();
        sortOnFrequency();
    }

    /*if ((gv->globalDr->getID() == 1388647) || (gv->globalDr->getID() == 4379220))
    {
        unsigned int br8k = 2;
    }*/

    i = 0;
    isGivenName = false;
    isCommonlyUsedName = false;
    isFormalName = false;
    int lastRec = listOfFirstWords.size();

    while (!isGivenName && (i < lastRec))
    {
        word = OQString(listOfFirstWords.at(i));
        word.removeEnding(",");
        if ((word.getLength() > 1) && word.isAlpha() && !word.isPronoun() && !gv->globalDr->isASavedName(word))
        {
            potentialName = word.getString();
            isGivenName = dbSearch.givenNameLookup(potentialName, gv, gv->globalDr->getGender());
            if (isGivenName)
            {
                isCommonlyUsedName = !OQString(potentialName).isProblematicFirstName() &&
                                     (gv->globalDr->isAMiddleName(word) || gv->globalDr->isANickName(word));
                if (!isCommonlyUsedName)
                    isFormalName = gv->globalDr->isAFormalName(word);
            }

            if ((potentialName.size() < 7) && !isFormalName && !isCommonlyUsedName && !word.isRecognized() && !word.isRecognizedFirstWord() && !word.isGenderWord() && !word.isWrittenMonth(gv->globalDr->getLanguage()) && !gv->globalDr->isASavedName(word.proper()))
            {
                PQString warningMessage;
                warningMessage << "Most common first word \"" << potentialName << "\" was rejected for: \"" << gv->globalDr->getFullName() << "\"  source: " << gv->globalDr->getURL();
                gv->logMsg(DefdErrorRecord, warningMessage);

                isGivenName = false;
            }
        }

        if (!(isCommonlyUsedName || isFormalName))
            i++;
    }

    if (isCommonlyUsedName || isFormalName)
    {
        nameInfo.name = PQString(potentialName).proper();
        nameInfo.numWords = nameInfo.name.countWords();
        nameInfo.type = ntFirst;
        nameFreq = frequencyCounts.at(i);
    }

    return nameInfo;
}

QString mostCommonName::readPotentialFirstName(GLOBALVARS *gv)
{    
    // This function looks at the first word of each sentence
    // Reads through listOfFirstWords, which was previously filled in and partially cleansed of common words
    // Initial sorting and processing is assumed to have occurred, or that processing is triggered
    // Objective is to find the first potential given name that starts a sentence

    databaseSearches dbSearch;
    QString potentialName;
    OQString word;
    bool isGivenName;
    int i;

    int origSize = listOfFirstWords.size();
    if (origSize == 0)
        return potentialName;

    if (!SetupCompleted)
    {
        cleanUpList();
        sortOnWords();
        consolidateWords();
        sortOnFrequency();
    }

    i = 0;
    isGivenName = false;
    int lastRec = listOfFirstWords.size();

    while (!isGivenName && (i < lastRec))
    {
        word = OQString(listOfFirstWords.at(i));
        word.removeEnding(",");
        if ((word.getLength() > 1) && word.isAlpha() && !word.isWrittenMonth())
        {
            potentialName = word.getString();
            isGivenName = dbSearch.givenNameLookup(potentialName, gv, gv->globalDr->getGender());
            if (!isGivenName || (potentialName == QString("elle")))
                potentialName.clear();
        }
        i++;
    }

    return potentialName;
}

GENDER mostCommonName::startsWithGenderWord(GLOBALVARS *gv)
{
    GENDER potentialGender = genderUnknown;
    bool potentialMale = false;
    bool potentialFemale = false;
    int maleCount = 0;
    int femaleCount = 0;

    int origSize = listOfFirstWords.size();
    if (origSize == 0)
        return potentialGender;

    if (!SetupCompleted)
    {
        cleanUpList();
        sortOnWords();
        consolidateWords();
        sortOnFrequency();
    }

    int i = 0;
    bool genderWord = false;

    LANGUAGE language = gv->globalDr->getLanguage();
    OQString potentialGenderWord;

    while (i < listOfFirstWords.size())
    {
        potentialGenderWord = OQString(listOfFirstWords.at(i));
        genderWord = potentialGenderWord.isGenderWord(genderUnknown, language) || potentialGenderWord.isTitle(language, genderUnknown);
        if (genderWord)
        {
            listOfFirstWords.removeAt(i);
            if (potentialGenderWord.isMaleGenderWord(language) || potentialGenderWord.isMaleTitle(language))
            {
                potentialMale = true;
                maleCount += frequencyCounts[i];
            }
            else
            {
                if (potentialGenderWord.isFemaleGenderWord(language) || potentialGenderWord.isFemaleTitle(language))
                {
                    potentialFemale = true;
                    femaleCount += frequencyCounts[i];
                }
            }
        }
        else
            i++;
    }

    // Default to female if any exists as in any conflicts male references likely refer to god
    if (potentialFemale && !potentialMale)
        potentialGender = Female;
    else
    {
        if (potentialMale && !potentialFemale)
            potentialGender = Male;
    }

    return potentialGender;
}

void mostCommonName::clear()
{
    listOfFirstWords.clear();
    frequencyCounts.clear();
    nameFreq = 0;
    SetupCompleted = false;
}


