// OQStream.cpp

#include "OQStream.h"

OQStream::OQStream() : OQString(), position(0), EOS(true)
{
}

OQStream::OQStream(const OQStream &source) : OQString(source), position(0)
{
    if (itsString.length() > 0)
        EOS = false;
    else
        EOS = true;
}

OQStream::OQStream(const OQString source) : OQString(source), position(0)
{
    if (itsString.length() > 0)
		EOS = false;
	else
		EOS = true;
}

OQStream::OQStream(const PQString source) : OQString(source), position(0)
{
    if (itsString.length() > 0)
        EOS = false;
    else
        EOS = true;
}

OQStream::OQStream(const QString source) : OQString(source), position(0)
{
    if (itsString.length() > 0)
        EOS = false;
    else
        EOS = true;
}

OQStream::OQStream(const std::wstring source) : OQString(source), position(0)
{
    if (itsString.length() > 0)
		EOS = false;
	else
		EOS = true;
}

void OQStream::forward(unsigned int numPosition)
{
    int itsLength = itsString.length();
    int numPositions = static_cast<int>(numPosition);

    int maxMove = itsLength - position;
    if (numPositions <= maxMove)
		position += numPosition;
	else
		position = itsLength;
	if (position == itsLength)
		EOS = true;
}

void OQStream::backward(unsigned int numPosition)
{
    int maxMove = position;
    int numPositions = static_cast<int>(numPosition);


    if (numPositions <= maxMove)
        position -= numPositions;
	else
		position = 0;
    if (position < itsString.length())
		EOS = false;
}

void OQStream::beg()
{
	position = 0;
    if (itsString.length() > 0)
		EOS = false;
	else
		EOS = true;
}

void OQStream::end()
{
    EOS = true;
    position = itsString.length() - 1;
    if (itsString.length() > 0)
        position = itsString.length() - 1;
    else
        position = 0;
}

unsigned int OQStream::getPosition() const
{
    return static_cast<unsigned int>(position);
}


bool OQStream::moveTo(const QString &string, int maxMove, unsigned int repositionAfter)
{
    int result = itsString.indexOf(string, position, Qt::CaseSensitive);
    if (result == -1)
    {
        position = itsString.length();
        EOS = true;
        return false;
    }
    else
    {
        if (result < (position + maxMove))
        {
            position = result + string.length();
            if (position == itsString.length())
                EOS = true;
            return true;
        }
        else
        {
            if (repositionAfter == 1)
                forward(maxMove);
            return false;
        }
    }
}

bool OQStream::moveToEarliestOf(const QString &string1, const QString &string2, int maxMove)
{
    int result, result1, result2;
    QString string;

    result1 = itsString.indexOf(string1, position, Qt::CaseSensitive);
    result2 = itsString.indexOf(string2, position, Qt::CaseSensitive);

    if (result1 == -1)
        result1 = position + maxMove;
    if (result2 == -1)
        result2 = position + maxMove;

    if (result1 <= result2)
    {
        result = result1;
        string = string1;
    }
    else
    {
        result = result2;
        string = string2;
    }

    if (result < (position + maxMove))
    {
        position = result + string.length();
        if (position == itsString.length())
            EOS = true;
        return true;
    }
    else
    {
        position = itsString.length();
        EOS = true;
        return false;
    }
}

bool OQStream::moveToEarliestOf(const QString &string1, const QString &string2, const QString &string3, int maxMove)
{
    int result1, result2;
    QString string;

    result1 = itsString.indexOf(string1, position, Qt::CaseSensitive);
    result2 = itsString.indexOf(string2, position, Qt::CaseSensitive);

    if (result1 == -1)
        result1 = position + maxMove;
    if (result2 == -1)
        result2 = position + maxMove;

    if (result1 <= result2)
        return moveToEarliestOf(string1, string3, maxMove);
    else
        return moveToEarliestOf(string2, string3, maxMove);
}

bool OQStream::moveBackwardTo(const QString &string, int maxMove)
{
    int result = itsString.lastIndexOf(string, position, Qt::CaseSensitive);
    if (result == -1)
    {
        position = 0;
        EOS = false;
        return false;
    }
    else
    {
        if (result > (position - maxMove))
        {
            position = result + string.length();
            if (position == itsString.length())
                EOS = true;
            else
                EOS = false;
            return true;
        }
        else
            return false;
    }
}

