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

NAMEINFO mostCommonName::readMostCommonName(GLOBALVARS *gv, QString structuredNamesProcessed)
{
    // Function is called twice, once before and once structured read

    // This function looks at the first word of each sentence
    // Reads through listOfFirstWords, which was previously filled in and partially cleansed of common words
    // Initial sorting and processing is assumed to have occurred as part of startsWithGenderWord function called earlier
    // Objective is to find the person's used name that (i) was included in structured names and (ii) was used in unstructured obit

    databaseSearches dbSearch;
    NAMESTATS nameStats;
    NAMEINFO nameInfo;
    QString potentialName, usedFirstNameFromStructured, usedFirstNameFromUnstructured, title;
    OQString word;
    bool isGivenName, isCommonlyUsedName, isFormalName, isUsedName, isUsedUnstructuredName, structuredSet, unstructuredSet;
    int i;
    int freq;

    QRegularExpression targetI;
    QRegularExpressionMatch match;
    targetI.setPatternOptions(QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);

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

    i = 0;
    isGivenName = false;
    isCommonlyUsedName = false;
    isFormalName = false;

    usedFirstNameFromStructured = gv->globalDr->getUsedFirstNameFromStructured();
    usedFirstNameFromUnstructured = gv->globalDr->getUsedFirstNameFromUnstructured();

    structuredSet = (usedFirstNameFromStructured.length() > 0);
    unstructuredSet = false;  // Set to false even with a prior mcn match to capture case where first real usage is mid sentence
    title = structuredNamesProcessed;
    int lastRec = listOfFirstWords.size();

    while (!unstructuredSet && (i < lastRec))
    {
        freq = frequencyCounts.at(i);
        word = OQString(listOfFirstWords.at(i)).proper();
        word.removeEnding(",");

        isUsedUnstructuredName = (word == usedFirstNameFromUnstructured);
        isUsedName = isUsedUnstructuredName || gv->globalDr->isASavedName(word);
        if (!isUsedName)
        {
            // This will always be true on first call of function
            targetI.setPattern("\\b" + word.getString() + "\\b");
            match = targetI.match(title);
            isUsedName = match.hasMatch();
            /*if (!isUsedName)
            {
                // problem with extra words in title like "residence"
                title = gv->globalDr->getTitle().getString();
                match = targetI.match(title);
                isUsedName = match.hasMatch();
            }*/
        }

        if ((word.getLength() > 1) && word.isAlpha() && !word.isPronoun() && !(word.isWrittenMonth() && (freq == 1)))
        {
            potentialName = word.getString();
            dbSearch.nameStatLookup(potentialName, gv, nameStats, gv->globalDr->getGender());
            isGivenName = nameStats.isGivenName && !word.isFoundIn(OQString::problematicFirstNames);

            if (!structuredSet && isUsedName && ((isGivenName && !nameStats.isLikelySurname) || (freq >= 3)))
            {
                usedFirstNameFromStructured = potentialName;
                gv->globalDr->setUsedFirstNameFromStructured(usedFirstNameFromStructured);
                structuredSet = true;
            }

            if ((isGivenName || isUsedName)  && !gv->globalDr->isASavedName(word) && (potentialName != gv->globalDr->getUsedFirstNameFromStructured()))
            {
                isCommonlyUsedName = !OQString(potentialName).isProblematicFirstName() &&
                                     (gv->globalDr->isAMiddleName(word) || gv->globalDr->isANickName(word));
                if (!isCommonlyUsedName)
                    isFormalName = gv->globalDr->isAFormalName(word) || (isUsedName && !nameStats.isLikelySurname);
            }
        }

        if (isCommonlyUsedName || isFormalName)
        {
            nameInfo.name = PQString(potentialName).proper();
            nameInfo.numWords = nameInfo.name.countWords();
            nameInfo.type = ntFirst;
            nameFreq = frequencyCounts.at(i);

            usedFirstNameFromUnstructured = potentialName;
            gv->globalDr->setUsedFirstNameFromUnstructured(usedFirstNameFromUnstructured);
            unstructuredSet = true;
        }
        else
        {
            i++;
            if (isUsedUnstructuredName)
                unstructuredSet = true;
        }
    }

    return nameInfo;
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


