// qUnstructuredContent.cpp

#include "qUnstructuredContent.h"
#include "readObit.h"

GLOBALVARS* unstructuredContent::globals = nullptr;

unstructuredContent::unstructuredContent() : OQStream(), contentLanguage(language_unknown), contentGender(genderUnknown), numEnglishDates(0), numFrenchDates(0), numSpanishDates(0), realSentenceEncountered(false)
{
}

unstructuredContent::unstructuredContent(const OQString source) : OQStream(source), contentLanguage(language_unknown), contentGender(genderUnknown), numEnglishDates(0), numFrenchDates(0), numSpanishDates(0), realSentenceEncountered(false)
{    
}

unstructuredContent::unstructuredContent(const unstructuredContent &orig)
{
    itsString = orig.getString();
    copyKeyVariablesFrom(orig);

    position = 0;
    if (itsString.length() > 0)
        EOS = false;
    else
        EOS = true;

    dateToday = orig.dateToday;
    globals = orig.globals;

    sentenceStartPositions = orig.sentenceStartPositions;
    lastSentenceStartPosition = orig.lastSentenceStartPosition;
    realSentenceEncountered = orig.realSentenceEncountered;
}

unstructuredContent::~unstructuredContent()
{	
}

void unstructuredContent::determineLanguageAndGender(OQStream &cleanedUpUC, OQStream &justInitialNamesUC)
{
	// While determining language, a few other tasks are also completed
	//	1. Gender is determined if possible
	//  2. The internal string is cleaned up by dropping off mostly punctuation (implemented via "tempString")
    //  3. A separate "cleanedUpString" is created by removing generic words, but punctuation is left in (or inserted) to retain sentence structure
    //  4. A list of names ("justInitialNames") is created by saving all words until a matched keyword is read in or a divider is encountered (does not check against list of known names as no earlier processing is assumed)

    LANGUAGE lang = globals->globalDr->getLanguage();  // Occasionally this will be known
    LANGUAGE firstLanguageEncountered = language_unknown;   // Used for bilingual obits
    unsigned int languageThresholdCount = 5;

    unsigned int wordLength, numDates, totalWordCount, numCommas;
    unsigned int englishCount, frenchCount, spanishCount;
    int numMaleEnglishWords, numFemaleEnglishWords, numMaleFrenchWords, numFemaleFrenchWords, numMaleSpanishWords, numFemaleSpanishWords;
	bool englishMatch, frenchMatch, spanishMatch;
    OQString word, originalWord, nextWord, twoWords, lastChar, firstChar, justNames, checkCap;
    OQString cleanedUpString, justInitialNames;
    bool inQuotes, inParentheses, isInitial, containsDOBandDOD, namesFinished, wordTerminatedWithPeriod, wordTerminatedWithColon, capitalized;
    bool relationshipWordsMatch, lastRelationshipWordsMatch;
    bool dividerBeforeWord, dividerAfterWord, standAlonePeriod, standAloneComma, wasPossessed, firstWord;
    bool mustAllBeAlpha = false;

    numCommas = 0;
    totalWordCount = 0;

    QString comma(",");
    QString period(".");
    QString space(" ");
    QString colon(":");
    OQString tempString, cleanString, nextChar;
    OQStream tempStream(itsString);
    QList<QString> firstNameList = globals->globalDr->getFirstNameList();

    QList<QString> listOfNames;
    QString plainName;

    englishCount = frenchCount = spanishCount = 0;
    numMaleEnglishWords = numFemaleEnglishWords = numMaleFrenchWords = numFemaleFrenchWords = numMaleSpanishWords = numFemaleSpanishWords = 0;
	namesFinished = false;
    firstWord = true;
    lastRelationshipWordsMatch = relationshipWordsMatch = false;
 
	while (!tempStream.isEOS())
	{
        dividerBeforeWord = dividerAfterWord = standAloneComma = standAlonePeriod = containsDOBandDOD = isInitial = false;

        originalWord = tempStream.getWord(true);
        totalWordCount++;
		while ((originalWord.getLength() == 0) && !tempStream.isEOS())
		{
			originalWord = tempStream.getWord(true);
		}
        nextChar = tempStream.peekAtNext(1);

        originalWord.removeEnding(CCRLF);
		word = originalWord.lower();
        twoWords = word + space + tempStream.peekAtWord(true).lower();
        if (originalWord.isAllCaps(mustAllBeAlpha))
            justNames = originalWord.proper();
        else
            justNames = originalWord;
        wasPossessed = justNames.removePossessive();

        /*************************************************/
        /* Run some checks to get information about word */
        /* to assist with picking off initial names only */
        /*************************************************/

        inQuotes = word.removeBookEnds(QUOTES);
        inParentheses = word.removeBookEnds(PARENTHESES);

        // Check for potential (DOB - DOD)
        if ((globals->globalDr->missingDOB() || globals->globalDr->missingDOD()) && (totalWordCount < 10))
        {
            if (inParentheses && (word.countWords(HYPHENATED, ALPHANUMERIC) >=2))
            {
                QList<QDate> dateList;
                DATES dates;
                unstructuredContent tempUC(word);
                LANGUAGE lang;
                tempUC.copyKeyVariablesFrom(*this);

                // Start with complete dates  (mmm dd, yyyy - mmm dd, yyyy)
                unsigned int i = 1;
                while ((i <= 3) && !containsDOBandDOD)
                {
                    lang = static_cast<LANGUAGE>(i);
                    numDates = tempUC.pullOutDates(lang, dateList, 2, cleanString, true);
                    if (numDates >= 1)
                    {
                        if (numDates == 1)
                        {
                            unsigned int maxSentences = 2;
                            dates = tempUC.contentKeyWordsAndDates(firstNameList, lang, maxSentences);
                            globals->globalObit->processNewDateInfo(dates, 3);
                        }
                        else
                        {
                            dates = tempUC.readDOBandDOD();
                            globals->globalObit->fillInDatesStructured(tempUC);
                        }
                        containsDOBandDOD = true;
                        namesFinished = true;
                    }
                    i++;
                }

                // Check for just years  (yyyy - yyyy)
                if (!containsDOBandDOD)
                {
                    dates = tempUC.readYears();
                    if (dates.hasDateInfo())
                    {
                        globals->globalObit->processNewDateInfo(dates, 2);
                        containsDOBandDOD = true;
                        namesFinished = true;
                    }
                }
            }            
        }

        // Gather other info for justInitialNames
        firstChar = justNames.left(1);
        lastChar = justNames.right(1);
        checkCap = originalWord;
        checkCap.removeBookEnds(QUOTES);
        checkCap.removeBookEnds(PARENTHESES);
        capitalized = checkCap.isCapitalized();

        if (inParentheses || inQuotes)
        {
            if ((wordTerminatedWithColon = (nextChar == colon)))
                nextChar = tempStream.get();  // used to move position up
        }
        else
            wordTerminatedWithColon = (lastChar == colon);

        if (wordTerminatedWithColon)
        {
            justNames.removeEnding(colon);
            justNames += comma;
        }

        wordLength = word.getLength();

        // Used to identify sentences as well
        if (lastChar == period)
        {
            if (!(word.isTitle() || word.isSaint() || word.isPrefix() || word.isSuffix()))
                wordTerminatedWithPeriod = true;
            else
                wordTerminatedWithPeriod = false;

            if (wordLength == 1)
                standAlonePeriod = true;

            if (wordLength == 2)
                isInitial = true;
        }
		else
        {
			wordTerminatedWithPeriod = false;
            if (capitalized && (wordLength == 1))
                isInitial = true;
        }

        // If second comma is encountered, consider it a divider
        if ((lastChar == comma) || wordTerminatedWithColon)
        {
            numCommas++;
            if (numCommas >= 2)
            {
                justNames.dropRight(1);
                dividerAfterWord = true;
            }

            if (wordLength == 1)
                standAloneComma = true;
        }

        // Look for other dividers
        // a. lastname-lastname (i.e. hyphenated) name won't have lastChar as divider, so no "natural" break triggered between the names
        // b. lastname- Text will trigger a break due to space
        // c. lastname - Text will trigger a break due to hyphen
        // d. lastname -Text will trigger a break due to space

        if (!namesFinished && word.isHyphenated())  // Potential for case a. where lastname-NOTlastname
        {
            QString fullName, firstPart, secondPart;
            OQString singlePart, singlePartLower, tempWord;
            int i;

            tempWord = originalWord;
            tempWord.removeEnding(PUNCTUATION);
            fullName = tempWord.getString();
            i = fullName.indexOf(QChar(45));
            if (i == -1)
                i = fullName.indexOf(QChar(8211));
            firstPart = fullName.left(i);
            secondPart = fullName.right(static_cast<int>(tempWord.getLength()) - (i+1));
            // Check for second hyphen
            if (PQString(secondPart).isHyphenated())
            {
                i = secondPart.indexOf(QChar(45));
                if (i == -1)
                    i = secondPart.indexOf(QChar(8211));
                secondPart = secondPart.left(i);
            }
            singlePart = OQString(secondPart);
            singlePart.cleanUpEnds();
            singlePartLower = singlePart.lower();

            if (singlePartLower.isRecognized())
            {
                justNames = OQString(firstPart);
                justNames.cleanUpEnds();
                originalWord = justNames + OQString(" - ") + singlePart;
                dividerAfterWord = true;
            }
        }

        if ((lastChar.getCharType() & DIVIDER) == DIVIDER)  // cases b. and c.
        {
            justNames.dropRight(1);
            if (word.getLength() > 1)
            {
                OQString divider(" ");
                divider += word.right(1);
                word.dropRight(1);
                originalWord = originalWord.left(originalWord.getLength() - 1) + divider;
            }
            dividerAfterWord = true;
        }

        if ((firstChar.getCharType() & DIVIDER) == DIVIDER)  // cases c. and d.
        {
            justNames.dropLeft(1);
            if (word.getLength() > 1)
            {
                OQString divider = word.right(1);
                divider += OQString(" ");
                word.dropLeft(1);
                originalWord = divider + originalWord.right(originalWord.getLength() - 1);
            }
            dividerBeforeWord = true;
        }

        /*************************************************/
        /*                Analyze word                   */
        /*************************************************/

        word.removeEnding(PUNCTUATION);
        word.removeEnding(CCRLF);
        wordLength = word.getLength();
		if (word.isAlpha() && (wordLength > 0))
		{
			englishMatch = false;
			frenchMatch = false;
			spanishMatch = false;

            if (word.isGodReference())
                globals->globalObit->setGodReference(true);
			
            // First run is through common words in each language (excluding gender), counting matches along the way
            if (word.isEnglish())
            {
                englishMatch = true;
                englishCount++;
                if ((firstLanguageEncountered == language_unknown) && (englishCount >= languageThresholdCount))
                    firstLanguageEncountered = english;
            }

            if (word.isFrench())
            {
                frenchMatch = true;
                frenchCount++;
                if ((firstLanguageEncountered == language_unknown) && (frenchCount >= languageThresholdCount))
                    firstLanguageEncountered = french;
            }

            if (word.isSpanish())
            {
                spanishMatch = true;
                spanishCount++;
                if ((firstLanguageEncountered == language_unknown) && (spanishCount >= languageThresholdCount))
                    firstLanguageEncountered = spanish;
            }

            // Second run is through gender specific words in each language, counting language and gender matches along the way
            if (word.isEnglishMale())
            {
                englishMatch = true;
                englishCount++;
                numMaleEnglishWords++;
                if ((firstLanguageEncountered == language_unknown) && (englishCount >= languageThresholdCount))
                    firstLanguageEncountered = english;
            }

            if (word.isEnglishFemale())
            {
                englishMatch = true;
                englishCount++;
                numFemaleEnglishWords++;
                if ((firstLanguageEncountered == language_unknown) && (englishCount >= languageThresholdCount))
                    firstLanguageEncountered = english;
            }

            if (word.isFrenchMale())
            {
                frenchMatch = true;
                frenchCount++;
                numMaleFrenchWords++;
                if ((firstLanguageEncountered == language_unknown) && (frenchCount >= languageThresholdCount))
                    firstLanguageEncountered = french;
            }

            if (word.isFrenchFemale())
            {
                frenchMatch = true;
                frenchCount++;
                numFemaleFrenchWords++;
                if ((firstLanguageEncountered == language_unknown) && (frenchCount >= languageThresholdCount))
                    firstLanguageEncountered = french;
            }

            if (word.isSpanishMale())
            {
                spanishMatch = true;
                spanishCount++;
                numMaleSpanishWords++;
                if ((firstLanguageEncountered == language_unknown) && (spanishCount >= languageThresholdCount))
                    firstLanguageEncountered = spanish;
            }

            if (word.isSpanishFemale())
            {
                spanishMatch = true;
                spanishCount++;
                numFemaleSpanishWords++;
                if ((firstLanguageEncountered == language_unknown) && (spanishCount >= languageThresholdCount))
                    firstLanguageEncountered = spanish;
            }

			// Deal with all the matched "regular" words
            if (englishMatch || frenchMatch || spanishMatch)
			{
				// All matched words get copied into "tempString" in lower case form
                tempString += space;
                tempString += originalWord.lower();

				// All matched words which are keywords get copied into "cleanedUpString" as well in lower case form
				bool keyWordMatch = false;

				// If matched but a birth, death or age word, keep the lower case version as it is a keyword to be referenced later
                if (word.isPronoun() || word.isBirthWord(lang) || word.isDeathWord(lang) || word.isDeathIndicator(lang) || word.isAgeWord(lang) || word.isOtherPersonReference(lang) || word.isMiscKeeperWord(lang))
                    keyWordMatch = true;

                // Keep the relationship double words
                lastRelationshipWordsMatch = relationshipWordsMatch;
                relationshipWordsMatch = twoWords.areRelationshipWords(globals->globalDr->getGender(), lang);
                if (relationshipWordsMatch)
                {
                    cleanedUpString += space;
                    cleanedUpString += twoWords;
                }

                // Keep lowercase version of word if matched - extra coding to avoid double inclusion
                if (keyWordMatch && !(relationshipWordsMatch || lastRelationshipWordsMatch))
				{
                    cleanedUpString += space;
                    cleanedUpString += originalWord;
				}

				// To maintain sentence structure, add "period" back in to cleanedUpString if it was a matched word but not a keyword match
				if (!keyWordMatch && wordTerminatedWithPeriod)
				{
                    cleanedUpString += period;
				}

				// Because a matched word is found, assume name is finished
				namesFinished = true;
			}
            else  	// If word doesn't match any language, or if word is a date, keep the original (with original formatting except ALL CAPS)
			{
                tempString += space;
                tempString += originalWord;

                if (!word.isTitle() && !containsDOBandDOD)
                {
                    cleanedUpString += space;
                    if (originalWord.isAllCaps(mustAllBeAlpha) && !originalWord.isSuffixAllCaps())
                        cleanedUpString += originalWord.proper();
                    else
                        cleanedUpString += originalWord;
                }

				// Check if the unmatched word is a date
				// Additional checking in english since alpha dates start with months and could conflict with names June, April
                if (!namesFinished && word.isWrittenMonth(english))
				{
					nextWord = tempStream.peekAtWord(true);
					nextWord.removeEnding(PUNCTUATION);
                    nextWord.removeEnding(CCRLF);
                    OQString ending = nextWord.right(2);
                    if (ending.isOrdinal(english))
                        nextWord.dropRight(2);
					if (nextWord.isNumeric() && (nextWord.asNumber() > 0) && (nextWord.asNumber() <= 31))
					{
						numEnglishDates++;
						namesFinished = true;
					}
                    else
                    {
                        if (nextWord.isNumeric() && (nextWord.asNumber() > 1890) && (nextWord.asNumber() < globals->today.year()))
                            namesFinished = true;
                    }
				}
				
                if (!namesFinished && word.isWrittenMonth(french))
				{
					nextWord = tempStream.peekAtWord(true);
					nextWord.removeEnding(PUNCTUATION);
                    nextWord.removeEnding(CCRLF);
                    OQString ending = nextWord.right(4);
                    if (ending.isOrdinal(french))
                        nextWord.dropRight(4);
                    if (nextWord.isNumeric() && (nextWord.asNumber() > 1890) && (nextWord.asNumber() < globals->today.year()))
					{
						numFrenchDates++;
						namesFinished = true;
					}
				}

                if (!namesFinished && word.isWrittenMonth(spanish))
				{
					nextWord = tempStream.peekAtWord(true);
					nextWord.removeEnding(PUNCTUATION);
                    nextWord.removeEnding(CCRLF);
                    OQString ending = nextWord.right(2);
                    if (ending.isOrdinal(spanish))
                        nextWord.dropRight(2);
                    else
                    {
                        ending = nextWord.right(1);
                        if (ending.isOrdinal(spanish))
                            nextWord.dropRight(1);
                    }
                    if (nextWord.isNumeric() && (nextWord.asNumber() > 1890) && (nextWord.asNumber() < globals->today.year()))
					{
						numSpanishDates++;
						namesFinished = true;
					}
				}

                // Deal with sites which sometimes include city of residence as part of name
                unsigned int skipCode = lookupFHspecialCode(globals, globals->globalDr->getProvider(), globals->globalDr->getProviderKey());
                if (skipCode == 1)
                {
                    if (word.isLocation())
                        namesFinished = true;
                }

                // Save words in separate string until a matched word or divider is encountered,
                // the expectation is that it should just be names
                if (!namesFinished)
				{
                    // Check for odd ball stuff
                    OQString wordCheck = justNames;
                    wordCheck.removeEnding(PUNCTUATION);
                    wordCheck.removeEnding(CCRLF);
                    wordCheck.removeBookEnds(PARENTHESES | QUOTES);
                    wordCheck.removeInternalPeriods();
                    if (wordCheck.isAltNameIndicator())
                    {
                        namesFinished = true;
                        if (wordCheck.isNeeEtAl())
                        {
                            OQString possibleMaidenName;
                            bool validName = false;
                            unsigned int i = 1;
                            nextWord = tempStream.peekAtWord(false, i);
                            possibleMaidenName = nextWord;
                            while (nextWord.isCompoundName())
                            {
                                validName = true;
                                i++;
                                possibleMaidenName += space;
                                nextWord = tempStream.peekAtWord(false, i);
                                possibleMaidenName += nextWord;
                            }

                            while (nextWord.isAboriginalName(tempStream.peekAtWord(false, i+1)))
                            {
                                validName = true;
                                possibleMaidenName += nextWord;
                                possibleMaidenName += space;
                                i++;
                                nextWord = tempStream.peekAtWord(false, i);
                            }

                            possibleMaidenName.removeEnding(PUNCTUATION);
                            possibleMaidenName.removeEnding(CCRLF);
                            possibleMaidenName.compressCompoundNames();
                            validName = validName || possibleMaidenName.isCapitalized();
                            if (validName)
                            {
                                NAMEINFO ni;
                                ni.name = possibleMaidenName.getString();
                                ni.type = ntLast;
                                ni.numWords = ni.name.countWords();
                                globals->globalDr->setAlternates(ni);

                                if (possibleMaidenName.isFoundIn(problematicFemaleMiddleNames, 1))
                                    globals->globalDr->wi.nameWarningException = true;
                            }
                        }
                    }

                    // Eliminate anything not capitalized (with exceptions)
                    if (!namesFinished && !capitalized && !word.isUncapitalizedName())
                    {
                        OQStream tmpStream(word);
                        OQString tmpWordA, tmpWordB;
                        tmpWordA = tmpStream.getWord();
                        if (!tmpWordA.isNeeEtAl())
                            namesFinished = true;
                        else
                        {
                            tmpWordA = tmpStream.getWord();
                            tmpWordB = tmpWordA;
                            while (!tmpStream.isEOS() && (tmpWordA.isUncapitalizedName() || tmpWordA.isAboriginalName(tmpStream.peekAtWord())))
                            {
                                tmpWordA = tmpStream.getWord();
                                tmpWordB += tmpWordA;
                            }
                            if (surnameLookup(tmpWordB.getString(), globals) == 0)
                                namesFinished = true;
                        }
                    }

                    // Check for word with possessive removed, as it would not have been recognized (e.g. It's)
                    if (wasPossessed && justNames.isRecognized())
                        namesFinished = true;

                    // Check for new sentence starting with "On", which is also a chinese name
                    if (originalWord == OQString("On"))
                    {
                        nextWord = tempStream.peekAtWord(true);
                        nextWord.removeEnding(PUNCTUATION);
                        if (nextWord.isWrittenMonth(english) || nextWord.isWrittenDayOfWeek(english))
                            namesFinished = true;
                    }

                    if (dividerBeforeWord)
                        namesFinished = true;

                    if (!namesFinished)
                    {
                        tempString = justNames;
                        tempString.removeEnding(PUNCTUATION);
                        tempString.removeBookEnds(PARENTHESES | QUOTES);
                        tempString.removeLeadingNeeEtAl();
                        plainName = tempString.lower().getString();
                        if (listOfNames.contains(plainName))
                            namesFinished = true;
                        else
                        {
                            if (firstWord && wordTerminatedWithColon)
                            {
                                // Except for few exceptions, just skip word and keep going like it wasn't there
                                if (globals->globalDr->getNumFamilyNames() == 0)
                                {
                                    NAMESTATS nameStats;
                                    nameStatLookup(plainName, globals, nameStats, genderUnknown);
                                    if (nameStats.isLikelySurname)
                                        justInitialNames += justNames;
                                }
                            }
                            else
                            {
                                listOfNames.append(plainName);
                                justInitialNames += justNames;
                                if (dividerAfterWord || (wordTerminatedWithPeriod && !isInitial))
                                    namesFinished = true;
                                else
                                    justInitialNames += space;
                            }
                        }
                    }
				}
			}

		}  // end if word isAlpha
		else
		{
			// Since the word is not "alpha", it can't be a name and names must be finished
            if(!standAloneComma)
                namesFinished = true;
			
            if (!(standAloneComma || standAlonePeriod))
            {
                // Save nearly all other words to both strings
                if (word.isNumeric() || word.isAlphaNumeric() || word.removeLeadingNeeEtAl())
                {
                    tempString += space;
                    tempString += originalWord;

                    cleanedUpString += space;
                    cleanedUpString += originalWord;
                }
                else
                {
                    if (wordTerminatedWithPeriod)
                    {
                        lastChar = tempString.right(1);
                        if (lastChar.getCharType() == PUNCTUATION)
                            tempString.dropRight(1);
                        tempString += period;

                        lastChar = cleanedUpString.right(1);
                        if (lastChar.getCharType() == PUNCTUATION)
                            cleanedUpString.dropRight(1);
                        cleanedUpString += period;
                    }
                }
            }
            else
            {
                // Add punctuation to string after deleting any preceding punctuation
                if (standAlonePeriod)
                {
                    // Always replace prior punctuation
                    lastChar = tempString.right(1);
                    if (lastChar.getCharType() == PUNCTUATION)
                        tempString.dropRight(1);
                    tempString += originalWord;

                    lastChar = cleanedUpString.right(1);
                    if (lastChar.getCharType() == PUNCTUATION)
                        cleanedUpString.dropRight(1);
                    cleanedUpString += originalWord;

                    if (!namesFinished)
                    {
                        lastChar = justInitialNames.right(1);
                        if (lastChar.getCharType() == PUNCTUATION)
                            justInitialNames.dropRight(1);

                        plainName = originalWord.lower().getString();
                        if (listOfNames.contains(plainName))
                            namesFinished = true;
                        else
                        {
                            listOfNames.append(plainName);
                            justInitialNames += originalWord;
                        }
                    }
                }

                if (standAloneComma)
                {
                    // Only replace non sentence ending chars
                    lastChar = tempString.right(1);
                    if (lastChar.getCharType() == PUNCTUATION)
                        tempString.dropRight(1);
                    tempString += originalWord;

                    lastChar = cleanedUpString.right(1);
                    if (lastChar.getCharType() == PUNCTUATION)
                        cleanedUpString.dropRight(1);
                    cleanedUpString += originalWord;

                    if (!namesFinished)
                    {
                        lastChar = justInitialNames.right(1);
                        if (lastChar.getCharType() == PUNCTUATION)
                            justInitialNames.dropRight(1);

                        plainName = originalWord.lower().getString();
                        if (listOfNames.contains(plainName))
                            namesFinished = true;
                        else
                        {
                            listOfNames.append(plainName);
                            justInitialNames += originalWord;
                        }
                    }
                }

            }
		}

        firstWord = false;
	}  // end while

	// Copy cleaned up list of words to itsString
    tempString.removeLeading(space);
    //setItsString(tempString.getString());

    // Tidy up and store the other strings
    cleanedUpString.removeLeading(space);
    cleanedUpString.removeLeading(period);
    cleanedUpString.removeLeading(space);
    cleanedUpString.cleanUpEnds();
    cleanedUpUC = cleanedUpString;

    justInitialNames.removeLeading(space);
    justInitialNames.cleanUpEnds();
    justInitialNamesUC = justInitialNames;

	// Add in dates to counts while leaving them in the content
	englishCount += numEnglishDates;
	frenchCount += numFrenchDates;
	spanishCount += numSpanishDates;

    // Save male and female counts for further referencing where helpful
    globals->globalObit->setNumMaleWords(numMaleEnglishWords + numMaleFrenchWords + numMaleSpanishWords);
    globals->globalObit->setNumFemaleWords(numFemaleEnglishWords + numFemaleFrenchWords + numFemaleSpanishWords);

	// Determine language
    int numLanguages = 0;
    if (englishCount >= languageThresholdCount)
        numLanguages++;
    if (frenchCount >= languageThresholdCount)
        numLanguages++;
    if (spanishCount >= languageThresholdCount)
        numLanguages++;
    globals->globalDr->wi.bilingualFlag = numLanguages;

    if (firstLanguageEncountered != language_unknown)
        contentLanguage = firstLanguageEncountered;
    else
    {
        if ((englishCount >= frenchCount) && (englishCount >= spanishCount))
            contentLanguage = english;

        if ((frenchCount > englishCount) && (frenchCount >= spanishCount))
            contentLanguage = french;

        if ((spanishCount > frenchCount) && (spanishCount > englishCount))
            contentLanguage = spanish;

        if (!englishCount && !frenchCount && !spanishCount)
            contentLanguage = lang;
    }

    // Flag set if an expected outcome was not realized
    if ((lang != language_unknown) && (contentLanguage != lang))
        globals->globalDr->wi.bilingualFlag = numLanguages + 10;

    if (globals->globalDr->wi.bilingualFlag >= 2)
        globals->globalDr->setLanguage(multiple);

	// Determine gender
	switch (contentLanguage)
	{
    // Go with simple majority unless there are a lot of opposing gender words, in which case don't
    // classify and leave it for a follow-up query

	case french:
        if ((numMaleFrenchWords - numFemaleFrenchWords) > 3)
			contentGender = Male;
        if ((numFemaleFrenchWords - numMaleFrenchWords) > 3)
			contentGender = Female;
		break;

	case spanish:
        if ((numMaleSpanishWords - numFemaleSpanishWords) > 3)
			contentGender = Male;
        if ((numFemaleSpanishWords - numMaleSpanishWords) > 3)
			contentGender = Female;
		break;

	case english:
	default:
        if ((numMaleEnglishWords - numFemaleEnglishWords) > 3)
            contentGender = Male;
        if ((numFemaleEnglishWords - numMaleEnglishWords) > 3)
			contentGender = Female;
	}

    // Final check in case males references could be associated to god
    if (globals->globalObit->hasGodReference() && (contentGender == Male))
    {
        double unisex = 0;
        QString potentialName = globals->globalDr->getFirstName().getString();
        if (potentialName.size() == 0)
            potentialName = globals->globalDr->getPotentialFirstName();
        if (potentialName.size() == 0)
            potentialName = globals->globalDr->getTitleKey().getString();

        if (potentialName.size() > 0)
        {
            QList<QString> list;
            list.append(potentialName);
            unisex = genderLookup(list, globals);
        }

        if (unisex < 0.75)
        {
            int netFemale = globals->globalObit->getNumFemaleWords();
            if ((netFemale > 0) && (globals->globalObit->getNumMaleWords() > 15))
                netFemale--;

            switch(netFemale)
            {
            case 0:
                if (unisex < 0.025)
                    contentGender = Female;
                else
                {
                    if (unisex < 0.100)
                        contentGender = genderUnknown;
                }
                break;

            case 1:
                if (unisex < 0.050)
                    contentGender = Female;
                else
                {
                    if (unisex < 0.200)
                        contentGender = genderUnknown;
                }
                break;

            case 2:
            default:
                contentGender = Female;
                break;
            }
        }
    }
}

GENDER unstructuredContent::getGender() const
{
	if (contentGender != genderUnknown)
		return contentGender;
	else
        //TODO name search
		return contentGender;
}

LANGUAGE unstructuredContent::getLanguage() const
{
    return contentLanguage;
}

DATES unstructuredContent::sentencePullOutDates(const LANGUAGE lang, const unsigned int maxSentences)
{
    DATES resultDates;
    QList<QDate> dateList;
    unstructuredContent sentence;
    OQString cleanString;
    unsigned int maxDates = 3;
    bool limitWords = false;
    bool fullyCredible = false;
    unsigned int i = 1;

    while(!isEOS() && (i <= maxSentences) && !resultDates.hasDateInfo())
    {
        dateList.clear();
        sentence = getSentence(lang);
        sentence.pullOutDates(lang, dateList, maxDates, cleanString, limitWords);

        if (dateList.size() == 2)
        {
            if (dateList[0] < dateList[1])
            {
                resultDates.potentialDOB = dateList[0];
                resultDates.potentialDOD = dateList[1];
                resultDates.fullyCredible = fullyCredible;
            }
            else
            {
                resultDates.potentialDOB = dateList[1];
                resultDates.potentialDOD = dateList[0];
                resultDates.fullyCredible = fullyCredible;
            }
        }

        i++;
    }

    return resultDates;
}


unsigned int unstructuredContent::pullOutDates(const LANGUAGE lang, QList<QDate> &dateList, unsigned int maxDates, OQString &cleanString, bool limitWords, bool serviceDate)
{
	unsigned int numDates;

	switch (lang)
	{
	case french:
        numDates = pullOutFrenchDates(dateList, maxDates, cleanString, limitWords, serviceDate);
		break;

	case spanish:
        numDates = pullOutSpanishDates(dateList, maxDates, cleanString, limitWords, serviceDate);
		break;

	case english:
	default:
        numDates = pullOutEnglishDates(dateList, maxDates, cleanString, limitWords, serviceDate);
		break;

	}
	return numDates;
}