bool OQStream::conditionalMoveTo(const QString &targetString, const QString &stopString, unsigned int repositionAfter, int maxMove)
{
    // Move or positioning is contingent on what is found and where
    // Final parameter determines final positioning where target not found before stopString;
    //   0 = Return to original position
    //   1 = Position at stopString location
    //   2 = Position after stopString location

    int itsStringLength = itsString.length();

    int targetResult = itsString.indexOf(targetString, position, Qt::CaseSensitive);
    int stopResult = itsString.indexOf(stopString, position, Qt::CaseSensitive);

    if ((targetResult == -1) && (stopResult == -1)) // Neither found
    {
        if (repositionAfter != 0)
        {
            position = itsStringLength;
            EOS = true;
        }
        return false;
    }
    else
    {
        if ((targetResult == -1) && (stopResult >= 0))  // Only stopTarget found
        {
            if (repositionAfter != 0)
            {
                position = stopResult;
                if (repositionAfter == 2)
                    position += stopString.length();
                if (position == itsStringLength)
                    EOS = true;
            }
            return false;
        }
        else    // targetString was found
        {
            if ((stopResult == -1) || ((stopResult > targetResult) && ((targetResult - position) < maxMove)))  // targetString found first and within maxMove
            {
                position = targetResult + targetString.length();
                if (position == itsStringLength)
                    EOS = true;
                return true;
            }
            else    // Both found, but stopTarget located first
            {
                if (repositionAfter != 0)
                {
                    position = stopResult;
                    if (repositionAfter == 2)
                        position += stopString.length();
                    if (position == itsStringLength)
                        EOS = true;
                }
                return false;
            }
        }
    }
}

bool OQStream::consecutiveMovesTo(const int maxMove, const QString &string1, const QString &string2, const QString &string3, const QString &string4)
{
    // maxMove is measured from point first string is matched, adjusted by the lengths of the string objects

    int initialPosition, endingPosition;
    initialPosition = position;
    endingPosition = position;
    bool success = false;

    while (!success && (initialPosition < (itsString.length() - string1.length() - string2.length() -  string3.length() - string4.length())))
    {
        success = moveTo(string1);
        if (success)
        {
            initialPosition = position - string1.length() + 1;     // Save the initial "match point" to restart if necessary
            success = moveTo(string2);
            if (success)
                endingPosition = position;
            else
                position = initialPosition; // Try again starting from last "match point"
        }
        else
            initialPosition = position;     // Required to move the stream to EOS when first string no longer findable

        if (success && (string3.size() > 0))
        {
            success = moveTo(string3);
            if (success)
                endingPosition = position;
            else
                position = initialPosition; // Try again starting from last "match point"
        }

        if (success && (string4.size() > 0))
        {
            success = moveTo(string4);
            if (success)
                endingPosition = position;
            else
                position = initialPosition; // Try again starting from last "match point"
        }

        if (success)
            success = (maxMove >= (endingPosition - initialPosition - string1.size() - string2.size() - string3.size() - string4.size()));

        if (!success)
            position = initialPosition; // Try again starting from last "match point"

    }

    return success;
}

bool OQStream::isEOS() const
{
	return EOS;
}

bool OQStream::contains(const QString &string, bool ignoreBookendedLetters)
{
    QString tempString = itsString;

    if (ignoreBookendedLetters)
    {
        QRegularExpression target;
        QRegularExpressionMatch match;

        // Non-greedy match required in case of 2+ sets of parentheses or quotes
        target.setPatternOptions(QRegularExpression::InvertedGreedinessOption);

        target.setPattern("(\\(.+\\))");
        match = target.match(tempString);
        while (match.hasMatch())
        {
            tempString.replace(target, "");
            match = target.match(tempString);
        }

        target.setPattern("(\".+\")");
        match = target.match(tempString);
        while (match.hasMatch())
        {
            tempString.replace(target, "");
            match = target.match(tempString);
        }
    }

    bool found = tempString.contains(string);

    return found;
}

bool OQStream::hasAllWordsCapitalized(bool mustBeAlpha)
{
    if (getLength() == 0)
        return true;

    bool stillGood = true;
    OQString word, firstChar;

    QString tempString = itsString;
    tempString.replace(QString("("), "");
    tempString.replace(QString("\'"), "");

    OQStream tempStream(tempString);

    tempStream.beg();
    while (stillGood && !tempStream.isEOS())
    {
        word = tempStream.getWord();
        firstChar = word.left(1);
        if (firstChar.isAlpha())
        {
            stillGood = firstChar.isAllCaps();
        }
        else
        {
            if (mustBeAlpha)
                stillGood = false;
        }
    }

    return stillGood;
}

bool OQStream::streamIsJustDates()
{
    OQString word;
    OQString tempCRLF(QString(QChar(65533)));
    bool isJustDates = true;
    int i = 1;
    beg();

    while (isJustDates && !isEOS())
    {
        word = getWord();
        while ((i == 1) && (word == tempCRLF)){
            word = getWord();}
        if (!(word.isHyphen() || word.isNumeric() || word.isWrittenMonth()))
            isJustDates = false;
        i++;
    }

    return isJustDates;
}

int OQStream::firstDifferenceFrom(QString comparisonString)
{
    int result = -1;
    int i = 0;
    int length = static_cast<int>(getLength());
    bool diffEncountered = false;

    if ((length == 0) && (comparisonString.length() == 0))
        return -1;
    else
    {
        if ((length == 0) || (comparisonString.length() == 0))
            return 0;
    }

    while ((i < length) && !diffEncountered && (result < 0))
    {
        diffEncountered = (itsString.at(i) != comparisonString.at(i));
        if (diffEncountered || ((i + 1) == comparisonString.length()))
            result = i;
        i++;
    }

    return result;
}

OQString OQStream::get()
{
    OQString result;

    if (!EOS)
        result = itsString.at(static_cast<int>(position));
    else
        result = OQString(_T("\0"));

    position++;
    if (position >= itsString.length())
		EOS = true;

    return result;
}

QChar OQStream::getQChar()
{
    QChar result;

    if (!EOS)
        result = itsString.at(position);
    else
        result = QChar('\0');

    position++;
    if (position >= itsString.length())
        EOS = true;

    return result;
}

OQString OQStream::getWord(const bool considerParentheses, unsigned int secondaryBreakChar, const bool secondaryBreakInParentheses)
{
    if (EOS)
        return OQString();

    bool wordStarted = false;
    bool wordEnded = false;
    bool ignoreSecondaryBreak = false;
    bool hasParentheses;
    OQString word;
    OQString singleChar;
    OQString stopChar;
    OQString space(_T(" "));
    unsigned int singleCharType;
    int startPos, endPos;

    // Get first non-space and non-break character
    while (!wordStarted && !EOS)
    {
        hasParentheses = false;
        singleChar = get();
        singleCharType = singleChar.getCharType() & secondaryBreakChar;
        if ((singleChar != space) && (singleCharType != secondaryBreakChar))
        {
            wordStarted = true;
            word = singleChar;
        }
    }

    // Set end point
    if (considerParentheses)
    {
        if ((singleChar.getCharType() & (OPENING | PARENTHESES)) == (OPENING | PARENTHESES))
        {
            stopChar = singleChar.reciprocal();
            ignoreSecondaryBreak = !secondaryBreakInParentheses;
            startPos = position - 1;
            hasParentheses = true;
        }
        else
        {
            if ((singleChar.getCharType() & (OPENING | QUOTES)) == (OPENING | QUOTES))
            {
                stopChar = singleChar.reciprocal();
                ignoreSecondaryBreak = !secondaryBreakInParentheses;
                startPos = position - 1;
                hasParentheses = true;
            }
            else
                stopChar = space;
        }
    }
    else
        stopChar = space;

    // Continue reading characters until end of word is reached
    while (!EOS && !wordEnded)
    {
        singleChar = get();
        singleCharType = singleChar.getCharType() & secondaryBreakChar;
        if ((singleChar != stopChar) && (ignoreSecondaryBreak || (singleCharType != secondaryBreakChar)))
            word += singleChar;
        else
        {
            wordEnded = true;
            endPos = position - 1;
            // Copy ending parentheses or quotes
            if ((stopChar != space) && (singleCharType != secondaryBreakChar))
                word += stopChar;
        }
    }

    if (hasParentheses && (word.getLength() > 100))
    {
        // An entire sentence or the entire unstructured text is a single block.
        QString tempString = itsString;
        tempString.remove(endPos, 1);
        tempString.remove(startPos, 1);
        itsString.clear();
        itsString = tempString;;
        position = startPos;
        word = getWord(considerParentheses, secondaryBreakChar, secondaryBreakInParentheses);
    }

    return word;
}


OQString OQStream::getCleanWord(const bool considerParentheses)
{
    OQString word;

    while (!EOS && (word.getLength() == 0))
    {
        word = getWord(considerParentheses);
        word.removeLeading(PUNCTUATION | PARENTHESES | QUOTES | SPACE | DIVIDER);
        word.removeEnding(PUNCTUATION | PARENTHESES | QUOTES | SPACE | DIVIDER);
    }

    return word;
}

OQStream OQStream::getSentence(const LANGUAGE &language)
{
    QList<QString> emptyFirstNames;
    bool defaultFlag = false;
    return getSentence(emptyFirstNames, defaultFlag, language);
}

OQStream OQStream::getSentence(bool &realSentenceEncounteredFlag, const LANGUAGE &language)
{
    QList<QString> emptyFirstNames;
    return getSentence(emptyFirstNames, realSentenceEncounteredFlag, language);
}

OQStream OQStream::getSentence(const QList<QString> &firstNames, bool &realSentenceEncounteredFlag, const LANGUAGE &language)
{
    QStringList emptyConditionalStopWords;
    return getSentence(firstNames, realSentenceEncounteredFlag, language, emptyConditionalStopWords);
}