unsigned int unstructuredContent::pullOutEnglishDates(QList<QDate> &dateList, unsigned int maxDates, OQString &cleanString, bool limitWords, bool serviceDate, int forceYear)
{
    QDate testDate;
    OQString list, cumulativeGet;
    QListIterator<monthInfo> iter(list.monthsEnglish);

    int yyyy, mm, dd, tempNum;
    unsigned int wordLength;
    OQString word, lowerWord, originalWord, tempWord, lastChar;
    OQString space(" ");
    monthInfo mi;
    bool englishStyleDateStarted, frenchStyleDateStarted, stillOK;

	// Limit only applies in attempts to pull two dates
	bool overLimit = false;
    bool abandon = false;
	unsigned int wordLimit = 8;  // allows for two "words" to be dropped since each date is three words
	unsigned int numWordsRead = 0;
	unsigned int numValidDates = 0;
    unsigned int numPartialDates = 0;
    unsigned int partialDateMonth = 0;

    yyyy = mm = dd = 0;
	beg();		// go to beginning of stream

    while (!EOS && (numValidDates < maxDates) && !overLimit && !abandon)
	{
        cumulativeGet.clear();
        word = getWord(true);		// pull next word from content to check for month
        originalWord = word;
        cumulativeGet = originalWord + space;
        numWordsRead++;

        lastChar = word.right(1);
		if ((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION)
			word.dropRight(1);
        if (limitWords)
        {
            switch(numValidDates)
            {
            case 0:
                if (numWordsRead > static_cast<unsigned int>(wordLimit/2 - 2))		// Two offset to allow for day and year
                    overLimit = true;
                break;

            case 1:
            default:
                if (numWordsRead >(wordLimit - 2))		// Two offset to allow for day and year
                    overLimit = true;
                break;
            }
        }
		lowerWord = word.lower();
        englishStyleDateStarted = false;
        frenchStyleDateStarted = false;
        stillOK = false;

        // Loop through all potential alpha dates - stop if found or at end of list
        if (lowerWord.isAlpha())
        {
            // Assume any date after "service" or "celebration" is funeral date
            tempWord = lowerWord;
            tempWord.removeEnding("s");
            if (!serviceDate && tempWord.isServiceWord(english))
                abandon = true;

            iter.toFront();
            while (!englishStyleDateStarted && iter.hasNext())
            {
                mi = iter.next();
                if (lowerWord == mi.monthAlpha)
                {
                    englishStyleDateStarted = true;
                    mm = static_cast<int>(mi.monthNumeric);
                    stillOK = true;
                }
            }
        }
        else
        {
            tempNum = 0;
            if (word.isNumeric())
                tempNum = static_cast<int>(word.asNumber());
            else
            {
                if (word.isAlphaNumeric())
                    tempNum = static_cast<int>(word.drawOutNumber());
            }

            if ((tempNum >= 1) && (tempNum <= 31))
            {
                frenchStyleDateStarted = true;
                dd = tempNum;
                stillOK = true;
            }
        }

		// Read in day and year and attempt to validate
		if (stillOK)
		{
            stillOK = false;
            word = getWord(true);		// pull next word from content
            originalWord = word;

            if (word.getLength() > 0)
            {
                numWordsRead++;
                wordLength = word.getLength();

                lastChar = word.right(1);
                if ((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION)
                    word.dropRight(1);

                if (englishStyleDateStarted)
                {
                    dd = 0;

                    // Address rare instance of "August the 8th, 2020"
                    if (word.isAlpha() && (word == OQString("the")))
                    {
                        cumulativeGet += originalWord + space;
                        word = getWord(true);
                        originalWord = word;

                        if (word.getLength() > 0)
                        {
                            numWordsRead++;
                            wordLength = word.getLength();

                            lastChar = word.right(1);
                            if ((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION)
                                word.dropRight(1);
                        }
                    }

                    if (word.isNumeric())
                    {
                        dd = static_cast<int>(word.asNumber());
                        stillOK = true;
                        cumulativeGet += originalWord + space;
                    }
                    else
                    {
                        if (word.isAlphaNumeric())      // e.g., 3rd
                            dd = static_cast<int>(word.drawOutNumber());

                        if ((dd >= 1) && (dd <= 31))
                        {
                            stillOK = true;
                            cumulativeGet += originalWord + space;
                        }
                        else
                        {
                            if (!EOS)
                                backward(wordLength + 1);  // back up position by last word (plus space) and don't add to cumulativeGet
                            numWordsRead--;
                        }
                    }
                }

                if (frenchStyleDateStarted)
                {
                    lowerWord = word.lower();

                    if (lowerWord.isAlpha())
                    {
                        // Assume any date after "service" or "celebration" is funeral date
                        tempWord = lowerWord;
                        tempWord.removeEnding("s");
                        if (!serviceDate && tempWord.isServiceWord(english))
                            abandon = true;

                        iter.toFront();
                        stillOK = false;
                        while (!stillOK && iter.hasNext())
                        {
                            mi = iter.next();
                            if (lowerWord == mi.monthAlpha)
                            {
                                mm = static_cast<int>(mi.monthNumeric);
                                stillOK = true;
                                cumulativeGet += originalWord + space;
                            }
                        }
                    }

                    if (!stillOK && !EOS)
                    {
                        backward(wordLength + 1);  // back up position by last word (plus space) and don't add to cumulativeGet
                        numWordsRead--;
                    }
                }
            }

            if (stillOK)
			{
                word = getWord(true);		// pull next word (potential year) from content
                originalWord = word;
                numWordsRead++;
                wordLength = word.getLength();

                lastChar = word.right(1);
                if ((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION)
                    word.dropRight(1);

                // Make adjustments if hyphen was used between dates without spacing
                if (word.isHyphenated() || ((lastChar.getCharType() & HYPHENATED) == HYPHENATED))
                {
                    QString tempWord = word.getString();
                    int i = tempWord.indexOf(QChar(45));
                    if (i == -1)
                        i = tempWord.indexOf(QChar(8211));
                    unsigned int ui = static_cast<unsigned int>(i);
                    OQString firstPart = word.left(ui);
                    if (firstPart.isNumeric())
                    {
                        // Use first part only and reposition position after hyphen
                        backward(wordLength - ui);
                        word = firstPart;
                        originalWord = word;
                    }
                }

				if (word.isNumeric())
                {
                    yyyy = static_cast<int>(word.asNumber());

                    // Check for rare "...died on March 27 (at) 94 years .."
                    if ((yyyy > 0) && (yyyy < 125))
                    {
                        OQString peekNext = peekAtWord();
                        if (peekNext.isAgeWord())
                        {
                            // Following coding handles reporting of partial date, or creation of complete date, depending on "forceYear"
                            if (forceYear == 0)
                            {
                                if (globals->globalDr->getDeemedYOD() > 0)
                                {
                                    stillOK = true;
                                    yyyy = globals->globalDr->getDeemedYOD();
                                    maxDates++;
                                    cumulativeGet += originalWord + space;
                                }
                                else
                                {
                                    numPartialDates++;
                                    partialDateMonth = mi.monthNumeric;
                                }
                            }
                            else
                            {
                                stillOK = true;
                                yyyy = forceYear;
                                maxDates++;
                                cumulativeGet += originalWord + space;
                            }
                        }
                    }
                    else
                        cumulativeGet += originalWord + space;
                }
				else
				{
                    // Check if date was entered as Nov 9 th 1966 (i.e., space after number)
                    OQString ending = word.lower();
                    if (ending.isOrdinal(english))
                    {
                        word = getWord(true);		// pull next word (potential year) from content
                        originalWord = word;
                        numWordsRead++;

                        lastChar = word.right(1);
                        if ((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION)
                            word.dropRight(1);
                        if (word.isNumeric())
                        {
                            yyyy = static_cast<int>(word.asNumber());
                            cumulativeGet += originalWord + space;
                        }
                    }
                    else
                    {
                        stillOK = false;
                        if (!EOS)
                            backward(wordLength + 1);  // back up position by last word and don't add to cumulativeGet
                        numWordsRead--;

                        // Following coding handles reporting of partial date, or creation of complete date, depending on "forceYear"
                        if (forceYear == 0)
                        {
                            if (globals->globalDr->getDeemedYOD() > 0)
                            {
                                stillOK = true;
                                yyyy = globals->globalDr->getDeemedYOD();
                                maxDates++;
                                cumulativeGet += originalWord + space;
                            }
                            else
                            {
                                numPartialDates++;
                                partialDateMonth = mi.monthNumeric;
                            }
                        }
                        else
                        {
                            stillOK = true;
                            yyyy = forceYear;
                            maxDates++;
                        }
                    }
				}
			}

			// Validate any potential date
			if (stillOK)
			{
                bool reject = (yyyy > globals->today.year()) || (yyyy < 1875);
                testDate = QDate(yyyy, mm, dd);
                if (testDate.isValid() && !reject)
				{
                    dateList.append(testDate);
					numValidDates++;
                    cumulativeGet.clear();
				}
			}  // end if - date validation
        } // end if - reading in day and year

        cleanString += cumulativeGet;

    } // end while

    cleanString.removeEnding(SPACE);

    if (numValidDates == 0)
        cleanString = itsString;

    if ((numPartialDates == 1) && (numValidDates == 0))
        return 9900 + partialDateMonth;
    else
        return numValidDates;
}

unsigned int unstructuredContent::pullOutFrenchDates(QList<QDate> &dateList, unsigned int maxDates, OQString &cleanString, bool limitWords, bool serviceDate, int forceYear)
{
    QDate testDate;
    OQString list, cumulativeGet;
    QListIterator<monthInfo> iter(list.monthsFrench);

    int yyyy, mm, dd;
    unsigned int wordLength;
    OQString word, lowerWord, lastChar, originalWord, tempWord;
    OQString space(" ");
	bool dateStarted, stillOK, matched;
    monthInfo mi;

	// Limit only applies in attempts to pull two dates
	bool overLimit = false;
    bool abandon = false;
	unsigned int wordLimit = 8;  // allows for two "words" to be dropped since each date is three words
	unsigned int numWordsRead = 0;
	unsigned int numValidDates = 0;
    unsigned int numPartialDates = 0;
    unsigned int partialDateMonth = 0;
    yyyy = mm = dd = 0;

	beg();		// go to beginning of stream

    while (!EOS && (numValidDates < maxDates) && !overLimit && !abandon)
	{
        cumulativeGet.clear();
        word = getWord(true);		// pull next word from content to check for month
        originalWord = word;
        cumulativeGet = originalWord + space;
        numWordsRead++;

        lastChar = word.right(1);
		if ((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION)
			word.dropRight(1);
        lowerWord = word.lower();
        if (limitWords)
        {
            switch(numValidDates)
            {
            case 0:
                if (numWordsRead > static_cast<unsigned int>(wordLimit/2 - 2))		// Two offset to allow for day and year
                    overLimit = true;
                break;

            case 1:
            default:
                if (numWordsRead >(wordLimit - 2))		// Two offset to allow for day and year
                    overLimit = true;
                break;
            }
        }
        dateStarted = false;
		stillOK = false;

		if (word.isNumeric())
		{
            dd = static_cast<int>(word.asNumber());
            if ((dd >= 1) && (dd <= 31))
			{
				dateStarted = true;
				stillOK = true;
			}
		}
		else
		{
			if (word.isAlphaNumeric())
                dd = static_cast<int>(word.drawOutNumber());
			if ((dd != 0) && (dd <= 31))
			{
				dateStarted = true;
				stillOK = true;
			}

            // Assume any date after "service" or "celebration" is funeral date
            tempWord = lowerWord;
            tempWord.removeEnding("s");
            if (!serviceDate && tempWord.isServiceWord(french))
                abandon = true;
        }

		// Read in next word to try to match to month
		if (stillOK)
		{
			word = getWord(true);		// pull next word from content
            originalWord = word;
            wordLength = word.getLength();
			numWordsRead++;
			lastChar = word.right(1);
			if ((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION)
				word.dropRight(1);
			lowerWord = word.lower();
			matched = false;

            // Loop through all potential alpha dates - stop if found or at end of list
            iter.toFront();
            while (!matched && iter.hasNext())
            {
                mi = iter.next();
                if (lowerWord == mi.monthAlpha)
                {
                    matched = true;
                    mm = static_cast<int>(mi.monthNumeric);
                }
            }

			if (matched)
            {
				stillOK = true;
                cumulativeGet += originalWord + space;
            }
			else
			{
				stillOK = false;
                if (!EOS)
                    backward(wordLength + 1);  // back up position by last word plus space
				numWordsRead--;
			}

			// Attempt to read in year
			if (stillOK)
			{
				word = getWord(true);		// pull next word from content
                originalWord = word;
                wordLength = word.getLength();
				numWordsRead++;
				lastChar = word.right(1);
                if (((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION) || lastChar.isHyphen())
                    word.dropRight(1);
				if (word.isNumeric())
                {
                    yyyy = static_cast<int>(word.asNumber());
                    cumulativeGet += originalWord + space;
                }
				else
				{
					stillOK = false;
                    if (!EOS)
                        backward(wordLength + 1);  // back up position by last word
					numWordsRead--;

                    // Following coding handles reporting of partial date, or creation of complete date, depending on "forceYear"
                    if (forceYear == 0)
                    {
                        if (globals->globalDr->getDeemedYOD() > 0)
                        {
                            stillOK = true;
                            yyyy = globals->globalDr->getDeemedYOD();
                            maxDates++;
                            cumulativeGet += originalWord + space;
                        }
                        else
                        {
                            numPartialDates++;
                            partialDateMonth = mi.monthNumeric;
                        }
                    }
                    else
                    {
                        stillOK = true;
                        yyyy = forceYear;
                    }
				}
			}

			// Validate any potential date
			if (stillOK)
			{
                testDate = QDate(yyyy, mm, dd);
                if (testDate.isValid())
                {
                    dateList.append(testDate);
                    numValidDates++;
                    cumulativeGet.clear();
                }
            }  // end if - date validation
		} // end if - reading in day and year

        cleanString += cumulativeGet;

	} // end while

    cleanString.removeEnding(SPACE);

    if ((numPartialDates == 1) && (numValidDates == 0))
        return 9900 + partialDateMonth;
    else
        return numValidDates;
}

unsigned int unstructuredContent::pullOutSpanishDates(QList<QDate> &dateList, unsigned int maxDates, OQString &cleanString, bool limitWords, bool serviceDate, int forceYear)
{
    Q_UNUSED(maxDates)
    Q_UNUSED(limitWords)
    Q_UNUSED(cleanString)
    Q_UNUSED(serviceDate)
    Q_UNUSED(forceYear)

    unsigned int numDates = 0;
    dateList.clear();

	return numDates;
}

DATES unstructuredContent::contentPullBackToBackDates(LANGUAGE lang)
{
    DATES resultDates;
    unstructuredContent sentence;
    QList<QString> firstNamesList = globals->globalDr->getFirstNameList();
    bool realSentenceEncountered = true;

    beg();

    if (lang == language_unknown)
        lang = contentLanguage;

    while ((!resultDates.potentialDOB.isValid() || !resultDates.potentialDOD.isValid()) && !isEOS())
    {
        sentence = getSentence(firstNamesList, realSentenceEncountered, lang);
        sentence.sentencePullBackToBackDates(lang, resultDates);
    }

    return resultDates;
}

void unstructuredContent::sentencePullBackToBackDates(LANGUAGE lang, DATES &dates)
{
    if (itsString.size() == 0)
        return;

    QList<QDate> dateList;
    bool success = false;


    switch (lang)
    {
    case french:
        success = pullBackToBackFrenchDates(dateList);
        break;

    case spanish:
        success = pullBackToBackSpanishDates(dateList);
        break;

    case english:
    default:
        success = pullBackToBackEnglishDates(dateList);
        break;
    }

    if (success)
    {
        if (dateList[0] <= dateList[1])
        {
            dates.potentialDOB = dateList[0];
            dates.potentialDOD = dateList[1];
            dates.fullyCredible = false;
        }
        else
        {
            dates.potentialDOB = dateList[1];
            dates.potentialDOD = dateList[0];
            dates.fullyCredible = false;
        }
    }
}

bool unstructuredContent::pullBackToBackEnglishDates(QList<QDate> &dateList)
{
    bool success = false;
    dateList.clear();
    beg();		// go to beginning of stream

    QDate testDate;
    OQString list;
    QListIterator<monthInfo> iter(list.monthsEnglish);

    int yyyy, mm, dd;
    OQString word, lowerWord, lastChar;
    monthInfo mi;
    bool dateStarted, stillOK, firstDateFound;

    unsigned int numWordsRead = 0;
    unsigned int numValidDates = 0;
    bool abandon = false;
    firstDateFound = false;
    yyyy = mm = dd = 0;

    while (!EOS && (numValidDates < 2) && !abandon)
    {
        word = getWord(true);		// pull next word from content to check for month
        if (firstDateFound)
            numWordsRead++;
        if (numWordsRead > 2)
            abandon = true;

        if (!abandon)
        {
            lastChar = word.right(1);
            if ((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION)
                word.dropRight(1);
            lowerWord = word.lower();
            dateStarted = false;
            stillOK = false;

            // Loop through all potential alpha dates - stop if found or at end of list
            if (lowerWord.isAlpha())
            {
                iter.toFront();
                while (!dateStarted && iter.hasNext())
                {
                    mi = iter.next();
                    if (lowerWord == mi.monthAlpha)
                    {
                        dateStarted = true;
                        mm = static_cast<int>(mi.monthNumeric);
                        stillOK = true;
                    }
                }
            }

            // Read in day and year and attempt to validate
            if (stillOK)
            {
                dd = 0;
                word = getWord(true);		// pull next word (potential day) from content
                lastChar = word.right(1);
                if ((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION)
                    word.dropRight(1);
                if (word.isNumeric())
                    dd = static_cast<int>(word.asNumber());
                else
                {
                    if (word.isAlphaNumeric())      // e.g., 3rd
                        dd = static_cast<int>(word.drawOutNumber());
                    if (dd == 0)
                    {
                        stillOK = false;
                        backward(word.getLength());  // back up position by last word
                    }
                }
                if (stillOK)
                {
                    word = getWord(true);		// pull next word (potential year) from content

                    // Make adjustments if hyphen was used between dates without spacing
                    if (word.isHyphenated())
                    {
                        QString tempWord = word.getString();
                        int i = tempWord.indexOf(QChar(45));
                        if (i == -1)
                            i = tempWord.indexOf(QChar(8211));
                        unsigned int ui = static_cast<unsigned int>(i);
                        OQString firstPart = word.left(ui);
                        if (firstPart.isNumeric())
                        {
                            // Use first part only and reposition position after hyphen
                            backward(word.getLength() - ui);
                            word = firstPart;
                        }
                    }

                    lastChar = word.right(1);
                    if (((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION) || lastChar.isHyphen())
                        word.dropRight(1);
                    if (word.isNumeric())
                        yyyy = static_cast<int>(word.asNumber());
                    else
                    {
                        // Check if date was entered as Nov 9 th 1966 (i.e., space after number)
                        OQString ending = word.lower();
                        if (ending.isOrdinal(english))
                        {
                            word = getWord(true);		// pull next word (potential year) from content
                            lastChar = word.right(1);
                            if ((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION)
                                word.dropRight(1);
                            if (word.isNumeric())
                                yyyy = static_cast<int>(word.asNumber());
                        }
                        else
                        {
                            stillOK = false;
                            backward(word.getLength());  // back up position by last word
                        }
                    }
                }

                // Adjust for any years entered as two digits
                if (stillOK && (yyyy < 100))
                {
                    int currentYear = globals->today.year() - 2000;
                    if (yyyy > currentYear)
                        yyyy += 1900;
                    else
                        yyyy += 2000;
                }

                // Validate any potential date
                if (stillOK)
                {
                    testDate = QDate(yyyy, mm, dd);
                    if (testDate.isValid())
                    {
                        dateList.append(testDate);
                        numValidDates++;
                        if (firstDateFound)
                            success = true;
                        else
                            firstDateFound = true;
                    }
                }  // end if - date validation
            } // end if - reading in day and year
        } // end !abandon
    } // end while

    return success;
}

bool unstructuredContent::pullBackToBackFrenchDates(QList<QDate> &dateList)
{
    bool success = false;
    dateList.clear();
    beg();		// go to beginning of stream

    QDate testDate;
    OQString list;
    QListIterator<monthInfo> iter(list.monthsFrench);

    int yyyy, mm, dd;
    OQString word, lowerWord, lastChar;
    monthInfo mi;
    bool dateStarted, stillOK, firstDateFound, matched;

    unsigned int numWordsRead = 0;
    unsigned int numValidDates = 0;
    bool abandon = false;
    firstDateFound = false;
    yyyy = mm = dd = 0;

    while (!EOS && (numValidDates < 2) && !abandon)
    {
        word = getWord(true);		// pull next word from content to check for month
        if (firstDateFound)
            numWordsRead++;
        if (numWordsRead > 2)
            abandon = true;

        if (!abandon)
        {
            lastChar = word.right(1);
            if ((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION)
                word.dropRight(1);
            lowerWord = word.lower();
            dateStarted = false;
            stillOK = false;

            if (word.isNumeric())
            {
                dd = static_cast<int>(word.asNumber());
                if (dd <= 31)
                {
                    dateStarted = true;
                    stillOK = true;
                }
            }
            else
            {
                if (word.isAlphaNumeric())
                    dd = static_cast<int>(word.drawOutNumber());
                if ((dd != 0) && (dd <= 31))
                {
                    dateStarted = true;
                    stillOK = true;
                }
            }

            // Read in next word to try to match to month
            if (stillOK)
            {
                word = getWord(true);		// pull next word from content
                lastChar = word.right(1);
                if ((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION)
                    word.dropRight(1);
                lowerWord = word.lower();
                matched = false;

                // Loop through all potential alpha dates - stop if found or at end of list
                iter.toFront();
                while (!matched && iter.hasNext())
                {
                    mi = iter.next();
                    if (lowerWord == mi.monthAlpha)
                    {
                        matched = true;
                        mm = static_cast<int>(mi.monthNumeric);
                    }
                }

                if (matched)
                    stillOK = true;
                else
                {
                    stillOK = false;
                    backward(word.getLength());  // back up position by last word
                }

                // Attempt to read in year
                if (stillOK)
                {
                    word = getWord(true);		// pull next word from content
                    lastChar = word.right(1);
                    if (((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION) || lastChar.isHyphen())
                        word.dropRight(1);
                    if (word.isNumeric())
                        yyyy = static_cast<int>(word.asNumber());
                    else
                    {
                        stillOK = false;
                        backward(word.getLength());  // back up position by last word
                    }
                }

                // Adjust for any years entered as two digits
                if (stillOK && (yyyy < 100))
                {
                    int currentYear = globals->today.year() - 2000;
                    if (yyyy > currentYear)
                        yyyy += 1900;
                    else
                        yyyy += 2000;
                }

                // Validate any potential date
                if (stillOK)
                {
                    testDate = QDate(yyyy, mm, dd);
                    if (testDate.isValid())
                    {
                        dateList.append(testDate);
                        numValidDates++;
                        if (firstDateFound)
                            success = true;
                        else
                            firstDateFound = true;
                    }
                }  // end if - date validation
            } // end if - reading in day and year
        } // end !abandon
    } // end while

    return success;
}

bool unstructuredContent::pullBackToBackSpanishDates(QList<QDate> &dateList)
{
    bool success = false;
    dateList.clear();
    return success;
}

QDate unstructuredContent::fillInDate(const LANGUAGE lang, OQString &cleanString, const int year)
{
    QDate result;
    QList<QDate> dateList;

    switch (lang)
    {
    case french:
        pullOutFrenchDates(dateList, 1, cleanString, false, false, year);
        break;

    case spanish:
        pullOutSpanishDates(dateList, 1, cleanString, false, false, year);
        break;

    case english:
    default:
        pullOutEnglishDates(dateList, 1, cleanString, false, false, year);
        break;
    }

    if (dateList.size() == 1)
        result = dateList[0];

    return result;
}

int unstructuredContent::getNumPages()
{
    OQString numberOfPages;
    int numPages = -1;

    switch (globals->globalDr->getProvider())
    {
    case SIDS:
        if (moveTo("Page 1 of "))
        {
            numberOfPages = getWord();
            numPages = static_cast<int>(numberOfPages.extractFirstNumber());
        }
        else
        {
            beg();
            if (moveTo("No matching records were found"))
                numPages = 0;
        }
        break;

    default:
        // This is an error if it reaches this far
        numPages = -1;
        break;

    }

    return numPages;
}

QList<NAMEINFO> unstructuredContent::readAlternates(unsigned int endPoints, bool applyAdjacentChecks)
{
	// Creates and returns a vector of alternate names
	
    QList<NAMEINFO> nameInfoList;
    QList<QString> otherNameList;
    OQString name, word, tempWord, nextWord, doubleWord, tripleWord, wordBefore, wordAfter1, wordAfter2;
    bool inParentheses, inQuotes, finished, exclude, compoundName, standAloneComma, wordIsSavedName, hasComma;
	NAMEINFO nameInfo;
    unsigned int numWords, numWordsInSentence;
	unsigned int numWordsRead = 0;
    unsigned int sentencesRead = 0;
    bool alternateFound = false;
    bool forceStopSentence = false;
    bool minOneSavedName;     // Will only save an alternate name if another confirmed name is encountered in sentence first
    bool isSurname, isGivenName, isNickName, isPureNickName, isAboriginal;
    bool wasNeeEtAl;

    PQString comma(",");
    PQString space(" ");
    unstructuredContent tempContent, sentence;
    QList<QString> firstNamesList = globals->globalDr->getFirstNameList();
    bool realSentenceEncountered = true;

    LANGUAGE lang = globals->globalDr->getLanguage();
    GENDER gender = globals->globalDr->getGender();
    GENDER workingGender = gender;

    beg();      // go to beginning of content
    while (!EOS && ((numWordsRead < 25) || (sentencesRead < 2)))
	{
        sentence = getSentence(firstNamesList, realSentenceEncountered, lang);
        sentencesRead++;
        numWordsInSentence = 0;
        minOneSavedName = false;
        wordIsSavedName = false;
        word.clear();

        // For purposes of cutoffs, ignore a sentence if it is (mmmm dd, yyyy - mmmm dd, yyyy)
        if (sentence.isJustDates())
            sentence = getSentence(firstNamesList, realSentenceEncountered, lang);

        sentence.cleanUpEnds();
        while (!sentence.isEOS() && (numWordsRead < 25) && !forceStopSentence)
        {
            wordBefore = word;
            word = sentence.getWord(true);		// pull next word from content - don't split words in parentheses or quotes
            numWordsRead++;
            numWordsInSentence++;
            wordAfter1 = sentence.peekAtWord(true, 1);
            wordAfter2 = sentence.peekAtWord(true, 2);
            doubleWord = word + space + wordAfter1;
            tripleWord = doubleWord + space + wordAfter2;

            // Assume rest of sentence is unreliable if it is likely going to refer to someone else
            if (word.isOtherPersonReference(lang) || doubleWord.areRelationshipWords(gender, lang)
                                                  || tripleWord.areRelationshipWords(gender, lang))
                forceStopSentence = true;

            // If information will still be useful, keep going
            inParentheses = word.hasBookEnds(endPoints & PARENTHESES);
            inQuotes = word.hasBookEnds(endPoints & QUOTES);
            if ((word.getLength() == 1) && (word == comma))
                standAloneComma = true;
            else
                standAloneComma = false;

            if ((inParentheses || inQuotes) && !forceStopSentence)
            {
                finished = false;
                exclude = false;
                compoundName = false;
                nameInfo.type = ntUnknown;
                word.removeBookEnds(PARENTHESES);
                word.removeBookEnds(QUOTES);
                word.cleanUpEnds();
                word.removeEnding(QString(","));
                word.removeEnding(QString("."));
                word.compressCompoundNames();
                name = word;
                numWords = name.countWords();

                // Check to see if it might be (DOB - DOD)
                // Returns true if dates found and set
                tempContent = name;
                tempContent.copyKeyVariablesFrom(*this);
                exclude = globals->globalObit->fillInDatesStructured(tempContent) || (name.getLength() < 2);

                // Start with easy case where we know it is last name at birth due to the "nee" prefix
                if (exclude)
                    wasNeeEtAl = false;
                else
                    wasNeeEtAl = name.removeLeadingNeeEtAl();

                if (wasNeeEtAl)
                {
                    if (name.left(3).lower() == OQString("of "))
                    {
                        wasNeeEtAl = false;
                        exclude = true;
                    }
                }

                if (wasNeeEtAl)
                {
                    numWords--;
                    if (gender == Male)
                    {
                        // Assumed to be picking up maiden name of mother
                        wasNeeEtAl = false;
                        exclude = true;
                    }
                    else
                    {
                        globals->globalDr->setGender(Female);
                        workingGender = Female;
                    }

                    // Adjust for rare "Nee - Smith)
                    name.removeLeading(HYPHENATED);
                    name.cleanUpEnds();
                }

                // Identify compound names - moved to above
                /*if (!exclude && (numWords >= 2) && (numWords <= 3))
                {
                    OQStream temp(name);
                    tempWord = temp.getWord();
                    bool matched = true;
                    isAboriginal = tempWord.isAboriginalName(temp.peekAtWord());
                    for (unsigned int i = 0; i < (numWords - 1); i++)
                    {
                        matched = matched && (tempWord.isCompoundName() || isAboriginal) && (tempWord != OQString("E."));
                        tempWord = temp.getWord();
                        hasComma = (tempWord.right(1) == OQString(","));
                        isAboriginal = !hasComma && isAboriginal && tempWord.isAboriginalName(temp.peekAtWord());
                    }
                    if (matched)
                    {
                        compoundName = true;

                        // Format based on last of the compound names
                        if (tempWord.isAllCaps())
                            name = name.proper();
                    }
                }*/

                if (!exclude && wasNeeEtAl)
                {
                    // Fix hyphenated spacing if possible (eg. "Smith - Rogers"), which counts as two words
                    if ((numWords == 2) && name.isHyphenated())
                    {
                        QString extract = name.getString();
                        bool fixable = extract.contains(" - ", Qt::CaseInsensitive);
                        if (fixable)
                        {
                            extract.replace(QString(" - "), QString("-"), Qt::CaseInsensitive);
                            name = extract;
                            numWords = name.countWords();
                        }
                    }

                    // Save alternate last name where appropriate
                    if ((name.getLength() > 0) && ((numWords == 1) || compoundName))
                    {
                        nameInfo.type = ntLast;
                        nameInfo.name = name.proper();
                        nameInfo.numWords = numWords;
                        alternateFound = true;
                        globals->globalDr->setNeeEtAlEncountered(true);
                        PQString pln = globals->globalDr->getParentsLastName();
                        if ((pln.getLength() > 0) && (pln != nameInfo.name))
                        {
                            globals->globalDr->removeFromLastNames(pln);
                            globals->globalDr->setParentsLastName("");
                        }

                        if (name.isFoundIn(problematicFemaleMiddleNames, 1))
                            globals->globalDr->wi.nameWarningException = true;
                    }
                    else
                    {
                        // Try to pick out surnames from a list
                        if (numWords <= 5)
                        {
                            OQStream temp(name);
                            NAMESTATS nameStats;
                            tempWord = temp.getWord(false, SLASH);
                            tempWord.removeEnding(PUNCTUATION);

                            while(tempWord.getLength() > 0)
                            {
                                exclude = false;

                                // Check for embedded neeEtal like (formerly Smith, nee Jacobson)
                                if (tempWord.isNeeEtAl())
                                {
                                    tempWord = temp.getWord(false, SLASH);
                                    tempWord.removeEnding(PUNCTUATION);
                                    workingGender = Female;
                                }

                                nameStats.clear();
                                if (tempWord.isCapitalized())
                                {
                                    if (tempWord.isWrittenMonth(globals->globalDr->getLanguage()))
                                    {
                                        nextWord = temp.peekAtWord();
                                        nextWord.removeEnding(PUNCTUATION);
                                        if (nextWord.isNumeric() || nextWord.removeOrdinal(globals->globalDr->getLanguage()))
                                            exclude = true;
                                    }

                                    if (!exclude)
                                    {
                                        nameStatLookup(tempWord.getString(), globals, nameStats, workingGender);
                                        if (nameStats.isLikelySurname)
                                        {
                                            alternateFound = true;
                                            globals->globalDr->setNeeEtAlEncountered(true);
                                            globals->globalDr->setFamilyName(tempWord);
                                            globals->globalDr->setGender(Female);
                                            workingGender = Female;

                                            if (tempWord.isFoundIn(problematicFemaleMiddleNames, 1))
                                                globals->globalDr->wi.nameWarningException = true;
                                        }
                                    }
                                }
                                tempWord = temp.getWord(false, SLASH);
                                tempWord.removeEnding(PUNCTUATION);
                            }
                        }
                        else
                            exclude = true;
                    }

                    finished = true;
                }

                // Next easy case is where the "aka" prefix is used
                if (!exclude && name.removeLeadingAKA())
                {
                    numWords--;
                    if (name.getLength() > 0)
                    {
                        nameInfo.type = ntFirst;
                        nameInfo.name = name.proper();
                        nameInfo.numWords = numWords;
                    }
                    finished = true;
                }

                // Eliminate problem exceptions
                if ((name.lower() == PQString("in memoriam")) || !name.isAlpha())
                    exclude = true;

                // Save compound names
                if (!exclude && compoundName)
                {
                    nameInfo.type = ntLast;
                    nameInfo.name = name;
                    nameInfo.numWords = numWords;

                    alternateFound = true;
                    finished = true;
                }
                else
                {
                    // Keep going as it could be a nickname (e.g. "uncle vinny")
                    //exclude = true;
                }

                // Eliminate anything remaining with two or more words
                if ((nameInfo.type == ntUnknown) && (numWords >= 2))
                {
                    exclude = true;

                    // Issue warning if they could potentially be multiple former last names
                    bool allMatched = true;
                    QStringList list = name.getString().split(" ", QString::SkipEmptyParts);
                    int i = 0;;

                    while (allMatched && (i < list.size()))
                    {
                           allMatched = allMatched && (surnameLookup(list.at(i), globals) > 0);
                           i++;
                    }

                    if (allMatched)
                    {
                        i = 0;
                        PQString warningMessage;
                        OQString tempName;
                        OQString potentialFormalName = list.at(i);
                        NAMEINFO newName;
                        NAMESTATS nameStats;

                        if (globals->globalDr->isAFirstName(potentialFormalName) || potentialFormalName.isFormalVersionOf(globals->globalDr->getFirstName().lower().getString(), warningMessage))
                        {
                            if (!globals->globalDr->isASavedName(potentialFormalName))
                            {
                                tempName = OQString(list.at(i)).proper();
                                newName.name = tempName;
                                newName.type = ntFirst;
                                newName.numWords = tempName.countWords();
                                globals->globalDr->setAlternates(newName);
                            }
                            i++;

                            while (i < list.size())
                            {
                                newName.clear();
                                tempName = OQString(list.at(i)).proper();
                                nameStatLookup(tempName.getString(), globals, nameStats, workingGender);
                                if (!nameStats.isLikelySurname)
                                {
                                    newName.name = tempName;
                                    newName.type = ntMiddle;
                                    newName.numWords = tempName.countWords();
                                    globals->globalDr->setAlternates(newName);
                                }
                                i++;
                            }
                        }
                        else
                        {
                            warningMessage << "Verify if: (" << name << ") should be included as last names for: " << globals->globalDr->getURL();
                            //globals->logMsg(ErrorRecord, warningMessage);
                            globals->globalDr->wi.confirmTreatmentName = name.getString();
                        }
                    }
                }

                // Check if name is already saved, with potential correction in some cases
                if (!exclude)
                {
                    wordIsSavedName = static_cast<bool>(globals->globalDr->isASavedName(name));
                    if (wordIsSavedName)
                    {
                        if (wasNeeEtAl && globals->globalDr->isAFirstName(name))
                            globals->globalDr->removeFromFirstNames(name);
                        else
                        {
                            if (wasNeeEtAl && globals->globalDr->isAMiddleName(name))
                                globals->globalDr->removeFromMiddleNames(name);
                            else
                            {
                                minOneSavedName = true;
                                exclude = true;
                            }
                        }
                    }
                }

                // Check for suffix
                if (!finished && !exclude && name.isSuffix())
                {
                    if (name.isSuffixKeep())
                    {
                        nameInfo.type = ntSuffix;
                        nameInfo.numWords = numWords;
                        if (name.isSuffixAllCaps())
                            nameInfo.name = name.upper();
                        else
                            nameInfo.name = name.proper();
                        finished = true;
                    }
                    else
                    {
                        exclude = true;
                    }
                }

                // Check for prefix
                if (!finished && !exclude && name.isPrefix())
                {
                    if ((name.lower() == OQString("gen")) && globals->globalDr->isANickName(name))
                        nameInfo.type = ntFirst;
                    else
                        nameInfo.type = ntPrefix;

                    nameInfo.numWords = numWords;
                    nameInfo.name = name.proper();
                    finished = true;
                }

                // Check for nicknames of known first and middle names
                if (!finished && !exclude &&  globals->globalDr->isANickName(name))
                {
                    nameInfo.type = ntFirst;
                    nameInfo.numWords = numWords;
                    nameInfo.name = name.proper();
                    finished = true;
                }

                // Down to single word names
                // Exclude name if word on either side is not a saved name
                if (applyAdjacentChecks && !finished && !exclude)
                {
                    if (!(static_cast<bool>(globals->globalDr->isAName(wordBefore)) || static_cast<bool>(globals->globalDr->isAName(wordAfter1))))
                        exclude = true;
                }

                // Database searches and usage analysis on the name
                if (!finished && !exclude)
                {
                    // Lower conviction decisions include a warning message

                    bool consistentNameAndGender = false;
                    int frequencyFirst;
                    PQString warningMessage;

                    workingGender = gender;

                    frequencyFirst = countFrequencyFirst(globals->cleanedUpUC, name.getString(), Qt::CaseInsensitive);
                    isSurname = (surnameLookup(name.getString(), globals) > 0);
                    isGivenName = givenNameLookup(name.getString(), globals, workingGender);

                    if (isGivenName)
                    {
                        consistentNameAndGender = nameAndGenderConsistent(name.getString(), workingGender);
                        isPureNickName = pureNickNameLookup(name.getString(), globals);
                        if (isPureNickName)
                        {
                            isGivenName = false;
                            exclude = true;
                        }
                    }

                    if (isGivenName && !isSurname)
                    {
                        isNickName = globals->globalDr->isANickName(name) || nickNameInList(name.lower(), otherNameList, globals);

                        if (inParentheses && !isNickName)
                        {
                            // Second chance check - Assume it is a unrecorded nickname if member already has a middlename recorded
                            if ((globals->globalDr->getMiddleNames().getLength() == 0) && (otherNameList.size() == 0))
                            {
                                // Most likely the spouse of a relative
                                //warningMessage << "Confirm treatment of (" << name.proper() << ") for: " << globals->globalDr->getURL();
                                globals->globalDr->wi.confirmTreatmentName = name.getString();
                            }
                            else
                            {
                                nameInfo.type = ntFirst;
                                nameInfo.numWords = numWords;
                                nameInfo.name = name.proper();

                                if ((minOneSavedName) && !(globals->globalDr->isASavedName(wordBefore) && globals->globalDr->isASavedName(wordAfter1)))
                                {
                                    //warningMessage << "Confirm inclusion of (" << nameInfo.name << ") as nickname for: \"" << globals->globalDr->getFullName() << "\"  source: "<< globals->globalDr->getURL();
                                    globals->globalDr->wi.checkInclName = name.getString();
                                }
                            }
                        }
                        else
                        {
                            // Low risk combination
                            nameInfo.type = ntFirst;
                            nameInfo.numWords = numWords;
                            nameInfo.name = name.proper();

                            if (((gender == Female) && !isNickName) || !consistentNameAndGender)
                            {
                                //warningMessage << "Confirm inclusion of (" << nameInfo.name << ") as first name for: \"" << globals->globalDr->getFullName() << "\"  source: "<< globals->globalDr->getURL();
                                globals->globalDr->wi.checkInclName = name.getString();
                            }
                        }
                    }

                    if (!isGivenName && isSurname)
                    {
                        if ((workingGender == Male) && inParentheses && (numWordsRead > 7))
                        {
                            // Much more likely to be maiden name of mother
                            exclude = true;
                            //warningMessage << "Confirm exclusion of (" << name.proper() << ") as assumed mother's maiden name for: \"" << globals->globalDr->getFullName() << "\"  source: "<< globals->globalDr->getURL();
                            globals->globalDr->wi.checkExclName = name.getString();
                        }
                        else
                        {
                            if (workingGender == genderUnknown)
                            {
                                double unisex = genderLookup(globals->globalDr, globals);
                                if (unisex >= 0.9)
                                    workingGender = Male;
                                else
                                {
                                    if (unisex <= 0.1)
                                        workingGender = Female;
                                }
                            }

                            if ((workingGender == Male) && inQuotes)
                                nameInfo.type = ntFirst;
                            else
                            {
                                nameInfo.type = ntLast;
                                alternateFound = true;
                            }

                            nameInfo.numWords = numWords;
                            nameInfo.name = name.proper();
                        }
                    }

                    if (isGivenName && isSurname)
                    {
                        bool first = false;

                        if (frequencyFirst >= 2)
                            first = true;
                        else
                        {
                            if (inQuotes)
                            {
                                first = true;
                                if ((gender == Female) && !consistentNameAndGender)
                                {
                                    //warningMessage << "Confirm inclusion of (" << name << ") as first name for: " << globals->globalDr->getURL();
                                    globals->globalDr->wi.checkInclName = name.getString();
                                }
                            }
                            else // Parentheses
                            {
                                if ((frequencyFirst >= 1)  || (gender == Male))
                                    first = true;
                            }
                        }

                        if (first)
                        {
                            nameInfo.type = ntFirst;
                            nameInfo.numWords = numWords;
                            nameInfo.name = name.proper();
                        }
                        else
                        {
                            nameInfo.type = ntLast;
                            nameInfo.numWords = numWords;
                            nameInfo.name = name.proper();
                            alternateFound = true;
                        }
                    }

                    if (!isGivenName && !isSurname)
                    {
                        // Initially set up same as (isGivenName && isSurname)
                        bool first = false;
                        bool skip = false;

                        if (frequencyFirst >= 2)
                            first = true;
                        else
                        {
                            isNickName = globals->globalDr->isANickName(name);
                            if (inQuotes || isNickName)
                            {
                                first = true;
                                consistentNameAndGender = nameAndGenderConsistent(name.getString(), gender);
                                if (((gender == Female) && !isNickName) || !consistentNameAndGender)
                                {
                                    //warningMessage << "Confirm inclusion of (" << name.proper() << ") as first name for: \"" << globals->globalDr->getFullName() << "\"  source: "<< globals->globalDr->getURL();
                                    globals->globalDr->wi.checkInclName = name.getString();
                                }
                            }
                            else // Parentheses
                            {
                                if (frequencyFirst >= 1)
                                    first = true;
                                else
                                {
                                    if (gender == Male)
                                    {
                                        skip = true;
                                        // TODO:  Remove this warning as it does not appear to add value
                                        //warningMessage << "Confirm exclusion of (" << name.proper() << ") as first/last name for: \"" << globals->globalDr->getFullName() << "\"  source: "<< globals->globalDr->getURL();
                                        globals->globalDr->wi.checkExclName = name.getString();
                                    }
                                }

                            }
                        }

                        if (!skip)
                        {
                            if (first)
                            {
                                nameInfo.type = ntFirst;
                                nameInfo.numWords = numWords;
                                nameInfo.name = name.proper();
                            }
                            else
                            {
                                nameInfo.type = ntLast;
                                nameInfo.numWords = numWords;
                                nameInfo.name = name.proper();
                                alternateFound = true;
                            }
                        }
                    }

                    if (warningMessage.getLength() > 0)
                        globals->logMsg(ErrorRecord, warningMessage);

                }

                if (!exclude && (minOneSavedName || (wasNeeEtAl && (numWordsInSentence == 1))))
                    nameInfoList.append(nameInfo);

                if (!exclude && (nameInfo.type == ntFirst || nameInfo.type == ntMiddle))
                    otherNameList.append(nameInfo.name.lower().getString());

            }  // end if parentheses or quotes
            else
            {
                word.removeEnding(QString(","));
                word.removeEnding(QString("."));

                // Track if the word is a saved name
                if (word.isSaint())
                {
                    nextWord = sentence.peekAtWord();
                    inParentheses = nextWord.hasBookEnds(endPoints & PARENTHESES);
                    inQuotes = nextWord.hasBookEnds(endPoints & QUOTES);
                    if (!inParentheses && !inQuotes)
                    {
                        word += space;
                        word += sentence.getWord();
                        // Don't count as an extra word
                    }
                }

                isAboriginal = word.isAboriginalName(sentence.peekAtWord());
                if ((word.isCompoundName() || isAboriginal) && (word.lower() != OQString("e")))
                {
                    nextWord = sentence.peekAtWord();
                    inParentheses = nextWord.hasBookEnds(endPoints & PARENTHESES);
                    inQuotes = nextWord.hasBookEnds(endPoints & QUOTES);
                    if (!inParentheses && !inQuotes)
                    {
                        nextWord = sentence.getWord();
                        word += space;
                        word += nextWord;
                        // Don't count as an extra word
                        if (nextWord.isCompoundName() || nextWord.isAboriginalName(sentence.peekAtWord()))
                        {
                            nextWord = sentence.peekAtWord();
                            inParentheses = nextWord.hasBookEnds(endPoints & PARENTHESES);
                            inQuotes = nextWord.hasBookEnds(endPoints & QUOTES);
                            if (!inParentheses && !inQuotes)
                            {
                                nextWord = sentence.getWord();
                                word += space;
                                word += nextWord;
                                // Don't count as an extra word
                            }
                        }
                    }
                }

                wordIsSavedName = static_cast<bool>(globals->globalDr->isASavedName(word));
                if(wordIsSavedName)
                    minOneSavedName = true;
                else
                {
                    isGivenName = givenNameLookup(word.getString(), globals, gender);
                    if(isGivenName)
                        otherNameList.append(word.lower().getString());
                }

                // If one alternate was previously found and the current word isn't an alternate or a saved name,
                // then force stop to avoid reading in what is likely to be garbage
                if (alternateFound && !standAloneComma && !wordIsSavedName && !word.isMiscKeeperWord(globals->globalDr->getLanguage()))
                    forceStopSentence = true;
            }

        } // end while sentence
    }	// end while content

    return nameInfoList;
}

DATES unstructuredContent::readDOBandDOD(bool reliable)
{
    // This function is only designed to be called with a clean short stream
    DATES resultDates;

    unsigned int maxDates = 3;           // This will be an error condition
    bool limitWords = true;
    bool serviceDate = false;
    QList<QDate> dateList;
    OQString cleanString;
    PQString errMsg;

    LANGUAGE lang = this->getLanguage();
    if (lang == language_unknown)
        lang = globals->globalDr->getLanguage();

    switch (lang)
    {
    case french:
        pullOutFrenchDates(dateList, maxDates, cleanString, limitWords, serviceDate);
        break;

    case spanish:
        pullOutSpanishDates(dateList, maxDates, cleanString, limitWords, serviceDate);
        break;

    case english:
    default:
        pullOutEnglishDates(dateList, maxDates, cleanString, limitWords, serviceDate);
        break;
    }

    if (!reliable)
    {
        if ((dateList.size() == 2) && (dateList[0] == dateList[1]))
            dateList.removeAt(0);
    }

    if (dateList.size() == 2)
    {
        if (dateList[0] <= dateList[1])
        {
            resultDates.potentialDOB = dateList[0];
            resultDates.potentialDOD = dateList[1];
            resultDates.fullyCredible = true;
        }
        else
        {
            resultDates.potentialDOB = dateList[1];
            resultDates.potentialDOD = dateList[0];
            resultDates.fullyCredible = false;
            errMsg << "Investigate reversed dates for: " << globals->globalDr->getURL();
            globals->globalDr->wi.dateFlag = 1;
        }
    }
    else
    {
        if (dateList.size() == 3)
        {
            errMsg << "Investigate three date grouping for: " << globals->globalDr->getURL();
            globals->globalDr->wi.dateFlag = 1;
        }
    }

    if (dateList.size() == 1)
    {
        // Process as DOB if the date is within three weeks of current date
        int spread = globals->today.toJulianDay() - dateList[0].toJulianDay();
        if ((spread > 0) && (spread < 22))
        {
            resultDates.potentialDOB = dateList[0];
            resultDates.fullyCredible = false;
        }
    }

    //if (errMsg.getLength() > 0)
    //    globals->logMsg(ErrorRecord, errMsg);

    return resultDates;
}

bool unstructuredContent::isJustDates()
{
    unsigned int numWords = countWords();

    if ((numWords > 8) || (itsString.length() == 0))
        return false;

    // Special case of yyyy-yyyy
    if ((numWords == 0) && (itsString.length() == 9))
    {
        if (itsString.at(4) == QString("-"))
            return true;
    }

    LANGUAGE language = globals->globalDr->getLanguage();
    int todayYear = globals->today.year();
    bool abandon = false;
    bool wordHandled;
    int number;
    unsigned int length;

    OQString word;
    unstructuredContent tempUC = *this;

    tempUC.removeBookEnds();

    while (!tempUC.EOS && !abandon)
    {
        word = tempUC.getWord().lower();
        word.removeEnding(" ");
        word.removeEnding(",");
        wordHandled = false;
        length = word.getLength();

        if (word.isHyphen() || (length == 0))
            wordHandled = true;

        if (!wordHandled && word.isAlpha())
        {
            if (word.isWrittenMonth(language))
                wordHandled = true;
            else
            {
                abandon = true;
                wordHandled = true;
            }
        }

        if (!wordHandled && word.isNumeric())
        {
            number = static_cast<int>(word.asNumber());
            if (((number >= 1) && (number <= 31)) || ((number > 1875) && (number <= todayYear)))
                wordHandled = true;
            else
            {
                abandon = true;
                wordHandled = true;
            }
        }

        if (!wordHandled && word.isAlphaNumeric())
        {
            number = static_cast<int>(word.drawOutNumber());
            if (((number >= 1) && (number <= 31)) || ((number > 1875) && (number <= todayYear)))
                wordHandled = true;
            else
            {
                abandon = true;
                wordHandled = true;
            }
        }

        // If we still having something to deal with at this point, it's not pure dates
        if (!wordHandled)
            abandon = true;
    }

    // Reset position as it would be assumed this function does leave stream mid-way
    position = 0;

    return !abandon;
}

bool unstructuredContent::isJustNames()
{
    bool stillOK = true;
    OQString word;
    beg();

    if (isEOS())
        return false;

    while (!isEOS() && stillOK)
    {
        word = getWord(true).lower();
        word.removeEnding(PUNCTUATION);
        word.removeBookEnds();
        word.removeLeadingNeeEtAl();

        stillOK = (word.getLength() == 0) || globals->globalDr->isASavedName(word) || globals->globalDr->isAnInitial(word);
        if (!stillOK)
            stillOK = givenNameLookup(word.getString(), globals, globals->globalDr->getGender());
        if (!stillOK)
            stillOK = (surnameLookup(word.getString(), globals) > 0);
    }

    return stillOK;
}

void unstructuredContent::processStructuredNames(QList<NAMESTATS> &nameStatsList, bool fixHyphens)
{
    removeHTMLtags(false);
    unQuoteHTML();
    fixQuotes();
    fixBasicErrors(true);
    if (fixHyphens)
        fixHyphenatedNames();       // Must be confident content only contains names
    bool removeRecognized = false;  // Must be confident content only contains names
    prepareNamesToBeRead(removeRecognized);
    if (contains(","))
        readLastNameFirst(nameStatsList);
    else
        readFirstNameFirst(nameStatsList);
}


PQString unstructuredContent::processAllNames()
{
    // This function is called solely when OQStream only holds names
	// References the datarecord to match against names that have already been saved
	// Returns middle name(s), if any, and sets other names along the way if they are encountered

    QList<NAMEINFO> nameInfoList;
	NAMEINFO nameInfo, priorName;
    PQString middleName;

	if (getLength() == 0)
		return middleName;

	bool nameStarted = false;
	bool nameEnded = false;
    bool nameEndedException = false;
	bool lastNameAttained = false;
	bool lastNameFirst = false;  // set if comma appears before a first name
	bool firstNameEncountered = false;
    bool compoundName, compoundNameFound, isFormerLastName;
    bool isSurname = false;
    bool isGivenName = false;
    bool isLikelySurname = false;
    bool isLikelyGivenName = false;
    bool handled, isAboriginal;
    OQString temp, word, originalWord, nextWord;
    OQString space(" ");
    OQStream existingMiddleNames;
    bool wordMatched, hasBookEnds, possibleName, hasComma, standAloneComma, hasPeriod, skipWordLookup;
    bool compoundException, deException;
	unsigned int position = 1;
	unsigned int commaPositionAfter = 0;
	unsigned int numRotate = 0;
	unsigned int i, j;
    int commaIndex;

    LANGUAGE lang = globals->globalDr->getLanguage();

    existingMiddleNames = OQStream(globals->globalDr->getMiddleNames());
	j = existingMiddleNames.countWords();
    commaIndex = itsString.indexOf(QString(","),0);

	beg();      // go to beginning of content

	while (!EOS && !nameEnded)
	{
		nameInfo.clear();
        nameInfo.nameStatsRun = false;
		wordMatched = false;
		compoundName = false;
		compoundNameFound = false;
        standAloneComma = false;
        handled = false;
        originalWord = getWord(true);		// Pull next word from content - don't split words in parentheses or quotes
        if (originalWord.isAllCaps(false))
            word = originalWord.proper();
        else
            word = originalWord;

        // Deal with prefixes immediately and remove from list of names
        while (word.isPrefix() && !globals->globalDr->isALastName(word))
		{
            word.removeEnding(".");
            nameInfo.name = word.proper();
            nameInfo.type = ntPrefix;
            if (!word.isTitle(lang, globals->globalDr->getGender()))
                globals->globalDr->setAlternates(nameInfo);
            else
                if (globals->globalDr->getGender() == genderUnknown)
                {
                    if (word.isMaleTitle(lang))
                        globals->globalDr->setGender(Male);
                    else
                        if (word.isFemaleTitle(lang))
                            globals->globalDr->setGender(Female);
                }
            nameInfo.clear();
            word = getWord(true);
		}

        // Clean up word
        hasComma = word.removeEnding(",");
        if (hasComma && (word.getLength() == 0))
            standAloneComma = true;
        hasPeriod = word.removeEnding(".");
        if (!hasComma && !hasPeriod)
            hasComma = word.removeEnding(":") || word.removeEnding("-");
        hasBookEnds = word.removeBookEnds(PARENTHESES | QUOTES);
        word.cleanUpEnds();
        if (word.removeLeadingNeeEtAl())
        {
            // Fix rare case of "Nee - Smith)
            word.removeLeading(HYPHENATED);
            word.cleanUpEnds();

            if (word.getLength() > 0)
                isFormerLastName = true;
            else
            {
                nameEndedException = true;
                wordMatched = true;
            }
            compoundNameFound = (word.countWords() > 1);
        }        

        // check for compound name that wasn't led by "nee "
		temp = word;
        deException = false;
        if ((originalWord == OQString("de")) && (globals->globalDr->getLanguage() == french))
            deException = true;
        compoundException = (originalWord == OQString("E.")) ||
                           ((originalWord == OQString("E")) && globals->globalDr->isAnInitial(originalWord)) ||
                             deException;		// Includes "St.", but exclude "E."

        isAboriginal = !hasComma && word.isAboriginalName(peekAtWord());
        compoundName = (word.isCompoundName() && !compoundException && !isEOS()) || (isAboriginal && !hasBookEnds);
		while (compoundName)
		{
			compoundNameFound = true;
            if (hasPeriod)
                temp += OQString(".");
            originalWord = getWord(true);		// Pull next word from content - don't split words in parentheses or quotes
            if (originalWord.isAllCaps(false))
                word = originalWord.proper();
            else
                word = originalWord;
            hasComma = word.removeEnding(",");
            hasPeriod = word.removeEnding(".");
            //temp += space;
			temp += word;
            isAboriginal = !hasComma && isAboriginal && word.isAboriginalName(peekAtWord());
            compoundName = word.isCompoundName() || isAboriginal;
		}
        word = temp.proper();
        word.removeEnding(SPACE);

        // Exclude anything in bookends with 2+ words that isn't a compound name
        if (hasBookEnds && (word.countWords() >= 2) && !compoundName)
        {
            wordMatched = true;

            // Issue warning if they could potentially be multiple former last names
            bool allMatched = true;
            QStringList list = word.getString().split(" ", QString::SkipEmptyParts);
            int i = 0;

            while (allMatched && (i < list.size()))
            {
                   allMatched = allMatched && (surnameLookup(list.at(i), globals) > 0);
                   i++;
            }

            if (allMatched)
            {
                PQString warningMessage;
                warningMessage << "Verify if: (" << word << ") should be included as last names for: " << globals->globalDr->getURL();
                //globals->logMsg(ErrorRecord, warningMessage);
                globals->globalDr->wi.confirmTreatmentName = word.getString();
            }
        }

		possibleName = word.isAlpha();

        if (nameStarted && (deException || !(possibleName || nameEndedException || standAloneComma)))
			nameEnded = true;

		// Attempt to match word against all known names already saved
        if (!nameEnded && !wordMatched && globals->globalDr->isAFirstName(word))
        {
            wordMatched = true;
            nameStarted = true;

            if (word.lower() == globals->globalDr->getFirstName().lower())
                nameInfo.name = globals->globalDr->getFirstName();
            else
            {
                if (word.lower() == globals->globalDr->getFirstNameAKA1().lower())
                    nameInfo.name = globals->globalDr->getFirstNameAKA1();
                else
                    nameInfo.name = globals->globalDr->getFirstNameAKA2();
            }

            nameInfo.type = ntFirst;
            nameInfo.position = position;
            nameInfo.hadBookEnds = hasBookEnds;
            nameInfo.hadComma = hasComma;
            nameInfo.matched = wordMatched;
            firstNameEncountered = true;
            position++;
        }

        if (!nameEnded && !wordMatched && globals->globalDr->isALastName(word))
        {
            wordMatched = true;
            nameStarted = true;
            lastNameAttained = true;

            if (word.lower() == globals->globalDr->getLastName().lower())
                nameInfo.name = globals->globalDr->getLastName();
            else
            {
                if (word.lower() == globals->globalDr->getLastNameAlt1().lower())
                    nameInfo.name = globals->globalDr->getLastNameAlt1();
                else
                {
                    if (word.lower() == globals->globalDr->getLastNameAlt2().lower())
                        nameInfo.name = globals->globalDr->getLastNameAlt2();
                    else
                        nameInfo.name = globals->globalDr->getLastNameAlt3();
                }
            }

            if (!firstNameEncountered)
            {
                lastNameFirst = true;
                numRotate++;
                if ((hasComma) || (commaIndex == -1))
                    commaPositionAfter = position;
            }

            nameInfo.type = ntLast;
            nameInfo.position = position;
            nameInfo.hadBookEnds = hasBookEnds;
            nameInfo.hadComma = hasComma;
            nameInfo.matched = wordMatched;
            position++;
        }

        if (!nameEnded && !wordMatched && globals->globalDr->isAMiddleName(word))
        {
            wordMatched = true;
            nameStarted = true;

            nameInfo.name = word.proper();
            nameInfo.type = ntMiddle;
            nameInfo.position = position;
            nameInfo.hadBookEnds = hasBookEnds;
            nameInfo.hadComma = hasComma;
            nameInfo.matched = wordMatched;
            position++;
        }

		if (!nameEnded && !wordMatched && compoundNameFound)
		{
			wordMatched = true;
            nameInfo.name = word;
			nameInfo.type = ntLast;
			nameInfo.position = position;
			nameInfo.hadBookEnds = hasBookEnds;
            nameInfo.hadComma = hasComma;
            nameInfo.matched = wordMatched;
			position++;
		}

        if (!nameEnded && !wordMatched && !standAloneComma && (word.lower() == globals->globalDr->getSuffix().lower()))
		{
			wordMatched = true;
			if (nameStarted)
            {
                temp = peekAtWord();
                if (!temp.isSuffix())
                    nameEnded = true;
            }
            nameInfo.name = globals->globalDr->getSuffix();
			nameInfo.type = ntSuffix;
			nameInfo.position = position;
			nameInfo.hadBookEnds = hasBookEnds;
            nameInfo.hadComma = hasComma;
            nameInfo.matched = wordMatched;
			position++;
		}

		i = 0;
        existingMiddleNames.beg();
        while (!nameEnded && !wordMatched && (i < j) && !standAloneComma)
		{
            temp = existingMiddleNames.getWord();
			if (word.lower() == temp.lower())
			{
				wordMatched = true;
                nameInfo.name = word;
				nameInfo.type = ntMiddle;
				nameInfo.position = position;
				nameInfo.hadBookEnds = hasBookEnds;
                nameInfo.hadComma = hasComma;
                nameInfo.matched = wordMatched;
				position++;
			}
			i++;
		}

        if (standAloneComma)
        {
            wordMatched = true;
            commaPositionAfter = position - 1;
        }

        // For any unmatched words, attempt to figure out if it is an additional name
        if (wordMatched)
            handled = true;
        else
        {            
            if ((word.isSuffix() || word.isPrefix()) && hasPeriod)
            {
                skipWordLookup = true;
                handled = false;
            }
            else
                skipWordLookup = false;

            if (possibleName && !skipWordLookup)
            {
                nameStatLookup(word.getString(), globals, nameInfo.nameStats, globals->globalDr->getGender());
                isSurname = nameInfo.nameStats.isSurname;
                isGivenName = nameInfo.nameStats.isGivenName;
                isLikelySurname = nameInfo.nameStats.isLikelySurname;
                isLikelyGivenName = nameInfo.nameStats.isLikelyGivenName;
                nameInfo.nameStatsRun = true;
                handled = false;
            }
            else
            {
                isGivenName = false;
                isSurname = false;
            }
        }

        // Check for more formal, or less formal, versions of the first names included in the structured read
        if (!nameEnded && !handled && word.isAlpha())
        {
            if (isGivenName || isSurname)
            {
                nameInfo.name = word;
                nameInfo.position = position;
                nameInfo.hadBookEnds = hasBookEnds;
                nameInfo.hadComma = hasComma;
                nameInfo.matched = wordMatched;
                position++;

                if (isGivenName && !isSurname)
                {
                    if (nameStarted && firstNameEncountered && !globals->globalDr->isANickName(word))
                    {
                        nameInfo.type = ntMiddle;
                        middleName += space;
                        middleName += nameInfo.name;
                    }
                    else
                    {
                        nameInfo.type = ntFirst;
                        firstNameEncountered = true;
                    }
                }
                else
                {
                    if ((globals->globalDr->getNumFamilyNames() == 0) && isLikelySurname)
                    {
                        nameInfo.type = ntLast;

                        if (!firstNameEncountered)
                        {
                            lastNameFirst = true;
                            numRotate++;
                            if ((hasComma) || (commaIndex == -1))
                                commaPositionAfter = nameInfo.position;
                        }
                    }
                    else
                    {
                        // Check for rare instance of "new" firstname being used
                        PQString errMsg;
                        QString nameUsed;
                        nameInfo.type = ntUnknown;
                        int numSavedFirstNames = static_cast<int>(globals->globalDr->getNumFirstNames());
                        int i = 0;

                        if (numSavedFirstNames == 0)
                        {
                            if (isLikelyGivenName)
                            {
                                nameInfo.type = ntFirst;
                                firstNameEncountered = true;
                            }
                        }
                        else
                        {
                            while ((i < numSavedFirstNames) && (nameInfo.type == ntUnknown))
                            {
                                switch(i)
                                {
                                case 0:
                                    nameUsed = globals->globalDr->getFirstName().getString();
                                    break;

                                case 1:
                                    nameUsed = globals->globalDr->getFirstNameAKA1().getString();
                                    break;

                                case 2:
                                    nameUsed = globals->globalDr->getFirstNameAKA2().getString();
                                    break;

                                }

                                if (isGivenName && ((word.isInformalVersionOf(nameUsed, errMsg) || word.isFormalVersionOf(nameUsed, errMsg))))
                                {
                                    if (numSavedFirstNames < 3)
                                    {
                                        nameInfo.type = ntFirst;
                                        firstNameEncountered = true;
                                    }
                                    else
                                    {
                                        nameInfo.type = ntMiddle;
                                        if (middleName.getLength() > 0)
                                            middleName += space;
                                        middleName += nameInfo.name;
                                    }
                                    handled = true;
                                }

                                if (errMsg.getLength() > 0)
                                {
                                    globals->logMsg(ErrorSQL, errMsg);
                                }

                                i++;
                            }   // end while

                            // Try to identify new unused nicknames
                            if (!handled && hasBookEnds && (word.left(3).lower() == globals->globalDr->getFirstName().left(3).lower()))
                            {
                                if (numSavedFirstNames < 3)
                                {
                                    nameInfo.type = ntFirst;
                                    firstNameEncountered = true;
                                }
                            }
                        }
                    }
                }

                nameStarted = true;
                handled = true;
            }
            else
            {
                if (nameStarted && (word.getLength() == 1) && word.isCapitalized())
                {
                    nameInfo.name = word;
                    nameInfo.type = ntMiddle;
                    nameInfo.position = position;
                    nameInfo.hadBookEnds = hasBookEnds;
                    nameInfo.hadComma = hasComma;
                    nameInfo.matched = wordMatched;
                    position++;
                    middleName += space;
                    middleName += nameInfo.name;
                    handled = true;
                }
            }
        }

        // Coding to catch something like "Dr. Smith"
        if (!nameEnded && nameStarted && !handled && word.isAlpha())
        {
            if ((globals->globalDr->getFirstName().getLength() == 0) && isGivenName)
            {
                middleName = word;
                globals->globalDr->setFirstName(middleName);
                nameInfo.name = word;
                nameInfo.type = ntFirst;
                nameInfo.position = position;
                nameInfo.hadBookEnds = hasBookEnds;
                nameInfo.hadComma = hasComma;
                nameInfo.matched = wordMatched;
                position++;
                firstNameEncountered = true;
                handled = true;
            }

            if (!handled && word.isPrefix())
            {
                nameInfo.type = ntPrefix;
                nameInfo.numWords = word.countWords();
                nameInfo.name = word;
                nameInfo.position = position;
                nameInfo.hadBookEnds = hasBookEnds;
                nameInfo.hadComma = hasComma;
                if (globals->globalDr->getGender() == genderUnknown)
                {
                    if (word.isMaleTitle(lang))
                        globals->globalDr->setGender(Male);
                    else
                        if (word.isFemaleTitle(lang))
                            globals->globalDr->setGender(Female);
                }
                handled = true;
            }

            if (!handled && word.isSuffix())
            {
                if (word.isSuffixKeep())
                {
                    nameInfo.type = ntSuffix;
                    nameInfo.numWords = word.countWords();
                    nameInfo.name = word;
                    nameInfo.position = position;
                    nameInfo.hadBookEnds = hasBookEnds;
                    nameInfo.hadComma = hasComma;
                }
                else  // One of the drop suffixes
                {
                    // Ignore and move on
                }
                handled = true;
            }

            if (!handled && word.isTitle(lang))  // Titles include Mr. Mrs. Ms.
            {
                // Ignore and move on
                handled = true;
                if (globals->globalDr->getGender() == genderUnknown)
                {
                    if (word.isMaleTitle(lang))
                        globals->globalDr->setGender(Male);
                    else
                    {
                        if (word.isFemaleTitle(lang))
                            globals->globalDr->setGender(Female);
                    }
                }
            }

            if (!handled && ((lastNameAttained && !hasBookEnds && !lastNameFirst) || deException))
            {
                // Assume word is no good;
                nameEnded = true;
                handled = true;
            }

            if (!handled && word.isHyphenated())
            {
                nameInfo.name = word;
                nameInfo.type = ntLast;
                nameInfo.position = position;
                nameInfo.hadBookEnds = hasBookEnds;
                nameInfo.hadComma = hasComma;
                nameInfo.matched = wordMatched;
                position++;
                handled = true;
            }

            if (!handled && word.countWords() > 1)
            {
                nameEnded = true;
                handled = true;
            }

            if (!handled)
            {
                if (isEOS() && !lastNameAttained && (globals->globalDr->getNumFamilyNames() == 0))
                    nameInfo.type = ntLast;
                else
                {
                    if (nameInfo.nameStats.frequencyFirst == -1)
                        nameInfo.nameStats.frequencyFirst = countFrequencyFirst(globals->cleanedUpUC, word.getString(), Qt::CaseInsensitive);

                    // Assume it is a middle name or last name, except for special case of starting sentences
                    if (nameInfo.nameStats.frequencyFirst >= 2)
                        nameInfo.type = ntFirst;  // Most likely a nickname since it didn't show up in structured read
                    else
                        nameInfo.type = ntUnknown;
                }

                nameInfo.name = word;
                nameInfo.position = position;
                nameInfo.hadBookEnds = hasBookEnds;
                nameInfo.hadComma = hasComma;
                nameInfo.matched = wordMatched;
                position++;
                handled = true;
            }
        }

        if (!handled && !nameStarted && word.isAlpha() && !word.isRecognized())
        {
            // Most likely scenario is structured read having only included first name (eg. "Jim")
            // with a brand new last name not previously encountered
            if (hasComma)
            {
                lastNameFirst = true;
                numRotate++;
                commaPositionAfter = position;

                nameInfo.name = word;
                nameInfo.type = ntLast;
                nameInfo.position = position;
                nameInfo.hadBookEnds = hasBookEnds;
                nameInfo.hadComma = hasComma;
                nameInfo.matched = wordMatched;
                position++;

                nameStarted = true;
                handled = true;
            }
            else
            {
                // Nothing further coded for now
                // Name will be skipped
                ;
            }
        }

		if (nameInfo.name.getLength() > 0)
        {
            nameInfo.numWords = nameInfo.name.countWords();
            nameInfoList.append(nameInfo);
        }

	} // end while

    // Fill in gaps where possible
    if (!lastNameAttained && nameInfoList[0].hadComma && nameInfoList[0].nameStats.isLikelySurname)
    {
        nameInfoList[0].type = ntLast;
        commaPositionAfter = 1;
        lastNameFirst = true;
        lastNameAttained = true;
    }

	// Re-order the names (rotate left) if a last name appeared first
	if (lastNameFirst)
	{
		if (commaPositionAfter == 0)
			commaPositionAfter = numRotate;
		for (i = 0; i < commaPositionAfter; i++)
        {
            nameInfo = nameInfoList.takeFirst();
            if (nameInfo.type == ntUnknown)
                nameInfo.type = ntLast;
            nameInfoList.append(nameInfo);
        }
        for (i = 0; i < static_cast<unsigned int>(nameInfoList.size()); i++)
            nameInfoList[static_cast<int>(i)].position = i + 1;
	}
	
    GENDER gender = globals->globalDr->getGender();

	// Loop backwards through words
	priorName.type = ntUnknown;
    unsigned int origListSize = static_cast<unsigned int>(nameInfoList.size());

    while (nameInfoList.size() > 0)
	{
        nameInfo = nameInfoList.takeLast();

        // Complete additional analysis for remaining unknowns only
        if (nameInfo.type == ntUnknown)
		{
            QString name = nameInfo.name.getString();
            if (!nameInfo.nameStatsRun)
                nameStatLookup(name.toLower(), globals, nameInfo.nameStats, gender);
            isSurname = nameInfo.nameStats.isSurname;
            isGivenName = nameInfo.nameStats.isGivenName;

            // Check for rare instance of structured read drawing a complete blank, combined with first take being ntUnknown (i.e., must be last name)
            if (isSurname && (globals->globalDr->getLastName().getLength() == 0))
                nameInfo.type = ntLast;
            else
            {

                if (nameInfo.hadBookEnds)
                {
                    // Set type based on lookup with clear indication, otherwise assume
                    // it is either a nickname (male or female) or a maiden name (female only)
                    if (isGivenName && !isSurname)
                        nameInfo.type = ntFirst;
                    else
                    {
                        if (!isGivenName && isSurname)
                            nameInfo.type = ntLast;
                        else
                        {
                            if (gender == Male)
                            {
                                if ((priorName.type == ntMiddle) || (priorName.type == ntLast))
                                    nameInfo.type = ntFirst;
                                else
                                    nameInfo.type = ntLast;
                            }
                            else
                                nameInfo.type = ntFirst;
                        }
                    }
                }
                else
                {
                    // Assume it is a first name if it is the first name on the list (last read in), otherwise
                    // assume it is a middlename unless (Female && Surname)
                    if (nameInfoList.size() == 0)
                        nameInfo.type = ntFirst;
                    else
                    {
                        int frequencyFirst, frequencyTotal;
                        if (nameInfo.nameStats.frequencyFirst == -1)
                            nameInfo.nameStats.frequencyFirst = countFrequencyFirst(globals->cleanedUpUC, name, Qt::CaseSensitive);
                        frequencyFirst = nameInfo.nameStats.frequencyFirst;
                        if (nameInfo.nameStats.frequencyTotal == -1)
                            nameInfo.nameStats.frequencyTotal = countFrequency(globals->cleanedUpUC, name, Qt::CaseSensitive);
                        frequencyTotal = nameInfo.nameStats.frequencyTotal;
                        // Case sensitive used to minimize "Lee" counted within "glee", but all caps will be missed

                        bool onlyMiddleNamePossibility = isGivenName && (globals->globalDr->getMiddleNames().getLength() == 0) && (nameInfo.position == 2) && !nameInfo.nameStats.isLikelySurname;
                        bool surnameExceptionBase = isSurname && (gender == Female) && !(onlyMiddleNamePossibility || nameInfo.nameStats.isLikelyGivenName);
                        bool surnameException1 = (frequencyTotal > 1) && (frequencyFirst == 0);
                        bool surnameException2 = !isGivenName && (nameInfo.position == (origListSize - 1));

                        if (surnameExceptionBase && (surnameException1 || surnameException2))
                            nameInfo.type = ntLast;
                        else
                        {
                            nameInfo.type = ntMiddle;
                            if (middleName.getLength() > 0)
                                middleName += space;
                            middleName += nameInfo.name.proper();
                            if ((gender == Female) && (onlyMiddleNamePossibility || (isSurname && (frequencyTotal == 1) && (frequencyFirst == 0))))
                            {
                                //PQString warningMessage;
                                //warningMessage << "Check middle name: " << nameInfo.name << " as it may be a last name for: \"" << globals->globalDr->getFullName() << "\"  source: "<< globals->globalDr->getURL();
                                //globals->logMsg(ErrorRecord, warningMessage);
                                globals->globalDr->wi.confirmMiddleName = nameInfo.name.getString();
                            }
                        }
                    }
                }
			}
		}

		switch (nameInfo.type)
		{
		case ntMiddle:
            // Do nothing as all newly identified middle names are added via the function return, and
            // all other middle names were matched to an existing middle name
			break;

		case ntSuffix:
            globals->globalDr->setSuffix(nameInfo.name);
			break;

		case ntFirst:
		case ntLast:
		default:
            globals->globalDr->setAlternates(nameInfo);
			break;

		}
		priorName.type = nameInfo.type;
	}

    middleName.cleanUpEnds();
	return middleName;
}

void unstructuredContent::splitIntoBlocks(CONTENTREAD &cr)
{
    // Strip out everything within HTML tags as well
    // Separation of blocks limited to </p>, <br> and </div> html tags

    bool validBlock, endOfBlock;

    OQString block, HTMLtag;
    OQString openTag("<");
    OQString closeTag(">");
    QString space(" ");
    QString period(".");
    unsigned int blocksKept = 0;
    unsigned int blockLength;
    int firstOpenTag, firstCloseTag;

    cr.clear();

    if (this->getLength() == 0)
        return;

    // In the unlikely event we are starting within an HTML tag, discard the starting codes
    firstOpenTag = this->getString().indexOf(QChar('<'));
    firstCloseTag = this->getString().indexOf(QChar('>'));
    bool startWithinHTMLtag = false;
    if ((firstCloseTag >= 0) && (firstOpenTag == -1))
        startWithinHTMLtag = true;
    if ((firstCloseTag >= 0) && (firstOpenTag >= 0) && (firstCloseTag < firstOpenTag))
        startWithinHTMLtag = true;
    if (startWithinHTMLtag)
        moveTo(closeTag.getString());

    while (!isEOS())
    {
        // Read in all data until an open HTML tag "<" is encountered
        block += getUntil(openTag.getString(), 3000);

        // Read the next chunk of data within the HTML tags
        endOfBlock = false;
        if (!isEOS())
        {
            HTMLtag = openTag + getUntil(closeTag.getString(), 500, false);
            endOfBlock = HTMLtag.isEndOfBlockTag();
        }

        // Deal with the block if no more data is to be added to it
        if (isEOS() || endOfBlock)
        {
            block = block.unQuoteHTML();
            block.cleanUpEnds();
            blockLength = block.getLength();
            validBlock = blockLength > 2;

            if (validBlock)
            {
                // Ensure the block ends with a period
                block.removeEnding(space);
                if (block.right(1) != period)
                    block += period;

                block.fixBasicErrors();

                // Next part fixes quotes by reading sentence by sentence
                if (block.fixQuotes())
                {
                    PQString errMsg;
                    errMsg << "Potential issue with mismatched quotes for: " << globals->globalDr->getURL();
                    globals->logMsg(ErrorRecord, errMsg);
                }

                cr.allContent += block;
                cr.allContent += space;

                // Save any block containing the TitleKey into the separate blocks
                bool matched = false;
                unsigned int i = 0;
                unsigned int cutOff = block.countWords();
                if (cutOff > 13)
                    cutOff = 13;
                OQStream checkStream(block);
                PQString checkWord;
                PQString titleKey = globals->globalDr->getTitleKey().lower();
                while (!matched && (i < cutOff))
                {
                    checkWord = checkStream.getWord().getString();
                    checkWord.removeBookEnds();
                    checkWord = checkWord.lower();
                    if (checkWord == titleKey)
                        matched = true;
                    i++;
                }
                if (matched)
                {
                    switch(blocksKept)
                    {
                    case 0:
                        cr.firstBlock = block;
                        break;
                    case 1:
                        cr.secondBlock = block;
                        break;
                    case 2:
                        cr.thirdBlock = block;
                        break;
                    }
                    blocksKept++;
                }
            } // end valid block
            block.clear();  // Since it was either saved or deemed to be garbage
        }
        else
        {
            // Ignore HTML tag and keep reading next chunk of data
            block += space;
        }
    }

    cr.allContent = cr.allContent.getString().replace(QString("  "), QString(" "));
    cr.cleanUpEnds();

    if (cr.firstBlock.getLength() == 0)
        cr.firstBlock = cr.allContent;
}

void unstructuredContent::splitIntoBlocks(CONTENTREAD &cr, QList<QString> &firstWordList)
{
    // Strip out everything within HTML tags as well
    // Separation of blocks limited to </p>, <br> and </div> html tags
    // Separation into sentences achieved by recording positions of sentence starts

    bool validBlock, endOfBlock;

    OQString block, HTMLtag;
    OQString openTag("<");
    OQString closeTag(">");
    QString space(" ");
    QString period(".");
    unsigned int blocksKept = 0;
    unsigned int blockLength;
    int firstOpenTag, firstCloseTag;

    cr.clear();

    if (this->getLength() == 0)
        return;

    // In the unlikely event we are starting within an HTML tag, discard the starting codes
    firstOpenTag = this->getString().indexOf(QChar('<'));
    firstCloseTag = this->getString().indexOf(QChar('>'));
    bool startWithinHTMLtag = false;
    if ((firstCloseTag >= 0) && (firstOpenTag == -1))
        startWithinHTMLtag = true;
    if ((firstCloseTag >= 0) && (firstOpenTag >= 0) && (firstCloseTag < firstOpenTag))
        startWithinHTMLtag = true;
    if (startWithinHTMLtag)
        moveTo(closeTag.getString());

    while (!isEOS())
    {
        // Read in all data until an open HTML tag "<" is encountered
        block += getUntil(openTag.getString(), 3000);

        // Read the next chunk of data within the HTML tags
        endOfBlock = false;
        if (!isEOS())
        {
            HTMLtag = openTag + getUntil(closeTag.getString(), 500, false);
            endOfBlock = HTMLtag.isEndOfBlockTag();
        }

        // Deal with the block if no more data is to be added to it
        if (isEOS() || endOfBlock)
        {
            block = block.unQuoteHTML();
            block.cleanUpEnds();
            blockLength = block.getLength();
            validBlock = blockLength > 2;

            if (validBlock)
            {
                // Ensure the block ends with a period
                block.removeEnding(space);
                if (block.right(1) != period)
                    block += period;

                block.fixBasicErrors();

                // Next part fixes quotes by reading sentence by sentence
                if (block.fixQuotes(firstWordList, globals->globalDr->getGender(), globals->globalDr->getLanguage()))
                {
                    PQString errMsg;
                    errMsg << "Potential issue with mismatched quotes for: " << globals->globalDr->getURL();
                    globals->logMsg(ErrorRecord, errMsg);
                }

                cr.allContent += block;
                cr.allContent += space;

                // Save any block containing the TitleKey into the separate blocks
                bool matched = false;
                unsigned int i = 0;
                unsigned int cutOff = block.countWords();
                if (cutOff > 13)
                    cutOff = 13;
                OQStream checkStream(block);
                PQString checkWord;
                PQString titleKey = globals->globalDr->getTitleKey().lower();
                while (!matched && (i < cutOff))
                {
                    checkWord = checkStream.getWord().getString();
                    checkWord.removeBookEnds();
                    checkWord = checkWord.lower();
                    if (checkWord == titleKey)
                        matched = true;
                    i++;
                }
                if (matched)
                {
                    switch(blocksKept)
                    {
                    case 0:
                        cr.firstBlock = block;
                        break;
                    case 1:
                        cr.secondBlock = block;
                        break;
                    case 2:
                        cr.thirdBlock = block;
                        break;
                    }
                    blocksKept++;
                }
            } // end valid block
            block.clear();  // Since it was either saved or deemed to be garbage
        }
        else
        {
            // Ignore HTML tag and keep reading next chunk of data
            block += space;
        }
    }

    cr.allContent = cr.allContent.getString().replace(QString("  "), QString(" "));
    cr.cleanUpEnds();

    if (cr.firstBlock.getLength() == 0)
        cr.firstBlock = cr.allContent;
}

void unstructuredContent::splitIntoSentences()
{
    QList<QString> dummyList;
    splitIntoSentences(dummyList);
    dummyList.clear();
}

void unstructuredContent::splitIntoSentences(QList<QString> &firstWordList)
{
    beg();
    unstructuredContent sentence;
    OQString firstWord, nextWord;
    bool hadComma;
    bool pastFirstRealSentence = false;
    int sentenceCount = 0;

    while (!isEOS())
    {
        sentence = getSentence(pastFirstRealSentence);
        sentenceCount++;

        // Make adjustments to initial headings/titles if appropriate
        if ((sentenceCount == 1) && (sentence.countWords() == 1))
        {
            NAMESTATS nameStats1, nameStats2;
            firstWord = sentence.getWord();
            nameStatLookup(firstWord.getString(), globals, nameStats1);
            if (nameStats1.isLikelySurname)
            {
                nextWord = peekAtWord();
                nameStatLookup(nextWord.getString(), globals, nameStats2);
                if (nameStats2.isLikelyGivenName)
                {
                    replace(sentence.getLength(), 1, QString(","));
                    clearSentenceStartPositions();
                    beg();
                    sentence = getSentence();
                }
            }
            else
                sentence.beg();
        }

        firstWord = sentence.getWord();
        if (firstWord.getLength() > 0)
        {
            firstWord.removeEnding(" ");
            firstWord.removeBookEnds(QUOTES | PARENTHESES);
            hadComma = firstWord.removeEnding(COMMA);
            firstWord.removeEnding(PUNCTUATION);
            firstWord = firstWord.lower();
            firstWord.removePossessive();
            if ((firstWord.getLength() > 0) && !hadComma && (firstWord.isGenderWord(globals->globalDr->getGender(), globals->globalDr->getLanguage()) || !firstWord.isRecognized()))
                firstWordList.append(firstWord.getString());
        }
    }

    if (sentenceStartPositions.size() > 0)
        lastSentenceStartPosition = sentenceStartPositions.at(sentenceStartPositions.size() - 1);
}

OQStream unstructuredContent::getSentence(const LANGUAGE &language, const int sentenceNum)
{
    QList<QString> emptyFirstNames;
    bool realSentenceEncountered = true;
    return getSentence(emptyFirstNames, realSentenceEncountered, language, sentenceNum);
}

OQStream unstructuredContent::getSentence(bool &realSentenceEncountered, const LANGUAGE &language, const int sentenceNum)
{
    QList<QString> emptyFirstNames;
    return getSentence(emptyFirstNames, realSentenceEncountered, language, sentenceNum);
}

OQStream unstructuredContent::getSentence(const QList<QString> &firstNames, bool &realSentenceEncountered, const LANGUAGE &language, const int sentenceNum)
{
    // If the stream has previously been parsed sufficiently in whole or in part, a direct read is returned, otherwise
    // the sentence is formed and returned one character at at time. If sentenceNum == 0, read from current position,
    // otherwise find the appropriate sentence to return.

    OQStream result, sentence;
    bool fullyParsed, partiallyParsed, neverParsed;

    if (((sentenceNum == 0) && isEOS()) || (itsString.size() == 0))
        return result;

    if (sentenceStartPositions.size() == 0)
    {
        fullyParsed = false;
        partiallyParsed = false;
        neverParsed = true;
    }
    else
    {
        neverParsed = false;
        if (lastSentenceStartPosition == -1)
        {
            fullyParsed = false;
            partiallyParsed = true;
        }
        else
        {
            fullyParsed = true;
            partiallyParsed = false;
        }
    }

    if (fullyParsed && (sentenceNum > sentenceStartPositions.size()))
    {
        // Return nothing
        return result;
    }

    if (fullyParsed || (partiallyParsed && (position < sentenceStartPositions.at(sentenceStartPositions.size() - 1))))
    {
        /*****************/
        /*  Direct read  */
        /*****************/

        // Start and end points are directly determinable
        int startPos, endPos;
        int sentenceCount, sentenceStartPosition;

        // Set start position for read
        if (sentenceNum == 0)
            startPos = position;
        else
            startPos = sentenceStartPositions.at(sentenceNum - 1);

        // Set end position for read
        if (fullyParsed && (sentenceNum == sentenceStartPositions.size()))
        {
            endPos = itsString.size() - 1;
            EOS = true;
        }
        else
        {
            if (sentenceNum > 0)
                endPos = sentenceStartPositions.at(sentenceNum) - 1;
            else
            {
                sentenceCount = 0;
                sentenceStartPosition = 0;
                while (sentenceStartPosition <= position)
                {
                    sentenceCount++;
                    if (sentenceCount >= sentenceStartPositions.size())
                    {
                        sentenceStartPosition = itsString.size();
                        EOS = true;
                    }
                    else
                        sentenceStartPosition = sentenceStartPositions.at(sentenceCount);
                }
                endPos = sentenceStartPosition - 1;
            }
        }

        result = itsString.mid(startPos, endPos - startPos + 1);
        position = endPos + 1;
    }
    else
    {
        /*******************/
        /* Create Sentence */
        /*******************/

        // Record and store parsing as it occurs
        int initialPosition = position;
        int currentSentenceNum;
        bool targetSentenceRead = false;

        if (neverParsed)
        {
            position = 0;
            currentSentenceNum = 1;
            sentenceStartPositions.append(position);
        }
        else
        {
            position = sentenceStartPositions.at(sentenceStartPositions.size() - 1);
            currentSentenceNum = sentenceStartPositions.size();
        }

        while (!isEOS() && (position <= initialPosition) && !targetSentenceRead)
        {
            sentence = OQStream::getSentence(firstNames, realSentenceEncountered, language);
            if ((sentenceNum == 0) || (sentenceNum == currentSentenceNum))
            {
                targetSentenceRead = true;
                result = sentence;
            }
            else
                currentSentenceNum++;

            if (isEOS())
                lastSentenceStartPosition = sentenceStartPositions.at(sentenceStartPositions.size() - 1);
            else
                sentenceStartPositions.append(position);

        }
    }
    result.cleanUpEnds();
    return result;
}

void unstructuredContent::prepareNamesToBeRead(bool removeRecognized)
{
    // The  logic is aligned with justInitialNames contained in determineLanguageAndGender
    // It is assumed that the first words are names

    this->removeHTMLtags();
    this->unQuoteHTML();
    this->removeEnding(SPACE);

    OQString word, originalWord, nextWord;
    OQString newString, firstChar, lastChar;
    bool hadComma, hadPeriod, wordAssessed, breakExemption, inQuotes, isForeign, standAloneComma;
    unsigned int wordLength;

    QString comma(",");
    QString period(".");
    QString space(" ");
    unsigned int numCommas = 0;
    bool dividerBeforeWord = false;
    bool dividerAfterWord = false;
    bool namesFinished = false;
    bool keepWord, isSurname, hyphenatedName;

    OQString tempString(itsString);
    OQStream tempStream(tempString);
    tempString.clear();

    while (!tempStream.isEOS() && !namesFinished)
    {
        originalWord = tempStream.getWord(true);
        while ((originalWord.getLength() == 0) && !tempStream.isEOS())
        {
            originalWord = tempStream.getWord(true);
        }
        hadComma = originalWord.removeEnding(comma);
        standAloneComma = hadComma && (originalWord.getLength() == 0);
        originalWord.removeInternalPeriods();
        hadPeriod = originalWord.removeEnding(period);
        word = originalWord.lower();  // Original word as modified

        keepWord = true;
        breakExemption = false;
        wordAssessed = false;
        hyphenatedName = false;

        /*************************************************/
        /* Run some checks to get information about word */
        /* to assist with picking off initial names only */
        /*************************************************/

        firstChar = word.left(1);
        lastChar = word.right(1);
        isForeign = word.isForeignLanguage();

        // If second comma is encountered, consider it a divider
        if (hadComma)
        {
            numCommas++;
            if (numCommas >= 2)
                dividerAfterWord = true;
        }

        // Look for other dividers
        // a. lastname-lastname (i.e. hyphenated) name won't have lastChar as divider, so no break triggered between the names
        // b. lastname- Text will trigger a break due to space (unless "Text" is a lastname)
        // c. lastname - Text will trigger a break due to hyphen (unless "Text" is a lastname)
        // d. lastname -Text will trigger a break due to space (unless "Text" is a lastname)
        if (((lastChar.getCharType() & DIVIDER) == DIVIDER) && (firstChar != lastChar))  // case b.
        {
            nextWord = tempStream.peekAtWord().lower();
            if (nextWord.isAlpha())
                isSurname = (surnameLookup(nextWord.getString(), globals) > 0);
            else
                isSurname = false;
            if (!isSurname && !nextWord.isSaint())
            {
                word.dropRight(1);
                dividerAfterWord = true;
            }
            else
            {
                breakExemption = true;
                wordAssessed = true;
                if (word.isAlpha())
                {
                    hyphenatedName = true;
                    keepWord = true;
                }
            }
        }

        if ((firstChar.getCharType() & DIVIDER) == DIVIDER)  // cases c. and d.
        {
            nextWord = tempStream.peekAtWord().lower();
            if (nextWord.isAlpha())
                isSurname = (surnameLookup(nextWord.getString(), globals) > 0);
            else
                isSurname = false;
            if (!isSurname && !nextWord.isSaint())
            {
                dividerBeforeWord = true;
                keepWord = false;
                wordAssessed = true;
            }
            else
            {
                breakExemption = true;
                wordAssessed = true;
                if (word.isAlpha())
                {
                    hyphenatedName = true;
                    keepWord = true;
                }
            }
        }

        inQuotes = word.hasBookEnds(QUOTES | PARENTHESES);

        // Fix data entry errors
        if (!inQuotes)
        {
            if (word.removeEnding(QUOTES))
                originalWord.removeEnding(QUOTES);
        }

        // Flag likely double memorials, which always read in incorrectly
        if ((word == OQString("and")) || (word.getString().compare(QChar('&')) == 0))
        {
            PQString errMsg;
            errMsg << "Suspected double memorial for: " << globals->globalDr->getURL();
            //globals->logMsg(ErrorRecord, errMsg);
            globals->globalDr->wi.doubleMemorialFlag = 1;
        }

        /*************************************************/
        /*                Analyze word                   */
        /*************************************************/

        wordLength = word.getLength();
        if (inQuotes || (word.isAlpha() && (wordLength > 0)) || hyphenatedName || isForeign)
        {
            if (inQuotes)
            {
                keepWord = true;
                wordAssessed = true;
                breakExemption = true;

                // If a comma exists within quotes, assume it is a list of former last names
                for (unsigned int i = 0; i < originalWord.getLength(); i++)
                {
                    if (originalWord[i] == QChar(','))
                        originalWord[i] = QChar('|');
                }
            }

            if (isForeign)
            {
                keepWord = false;
                wordAssessed = true;
                breakExemption = true;
            }

            // Check for initials or compound educational degrees starting with "B." or "M."
            if (!wordAssessed && (wordLength == 1))
            {
                if ((word != OQString("b")) && (word != OQString("m")))
                {
                    keepWord = true;
                    wordAssessed = true;
                    breakExemption = true;
                }
                else  // Check for degrees such as B.Ed., B.Voc.Ed., M.Sc.
                {
                    if (!tempStream.isEOS())
                    {
                        nextWord = tempStream.peekAtWord().lower();
                        nextWord.removeEnding(comma);
                        nextWord.removeInternalPeriods();
                        nextWord.removeEnding(period);
                        if (nextWord.isDegree())
                        {
                            // Discard the full degree
                            originalWord = tempStream.getWord();
                            keepWord = false;
                            wordAssessed = true;

                            bool keepGoing = true;
                            while (keepGoing && !tempStream.isEOS())
                            {
                                nextWord = tempStream.peekAtWord().lower();
                                nextWord.removeEnding(comma);
                                nextWord.removeInternalPeriods();
                                nextWord.removeEnding(period);
                                if (nextWord.isDegree())
                                    originalWord = tempStream.getWord();
                                else
                                    keepGoing = false;
                            }
                        }
                        else
                        {
                            keepWord = true;
                            wordAssessed = true;
                            breakExemption = true;
                        }
                    }
                }
            }

            if (!wordAssessed && word.isSuffixDrop())
            {
                // Allow for potentially different processing - same for now
                keepWord = false;
                wordAssessed = true;
                breakExemption = true;
            }

            if (!wordAssessed && (word.isTitle() || word.isSaint() || word.isPrefix() || word.isSuffixKeep()))
            {
                keepWord = true;
                wordAssessed = true;
                breakExemption = true;
            }

            if (removeRecognized)
            {
                if (!wordAssessed && word.isEnglish())
                {
                    keepWord = false;
                    wordAssessed = true;
                }

                if (!wordAssessed && word.isFrench())
                {
                    keepWord = false;
                    wordAssessed = true;
                }

                if (!wordAssessed && word.isSpanish())
                {
                    keepWord = false;
                    wordAssessed = true;
                }

                if (!wordAssessed && word.isEnglishMale())
                {
                    keepWord = false;
                    wordAssessed = true;
                }

                if (!wordAssessed && word.isEnglishFemale())
                {
                    keepWord = false;
                    wordAssessed = true;
                }

                if (!wordAssessed && word.isFrenchMale())
                {
                    keepWord = false;
                    wordAssessed = true;
                }

                if (!wordAssessed && word.isFrenchFemale())
                {
                    keepWord = false;
                    wordAssessed = true;
                }

                if (!wordAssessed && word.isSpanishMale())
                {
                    keepWord = false;
                    wordAssessed = true;
                }

                if (!wordAssessed && word.isSpanishFemale() && (word != OQString("ella")))
                {
                    keepWord = false;
                    wordAssessed = true;
                }
            }

            // If a matched word is found, assume name is finished
            if ((wordAssessed && !breakExemption) || dividerBeforeWord)
                namesFinished = true;

            if (keepWord)
            {
                if (hyphenatedName)
                {
                    newString.removeEnding(space);
                    newString += QString("-");
                }
                else
                {
                    newString += originalWord;
                    if (hadPeriod)
                        newString += period;
                    if ((hadComma) && (numCommas < 2))
                        newString += comma;
                    if (dividerAfterWord)
                        namesFinished = true;
                    if (!namesFinished)
                        newString += space;
                }
            }

        }  // end if word isAlpha
        else
        {
            if (standAloneComma)
            {
                newString.dropRight(1);
                newString += comma;
                newString += space;
            }
            else
            {
                // Since the word is not "alpha", it can't be a name and names must be finished
                namesFinished = true;
            }
        }
    }  // end while

    // Tidy up, remove spaces from compound names and and store the other strings
    newString.removeEnding(space);
    newString.cleanUpEnds();
    newString.compressCompoundNames();

    // Copy cleaned up list of words to itsString
    setItsString(newString.getString());
}

void unstructuredContent::pickOffNames()
{
    // This logic only works once structured names have already been read in
    // Funny ordering is an attempt to avoid the most cycle intensive checks

    OQString newString, originalWord, word, nextWord, potentialTrailer;
    OQString space(" ");
    bool capitalized, hadBookEnds, hadComma, lastWasCompound, isAboriginal;
    bool keepGoing = true;
    bool isCompound = false;
    bool standAloneComma = false;
    unsigned int namesKept = 0;
    NAMETYPE nameType;
    GENDER gender = globals->globalDr->getGender();
    LANGUAGE lang = globals->globalDr->getLanguage();

    beg();
    while (!EOS && keepGoing && (namesKept < 10))
    {
        originalWord = getWord(true);
        word = originalWord;
        hadBookEnds = word.removeBookEnds();
        capitalized = word.left(1).isCapitalized();
        standAloneComma = (word.getLength() == 1) && (word == OQString(","));
        word = word.lower();
        hadComma = word.removeEnding(",");
        word.removeEnding(PUNCTUATION);

        if (word.isHyphenated())
        {
            int index = word.findPosition(PQString("-"));
            OQString nameA = word.left(index);
            OQString nameB = word.right(word.getLength() - index - 1);
            if (nameA.isSaint())
                word = nameA + OQString(".") + nameB;
            else
            {
                word = nameA;
                backward(originalWord.getLength() - word.getLength() - 1);
                originalWord = originalWord.left(word.getLength());
            }
        }

        isAboriginal = !hadComma && word.isAboriginalName(peekAtWord());
        lastWasCompound = isCompound;
        isCompound = (word.isCompoundName() && !((lang == french) && (word == OQString("de")))) || (capitalized && isAboriginal && !hadBookEnds);

        if (word.getLength() < 2)
        {
            if (word.getLength() == 1)
            {
                if (globals->globalDr->isAnInitial(word))
                {
                    newString += originalWord;
                    newString += space;
                    namesKept++;
                }
                else
                {
                    // Decision made based on the next word available
                    nextWord = peekAtWord(true).lower();
                    nextWord.removeBookEnds();
                    nextWord.removeEnding(PUNCTUATION);

                    PQString errMsg;
                    bool acceptableFirst, acceptableAll;

                    acceptableFirst = globals->globalDr->isAFirstName(nextWord) || nextWord.isInformalVersionOf(globals->globalDr->getFirstName().getString(), errMsg ) ||
                                      nextWord.isFormalVersionOf(globals->globalDr->getFirstName().getString(), errMsg );
                    acceptableAll  = acceptableFirst || globals->globalDr->isASavedName(nextWord);

                    // Look for key situation of "LASTNAME - FirstName MiddleName LastName ....."
                    if (((word.getCharType() & HYPHENATED) == HYPHENATED) && (namesKept == 1))
                    {
                        if (acceptableFirst)
                        {
                            potentialTrailer = newString;   // Save to potentially add back at end
                            potentialTrailer.removeEnding(SPACE);
                            newString.clear();
                            namesKept = 0;
                        }
                        else
                            keepGoing = false;

                        if (errMsg.getLength() > 0)
                            globals->logMsg(ErrorSQL, errMsg);
                    }
                    else
                    {
                        // Check for a new initial
                        if (capitalized && acceptableAll)
                        {
                            newString += originalWord;
                            newString += space;
                            namesKept++;
                        }
                        else
                            keepGoing = false;
                    }
                }
            }
            else
            {
                if (standAloneComma)
                {
                    newString.removeEnding(" ");
                    newString += OQString(",");
                    newString += space;
                }
            }
        }
        else // Dealing with length of at least 2
        {
            // Deal with problematic "will be"
            if (word == OQString("will"))
            {
                nextWord = peekAtWord().lower();
                if ((nextWord == OQString("be")) || (nextWord == OQString("always")) || (nextWord == OQString("not")))
                    keepGoing = false;
            }

            if (keepGoing && word.removeLeadingNeeEtAl())
            {
                if (word.getLength() > 0)
                {
                    newString += originalWord;
                    newString += space;
                    namesKept++;
                }
            }
            else
            {
                if (keepGoing && word.isAlpha())
                {
                    nameType = globals->globalDr->isAName(word);
                    if (static_cast<bool>(nameType))
                    {

                        if (keepGoing && (nameType >= ntFirst) && (nameType <= ntLast))
                        {
                            newString += originalWord;
                            newString += space;
                            namesKept++;
                        }
                        else
                        {
                            if ((nameType == ntPrefix) || (nameType == ntSuffix))
                            // Don't save word but keep going
                            ;
                        }
                    }
                    else
                    {
                        if (word.isTitle(lang) || word.isPrefix() || word.isSuffix())
                        {
                            // If gender unknown and title, then use
                            if ((gender == genderUnknown) && word.isTitle())
                            {
                                if (word.isMaleTitle(lang))
                                    globals->globalDr->setGender(Male);
                                else
                                {
                                    if (word.isFemaleTitle(lang))
                                        globals->globalDr->setGender(Female);
                                }
                            }
                        }
                        else
                        {
                            if (word.isSaint() || isCompound)
                            {
                                // Save, but don't count as a separate name
                                bool include = true;
                                if ((globals->globalDr->getLanguage() == french) && (word == OQString("le")))
                                {
                                    OQString checkNext = peekAtWord();
                                    if (checkNext.isNumeric())
                                    {
                                        keepGoing = false;
                                        include = false;
                                    }
                                }

                                if (include)
                                {
                                    newString += originalWord;
                                    newString += space;
                                }
                            }
                            else
                            {
                                if (word.isRecognized() || (word.removeLeading("-") && word.isRecognized()))
                                {
                                    keepGoing = false;
                                    // Note that prior recognized hits on isTitle, isSaint, isCompoundName, isSuffix
                                    // and isPrefix have all been checked already and come back negative
                                }
                                else
                                {
                                    if (static_cast<bool>(word.isWrittenMonth(globals->globalDr->getLanguage())))
                                    {
                                        nextWord = peekAtWord(true);
                                        nextWord.removeEnding(PUNCTUATION);
                                        if (nextWord.isAlphaNumeric() && !nextWord.isAlpha())
                                            keepGoing = false;
                                    }
                                    else
                                    {
                                        if (capitalized)
                                        {
                                            // Now get into database searches
                                            nextWord = peekAtWord(true).lower();
                                            nextWord.removeBookEnds();
                                            nextWord.removeEnding(PUNCTUATION);

                                            GENDER gender = globals->globalDr->getGender();
                                            if (lastWasCompound || hadBookEnds || nextWord.removeLeadingNeeEtAl() ||
                                                (surnameLookup(word.getString(), globals) > 0) || givenNameLookup(word.getString(), globals, gender) ||
                                                (surnameLookup(nextWord.getString(), globals) > 0) || givenNameLookup(nextWord.getString(), globals, gender))
                                            {
                                                newString += originalWord;
                                                newString += space;
                                                namesKept++;
                                            }
                                        }
                                        else
                                            keepGoing = false;
                                    }
                                }
                            }
                        }
                    }
                }
                else    // word is not alpha
                {
                    keepGoing = false;
                }
            }
        }
    }

    newString.removeEnding(QString(" "));
    newString.removeEnding(PUNCTUATION);
    newString.removeEnding(DIVIDER);

    // Add back potential trailer (last name) if required
    if (potentialTrailer.getLength() > 0)
    {
        QString checkString = newString.getString();
        int position = checkString.indexOf(potentialTrailer.getString(), 0, Qt::CaseInsensitive);
        if (position == -1)
        {
            newString += space;
            newString += potentialTrailer;
        }
    }

/*    // Eliminate potential extraneous information at end (locations, suffixes)
    OQStream tempStream(newString);
    newString.clear();
    keepGoing = true;

    while (keepGoing && !tempStream.isEOS())
    {
        word = tempStream.getWord(true).lower();
        word.removeEnding(PUNCTUATION);
        word.removeBookEnds(PARENTHESES | QUOTES);

        if (!globals->globalDr->isASavedName(word))
            keepGoing = word.existsInDB(globals);



    } */

    setItsString(newString.getString());
}


void unstructuredContent::fixHyphenatedNames()
{
    itsString.replace(QString("/"), QString("-"));
    itsString.replace(QString(" -"), QString("-"));
    itsString.replace(QString("- "), QString("-"));

    // Fix LASTNAME-FirstName MiddleName
    OQString string, fixedString;
    QString nameA, nameB;
    NAMESTATS nameStatsA, nameStatsB;
    bool hasComma, hasHyphen;
    int indexC, indexH;
    indexH = itsString.indexOf("-");
    hasHyphen = (indexH >= 0);
    if (hasHyphen)
    {
        indexC = itsString.indexOf(",");
        hasComma = (indexC >=0);
        if (hasComma)
        {
            if (indexC > indexH)
                return;
            else
            {
                // Not sure what to do yet, if anything
                return;
            }
        }
        else
        {
            // Need to find the part of the string containing the hyphenated words
            beg();

            while (!isEOS())
            {
                string = getWord();
                if (string.isHyphenated())
                {
                    string.removeEnding(PUNCTUATION);
                    while (string.isPrefix() && !isEOS())
                    {
                        string = getWord();
                        string.removeEnding(PUNCTUATION);
                    }

                    indexH = string.getString().indexOf("-");
                    nameA = string.left(static_cast<unsigned int>(indexH)).proper().getString();
                    nameB = string.right(string.getLength() - static_cast<unsigned int>(indexH) - 1).proper().getString();

                    if (OQString(nameA).isSaint())
                        string = nameA + QString(".") + nameB;
                    else
                    {
                        nameStatLookup(nameA, globals, nameStatsA, genderUnknown);
                        nameStatLookup(nameB, globals, nameStatsB, genderUnknown);

                        if (nameStatsA.isSurname && !nameStatsA.isGivenName && !nameStatsB.isSurname && nameStatsB.isGivenName)
                            string = nameA + QString(", ") + nameB;
                    }
                }

                if (fixedString.getLength() > 0)
                    fixedString += OQString(" ");

                fixedString += string;
            }

            setItsString(fixedString.getString());
        }
    }
}

/*void unstructuredContent::runStdProcessing(CONTENTREAD &bc, QList<QString> &firstWordList)
{
    simplify(true);
    //removeLeading(QString("\n"));
    //splitIntoBlocks(bc, firstWordList);   // Duplicates certain checks/fixes as this function requires HTML to work
    //splitIntoBlocks(bc);                    // Duplicates certain checks/fixes as this function requires HTML to work
    replaceHTMLentities();
    removeHTMLtags(true);
    removeBlankSentences();
    if (fixQuotes())
    {
        PQString errMsg;
        errMsg << "Potential issue with mismatched quotes for: " << globals->globalDr->getURL();
        globals->logMsg(ErrorRecord, errMsg);
    }
    fixBasicErrors();
    cleanUpEnds();
    bc.select(globals->globalDr->getTitleKey());

    splitIntoSentences(firstWordList);
}*/

void unstructuredContent::readLastNameFirst(QList<NAMESTATS> &nameStatsList)
{
    PQString result;

    // Reorganize to first name first
    int commaPos = itsString.indexOf(",");
    if (commaPos < 1)
        result = itsString;
    else
    {
        result = itsString.right(itsString.length() - commaPos - 1);
        result.removeLeading(" ");
        result += PQString(" ");
        result += itsString.left(commaPos);
    }

    setItsString(result.getString());
    readFirstNameFirst(nameStatsList);
}

void unstructuredContent::readFirstNameFirst(QList<NAMESTATS> &nameStatsList)
{
    NAMEINFO nameInfo;
    GENDER gender, workingGender;
    LANGUAGE lang;
    OQString word, tempWord, prefix, suffix, compoundName;
    unsigned int numLastNames, numFinalLastNames, numMiddleNames, numFirstNames, numWords, i;
    unsigned int totalWordsRemaining, totalBookEndsRemaining, totalSuffixesRemaining;
    bool notFinished, insertInstead, handled;
    bool isSurname, isGivenName, isInitial, isInitial1, isInitial2, potentialPrefix;
    bool inParentheses, inQuotes, nameIsAllCaps;
    bool firstNameLoaded, skipWord, isMaidenName;
    double unisex = 0.5;

    QList<OQString> names;
    QList<QString> firstNames, workingFirstNameList;
    NAMESTATS nameStats;

    // Assume that stream has already been scanned for commas, and none exist
    // Premise is to drop all suffixes and prefixes and just deal with names
    // Anything in parentheses or quotes is assigned to be a first name or last name
    // Otherwise try to count number of additional last names to then calculate number of middle names

    OQString temp(itsString);
    QString space(" ");
	if(temp.cleanUpEnds())
		*this = temp;

    // Setup
    // Assume at least one first name (in position 0) and only add to first name total based on names in quotes and parentheses.
    // For the number of last names, we start at zero and add what can be ascertained, recognizing for a brand new last
    // name ever encountered, the ending total can be zero.
    // The last (or second last word) will be deemed to be a last name if no others encountered along the way.
    // Additional coding to split up hyphenated names.
    // Logic at front end to simplify neeEtAl situations if possible as issues otherwise exist if they follow a compound name.

    numFirstNames = numMiddleNames = numLastNames = numFinalLastNames = 0;
    totalWordsRemaining = totalBookEndsRemaining = totalSuffixesRemaining = 0;
    firstNameLoaded = false;
    isSurname = isGivenName = false;

    // Read in gender, which may change along the way if it was not set before this function was called
    lang = globals->globalDr->getLanguage();
    gender = globals->globalDr->getGender();
    workingGender = gender;
    if ((workingGender == genderUnknown) && (globals->globalDr->getNumFirstNames() > 0))
    {
        double unisex = genderLookup(globals->globalDr, globals);
        if (unisex >= 0.9)
            workingGender = Male;
        else
        {
            if (unisex <= 0.1)
                workingGender = Female;
        }
    }

    // Remove neeEtAl where possible, or potential reference to location
    beg();
    OQString newString, originalWord;
    bool wordRemoved;
    bool firstWord = true;
    while (!EOS)
    {
        word = getWord(true);
        if (word.getLength() > 0)
        {
            wordRemoved = false;
            if (firstWord)
            {
                firstWord = false;
                if (word.isTitle(lang, gender))
                {
                    wordRemoved = true;
                    if (gender == genderUnknown)
                    {
                        if (word.isMaleTitle(lang))
                        {
                            gender = Male;
                            globals->globalDr->setGender(Male);
                            workingGender = Male;
                        }
                        else
                        {
                            if (word.isFemaleTitle(lang))
                            {
                                gender = Female;
                                globals->globalDr->setGender(Female);
                                workingGender = Female;
                            }
                        }
                    }
                }
            }

            originalWord = word;
            if (!wordRemoved && (word.removeBookEnds(PARENTHESES) || word.removeBookEnds(QUOTES)))
            {
                if (word.removeLeadingNeeEtAl(globals->globalDr->getLanguage()))
                {
                    globals->globalDr->setNeeEtAlEncountered(true);
                    globals->globalDr->setMaidenNames(word);
                    globals->globalDr->setGender(Female);
                    workingGender = Female;
                    wordRemoved = true;
                }
                else
                {
                    // Keep word if not last word
                    // If last, more checking
                    if (!isEOS())
                    {
                        QList<OQString> wordList = word.createList();
                        if ((wordList.size() == 1) && (workingGender == Male) && (word.getLength() < 12))
                        {
                            // Word length cutoff intended to catch long aborigional names
                            globals->globalDr->setFirstNames(word);
                            wordRemoved = true;
                        }
                        else
                        {
                            if (areUniquelySurnames(wordList, globals, globals->globalDr->getGender()))
                            {
                                wordList = word.createList();
                                globals->globalDr->setFamilyNames(wordList);
                                wordRemoved = true;
                            }
                            else
                            {
                                // Assume to be a location if not a name
                                wordList = word.createList();
                                if (!areAllNames(wordList, globals))
                                    wordRemoved = true;
                            }
                        }
                    }
                }
            }

            if (!wordRemoved)
            {
                newString += originalWord;
                newString += space;

                if (workingGender == genderUnknown)
                {
                    nameStatLookup(word.getString(), globals, nameStats, genderUnknown);
                    if (nameStats.isLikelyGivenName)
                        workingFirstNameList.append(word.getString());

                    double unisex = genderLookup(workingFirstNameList, globals);
                    if (unisex >= 0.9)
                        workingGender = Male;
                    else
                    {
                        if (unisex <= 0.1)
                            workingGender = Female;
                    }
                }
            }
        }
    }
    newString.cleanUpEnds();
    newString.compressCompoundNames();
    *this = newString;   

    beg();
    nameIsAllCaps = this->isAllCaps();
    while (!EOS)
    {
        word = getWord(true);
        if (word.getLength() > 0)
            totalWordsRemaining++;

        if (word.hasBookEnds(QUOTES | PARENTHESES))
            totalBookEndsRemaining++;

        if (word.isSuffix())
            totalSuffixesRemaining++;
    }
	
    // Check for potential flipping of names (easy case only) like Smith George
    // ValidateNameOrder may get run again later
    if((totalWordsRemaining >= 2) && (totalBookEndsRemaining == 0) && (totalSuffixesRemaining == 0))
        validateNameOrder();

    beg();
    while (!EOS)
	{
        word = getWord(true);
        totalWordsRemaining--;
        handled = false;
        notFinished = false;
        insertInstead = false;
        //potentialCompoundName = false;
        potentialPrefix = false;
        skipWord = false;
        isMaidenName = false;
        isInitial = false;
        numWords = 1;

        if (word.getLength() > 0)
		{
            inParentheses = word.removeBookEnds(PARENTHESES);
            if (inParentheses)
                word.removeBookEnds(PARENTHESES);       // Needed for weird entries
            inQuotes = word.removeBookEnds(QUOTES);
            if (inQuotes)
                word.removeBookEnds(QUOTES);            // Needed for weird entries
            isMaidenName = globals->globalDr->isALastName(word);  // Only potential name loaded to date

            if (isMaidenName)
            {
                numLastNames++;
                if (!(inParentheses || inQuotes))
                    numFinalLastNames++;
                handled = true;
            }
            else
            {
                if (inParentheses || inQuotes)
                {
                    word.cleanUpEnds();
                    totalBookEndsRemaining--;
                    // The following small section should now be redundant
                    if (word.removeLeadingNeeEtAl())
                    {
                        numLastNames++;
                        handled = true;
                        if (word.isFoundIn(problematicFemaleMiddleNames, 1))
                            globals->globalDr->wi.nameWarningException = true;
                        globals->globalDr->setNeeEtAlEncountered(true);
                        PQString pln = globals->globalDr->getParentsLastName();
                        if ((pln.getLength() > 0) && (pln != nameInfo.name))
                        {
                            globals->globalDr->removeFromLastNames(pln);
                            globals->globalDr->setParentsLastName("");
                        }
                    }

                    if (!handled)
                        numWords = word.countWords();

                    if (numWords > 1)
                    {
                        // If more than one word and was in parentheses, but not a compound name or a saint, special coding required
                        if (!handled)
                        {
                            int j = 0;
                            bool allMiddleNames = true;
                            bool allLastNames = true;
                            QString singleWord;
                            QList<QString> nameList = word.getString().split(" ", QString::SkipEmptyParts);
                            while ((j < nameList.size()) && (allMiddleNames || allLastNames))
                            {
                                singleWord = nameList.at(j).toLower();
                                allMiddleNames = allMiddleNames && givenNameLookup(singleWord, globals, gender);
                                allLastNames = allLastNames && (surnameLookup(singleWord, globals) > 0);
                                j++;
                            }

                            if (allMiddleNames || allLastNames)
                            {
                                for (j = 0; j < nameList.size(); j++)
                                {
                                    if (allMiddleNames)
                                    {
                                        numMiddleNames++;
                                        if (j != (nameList.size() - 1))  // Last load will occur at bottom
                                            names.append(nameList.at(j));
                                    }

                                    if (allLastNames)
                                    {
                                        numLastNames++;
                                        if (!(inParentheses || inQuotes))
                                            numFinalLastNames++;
                                        if (j != (nameList.size() - 1))  // Last load will occur at bottom
                                            names.append(nameList.at(j));
                                    }
                                }    // end for loop

                                handled = true;
                            }
                        }

                        // All remaining cases are problematic
                        if (!handled)
                        {
                            PQString errMsg;
                            errMsg << "Couldn't determine how to handle (" << word << ") for: " << globals->globalDr->getURL();
                            //globals->logMsg(ErrorRecord, errMsg);
                            globals->globalDr->wi.confirmTreatmentName = word.getString();

                            skipWord = true;
                            handled = true;
                            notFinished = false;
                        }
                    }
                }
                else
                {
                    // Check for naked "nee"
                    if (word.removeLeadingNeeEtAl())
                    {
                        word = getWord(true);
                        word.cleanUpEnds();
                        if (word.getLength() > 1)
                        {
                            numLastNames++;
                            handled = true;
                            globals->globalDr->setNeeEtAlEncountered(true);
                            if (word.isFoundIn(problematicFemaleMiddleNames, 1))
                                globals->globalDr->wi.nameWarningException = true;
                            PQString pln = globals->globalDr->getParentsLastName();
                            if ((pln.getLength() > 0) && (pln != nameInfo.name))
                            {
                                globals->globalDr->removeFromLastNames(pln);
                                globals->globalDr->setParentsLastName("");
                            }
                        }
                    }
                }

                potentialPrefix = (numFirstNames == 0) && (word.isPrefix() || word.isTitle(lang, gender));

                // Check for initial
                isInitial1 = isInitial2 = false;
                if (word.getLength() == 1)
                    isInitial1 = true;
                if (word.getLength() == 2)
                    isInitial2 = word.removeEnding(QString("."));
                isInitial = isInitial1 || isInitial2;

                // Remove potential duplicates
                if (names.contains(word.proper()))
                {
                    skipWord = true;
                    handled = true;
                }
            }

            // Deal with prefixes
            // If prefix found, new word read in, should status cannot change to "handled"
            if (!handled)
                notFinished = true;
            while (notFinished && !isInitial && potentialPrefix)  // Additional complexity due to double prefix such as "Lt. Col."
            {
                if (prefix.getLength() > 0)
                    prefix += space;
                prefix += word.proper();
                word.clear();
                if (!EOS)
                {
                    word = getWord(true);
                    potentialPrefix = word.isPrefix();
                    totalWordsRemaining--;
                    if (word.getLength() == 0)
                        notFinished = false;
                }
                else
                    notFinished = false;
            }

            if (prefix.getLength() > 0)
            {
                prefix.removeEnding(".");
                globals->globalDr->setPrefix(prefix);
                if (gender == genderUnknown)
                {
                    if (prefix.isMaleTitle(lang))
                    {
                        globals->globalDr->setGender(Male);
                        gender = Male;
                    }
                    else
                        if (prefix.isFemaleTitle(lang))
                        {
                            globals->globalDr->setGender(Female);
                            gender = Female;
                        }
                }
                prefix.clear();
                handled = true;
            }

            // Deal with suffixes
            // If suffix found, new word read in, should status cannot change to "handled"
            if (!handled)
                notFinished = true;
            while (notFinished && word.isSuffix())  // Additional complexity due to double suffix such as "FSA FCIA"
            {
                // Root out problem cases
                // If no last names yet and suffix is "i" or "v:
                // If no last names yet and suffix is "junior"

                bool isReallySuffix = true;
                totalSuffixesRemaining--;

                if ((numLastNames == 0) && isInitial)
                {
                    isReallySuffix = false;
                    notFinished = false;
                }

                if ((numLastNames == 0) && (inQuotes || inParentheses))
                {
                    isReallySuffix = false;
                    notFinished = false;
                }

                if (isReallySuffix)
                {
                    if (word.isSuffixKeep())
                    {
                        if (suffix.getLength() > 0)
                            suffix += space;
                        suffix += word;     // Retain original capitalization
                    }
                    word.clear();
                    if (!EOS && (totalWordsRemaining > 0))
                    {
                        word = getWord(true);
                        totalWordsRemaining--;
                        if (word.getLength() == 0)
                        {
                            notFinished = false;
                            handled = true;
                        }
                    }
                    else
                    {
                        notFinished = false;
                        handled = true;
                    }
                }
            }

            if (suffix.getLength() > 0)
            {
                suffix.removeEnding(".");
                globals->globalDr->setSuffix(suffix);
                suffix.clear();
                handled = true;
            }

            // Update gender if necessary and possible
            if (!handled && !isInitial)
            {
                if ((gender == genderUnknown) && firstNames.size() > 0)
                {
                    unisex = genderLookup(firstNames, globals);
                    if (unisex >= 0.95)
                    {
                        gender = Male;
                        if (unisex >= 0.99)
                            globals->globalDr->setGender(Male);
                    }
                    else
                    {
                        if (unisex <= 0.05)
                        {
                            gender = Female;
                            if (unisex <= 0.01)
                                globals->globalDr->setGender(Female);
                        }
                    }
                }
            }

            // From this point forward, use database searches to supplement logic
            if (!handled)
            {
                nameStats.clear();

                nameStats.name = word.lower().getString();
                nameStats.isInitial = isInitial;
                nameStats.inParentheses = inParentheses;
                nameStats.inQuotes = inQuotes;

                // Setup isSurname, isGivenname, and isNickname
                if (!isInitial)
                {
                    nameStatLookup(word.getString(), globals, nameStats, gender, true);
                    if (nameStats.inParentheses || nameStats.inQuotes)
                    {
                        nameStats.isNickName = nameStats.inQuotes || nickNameInList(word.lower(), firstNames, globals) || pureNickNameLookup(word.getString(), globals);

                        // Check for occasional inclusion of formal name in parentheses "Bob (Robert) Smith"
                        PQString errMsg;
                        int i = 0;
                        while (!nameStats.isNickName && nameStats.isLikelyGivenName && ( i < firstNames.size()))
                        {
                            nameStats.isNickName = word.isFormalVersionOf(firstNames.at(i), errMsg);
                            i++;
                        }
                    }
                }
            }

            // Words in parentheses can be a last name or nickname (first name)
            // Words in quotes will usually be a nickname (first name), but could be a last name
            // Any word handled by this section cannot be a middleName
            if (!handled && !nameStats.isInitial && (nameStats.inParentheses || nameStats.inQuotes))
            {
                // Use database search info
                if (nameStats.isGivenName && !nameStats.isSurname)
                {
                    if (!nameStats.isNickName && (numMiddleNames == 0) && (numLastNames == 0) && (totalWordsRemaining == 1))
                        numMiddleNames++;
                    else
                    {
                        if (names.size() > 1)
                            insertInstead = true;   // Puts an AKA name after first name but before middleName or lastName
                        numFirstNames++;
                        firstNames.append(word.lower().getString());
                        firstNameLoaded = true;
                    }
                }

                if (!nameStats.isGivenName && nameStats.isSurname)
                {
                    numLastNames++;
                    if (!(inParentheses || inQuotes))
                        numFinalLastNames++;
                }

                if ((!nameStats.isSurname && !nameStats.isGivenName) || (nameStats.isSurname && nameStats.isGivenName))
                {
                    if (nameStats.isGivenName && nameStats.isNickName)
                    {
                        numFirstNames++;
                        firstNames.append(word.lower().getString());
                        if (firstNameLoaded)
                            insertInstead = true;
                        else
                            firstNameLoaded = true;
                    }
                    else
                    {
                        if (nameStats.frequencyTotal == -1)
                            nameStats.frequencyTotal = countFrequency(globals->cleanedUpUC, word.getString(), Qt::CaseInsensitive);
                        if (nameStats.frequencyTotal >= 3)
                        {
                            // If the name appears this often, it is being used in the unstructured content as part of
                            // sentences. Make decision about how many times it is the first word in the sentence.
                            if (nameStats.frequencyFirst == -1)
                                nameStats.frequencyFirst = countFrequencyFirst(globals->cleanedUpUC, word.getString(), Qt::CaseInsensitive);
                            if (nameStats.frequencyFirst == 0)
                            {
                                numLastNames++;
                                if (!(inParentheses || inQuotes))
                                    numFinalLastNames++;
                            }
                            else
                            {
                                if (names.size() > 1)
                                    insertInstead = true;   // Puts an AKA name after first name but before middleName or lastName
                                numFirstNames++;
                                firstNames.append(word.lower().getString());
                                firstNameLoaded = true;
                            }
                        }
                        else
                        {
                            if (nameStats.isLikelyGivenName)
                            {
                                if (nameStats.frequencyFirst == -1)
                                    nameStats.frequencyFirst = countFrequencyFirst(globals->cleanedUpUC, word.getString(), Qt::CaseInsensitive);
                                if (nameStats.frequencyFirst == 0)
                                {
                                    numMiddleNames++;
                                    firstNames.append(word.lower().getString());
                                }
                                else
                                {
                                    if (names.size() > 1)
                                        insertInstead = true;
                                    numFirstNames++;
                                    firstNames.append(word.lower().getString());
                                    firstNameLoaded = true;
                                }
                            }
                            else
                            {
                                if (nameStats.inParentheses && (gender == Female))
                                {
                                    numLastNames++;
                                    if (!(inParentheses || inQuotes))
                                        numFinalLastNames++;
                                }
                                else
                                {
                                    if (names.size() > 1)
                                        insertInstead = true;
                                    numFirstNames++;
                                    firstNames.append(word.lower().getString());
                                    firstNameLoaded = true;
                                }
                            }
                        }
                    }
                }

                handled = true;
            }

            // Load first, middle and family names into QList
            if (word.getLength() > 0)
            {
                if (!handled && ((names.size() == 0) || !firstNameLoaded))
                {
                    numFirstNames++;
                    firstNames.append(word.lower().getString());
                    firstNameLoaded = true;
                    handled = true;
                }

                if (!handled && (numFinalLastNames == 0) && (isEOS() || ((totalWordsRemaining - totalBookEndsRemaining - totalSuffixesRemaining) == 0)))
                {
                    numLastNames++;
                    if (!(inParentheses || inQuotes))
                        numFinalLastNames++;
                    handled = true;
                }

                // If still not handled and already a firstName, then we are down to a middleName or LastName
                if (!handled && !isInitial)
                {
                    // Check for a poorly structured nickname first (i.e., something written as "Joshua Josh Christopher Smith")
                    if (nameStats.isNickName)
                    {
                        numFirstNames++;
                        firstNames.append(word.lower().getString());
                        handled = true;
                    }

                    // Use database search info
                    // Only add a second last name for females, other a male middleName
                    if (!handled && !nameStats.isGivenName && nameStats.isSurname && ((gender == Female) || ((gender == genderUnknown) && (unisex <= 0.5))))
                    {
                        numLastNames++;
                        if (!(inParentheses || inQuotes))
                            numFinalLastNames++;
                        handled = true;
                    }

                    if (!handled && ((nameStats.isGivenName && nameStats.isSurname) || (!nameStats.isGivenName && !nameStats.isSurname)))
                    {
                        if (nameStats.frequencyTotal == -1)
                            nameStats.frequencyTotal = countFrequency(globals->cleanedUpUC, word.getString(), Qt::CaseSensitive);
                        if (nameStats.frequencyTotal >= 3)
                        {
                            // If the name appears this often, it is being used in the unstructured content as part of
                            // sentences. Make decision about how many times it is the first word in the sentence.
                            if (nameStats.frequencyFirst == -1)
                                nameStats.frequencyFirst = countFrequencyFirst(globals->cleanedUpUC, word.getString(), Qt::CaseSensitive);
                            if (nameStats.frequencyFirst > 0)
                            {
                                if (numFirstNames == 0)
                                {
                                    numFirstNames++;
                                    firstNames.append(word.lower().getString());
                                    firstNameLoaded = true;
                                }
                                else
                                {
                                    if ((totalWordsRemaining - totalSuffixesRemaining + 1) > numLastNames)
                                        globals->globalDr->setMiddleNameUsedAsFirstName(word);
                                    else
                                    {
                                        if ((nameStats.frequencyFirst < 2) && (!nameStats.isGivenName && !nameStats.isSurname) & ((totalWordsRemaining - totalSuffixesRemaining) == 0) &&
                                                ((gender == Female) || ((gender == Male) && (numLastNames == 0))))
                                        {
                                            numLastNames++;
                                            if (!(inParentheses || inQuotes))
                                                numFinalLastNames++;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if ((gender == Female) && !word.isFoundIn(problematicFemaleMiddleNames) && (nameStats.surnameCount > 0))
                                {
                                    // There are approximately 1.25 first names (combined) for every last name                                    
                                    double ratio = nameStats.femaleCount / (nameStats.surnameCount * 1.25 * 0.5);
                                    if ((numMiddleNames >= 1) && (ratio < 1))
                                    {
                                        numLastNames++;
                                        if (!(inParentheses || inQuotes))
                                            numFinalLastNames++;
                                    }
                                }
                                else
                                {
                                    // Deal with any brand new names never recorded
                                    if ((nameStats.maleCount + nameStats.femaleCount + nameStats.surnameCount) == 0)
                                    {
                                        if (((nameStats.frequencyTotal >= 3) && (nameStats.frequencyFirst == 0)) ||
                                                ((gender == Female) && (numMiddleNames > 0) && (numLastNames == 0) && ((totalWordsRemaining - totalSuffixesRemaining) <= 1)))
                                        {
                                            numLastNames++;
                                            if (!(inParentheses || inQuotes))
                                                numFinalLastNames++;
                                        }
                                    }
                                }
                            }

                            handled = true;
                        }
                        else
                        {
                            // Default to an unknown lastname if another lastname or middlename already recorded
                            if ((!nameStats.isGivenName && (gender != Male) && ((numLastNames > 0) || (numMiddleNames > 0))) ||
                                (nameStats.isSurname && (numLastNames > 0)))
                            {
                                numLastNames++;
                                if (!(inParentheses || inQuotes))
                                    numFinalLastNames++;
                                handled = true;
                            }
                        }
                        // else defaults to middleName
                    }

                    // Everything else effectively defaults to middlename
                }

                if (!handled)
                {
                    numMiddleNames++;
                    if (!isInitial)
                        firstNames.append(word.lower().getString());
                }

                if (!skipWord)
                {
                    word.cleanUpEnds();
                    if (!insertInstead)
                    {
                        names.append(word.proper());
                        nameStatsList.append(nameStats);
                    }
                    else
                    {
                        names.insert(1, word.proper());
                        nameStatsList.insert(1, nameStats);
                    }
                }
            }
        }
    }  // end initial processing

    QListIterator<OQString> iter(names);
    unsigned int totalNumNames = static_cast<unsigned int>(names.size());
    // Assume something is wrong if too many words
    if (totalNumNames > 7)
    {
        OQString errMsg;
        errMsg << "Encountered too many names reading in: ";
        iter.toFront();
        while (iter.hasNext())
        {
            word = iter.next();
            errMsg << word << " ";
            globals->logMsg(ErrorRecord, errMsg);
            globals->globalDr->wi.nameFlagGeneral = 1;
        }
        return;
    }

    // A minimum of two words are required
    if (totalNumNames == 0)
    {
        OQString errMsg;
        errMsg << "Encountered insufficient names reading: " << globals->globalDr->getURL();
        //globals->logMsg(ErrorRecord, errMsg);
        globals->globalDr->wi.nameFlagGeneral = 1;
        return;
    }
    else
    {
        if (totalNumNames == 1)
        {
            if (nameStatsList.at(0).isLikelyGivenName)
            {
                numFirstNames = 1;
                numMiddleNames = 0;
                numLastNames = 0;
            }
            else
            {
                if (nameStatsList.at(0).isLikelySurname)
                {
                    numFirstNames = 0;
                    numMiddleNames = 0;
                    numLastNames = 1;
                }
                else
                    return;
            }
        }
        else
        {
            // Go through the name
            if (numLastNames < 1)
                numLastNames = 1;
            if (numLastNames > (totalNumNames - numFirstNames))
                numLastNames = (totalNumNames - numFirstNames);
            numMiddleNames = totalNumNames - (numFirstNames  + numLastNames);
        }
    }

    i = 0;
    iter.toFront();
    while (iter.hasNext())
	{
        word = iter.next();
        i++;
        if (i <= numFirstNames)
            nameInfo.type = ntFirst;
		else
		{
            if ((i - numFirstNames) <= numMiddleNames)
                nameInfo.type = ntMiddle;
            else
                nameInfo.type = ntLast;
		}
        nameInfo.name = word;
        globals->globalDr->setAlternates(nameInfo);

        if ((nameInfo.type == ntLast) && (word.isFoundIn(problematicFemaleMiddleNames)))
        {
                if (!iter.hasNext() || (globals->globalDr->getGender() == Male))
                    globals->globalDr->wi.nameWarningException = true;
        }
	}
}

void unstructuredContent::validateNameOrder(bool initialCheck)
{
    // Coded to look at names in the first and last positions only
    // Proper expected order is firstName secondName

    beg();
    bool firstIsSurname, firstIsGivenName, secondIsSurname, secondIsGivenName;
    double firstRatio, secondRatio;
    QString firstWord, secondWord, prefix;
    OQString temp;
    NAMESTATS nameStats1, nameStats2;
    bool valid;
    bool reversed = false;

    temp = getWord();
    while (temp.isCompoundName())
    {
        firstWord += temp.getString();
        firstWord += QString(" ");
        temp = getWord();
    }
    if (!temp.isPrefix())
        firstWord += temp.getString();
    else
    {
        while (temp.isPrefix())
        {
            prefix += temp.getString();
            prefix += QString(" ");
            temp = getWord();
        }
        firstWord += temp.getString();
    }

    while (!EOS)
    {
        secondWord.clear();
        temp = getWord();
        while (temp.isCompoundName())
        {
            secondWord += temp.getString();
            secondWord += QString(" ");
            temp = getWord();
        }
        secondWord += temp.getString();
    }

    nameStatLookup(firstWord, globals, nameStats1);
    nameStatLookup(secondWord, globals, nameStats2);

    firstIsGivenName = nameStats1.isGivenName;
    firstIsSurname = nameStats1.isSurname;
    secondIsGivenName = nameStats2.isGivenName;
    secondIsSurname = nameStats2.isSurname;

    valid = (firstIsGivenName && !firstIsSurname && secondIsSurname && !secondIsGivenName) || (prefix.length() > 0);

    if (!valid)
        reversed = !firstIsGivenName && firstIsSurname && !secondIsSurname && secondIsGivenName;

    // Next most reliable approach is to look for first name in the unstructured text
    if (!valid && !reversed)
    {
        OQString OQpotentialName, OQword;
        PQString warningMessage;
        QString potentialName = globals->globalDr->getPotentialFirstName();

        OQpotentialName = potentialName;
        OQword = firstWord;
        if ((potentialName.compare(firstWord, Qt::CaseInsensitive) == 0) || OQpotentialName.isInformalVersionOf(firstWord, warningMessage) || OQword.isInformalVersionOf(potentialName, warningMessage))
            valid = true;
        else
        {
            OQword = secondWord;
            if ((potentialName.compare(secondWord, Qt::CaseInsensitive) == 0) || OQpotentialName.isInformalVersionOf(secondWord, warningMessage) || OQword.isInformalVersionOf(potentialName, warningMessage))
                reversed = true;
        }
    }

    // If still not determined, start taking educated guesses
    if (!valid && !reversed)
    {
        // Err slightly to data being correct
        // There are approximately 1.25 first names for every last name
        if ((nameStats1.maleCount + nameStats1.femaleCount) > 0)
            firstRatio = nameStats1.surnameCount * 1.25 / (nameStats1.maleCount + nameStats1.femaleCount);
        else
            firstRatio = nameStats1.surnameCount * 1.25 / 0.5;

        if ((nameStats2.maleCount + nameStats2.femaleCount) > 0)
            secondRatio = nameStats2.surnameCount * 1.25 / (nameStats2.maleCount + nameStats2.femaleCount);
        else
            secondRatio = nameStats2.surnameCount * 1.25 / 0.5;


        // Case A - Brand new secondWord
        if (!(secondIsGivenName || secondIsSurname))
        {
            valid = (firstRatio <= 12.5);
            reversed = (firstRatio > 12.5);
        }

        // Case B - Brand new firstWord
        if (!valid && !reversed && !(firstIsGivenName || firstIsSurname))
        {
            valid =  (secondRatio >= 0.08);
            reversed = (secondRatio < 0.08);
        }

        // Case C - Both names can be either first name or last name
        if (!valid && !reversed)
            reversed = ((firstRatio > 2) && (secondRatio < 0.5)) ||
                       ((((nameStats1.maleCount + nameStats1.femaleCount) == 0) && ((nameStats2.maleCount + nameStats2.femaleCount) > 0) && ((nameStats1.surnameCount + nameStats2.maleCount + nameStats2.femaleCount) >= 3))) ||
                       (((nameStats2.surnameCount == 0) && (nameStats1.surnameCount > 0) && ((nameStats1.surnameCount + nameStats2.maleCount + nameStats2.femaleCount) >= 3)));

        // If the wording was reversed through this group of analysis, issue warning if same logic would reverse it back
        if (reversed && initialCheck)
        {
            unstructuredContent checkLogicA, checkLogicB;
            checkLogicA = secondWord + QString(" ") + firstWord;
            checkLogicB = checkLogicA;
            checkLogicA.validateNameOrder(false);
            if (checkLogicA != checkLogicB)
            {
                reversed = false;
                PQString errMsg;
                errMsg << "Check for potential reversing of names for: " << globals->globalDr->getURL();
                //globals->logMsg(ErrorRecord, errMsg);
                globals->globalDr->wi.nameReversalFlag = 1;
            }
        }
    }

    if (reversed)
    {
        // Too many false positives - changed to simply a flag
        //*this = prefix + secondWord + QString(" ") + firstWord;
        globals->globalDr->wi.nameReversalFlag += 1;
    }
}

bool unstructuredContent::validateNameOrder(QList<NAMESTATS> &nameStatsList, bool initialCheck)
{
    // Coded to look at names in the first and last positions only
    // Proper expected order is firstName secondName

    if (nameStatsList.size() != 2)
        return false;

    beg();
    bool firstIsSurname, firstIsGivenName, secondIsSurname, secondIsGivenName;
    double firstRatio, secondRatio;
    QString firstWord, secondWord, prefix;
    OQString temp;
    bool valid;
    bool reversed = false;

    firstWord = nameStatsList[0].name;
    secondWord = nameStatsList[1].name;

    firstIsGivenName  = ((nameStatsList[0].maleCount + nameStatsList[0].femaleCount) > 0);
    firstIsSurname    = (nameStatsList[0].surnameCount > 0);
    secondIsGivenName = ((nameStatsList[1].maleCount + nameStatsList[1].femaleCount) > 0);
    secondIsSurname   = (nameStatsList[1].surnameCount > 0);

    valid = (firstIsGivenName && !firstIsSurname && secondIsSurname && !secondIsGivenName);

    if (!valid)
        reversed = !firstIsGivenName && firstIsSurname && !secondIsSurname && secondIsGivenName;

    // Next most reliable approach is to look for first name in the unstructured text
    if (!valid && !reversed)
    {
        OQString OQpotentialName, OQword;
        PQString warningMessage;
        QString potentialName = globals->globalDr->getPotentialFirstName();

        OQpotentialName = potentialName;
        OQword = firstWord;
        if ((potentialName.compare(firstWord, Qt::CaseInsensitive) == 0) || OQpotentialName.isInformalVersionOf(firstWord, warningMessage) || OQword.isInformalVersionOf(potentialName, warningMessage))
            valid = true;
        else
        {
            OQword = secondWord;
            if ((potentialName.compare(secondWord, Qt::CaseInsensitive) == 0) || OQpotentialName.isInformalVersionOf(secondWord, warningMessage) || OQword.isInformalVersionOf(potentialName, warningMessage))
                reversed = true;
        }
    }

    // If still not determined, start taking educated guesses
    if (!valid && !reversed)
    {
        // Err slightly to data being correct
        // There are approximately 1.25 first names for every last name
        if ((nameStatsList[0].maleCount + nameStatsList[0].femaleCount) > 0)
            firstRatio = nameStatsList[0].surnameCount * 1.25 / (nameStatsList[0].maleCount + nameStatsList[0].femaleCount);
        else
            firstRatio = nameStatsList[0].surnameCount * 1.25 / 0.5;

        if ((nameStatsList[1].maleCount + nameStatsList[1].femaleCount) > 0)
            secondRatio = nameStatsList[1].surnameCount * 1.25 / (nameStatsList[1].maleCount + nameStatsList[1].femaleCount);
        else
            secondRatio = nameStatsList[1].surnameCount * 1.25 / 0.5;


        // Case A - Brand new secondWord
        if (!(secondIsGivenName || secondIsSurname))
        {
            valid = (firstRatio <= 12.5);
            reversed = (firstRatio > 12.5);
        }

        // Case B - Brand new firstWord
        if (!valid && !reversed && !(firstIsGivenName || firstIsSurname))
        {
            valid =  (secondRatio >= 0.08);
            reversed = (secondRatio < 0.08);
        }

        // Case C - Both names can be either first name or last name
        if (!valid && !reversed)
            reversed = ((firstRatio > 2) && (secondRatio < 0.5)) ||
                       (((nameStatsList[0].maleCount + nameStatsList[0].femaleCount) == 0) && ((nameStatsList[1].maleCount + nameStatsList[1].femaleCount) > 0) && ((nameStatsList[0].surnameCount + nameStatsList[1].maleCount + nameStatsList[1].femaleCount) >= 3)) ||
                       ((nameStatsList[1].surnameCount == 0) && (nameStatsList[0].surnameCount > 0) && ((nameStatsList[0].surnameCount + nameStatsList[1].maleCount + nameStatsList[1].femaleCount) >= 3));

        // If the wording was reversed through this group of analysis, issue warning if same logic would reverse it back
        if (reversed && initialCheck)
        {
            QList<NAMESTATS> reversedName;
            reversedName.append(nameStatsList[1]);
            reversedName.append(nameStatsList[0]);

            if (validateNameOrder(reversedName, false))
            {
                PQString errMsg;
                errMsg << "Check for potential reversing of names for: " << globals->globalDr->getURL();
                //globals->logMsg(ErrorRecord, errMsg);
                globals->globalDr->wi.nameReversalFlag += 1;
            }
        }
    }

    if (reversed)
    {
        // Too many false positives - changed to simply a flag
        //*this = secondWord + QString(" ") + firstWord;
        globals->globalDr->wi.nameReversalFlag += 1;
    }

    return reversed;
}

void unstructuredContent::copyKeyVariablesFrom(const unstructuredContent &uc)
{
    contentGender = uc.contentGender;
    contentLanguage = uc.contentLanguage;
    numEnglishDates = uc.numEnglishDates;
	numFrenchDates = uc.numFrenchDates;
	numSpanishDates = uc.numSpanishDates;
}

void unstructuredContent::setItsString(const OQString &rhs)
{
    clear();    // Reset content

    itsString = rhs.getString();
    position = 0;
    if (itsString.length() > 0)
        EOS = false;
    else
        EOS = true;
}

unstructuredContent& unstructuredContent::operator= (const OQString &rhs)
{
    clear();    // Reset content

    itsString = rhs.getString();
	position = 0;
    if (itsString.length() > 0)
		EOS = false;
	else
		EOS = true;

	return *this;
}

unstructuredContent& unstructuredContent::operator= (const unstructuredContent &orig)
{
    clear();

    itsString = orig.getString();
    copyKeyVariablesFrom(orig);

    position = 0;
    if (itsString.length() > 0)
        EOS = false;
    else
        EOS = true;

    dateToday = orig.dateToday;
    globals = orig.globals;

    sentenceStartPositions = orig.sentenceStartPositions;
    lastSentenceStartPosition = orig.lastSentenceStartPosition;
    realSentenceEncountered = orig.realSentenceEncountered;

    return *this;
}


DATES unstructuredContent::readDates(LANGUAGE lang, bool limitWords)
{
    DATES resultsA, resultsN;
    QList<QDate> dateList;
    OQString cleanString;
	unsigned int numValidDates = 0;
	unsigned int maxDates = 2;

    bool serviceDate = false;
    bool checkNumeric = false;

	switch (lang)
	{
	case french:
        numValidDates = pullOutFrenchDates(dateList, maxDates, cleanString, limitWords, serviceDate);
		break;

	case spanish:
        numValidDates = pullOutSpanishDates(dateList, maxDates, cleanString, limitWords, serviceDate);
		break;

	case english:
	default:
        numValidDates = pullOutEnglishDates(dateList, maxDates, cleanString, limitWords, serviceDate);
		break;
	}


	// Process dates
	if (numValidDates == 2)
	{
        if (dateList[0] < dateList[1])
		{
            resultsA.potentialDOB = dateList[0];
            resultsA.potentialDOD = dateList[1];
		}
		else
		{
            resultsA.potentialDOB = dateList[0];
            resultsA.potentialDOD = dateList[1];
		}
	}

	if (numValidDates == 1) 
	{
        unsigned int YOB = globals->globalDr->getYOB();
        unsigned int YOD = globals->globalDr->getYOD();

        if ((YOB > 0) && (YOB != YOD) && (YOB == static_cast<unsigned int>(dateList[0].year())))
            resultsA.potentialDOB = dateList[0];
		else
		{
            if ((YOD > 0) && (YOB != YOD) && (YOD == static_cast<unsigned int>(dateList[0].year())))
                resultsA.potentialDOD = dateList[0];
			else
			{
                // Make an educated guess using first possible DOD
                QSqlQuery query;
                QSqlError error;

                QDate fhFirstObit, thresholdDate;
                PQString errorMessage;
                bool success;

                unsigned int providerID = globals->globalDr->getProvider();
                unsigned int providerKey = globals->globalDr->getProviderKey();

                success = query.prepare("SELECT fhFirstObit FROM funeralhomedata WHERE providerID = :fhProviderID AND providerKey = :fhProviderKey");
                query.bindValue(":fhProviderID", QVariant(providerID));
                query.bindValue(":fhProviderKey", QVariant(providerKey));
                success = query.exec();

                if (!success || (query.size() != 1))
                {
                    error = query.lastError();

                    errorMessage << QString("SQL problem retrieving first obit date for:");
                    errorMessage << providerID << PQString(" ") << providerKey;
                    globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
                }
                else
                {
                    while (query.next())
                    {
                        fhFirstObit = query.value(0).toDate();
                        if (!fhFirstObit.isValid())
                            fhFirstObit = globals->today;
                    }
                }

                QDate EighteenYearOldDOB(globals->today.year()-18, globals->today.month(), globals->today.day());

                if (fhFirstObit < EighteenYearOldDOB)
                    thresholdDate = EighteenYearOldDOB;
                else
                    thresholdDate = fhFirstObit;

                if (dateList[0] > thresholdDate)
                    resultsA.potentialDOD = dateList[0];
				else
                    resultsA.potentialDOB = dateList[0];
			}
		}
	}

	// Move on to numeric dates if no valid alpha dates were processed
	if (!resultsA.potentialDOB.isValid() && !resultsA.potentialDOD.isValid())
		checkNumeric = true;

	if (checkNumeric)
	{
		resultsN = readNumericDates();
		return resultsN;
	}
	else
		return resultsA;
}

bool unstructuredContent::lookupUncapitalizedNames(OQString name)
{
	bool matched = false;

    QListIterator<QString> iter(this->uncapitalizedNames);

    while (!matched && iter.hasNext())
    {
        if (name == iter.next())
            matched = true;
    }

    return matched;
}

DATES unstructuredContent::readNumericDates(DATEORDER dateOrder)
{
    // Only returns a result if two dates are encountered

    DATES result;
	MAKEDATE tempDate1, tempDate2;

	unsigned int numValidDates, numPotentialDates, numFinalDates;
    OQString word, singleChar;
	bool stillOK;

	numValidDates = 0;
	numPotentialDates = 0;
	numFinalDates = 0;

    tempDate1.setToday(globals->today);
    tempDate2.setToday(globals->today);

	beg();		// go to beginning of stream

	while (!EOS && (numPotentialDates < 2))
	{
        word = getWord(true, SPACE, false);		// pull next word from content, but don't break on slash
        singleChar = word[0];

		// Quick validation checks to eliminate most words
		if ((word.getLength() >= 5) && (word.getLength() <= 10) && singleChar.isNumeric())
			stillOK = true;
		else
			stillOK = false;

		// Complete full validation and assessment
		if (stillOK)
		{
			switch (numPotentialDates)
			{
			case 0:
                parseDateFromAndTo(word, tempDate1, dateOrder);
				if (tempDate1.isValid)
				{
					numPotentialDates++;
					numValidDates++;
					if (tempDate1.isFinal)
						numFinalDates++;
				}
				if (tempDate1.analyzeFurther)
					numPotentialDates++;
				break;

			case 1:
                parseDateFromAndTo(word, tempDate2, dateOrder);
				if (tempDate2.isValid)
				{
					numPotentialDates++;
					numValidDates++;
					if (tempDate2.isFinal)
						numFinalDates++;
				}
				if (tempDate2.analyzeFurther)
					numPotentialDates++;
			}
		}

	} // end while

	// Try to use knowledge about one date to finalize the other
	if ((numPotentialDates == 2) && (numFinalDates == 1))
	{
		if (tempDate1.isFinal)
		{
			tempDate2.revisitAssuming(tempDate1.format);
			if (tempDate2.isFinal)
				numFinalDates++;
		}
		else
		{
			tempDate1.revisitAssuming(tempDate2.format);
			if (tempDate1.isFinal)
				numFinalDates++;
		}
	}

	// Wrap up
	if ((numFinalDates == 2) && (tempDate1.format == tempDate2.format))
	{
		if (tempDate1.finalDate > tempDate2.finalDate)
		{
			result.potentialDOB = tempDate2.finalDate;
			result.potentialDOD = tempDate1.finalDate;
		}
		else
		{
			result.potentialDOB = tempDate1.finalDate;
			result.potentialDOD = tempDate2.finalDate;
		}

	}

	return result;
}

QDate unstructuredContent::readDateField(DATEORDER dateOrder)
{
    // Returns the first found date

    QDate result;
    if (itsString.size() == 0)
        return result;

	MAKEDATE potentialDate;
	bool stillOK;
    OQString singleChar, cleanString;
	unsigned int numValidDates;

    beg();
    OQString word = getWord(false, SPACE);

	/***************************************/
	/*      Try numeric first              */
	/*  33% success rate at best (13-31)   */
	/***************************************/
	singleChar = word[0];

	// Quick validation checks to eliminate most words
	if ((word.getLength() >= 5) && (word.getLength() <= 10) && singleChar.isNumeric())
		stillOK = true;
	else
		stillOK = false;

	if (stillOK)
	{
        parseDateFromAndTo(word, potentialDate, dateOrder);
		if (potentialDate.isFinal)
			result = potentialDate.finalDate;
	}
	else    // try long form
	{
        QList<QDate> dateList;
		beg();
        numValidDates = pullOutDates(getLanguage(), dateList, 1, cleanString, false);
        if ((numValidDates) && (dateList.size() > 0))
            result = dateList[0];
	}

    return result;
}

DATES unstructuredContent::readYears(bool reliable)
{
    DATES resultDates;

    unsigned int numNumbersRead;
    int Year1, Year2;
    OQString word, firstChar, lastChar;
    QString space(" ");

    bool valid, inParentheses, inQuotes, skipWord;
	bool overLimit = false;
	unsigned int wordLimit = 3;
	unsigned int numWordsRead = 0;

	numNumbersRead = 0;
    Year1 = Year2 = 0;
	beg();		// go to beginning of stream

	while (!EOS && (numNumbersRead < 2) && !overLimit)
	{
		word = getWord(true);		// pull next word from content
        word.removeEnding(PUNCTUATION);

        // Exclude names and written months
        if (word.getLength() > 0)
            skipWord = false;
        else
            skipWord = true;

        if (word.isAlpha())
        {
            word.removeBookEnds();
            word.removeLeadingNeeEtAl();
            if (globals->globalDr->isASavedName(word) || word.isWrittenMonth())
                skipWord = true;
        }

        if (!skipWord)
        {
            numWordsRead++;
            if (numWordsRead > wordLimit)
                overLimit = true;

            // Remove bookends if they exist and any leading or trailing spaces
            inParentheses = word.hasBookEnds(PARENTHESES);
            inQuotes = word.hasBookEnds(QUOTES);
            if (inParentheses || inQuotes)
            {
                word.removeBookEnds();
                word.removeLeading(space);
                word.removeEnding(space);
            }

            // Pick out the two dates if written as "YYYY - YYYY" or "YYYY-YYYY"
            // "YYYY - YYYY" can exist within parentheses
            // "YYYY-YYYY" would be read in as a single word
            if (word.getLength() > 8)
            {
                firstChar = word.left(4);
                lastChar = word.right(4);
                if (firstChar.isNumeric() && lastChar.isNumeric())
                {
                    Year1 = static_cast<int>(firstChar.asNumber());
                    Year2 = static_cast<int>(lastChar.asNumber());
                    numNumbersRead = 2;
                }
            }

            // Look for years on their own
            if (word.isNumeric() && (numNumbersRead < 2))
            {
                if (numNumbersRead == 0)
                    Year1 = static_cast<int>(word.asNumber());
                else
                    Year2 = static_cast<int>(word.asNumber());
                numNumbersRead++;
            }
        }
	}

    if (!reliable)
    {
        if((numNumbersRead == 2) && (Year1 == Year2))
        {
            numNumbersRead--;
            Year2 = 0;
        }
    }

	// Validate potential years
	if (numNumbersRead == 2)
	{
        valid = (Year2 > Year1) && (Year1 > 1890) && (Year2 <= globals->today.year());
		if (valid)
        {
            resultDates.potentialYOB = Year1;
            resultDates.potentialYOD = Year2;
            resultDates.fullyCredible = true;
		}
	}

    // Store a single year for potential use later as a last resort
    if ((numNumbersRead == 1) && (Year1 > 1890) && (Year1 <= globals->today.year()))
        globals->globalDr->setSingleYear(static_cast<unsigned int>(Year1));

    return resultDates;
}

DATES unstructuredContent::contentKeyWordsAndDates(QList<QString> &firstNameList, LANGUAGE lang, unsigned int maxSentences)
{
    DATES resultDates;
    unstructuredContent sentence;

    beg();

    if (lang == language_unknown)
        lang = contentLanguage;

    unsigned int lastHandled = 99;
    unsigned int i = 1;
    unsigned int j = 1;
    bool handled = false;
    bool realSentenceEncountered = true;

    while ((!resultDates.potentialDOB.isValid() || !resultDates.potentialDOD.isValid()) && !isEOS() && (i <= maxSentences))
    {
        sentence = getSentence(firstNameList, realSentenceEncountered, lang, i);
        if (sentence.isJustDates())
        {
            DATES tempDates;
            tempDates = sentence.sentencePullOutDates(lang, 1);
            if (tempDates.potentialDOB.isValid() && tempDates.potentialDOD.isValid())
            {
                resultDates = tempDates;
                lastHandled = j;
            }
        }
        else
        {
            if(!(sentence.hasBookEnds(PARENTHESES) || sentence.isJustNames()))
            {
                handled = sentence.sentenceKeyWordsAndDates(lang, resultDates);
                if (handled)
                    lastHandled = j;
                j++;
            }
        }
        i++;
    }

    // Limit credibility once information is coming from later sentences
    if (lastHandled >= 3)
        resultDates.fullyCredible = false;

    return resultDates;
}

bool unstructuredContent::sentenceKeyWordsAndDates(const LANGUAGE lang, DATES &resultDates)
{
	// This function reads through a sentence to pick combinations of keywords and dates such as "born" and "November 9, 1966"
	// Dates must be complete and valid
	
    if (itsString.size() == 0)
        return false;

    unsigned int numDates;
	unsigned int maxDates = 2;
    int firstDateLocation, cumPosition;

	bool limitWords = false;
    bool handled = false;
    bool matched, birthMatch, deathMatch, stillbornMatch, marriedFlag, altMatch, ambiguouslyEntered, keepGoing;
    bool skipDeath, skipBirth, skipStillBorn, contextError;
    QList<QDate> dateList;
    QDate minDate, maxDate;
    OQString word, cleanString;

    matched = false;
    birthMatch = false;
    deathMatch = false;
    stillbornMatch = false;
    marriedFlag = false;
    ambiguouslyEntered = false;
    contextError = false;
    keepGoing = true;

    // Earliest information is always deemed the most credible, unless both DOB and DOD are in a single sentence later on
    if (resultDates.potentialDOB.isValid())
        skipBirth = true;
    else
        skipBirth = false;

    if (resultDates.potentialDOD.isValid())
        skipDeath = true;
    else
        skipDeath = false;

    if (resultDates.potentialDOB.isValid() && resultDates.potentialDOD.isValid())
        skipStillBorn = true;
    else
        skipStillBorn = false;
    cumPosition = 0;

    numDates = pullOutDates(lang, dateList, maxDates, cleanString, limitWords);

	// Only process if one or two dates were read
	if ((numDates == 1) || (numDates == 2))
	{
        // Find location of first date in sentence
        firstDateLocation = this->firstDifferenceFrom(cleanString.getString());

        // look for key birth or death words in the sentence
        beg();

        while (keepGoing && !isEOS())
		{
			word = getWord(true).lower();
            word.removeEnding(":");     // Needed for front end sentences such as "Born: January 1, 1984" or "Date of death: January 2, 2020"
            cumPosition += word.getLength();

            // Adjust for problematic wording like "Gail Smith-age 65" or "Gail Smith -age 65"
            if (word.isHyphenated() || (word.left(1) == PQString("-")))
                word = word.postHyphen();

            if (!birthMatch)
            {
                birthMatch = word.isBirthWord(lang);
                if ((birthMatch) && (numDates == 1))
                    ambiguouslyEntered = (word.lower() == OQString("entered"));
            }
            if (!deathMatch)
            {
                deathMatch = word.isDeathWord(lang);
                if (deathMatch && birthMatch && (numDates == 1))
                {
                    if (cumPosition < firstDateLocation)
                        birthMatch = false;
                }
            }
            if (!stillbornMatch)
                stillbornMatch = (word.lower() == OQString("stillborn")) || (word.lower() == OQString(QString(QLatin1String("mort-né")))) || (word.lower() == OQString("mortinato"));
            if (!marriedFlag)
            {
                marriedFlag = (word.lower() == OQString("married")) || (word.lower() == OQString(QString(QLatin1String("marié")))) || (word.lower() == OQString("casado"));
                if (marriedFlag && birthMatch && (numDates == 1))
                {
                    if (cumPosition < firstDateLocation)
                        birthMatch = false;
                }

            }

            // Try to eliminate ambiguity
            if (ambiguouslyEntered)
            {
                if (word.lower() == OQString("world"))
                {
                    birthMatch = true;
                    deathMatch = false;
                    ambiguouslyEntered = false;
                }

                if ((word.lower() == OQString("heaven")) || (word.lower() == OQString("afterlife")) || (word.lower() == OQString("rest")))
                {
                    birthMatch = false;
                    deathMatch = true;
                    ambiguouslyEntered = false;
                }

                if (word.isDeathIndicator(lang))
                {
                    birthMatch = false;
                    ambiguouslyEntered = false;
                }
            }

            if (numDates == 1)
                matched = birthMatch || deathMatch || stillbornMatch;
            else
            {
                matched = birthMatch && deathMatch && !marriedFlag;
                altMatch = birthMatch && !deathMatch && marriedFlag;
            }

            if (ambiguouslyEntered || !matched || ((numDates == 1) && (cumPosition < firstDateLocation)))
                keepGoing = true;
            else
                keepGoing = false;

            cumPosition++;  // for space

        } // end of looping through entire sentence

        if ((numDates == 1) && birthMatch && !deathMatch)
        {
            if (!skipBirth)
            {
                resultDates.potentialDOB = dateList[0];
                minDate = globals->globalDr->getMinDOB();
                if (!minDate.isValid())
                    minDate.setDate(1890,1,1);
                maxDate = globals->globalDr->getMaxDOB();
                if (!maxDate.isValid())
                    maxDate = globals->today;
                if ((resultDates.potentialDOB >= minDate) && (resultDates.potentialDOB <= maxDate) && !marriedFlag)
                    resultDates.fullyCredible = true;
                else
                    resultDates.fullyCredible = false;
            }
            handled = true;
        }

        if ((numDates == 1) && !birthMatch && deathMatch)
        {
            if (!skipDeath)
            {
                resultDates.potentialDOD = dateList[0];
                minDate.setDate(1990,1,1);
                maxDate = globals->today;
                if ((resultDates.potentialDOD >= minDate) && (resultDates.potentialDOD <= maxDate) && !marriedFlag)
                    resultDates.fullyCredible = true;
                else
                    resultDates.fullyCredible = false;
            }
            handled = true;
        }

        if ((numDates == 1) && stillbornMatch)
        {
            if (!skipStillBorn)
            {
                resultDates.potentialDOB = dateList[0];
                resultDates.potentialDOD = dateList[0];
                if (!(birthMatch || deathMatch || marriedFlag))
                    resultDates.fullyCredible = true;
                else
                    resultDates.fullyCredible = false;
            }
            handled = true;
        }

        if ((numDates == 2) && birthMatch && deathMatch && !marriedFlag)
		{
            if (dateList[0] <= dateList[1])
            {
                resultDates.potentialDOB = dateList[0];
                resultDates.potentialDOD = dateList[1];
                if ((dateList[0] < dateList[1])  && !marriedFlag)
                    resultDates.fullyCredible = true;
                else
                    resultDates.fullyCredible = false;
            }
            else
            {
                resultDates.potentialDOB = dateList[1];
                resultDates.potentialDOD = dateList[0];
            }
            handled = true;
        }

        if ((numDates == 2) && birthMatch && !deathMatch && marriedFlag)
        {
            if (dateList[0] <= dateList[1])
            {
                resultDates.potentialDOB = dateList[0];
                if (dateList[0] < dateList[1])
                    resultDates.fullyCredible = true;
                else
                    resultDates.fullyCredible = false;
            }
            else
                resultDates.potentialDOB = dateList[1];

            handled = true;
        }

        if ((numDates == 2) && birthMatch && !deathMatch && !marriedFlag && globals->globalDr->getMinDOB().isValid())
        {
            minDate = globals->globalDr->getMinDOB();
            maxDate = globals->globalDr->getMaxDOB();

            if ((dateList[0] >= minDate) && (dateList[0] <= maxDate))
            {
                resultDates.potentialDOB = dateList[0];
                resultDates.fullyCredible = false;
            }
            else
            {
                if ((dateList[1] >= minDate) && (dateList[1] <= maxDate))
                {
                    resultDates.potentialDOB = dateList[1];
                    resultDates.fullyCredible = false;
                }
            }

            handled = true;
        }

        if ((numDates == 1) && !birthMatch && !deathMatch && (globals->globalDr->getYOB() > 0) && !globals->globalDr->getDOB().isValid() && !resultDates.potentialDOB.isValid())
        {
            if (globals->globalDr->getYOB() == static_cast<unsigned int>(dateList[0].year()))
            {
                resultDates.potentialDOB = dateList[0];
                resultDates.fullyCredible = false;
            }

            handled = true;
        }

        // Look for bonus age at death (eg. John died on January 9, 2019 at 88 years of age) for fully credible ageAtDeath
        if (birthMatch || deathMatch)
        {
            unstructuredContent tempContent;
            unsigned int numNumbers = cleanString.countWords(SPACE, NUMERICAL);
            if (numNumbers > 0)
            {
                if (numNumbers == 1)
                    tempContent = cleanString;
                else
                    tempContent = cleanString.right(cleanString.getLength() - static_cast<unsigned int>(firstDateLocation));

                bool updateDirectly = false;
                unsigned int bonusAgeAtDeath = tempContent.sentenceReadAgeAtDeath(updateDirectly);
                if (bonusAgeAtDeath != 999)
                {
                    int ageLocation = this->findPosition(QString::number(bonusAgeAtDeath));
                    bool ageAtDeathFullyCredible;
                    if ((ageLocation - firstDateLocation) > 75)
                        ageAtDeathFullyCredible = false;
                    else
                        ageAtDeathFullyCredible = true;
                    globals->globalDr->setAgeAtDeath(bonusAgeAtDeath, ageAtDeathFullyCredible);
                    globals->globalDr->setMinMaxDOB();
                }
            }
        }

		// All other combinations (more than two dates or mismatch between number of keyword matches and number of dates) are ignored 
		// as there is no way to validate the information
        if ((deathMatch || birthMatch) && !handled)
        {
            PQString errMsg;
            errMsg << "Unable to validate potential good information within \"" << getString() << "\"  source: " << globals->globalDr->getURL();
            globals->logMsg(DefdErrorRecord, errMsg);
            //globals->globalDr->wi.outstandingErrMsg = 1;
        }
	}

    return handled;
}

void unstructuredContent::sentenceReadBornYear(const LANGUAGE lang, DATES &dates)
{
    if (itsString.size() == 0)
        return;

    OQString word, cleanString;
    QString tempString;
    int yearOfBirth = 0;
    int changedContextIndex = 0;
    int dateIndex;
    bool wordMatched = false;
    bool nameFound = false;
    bool numFound = false;
    bool dateFound = false;
    bool bornWordFirst = false;
    bool contextChangedFlag = false;
    bool contextIssue = false;
    bool ambiguousEntered = false;
    bool allFound = nameFound && (numFound || dateFound) && wordMatched;
    QDate dateOfBirth;

    // First step is to check if a single valid date is read
    unsigned int numDates;
    unsigned int maxDates = 2;
    bool limitWords = false;
    bool keepGoing = true;
    QList<QDate> dateList;

    beg();
    dateIndex = static_cast<int>(getLength());
    numDates = pullOutDates(lang, dateList, maxDates, cleanString, limitWords);
    if (numDates == 1)
    {
        dateFound = true;
        dateOfBirth = dateList[0];
        yearOfBirth = dateOfBirth.year();

        dateIndex = firstDifferenceFrom(cleanString.getString());
    }
    if (numDates > 1)
        keepGoing = false;
    else
    {
        // Name match if first word is "He", "She", name or a nickname
        beg();
        word = peekAtWord(false).lower();
        word.removeEnding(PUNCTUATION);
        word.removePossessive();
        if (word.isBirthWord(lang))
        {
            bornWordFirst = true;
            nameFound = true;       // Implied
        }
        if (!bornWordFirst && (word.isPronoun() || globals->globalDr->isAFirstName(word) || globals->globalDr->isANickName(word)))
            nameFound = true;
    }

    beg();
    while ((!allFound || (position < dateIndex)) && keepGoing && !isEOS() && (nameFound || bornWordFirst))
    {
        // Look for "born" and a first name
        word = getWord(true).lower();
        if (word.isAlpha() && !wordMatched)
        {
            // Check if the word matches one of the predefined age words
            if (word.isBirthWord(lang))
            {
                wordMatched = true;
                if (word.lower() == PQString("entered"))
                    ambiguousEntered = true;
            }
        }

        if (word.isAlpha() && !nameFound)
        {
            // Check if the word if a first name
            if (globals->globalDr->isAFirstName(word) || globals->globalDr->isANickName(word))
                nameFound = true;
        }

        if (word.isAlpha() && !contextChangedFlag)
        {
            // Check if the context of the sentence changed before the date was encountered
            if (word.isFoundIn(changedContextIndicators, 1))
            {
                contextChangedFlag = true;
                tempString = getString();
                changedContextIndex = tempString.indexOf(word.getString(), Qt::CaseInsensitive);
            }

            if (!contextChangedFlag && word.isDeathWord(lang))
            {
                contextChangedFlag = true;
                tempString = getString();
                changedContextIndex = tempString.indexOf(word.getString(), Qt::CaseInsensitive);
            }
        }

        if (word.isAlpha() && ambiguousEntered)
        {
            // Try to eliminate ambiguity either way
            if (word.lower() == PQString("world"))
                ambiguousEntered = false;
            if (word.isDeathIndicator(lang))
            {
                wordMatched = false;
                ambiguousEntered = false;
            }
        }

        // Look for year
        if (word.isNumeric() && !numFound)
        {
            // Check if the number is a valid birth year
            int num = static_cast<int>(word.asNumber());
            if ((num > 1875) && (num < globals->today.year()))
            {
                 numFound = true;
                 yearOfBirth = num;
            }
        }

        if (ambiguousEntered && !isEOS())
            allFound = false;
        else
            allFound = nameFound && (numFound || dateFound) && wordMatched;
    }

    // Deal with any changes in context mid-sentence
    if (contextChangedFlag)
    {
        if (changedContextIndex < dateIndex)
            contextIssue = true;
    }

    // Process data if valid information was read in
    if (allFound && !contextIssue)
    {
        if (dateFound)
            dates.potentialDOB = dateOfBirth;
        else
            dates.potentialYOB = yearOfBirth;

        dates.fullyCredible = false;
    }

    //if ((!allFound || contextIssue) && (dateFound || numFound) && wordMatched)
    if ((!allFound || contextIssue) && dateFound && wordMatched)
    {
        PQString errMsg;
        errMsg << "Potential DOB or YOB information within \"" << getString() << "\"  source: " << globals->globalDr->getURL();
        globals->logMsg(ErrorRecord, errMsg);
        globals->globalDr->wi.outstandingErrMsg = 1;
    }
}

DATES unstructuredContent::contentReadBornYear(LANGUAGE lang)
{
    DATES resultDates;
    unstructuredContent sentence;
    QList<QString> firstNamesList = globals->globalDr->getFirstNameList();
    bool realSentenceEncountered = true;

    beg();

    if (lang == language_unknown)
        lang = contentLanguage;

    while ((resultDates.potentialDOB.year() == 0) && (resultDates.potentialYOB == 0) && !isEOS())
    {
        sentence = getSentence(firstNamesList, realSentenceEncountered, lang);
        sentence.sentenceReadBornYear(lang, resultDates);
    }

    return resultDates;
}

bool unstructuredContent::genderRelationalReferences()
{
    // This function searches for "son of", "daughter of", "mother of", "father of", "husband of", "wife of" in order to determine gender
    // Special checking to differentiate between "wife of Paul" and "his wife of 40 years"

    LANGUAGE language = globals->globalDr->getLanguage();
    QString relationshipWords;
    bool reverse;

    // No match will be found if language unknown as none of the search words exist
    if (language == language_unknown)
        return true;

    bool unmatched = (globals->globalDr->getGender() == genderUnknown);

    if (unmatched)
    {
        QString sentence = this->getString();
        QList<QString> maleWords, femaleWords;

        switch (language)
        {
        default:
        case english:
            maleWords = relationshipWordsEnglishM;
            femaleWords = relationshipWordsEnglishF;
            break;

        case french:
            maleWords = relationshipWordsFrenchM;
            femaleWords = relationshipWordsFrenchF;
            break;

        case spanish:
            maleWords = relationshipWordsSpanishM;
            femaleWords = relationshipWordsSpanishF;
            break;
        }

        QListIterator<QString> iterMale(maleWords);
        QListIterator<QString> iterFemale(femaleWords);

        iterMale.toFront();
        while (unmatched && iterMale.hasNext())
        {
            relationshipWords = iterMale.next();
            if (sentence.contains(relationshipWords, Qt::CaseInsensitive))
            {
                // Check for exception cases before making conclusion
                QString specificInstanceA, specificInstanceB;
                reverse = false;
                switch (language)
                {
                default:
                case english:
                    // Exception #1
                    specificInstanceA = QString("husband of ");
                    if (relationshipWords == specificInstanceA)
                    {
                        QString extract;
                        int end = sentence.indexOf(specificInstanceA, 0, Qt::CaseInsensitive);
                        int start = (end < 40) ? 0 : (end - 40);
                        extract = sentence.mid(start, end - start);
                        if (extract.contains(QString("her "), Qt::CaseInsensitive) || extract.contains(QString("mourn"), Qt::CaseInsensitive))
                            reverse = true;
                    }

                    // Exception #2
                    if (!reverse)
                    {
                        specificInstanceA = QString("father of ");
                        specificInstanceB = QString("father to ");
                        if ((relationshipWords == specificInstanceA) || (relationshipWords == specificInstanceA))
                        {
                            QString extract;
                            int start = sentence.indexOf(specificInstanceA.left(6), 0, Qt::CaseInsensitive) + 10;
                            int end = sentence.indexOf(QString(" "), start);
                            extract = sentence.mid(start, end - start);
                            if ((extract.contains(QString(" her"), Qt::CaseInsensitive)) || ((extract.size() == 3) && (extract == QString("her"))))
                                reverse = true;
                        }
                    }

                    break;

                case french:
                    specificInstanceA = QString("marie de ");
                    if (relationshipWords == specificInstanceA)
                    {
                        QString extract;
                        int end = sentence.indexOf(specificInstanceA, 0, Qt::CaseInsensitive);
                        int start = (end < 30) ? 0 : (end - 30);
                        extract = sentence.mid(start, end - start);
                        if (extract.contains(QString("son "), Qt::CaseInsensitive) || extract.contains(QString("xxxxx"), Qt::CaseInsensitive))
                            reverse = true;
                    }
                    break;

                case spanish:
                    break;
                }

                unmatched = false;
                if (reverse)
                    globals->globalDr->setGender(Female);
                else
                    globals->globalDr->setGender(Male);
            }
        }

        iterFemale.toFront();
        while (unmatched && iterFemale.hasNext())
        {
            relationshipWords = iterFemale.next();
            if (sentence.contains(relationshipWords, Qt::CaseInsensitive))
            {
                // Check for exception cases before making conclusion
                QString specificInstanceA, specificInstanceB;
                reverse = false;
                switch (language)
                {
                default:
                case english:
                    // Exception #1
                    specificInstanceA = QString("wife of ");
                    if (relationshipWords == specificInstanceA)
                    {
                        QString extract;
                        int end = sentence.indexOf(specificInstanceA, 0, Qt::CaseInsensitive);
                        int start = (end < 30) ? 0 : (end - 30);
                        extract = sentence.mid(start, end - start);
                        if (extract.contains(QString("his "), Qt::CaseInsensitive) || extract.contains(QString("mourn"), Qt::CaseInsensitive))
                            reverse = true;
                    }

                    // Exception #2
                    if (!reverse)
                    {
                        specificInstanceA = QString("mother of ");
                        specificInstanceB = QString("mother to ");
                        if ((relationshipWords == specificInstanceA) || (relationshipWords == specificInstanceA))
                        {
                            QString extract;
                            int start = sentence.indexOf(specificInstanceA.left(6), 0, Qt::CaseInsensitive) + 10;
                            int end = sentence.indexOf(QString(" "), start);
                            extract = sentence.mid(start, end - start);
                            if (extract.contains(QString("his"), Qt::CaseInsensitive))
                                reverse = true;
                        }
                    }

                    break;

                case french:
                    specificInstanceA = QString("femme de ");
                    if (relationshipWords == specificInstanceA)
                    {
                        QString extract;
                        int end = sentence.indexOf(specificInstanceA, 0, Qt::CaseInsensitive);
                        int start = (end < 30) ? 0 : (end - 30);
                        extract = sentence.mid(start, end - start);
                        if (extract.contains(QString("sa "), Qt::CaseInsensitive) || extract.contains(QString("xxxxx"), Qt::CaseInsensitive))
                            reverse = true;
                    }
                    break;

                case spanish:
                    break;
                }

                unmatched = false;
                if (reverse)
                    globals->globalDr->setGender(Male);
                else
                    globals->globalDr->setGender(Female);
            }
        }
    }

    return unmatched;
}

void unstructuredContent::contentReadAgeAtDeath(unsigned int maxSentences)
{
    beg();
    unstructuredContent sentence;
    QList<QString> firstNamesList = globals->globalDr->getFirstNameList();
    bool realSentenceEncountered = true;
    unsigned int i = 1;

    while ((globals->globalDr->missingDOB() || globals->globalDr->missingDOD()) && !isEOS() && (globals->globalDr->getAgeAtDeath() == 0) && (i <= maxSentences))
    {
        sentence = getSentence(firstNamesList, realSentenceEncountered, globals->globalDr->getLanguage());
        if (!(sentence.hasBookEnds(PARENTHESES) || sentence.isJustDates() || sentence.isJustNames()))
        {
            sentence.sentenceReadAgeAtDeath();
            i++;
        }
    }
}

unsigned int unstructuredContent::sentenceReadAgeAtDeath(bool updateDirectly)
{
    if (itsString.size() == 0)
        return 999;

    // Designed to read through one sentence at a time
    // If a valid age at death is located and a discrepancy in DODs is found (title/header vs text), DOD in text is used and dataRecord is updated

    unsigned int numDates;
	unsigned int maxDates = 2;
    unsigned int num;
    long altNum = 0;
	bool limitWords = false;
    bool fullyCredible = false;
    QList<QDate> dateList;
    QDate DOD, DOB;
    OQString word, nextWord, doubleWord, cleanString;
    QString target;
    bool noDates, justDOD, DODerror, partialDate, leapDay;
    unsigned int tempYOD, tempMOD, tempDOD;
    tempYOD = tempMOD = tempDOD = 0;
    qint64 daysDifference;
    bool contextError = false;
    bool birthMatched = false;
    bool eventCheck = false;
    bool lastHadComma, currentHasComma, hadOrdinal, reduceByOne;

    bool wordMatched = false;
    bool precedingFlag = false;
    OQString space(" ");
    LANGUAGE lang = globals->globalDr->getLanguage();

    numDates = pullOutDates(lang, dateList, maxDates, cleanString, limitWords);
    DOB = globals->globalDr->getDOB();
    DOD = globals->globalDr->getDOD();
    unsigned int ageAtDeath = 999;

    // If only one date in sentence, determine if it is DOB
    if (numDates == 1)
    {
        // look for key birth or death words in the sentence
        birthMatched = false;
        beg();

        while (!birthMatched && !isEOS())
        {
            word = getWord(true).lower();
            birthMatched = word.isBirthWord(lang);
        }
    }

    // Address rare instance of partial date being used
    if ((numDates > 9900) && (globals->globalDr->getYOD() == 0))
    {
        // Use DOS to impute YOD
        QDate DOSD = globals->globalDr->getDOS();
        if (DOSD.isValid())
        {
            if (DOSD.month() >= 2)
                globals->globalDr->setYOD(static_cast<unsigned int>(DOSD.year()));
            else
            {
                if (numDates == 9901)
                    globals->globalDr->setYOD(static_cast<unsigned int>(DOSD.year()));
                else
                    globals->globalDr->setYOD(static_cast<unsigned int>(DOSD.year() - 1));
            }
        }
    }
    partialDate = ((numDates > 9900) && (globals->globalDr->getYOD() > 0)) ? true : false;
    if (partialDate)
    {
        // Where YOD is known, assume sentence reads "... passed away on May 23rd at the age of 65"
        unstructuredContent tempContent(cleanString);
        cleanString.clear();
        QDate tempDOD;
        tempDOD = tempContent.fillInDate(lang, cleanString, static_cast<int>(globals->globalDr->getYOD()));
        if ((DOD.isValid() && (tempDOD == DOD)) || !DOD.isValid())
        {
            // Reset variables to mimic date being found in sentence
            numDates = 1;
            dateList.append(DOD);
            if (!DOD.isValid() && tempDOD.isValid())
            {
                DOD = tempDOD;
                globals->globalDr->setDOD(DOD);
            }
        }
    }

    noDates = (numDates == 0) ? true : false;
    justDOD = ((numDates == 1) && !birthMatched && (dateList[0] == DOD)) ? true : false;
    DODerror = ((numDates == 1) && ((DOB.isValid() && (dateList[0] != DOB)) || (!DOB.isValid() && !birthMatched)) && DOD.isValid() && (dateList[0] != DOD)) ? true : false;

    if (DODerror)   // Assumes DOB is correct (much less likely to be incorrect)
        daysDifference = DOD.daysTo(dateList[0]);
    else
        daysDifference = 0;

    // Only process if a) no dates, or b) a single date, being the DOD, was read
    if (noDates || justDOD || DODerror)
	{
        bool numFound = false;

        if (!numFound)
        {
            if (justDOD)
            {
                tempYOD = static_cast<unsigned int>(DOD.year());
                tempMOD = static_cast<unsigned int>(DOD.month());
                tempDOD = static_cast<unsigned int>(DOD.day());
                leapDay = (tempMOD == 2) && (tempDOD == 29);
            }
        }

        bool spousalReference;
        bool contextChanged = false;
        unstructuredContent cleanContent(cleanString);
        cleanContent.beg();
        currentHasComma = false;

        while (!contextChanged && (!wordMatched || !numFound) && !cleanContent.isEOS())
		{
            spousalReference = false;
            if (!numFound)
                reduceByOne = false;
            lastHadComma = currentHasComma;

            word = cleanContent.getWord(true).lower();
            if (word.isHyphenated() || (word.left(1) == PQString("-")))
                word = word.postHyphen();
            currentHasComma = word.removeEnding(COMMA);

            switch (lang)
            {
            case french:
                if ((word == OQString("mari")) || (word == OQString("femme")) || (word == "partenaire"))
                    spousalReference = true;
                break;

            case spanish:
                if ((word == OQString("marido")) || (word == OQString("esposa")))
                    spousalReference = true;
                break;

            default:
                if ((word == OQString("husband")) || (word == OQString("wife")) || (word == OQString("partner")))
                    spousalReference = true;
                break;
            }

            if (spousalReference)
            {
                // Skip over everything until a comma or death word is encountered
                bool keepSkipping = true;
                while (!cleanContent.isEOS() && keepSkipping)
                {
                    lastHadComma = currentHasComma;
                    word = cleanContent.getWord(true).lower();
                    if (word.isHyphenated() || (word.left(1) == PQString("-")))
                        word = word.postHyphen();
                    currentHasComma = word.removeEnding(COMMA);

                    if (word.isDeathWord(lang) || currentHasComma)
                        keepSkipping = false;
                }

                word = cleanContent.getWord(true).lower();
                if (word.isHyphenated() || (word.left(1) == PQString("-")))
                    word = word.postHyphen();
                currentHasComma = word.removeEnding(COMMA);
            }

            word.removeEnding(";");
            hadOrdinal = word.removeOrdinal(lang);
            nextWord = cleanContent.peekAtWord();
            nextWord.removeEnding(PUNCTUATION);            
            doubleWord = word + space + nextWord;
            if (doubleWord.isFoundIn(precedingIndicators, 1))
            {
                if ((word == OQString("in")) || (word == OQString("dans")))
                {
                    OQString peekTwoAhead = cleanContent.peekAtWord(false, 2);
                    if (peekTwoAhead.removeOrdinal(lang) || peekTwoAhead.isNumeric())
                        precedingFlag = true;
                }
                else
                    precedingFlag = true;
            }

			if (word.isAlpha())
			{
				// Check if the word matches one of the predefined age words
                if (word.isAgeWord(lang))
                    wordMatched = true;
                else
                {
                    if (word.isFoundIn(changedContextIndicators, 1))
                        contextChanged = true;
                    else
                    {
                        if ((word == OQString("after")) || (word == OQString("apres")) || (word == OQString(QLatin1String("après"))))
                            eventCheck = true;
                    }
                }
			}
			else
			{

                if (hadOrdinal)
                {
                    if ((nextWord.lower() == PQString("year")) || (nextWord.lower() == PQString("annee")) || (nextWord.lower() == PQString(QLatin1String("année"))))
                        reduceByOne = true;

                    if ((nextWord.lower() == PQString("birthday")) || (nextWord.lower() == PQString("anniversaire")))
                        wordMatched = true;
                }

                if (word.isNumeric())
				{
                    num = static_cast<unsigned int>(word.asNumber());
                    if (num < 125)
                    {
                        numFound = true;
                        contextError = false;
                        ageAtDeath = num;
                        fullyCredible = false;

                        // Run error checking to catch errors such as "..at the age of 3 months"
                        bool endOfSentence = word.removeEnding(QString(".")) || cleanContent.isEOS();
                        if (!endOfSentence)
                        {
                            if ((nextWord == QString("days")) || (nextWord == QString("day")) || (nextWord == QString("jours")) || (nextWord == QString("días")))
                            {
                                contextError = true;
                                altNum = num / 365;
                            }

                            if ((nextWord == QString("months")) || (nextWord == QString("month")) || (nextWord == QString("mois")) || (nextWord == QString("meses")) || (nextWord == QString("mes")))
                            {
                                contextError = true;
                                altNum = num / 12;
                            }

                            if (eventCheck)
                            {
                                OQString peekTwoAhead = cleanContent.peekAtWord(false, 2);
                                if ((peekTwoAhead == OQString("battle")) || (peekTwoAhead == OQString("illness")) || (peekTwoAhead == OQString("courageous")))
                                {
                                    contextError = true;
                                    altNum = 999;
                                }
                            }
                        }

                        // Check for special instances of "John Smith, 68, left us...."
                        if (lastHadComma && currentHasComma)
                            wordMatched = true;
                    }
				}
			}
		} // end of looping through entire sentence

		// Store data if valid information was read in
        if (numFound && wordMatched)
		{
            if (precedingFlag || reduceByOne)
            {
                ageAtDeath--;
                if (reduceByOne)
                    globals->globalDr->setAgeNextReference(true);
            }

            if (contextError)
            {
                ageAtDeath = static_cast<unsigned int>(altNum);
                //PQString errMsg;
                //errMsg << "Confirm age at death for: " << globals->globalDr->getURL();
                //globals->logMsg(ErrorRecord, errMsg);
                globals->globalDr->wi.ageFlag = 1;
            }

            if (updateDirectly)
            {
                if (!DODerror)
                {
                    globals->globalDr->setAgeAtDeath(ageAtDeath, fullyCredible);
                    globals->globalDr->setMinMaxDOB();
                }
                else
                {
                    // Determine if information can be used
                    // "12" cutoff used to eliminate potential references to months or days from a certain date
                    if ((daysDifference >= -10) && (daysDifference <= 10) && (ageAtDeath >= 12))
                    {
                        fullyCredible = false;
                        globals->globalDr->setDOD(dateList[0], true);
                        globals->globalDr->setAgeAtDeath(ageAtDeath, fullyCredible);
                        globals->globalDr->setMinMaxDOB();
                    }
                }
            }
		}
        else
            ageAtDeath = 999;

	} // end if a single DOD existed

    return ageAtDeath;

}

unsigned int unstructuredContent::sentenceReadNakedAgeAtDeath(bool updateDirectly)
{
    if (itsString.size() == 0)
        return 999;

    // Designed to read through one single sentence
    // Initially replicates the non-Naked function logic, so perhaps combine the two down the road if appropriate

    unsigned int numDates;
    unsigned int num;
    long altNum = 0;
    bool limitWords = false;
    bool fullyCredible = false;
    QList<QDate> dateList;
    OQString word, nextWord, doubleWord, cleanString;
    QString target;
    bool contextError = false;
    bool eventCheck = false;
    bool lastHadComma, currentHasComma, hadOrdinal, reduceByOne;

    bool wordMatched = false;
    bool precedingFlag = false;
    OQString space(" ");
    LANGUAGE lang = globals->globalDr->getLanguage();

    numDates = pullOutDates(lang, dateList, 1, cleanString, limitWords);
    if (numDates > 0)
    {
        // Don't repeat earlier logic and only look at wording not analyzed before first date
        int firstDateLocation = this->firstDifferenceFrom(cleanString.getString());
        setItsString(left(firstDateLocation));
    }

    unsigned int ageAtDeath = 999;

    bool numFound = false;
    bool spousalReference;
    bool contextChanged = false;
    currentHasComma = false;
    beg();

    while (!contextChanged && (!wordMatched || !numFound) && !isEOS())
    {
        spousalReference = false;
        if (!numFound)
            reduceByOne = false;
        lastHadComma = currentHasComma;

        word = getWord(true).lower();
        if (word.isHyphenated() || (word.left(1) == PQString("-")))
            word = word.postHyphen();
        if (word.right(1) == PQString(","))
            currentHasComma = true;
        else
            currentHasComma = false;
        switch (lang)
        {
        case french:
            if ((word == OQString("mari")) || (word == OQString("femme")) || (word == "partenaire"))
                spousalReference = true;
            break;

        case spanish:
            if ((word == OQString("marido")) || (word == OQString("esposa")))
                spousalReference = true;
            break;

        default:
            if ((word == OQString("husband")) || (word == OQString("wife")) || (word == OQString("partner")))
                spousalReference = true;
            break;
        }

        if (spousalReference)
        {
            // Drop everything until a comma or death word is encountered
            bool keepGoing = true;
            while (!isEOS() && keepGoing)
            {
                lastHadComma = currentHasComma;
                word = getWord(true).lower();
                if (word.isHyphenated() || (word.left(1) == PQString("-")))
                    word = word.postHyphen();
                if (word.right(1) == PQString(","))
                    currentHasComma = true;
                else
                    currentHasComma = false;
                if (word.removeEnding(COMMA))
                {
                    lastHadComma = currentHasComma;
                    word = getWord(true).lower();
                    if (word.right(1) == PQString(","))
                        currentHasComma = true;
                    else
                        currentHasComma = false;
                    keepGoing = false;
                }
                if (word.isDeathWord(lang))
                    keepGoing = false;
            }
        }
        word.removeEnding(",");
        word.removeEnding(";");
        hadOrdinal = word.removeOrdinal(lang);
        nextWord = peekAtWord();
        nextWord.removeEnding(PUNCTUATION);
        doubleWord = word + space + nextWord;
        if (doubleWord.isFoundIn(precedingIndicators, 1))
        {
            if ((word == OQString("in")) || (word == OQString("dans")))
            {
                OQString peekTwoAhead = peekAtWord(false, 2);
                if (peekTwoAhead.removeOrdinal(lang) || peekTwoAhead.isNumeric())
                    precedingFlag = true;
            }
            else
                precedingFlag = true;
        }

        if (word.isAlpha())
        {
            // Check if the word matches one of the predefined age words
            if (word.isDeathWord(lang))
                wordMatched = true;
            else
            {
                if (word.isFoundIn(changedContextIndicators, 1))
                    contextChanged = true;
                else
                {
                    if ((word == OQString("after")) || (word == OQString("apres")) || (word == OQString(QLatin1String("après"))))
                        eventCheck = true;
                }
            }
        }
        else
        {
            if (hadOrdinal)
            {
                if ((nextWord.lower() == PQString("year")) || (nextWord.lower() == PQString("annee")) || (nextWord.lower() == PQString(QLatin1String("année"))))
                    reduceByOne = true;

                if ((nextWord.lower() == PQString("birthday")) || (nextWord.lower() == PQString("anniversaire")))
                    wordMatched = true;
            }

            if (word.isNumeric())
            {
                num = static_cast<unsigned int>(word.asNumber());
                if (num < 125)
                {
                    numFound = true;
                    contextError = false;
                    ageAtDeath = num;
                    fullyCredible = false;

                    // Run error checking to catch errors such as "..at the age of 3 months"
                    bool endOfSentence = word.removeEnding(QString("."));
                    if (!endOfSentence)
                    {
                        if ((nextWord == QString("days")) || (nextWord == QString("day")) || (nextWord == QString("jours")) || (nextWord == QString("días")))
                        {
                            contextError = true;
                            altNum = num / 365;
                        }

                        if ((nextWord == QString("months")) || (nextWord == QString("month")) || (nextWord == QString("mois")) || (nextWord == QString("meses")) || (nextWord == QString("mes")))
                        {
                            contextError = true;
                            altNum = num / 12;
                        }

                        if (eventCheck)
                        {
                            OQString peekTwoAhead = peekAtWord(false, 2);
                            if ((peekTwoAhead == OQString("battle")) || (peekTwoAhead == OQString("illness")) || (peekTwoAhead == OQString("courageous")))
                                contextError = true;
                        }
                    }

                    // Check for special instances of "John Smith, 68, left us...."
                    if (lastHadComma && currentHasComma)
                        wordMatched = true;
                }
            }
        }
    } // end of looping through entire sentence

    // Store data if valid information was read in
    if (numFound && wordMatched)
    {
        if (precedingFlag || reduceByOne)
        {
            ageAtDeath--;
            if (reduceByOne)
                globals->globalDr->setAgeNextReference(true);
        }

        if (contextError)
        {
            ageAtDeath = static_cast<unsigned int>(altNum);
            //PQString errMsg;
            //errMsg << "Confirm age at death for: " << globals->globalDr->getURL();
            //globals->logMsg(ErrorRecord, errMsg);
            globals->globalDr->wi.ageFlag = 1;
        }

        if (updateDirectly)
        {
            globals->globalDr->setAgeAtDeath(ageAtDeath, fullyCredible);
            globals->globalDr->setMinMaxDOB();
        }
    }
    else
        ageAtDeath = 999;


    return ageAtDeath;

}

void unstructuredContent::readDateOfService(QDate &DOSD, LANGUAGE lang)
{
    beg();
    unstructuredContent sentence;
    QList<QString> firstNamesList = globals->globalDr->getFirstNameList();
    bool realSentenceEncountered = true;

    while (!DOSD.isValid() && !isEOS())
    {
        sentence = getSentence(firstNamesList, realSentenceEncountered, lang);
        if (!sentence.isJustDates())
            sentence.sentenceReadDateOfService(DOSD, lang);
    }
}

void unstructuredContent::sentenceReadDateOfService(QDate &DOSD, LANGUAGE lang)
{
    if (itsString.size() == 0)
        return;

    bool validSentenceFound = false;

    if (itsString.contains(QString("funeral"), Qt::CaseInsensitive) || itsString.contains(QString("service"), Qt::CaseInsensitive) || itsString.contains(QString("celebration"), Qt::CaseInsensitive))
        validSentenceFound = true;

    if (!validSentenceFound && (lang == french) && (itsString.contains(QString(QLatin1String("funéraille")), Qt::CaseInsensitive) || itsString.contains(QString(QLatin1String("célébration")), Qt::CaseInsensitive)
                                                    || itsString.contains(QString("service"), Qt::CaseInsensitive)))
        validSentenceFound = true;

    if (validSentenceFound)
    {
        OQString cleanString;
        QList<QDate> dateList;
        bool limitWords = false;
        bool serviceDate = true;
        unsigned int numDates = pullOutDates(lang, dateList, 2, cleanString, limitWords, serviceDate);

        if (numDates == 1)
            DOSD = dateList[0];
    }
}

void unstructuredContent::parseDateFromAndTo(const OQString &word, MAKEDATE &makeDate, DATEORDER dateOrder)
{
    OQString singleChar;
	unsigned int total;
	bool pureNumbers = false;
    bool stillOK, numStarted, isNumber;

	// Repeat basic test in case called directly in future
	singleChar = word[0];  
	if (!singleChar.isNumeric())
		return;

	// Minimum requirement is to pull three distinct numbers from word
	total = 0;
	stillOK = true;
	numStarted = false;
    OQStream stream(word);
	while (!stream.isEOS() && stillOK)
	{
		singleChar = stream.get();
        isNumber = singleChar.isNumeric();
        if (isNumber)
		{
			numStarted = true;
            total = total * 10 + static_cast<unsigned int>(singleChar.asNumber());
			makeDate.countChar();
		}

        if (!isNumber || stream.isEOS())
		{
            if (singleChar == QString(":"))  // dealing with a time 2:30pm
                stillOK = false;
            else
            {
                if (numStarted)
                {
                    makeDate.store(total);
                    total = 0;
                    makeDate.nextNum();
                    numStarted = false;
                }
                else;
                    // assume extra dividers between numbers and do nothing
            }
		}
	}

	// Review information captured
	if (makeDate.totalStored == 3)
		stillOK = true;
	else
	{
		stillOK = false;
		if ((makeDate.totalStored == 1) && (makeDate.getNumChar(1) == 8) && (word.getLength() == 8))
			pureNumbers = true; 
	}

	// TO DO: code pulling out dates in this situation if required
	if (pureNumbers)
	{
        PQString errMsg;
        errMsg << "Need to code pure numbers to pull dates from :";
        errMsg << " search prior night's run";
        globals->logMsg(ErrorRecord, errMsg);
	}

	if (stillOK)
        makeDate.analyzeNumbers(dateOrder);

	return;
}

void unstructuredContent::removeAllKnownNames()
{
	unsigned int numWordsRead = 0;
    OQString word, originalWord, lastChar, tempString, nextWord;
    QString space(" ");
    bool inParentheses, inQuotes, isAboriginal, hasComma;

	beg();      // go to beginning of content
    while (!EOS && (numWordsRead < 1000))
	{
		originalWord = getWord(true);		// pull next word from content - don't split words in parentheses or quotes
		word = originalWord;
		numWordsRead++;
        isAboriginal = originalWord.isAboriginalName(peekAtWord());
        while ((originalWord.isCompoundName() || isAboriginal) && (originalWord.lower() != OQString("e.")) && !EOS)
        {
            word += space;
            originalWord = getWord(true);
            hasComma = (originalWord.right(1) == OQString(","));
            word += originalWord;
            isAboriginal = !hasComma && isAboriginal && originalWord.isAboriginalName(peekAtWord());
        }
        word.removeEnding(",");
		inParentheses = word.hasBookEnds(PARENTHESES);
		inQuotes = word.hasBookEnds(QUOTES);

		if (inParentheses || inQuotes)
		{
			word.removeBookEnds();
            word.cleanUpEnds();
            word.removeLeadingNeeEtAl(globals->globalDr->getLanguage());
            if (word.lower() == PQString("in memoriam"))
				word.clear();
			originalWord = word;
		}

        lastChar = word.right(1);
        if (((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION) && word.isAlpha())
			word.dropRight(1);

        if (!(static_cast<bool>(globals->globalDr->isASavedName(word) || globals->globalDr->isAnInitial(word))))
		{
			tempString += originalWord;
            tempString += space;
		}
	}

	// Copy cleaned up list of words to itsString
    // Invalidates any previously determined sentence breaks
    setItsString(tempString.getString());
}

OQStream unstructuredContent::getJustNames(const NAMESKNOWN nk)
{
	// Read strategy depends on what is known about names going in
	
    OQStream result;
    OQString originalWord, word, nextWord1, nextWord2, nextWord3;
    PQString space(" ");

	bool nameStarted = false;
	bool nameEnded = false;
	bool undoPeeks = false;
	bool possibleName, matched, potentialPartialA, potentialPartialB;
	unsigned int numWordsProcessed = 0;
	unsigned int i;

	/*************************************************/
	/*    First task is to identify start of name    */
	/*************************************************/
	
	beg();
	while (!nameStarted && !EOS)
	{
		// If additional words were peeked at, reset to proper starting point
		if (undoPeeks)
		{
			beg();
			for (i = 0; i < numWordsProcessed; i++)
				getWord(true);
			undoPeeks = false;
		}
		originalWord = getWord(true);
		numWordsProcessed++;
		word = originalWord;

		// Remove comma and parentheses if present
        word.removeEnding(",");
		word.removeBookEnds();
        word.cleanUpEnds();
		word.removeLeadingNeeEtAl();

		possibleName = word.isAlpha();

		if (possibleName)
		{
			switch (nk)
			{
			case nkLastOnly:
                matched = globals->globalDr->isALastName(word.lower());
				break;

			case nkFirstOnly:
                matched = globals->globalDr->isAFirstName(word.lower());
				break;

			case nkFirstAndLast:
                matched = globals->globalDr->isAFirstName(word.lower()) || globals->globalDr->isALastName(word.lower());
				break;

			case nkNone:
				matched = false;
			}

			if (matched)
			{
				nameStarted = true;
				result += originalWord;
			}
			else
			{
				// Coding to attempt reading name with little or no guidance at all
				// Read ahead to get the next three words
				undoPeeks = true;
				nextWord1.clear();
				nextWord2.clear();
				nextWord3.clear();
				if (!EOS)
					nextWord1 = getWord(true);
				if (!EOS)
					nextWord2 = getWord(true);
				if (!EOS)
					nextWord3 = getWord(true);

				// Start with uncapitalized partial last names such as "de" - allow for up to two of them
				potentialPartialA = lookupUncapitalizedNames(word);
				if (potentialPartialA)
				{
					potentialPartialB = lookupUncapitalizedNames(nextWord1);
					if (potentialPartialB)
					{
						if (nextWord2.isCapitalized())
						{
							bool notDate = true;
                            if ((globals->globalDr->getLanguage() == english) && nextWord2.isWrittenMonth(english) && nextWord3.isNumeric())
								notDate = false;
							if (notDate)
							{
								nameStarted = true;
								result += originalWord;
							}
							else
							{
								// Not a proper name, move onto to next word
							}
						}
						else
						{
							// Not a proper name, move onto to next word
						}
					}
					else   // follow up on first potentialPartial
					{
						if (nextWord1.isCapitalized())
						{
							bool notDate = true;
                            if ((globals->globalDr->getLanguage() == english) && nextWord1.isWrittenMonth(english) && nextWord2.isNumeric())
								notDate = false;
							if (notDate)
							{
								nameStarted = true;
								result += originalWord;
							}
							else
							{
								// Not a proper name, move onto to next word
							}
						}
						else
						{
							// Not a proper name, move onto to next word
						}
					}	// end if potential partial B
				}	// end if potential partial A
				else
				{
					// Check if original word is capitalized and not a month (actual date)
					if (word.isCapitalized())
					{
						bool notDate = true;
                        if ((globals->globalDr->getLanguage() == english) && word.isWrittenMonth(english) && nextWord1.isNumeric())
							notDate = false;
						if (notDate)
						{
							nameStarted = true;
							result += originalWord;
						}
						else
						{
							// Not a proper name, move onto to next word
						}
					}
					else
					{
						// Not a proper name, move onto to next word
					}
				}   // end of if capitalized
			}   // end of checking unmatched words
		}  // end of if possible name

	}  // end of while identifying start of names

	/*************************************************/
	/*     Next task is to identify end of name      */
	/*     Coding is very similar                    */
	/*************************************************/

	while (nameStarted && !nameEnded && !EOS)
	{
		// If additional words were peeked at, reset to proper starting point
		if (undoPeeks)
		{
			beg();
			for (i = 0; i < numWordsProcessed; i++)
				getWord(true);
			undoPeeks = false;
		}
		originalWord = getWord(true);
		numWordsProcessed++;
		word = originalWord;

		// Remove comma and parentheses if present
        word.removeEnding(",");
		word.removeBookEnds();
        word.cleanUpEnds();
		word.removeLeadingNeeEtAl();

		possibleName = word.isAlpha();

		if (possibleName)
		{
			switch (nk)
			{
			case nkLastOnly:
                matched = globals->globalDr->isALastName(word.lower());
				break;

			case nkFirstOnly:
                matched = globals->globalDr->isAFirstName(word.lower());
				break;

			case nkFirstAndLast:
                matched = globals->globalDr->isAFirstName(word.lower()) || globals->globalDr->isALastName(word.lower());
				break;

			case nkNone:
				matched = false;
			}

			if (matched)
			{
                result += space;
				result += originalWord;
			}
			else
			{
				// Coding to attempt reading name with potentially no guidance at all
				// Read ahead to get the next three words
				undoPeeks = true;
				nextWord1.clear();
				nextWord2.clear();
				nextWord3.clear();
				if (!EOS)
					nextWord1 = getWord(true);
				if (!EOS)
					nextWord2 = getWord(true);
				if (!EOS)
					nextWord3 = getWord(true);

				// Start with uncapitalized partial last names such as "de" - allow for up to two of them
				potentialPartialA = lookupUncapitalizedNames(word);
				if (potentialPartialA)
				{
					potentialPartialB = lookupUncapitalizedNames(nextWord1);
					if (potentialPartialB)
					{
						if (nextWord2.isCapitalized())
						{
							bool notDate = true;
                            switch (globals->globalDr->getLanguage())
							{
							case french:
								// TODO:
								break;

							case spanish:
								// TODO:
								break;

							case english:
							default:
								if (nextWord2.isWrittenMonth(english) && nextWord3.isNumeric())
									notDate = false;
								break;
							}
							if (notDate)
							{
                                result += space;
								result += originalWord;
							}
							else
							{
								nameEnded = true;
							}
						}
						else
						{
							nameEnded = true;
						}
					}
					else   // follow up on first potentialPartial
					{
						if (nextWord1.isCapitalized())
						{
							bool notDate = true;
                            switch (globals->globalDr->getLanguage())
							{
							case french:
								// TODO:
								break;

							case spanish:
								// TODO:
								break;

							case english:
							default:
								if (nextWord1.isWrittenMonth(english) && nextWord2.isNumeric())
									notDate = false;
								break;
							}
							if (notDate)
							{
                                result += space;
								result += originalWord;
							}
							else
							{
								nameEnded = true;
							}
						}
						else
						{
							nameEnded = true;
						}
					}	// end if potential partial B
				}	// end if potential partial A
				else
				{
					// Check if original word is capitalized and not a month (actual date)
					if (word.isCapitalized())
					{
						bool notDate = true;
                        switch (globals->globalDr->getLanguage())
						{
						case french:
							// TODO:
							break;

						case spanish:
							// TODO:
							break;

						case english:
						default:
							if (word.isWrittenMonth(english) && nextWord1.isNumeric())
								notDate = false;
							break;
						}
						if (notDate)
						{
                            result += space;
							result += originalWord;
						}
						else
						{
							nameEnded = true;
						}
					}
					else
					{
						nameEnded = true;
					}
				}   // end of if capitalized
			}   // end of checking unmatched words
		}	// of of possible name
		else
			nameEnded = true;

	}	// end while
	
	result.cleanUpEnds();
	return result;
}

int unstructuredContent::countFrequency(QString word, Qt::CaseSensitivity caseSensitivity) const
{
    int count = 0;
    int position = 0;

    while (position != -1)
    {
        position = itsString.indexOf(word, position, caseSensitivity);
        if (position >= 0)
        {
            count++;
            position++;
        }
    }

    return count;
}

int unstructuredContent::countFrequencyFirst(QString word, Qt::CaseSensitivity caseSensitivity) const
{
    int count = 0;
    PQString firstWord, target;
    OQStream sentence;

    if (caseSensitivity == Qt::CaseSensitive)
        target = word;
    else
        target = word.toLower();

    OQStream content(itsString);
    QString targetInQuotes = QString("\'") + target.getString() + QString("\'");
    QString targetAfterComma = QString(", ") + target.getString();

    while (!content.isEOS())
    {
        sentence = content.getSentence();
        if (caseSensitivity == Qt::CaseSensitive)
            firstWord = sentence.getWord();
        else
            firstWord = sentence.getWord().lower();

        // Remove quotes but intentionally leave ending commas such that LASTNAME, FIRSTNAME not be counted
        firstWord.removeBookEnds(QUOTES);

        if (firstWord == target)
            count++;
        else
        {
            if (sentence.getString().contains(targetInQuotes, Qt::CaseInsensitive) || sentence.getString().contains(targetAfterComma, Qt::CaseInsensitive))
                count++;
        }
    }

    return count;
}

int unstructuredContent::countFrequency(unstructuredContent *uc, QString word, Qt::CaseSensitivity caseSensitivity) const
{
    return uc->countFrequency(word, caseSensitivity);
}

int unstructuredContent::countFrequencyFirst(unstructuredContent *uc, QString word, Qt::CaseSensitivity caseSensitivity) const
{
    return uc->countFrequencyFirst(word, caseSensitivity);
}

bool unstructuredContent::nameAndGenderConsistent(QString name, GENDER gender)
{
    bool consistentNameAndGender = false;
    NAMESTATS nameStats;

    nameStatLookup(name, globals, nameStats);

    switch (gender)
    {
    case Male:
        switch(nameStats.credibility)
        {
        case veryHigh:
            if (nameStats.malePct > 0.95)
                consistentNameAndGender = true;
            break;

        case high:
            if (nameStats.malePct > 0.90)
                consistentNameAndGender = true;
            break;

        case medium:
            if (nameStats.malePct > 0.75)
                consistentNameAndGender = true;
            break;

        case low:
            if (nameStats.malePct > 0.50)
                consistentNameAndGender = true;
            break;

        case zero:
        default:
            if (nameStats.malePct > 0.25)
                consistentNameAndGender = true;
            break;
        }
        break;  // End of Male

    case Female:
        switch(nameStats.credibility)
        {
        case veryHigh:
            if (nameStats.malePct < 0.05)
                consistentNameAndGender = true;
            break;

        case high:
            if (nameStats.malePct < 0.10)
                consistentNameAndGender = true;
            break;

        case medium:
            if (nameStats.malePct < 0.25)
                consistentNameAndGender = true;
            break;

        case low:
            if (nameStats.malePct < 0.50)
                consistentNameAndGender = true;
            break;

        case zero:
        default:
            if (nameStats.malePct < 0.75)
                consistentNameAndGender = true;
            break;
        }
        break;  // End of Female

    case genderUnknown:
        // Use other names already loaded to guess at gender
        double unisex = genderLookup(globals->globalDr, globals);

        if (unisex > 0.9)
            consistentNameAndGender = nameAndGenderConsistent(name, Male);
        else
        {
            if (unisex < 0.1)
                consistentNameAndGender = nameAndGenderConsistent(name, Female);
            else
                consistentNameAndGender = false;  // Triggers warning message

        }

    }   // end of gender switch

    return consistentNameAndGender;
}


void unstructuredContent::setGlobalVars(GLOBALVARS &gv)
{
    globals = &gv;
    dateToday.setToday(globals->today);
}

void unstructuredContent::setContentLanguage(LANGUAGE lang)
{
    contentLanguage = lang;
}

void unstructuredContent::clear()
{
    contentGender = genderUnknown;
    contentLanguage = language_unknown;

    numEnglishDates = 0;
    numFrenchDates = 0;
    numSpanishDates = 0;

    itsString.clear();

    sentenceStartPositions.clear();
    lastSentenceStartPosition = -1;
    realSentenceEncountered = false;
}

void unstructuredContent::clearString()
{
    itsString.clear();

    sentenceStartPositions.clear();
    lastSentenceStartPosition = -1;
    realSentenceEncountered = false;
}

void unstructuredContent::clearSentenceStartPositions()
{
    sentenceStartPositions.clear();
}

int unstructuredContent::getNumSentences() const
{
    if (lastSentenceStartPosition == -1)
        return -1;
    else
        return sentenceStartPositions.size();
}

int unstructuredContent::getSentenceNum(int index) const
{
    int count = 0;

    if ((index < 0) || (index >= static_cast<int>(getLength())))
            return -1;

    while (count < sentenceStartPositions.size())
    {
        if (index >= sentenceStartPositions.at(count))
            count++;
        else
            return count;
    }

    return count;
}