OQStream OQStream::getSentence(const QList<QString> &firstNames, bool &realSentenceEncounteredFlag, const LANGUAGE &language, const QStringList &conditionalStopWords)
{   
    OQString word, nextWord, lastChar, result, checkForPureDates;
    OQStream tempStream;
    bool confirmedEndOfSentence = false;
    unsigned int charValue;
    std::wstring singleChar;
    OQString space(" ");
    OQString period(".");
    OQString conditionalBreakChar(QChar(65533));
    PQString errMsg;
    bool endingCharFound, keepPeriod, ignoreAbbreviationPeriod, stopWordEncountered, standAlonePeriod;
    bool forcedBreak = false;
    bool allNamesSoFar = true;
    int wordCount = 0;
    int charCount = 0;
    int start;
    QList<QDate> dateList;

    // read sentence from current location in stream
    while (!EOS && !confirmedEndOfSentence)
    {
        word = getWord(true);		// pull next word from content - don't split words in parentheses or quotes
        nextWord = peekAtWord(true);

        // Force sentence break for (mmmm dd, yyyy - mmmm dd, yyyy)
        checkForPureDates = word;
        if (checkForPureDates.removeBookEnds(PARENTHESES) && checkForPureDates.isPureDateRange(language))
            confirmedEndOfSentence = true;
        else
        {
            checkForPureDates = nextWord;
            if (checkForPureDates.removeBookEnds(PARENTHESES) && checkForPureDates.isPureDateRange(language))
                confirmedEndOfSentence = true;
        }

        if (!confirmedEndOfSentence)
        {
            if ((nextWord.getLength() == 0) && (getPosition() == getLength()))
                EOS = true;
            if (nextWord == conditionalBreakChar)
            {
                word += getWord(true);
                nextWord = peekAtWord(true);
            }
            wordCount++;
            charCount += word.getLength();
            lastChar = word.right(1);

            word.removeEnding(conditionalBreakChar.getString());
            nextWord.removeEnding(conditionalBreakChar.getString());

            if (allNamesSoFar)
                allNamesSoFar = word.isAGivenName(errMsg) || word.isALastName(errMsg);

            // Try to differentiate between valid sentences and forced breaks for long sentences
            if (lastChar == conditionalBreakChar)
            {
                if (allNamesSoFar)
                {
                    stopWordEncountered = conditionalStopWords.contains(nextWord.lower().getString());
                    allNamesSoFar = !stopWordEncountered &&
                                        ((nextWord.isAGivenName(errMsg) || nextWord.isALastName(errMsg)) && !nextWord.isWrittenMonth(language)) &&
                                        !(nextWord.isRecognized() || nextWord.isRecognizedFirstWord());
                }

                if (((wordCount < 7) && (charCount < 45) && !allNamesSoFar) || ((charCount + wordCount) > 135) || realSentenceEncounteredFlag)
                {
                    // Insert a period and force break
                    lastChar = period;
                    forcedBreak = true;
                }
                else
                {
                    // Check for potential isJustDates() w/o using unstructured content
                    bool justDates = false;
                    if (wordCount < 10)
                    {
                        tempStream = OQStream(result + space + word);
                        tempStream.removeBookEnds(PARENTHESES);
                        justDates = tempStream.streamIsJustDates();
                        /*OQString tempWord;
                        justDates = true;
                        while (justDates && !tempStream.isEOS())
                        {
                            tempWord = tempStream.getWord();
                            if (!(tempWord.isHyphen() || tempWord.isNumeric() || tempWord.isWrittenMonth()))
                                justDates = false;
                        }*/
                    }

                    if (justDates)
                    {
                        // Insert a period and force break
                        lastChar = period;
                        forcedBreak = true;
                    }
                    else
                    {
                        // Treat as a single sentence
                        lastChar = space;
                    }
                }

                word += lastChar;
                start = findPosition(conditionalBreakChar);
                if (start != -1)
                {
                    int currentPosition = position;
                    replace(start, 1, lastChar.getString());
                    position = currentPosition;
                }
            }

            // Resume normal processing
            singleChar = lastChar.getWString();
            charValue = static_cast<unsigned int>(singleChar[0]);
            keepPeriod = false;

            // Look for exclamation mark (33), period (46) or question mark (63)
            endingCharFound = ((charValue == 33) || (charValue == 46) || (charValue == 63));
            if (endingCharFound)
            {
                if ((charValue == 33) || (charValue == 63))
                {
                    word.dropRight(1);
                    confirmedEndOfSentence = true;
                }
                else    // charvalue == 46 or period
                {
                    standAlonePeriod = (word.getLength() == 1);

                    if (standAlonePeriod)
                    {
                        ignoreAbbreviationPeriod = false;
                        confirmedEndOfSentence = true;
                        result.removeEnding(SPACE);
                    }
                    else
                    {
                        // Don't end sentence for known abbreviations
                        if (word.isAbbreviation() || (word.isCapitalized() && (word.getLength() <= 5) && !word.containsVowel() && !word.isNoVowelName() && !word.isNumeric()))
                        {
                            if (nextWord.hasBookEnds(PARENTHESES | QUOTES))
                                ignoreAbbreviationPeriod = false;
                            else
                            {
                                if (!nextWord.isCapitalized())
                                    ignoreAbbreviationPeriod = true;
                                else
                                {
                                    if (nextWord.isPronoun() || nextWord.isWrittenMonth() || word.isProvAbbreviation())
                                        ignoreAbbreviationPeriod = false;
                                    else
                                    {
                                        if (firstNames.size() == 0)
                                            ignoreAbbreviationPeriod = true;
                                        else
                                        {
                                            bool matched = false;
                                            QString name;
                                            QListIterator<QString> iter(firstNames);
                                            while (!matched && iter.hasNext())
                                            {
                                                name = iter.next();
                                                if (name.compare(nextWord.getString(), Qt::CaseInsensitive) == 0)
                                                    matched = true;
                                            }
                                            if (matched)
                                                ignoreAbbreviationPeriod = false;
                                            else
                                                ignoreAbbreviationPeriod = true;
                                        }
                                    }
                                }
                            }
                        }
                        else
                            ignoreAbbreviationPeriod = false;


                        if (ignoreAbbreviationPeriod || word.isAbbreviatedPrefix() || word.isSaint()  || word.isTitle(language) || word.isWrittenMonthAbbreviation(language) || word.isSuffix())
                        {
                            if (word.isSuffix()  && nextWord.isWrittenMonth())
                            {
                                keepPeriod = true;
                                confirmedEndOfSentence = true;
                            }
                            else
                            {
                                // Don't end sentence and keep going
                                keepPeriod = true;
                            }
                        }
                        else
                        {
                            // Check if the word is an initial(s) from the deceased's name
                            if (!word.isNumeric() && ((word.getLength() == 2) || ((word.getLength() == 4) && (word.middle(1,1) == period))))
                            {
                                // Don't end sentence here either
                                keepPeriod = true;
                            }
                            else
                            {
                                word.dropRight(1);
                                confirmedEndOfSentence = true;
                            }
                        }
                    }
                }
            }
        }

        // Drop all other ending punctuation
        if (!confirmedEndOfSentence && !keepPeriod && ((lastChar.getCharType() & PUNCTUATION) == PUNCTUATION) && word.isAlpha())
            word.dropRight(1);

        result += word;
        if (!confirmedEndOfSentence && (result.right(1) != space))
            result += space;
    }

    result.removeEnding(SPACE);
    OQStream sentence(result);

    if (realSentenceEncounteredFlag || !forcedBreak)
        realSentenceEncounteredFlag = true;

    return sentence;
}

OQString OQStream::getNext(unsigned int numChars)
{
    OQString word;
    OQString singleChar;
    unsigned int count = 0;

    while (!EOS && (count < numChars))
    {
        singleChar = get();
        word += singleChar;
        count++;
    }

    return word;
}

OQString OQStream::getUntil(QString target, unsigned int maxNumChars,  bool dropLast)
{
    OQString word;
    int index, resultLength, maxLength;
    bool matched;

    if ((itsString.length() - position) < static_cast<int>(maxNumChars))
        maxLength = itsString.length() - position;
    else
        maxLength = static_cast<int>(maxNumChars);

    index = itsString.indexOf(target, position);
    if (index == -1)
    {
        resultLength = maxLength;
        matched = false;
    }
    else
    {
        if ((index - position + target.length()) <= maxLength)
        {
            resultLength = index - position + target.length();
            matched = true;
        }
        else
        {
            resultLength = maxLength;
            matched = false;
        }
    }

    word = itsString.mid(position, resultLength);
    position += resultLength;
    if (position == itsString.length())
        EOS = true;

    // Special coding for JSON parsing to position properly
    if (target == QString(",\""))
        backward(1);

    if (matched && dropLast)
        word.dropRight(static_cast<unsigned int>(target.length()));

    return word;
}

OQString OQStream::getUntilEarliestOf(QString stop1, QString stop2, unsigned int maxNumChars, bool dropLast)
{
    QString target;
    int index1, index2;

    index1 = itsString.indexOf(stop1, position);
    index2 = itsString.indexOf(stop2, position);

    if ((index2 == -1) || ((index1 >= 0) && (index2 >= 0) && (index1 <= index2)))
        target = stop1;
    else
        target = stop2;

    return getUntil(target, maxNumChars, dropLast);
}

OQString OQStream::peekAtWord(const bool considerParentheses, const unsigned int howFar, const bool cleanWord)
{
    OQString word;
    int origPosition = this->position;		// Needed in case stream contains consecutive spaces
    for (unsigned int i = 0; i < howFar; i++)
        word = getWord(considerParentheses);
    this->backward(static_cast<unsigned int>(this->position - origPosition));  // EOS reset  by 'backward' if necessary

    if (cleanWord)
    {
        word.removeEnding(PUNCTUATION);
        word.removeInternalPeriods();
    }

	return word;
}

OQString OQStream::peekAtNext(const unsigned int numChars)
{
    OQString word;
    int origPosition = this->position;		// Needed in case stream contains consecutive spaces
    word = getNext(numChars);
    this->backward(static_cast<unsigned int>(this->position - origPosition));  // EOS reset  by 'backward' if necessary

    return word;
}

OQString OQStream::peekAtNextRealChar()
{
    OQString singleChar, word;
    unsigned int numChars = 1;
    bool finished = false;

    int origPosition = this->position;		// Needed in case stream contains consecutive spaces

    while (!finished && !isEOS())
    {
        word = getNext(numChars);
        this->backward(static_cast<unsigned int>(this->position - origPosition));  // EOS reset  by 'backward' if necessary
        singleChar = word.right(1);
        singleChar.simplify();
        if (singleChar.getLength() > 0)
            finished = true;
        else
            numChars++;
    }

    return singleChar;
}


OQString OQStream::readHTMLContent(unsigned int maxChar)
{
    // Looks for and picks out first set of wording between "quotes" or >parentheses<

    bool endReached = false;
    bool started = false;

    OQString heldSoFar;
    OQString singleChar, lastChar;
    OQString space(" ");
    OQString quote("\"");
    OQString openTag("<");
    OQString closeTag(">");
    OQString comma(",");

    while (!this->isEOS() && !endReached && (heldSoFar.getLength() < maxChar))
    {
        singleChar = this->get();
        if (!this->isEOS())
        {
            if (started)
            {
                if ((singleChar != quote) && (singleChar != openTag))
                {
                    // Ensure a blank space is included after each comma
                    if ((lastChar == comma) && (singleChar != space))
                        heldSoFar += space;
                    heldSoFar += singleChar;
                }
                else
                    endReached = true;
            }
            else
            {
                if ((singleChar == quote) || (singleChar == closeTag))
                    started = true;
                else;
                // Disregard;
            }
        }
        lastChar = singleChar;
    } // End while

    return heldSoFar.unQuoteHTML();
}

OQString OQStream::readNextBetween(unsigned int param)
{
    QChar singleChar;
    OQString heldSoFar;
    bool finished = false;
    unsigned int numParam = 0;
    unsigned int currentCharType;
    unsigned int numANSI;
    bool endPoint;

    while (!this->isEOS() && !finished)
    {
        singleChar = this->getQChar();
        numANSI = singleChar.unicode();

        // Address differences between ANSI and UNICODE below 256
        if(singleChar.unicode() >= 256)
        {
            PQString newSingle(singleChar);
            numANSI = newSingle.convertCharToANSI();
        }

        currentCharType = ANSI[numANSI].charType_defns;
        endPoint = ((currentCharType & param) == param);
        if (endPoint)
            numParam++;
        if ((numParam == 1) && !endPoint)
            heldSoFar += singleChar;
        if (numParam == 2)
            finished = true;
    }
    return heldSoFar;
}

OQString OQStream::readQuotedMetaContent()
{
    OQStream result;
    int index;
    QRegularExpression target;

    target.setPattern("[^a-z]>");
    index = itsString.indexOf(target, position);

    if ((index >= 0) && ((index - position) < 10000))
    {
        result = getNext(index - position + 1);
        result.removeEnding("/");
        result.cleanUpEnds();
        result.removeBookEnds(QUOTES);
        result.cleanUpEnds();
    }

    return result;
}

void OQStream::removeLinks()
{
    QRegularExpression targetI;
    QRegularExpressionMatch match;

    targetI.setPatternOptions(QRegularExpression::CaseInsensitiveOption | QRegularExpression::InvertedGreedinessOption);
    targetI.setPattern("<a href=\"(.*)(\">)(.*)(?!<a)(watch|click|view|video|stream|book|reserve|http|sign|@)(.*)(?!<a)</a>");

    // Coding for debugging
    match = targetI.match(this->itsString);
    while (match.hasMatch())
    {
        /*QString cap1 = match.captured(0);
        QString cap2 = match.captured(1);
        QString cap3 = match.captured(2);
        QString cap4 = match.captured(3);
        QString cap5 = match.captured(4);*/
        itsString.replace(targetI, "");
        match = targetI.match(this->itsString);
    }
}

OQStream& OQStream::removeHTMLtags(int insertPeriods)
{
    itsString.replace("<br", " <br");
    itsString.replace("  ", " ");

    // Remove all HTML tags and the content within them, along with consecutive blank spaces

    OQString openTag("<");		// 60
    OQString closeTag(">");     // 62
    OQString space(" ");		// 32
    OQString period(".");
    OQString comma(",");

    OQString singleChar, lastChar, HTMLtag;
    OQStream heldSoFar;

    bool tagOpen = false;
    lastChar = space;

    beg();
    while (!isEOS())
    {
        singleChar = get();

        if (singleChar == openTag)
            tagOpen = true;

        if (tagOpen)
            HTMLtag += singleChar;

        if (!tagOpen)
        {
            if(!((singleChar == space) && (lastChar == space)))
                heldSoFar += singleChar;
            lastChar = singleChar;
        }

        if (singleChar == closeTag)
        {
            tagOpen = false;
            if (((insertPeriods == 2) && HTMLtag.isEndOfBlockTag()) ||
                    ((insertPeriods == 1) && ((HTMLtag.left(3) != PQString("<br")) || heldSoFar.streamIsJustDates()) && HTMLtag.isEndOfBlockTag()))
            {
                heldSoFar.removeEnding(space.getString());
                if ((heldSoFar.right(1) != period) && (heldSoFar.right(1) != comma))
                {
                    heldSoFar += period;
                    lastChar = period;
                }

                if (heldSoFar.right(1) != space)
                {
                    heldSoFar += space;
                    lastChar = space;
                }
            }

            HTMLtag.clear();
        }
    }

    heldSoFar.removeLeading(period.getString());
    itsString = heldSoFar.getString();
    position = 0;
    EOS = (itsString.length() == 0);

    return *this;
}

OQStream& OQStream::removeContentWithin(unsigned int bookends)
{
    OQString heldSoFar;
    OQString word;
    QString space(" ");
    bool considerAndStopParentheses = true;

    while (!isEOS())
    {
        word = getWord(considerAndStopParentheses);
        if (word.removeBookEnds(bookends))
            word.clear();

        if (word.getLength() > 0)
        {
            if ((heldSoFar.getLength() > 0) && (word != OQString(",")))
                heldSoFar += space;
            heldSoFar += word;
        }
    }

    itsString = heldSoFar.getString();
    position = 0;
    EOS = (itsString.length() == 0);

    return *this;
}

OQStream& OQStream::operator= (const OQStream &rhs)
{
    itsString = QString(rhs.getString());
	position = rhs.position;
	EOS = rhs.EOS;

	return *this;
}

OQStream& OQStream::operator= (const OQString &rhs)
{
    itsString = rhs.getString();
    position = 0;
    EOS = (itsString.length() == 0);

    return *this;
}

OQStream& OQStream::operator= (const PQString &rhs)
{
    itsString = rhs.getString();
    position = 0;
    EOS = (itsString.length() == 0);

    return *this;
}

OQStream& OQStream::operator= (const QString &rhs)
{
    itsString = rhs;
    position = 0;
    EOS = (itsString.length() == 0);

    return *this;
}

OQStream& OQStream::operator= (const std::string &rhs)
{
    itsString = QString::fromStdString(rhs);

    position = 0;
    EOS = (itsString.length() == 0);

    return *this;
}

OQStream& OQStream::operator= (const std::wstring &rhs)
{
    itsString = QString::fromStdWString(rhs);

    position = 0;
    EOS = (itsString.length() == 0);

	return *this;
}

OQStream& OQStream::operator= (const QByteArray &rhs)
{
    //itsString = QString(QTextCodec::codecForMib(106)->toUnicode(rhs));
    itsString = QString(rhs);

    position = 0;
    EOS = (itsString.length() == 0);

    return *this;
}

bool OQStream::loadStringValue(const QString from, QString &toString, const bool stopAtRecord)
{
    bool fieldFound = false;
    bool null;
    OQString fieldValue;    
    OQString completeFrom(from);
    QString completedFrom;
    int startingPosition = position;

    if (completeFrom.hasBookEnds(QUOTES))
        completedFrom = completeFrom.getString() + QString(":");
    else
        completedFrom = QString("\"") + completeFrom.getString() + QString("\"") + QString(":");

    if (stopAtRecord)
    {
        fieldFound = conditionalMoveTo(completedFrom, QString("},{"), 0);
        if (!fieldFound)
            position = startingPosition;
    }
    else
        fieldFound = moveTo(completedFrom);

    if (fieldFound)
    {
        fieldValue = getUntilEarliestOf(QString(",\""), QString("}"));
        null = (fieldValue == OQString("null"));
        if (!null)
        {
            fieldValue.cleanUpEnds();
            fieldValue.removeBookEnds(QUOTES);
            fieldValue.unescapeJSONformat();
            fieldValue.replace("  ", " ");
            toString = fieldValue.getString();
        }
    }

    return fieldFound;
}

bool OQStream::loadValue(const QString from, QString &toString, const bool stopAtRecord)
{
    bool fieldFound = loadStringValue(from, toString, stopAtRecord);

    return fieldFound;
}

bool OQStream::loadValue(const QString from, PQString &toPQString, const bool stopAtRecord)
{
    QString stringResult;
    bool fieldFound = loadStringValue(from, stringResult, stopAtRecord);
    if (fieldFound)
        toPQString = PQString(stringResult);

    return fieldFound;
}

bool OQStream::loadValue(const QString from, int &toInt, const bool stopAtRecord)
{
    QString stringResult;
    bool fieldFound = loadStringValue(from, stringResult, stopAtRecord);
    if (fieldFound)
        toInt = stringResult.toInt();

    return fieldFound;
}

bool OQStream::loadValue(const QString from, unsigned int &toUInt, const bool stopAtRecord)
{
    QString stringResult;
    bool fieldFound = loadStringValue(from, stringResult, stopAtRecord);
    if (fieldFound)
        toUInt = stringResult.toUInt();

    return fieldFound;
}

bool OQStream::loadValue(const QString from, QDate &toDate, const QString dateFormat, const bool stopAtRecord)
{
    QString stringResult;
    QDate resultDate;
    QDateTime resultDT;
    bool fieldFound = loadStringValue(from, stringResult, stopAtRecord);
    if (fieldFound)
    {
        QRegularExpression target;
        target.setPattern("[T| ]\\d\\d:\\d\\d:\\d\\d");
        stringResult.replace(target, "");
        target.setPattern("\\.\\d+Z");
        stringResult.replace(target, "");

        /*int index = stringResult.indexOf("T", Qt::CaseSensitive);
        if (index > 6)
            stringResult.chop(stringResult.length() - index);*/
        resultDate = QDate::fromString(stringResult, dateFormat);
    }

    if (resultDate.isValid())
        toDate = resultDate;

    return fieldFound;
}

bool OQStream::loadValue(const QString from, bool &toBool, const bool stopAtRecord)
{
    QString stringResult;
    bool fieldFound = loadStringValue(from, stringResult, stopAtRecord);
    if (fieldFound)
    {
        if (stringResult == "true")
            toBool = true;
        else
            toBool = false;
    }

    return fieldFound;
}

// All overloaded functions below here

void OQStream::dropLeft(unsigned int num)
{
    PQString::dropLeft(num);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;
}

void OQStream::dropRight(unsigned int num)
{
    PQString::dropRight(num);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;
}

PQString& OQStream::simplify(bool insertPeriods)
{
    PQString::simplify(insertPeriods);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return *this;
}

void OQStream::replace(const QString target, const QString newString, const Qt::CaseSensitivity caseSensitive)
{
    PQString::replace(target, newString, caseSensitive);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;
}

void OQStream::replace(const int start, const int numChars, const QString newString)
{
    PQString::replace(start, numChars, newString);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;
}

void OQStream::replaceHTMLentities()
{
    PQString::replaceHTMLentities();
    if (itsString.length() == 0)
        EOS = true;
    position = 0;
}

void OQStream::removeSpecialChar()
{
    PQString::removeSpecialChar();
    if (itsString.length() == 0)
        EOS = true;
    position = 0;
}

bool OQStream::removeHyphens()
{
    bool result = PQString::removeHyphens();
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}

void OQStream::removeBlankSentences()
{
    PQString::removeBlankSentences();
    if (itsString.length() == 0)
        EOS = true;
    position = 0;
}

bool OQStream::removeBookEnds(unsigned int bookends)
{
    bool result = PQString::removeBookEnds(bookends);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}

bool OQStream::removeLeading(const unsigned int charCode)
{
    bool result = PQString::removeLeading(charCode);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}

bool OQStream::removeLeading(const std::wstring target)
{
    bool result = PQString::removeLeading(target);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}

bool OQStream::removeLeading(const QString target)
{
    bool result = PQString::removeLeading(target);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}

bool OQStream::removeEnding(const unsigned int charCode)
{
    bool result = PQString::removeEnding(charCode);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}

bool OQStream::removeEnding(const std::wstring target)
{
    bool result = PQString::removeEnding(target);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}
bool OQStream::removeEnding(const QString target)
{
    bool result = PQString::removeEnding(target);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}

bool OQStream::removeAll(const QString target)
{
    bool result = PQString::removeAll(target);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}

bool OQStream::removePossessive()
{
    bool result = PQString::removePossessive();
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}

bool OQStream::removeRepeatedLastName()
{
    OQString originalWord, cleanWord, word;
    QString newString;
    bool matched;
    bool keepGoing = true;

    itsString.replace(QChar(160), QChar(32));

    while (keepGoing)
    {
        beg();
        matched = false;
        originalWord = getWord();
        newString = originalWord.getString();
        cleanWord = originalWord.lower();
        cleanWord.removeEnding(PUNCTUATION);

        while (!isEOS())
        {
            word = getWord(true, SLASH, false);
            newString += QString(" ") + word.getString();
            word.removeEnding(PUNCTUATION);
            word.removeBookEnds();
            word = word.lower();

            if (word == cleanWord)
                matched = true;
        }

        if (matched)
        {
            newString.remove(0, originalWord.getLength() + 1);
            itsString = newString;
        }
        else
            keepGoing = false;
    }

    return matched;
}

bool OQStream::cleanUpEnds()
{
    bool result = PQString::cleanUpEnds();
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}

// OQString
bool OQStream::removeOrdinal(LANGUAGE language)
{
    bool result = OQString::removeOrdinal(language);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}

bool OQStream::removeLeadingNeeEtAl(LANGUAGE language)
{
    bool result = OQString::removeLeadingNeeEtAl(language);
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}

bool OQStream::removeLeadingAKA()
{
    bool result = OQString::removeLeadingAKA();
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}

bool OQStream::removeInternalPeriods()
{
    bool result = OQString::removeInternalPeriods();
    if (itsString.length() == 0)
        EOS = true;
    position = 0;

    return result;
}
