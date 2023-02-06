// OQString.cpp

#include "OQString.h"

OQString::OQString() : PQString()
{
}

OQString::OQString(const char * const cString) : PQString(cString)
{
}

OQString::OQString(const PQString &rhs) : PQString(rhs)
{
}

OQString::OQString(const QString &rhs) : PQString(rhs)
{
}

OQString::OQString(const OQString &rhs) : PQString(rhs)
{
}

OQString::OQString(const QChar& singleChar) : PQString(singleChar)
{
}

OQString::OQString(const std::string &stdString) : PQString(stdString)
{
}

OQString::OQString(const std::wstring &wString) : PQString(wString)
{
}

OQString::~OQString()
{
}

OQString OQString::operator+(const OQString& rhs)
{
    QString temp(itsString);
    temp.append(rhs.itsString);
    return temp;
}

OQString OQString::operator+(const PQString& rhs)
{
    QString temp(itsString);
    temp.append(rhs.getString());
    return temp;
}

OQString OQString::operator+(const QString& rhs)
{
    QString temp(itsString);
    temp.append(rhs);
    return temp;
}

OQString OQString::operator+(const QChar& rhs)
{
    QString temp(itsString);
    temp.append(rhs);
    return temp;
}

void OQString::operator+=(const OQString& rhs)
{
    itsString.append(rhs.itsString);
}

OQString& OQString::operator=(const OQString& rhs)
{
    if (this == &rhs)
        return *this;
    itsString = rhs.itsString;
    return *this;
}

OQString& OQString::operator=(const PQString& rhs)
{
    if (this == &rhs)
        return *this;
    itsString = QString(rhs.getString());
    return *this;
}

OQString& OQString::operator=(const QString& rhs)
{
    itsString = rhs;
    return *this;
}

OQString& OQString::operator=(const QChar& rhs)
{
    itsString = QString(rhs);
    return *this;
}

bool OQString::operator==(const OQString& rhs)
{
    if (itsString.compare(rhs.itsString, Qt::CaseSensitive) == 0)
        return true;
    else
        return false;
}

bool OQString::operator!=(const OQString &rhs)
{
    if (itsString.compare(rhs.itsString, Qt::CaseSensitive) == 0)
        return false;
    else
        return true;
}

OQString OQString::readURLparameter(QString param1, QString param2)
{
    int startPosition, endPosition;
    OQString result;
    QString target(param1), tempString;

    tempString = itsString;
    if (tempString.right(1) == QString("/"))
        tempString.chop(1);

    if (param1 == QString("LAST"))
    {
        int position = tempString.lastIndexOf(param2);
        result = tempString.right(tempString.length() - (position + 1));
    }
    else
    {
        startPosition = tempString.lastIndexOf(target);
        if (startPosition >= 0)
        {
            startPosition += target.length();
            if (param2.length() == 0)
                endPosition = tempString.indexOf("&", startPosition);
            else
                endPosition = tempString.indexOf(param2, startPosition);
            if (endPosition < 0)
            {
                endPosition = tempString.indexOf("|", startPosition);
                if (endPosition < 0)
                    endPosition = tempString.length();
            }
            result = tempString.mid(startPosition, endPosition - startPosition);
        }
    }

    return result;
}

OQString OQString::pullOutWord(unsigned int startingPosition)
{
    // Word includes the trailing space, if any, to a max of 1

    int index;
    int startPosition = static_cast<int>(startingPosition);
    OQString word;
    QString space(" ");

    if (startPosition >= itsString.length())
        return word;

    index = itsString.indexOf(space, startPosition);

    if (index == -1)
        word = itsString.right(itsString.length() - startPosition);
    else
        word = itsString.mid(startPosition, index - startPosition + 1);

    return word;
}

OQString OQString::pullOutSentence(unsigned int startingPosition)
{
    unsigned int wordLength;
    OQString sentence, word, lastChar;
    unsigned int charValue;
    std::wstring singleChar;
    OQString period(".");
    QString space(" ");
    bool endingCharFound;

    bool finished = false;

    while (!finished)
    {
        word = pullOutWord(startingPosition);
        wordLength = word.getLength();
        if (wordLength == 0)
            finished = true;
        else
        {
            word.removeEnding(space);
            sentence += space;
            sentence += word;
            startingPosition += wordLength;

            // Check to see if the word ends a sentence
            lastChar = word.right(1);
            singleChar = lastChar.getWString();
            charValue = static_cast<unsigned int>(singleChar[0]);

            // Look for exclamation mark (33), period (46) or question mark (63)
            endingCharFound = ((charValue == 33) || (charValue == 46) || (charValue == 63));
            if (endingCharFound)
            {
                if ((charValue == 33) || (charValue == 63))
                    finished = true;
                else    // charvalue == 46 or period
                {
                    // Don't end sentence for known abbreviations
                    if (word.isPrefix() || word.isSuffix() || word.isSaint()  || word.isTitle() || word.isAbbreviation())
                    {
                        // Don't end sentence and keep going
                    }
                    else
                    {
                        // Check if the word is an initial(s)
                        if ((word.getLength() == 2) || ((word.getLength() == 4) && (word.middle(1,1) == period)))
                        {
                            // Don't end sentence here either
                        }
                        else
                            finished = true;
                    }
                }
            }
        }
    }

    sentence.removeLeading(space);

    return sentence;
}

QString OQString::convertToID()
{
    simplify();
    QString tempString = itsString;
    tempString.replace(" ", "_");
    tempString.replace(",", "$");
    tempString.replace("/", "#");

    return tempString;
}

QString OQString::convertFromID()
{
    QString tempString = itsString;
    tempString.replace("_", " ");
    tempString.replace("$", ",");
    tempString.replace("#", "/");

    return tempString;
}

OQString& OQString::tidyUp()
{
    itsString.replace("(-)", "");
    cleanUpEnds();
    removeEnding("-");
    cleanUpEnds();
    removeEnding("-");
    cleanUpEnds();

    return *this;
}

QList<OQString> OQString::createList() const
{
    QList<OQString> resultList;
    OQString interimWord, finalWord, singleChar;
    unsigned int length = getLength();
    bool EOS;

    for (unsigned int i = 0; i < length; i++)
    {
        EOS = (i == (length -1 ));
        singleChar = itsString[i];
        if ((singleChar.isAlpha() || singleChar.isSingleQuote()) && (singleChar != OQString(" ")))
        {
            interimWord += singleChar;
            if (EOS)
            {
                finalWord += interimWord;
                if (finalWord.getLength() > 0)
                    resultList.append(finalWord);
            }
        }
        else
        {
            finalWord += interimWord;
            if (interimWord.isCompoundName() && !EOS)
            {
                finalWord += OQString(" ");
                interimWord.clear();
            }
            else
            {
                if (finalWord.getLength() > 0)
                    resultList.append(finalWord);
                interimWord.clear();
                finalWord.clear();
            }
        }
    }

    return resultList;
}

bool OQString::removeOrdinal(LANGUAGE language)
{
    bool removed = false;

    if (itsString.length() < 2)
        return false;

    if (!isAlpha() && !isNumeric() && isAlphaNumeric())
    {
        OQString ending;
        switch (language)
        {
        case french:
            ending = right(4);
            if (ending.isOrdinal(french))
            {
                dropRight(4);
                removed = true;
            }
            break;

        case spanish:
            ending = right(2);
            if (ending.isOrdinal(spanish))
            {
                dropRight(2);
                removed = true;
            }
            else
            {
                ending = right(1);
                if (ending.isOrdinal(spanish))
                {
                    dropRight(1);
                    removed = true;
                }
            }
            break;

        case english:
            ending = right(2);
            if (ending.isOrdinal(english))
            {
                dropRight(2);
                removed = true;
            }
            break;

        default:
            removed = removeOrdinal(english) || removeOrdinal(french) || removeOrdinal(spanish);
            break;

        } // end switch
    }

    return removed;
}

bool OQString::removeLeadingNeeEtAl(OQString nextWord, LANGUAGE language)
{
    if (nextWord.lower() == OQString("le"))
        return false;
    else
    {
        if ((nextWord.getLength() == 0) && (itsString == "Born"))
            return false;
        else
            return removeLeadingNeeEtAl(language);
    }
}

bool OQString::removeLeadingNeeEtAl(LANGUAGE language)
{
    bool removed = false;
    bool dateFollows = false;
    QString space(" ");
    QString remainingString;
    OQString firstWord, nextWord;

    if (itsString.length() < 3)
        return false;

    int i = itsString.indexOf(space, 0);
    if (i >= 0)
    {
        firstWord = itsString.left(i);              // Excludes space
        remainingString = itsString.mid(i+1, -1);   // Excludes space
    }
    else
        firstWord = itsString;

    firstWord.removeBookEnds(QUOTES);
    firstWord.removeEnding(PUNCTUATION);
    firstWord.removeInternalPeriods();
    firstWord = firstWord.lower();

    // Check if there is a date immediately following the firstWord
    QStringList wordList = remainingString.split(QString(" "), Qt::SkipEmptyParts, Qt::CaseInsensitive);
    if (wordList.size() >= 3)
    {
        nextWord = wordList.at(0);
        nextWord = nextWord.lower();

        switch (language)
        {
        case english:
            if (nextWord.isWrittenMonth())
                dateFollows = true;
            break;

        case french:
            if (nextWord.isNumeric())
                dateFollows = true;
            break;

        case spanish:
            break;

        default:
            // Do nothing
            break;
        }
    }

    if (firstWord.isNeeEtAl(nextWord) && !dateFollows)
    {
        removed = true;
        itsString = remainingString;
    }

	return removed;
}

bool OQString::removeLeadingAKA()
{
    bool removed = false;
    QString space(" ");
    OQString firstWord, remainingString;

    if (itsString.length() < 5)
        return false;

    int i = itsString.indexOf(space, 0);
    if (i >= 0)
    {
        firstWord = itsString.left(i);              // Excludes space
        remainingString = itsString.mid(i+1, -1);   // Excludes space
    }
    else
        firstWord = itsString;

    firstWord.removeBookEnds(QUOTES);
    firstWord.removeEnding(PUNCTUATION);
    firstWord.removeInternalPeriods();
    firstWord.lower();

    if ((firstWord == OQString("aka")) || (firstWord == OQString("dit"))  || (firstWord == OQString("dite")))
    {
        removed = true;
        itsString = remainingString.getString();
    }

    return removed;
}

bool OQString::removeInternalPeriods()
{
    // Doesn't remove leading or ending periods, or a period after leading "st."

    QString period(".");
    QString space(" ");
    if (!itsString.contains(period))
        return false;

    int i, j;
    int start = 1;
    bool removed = false;
    bool keepPeriod = false;
    bool keepGoing = true;
    QString newString = itsString.at(0);
    OQString mid;

    while (keepGoing)
    {
        i = itsString.indexOf(period, start, Qt::CaseInsensitive);

        if ((i == -1) || (i == itsString.size()))
        {
            // No remaining internal periods
            newString += itsString.right(itsString.size() - start);
            keepGoing = false;
        }
        else
        {
            // An internal period exists
            j = itsString.lastIndexOf(QString("s"), i, Qt::CaseInsensitive);
            if (j == -1)
            {
                // No possibility of saint
                keepPeriod = false;
            }
            else
            {
                // Verify if preceded by saint
                mid = OQString(itsString.mid(j, i - j)).lower();
                if (mid.isSaint())
                {
                    if (j == 0)
                        keepPeriod = true;
                    else
                    {
                        QString precedingChar = itsString.at(j - 1);
                        if ((precedingChar == space) || (precedingChar == QString("-")) || (precedingChar == QString("(")))
                            keepPeriod = true;
                        else
                            keepPeriod = false;
                    }
                }
                else
                    keepPeriod = false;
            }

            if (keepPeriod)
                newString += itsString.mid(start, i - start + 1);
            else
            {
                newString += itsString.mid(start, i - start);
                removed = true;
            }
        }
        start = i + 1;
    }

    itsString = newString;
    return removed;
}

bool OQString::removeSpousalReference(const LANGUAGE lang, bool firstSentence)
{
    bool removed = false;

    QList<QString> targetPhrases;
    QString targetText;
    int index, subIndexA, subIndexB, start;

    if (firstSentence)
        start = 5;
    else
        start = 0;

    targetPhrases = getSpousalReferences(lang);

    while (!removed && (targetPhrases.size() > 0))
    {
        targetText = targetPhrases.takeFirst();
        index = itsString.indexOf(targetText, 0, Qt::CaseInsensitive);
        if (index >= start)
        {
            subIndexA = itsString.indexOf(QString(","), index, Qt::CaseInsensitive);
            subIndexB = itsString.indexOf(QString(";"), index, Qt::CaseInsensitive);
            if ((subIndexA > 0) && (subIndexB > 0) && (subIndexB < subIndexA))
                subIndexA = subIndexB;
            if (subIndexA == -1)
                itsString = itsString.left(index);
            else
                itsString = itsString.left(index) + itsString.right(itsString.length() - subIndexA);
            removed = true;
        }
    }

    return removed;
}

void OQString::removeWrittenMonths()
{
    QRegularExpression target;

    target.setPattern("(January|February|March|April|May|June|July|August|September|October|November|December)");
    itsString.replace(target, "");
    target.setPattern("(janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre)");
    itsString.replace(target, "");
    target.setPattern("(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)");
    itsString.replace(target, "");
    target.setPattern("(jan|janv|fév|févr|mars|avr|mai|juin|juil|août|sep|sept|oct|nov|déc)");
    itsString.replace(target, "");

    itsString.replace("  ", " ");
}

void OQString::removeAllSuffixPrefix()
{
    replaceHTMLentities();
    QRegularExpression target;
    target.setPattern("\\b[R|r][E|e][T|t]'[D|d]\\b");
    itsString.replace(target, "");
    target.setPattern("\\b[P|p]\\.?\\s?[E|e][N|n]'[G|g]\\b");
    itsString.replace(target, "");

    removeSuffixPrefix(suffixesDropEnglish);
    removeSuffixPrefix(prefixesAbbreviatedEnglish);
    removeSuffixPrefix(prefixesAbbreviatedFrench);

    if (itsString.contains("Honourable", Qt::CaseSensitive) || itsString.contains("Honorable", Qt::CaseSensitive))
    {
        itsString.replace("Honourable ", "", Qt::CaseSensitive);
        itsString.replace("Honorable ", "", Qt::CaseSensitive);
        itsString.replace("The ", "", Qt::CaseSensitive);
        itsString.replace("Right ", "", Qt::CaseSensitive);
    }

    itsString.replace("\\(|\\)", "");
    itsString.replace("''", "");
    itsString.replace("  ", " ");

    target.setPattern("\\(,");
    itsString.replace(target, "(");
    target.setPattern(",\\)");
    itsString.replace(target, ")");
    target.setPattern("\\(\\.");
    itsString.replace(target, "(");
    target.setPattern("\\.\\)");
    itsString.replace(target, ")");
    target.setPattern("\\(\\s+?\\)");
    itsString.replace(target, "");
}

void OQString::removeSuffixPrefix(const QList<QString> &list)
{
    QRegularExpression target;
    target.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);
    QString tgt, pattern, upperLowerChar, singleChar;
    QList<QString> targets = list;
    sort(targets, false, false, false, true);  // sort by decreasing length

    while (targets.size() > 0)
    {
        tgt = targets.takeFirst();

        if ((tgt.length() > 1) && (tgt != QString("fr")))
        {
            pattern = QString("\\b");
            for (int i = 0; i < tgt.size(); i++)
            {
                singleChar = tgt.at(i);
                upperLowerChar = QString("[") + singleChar.toUpper() + singleChar.toLower() + QString("]");
                pattern +=  upperLowerChar + QString("\\.?\\s?");
            }
            pattern += QString("\\b");

            target.setPattern(pattern);
            itsString.replace(target, "");
        }
    }

    itsString.replace("  ", " ");
    itsString.replace(",,", ",");
    itsString.replace(".,", ",");
    itsString.replace("( ", "(");
    itsString.replace(" )", ")");
    itsString.replace("()", "");
    itsString.replace("(.)", "");
    itsString.replace("(,)", "");
    itsString.replace(", ,", ",");
    itsString.replace(" ,", ",");
    itsString.replace("\"\"", "");
    removeLeading(",");
    removeLeading(" ");
}

void OQString::removeStrings(const QList<QString> &list)
{
    QRegularExpression target;
    target.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);
    QString tgt, pattern;
    QList<QString> targets = list;

    while (targets.size() > 0)
    {
        tgt = targets.takeFirst();

        if (tgt.length() > 1)
        {
            pattern = QString("\\b") + tgt + QString("\\b");
            target.setPattern(pattern);
            itsString.replace(target, "");
        }
    }

    itsString.replace("  ", " ");
    removeLeading(" ");
}

void OQString::removeIntroductions()
{
    QRegularExpression target;
    target.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

    target.setPattern("in memory[ of]?\\s?[:]?\\s?");
    itsString.replace(target, "");

    target.setPattern("\\bView\\s\\b");
    itsString.replace(target, "");

    target.setPattern("\\bWatch\\s\\b");
    itsString.replace(target, "");

    cleanUpEnds();
    itsString.replace("  ", " ");
}

void OQString::compressCompoundNames(const LANGUAGE lang)
{
    OQString result, word, singleChar, nextChar, cleanWord, nextWord, endHyphenatedName;
    QString space(" ");
    bool isCompound, hasParentheses, hasQuotes, openEnded;
    unsigned int charType;
    int start = 0;
    int end = 0;
    int last = itsString.length();

    if ((last == 0) || (lang == french))
        return;

    while (end < last)
    {
        singleChar = itsString.mid(start, 1);
        hasParentheses = ((singleChar.getCharType() & PARENTHESES) == PARENTHESES);
        hasQuotes = ((singleChar.getCharType() & QUOTES) == QUOTES);
        if (hasParentheses || hasQuotes)
        {
            if (hasParentheses)
                end = itsString.indexOf(QString(")"), start + 1, Qt::CaseInsensitive);
            else
                end = itsString.indexOf(QString("\'"), start + 1, Qt::CaseInsensitive);

            if (end == -1)
            {
                end = last;
                openEnded = true;
            }
            else
            {
                end++;
                openEnded = false;
            }

            word = itsString.mid(start, end - start);
            cleanWord = word;
            cleanWord.removeBookEnds(PARENTHESES | QUOTES, true);
            if (!openEnded)
                cleanWord.compressCompoundNames(lang);

            if (hasParentheses)
            {
                result += OQString("(") + cleanWord;
                if (!openEnded)
                    result += OQString(")");
                if ((end < last) && space == itsString.at(end))
                    result += space;
            }
            else
            {
                result += OQString("\'") + cleanWord;
                if (!openEnded)
                    result += OQString("\'");
                if ((end < last) && space == itsString.at(end))
                    result += space;
            }

            while ((end < last) && (space == itsString.at(end)))
            {
                end++;
            }
            start = end;
        }
        else
        {
            end = itsString.indexOf(space, start, Qt::CaseInsensitive);
            if (end == -1)
                end = last;
            word = itsString.mid(start, end - start);
            cleanWord = word;
            if (word.isHyphenated())
            {
                int index = word.getString().indexOf(QString("-"));
                endHyphenatedName = word.right(word.getLength() - index - 1);
            }
            else
                endHyphenatedName.clear();
            isCompound = cleanWord.isCompoundName() || cleanWord.isSaint() || endHyphenatedName.isCompoundName();

            // Deal with special cases
            // Error will occur with nicknames "Di" and "Von" if they are not in quotes or parentheses
            // Assess if stand alone "e" is an initial or compound name
            if (isCompound && ((cleanWord == OQString("E")) || (cleanWord == OQString("E."))))
            {
                OQString temp(itsString);
                if (!temp.isAllCaps())
                    isCompound = false;
            }

            if (isCompound)
                word = word.proper();

            if (isCompound && ((word.lower() == OQString("st")) || (word.lower() == OQString("ste"))))
                word += OQString(".");

            result += word;
            start = end + 1;

            if (end < last)
            {
                // Need to obtain next word for aboriginal check and other exceptions
                bool cancel;
                int tempEnd = itsString.indexOf(space, start, Qt::CaseInsensitive);
                if (tempEnd == -1)
                    tempEnd = last;
                nextWord = itsString.mid(start, tempEnd - start);
                cancel = nextWord.removeBookEnds(PARENTHESES | QUOTES, true);
                cancel = cancel || nextWord.removeLeading(PARENTHESES | QUOTES);
                cancel = cancel || (nextWord.lower() == PQString("obituary"));
                cancel = cancel || (isCompound && word.isCapitalized() && !nextWord.isCapitalized());
                cancel = cancel || word.isSingleVan();

                if (cancel || !(isCompound || cleanWord.isAboriginalName(nextWord)))
                    result += space;
            }
        }
    }

    itsString = result.getString();

    // Second pass through to get consistent capitalizations
    start = 0;
    end = 0;
    result.clear();
    last = itsString.length();

    while (end < last)
    {
        end = itsString.indexOf(space, start, Qt::CaseInsensitive);
        if (end == -1)
            end = last;
        word = itsString.mid(start, end - start);

        singleChar = word.left(1);
        charType = singleChar.getCharType();
        hasParentheses = ((charType & PARENTHESES) == PARENTHESES);
        hasQuotes = ((charType & QUOTES) == QUOTES);
        cleanWord = word;

        if (hasParentheses || hasQuotes)
        {
            cleanWord.removeLeading(PARENTHESES | QUOTES);
            cleanWord = cleanWord.proper();
            word = singleChar + cleanWord;
        }
        else
        {
            if (cleanWord.isCapitalized())
                word = cleanWord.proper();
        }
        result += word;
        start = end + 1;
        if (end < last)
            result += space;
    }

    itsString = result.getString();
}

void OQString::conditionalBreaks()
{
    // Analyzes a string with many conditional breaks to determine if the bulk should either be treated as
    // sentence breaks or long sentences with forced formatting breaks.

    QChar conditionalBreak(QChar(65533));

    QString unspaced(conditionalBreak);
    QString spaced = QString(" ") + unspaced + QString(" ");
    itsString.replace(unspaced, spaced);

    int index = itsString.indexOf(conditionalBreak);

    if (index == -1)
        return;

    int start, end, numBreaks, numPeriods, nextBreakIndex;
    bool periodCounted;
    bool insertsPeriods = false;
    bool process = false;

    numBreaks = 1;
    if (index < 100)
        index = 100;
    start = index;

    while (index != -1)
    {
        index = itsString.indexOf(conditionalBreak, index + 1);
        if (index != -1)
            numBreaks++;
    }

    if (numBreaks >= 4)
    {
        process = true;
        end = itsString.lastIndexOf(conditionalBreak);
        if ((itsString.size() - end) < 10)
        {
            end = itsString.lastIndexOf(conditionalBreak, end - 1);
            numBreaks--;
        }

        index = start;
        nextBreakIndex = itsString.indexOf(conditionalBreak, start);
        if (nextBreakIndex == -1)
            nextBreakIndex = itsString.length();
        numPeriods = 0;
        periodCounted = false;

        // Count number of periods, max of 1 between any two conditional breaks
        while (index != -1)
        {
            index = itsString.indexOf(QString("."), index + 1);
            if (index > nextBreakIndex)
            {
                if (!periodCounted)
                    numPeriods++;
                nextBreakIndex = itsString.indexOf(conditionalBreak, index);
                if (nextBreakIndex == -1)
                    nextBreakIndex = itsString.length();
                periodCounted = false;
            }
            else
            {
                if ((index != -1) && !periodCounted)
                {
                    numPeriods++;
                    periodCounted = true;
                }
            }
        }
    }

    if (process)
    {
        if ((numPeriods == 0) || (((end - start) / numPeriods) < 75))
            insertsPeriods = true;

        index = start - 1;
        while ((index < end) && (index != -1))
        {
            index = itsString.indexOf(conditionalBreak, index + 1);
            if (index != -1)
            {
                if (insertsPeriods)
                    itsString.replace(index, 1, QChar(46));
                else
                    itsString.remove(index, 1);
            }
        }
    }
}

void OQString::fixBasicErrors(bool onlyContainsNames)
{
    // Need to be careful in types of fixes as this can easily break prior working code
    QRegularExpression targetS, targetI;
    targetI.setPatternOptions(QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
    targetS.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);
    QRegularExpressionMatch match;
    int index;

    // Remove double characters
    index = itsString.indexOf(QChar(769));
    if (index != -1)
    {
        if ((index > 0) && (itsString.at(index - 1) == QChar('e')))
        {
            QString newString(itsString.left(index - 1));
            newString += "é";
            newString += itsString.right(itsString.length() - index - 1);
            itsString = newString;
        }
    }

    // Fix encoding issues within website (i.e. website doesn't display properly)
    itsString.replace(QString(" ï¿½ "), "-", Qt::CaseInsensitive);

    targetS.setPattern("([A-Z]|[a-z])nï¿½e");
    itsString.replace(targetS, "\\1ée");

    targetS.setPattern(" ï¿½(\\w+)ï¿½");
    itsString.replace(targetS, " \"\\1\"");

    // Replace problematic chars
    itsString.replace(QString("œ"), "oe", Qt::CaseInsensitive);
    targetS.setPattern("\\b1/2\\b");
    itsString.replace(targetS, "");
    itsString.replace(QChar(173), QString(""));  // Soft hyphen

    // Remove all "RR #" references
    targetS.setPattern(" RR [0-9]+");
    itsString.replace(targetS, " ");

    // Remove problematic degrees
    targetS.setPattern("\\,? Ph\\.?\\s?D\\.?,?\\b");
    itsString.replace(targetS, " ");
    targetS.setPattern("\\,? O\\.?\\s?C\\.?,?\\b");
    itsString.replace(targetS, " ");

    // Remove the leading "le" from all French dates
    targetS.setPattern("\\ble ([0-9]{2} )");
    itsString.replace(targetS, " \\1");

    // Replace January 1, 1983 to June 8, 2020 to hyphenated
    targetS.setPattern("(, [0-9]{4} )(to)( [A-Z])");
    itsString.replace(targetS, "\\1-\\3");

    // Fix malformed ordinals and then remove all
    targetS.setPattern("(\\d) (st|nd|rd|th)( |,)");
    itsString.replace(targetS, "\\1 ");
    targetS.setPattern("(\\d) (ième|ieme|ière|iere)( |,)");
    itsString.replace(targetS, "\\1 ");
    targetS.setPattern("(\\d) (st|nd|rd|th) ");
    itsString.replace(targetS, "\\1 ");
    targetS.setPattern("(\\d) (e|er|ième|ieme|ière|iere) ");
    itsString.replace(targetS, "\\1 ");
    targetS.setPattern("(\\d)(st|nd|rd|th) ");
    itsString.replace(targetS, "\\1 ");
    targetS.setPattern("(\\d)(e|er|ième|ieme|ière|iere) ");
    itsString.replace(targetS, "\\1 ");

    // Remove written days of the week
    QStringList writtenDays = QString("Sunday|Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|dimanche|lundi|mardi|mecredi|jeudi|vendredi|samedi").split("|");
    while (writtenDays.count() >= 1)
    {
        QString writtenDay;
        writtenDay = writtenDays.takeFirst();
        targetI.setPattern(writtenDay + QString(",?"));
        itsString.replace(targetI, "");
    }

    // Remove double spaces - Interim run as prior additions mess up other changes below
    targetS.setPattern("  ");
    itsString.replace(targetS, " ");

    // Remove unnecessary words within dates
    itsString.replace(QString(" day of "), QString(" "), Qt::CaseSensitive);
    itsString.replace(QString(" jour du mois de "), QString(" "), Qt::CaseInsensitive);
    itsString.replace(QString(" jour de "), QString(" "), Qt::CaseInsensitive);

    // Ensure all french dates are lower case
    targetS.setPattern(QString("\\b(Janvier|Fevrier|Février|Mars|Avril|Mai|Juin|Juillet|Aout|Août|Septembre|Octobre|Novembre|Decembre|Décembre)\\b"));
    match = targetS.match(itsString);
    while (match.hasMatch())
    {
        QString capturedWord = match.captured(1);
        QString replacementWord = capturedWord.toLower();
        itsString.replace(capturedWord, replacementWord);
        match = targetS.match(itsString);
    }

    // Eliminate all cap English months
    targetS.setPattern(QString("\\b(JANUARY|FEBRUARY|MARCH|APRIL|MAY|JUNE|AUGUST|SEPTEMBER|OCTOBER|NOVEMBER|DECEMBER)\\b"));
    match = targetS.match(itsString);
    while (match.hasMatch())
    {
        QString capturedWord = match.captured(1);
        QString replacementWord = capturedWord.left(1).toUpper() + capturedWord.mid(1).toLower();
        itsString.replace(capturedWord, replacementWord);
        match = targetS.match(itsString);
    }

    // Standardize abbreviated dates
    targetS.setPattern(QString("(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec|jan|janv|fév|févr|mar|avr|juil|sept|oct|nov|déc)\\.?\\s?(\\d)"));
    itsString.replace(targetS, "\\1 \\2");

    // Address unusual date format
    targetS.setPattern(" of ([1|2][0|9][0-9][0-9])");
    itsString.replace(targetS, " \\1");

    // Fix malformed dates
    targetS.setPattern("\\b(January|February|March|April|May|June|July|August|September|October|November|December) (\\d\\d?)\\.\\s?([1-2][0|9][0-9][0-9])\\b");  // period instead of comma
    itsString.replace(targetS, "\\1 \\2, \\3");
    targetS.setPattern("\\b(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec) (\\d\\d?)\\.\\s?([1-2][0|9][0-9][0-9])\\b");  // period instead of comma
    itsString.replace(targetS, "\\1 \\2, \\3");
    targetS.setPattern("\\.( \\d\\d?, [1-2][0|9][0-9][0-9])"); // Extra period of month
    itsString.replace(targetS, "\\1");
    targetS.setPattern(" in ([1-2][0|9][0-9][0-9])"); // born on January 11 in 1973
    itsString.replace(targetS, " \\1");
    targetS.setPattern("( \\d\\d?,)([1-2][0|9][0-9][0-9])"); // Missing space
    itsString.replace(targetS, "\\1 \\2");
    targetS.setPattern("(, [1-2][0|9][0-9][0-9])(\\S)"); // Missing space #2
    itsString.replace(targetS, "\\1 \\2");
    targetS.setPattern("([A-Z]|[a-z]|\\S)(January|February|March|April|May|June|July|August|September|October|November|December)\\s?(\\d\\d?)\\,\\s?([1-2][0|9][0-9][0-9])");  // Missing space #3
    itsString.replace(targetS, "\\1 \\2 \\3, \\4");
    //targetS.setPattern("(A-Z|a-z|\\s)(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec)\\.?\\s?(\\d\\d?)\\,\\s?([1-2][0|9][0-9][0-9])");  // Missing space #4, #5, #6
    targetS.setPattern("([A-Z]|[a-z]|\\S)(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec)\\.?\\s?(\\d\\d?)\\,\\s?([1-2][0|9][0-9][0-9])");  // Missing space #4, #5, #6
    itsString.replace(targetS, "\\1 \\2 \\3 \\4");
    targetS.setPattern(" (\\d\\d?)\\.\\s?([1-2][0|9][0-9][0-9])"); // Period instead of comma, or missing comma
    itsString.replace(targetS, " \\1, \\2");
    targetS.setPattern(" (\\d\\d?) ([1-2][0|9][0-9][0-9])"); // Missing comma
    itsString.replace(targetS, " \\1, \\2");
    targetS.setPattern("(\\d) of (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec)");
    itsString.replace(targetS, "\\1 \\2");
    targetS.setPattern("(January|February|March|April|May|June|July|August|September|October|November|December)(\\d)");
    itsString.replace(targetS, "\\1 \\2");
    targetS.setPattern("(janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre)(\\d)");
    itsString.replace(targetS, "\\1 \\2");
    targetS.setPattern("([1-2][0|9][0-9][0-9])(\\s?[-|~]\\s?)(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec)");
    itsString.replace(targetS, "\\1 - \\3");
    targetS.setPattern("([1-2][0|9][0-9][0-9])(\\s?[-|~]\\s?)(January|February|March|April|May|June|July|August|September|October|November|December)");
    itsString.replace(targetS, "\\1 - \\3");
    /*targetS.setPattern("([1-2][0|9][0-9][0-9])-(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec)");
    itsString.replace(targetS, "\\1 - \\2");
    targetS.setPattern("([1-2][0|9][0-9][0-9]) -(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec)");
    itsString.replace(targetS, "\\1 - \\2");
    targetS.setPattern("([1-2][0|9][0-9][0-9])- (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept||Oct|Nov|Dec)");
    itsString.replace(targetS, "\\1 - \\2");*/

    // English dates in French format
    // Must avoid converting French dates written with capitals
    //targetI.setPattern("\\b(janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre|janv|fév|févr|mar|avr|juil|sep|déc)\\b");
    targetI.setPattern("(\\d\\d?)(\\s|-)?(janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre|janv|fév|févr|avr|juil|déc)(\\s|-)?([1-2][0|9][0-9][0-9])");
    match = targetI.match(itsString);
    if (!match.hasMatch())
    {
        targetS.setPattern("(\\d\\d?)\\s?(January|February|March|April|May|June|July|August|September|October|November|December)\\s?([1-2][0|9][0-9][0-9])");
        itsString.replace(targetS, "\\2 \\1, \\3");
        targetS.setPattern("(\\d\\d?)\\s?(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec)\\s?([1-2][0|9][0-9][0-9])");
        itsString.replace(targetS, "\\2 \\1, \\3");
        targetS.setPattern("(\\d\\d?)-(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec)-([1-2][0|9][0-9][0-9])");
        itsString.replace(targetS, "\\2 \\1, \\3");
    }

    // Fix malformed french dates
    targetS.setPattern("(janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre),?\\s+?(\\d{4})");
    itsString.replace(targetS, "\\1 \\2");
    targetS.setPattern("(\\s+\\d+)(janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre)");
    itsString.replace(targetS, "\\1 \\2");
    targetS.setPattern("(\\s+\\d+)(jan|janv|fév|févr|mar|avr|juil|sep|sept|oct|nov|déc)");
    itsString.replace(targetS, "\\1 \\2");
    targetS.setPattern("(janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre) (\\d\\d?),? (\\d{4})");
    itsString.replace(targetS, "\\2 \\1 \\3");

    // Fix abbreviated yy dates
    targetS.setPattern("\\b(January|February|March|April|May|June|July|August|September|October|November|December) (\\d\\d?)/(0|1|2)(\\d)\\b");
    itsString.replace(targetS, "\\1 \\2, 20\\3\\4");
    targetS.setPattern("\\b(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) (\\d\\d?)/(0|1|2)(\\d)\\b");
    itsString.replace(targetS, "\\1 \\2, 20\\3\\4");
    targetI.setPattern("\\b(janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre) (\\d\\d?)/(0|1|2)(\\d)\\b");
    itsString.replace(targetI, "\\1 \\2, 20\\3\\4");
    targetI.setPattern("\\b(jan|janv|fév|févr|mars|avr|mai|juin|juil|août|sep|sept|oct|nov|déc) (\\d\\d?)/(0|1|2)(\\d)\\b");
    itsString.replace(targetI, "\\1 \\2, 20\\3\\4");
    targetS.setPattern("\\b(January|February|March|April|May|June|July|August|September|October|November|December) (\\d\\d?)/(\\d\\d)\\b");
    itsString.replace(targetS, "\\1 \\2, 19\\3");
    targetS.setPattern("\\b(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) (\\d\\d?)/(\\d\\d)\\b");
    itsString.replace(targetS, "\\1 \\2, 19\\3");
    targetI.setPattern("\\b(janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre) (\\d\\d?)/(\\d\\d)\\b");
    itsString.replace(targetI, "\\1 \\2, 19\\3");
    targetS.setPattern("\\b(jan|janv|fév|févr|mars|avr|mai|juin|juil|août|sep|sept|oct|nov|déc) (\\d\\d?)/(\\d\\d)\\b");
    itsString.replace(targetS, "\\1 \\2, 19\\3");

    // Standardize french dates and strip out the "le"
    targetS.setPattern("(\\b[Ll]e\\s+)?(\\d+ )(janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre),? (\\d{4})");
    itsString.replace(targetS, "\\2\\3 \\4");

    // Flip reversed written dates
    targetS.setPattern("(\\d{4}) (January|February|March|April|May|June|July|August|September|October|November|December) (\\d\\d?) - (\\d{4}) (January|February|March|April|May|June|July|August|September|October|November|December) (\\d\\d?)");
    itsString.replace(targetS, "(\\2 \\3, \\1 - \\5 \\6, \\4)");
    targetS.setPattern("(\\d{4}) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec) (\\d\\d?) - (\\d{4}) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec) (\\d\\d?)");
    itsString.replace(targetS, "(\\2 \\3, \\1 - \\5 \\6, \\4)");

    // Put parentheses around DOB - DOD - FIRST IMPLEMENTATION run before removing locations due to "May 1, 1960 - June 1, 2021 On June 1, 2021 Jeff ...." being caught in parentheses at end
    // Also combine any back to back dates missing hyphen
    targetS.setPattern("(January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})( - | )(January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})");
    itsString.replace(targetS, "(\\1 \\2 - \\4 \\5)");
    targetS.setPattern("(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec) (\\d+, \\d{4})( - | )(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec) (\\d+, \\d{4})");
    itsString.replace(targetS, "(\\1 \\2 - \\4 \\5)");
    targetS.setPattern("(\\d+) (janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre) (\\d{4})( - | )(\\d+) (janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre) (\\d{4})");
    itsString.replace(targetS, "(\\1 \\2 \\3 - \\5 \\6 \\7)");
    targetS.setPattern("(\\d+) (jan|janv|fév|févr|mars|avr|mai|juin|juil|août|sep|sept|oct|nov|déc) (\\d{4})( - | )(\\d+) (jan|janv|fév|févr|mars|avr|mai|juin|juil|août|sep|sept|oct|nov|déc) (\\d{4})");
    itsString.replace(targetS, "(\\1 \\2 \\3 - \\5 \\6 \\7)");
    itsString.replace("( ", "(");
    itsString.replace(" )", ")");
    itsString.replace("((", "(");
    itsString.replace("))", ")");
    itsString.replace(") -", ")");
    itsString.replace("- (", "(");

    // Strip out locations within DOB locationA - DOD locationB
    targetS.setPattern("(January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})(\\s*-?\\s*\\b[A-Z]\\w{1,15},? [^a-z0-9]\\w{0,15}\\s*)(January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})(\\s*-?\\s*\\w{1,15},? \\w{0,15}\\s*)");
    itsString.replace(targetS, "(\\1 \\2 - \\4 \\5) ");
    targetS.setPattern("(January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})(January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})(\\s*-?\\s*\\w{1,15},? \\w{0,15}\\s*)");
    itsString.replace(targetS, "(\\1 \\2 - \\3 \\4) ");
    targetS.setPattern("(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec) (\\d+, \\d{4})(\\s*-?\\s*\\w{1,15},? \\w{0,15}\\s*)(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec) (\\d+, \\d{4})(\\s*-?\\s*\\w{1,15},? \\w{0,15}\\s*)");
    itsString.replace(targetS, "(\\1 \\2 - \\4 \\5) ");
    targetS.setPattern("(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec) (\\d+, \\d{4})(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec) (\\d+, \\d{4})(\\s*-?\\s*\\w{1,15},? \\w{0,15}\\s*)");
    itsString.replace(targetS, "(\\1 \\2 - \\3 \\4) ");

    // Put parentheses around DOB - DOD - SECOND implementation
    targetS.setPattern("(January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4}) - (January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})");
    itsString.replace(targetS, "(\\1 \\2 - \\3 \\4)");
    targetS.setPattern("(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec) (\\d+, \\d{4}) - (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec) (\\d+, \\d{4})");
    itsString.replace(targetS, "(\\1 \\2 - \\3 \\4)");
    targetS.setPattern("(\\d+) (janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre) (\\d{4}) - (\\d+) (janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre) (\\d{4})");
    itsString.replace(targetS, "(\\1 \\2 \\3 - \\4 \\5 \\6)");
    itsString.replace("( ", "(");
    itsString.replace(" )", ")");
    itsString.replace("((", "(");
    itsString.replace("))", ")");
    itsString.replace("),(", "), (");

    // Fix poorly formatted dividers
    targetS.setPattern(" -([1-9])"); // No space in front of number
    itsString.replace(targetS, " - \\1");
    targetS.setPattern("([0-9])- "); // No space after number
    itsString.replace(targetS, "\\1 - ");

    // Fix missing spaces
    targetS.setPattern("\\s+(\\d+)(year|an)s");
    itsString.replace(targetS, " \\1 \\2s");
    targetS.setPattern("\\s+de(\\d+)\\s+");
    itsString.replace(targetS, " de \\1 ");

    // Remove space in "O' Hara"
    targetS.setPattern("O' ([A-Z])");
    itsString.replace(targetS, "O'\\1");
    targetS.setPattern("O\" ([A-Z])");
    itsString.replace(targetS, "O'\\1");
    targetS.setPattern("O\"([A-Z])");
    itsString.replace(targetS, "O'\\1");

    // Remove space or hypen in "Mc Donald"
    targetS.setPattern("\\b(M[A|a]?[C|c])[ |-]([A-Z])");
    itsString.replace(targetS, "\\1\\2");

    // Remove hyphen in "pre-deceased"
    targetS.setPattern("([P]|[p])re-([D]|[d])eceased");
    itsString.replace(targetS, "\\1redeceased");

    // Replce problematic "parents-in-law"
    itsString.replace("parents-in-law", "outlaws", Qt::CaseInsensitive);

    // Remove all instances of "loving" within "his loving wife"
    itsString.replace(QString("loving "), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString("bien-aimée "), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString("bien aimée "), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString("bien-aimé "), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString("bien aimé "), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString("adorée "), QString(""), Qt::CaseInsensitive);

    // Remove plural references
    itsString.replace(QString("daughters"), QString("daughter"), Qt::CaseInsensitive);
    itsString.replace(QString(" sons"), QString(" son"), Qt::CaseInsensitive);
    itsString.replace(QString("sisters"), QString("sister"), Qt::CaseSensitive);
    itsString.replace(QString("brothers"), QString("brother"), Qt::CaseSensitive);
    itsString.replace(QString("siblings"), QString("sibling"), Qt::CaseInsensitive);
    itsString.replace(QString("nieces"), QString("niece"), Qt::CaseInsensitive);
    itsString.replace(QString("nephews"), QString("nephew"), Qt::CaseInsensitive);
    itsString.replace(QString("children"), QString("child"), Qt::CaseInsensitive);
    itsString.replace(QString("announces"), QString("announce"), Qt::CaseInsensitive);

    // Standardize other decendant references
    itsString.replace(QString("grandkids"), QString("child"), Qt::CaseInsensitive);

    // Standardize references
    itsString.replace(QString("courageously"), QString("courageous"), Qt::CaseInsensitive);
    itsString.replace(QString("battling"), QString("battle"), Qt::CaseInsensitive);
    itsString.replace(QString("battled"), QString("battle"), Qt::CaseInsensitive);
    itsString.replace(QString("struggling"), QString("battle"), Qt::CaseInsensitive);
    itsString.replace(QString("fighting"), QString("battle"), Qt::CaseInsensitive);
    itsString.replace(QString("fought"), QString("battle"), Qt::CaseInsensitive);

    // Remove all instances of "sudden" within "his sudden passing"
    itsString.replace(QString("sudden "), QString(""), Qt::CaseInsensitive);

    // Remove all instances of "dearly" within "missed dearly by" and "remembered fondly by"
    itsString.replace(QString("dearly "), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString("fondly "), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString("with love "), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString("with much love "), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString("beloved "), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString("every day "), QString(""), Qt::CaseInsensitive);

    // Address age at death issues
    targetS.setPattern("(\\d) (long|hard) ");
    itsString.replace(targetS, "\\1 ");

    // Remove partial year references
    itsString.replace(QString(" and a half "), QString(" "), Qt::CaseInsensitive);
    itsString.replace(QString(" and ½ "), QString(" "), Qt::CaseInsensitive);
    itsString.replace(QChar(189), QString(""));
    targetS.setPattern("(\\b\\d\\d+?( years| year)?) and \\d\\d? (months|month)( old)?");
    itsString.replace(targetS, "\\1 years old");
    targetS.setPattern("(\\b\\d\\d+?( ans)?) et \\d\\d? mois");
    itsString.replace(targetS, "\\1 ans");

    // Remove all Covid references
    itsString.replace(QString("covid-19"), QString("covid"), Qt::CaseInsensitive);
    itsString.replace(QString("covid19"), QString("covid"), Qt::CaseInsensitive);
    itsString.replace(QString("covid 19"), QString("covid"), Qt::CaseInsensitive);

    // Remove problematic "heavy hearts" due to aboriginal and compound impact
    itsString.replace(QString("heavy hearts "), QString(""), Qt::CaseInsensitive);

    // Remove other problematic filler words
    itsString.replace(QString(" and best friend"), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString(" and soul mate"), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString(" and soulmate"), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString(" many "), QString(" "), Qt::CaseSensitive);
    itsString.replace(QString(" may be "), QString(" "), Qt::CaseSensitive);
    itsString.replace(QString(" will be "), QString(" "), Qt::CaseSensitive);
    itsString.replace(QString(" late "), QString(" "), Qt::CaseSensitive);
    itsString.replace(QString(" feu "), QString(" "), Qt::CaseSensitive);
    itsString.replace(QString(" hard "), QString(" "), Qt::CaseSensitive);
    itsString.replace(QString(" fought "), QString(" "), Qt::CaseSensitive);
    itsString.replace(QString(" declining "), QString(" "), Qt::CaseSensitive);
    //itsString.replace(QString(" de monsieur "), QString(" de "), Qt::CaseInsensitive);
    //itsString.replace(QString(" de madame "), QString(" de "), Qt::CaseInsensitive);
    itsString.replace(QString(" de dame "), QString(" de "), Qt::CaseInsensitive);
    targetS.setPattern("( de )(M|m)(onsieur|adame|ademoiselle|me|lle)?(\\.? )");
    itsString.replace(targetS,"\\1");

    // Fix problematic English word combinations
    targetS.setPattern("([M|m]emorial) ([A-Z]|[a-z])");
    itsString.replace(targetS, "\\1\\2");
    itsString.replace(QString("memorialis "), QString("memorial is "), Qt::CaseInsensitive);
    itsString.replace(QString("memorialwill "), QString("memorial will "), Qt::CaseInsensitive);

    // Fix problematic French word combinations
    itsString.replace(QString("décès de "), QString("décès "), Qt::CaseInsensitive);
    itsString.replace(QString(" mort de "), QString(" mort "), Qt::CaseInsensitive);
    itsString.replace(QString(" le regret "), QString(" leregret "), Qt::CaseInsensitive);
    itsString.replace(QString("services sociaux"), QString("servicesSociaux"), Qt::CaseInsensitive);
    itsString.replace(QString("centre de service"), QString("centreDeService"), Qt::CaseInsensitive);
    itsString.replace(QString("beau-frère"), QString("beauffrrère"), Qt::CaseInsensitive);
    itsString.replace(QString("beaux-frère"), QString("beauffrrère"), Qt::CaseInsensitive);

    // Remove problematic French words
    targetS.setPattern("([Ss]ecteur\\s+\\w+\\s?)");
    itsString.replace(targetS, "");
    targetS.setPattern("\\b[D|d]it\\b");
    itsString.replace(targetS, "aka");
    itsString.replace(QString("commémoration"), "", Qt::CaseInsensitive);

    // Remove problematic funeral home names
    targetI.setPattern ("brothers'? funeral home");
    itsString.replace(targetI, "");

    // Fix French grammatical challenges
    targetS.setPattern("d'(A|E|I|O|U|H])");
    itsString.replace(targetS, "de \\1");
    targetS.setPattern("l'(A|E|I|O|U|H])");
    itsString.replace(targetS, "les \\1");
    /*itsString.replace(QString("mère d'"), QString("mère de "), Qt::CaseSensitive);
    itsString.replace(QString("père d'"), QString("père de "), Qt::CaseSensitive);
    itsString.replace(QString("femme d'"), QString("femme de "), Qt::CaseSensitive);
    itsString.replace(QString("mari d'"), QString("mari de "), Qt::CaseSensitive);
    itsString.replace(QString("fille d'"), QString("fille de "), Qt::CaseSensitive);
    itsString.replace(QString("fils d'"), QString("fils de "), Qt::CaseSensitive);
    itsString.replace(QString("épouse d'"), QString("épouse de "), Qt::CaseSensitive);
    itsString.replace(QString("époux d'"), QString("époux de "), Qt::CaseSensitive);*/

    // Replace "œ" with "oe"
    itsString.replace(QChar(339), QString("oe"), Qt::CaseInsensitive);

    // Fix née problems
    targetS.setPattern("\\b[N|n][é|è|e][é|è|e]?\\b\\s");
    itsString.replace(targetS,"née ");
    itsString.replace("Irènée", "Irène");  // "è" is treated as a non-word character
    itsString.replace("Hélènée", "Hélène");  // "è" is treated as a non-word character
    targetS.setPattern("\\b[N|n][é|è|e][é|è|e]\\b\\s?[-|:]?");
    itsString.replace(targetS,"née ");
    targetS.setPattern("'née\\s'");
    itsString.replace(targetS,"née");
    targetS.setPattern("\\bnée\\s'");
    itsString.replace(targetS,"née");
    targetS.setPattern("\\b[N|n][é|è|e][é|è|e]\\s(\\w{0,20}, )[N|n][é|è|e][é|è|e]\\s");
    itsString.replace(targetS,"née \\1");

    // Fix hyphenated "Nee" names
    targetS.setPattern("\\b(N|n)(é|e)e\\b\\s?-\\s?([A-Z])");
    itsString.replace(targetS,"Née \\3");

    // Remove unnecessary punctuation starting a list - needed for truncation based on key words
    itsString.replace(QString(" by[:|;|,]"), QString(" by"), Qt::CaseInsensitive);
    itsString.replace(QString(" mourn[:|;|,]"), QString(" mourn"), Qt::CaseInsensitive);
    itsString.replace(QString(" behind[:|;|,]"), QString(" behind"), Qt::CaseInsensitive);
    itsString.replace(QString(" leaves[:|;|,]"), QString(" leaves"), Qt::CaseInsensitive);

    // Replace BRACKETS with PARENTHESES
    itsString.replace(QString("["), QString("("), Qt::CaseInsensitive);
    itsString.replace(QString("]"), QString(")"), Qt::CaseInsensitive);
    itsString.replace(QString("{"), QString("("), Qt::CaseInsensitive);
    itsString.replace(QString("}"), QString(")"), Qt::CaseInsensitive);
    itsString.replace(QChar(171), QString("("), Qt::CaseInsensitive);
    itsString.replace(QChar(187), QString(")"), Qt::CaseInsensitive);

    // Ensure space after comma (fix double space at end) - potential issue with numbers eg. 10,000
    itsString.replace(QString(","), QString(", "));

    // Ensure space after double quote (fix double space at end)
    itsString.replace(QString("\""), QString(" \""));

    // Ensure space before parentheses (fix double space at end)
    itsString.replace(QString("("), QString(" ("));

    // Ensure space after parentheses (fix double space at end)
    itsString.replace(QString(")"), QString(") "));

    // Ensure space after question mark (fix double space at end)
    itsString.replace(QString("?"), QString("? "));

    // Ensure space after exclamation mark (fix double space at end)
    itsString.replace(QString("!"), QString("! "));

    // Ensure space after semi colon (fix double space at end)
    itsString.replace(QString("));"), QString(")); "));

    // Remove unnecessary space after opening parentheses
    itsString.replace(QString("( "), QString("("));

    // Remove unnecessary space before closing parentheses
    itsString.replace(QString(" )"), QString(")"));

    // Remove unnecessary spaces before commas
    itsString.replace(QString(" ,"), QString(","));

    // Remove unnecessary spaces before periods (add space to deal with misplaced periods)
    itsString.replace(QString(" ."), QString(". "));

    // Replace newline character with space
    itsString.replace(QString("\n"), QString(" "));

    // Replace double quotes
    itsString.replace(QString("\'\'"), QString("\'"));

    // Replace underscore and wide divider with hypen
    itsString.replace(QString("_"), QString("-"));
    itsString.replace(QChar(8212), QString(" - "));
    itsString.replace(QString("~"), QString("-"));

    // Remove double spaces
    targetS.setPattern("  ");
    itsString.replace(targetS, " ");

    // Convert all hyphens to same code
    itsString.replace(QChar(8211), QChar(45));

    // Convert all quotes to single variation
    itsString.replace(QChar(34), QChar(39));
    itsString.replace(QChar(145), QChar(39));
    itsString.replace(QChar(146), QChar(39));
    itsString.replace(QChar(147), QChar(39));
    itsString.replace(QChar(148), QChar(39));
    itsString.replace(QChar(8216), QChar(39));
    itsString.replace(QChar(8217), QChar(39));
    itsString.replace(QChar(8220), QChar(39));
    itsString.replace(QChar(8221), QChar(39));

    // Fix misleading French references to "Il"
    targetS.setPattern("(Il) (n\')?(y)");
    itsString.replace(targetS, "\\1\\2\\3");
    targetS.setPattern("(Il) (est|vous)");
    itsString.replace(targetS, "\\1\\2");

    // Address conditional break after comma, with space added above
    itsString.replace(QString(", ¤"), QString(",¤"));

    // Standardize Saint references
    targetS.setPattern("\\b(S[t|T])([e|E]?)(\\.?)[ |-]([A-Z])");
    //targetS.setPattern("\\b(S[t|T][e|E]?)[\\.|-]?\\s?([A-Z])");
    itsString.replace(targetS, "\\1\\2.\\4");
    /*targetS.setPattern("\bSt ([A-Z])");
    itsString.replace(targetS, "St\\.\\1");
    targetS.setPattern("\bSte ([A-Z])");
    itsString.replace(targetS, "Ste\\.\\1");
    targetS.setPattern("\bSte-([A-Z])");
    itsString.replace(targetS, "Ste\\.\\1");*/
    if (onlyContainsNames)
    {
        targetS.setPattern("\bSt\\. ([A-Z])");
        itsString.replace(targetS, "St.\\1");
        targetS.setPattern("\bSte\\. ([A-Z])");
        itsString.replace(targetS, "Ste.\\1");
    }
    else
    {
        targetS.setPattern("-St\\. ([A-Z])");
        itsString.replace(targetS, "-St.\\1");
        targetS.setPattern("-Ste\\. ([A-Z])");
        itsString.replace(targetS, "-Ste.\\1");
    }

    // Fix missing space at end of sentence
    if (!onlyContainsNames)
    {
        targetS.setPattern("([a-z]|[0-9])\\.[A-Z]");
        int i= -1;
        while (-1 != (i = itsString.indexOf(targetS, i+1)))
            itsString.insert(i + 2, QString(" "));

        // Loop back and revisit very rare  "Jane Smith-St. Jacques" if found at beginning
        int index = itsString.indexOf("-St. ", 0, Qt::CaseSensitive);
        if ((index != -1) && (index < 100))
            itsString.replace(index, 5, QString("-St."));
        index = itsString.indexOf("-Ste. ", 0, Qt::CaseSensitive);
        if ((index != -1) && (index < 100))
            itsString.replace(index, 6, QString("-Ste."));
    }
}

void OQString::fixDateFormats()
{
    QRegularExpression target;

    target.setPattern("(\\d{4})/(\\d+)/(\\d+)-(\\d{4})/(\\d+)/(\\d+)");
    itsString.replace(target, "\\1/\\2/\\3 - \\4/\\5/\\6");

    target.setPattern("(\\d{4})-(\\d+)-(\\d+)");
    itsString.replace(target, " \\1/\\2/\\3 ");

    target.setPattern("(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sept|Oct|Nov|Dec)-(\\d+)-(\\d{4})");
    itsString.replace(target, " \\1. \\2, \\3 ");

    target.setPattern("(January|February|March|April|May|June|July|August|September|October|November|December)-(\\d+)-(\\d{4})");
    itsString.replace(target, " \\1 \\2, \\3 ");

    itsString.replace("  ", " ");
    OQString tempString(itsString);
    tempString.cleanUpEnds();

    itsString = tempString.getString();
}

bool OQString::fixHyphenatedSaint()
{
    bool fixApplied = false;

    if ((countWords() > 1) || !isHyphenated())
        return fixApplied;

    int index = itsString.indexOf("-", 0, Qt::CaseInsensitive);
    if (index >= 0)
    {
        OQString tempString(itsString.left(index));
        if (tempString.isSaint())
        {
            itsString.replace("-", ".");
            fixApplied = true;
        }
    }

    return fixApplied;
}

bool OQString::checkQuotes(unsigned int numQuotes)
{
    bool issueWarning = false;  // Not used for now

    // No checks run if even number of quotes exist
    if ((numQuotes % 2) == 0)
        return issueWarning;

    OQString lastChar;
    QString word, string, newString;
    QString space(" ");
    int i, length, maxPosition;
    unsigned int numOpeningQuotes, numClosingQuotes;
    int startingPosition;

    QRegularExpression anyQuoteVariation("(\\x0022|\\x0027|\\x0060|\\x009[1-4])");
    QRegularExpression openingQuoteVariations("([ (]\\x0022|[ (]\\x0027|[ (]\\x0060|[ (]\\x009[1-4])");
    QRegularExpression closingQuoteVariations("(\\x0022[ )]|\\x0027[ )]|\\x0060[ )]|\\x009[1-4][ )])");
    // The next two were enhanced to capture � as being adjacent instead of space
    //QRegExp openingQuoteVariations("([ |(|\\xfffd]\\x0022|[ |(|\\xfffd]\\x0027|[ |(|\\xfffd]\\x0060|[ |(|\\xfffd]\\x009[1-4]");
    //QRegExp closingQuoteVariations("(\\x0022[ |)|\\xfffd]|\\x0027[ |)|\\xfffd]|\\x0060[ |)|\\xfffd]|\\x009[1-4][ |)|\\xfffd]");

    string = itsString;

    // Need to add a space if the last character in the string is a quote
    lastChar = OQString(itsString[itsString.length() - 1]);
    if ((lastChar.getCharType() & QUOTES) == QUOTES)
        string += space;

    maxPosition = string.length();

    // If number of opening quotes matches number of closing quotes, do nothing
    numOpeningQuotes = 0;
    numClosingQuotes = 0;

    startingPosition = 0;
    while (startingPosition < maxPosition)
    {
        i = string.indexOf(openingQuoteVariations, startingPosition);
        if (i == -1)
            startingPosition = maxPosition;
        else
        {
            numOpeningQuotes++;
            startingPosition = i + 1;
        }
    }

    startingPosition = 0;
    while (startingPosition < maxPosition)
    {
        i = string.indexOf(closingQuoteVariations, startingPosition);
        if (i == -1)
            startingPosition = maxPosition;
        else
        {
            numClosingQuotes++;
            startingPosition = i + 1;
        }
    }

    if (numOpeningQuotes != numClosingQuotes)
    {
        // Something is amiss, so attempt to fix
        startingPosition = 0;

        while (startingPosition < maxPosition)
        {
            i = string.indexOf(space, startingPosition, Qt::CaseInsensitive);
            if (i == -1)
                length = string.length()- startingPosition;
            else
                length = i - startingPosition + 1;

            // Extract word, dropping trailing space
            word = string.mid(startingPosition, length);
            if (word.right(1) == space)
                word.chop(1);
            startingPosition = startingPosition + length;

            // Search word for quotes
            i = word.indexOf(anyQuoteVariation, 0);
            if (i == -1)
            {
                // No quotation mark found, save word
                if ((newString.right(1) != space) && (newString.length() > 0) && (word.length() > 0))
                    newString += space;
                newString += word;
            }
            else    // Word contains a quote somewhere
            {
                if (word.length() == 1)
                {
                    // Dangling quote on its own - to be dropped by doing nothing
                }
                else
                {
                    // Add matching quotes where appropriate
                    // If quote is somewhere in the middle (eg. O'Donnell), this shouldn't present a problem, so leave for now

                    if ((newString.right(1) != space) && (newString.length() > 0))
                        newString += space;

                    // Deal with last char being a quote
                    if (i == (word.length() - 1))
                    {
                        PQString firstChar = word.left(1);
                        if ((firstChar.getCharType() & (PARENTHESES | OPENING)) == (PARENTHESES | OPENING))
                        {
                            newString += word.left(1);
                            newString += word.right(1);
                            newString += word.mid(1,-1);
                        }
                        else
                        {
                            // Add a quote upfront unless it is a possessive word (eg. Chris')
                            PQString penultimateChar = word.right(2);
                            penultimateChar.dropRight(1);
                            if ((penultimateChar != PQString("s")) && (penultimateChar != PQString("S")))
                            {
                                newString += word.right(1);
                                newString += word;
                            }
                            else
                                newString += word;
                        }
                    }

                    // Deal with first char being a quote
                    if (i == 0)
                    {
                        newString += word;
                        PQString lastChar = word.right(1);
                        if ((lastChar.getCharType() & (PARENTHESES | CLOSING)) == (PARENTHESES | CLOSING))
                        {
                            newString.chop(1);  // Drop the parentheses
                            newString += word.left(1);
                            newString += lastChar.getString();
                        }
                        else
                            newString += word.left(1);
                    }
                }
            }
        }

        itsString = newString;
    }

    return issueWarning;
}

unsigned int OQString::standardizeQuotes()
{
    QList<QString> listOfFirstWords;
    QString DNPL("Do not process list");
    listOfFirstWords.append(DNPL);
    GENDER gender(genderUnknown);
    LANGUAGE lang(language_unknown);

    return standardizeQuotes(listOfFirstWords, gender, lang);
}

unsigned int OQString::standardizeQuotes(QList<QString> &listOfFirstWords, GENDER gender, LANGUAGE lang)
{
    // Replaces any type of quote mark with the standard single quote
    // Process one sentence at a time so we can limit the counting of quotes to first two sentences
    // Save first word of each sentence which is not a gender word

    ushort unicodeNum;
    unsigned int numQuotes = 0;
    unsigned int numSentences = 0;
    unsigned int startingPosition = 0;
    unsigned int sentenceLength;
    OQString sentence, word;
    bool hadComma;
    QRegularExpression target;

    // Fix missing space before double quote
    target.setPattern("([A-Z]|[a-z])\"([A-Z]|[a-z])");
    itsString.replace(target, "\\1 \"\\2");

    // Only create list of first words if explicitly requested
    bool createFirstWordList = true;
    if (listOfFirstWords.size() == 1)
    {
        QString tempWord = listOfFirstWords.at(0);
        if (tempWord == QString("Do not process list"))
            createFirstWordList = false;
    }

    bool finished = false;

    while (!finished && (startingPosition < getLength()))
    {
        sentence = pullOutSentence(startingPosition);
        sentenceLength = sentence.getLength();
        if(sentenceLength > 0)
            numSentences++;

        if (sentenceLength == 0)
            finished = true;
        else
        {
            startingPosition += sentenceLength + 1;

            for (unsigned int i = 0; i < sentenceLength; i++)
            {
                unicodeNum= itsString.at(static_cast<int>(i)).unicode();

                switch(unicodeNum)
                {
                case 34:        //  "&#34;", "&quot;", "Double quotes (or speech marks)"
                case 145:       //  ‘  "&#145;", "&lsquo;", "Left single quotation mark"
                case 146:       //  ’  "&#146;", "&rsquo;", "Right single quotation mark"
                case 147:       //  “  "&#147;", "&ldquo;", "Left double quotation mark"
                case 148:       //  ”  "&#148;", "&rdquo;", "Right double quotation mark"
                case 8216:      //  ‘	0x91	U+2018	&lsquo
                case 8217:      //  ’	0x92	U+2019	&rsquo
                case 8220:      //  “  "&#147;", "&ldquo;", "Left double quotation mark"
                case 8221:      //  ”  "&#148;", "&rdquo;", "Right double quotation mark"
                case 96:        //  `  "&#96;"   "Grave accent"
                case 180:       //  ´  "&#180;", "&acute;", "Acute accent - spacing acute"

                    itsString[i] = QChar(39);
                    if (numSentences <= 2)
                        numQuotes++;
                    break;

                default:
                    // Do nothing
                    break;
                }
            }

            // This is the second optional step
            if (createFirstWordList)
            {
                word = sentence.pullOutWord(0);
                if (word.getLength() < sentenceLength)
                {
                    // Excludes all one word sentences as they are likely to be just the last name
                    word.removeEnding(" ");
                    hadComma = word.removeEnding(COMMA);
                    word.removeEnding(PUNCTUATION);
                    word = word.lower();
                    word.removePossessive();
                    if ((word.getLength() > 0) && !hadComma && (word.isGenderWord(gender, lang) || !word.isRecognized()))
                        listOfFirstWords.append(word.getString());
                }
            }
        }
    }

    return numQuotes;
}

bool OQString::fixParentheses()
{
    // Looks for unmatched parentheses and deletes where found
    int indexA, indexB;
    int start = 0;
    bool keepGoing = true;
    bool adjusted = false;

    while (keepGoing)
    {
        indexA = itsString.indexOf("(", start);
        indexB = itsString.indexOf(")", start);

        if ((indexA == -1) && (indexB == -1))
            keepGoing = false;

        if ((indexA == -1) && (indexB >= 0))
        {
            itsString.remove(indexB, 1);
            adjusted = true;
            keepGoing = false;
        }

        if ((indexA >= 0) && (indexB == -1))
        {
            itsString.remove(indexA, 1);
            adjusted = true;
            keepGoing = false;
        }

        start = indexB + 1;
    }

    return adjusted;
}

bool OQString::fixQuotes()
{
    unsigned int numQuotes;
    bool issueWarning = false;

    numQuotes = standardizeQuotes();
    if ((numQuotes % 2) == 1)
        issueWarning = checkQuotes(numQuotes);

    return issueWarning;
}

bool OQString::fixQuotes(QList<QString> &listOfFirstWords, GENDER gender, LANGUAGE lang)
{
    unsigned int numQuotes;
    bool issueWarning = false;

    numQuotes = standardizeQuotes(listOfFirstWords, gender, lang);
    if ((numQuotes % 2) == 1)
        issueWarning = checkQuotes(numQuotes);

    return issueWarning;
}

bool OQString::isPrefix(LANGUAGE lang) const
{
    return isAbbreviatedPrefix(lang) || isFullPrefix(lang);
}

bool OQString::isFullPrefix(LANGUAGE lang) const
{
    switch(lang)
    {
    case english:
        return isFoundIn(prefixesFullEnglish, 1);
        break;

    case french:
        return isFoundIn(prefixesFullFrench, 1);
        break;

    case spanish:
        return isFoundIn(prefixesFullSpanish, 1);
        break;

    default:
        return isFoundIn(prefixesFullEnglish, 1) || isFoundIn(prefixesFullFrench, 1) || isFoundIn(prefixesFullSpanish, 1);
        break;
    }

    return false;
}

bool OQString::isAbbreviatedPrefix(LANGUAGE lang) const
{
    switch(lang)
    {
    case english:
        return isFoundIn(prefixesAbbreviatedEnglish, 1);
        break;

    case french:
        return isFoundIn(prefixesAbbreviatedFrench, 1);
        break;

    case spanish:
        return isFoundIn(prefixesAbbreviatedSpanish, 1);
        break;

    default:
        return isFoundIn(prefixesAbbreviatedEnglish, 1) || isFoundIn(prefixesAbbreviatedFrench, 1) || isFoundIn(prefixesAbbreviatedSpanish, 1);
        break;
    }

    return false;
}

bool OQString::isSuffix(LANGUAGE lang) const
{
    return isSuffixKeep(lang) || isSuffixDrop(lang);
}

bool OQString::isSuffixKeep(LANGUAGE lang) const
{
    switch(lang)
    {
    case english:
        return isFoundIn(suffixesKeepEnglish, 1);
        break;

    case french:
        return isFoundIn(suffixesKeepFrench, 1);
        break;

    case spanish:
        return isFoundIn(suffixesKeepSpanish, 1);
        break;

    default:
        return isFoundIn(suffixesKeepEnglish, 1) || isFoundIn(suffixesKeepFrench, 1) || isFoundIn(suffixesKeepSpanish, 1);
        break;
    }

    return false;
}

bool OQString::isSuffixDrop(LANGUAGE lang) const
{
    switch(lang)
    {
    case english:
        return isFoundIn(suffixesDropEnglish, 1);
        break;

    case french:
        return isFoundIn(suffixesDropFrench, 1);
        break;

    case spanish:
        return isFoundIn(suffixesDropSpanish, 1);
        break;

    default:
        return isFoundIn(suffixesDropEnglish, 1) || isFoundIn(suffixesDropFrench, 1) || isFoundIn(suffixesDropSpanish, 1);
        break;
    }

    return false;
}

bool OQString::isSuffixAllCaps(LANGUAGE lang) const
{
    if (getLength() == 0)
		return false;

    if (isAllCaps() && isSuffix(lang))
        return true;
    else
        return false;
}

bool OQString::isMaleTitle(LANGUAGE lang) const
{
    if (getLength() == 0)
        return false;

    bool result;

    switch (lang)
    {
    case english:
        result = isFoundIn(maleTitlesEnglish, 1);
        break;

    case french:
        result = isFoundIn(maleTitlesFrench, 1);
        break;

    case spanish:
        result = isFoundIn(maleTitlesSpanish, 1);
        break;

    default:
        result = isFoundIn(maleTitlesEnglish, 1) || isFoundIn(maleTitlesFrench, 1) || isFoundIn(maleTitlesSpanish, 1);
    }

    return result;
}

bool OQString::isFemaleTitle(LANGUAGE lang) const
{
    if (getLength() == 0)
        return false;

    bool result;

    switch (lang)
    {
    case english:
        result = isFoundIn(femaleTitlesEnglish, 1);
        break;

    case french:
        result = isFoundIn(femaleTitlesFrench, 1);
        break;

    case spanish:
        result = isFoundIn(femaleTitlesSpanish, 1);
        break;

    default:
        result = isFoundIn(femaleTitlesEnglish, 1) || isFoundIn(femaleTitlesFrench, 1) || isFoundIn(femaleTitlesSpanish, 1);
    }

    return result;
}

bool OQString::isEnglishTitle(GENDER gender) const
{
    if (getLength() == 0)
        return false;

    bool result;

    switch (gender)
    {
    case Male:
        result = isFoundIn(maleTitlesEnglish, 1);
        break;

    case Female:
        result = isFoundIn(femaleTitlesEnglish, 1);
        break;

    default:
        result = isFoundIn(maleTitlesEnglish, 1) || isFoundIn(femaleTitlesEnglish, 1);
        break;
    }

    return result;
}

bool OQString::isFrenchTitle(GENDER gender) const
{
    if (getLength() == 0)
        return false;

    bool result;

    switch (gender)
    {
    case Male:
        result = isFoundIn(maleTitlesFrench, 1);
        break;

    case Female:
        result = isFoundIn(femaleTitlesFrench, 1);
        break;

    default:
        result = isFoundIn(maleTitlesFrench, 1) || isFoundIn(femaleTitlesFrench, 1);
        break;
    }

    return result;
}

bool OQString::isSpanishTitle(GENDER gender) const
{
    if (getLength() == 0)
        return false;

    bool result;

    switch (gender)
    {
    case Male:
        result = isFoundIn(maleTitlesSpanish, 1);
        break;

    case Female:
        result = isFoundIn(femaleTitlesSpanish, 1);
        break;

    default:
        result = isFoundIn(maleTitlesSpanish, 1) || isFoundIn(femaleTitlesSpanish, 1);
        break;
    }

    return result;
}

bool OQString::isTitle(LANGUAGE lang, GENDER gender) const
{
    if (getLength() == 0)
        return false;

    bool result;

    switch (lang)
    {
    case english:
        result = isEnglishTitle(gender);
        break;

    case french:
        result = isFrenchTitle(gender);
        break;

    case spanish:
        result = isSpanishTitle(gender);
        break;

    default:
        result = isEnglishTitle(gender) || isFrenchTitle(gender) || isSpanishTitle(gender);
    }

    return result;
}

bool OQString::isSaint() const
{
    return isFoundIn(saints, 1);
}

bool OQString::isGodReference() const
{
    return isFoundIn(godReferences, 1);
}

bool OQString::isAbbreviation() const
{
    return isFoundIn(provAbbreviations, 1) || isFoundIn(otherAbbreviations, 1) || (isAllCaps(false) && !containsVowel() && !isNumeric());
}

bool OQString::isProvince() const
{
    OQString simplified = getUnaccentedString();
    simplified.replace("-", " ");

    return isProvAbbreviation() || simplified.isFoundIn(provLong, 1);
}

bool OQString::isProvAbbreviation() const
{
    return isFoundIn(provAbbreviations, 1);
}

bool OQString::isCompoundName() const
{
    return isFoundIn(uncapitalizedNames, 1);
}

bool OQString::isUncapitalizedName() const
{
    return isFoundIn(uncapitalizedNames, 1);
}

bool OQString::isSingleVan() const
{
    return isFoundIn(singleVans, 1);
}

bool OQString::isNoVowelName() const
{
    return isFoundIn(noVowelNames, 1);
}

bool OQString::isSpousalReference(const LANGUAGE lang) const
{
    bool result = false;
    OQString word = itsString;

    switch (lang)
    {
    case french:
        if ((word == OQString("mari")) || (word == OQString("femme")) || (word == OQString("partenaire")) || (word == OQString("conjoint")) || (word == OQString("conjointe")) || (word == OQString("époux")) || (word == OQString("épouse")))
            result = true;
        break;

    case spanish:
        if ((word == OQString("marido")) || (word == OQString("esposa")))
            result = true;
        break;

    default:
        if ((word == OQString("husband")) || (word == OQString("wife")) || (word == OQString("partner")) || (word == OQString("soulmate")) || (word == OQString("spouse")))
            result = true;
        break;
    }

    return result;
}

bool OQString::isHyphenatedName() const
{
    OQString target;
    int index;

    index = itsString.indexOf(QString("-"));
    if (index == -1)
        target = itsString;
    else
        target = itsString.left(index);

    return target.isFoundIn(hyphenatedNameBeginnings, 1);
}

bool OQString::isDegree() const
{
    return isFoundIn(suffixesDegree, 1);
}

bool OQString::isAltNameIndicator() const
{
    return isFoundIn(altNameIndicators, 0);
}

bool OQString::isNeeEtAl() const
{
    return isFoundIn(neeEtAlwords, 0);
}

bool OQString::isNeeEtAl(OQString nextWord) const
{
    QStringList disqualifyingWords = QString("le|on|to|with").split("|");

    if (disqualifyingWords.contains(nextWord.lower().getString()) || nextWord.isWrittenMonth())
        return false;
    else
        return isFoundIn(neeEtAlwords, 0);
}

bool OQString::isEndOfBlockTag() const
{
    return isFoundIn(endOfBlockTags, 0);
}

bool OQString::isAnd() const
{
    QString lowerWord = itsString.toLower();

    return  (lowerWord == QString("and")) || (lowerWord == QString("et")) || (lowerWord == QString("y")) || (lowerWord == QString("&"));
}

bool OQString::isLocation() const
{
    return isFoundIn(locations, 1);
}

bool OQString::isInitial() const
{
    bool result = false;

    OQString temp(itsString);
    if (temp.right(1) == OQString("."))
        temp.dropRight(1);

    if ((temp.getLength() == 1) && temp.isAlpha() && temp.isCapitalized())
        result = true;

    return result;
}

bool OQString::isAboriginalName(OQString nextWord, bool firstCall) const
{
    // For the problematic names that are also non-aboriginal last names (e.g. Young), a secondary check is performed

    if (firstCall && ((nextWord.getLength() == 0) || (nextWord.left(1) == OQString("("))))
        return false;

    if (PQString(itsString).isCapitalized() && !nextWord.isCapitalized())
        return false;

    bool potential = false;
    bool result = false;

    potential = isFoundIn(aboriginalNames, 1);
    if (potential)
    {
        if (firstCall && isFoundIn(problematicAboriginalNames, 1))
        {
           result = nextWord.isAboriginalName(nextWord, false);
        }
        else
            result = true;
    }

    return result;
}

bool OQString::isOrdinal(const LANGUAGE lang) const
{
    switch (lang)
    {
    case english:
        if (itsString.length() == 2)
            return isFoundIn(ordinalsEnglish, 0);
        break;

    case french:
        if (itsString.length() == 4)
            return isFoundIn(ordinalsFrench, 0);
        break;

    case spanish:
        if ((itsString.length() == 1) || (itsString.length() == 2))
            return isFoundIn(ordinalsSpanish, 0);
        break;

    default:
        return isOrdinal(english) || isOrdinal(french) || isOrdinal(spanish);

    }

    return false;
}

bool OQString::isOrdinalNumber(const LANGUAGE lang) const
{
    return static_cast<bool>(getOrdinalNumber(lang));
}

unsigned int OQString::getOrdinalNumber(const LANGUAGE lang) const
{
    unsigned int result = 0;
    unsigned int number;
    OQString string, ending;


    switch (lang)
    {
    case english:
        // Allow for numbers 1st to 125th
        if ((itsString.length() >= 3) && (itsString.length() <= 5))
        {
            ending = itsString.right(2).toLower();
            if (ending.isOrdinal(english))
            {
                string = itsString.toLower();
                string.dropRight(2);
                if (string.isNumeric())
                {
                    number = static_cast<unsigned int>(string.asNumber());
                    if ((number >= 1) && (number <= 125))
                        result = number;
                }
            }
        }
        break;

    case french:
        // Allow for numbers 1iere to 125ieme
        if ((itsString.length() >= 5) && (itsString.length() <= 7))
        {
            ending = itsString.right(4).toLower();
            if (ending.isOrdinal(french))
            {
                string = itsString.toLower();
                string.dropRight(4);
                if (string.isNumeric())
                {
                    number = static_cast<unsigned int>(string.asNumber());
                    if ((number >= 1) && (number <= 125))
                        result = number;
                }
            }
        }
        break;

    case spanish:
        // Allow for numbers 1 to 125
        if ((itsString.length() >= 2) && (itsString.length() <= 5))
        {
            ending = itsString.right(1).toLower();
            if (ending.isOrdinal(spanish))
            {
                string = itsString.toLower();
                string.dropRight(1);
                if (string.isNumeric())
                {
                    number = static_cast<unsigned int>(string.asNumber());
                    if ((number >= 1) && (number <= 125))
                        result = number;
                }
            }
            else
            {
                ending = itsString.right(2).toLower();
                if (ending.isOrdinal(spanish))
                {
                    string = itsString.toLower();
                    string.dropRight(2);
                    if (string.isNumeric())
                    {
                        number = static_cast<unsigned int>(string.asNumber());
                        if ((number >= 1) && (number <= 125))
                            result = number;
                    }
                }
            }
        }
        break;

    default:
        result = getOrdinalNumber(english);
        if (!result)
            result = getOrdinalNumber(french);
        if (!result)
            result = getOrdinalNumber(spanish);

        break;

    }

    return result;
}

unsigned int OQString::isWrittenMonth(const LANGUAGE &lang, const bool fullWordsOnly)
{
    if ((this->getLength() == 0) || (this->getLength() > 10))
        return false;

    PQString lowerWord;
    PQString *word;
    QList<monthInfo> *listOfMonths;
    monthInfo mi;

    unsigned int numericMonth = 0;
    bool matched = false;
    bool execute;
    word = new PQString(itsString);
    word->removeEnding(".");
    lowerWord = word->lower();

    int i = 1;
    while (!matched && (i <= 3))
    {
        switch (i)
        {
        case 1:
            listOfMonths = &monthsEnglishFull;
            execute = (lang != french) && (lang != spanish);
            break;

        case 2:
            listOfMonths = &monthsFrenchFull;
            execute = (lang != english) && (lang != spanish);
            break;

        case 3:
            listOfMonths = &monthsSpanishFull;
            execute = (lang != english) && (lang != french);
            break;
        }

        if (execute)
        {
            QListIterator<monthInfo> iter(*listOfMonths);

            while (!matched && iter.hasNext())
            {
                mi = iter.next();
                if (lowerWord == mi.monthAlpha)
                {
                    matched = true;
                    numericMonth = mi.monthNumeric;
                }
            }
        }

        i++;
    }

    if (!matched && !fullWordsOnly)
        numericMonth = isWrittenMonthAbbreviation(lang);

    delete word;

    return numericMonth;
}

unsigned int OQString::isWrittenMonthAbbreviation(const LANGUAGE &lang)
{
    if ((this->getLength() == 0) || (this->getLength() > 5))
        return false;

    PQString lowerWord;
    PQString *word;
    QList<monthInfo> *listOfMonths;
    monthInfo mi;

    unsigned int numericMonth = 0;
    bool matched = false;
    bool execute;
    word = new PQString(itsString);
    word->removeEnding(".");
    lowerWord = word->lower();

    int i = 1;
    while (!matched && (i <= 3))
    {
        switch (i)
        {
        case 1:
            listOfMonths = &monthsEnglishAbbreviated;
            execute = (lang != french) && (lang != spanish);
            break;

        case 2:
            listOfMonths = &monthsFrenchAbbreviated;
            execute = (lang != english) && (lang != spanish);
            break;

        case 3:
            listOfMonths = &monthsSpanishAbbreviated;
            execute = (lang != english) && (lang != french);
            break;
        }

        if (execute)
        {
            QListIterator<monthInfo> iter(*listOfMonths);

            while (!matched && iter.hasNext())
            {
                mi = iter.next();
                if (lowerWord == mi.monthAlpha)
                {
                    matched = true;
                    numericMonth = mi.monthNumeric;
                }
            }
        }

        i++;
    }

    delete word;

    return numericMonth;
}

unsigned int OQString::isWrittenDayOfWeek(const LANGUAGE &lang)
{
    if ((this->getLength() == 0) || (this->getLength() > 9))
        return false;

    PQString lowerWord;
    PQString *word;
    QList<dayOfWeekInfo> *listOfDays;
    dayOfWeekInfo dowi;

    unsigned int numericDayOfWeek = 0;
    bool matched = false;
    bool execute;
    word = new PQString(itsString);
    word->removeEnding(".");
    lowerWord = word->lower();

    int i = 1;
    while (!matched && (i <= 3))
    {
        switch (i)
        {
        case 1:
            listOfDays = &daysOfWeekEnglish;
            execute = (lang != french) && (lang != spanish);
            break;

        case 2:
            listOfDays = &daysOfWeekFrench;
            execute = (lang != english) && (lang != spanish);
            break;

        case 3:
            listOfDays = &daysOfWeekSpanish;
            execute = (lang != english) && (lang != french);
            break;
        }

        if (execute)
        {
            QListIterator<dayOfWeekInfo> iter(*listOfDays);

            while (!matched && iter.hasNext())
            {
                dowi = iter.next();
                if (lowerWord == dowi.dayOfWeekAlpha)
                {
                    matched = true;
                    numericDayOfWeek = dowi.dayOfWeekNumeric;
                }
            }
        }

        i++;
    }

    delete word;

    return numericDayOfWeek;
}

bool OQString::isPureDateRange(LANGUAGE lang) const
{
    bool result = false;
    QRegularExpression target;
    QRegularExpressionMatch match;
    QString matchedString;

    switch(lang)
    {
    case english:
        target.setPattern("(January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4}) - (January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})");
        match = target.match(itsString);
        if (match.hasMatch())
        {
            matchedString = match.captured(1) + QString(" ") + match.captured(2) + QString(" - ") + match.captured(3) + QString(" ") + match.captured(4);
            if (itsString == matchedString)
                return true;
        }
        else
        {
            target.setPattern("(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) (\\d+, \\d{4}) - (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) (\\d+, \\d{4})");
            match = target.match(itsString);
            if (match.hasMatch())
            {
                matchedString = match.captured(1) + QString(" ") + match.captured(2) + QString(" - ") + match.captured(3) + QString(" ") + match.captured(4);
                if (itsString == matchedString)
                    return true;
                else
                    return false;
            }
            else
                return false;
        }
        break;

    case french:
        // Relies on standardizing of dates within fixBasicErrors
        target.setPattern("(\\d+ )(janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre) (\\d{4}) - (\\d+ )(janvier|fevrier|février|mars|avril|mai|juin|juillet|aout|août|septembre|octobre|novembre|decembre|décembre) (\\d{4})");
        match = target.match(itsString);
        if (match.hasMatch())
        {
            matchedString = match.captured(1) + match.captured(2) + QString(" ") + match.captured(3) + QString(" - ") + match.captured(4) + match.captured(5) + QString(" ") + match.captured(6);
            if (itsString == matchedString)
                return true;
        }
        break;

    case spanish:
        break;

    default:
    case language_unknown:
        result = isPureDateRange(english);
        if (!result)
            result = isPureDateRange(french);
        if (!result)
            result = isPureDateRange(spanish);
        break;
    }

    return result;
}

bool OQString::isRecognized(LANGUAGE lang) const
{
    if (getLength() == 0)
        return false;

    if (isPrefix(lang) || isSuffix(lang) || isTitle(lang) || isSaint() || isCompoundName())
        return true;

    if ((lang != french) && (lang != spanish))
    {
        if (isEnglish() || isEnglishMale() || isEnglishFemale())
            return true;

        if (isAgeWord(english) || isBirthWord(english) || isDeathWord(english))
            return true;
    }

    if ((lang != english) && (lang != spanish))
    {
        if (isFrench() || isFrenchMale() || isFrenchFemale())
            return true;

        if (isAgeWord(french) || isBirthWord(french) || isDeathWord(french))
            return true;
    }

    if ((lang != english) && (lang != french))
    {
        if (isSpanish() || isSpanishMale() || isSpanishFemale())
            return true;

        if (isAgeWord(spanish) || isBirthWord(spanish) || isDeathWord(spanish))
            return true;
    }

    return false;
}

bool OQString::isRecognizedFirstWord() const
{
    if (getLength() == 0)
        return false;

    switch(itsString.length())
    {
    case 1:
        return isFoundIn(ignoreFirstWords1, 1);
    case 2:
        return isFoundIn(ignoreFirstWords2, 1);
    case 3:
        return isFoundIn(ignoreFirstWords3, 1);
    case 4:
        return isFoundIn(ignoreFirstWords4, 1);
    case 5:
        return isFoundIn(ignoreFirstWords5, 1);
    case 6:
        return isFoundIn(ignoreFirstWords6, 1);
    case 7:
        return isFoundIn(ignoreFirstWords7, 1);
    case 8:
        return isFoundIn(ignoreFirstWords8, 1);
    case 9:
        return isFoundIn(ignoreFirstWords9, 1);
    case 10:
        return isFoundIn(ignoreFirstWords10, 1);
    case 11:
        return isFoundIn(ignoreFirstWords11, 1);
    case 12:
        return isFoundIn(ignoreFirstWords12, 1);
    case 13:
        return isFoundIn(ignoreFirstWords13, 1);
    case 14:
        return isFoundIn(ignoreFirstWords14, 1);
    default:
        return false;
    }
}


bool OQString::isInformalVersionOf(const QString &formalname, PQString &errMsg) const
{
    if ((formalname.size() == 0) || (itsString.size() == 0))
        return false;

    QSqlQuery query;

    bool success;
    unsigned int numNames;
    int compareResult;
    QStringList listOfNames;
    QString unparsedList;

    bool matched = false;
    unsigned int i = 0;

    success = query.prepare("SELECT altNames FROM death_audits.firstnames WHERE name = :formalname");
    query.bindValue(":formalname", QVariant(formalname));
    success = query.exec();

    if (!success || (query.size() >= 2))
    {
        errMsg << QString("SQL problem looking up nicknames for: ") << formalname << " in firstnames database";
        return false;
    }
    else
    {
        if (query.size() == 0)
            matched = false;
        else
        {
            query.next();
            unparsedList = query.value(0).toString();
            listOfNames = unparsedList.split(" ", Qt::SkipEmptyParts);
            if ((listOfNames.size() == 1) && (listOfNames.at(0).length() == 0))
                listOfNames.clear();
            numNames = static_cast<unsigned int>(listOfNames.size());

            while (!matched && (i < numNames))
            {
                compareResult = itsString.compare(listOfNames.at(static_cast<int>(i)), Qt::CaseInsensitive);
                if (compareResult == 0)
                    matched = true;
                else
                {
                    if (compareResult > 0)
                    {
                        // itsString is greater than target lexically
                        i++;
                    }
                    else
                    {
                        // compareResult == -1, so no longer a chance of matching since list is alphabetical
                        i = numNames;
                    }
                }
            }
        }
    }

    query.clear();

    return matched;
}

bool OQString::isFormalVersionOf(const QString &nickname, PQString &errMsg) const
{
    QSqlQuery query;

    bool success;
    unsigned int numNames;
    int compareResult;
    QStringList listOfNames;
    QString unparsedList;

    bool matched = false;
    unsigned int i = 0;

    success = query.prepare("SELECT formalNames FROM death_audits.nicknames WHERE nickname = :nickname");
    query.bindValue(":nickname", QVariant(nickname));
    success = query.exec();

    if (!success || (query.size() >= 2))
    {
        errMsg << QString("SQL problem looking up formal names for: ") << nickname << " in nicknames database";
        return false;
    }
    else
    {
        if (query.size() == 0)
            matched = false;
        else
        {
            query.next();
            unparsedList = query.value(0).toString();
            listOfNames = unparsedList.split(" ", Qt::SkipEmptyParts);
            if ((listOfNames.size() == 1) && (listOfNames.at(0).length() == 0))
                listOfNames.clear();
            numNames = static_cast<unsigned int>(listOfNames.size());

            while (!matched && (i < numNames))
            {
                compareResult = itsString.compare(listOfNames.at(static_cast<int>(i)), Qt::CaseInsensitive);
                if (compareResult == 0)
                    matched = true;
                else
                {
                    if (compareResult > 0)
                    {
                        // itsString is greater than target lexically
                        i++;
                    }
                    else
                    {
                        // compareResult < 0, so no longer a chance of matching since list is alphabetical
                        i = numNames;
                    }
                }
            }
        }
    }

    query.clear();

    return matched;
}

bool OQString::isAGivenName(PQString &errMsg) const
{
    QSqlQuery query;

    bool success;
    bool matched = true;

    QList<QString> nameList = itsString.split("-");

    while (matched && nameList.size() >= 1)
    {
        success = query.prepare("SELECT name FROM death_audits.firstnames WHERE name = :name");
        query.bindValue(":name", QVariant(nameList.takeFirst()));
        success = query.exec();

        if (!success || (query.size() >= 2))
        {
            errMsg << QString("SQL problem looking up potential first name: ") << itsString << " in firstnames database";
            matched = false;
        }
        else
        {
            if (query.size() == 1)
                matched = true;
            else
                matched = false;
        }

        query.clear();
    }

    return matched;
}

bool OQString::isALastName(PQString &errMsg) const
{
    QSqlQuery query;

    bool success;
    bool matched = false;

    success = query.prepare("SELECT lastname FROM death_audits.deceased WHERE lastname = :name");
    query.bindValue(":name", QVariant(itsString));
    success = query.exec();

    if (!success)
    {
        errMsg << QString("SQL problem looking up potential last name: ") << itsString << " in deceased database";
        return false;
    }
    else
    {
        if (query.size() > 0)
            matched = true;
    }

    query.clear();

    return matched;
}

bool OQString::followedByPrecedingIndicators(OQString &peek1, OQString &peek2, OQString &peek3, LANGUAGE lang) const
{
    OQString doubleWord;
    OQString word = OQString(itsString);
    bool precedingFlag = false;

    doubleWord = word + OQString(" ") + peek1;

    if (doubleWord.isFoundIn(precedingIndicators, 1))
    {
        if ((word == OQString("in")) || (word == OQString("dans")))
        {
            if (peek2.removeOrdinal(lang) || peek2.isNumeric())
                precedingFlag = true;
        }
        else
        {
            if (word == OQString("on"))
            {
                if (peek2.removeOrdinal(lang) || peek2.isNumeric())
                {
                    if (peek3 == OQString("year"))
                        precedingFlag = true;
                }
            }
            else
                precedingFlag = true;
        }
    }

    return precedingFlag;
}

bool OQString::isFoundIn(QList<QString> &wordList, int sortOrder) const
{
    // sortOrder is the order of the wordList
    // 0 = unsorted
    // 1 = sorted alphabetically
    // 2 = sorted by string length

    if ((itsString.length() > MAXWORDLENGTH) || (itsString.length() == 0))
        return false;

    bool matched = false;
    bool stillPossible = true;
    QString space(" ");
    QListIterator<QString> iter(wordList);
    QString wordFromList;
    int comparison;

    QString targetWord = this->lower().getString();
    targetWord.remove(QChar('.'), Qt::CaseInsensitive);

    while (stillPossible && !matched && iter.hasNext())
    {
        wordFromList = iter.next();
        if (wordFromList.right(1) == space)
            wordFromList.chop(1);
        if (wordFromList.left(1) == space)
            wordFromList.remove(0, 1);

        comparison = targetWord.compare(wordFromList);

        if (comparison == 0)
            matched = true;
        else
        {
            if ((sortOrder == 1) && (comparison < 0))
                stillPossible = false;
            if ((sortOrder == 2) && (comparison < 0) && (targetWord.length() < wordFromList.length()))
                stillPossible = false;
        }

    }

    return matched;
}

bool OQString::isEnglish() const
{
    switch(itsString.length())
    {
    case 1:
        return isFoundIn(ignoreWordsEnglish1, 1);
    case 2:
        return isFoundIn(ignoreWordsEnglish2, 1);
    case 3:
        return isFoundIn(ignoreWordsEnglish3, 1);
    case 4:
        return isFoundIn(ignoreWordsEnglish4, 1);
    case 5:
        return isFoundIn(ignoreWordsEnglish5, 1);
    case 6:
        return isFoundIn(ignoreWordsEnglish6, 1);
    case 7:
        return isFoundIn(ignoreWordsEnglish7, 1);
    case 8:
        return isFoundIn(ignoreWordsEnglish8, 1);
    case 9:
        return isFoundIn(ignoreWordsEnglish9, 1);
    case 10:
        return isFoundIn(ignoreWordsEnglish10, 1);
    case 11:
        return isFoundIn(ignoreWordsEnglish11, 1);
    case 12:
        return isFoundIn(ignoreWordsEnglish12, 1);
    case 13:
        return isFoundIn(ignoreWordsEnglish13, 1);
    case 14:
        return isFoundIn(ignoreWordsEnglish14, 1);
    default:
        return false;
    }
}

bool OQString::isEnglishMale() const
{
    return isFoundIn(genderWordsEnglishM);
}

bool OQString::isEnglishFemale() const
{
    return isFoundIn(genderWordsEnglishF);
}

bool OQString::isFrench() const
{
    switch(itsString.length())
    {
    case 1:
        return isFoundIn(ignoreWordsFrench1, 1);
    case 2:
        return isFoundIn(ignoreWordsFrench2, 1);
    case 3:
        return isFoundIn(ignoreWordsFrench3, 1);
    case 4:
        return isFoundIn(ignoreWordsFrench4, 1);
    case 5:
        return isFoundIn(ignoreWordsFrench5, 1);
    case 6:
        return isFoundIn(ignoreWordsFrench6, 1);
    case 7:
        return isFoundIn(ignoreWordsFrench7, 1);
    case 8:
        return isFoundIn(ignoreWordsFrench8, 1);
    case 9:
        return isFoundIn(ignoreWordsFrench9, 1);
    case 10:
        return isFoundIn(ignoreWordsFrench10, 1);
    case 11:
        return isFoundIn(ignoreWordsFrench11, 1);
    case 12:
        return isFoundIn(ignoreWordsFrench12, 1);
    case 13:
        return isFoundIn(ignoreWordsFrench13, 1);
    case 14:
        return isFoundIn(ignoreWordsFrench14, 1);
    default:
        return false;
    }
}

bool OQString::isFrenchMale() const
{
    return isFoundIn(genderWordsFrenchM);
}

bool OQString::isFrenchFemale() const
{
    return isFoundIn(genderWordsFrenchF);
}

bool OQString::isSpanish() const
{
    switch(itsString.length())
    {
    case 1:
        return isFoundIn(ignoreWordsSpanish1, 1);
    case 2:
        return isFoundIn(ignoreWordsSpanish2, 1);
    case 3:
        return isFoundIn(ignoreWordsSpanish3, 1);
    case 4:
        return isFoundIn(ignoreWordsSpanish4, 1);
    case 5:
        return isFoundIn(ignoreWordsSpanish5, 1);
    case 6:
        return isFoundIn(ignoreWordsSpanish6, 1);
    case 7:
        return isFoundIn(ignoreWordsSpanish7, 1);
    case 8:
        return isFoundIn(ignoreWordsSpanish8, 1);
    case 9:
        return isFoundIn(ignoreWordsSpanish9, 1);
    case 10:
        return isFoundIn(ignoreWordsSpanish10, 1);
    case 11:
        return isFoundIn(ignoreWordsSpanish11, 1);
    case 12:
        return isFoundIn(ignoreWordsSpanish12, 1);
    case 13:
        return isFoundIn(ignoreWordsSpanish13, 1);
    case 14:
        return isFoundIn(ignoreWordsSpanish14, 1);
    default:
        return false;
    }
}

bool OQString::isSpanishMale() const
{
    return isFoundIn(genderWordsSpanishM);
}

bool OQString::isSpanishFemale() const
{
    return isFoundIn(genderWordsSpanishF);
}

bool OQString::isBirthWord(LANGUAGE language) const
{
    switch(language)
    {
    case english:
        return isFoundIn(birthWordsEnglish, 1);
        break;

    case french:
        return isFoundIn(birthWordsFrench, 1);
        break;

    case spanish:
        return isFoundIn(birthWordsSpanish, 1);
        break;

    default:
        return isFoundIn(birthWordsEnglish, 1) || isFoundIn(birthWordsFrench, 1) || isFoundIn(birthWordsSpanish, 1);
    }
}

bool OQString::isAgeWord(LANGUAGE language) const
{
    switch(language)
    {
    case english:
        return isFoundIn(ageWordsEnglish, 1);
        break;

    case french:
        return isFoundIn(ageWordsFrench, 1);
        break;

    case spanish:
        return isFoundIn(ageWordsSpanish, 1);
        break;

    default:
        return isFoundIn(ageWordsEnglish, 1) || isFoundIn(ageWordsFrench, 1) || isFoundIn(ageWordsSpanish, 1);
    }
}

bool OQString::isDeathWord(LANGUAGE language) const
{
    switch(language)
    {    
    case english:
        return isFoundIn(deathWordsEnglish, 1);
        break;

    case french:
        return isFoundIn(deathWordsFrench, 1);
        break;

    case spanish:
        return isFoundIn(deathWordsSpanish, 1);
        break;

    default:
        return isFoundIn(deathWordsEnglish, 1) || isFoundIn(deathWordsFrench, 1) || isFoundIn(deathWordsSpanish, 1);
    }
}

bool OQString::isServiceWord(LANGUAGE language) const
{
    switch(language)
    {
    case english:
        return isFoundIn(serviceWordsEnglish, 1);
        break;

    case french:
        return isFoundIn(serviceWordsFrench, 1);
        break;

    case spanish:
        return isFoundIn(serviceWordsSpanish, 1);
        break;

    default:
        return isFoundIn(serviceWordsEnglish, 1) || isFoundIn(serviceWordsFrench, 1) || isFoundIn(serviceWordsSpanish, 1);
    }
}

bool OQString::isDeathIndicator(LANGUAGE language) const
{
    switch(language)
    {
    case english:
        return isFoundIn(deathIndicatorsEnglish, 1);
        break;

    case french:
        return isFoundIn(deathIndicatorsFrench, 1);
        break;

    case spanish:
        return isFoundIn(deathIndicatorsSpanish, 1);
        break;

    default:
        return isFoundIn(deathIndicatorsEnglish, 1) || isFoundIn(deathIndicatorsFrench, 1) || isFoundIn(deathIndicatorsSpanish, 1);
    }
}

bool OQString::isOtherPersonReference(LANGUAGE language) const
{
    switch(language)
    {
    case english:
        return isFoundIn(otherPersonReferenceWordsEnglish, 1);
        break;

    case french:
        return isFoundIn(otherPersonReferenceWordsFrench, 1);
        break;

    case spanish:
        return isFoundIn(otherPersonReferenceWordsSpanish, 1);
        break;

    default:
        return isFoundIn(otherPersonReferenceWordsEnglish, 1) || isFoundIn(otherPersonReferenceWordsFrench, 1) || isFoundIn(otherPersonReferenceWordsSpanish, 1);
    }
}

bool OQString::isMiscKeeperWord(LANGUAGE language) const
{
    switch(language)
    {
    case english:
        return isFoundIn(otherMiscKeeperWordsEnglish, 2);

    case french:
        return isFoundIn(otherMiscKeeperWordsFrench, 2);
        break;

    case spanish:
        return isFoundIn(otherMiscKeeperWordsSpanish, 2);
        break;

    default:
        return isFoundIn(otherMiscKeeperWordsEnglish, 2) || isFoundIn(otherMiscKeeperWordsFrench, 2) || isFoundIn(otherMiscKeeperWordsSpanish, 2);
    }
}

bool OQString::isProblematicFirstName() const
{
    return isFoundIn(problematicFirstNames);
}

bool OQString::isPronoun() const
{
    return isFoundIn(pronouns, 1);
}

bool OQString::isGenderWord(GENDER gender, LANGUAGE language) const
{
    switch(language)
    {
    case english:
        switch(gender)
        {
        case Male:
            return isEnglishMale();
            break;

        case Female:
            return isEnglishFemale();
            break;

        case genderUnknown:
            return isEnglishMale() || isEnglishFemale();
        }
        break;

    case french:
        switch(gender)
        {
        case Male:
            return isFrenchMale();
            break;

        case Female:
            return isFrenchFemale();
            break;

        case genderUnknown:
            return isFrenchMale() || isFrenchFemale();
        }
        break;

    case spanish:
        switch(gender)
        {
        case Male:
            return isSpanishMale();
            break;

        case Female:
            return isSpanishFemale();
            break;

        case genderUnknown:
            return isSpanishMale() || isSpanishFemale();
            break;
        }
        break;

    default:
        switch(gender)
        {
        case Male:
            return isEnglishMale() || isFrenchMale()  || isSpanishMale();
            break;

        case Female:
            return isEnglishFemale() || isFrenchFemale()  || isSpanishFemale();
            break;

        case genderUnknown:
            return isEnglishMale() || isEnglishFemale() ||
                   isFrenchMale()  || isFrenchFemale()  ||
                   isSpanishMale() || isSpanishFemale();
            break;
        }
        break;
    }

    return false;
}

bool OQString::isMaleGenderWord(LANGUAGE language) const
{
    switch(language)
    {
    case english:
        return isEnglishMale();
        break;

    case french:
        return isFrenchMale();
        break;

    case spanish:
        return isSpanishMale();
        break;

    default:
        return isEnglishMale() || isFrenchMale()  || isSpanishMale();
    }

    return false;
}

bool OQString::isFemaleGenderWord(LANGUAGE language) const
{
    switch(language)
    {
    case english:
        return isEnglishFemale();
        break;

    case french:
        return isFrenchFemale();
        break;

    case spanish:
        return isSpanishFemale();
        break;

    default:
        return isEnglishFemale() || isFrenchFemale()  || isSpanishFemale();
    }

    return false;
}

bool OQString::areParentWords(LANGUAGE lang, GENDER gender) const
{
    QList<QString> searchList = getParentReferences(lang, gender);

    return searchList.contains(itsString.toLower());
}

QList<QString> OQString::getParentReferences(const LANGUAGE lang, GENDER gender) const
{
    QList<QString> resultList;

    if ((lang != french) && (lang != spanish))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(parentWordsEnglishM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(parentWordsEnglishF);

        resultList.append(parentWordsEnglishU);
    }

    if ((lang != english) && (lang != spanish))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(parentWordsFrenchM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(parentWordsFrenchF);

        resultList.append(parentWordsFrenchU);
    }

    if ((lang != english) && (lang != french))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(parentWordsSpanishM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(parentWordsSpanishF);

        resultList.append(parentWordsSpanishU);
    }

    return resultList;
}

QList<QString> OQString::getBrotherReferences(const LANGUAGE language) const
{
    switch(language)
    {
    case english:
        return brotherWordsEnglish;

    case french:
        return brotherWordsFrench;

    case spanish:
        return brotherWordsSpanish;

    default:
        return brotherWordsEnglish + brotherWordsFrench + brotherWordsSpanish;
    }
}

QList<QString> OQString::getSpousalReferences(const LANGUAGE lang, GENDER gender) const
{
    QList<QString> resultList;

    if ((lang != french) && (lang != spanish))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(spousalReferencesEnglishM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(spousalReferencesEnglishF);

        resultList.append(spousalReferencesEnglishU);
    }

    if ((lang != english) && (lang != spanish))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(spousalReferencesFrenchM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(spousalReferencesFrenchF);

        resultList.append(spousalReferencesFrenchU);
    }

    if ((lang != english) && (lang != french))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(spousalReferencesSpanishM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(spousalReferencesSpanishF);

        resultList.append(spousalReferencesSpanishU);
    }

    return resultList;
}

QList<QString> OQString::getSiblingReferences(const LANGUAGE lang, GENDER gender) const
{
    QList<QString> resultList;

    if ((lang != french) && (lang != spanish))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(siblingReferencesEnglishM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(siblingReferencesEnglishF);

        resultList.append(siblingReferencesEnglishU);
    }

    if ((lang != english) && (lang != spanish))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(siblingReferencesFrenchM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(siblingReferencesFrenchF);

        resultList.append(siblingReferencesFrenchU);
    }

    if ((lang != english) && (lang != french))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(siblingReferencesSpanishM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(siblingReferencesSpanishF);

        resultList.append(siblingReferencesSpanishU);
    }

    return resultList;
}

QList<QString> OQString::getChildReferences(const LANGUAGE lang, GENDER gender) const
{
    QList<QString> resultList;

    if ((lang != french) && (lang != spanish))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(childReferencesEnglishM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(childReferencesEnglishF);

        resultList.append(childReferencesEnglishU);
    }

    if ((lang != english) && (lang != spanish))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(childReferencesFrenchM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(childReferencesFrenchF);

        resultList.append(childReferencesFrenchU);
    }

    if ((lang != english) && (lang != french))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(childReferencesSpanishM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(childReferencesSpanishF);

        resultList.append(childReferencesSpanishU);
    }

    return resultList;
}

QList<QString> OQString::getRelativeReferences(const LANGUAGE lang) const
{
    QList<QString> resultList;

    if ((lang != french) && (lang != spanish))
        resultList.append(relativeReferencesEnglish);

    if ((lang != english) && (lang != spanish))
        resultList.append(relativeReferencesFrench);

    if ((lang != english) && (lang != french))
        resultList.append(relativeReferencesSpanish);

    return resultList;
}

QList<QString> OQString::getRelationshipWords(const LANGUAGE lang, GENDER gender) const
{
    QList<QString> resultList;

    if ((lang != french) && (lang != spanish))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(relationshipWordsEnglishM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(relationshipWordsEnglishF);

        resultList.append(relationshipWordsEnglishU);
    }

    if ((lang != english) && (lang != spanish))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(relationshipWordsFrenchM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(relationshipWordsFrenchF);

        resultList.append(relationshipWordsFrenchU);
    }

    if ((lang != english) && (lang != french))
    {
        if ((gender == Male) || (gender == genderUnknown))
            resultList.append(relationshipWordsSpanishM);

        if ((gender == Female) || (gender == genderUnknown))
            resultList.append(relationshipWordsSpanishF);

        resultList.append(relationshipWordsSpanishU);
    }

    return resultList;
}

bool OQString::hasDualMeaning(LANGUAGE language) const
{
    if (itsString.length() == 0)
        return false;

    OQString string(itsString);
    string = string.lower();

    switch(language)
    {
    case english:
        if (string == OQString("will"))
            return true;
        break;

    case french:
        break;

    case spanish:
        break;

    default:
        return string.hasDualMeaning(english) || string.hasDualMeaning(french) || string.hasDualMeaning(spanish);
        break;
    }

    return false;
}


bool OQString::areRelationshipWords(GENDER gender, LANGUAGE language) const
{
    switch(language)
    {
    case english:
        switch(gender)
        {
        case Male:
            return areEnglishMaleRelationshipWords() || areEnglishUnisexRelationshipWords();
            break;

        case Female:
            return areEnglishFemaleRelationshipWords() || areEnglishUnisexRelationshipWords();
            break;

        case genderUnknown:
            return areEnglishMaleRelationshipWords() || areEnglishFemaleRelationshipWords() || areEnglishUnisexRelationshipWords();
        }
        break;

    case french:
        switch(gender)
        {
        case Male:
            return areFrenchMaleRelationshipWords() || areFrenchUnisexRelationshipWords();
            break;

        case Female:
            return areFrenchFemaleRelationshipWords() || areFrenchUnisexRelationshipWords();
            break;

        case genderUnknown:
            return areFrenchMaleRelationshipWords() || areFrenchFemaleRelationshipWords() || areFrenchUnisexRelationshipWords();
        }
        break;

    case spanish:
        switch(gender)
        {
        case Male:
            return areSpanishMaleRelationshipWords() || areSpanishUnisexRelationshipWords();
            break;

        case Female:
            return areSpanishFemaleRelationshipWords() || areSpanishUnisexRelationshipWords();
            break;

        case genderUnknown:
            return areSpanishMaleRelationshipWords() || areSpanishFemaleRelationshipWords() || areSpanishUnisexRelationshipWords();
        }
        break;

    default:
        switch(gender)
        {
        case Male:
            return areEnglishMaleRelationshipWords() || areFrenchMaleRelationshipWords() || areSpanishMaleRelationshipWords() ||
                   areEnglishUnisexRelationshipWords() || areFrenchUnisexRelationshipWords() || areSpanishUnisexRelationshipWords();
            break;

        case Female:
            return areEnglishFemaleRelationshipWords() || areFrenchFemaleRelationshipWords() || areSpanishFemaleRelationshipWords() ||
                   areEnglishUnisexRelationshipWords() || areFrenchUnisexRelationshipWords() || areSpanishUnisexRelationshipWords();
            break;

        case genderUnknown:
            return areEnglishMaleRelationshipWords() || areEnglishFemaleRelationshipWords() ||
                   areFrenchMaleRelationshipWords()  || areFrenchFemaleRelationshipWords()  ||
                   areSpanishMaleRelationshipWords() || areSpanishFemaleRelationshipWords() ||
                   areEnglishUnisexRelationshipWords() || areFrenchUnisexRelationshipWords() ||
                   areSpanishUnisexRelationshipWords();
        }
        break;
    }

    return false;
}

bool OQString::areEnglishRelationshipWords() const
{
    return areEnglishMaleRelationshipWords() || areEnglishFemaleRelationshipWords() || areEnglishUnisexRelationshipWords();
}

bool OQString::areEnglishMaleRelationshipWords() const
{
    return isFoundIn(relationshipWordsEnglishM, 2);
}

bool OQString::areEnglishFemaleRelationshipWords() const
{
    return isFoundIn(relationshipWordsEnglishF, 2);
}

bool OQString::areEnglishUnisexRelationshipWords() const
{
    return isFoundIn(relationshipWordsEnglishU, 2);
}

bool OQString::areFrenchRelationshipWords() const
{
    return areFrenchMaleRelationshipWords() || areFrenchFemaleRelationshipWords() || areFrenchUnisexRelationshipWords();
}

bool OQString::areFrenchMaleRelationshipWords() const
{
    return isFoundIn(relationshipWordsFrenchM, 2);
}

bool OQString::areFrenchFemaleRelationshipWords() const
{
    return isFoundIn(relationshipWordsFrenchF, 2);
}

bool OQString::areFrenchUnisexRelationshipWords() const
{
    return isFoundIn(relationshipWordsFrenchU, 2);
}

bool OQString::areSpanishRelationshipWords() const
{
    return areSpanishMaleRelationshipWords() || areSpanishFemaleRelationshipWords() || areSpanishUnisexRelationshipWords();
}

bool OQString::areSpanishMaleRelationshipWords() const
{
    return isFoundIn(relationshipWordsSpanishM, 2);
}

bool OQString::areSpanishFemaleRelationshipWords() const
{
    return isFoundIn(relationshipWordsSpanishF, 2);
}

bool OQString::areSpanishUnisexRelationshipWords() const
{
    return isFoundIn(relationshipWordsSpanishU, 2);
}

QUrl OQString::asQUrl()
{
    QUrl qUrl(itsString);
    if (qUrl.isValid())
        return qUrl;
    else
        return QUrl();
}

QList<monthInfo> OQString::monthsEnglishFull = QList<monthInfo> ()
    << monthInfo("january", 1 )
    << monthInfo("february", 2 )
    << monthInfo("march", 3 )
    << monthInfo("april", 4 )
    << monthInfo("may", 5 )
    << monthInfo("june", 6 )
    << monthInfo("july", 7 )
    << monthInfo("august", 8 )
    << monthInfo("september", 9 )
    << monthInfo("october", 10 )
    << monthInfo("november", 11 )
    << monthInfo("december", 12 );

QList<monthInfo> OQString::monthsEnglishAbbreviated = QList<monthInfo> ()
    << monthInfo("sept", 9 )
    << monthInfo("jan", 1 )
    << monthInfo("feb", 2 )
    << monthInfo("mar", 3 )
    << monthInfo("apr", 4 )
    << monthInfo("may", 5 )
    << monthInfo("jun", 6 )
    << monthInfo("jul", 7 )
    << monthInfo("aug", 8 )
    << monthInfo("sep", 9 )
    << monthInfo("oct", 10 )
    << monthInfo("nov", 11 )
    << monthInfo("dec", 12 );

QList<monthInfo> OQString::monthsFrenchFull = QList<monthInfo> ()
    << monthInfo("janvier", 1 )
    << monthInfo("fevrier", 2 )
    << monthInfo("mars", 3 )
    << monthInfo("avril", 4 )
    << monthInfo("mai", 5 )
    << monthInfo("juin", 6 )
    << monthInfo("juillet", 7 )
    << monthInfo("aout", 8 )
    << monthInfo("septembre", 9 )
    << monthInfo("octobre", 10 )
    << monthInfo("novembre", 11 )
    << monthInfo("decembre", 12 )
    << monthInfo("février", 2 )
    << monthInfo("août", 8 )
    << monthInfo("décembre", 12 );

QList<monthInfo> OQString::monthsFrenchAbbreviated = QList<monthInfo> ()
        << monthInfo("jan", 1 )
        << monthInfo("janv", 1 )
        << monthInfo("fév", 2 )
        << monthInfo("févr", 2 )
        << monthInfo("mar", 3 )
        << monthInfo("avr", 4 )
        << monthInfo("juil", 7 )
        << monthInfo("sep", 9 )
        << monthInfo("sept", 9 )
        << monthInfo("oct", 10 )
        << monthInfo("nov", 11 )
        << monthInfo("déc", 12 );

QList<monthInfo> OQString::monthsSpanishFull = QList<monthInfo> ()
    << monthInfo("enero", 1 )
    << monthInfo("febrero", 2 )
    << monthInfo("marzo", 3 )
    << monthInfo("abril", 4 )
    << monthInfo("mayo", 5 )
    << monthInfo("junio", 6 )
    << monthInfo("julio", 7 )
    << monthInfo("agosto", 8 )
    << monthInfo("septiembre", 9 )
    << monthInfo("octubre", 10 )
    << monthInfo("noviembre", 11 )
    << monthInfo("diciembre", 12 );

QList<monthInfo> OQString::monthsSpanishAbbreviated = QList<monthInfo> ()
        << monthInfo("feb", 2 )
        << monthInfo("abr", 4 )
        << monthInfo("jun", 6 )
        << monthInfo("jul", 7 )
        << monthInfo("set", 9 )
        << monthInfo("sept", 9 )
        << monthInfo("oct", 10 )
        << monthInfo("nov", 11 )
        << monthInfo("dec", 12 );

QList<dayOfWeekInfo> OQString::daysOfWeekEnglish = QList<dayOfWeekInfo> ()
    << dayOfWeekInfo("sunday", 1 )
    << dayOfWeekInfo("monday", 2 )
    << dayOfWeekInfo("tuesday", 3 )
    << dayOfWeekInfo("wednesday", 4 )
    << dayOfWeekInfo("thursday", 5 )
    << dayOfWeekInfo("friday", 6 )
    << dayOfWeekInfo("saturday", 7 );

QList<dayOfWeekInfo> OQString::daysOfWeekFrench = QList<dayOfWeekInfo> ()
    << dayOfWeekInfo("dimanche", 1 )
    << dayOfWeekInfo("lundi", 2 )
    << dayOfWeekInfo("mardi", 3 )
    << dayOfWeekInfo("mecredi", 4 )
    << dayOfWeekInfo("jeudi", 5 )
    << dayOfWeekInfo("vendredi", 6 )
    << dayOfWeekInfo("samedi", 7 );

QList<dayOfWeekInfo> OQString::daysOfWeekSpanish = QList<dayOfWeekInfo> ()
    << dayOfWeekInfo("domingo", 1 )
    << dayOfWeekInfo("lunes", 2 )
    << dayOfWeekInfo("martes", 3 )
    << dayOfWeekInfo("miércoles", 4 )
    << dayOfWeekInfo("jueves", 5 )
    << dayOfWeekInfo("viernes", 6 )
    << dayOfWeekInfo("sábado", 7 )
    << dayOfWeekInfo("miercoles", 4 )
    << dayOfWeekInfo("sabado", 7 );

QList<QString> OQString::provAbbreviations  = QList<QString> () << "ab" << "bc" << "mb" << "nb" << "nf" << "nfld" << "nl" << "ns" << "on" << "pe" << "pei" << "qc" << "sask" << "sk";
QList<QString> OQString::provLong           = QList<QString> () << "alberta" << "british columbia" << "colombie britannique" << "ile du prince eduoard"
                                                                << "manitoba" << "new brunswick" << "newfoundland" << "nouveau brunswick" << "nouvelle ecosse" << "nova scotia" << "ontario"
                                                                << "prince edward island" << "quebec" << "saskatchewan" << "terreneuve";
QList<QString> OQString::otherAbbreviations = QList<QString> () << "fr" << "mt" << "sts";

QList<QString> OQString::aboriginalNames = QList<QString> () << "alone" << "bad" << "bare" << "bear" << "bears" << "big" << "bird" << "black" << "bone" << "bow" << "brave" << "bruised" << "bull"
                                                             << "calf" << "canoe" << "cat" << "chief" << "child" << "crazy" << "crop" << "cross" << "crow" << "day" << "duck"
                                                             << "eagle" << "eared" << "face" << "feathers" << "fingers" << "first" << "good" << "grass" << "grey" << "guns"
                                                             << "hawk" << "head" << "heavy" << "horn" << "horses" << "hungry" << "iron" << "legs" << "light" << "little" << "long" << "low"
                                                             << "man" << "many" << "melting" << "mistaken" << "moccasin" << "moon" << "morning" << "no" << "north" << "old"
                                                             << "panther" << "peigan" << "pine" << "plain" << "plume" << "quills" << "red" << "rider" << "robe" << "runner" << "running"
                                                             << "shin" << "shoes" << "shots" << "sitting" << "sky" << "skies" << "small" << "smoke" << "speaker" << "spear" << "spotted" << "squirrel"
                                                             << "standing" << "strangling" << "striker" << "striped" << "sun" << "swallow" << "swan" << "sweet"
                                                             << "tail" << "tallow" << "tent" << "three" << "throat" << "time" << "turning" << "two" << "water" << "weasel" << "wolf" << "yellow" << "young";
QList<QString> OQString::problematicAboriginalNames = QList<QString> ()    << "bird" << "black" << "cross" << "day" << "good" << "grey" << "hawk" << "head" << "iron" << "little" << "north" << "pine"
                                                                           << "small" << "sweet" << "wolf" << "young";
QList<QString> OQString::noVowelNames = QList<QString> ()    << "lynn";

QList<QString> OQString::altNameIndicators  = QList<QString> () << "aka" << "born" << "dit" << "dite" << "formerly" << "nee"
                                                                << "neé" << "née";
QList<QString> OQString::changedContextIndicators = QList<QString> () << "attended" << "battling" << "came" << "emigrated" << "grade" << "graduated" << "immigrated" << "married" << "moved" << "moving"
                                                                      << "relocated" << "returned" << "served" << "settled" << "spent" << "until" << "worked";
QList<QString> OQString::endOfBlockTags    = QList<QString> () << "</p>" << "</div>" << "<br>" << "<br />" << "<br/>" << "</h1>" << "</h2>" << "</h3>" << "</h4>" << "</h5>" << "</h6>";

QList<QString> OQString::genderWordsEnglishM = QList<QString> () << "he" << "his";
QList<QString> OQString::genderWordsEnglishF = QList<QString> () << "she" << "her";
QList<QString> OQString::genderWordsFrenchM  = QList<QString> () << "il" << "qu'il";
QList<QString> OQString::genderWordsFrenchF  = QList<QString> () << "elle" << "qu'elle";
QList<QString> OQString::genderWordsSpanishM = QList<QString> () << "el" << "su" << "él";
QList<QString> OQString::genderWordsSpanishF = QList<QString> () << "ella" << "su";
QList<QString> OQString::pronouns = QList<QString> () << "el" << "ella" << "elle" << "he" << "il" << "she";

QList<QString> OQString::godReferences   = QList<QString> ()  << "dieu" << "god" << "lord" << "seigneur";
QList<QString> OQString::locations       = QList<QString> ()  << "beaverbank" << "bedford" << "blytheswood" << "dartmouth"
                                                              << "eastern" << "halifax" << "port" << "porters" << "sambro"
                                                              << "st. john's";
QList<QString> OQString::routes          = QList<QString> ()  << "ave" << "avenue" << "blvd" << "boulevard" << "cres" << "crescent" << "dr" << "drive" << "hwy" << "highway" << "rd" << "road" << "st" << "street";

QList<QString> OQString::femaleTitlesEnglish   = QList<QString> () << "miss" << "mlle" << "mrs" << "ms" ;
QList<QString> OQString::femaleTitlesFrench    = QList<QString> () << "dame" << "madame" << "mademoiselle" << "mlle" << "mme";
QList<QString> OQString::femaleTitlesSpanish   = QList<QString> () << "señora" << "sra";
QList<QString> OQString::maleTitlesEnglish     = QList<QString> () << "mr" << "mst";
QList<QString> OQString::maleTitlesFrench      = QList<QString> () << "m" << "monsieur" << "mssr";
QList<QString> OQString::maleTitlesSpanish     = QList<QString> () << "senor" << "sr";

QList<QString> OQString::neeEtAlwords    = QList<QString> () << "born" << "formerly" << "maiden" << "n" << "nee"
                                                             << "neé" << "né" << "née" << "nèe";
QList<QString> OQString::ordinalsEnglish = QList<QString> () << "st" << "nd" << "rd" << "th";
QList<QString> OQString::ordinalsFrench  = QList<QString> () << "ieme" << "iere"  << "ième" << "ière";
QList<QString> OQString::ordinalsSpanish = QList<QString> () << "a" << "er";

QList<QString> OQString::precedingIndicators = QList<QString> () << "before attaining" << "before her" << "before his" << "during her" << "during his" << "in her" << "in his"
                                                                 << "on her" << "on his" << "prior to" << "short of" << "shy of";

QList<QString> OQString::prefixesAbbreviatedEnglish = QList<QString> () << "aalu" << "adm" << "arch" << "bish" << "br"
                                                                        << "capt" << "cdr" << "cdt" << "cfp" << "clu" << "cmdr" << "cnd" << "col" << "cpl" << "cpo" << "cpsm" << "cpt" << "cst" << "csv" << "cwo"
                                                                        << "dc" << "dr" << "facog" << "fcsse" << "feic" << "flmi" << "flt" << "fr" << "frcs" << "gen" << "gov"
                                                                        << "hon" << "lcdr" << "lcol" << "llb" << "lt"
                                                                        << "maj" << "mcpl" << "mdiv" << "mgr" << "msgr" << "mwa" << "mwo" << "ofm" << "omm" << "prof" << "pssf" << "pvt"
                                                                        << "raf" << "rcaf" << "rcmp" << "rcn" << "rep"  << "rev" << "rjm" << "rn" << "rsm" << "rsr" << "rt" << "rtd"
                                                                        << "sasv" << "scsl" << "sen" << "sgt" << "sj" << "ssccjm" << "wo";
QList<QString> OQString::prefixesAbbreviatedFrench  = QList<QString> () << "dre" << "osu" << "rsr";
QList<QString> OQString::prefixesAbbreviatedSpanish = QList<QString> () << "testest";

QList<QString> OQString::prefixesFullEnglish = QList<QString> () << "admiral" << "archbishop" << "baby" << "bishop" << "brigadier" << "brother"
                                                                 << "cadet" << "canon" << "captain" << "colonel" << "commander" << "constable" << "corporal"
                                                                 << "deacon" << "doctor" << "father" << "flt" << "frère" << "general" << "governor"
                                                                 << "honorable" << "honourable" << "justice" << "lieutenant"
                                                                 << "madame" << "major" << "master" << "monsignor" << "officer" << "padre" << "pastor" << "private" << "professor"
                                                                 << "retired" << "reverend" << "right"
                                                                 << "sargent" << "senator" << "sergeant" <<  "sister" << "staff" << "submariner" << "the" << "veteran" << "warrant";
QList<QString> OQString::prefixesFullFrench  = QList<QString> () << "abbé" << "docteur" << "frère" << "l'abbe" << "l'abbé"
                                                                 << "l'honorable" << "madame" << "monsieur" << "père" << "soeur" << "sœur";
QList<QString> OQString::prefixesFullSpanish = QList<QString> () << "testtest";

QList<QString> OQString::problematicFirstNames = QList<QString> () << "an";
QList<QString> OQString::problematicFemaleMiddleNames = QList<QString> () << "ann" << "anne" << "elizabeth" << "jane" << "jean" << "joyce"
                                                                          << "lee" << "lou" << "lynn" << "may" << "rose" << "shannon";

QList<QString> OQString::saints          = QList<QString> () << "saint" << "st" << "st." << "ste" << "ste.";

QList<QString> OQString::suffixesDegree  = QList<QString> () << "a" << "comm" << "ed" << "phys" << "s" << "sc" << "voc";  // Preceded by "B" or "M"

QList<QString> OQString::suffixesDropEnglish    = QList<QString> () << "acii" << "ba" << "bcomm" << "bed" << "bmsc" << "bpe"
                                                                    << "bs" << "bsc" << "bscn" << "bve" << "bvoced"
                                                                    << "ca" << "cd" << "cds" <<  "cfc" << "cga" << "cm" << "cma" << "cmm" << "cpa" << "crcs" << "crm" << "csb" << "csc" << "csj" << "cssr" << "cvo"
                                                                    << "dds" << "dip" << "doctor" << "dsm" << "dvm" << "eng" << "esq"
                                                                    << "facs" << "fca" << "fcahs" <<  "fcia" << "fcscj" << "fiic" << "fj" << "fma" << "fr" << "frcp" << "frcpc" << "frcsc" << "fsa" << "gclj" << "gm"
                                                                    << "honors" << "honours" << "ing" << "juris" << "kstj"
                                                                    << "llb" << "lld" << "llm" << "masc" << "mba" << "md" << "med" << "meng" << "mgr" << "msc" << "ndsc"
                                                                    << "obgyn" << "ocad" << "offm" << "ohc" << "olm" << "omi" << "omri" << "onl" << "osb" << "osbm"<< "pc" << "peng" << "pgeo" << "ph" << "phd"
                                                                    << "qc" << "qffq" << "rcmp" << "ret" << "ret'd" << "retd" << "rev" << "rn" << "rndm" << "sdb" << "sgm" << "ssm" << "ssmi"
                                                                    << "ue" << "wwi" << "wwii";
QList<QString> OQString::suffixesDropFrench     = suffixesDropEnglish;
QList<QString> OQString::suffixesDropSpanish    = QList<QString> () << "testtest";

QList<QString> OQString::suffixesKeepEnglish    = QList<QString> () <<  "i" << "ii" << "iii" << "iv"
                                                                    <<  "jr" << "junior" << "senior" << "sr";
QList<QString> OQString::suffixesKeepFrench     = QList<QString> () <<  "testtest";
QList<QString> OQString::suffixesKeepSpanish    = QList<QString> () <<  "testtest";

QList<QString> OQString::uncapitalizedNames  = QList<QString> () << "cinq" << "da" << "dalla" << "das" << "de" << "del" << "dela" << "den" << "der" << "des"
                                                                 << "di" << "do" << "dos" << "du" << "dè" << "e" << "el"
                                                                 << "la" << "le" << "lo" << "los" << "san" << "st" << "st." << "te" << "ten" << "ter"
                                                                 << "van" << "van't" << "vande" << "vanden" << "vander" << "von" ;
QList<QString> OQString::singleVans          = QList<QString> () << "du" << "ho" << "hoan" << "le" << "luong" << "nguyen" << "pham" << "phan" << "tran" << "vo";
QList<QString> OQString::hyphenatedNameBeginnings = QList<QString> () << "abu" << "ad" << "al" << "el" << "ud";

QList<QString> OQString::ignoreWordsEnglish1 = QList<QString> () << "aa";
QList<QString> OQString::ignoreWordsEnglish2 = QList<QString> () << "as" << "at" << "by" << "if" << "in" << "is" << "it"
                                                                 << "my" << "no" << "of" << "on" << "or" << "to" << "us" << "we";
QList<QString> OQString::ignoreWordsEnglish3 = QList<QString> () << "age" << "all" << "and" << "are" << "but" << "cow" << "dad" << "due" << "for"
                                                                 << "had" << "has" << "its" << "mom" << "new" << "not" << "off" << "old" << "our" << "sad" << "say"
                                                                 << "the" << "was" << "way" << "who" << "you";
QList<QString> OQString::ignoreWordsEnglish4 = QList<QString> () << "aged" << "also" << "away" << "best" << "born" << "care" << "dada" << "dadi" << "dear" << "deep" << "died" << "down" << "from" << "gift" << "good"
                                                                 << "have" << "held" << "here" << "high" << "home" << "into" << "late" << "left" << "life" << "lock" << "lost"
                                                                 << "made" << "many" << "mass" << "miss" << "nana" << "nani" << "near" << "once" << "ones" << "papa"
                                                                 << "rest" << "safe" << "sale" << "side" << "that" << "they" << "this" << "urne"
                                                                 << "wall" << "went" << "when" << "wife" << "with" << "year" << "your";
QList<QString> OQString::ignoreWordsEnglish5 = QList<QString> () << "after" << "began" << "civil" << "click" << "covid" << "death" << "early" << "ended" << "first" << "grand" << "great" << "happy" << "honey"
                                                                 << "known" << "lodge" << "loved" << "lower" << "never" << "peace" << "phone" << "place" << "royal"
                                                                 << "sadly" << "spent" << "story" << "thank" << "there" << "these" << "third" << "those" << "uncle"
                                                                 << "watch" << "while" << "would" << "years";
QList<QString> OQString::ignoreWordsEnglish6 = QList<QString> () << "access" << "active" << "adored" << "always" << "angels" << "auntie" << "badger" << "battle" << "better" << "behind" << "broken" << "burial"
                                                                 << "candle" << "caring" << "change" << "chapel" << "christ" << "dearly" << "divine" << "doting" << "during" << "earned" << "easter"
                                                                 << "family" << "father" << "former" << "friday" << "friend" << "garden"
                                                                 << "health" << "hearts" << "joined" << "leaves" << "loving"
                                                                 << "memory" << "missed" << "monday" << "mother" << "moving"
                                                                 << "online" << "parish" << "passed" << "please" << "public"
                                                                 << "second" << "sister" << "sorrow" << "source" << "sunday" << "thanks"
                                                                 << "walked" << "wishes" << "worked";
QList<QString> OQString::ignoreWordsEnglish7 = QList<QString> () << "adoring" << "beloved" << "brother" << "crossed" << "details" << "devoted" << "drifted"
                                                                 << "entered" << "eternal" << "flowers" << "forever" << "friends" << "funeral"
                                                                 << "goodbye" << "grandma" << "grandpa" << "healing" << "hospice" << "husband" << "illness"
                                                                 << "journey" << "married" << "outlaws" << "passing" << "prayers" << "private" << "retired" << "sadness" << "servant" << "service" << "special"
                                                                 << "through" << "tuesday" << "updated" << "quietly";
QList<QString> OQString::ignoreWordsEnglish8 = QList<QString> () << "although" << "announce" << "ceremony" << "children" << "daughter" << "deceased" << "departed" << "donation"
                                                                 << "formerly" << "hospital"
                                                                 << "lovingly" << "measures" << "memorial" << "memoriam" << "memories" << "monument" << "obituary" << "password" << "peaceful" << "precious"
                                                                 << "rejoined" << "resident" << "response" << "reunited" << "saturday" << "silently" << "suddenly" << "survived"
                                                                 << "thursday" << "together" << "tomorrow" << "visiting";
QList<QString> OQString::ignoreWordsEnglish9 = QList<QString> () << "announces" << "beautiful" << "cherished" << "christmas" << "cremation" << "departure" << "donations" << "everybody" << "following"
                                                                 << "heartfelt" << "interment" << "inurnment" << "long-time" << "professor"
                                                                 << "relatives" << "wednesday" ;
QList<QString> OQString::ignoreWordsEnglish10 = QList<QString> () << "celebrated" << "courageous" << "graciously" << "internment" << "livestream" << "originally" << "peacefully"
                                                                  << "registered" << "remembered" << "surrounded" << "tragically" << "visitation" ;
QList<QString> OQString::ignoreWordsEnglish11 = QList<QString> () << "brotherhood" << "candlelight" << "columbarium" << "condolences" << "expressions" << "grandfather"
                                                                  << "grandmother" << "information" << "predeceased" << "remembering" << "remembrance";
QList<QString> OQString::ignoreWordsEnglish12 = QList<QString> () << "arrangements" << "lightservice" << "unexpectedly";
QList<QString> OQString::ignoreWordsEnglish13 = QList<QString> () << "announcements" << "grandchildren" << "international";
QList<QString> OQString::ignoreWordsEnglish14 = QList<QString> () << "affectionately";

QList<QString> OQString::ignoreWordsFrench1 = QList<QString> () << "à";
QList<QString> OQString::ignoreWordsFrench2 = QList<QString> () << "au" << "dû" << "en" << "et" << "le" << "ou" << "tu" << "un";
QList<QString> OQString::ignoreWordsFrench3 = QList<QString> () << "des" << "les" << "que" << "ses" << "une" << "vos";
QList<QString> OQString::ignoreWordsFrench4 = QList<QString> () << "afin" << "avec" << "ceux" << "chez" << "dans" << "nous" << "pour" << "sera" << "tous" << "tout" << "vous";
QList<QString> OQString::ignoreWordsFrench5 = QList<QString> () << "achat" << "ainsi" << "année" << "c'est" << "cette" << "deuil" << "messe" << "notre" << "oncle" << "outre" << "salon"
                                                                << "selon" << "tante" << "toute" << "votre" << "épouse" << "époux" << "étant";
QList<QString> OQString::ignoreWordsFrench6 = QList<QString> () << "compte" << "grande" << "toutes" << "église";
QList<QString> OQString::ignoreWordsFrench7 = QList<QString> () << "c'était" << "détails" << "famille";
QList<QString> OQString::ignoreWordsFrench8 = QList<QString> () << "complexe" << "souvenir" << "toujours" << "veuillez";
QList<QString> OQString::ignoreWordsFrench9 = QList<QString> () << "l'hôpital" << "tristesse";
QList<QString> OQString::ignoreWordsFrench10 = QList<QString> () << "prédécédée" << "subitement";
QList<QString> OQString::ignoreWordsFrench11 = QList<QString> () << "considérant";
QList<QString> OQString::ignoreWordsFrench12 = QList<QString> () << "condoléances" << "conformément";
QList<QString> OQString::ignoreWordsFrench13 = QList<QString> () << "veuillez-vous";
QList<QString> OQString::ignoreWordsFrench14 = QList<QString> () << "éventuellement";

QList<QString> OQString::ignoreWordsSpanish1 = QList<QString> () << "dummy";
QList<QString> OQString::ignoreWordsSpanish2 = QList<QString> () << "spanishdummy";
QList<QString> OQString::ignoreWordsSpanish3 = QList<QString> () << "dummy";
QList<QString> OQString::ignoreWordsSpanish4 = QList<QString> () << "dummy";
QList<QString> OQString::ignoreWordsSpanish5 = QList<QString> () << "dummy";
QList<QString> OQString::ignoreWordsSpanish6 = QList<QString> () << "dummy";
QList<QString> OQString::ignoreWordsSpanish7 = QList<QString> () << "dummy";
QList<QString> OQString::ignoreWordsSpanish8 = QList<QString> () << "dummy";
QList<QString> OQString::ignoreWordsSpanish9 = QList<QString> () << "dummy";
QList<QString> OQString::ignoreWordsSpanish10 = QList<QString> () << "dummy";
QList<QString> OQString::ignoreWordsSpanish11 = QList<QString> () << "dummy";
QList<QString> OQString::ignoreWordsSpanish12 = QList<QString> () << "dummy";
QList<QString> OQString::ignoreWordsSpanish13 = QList<QString> () << "internationyy";
QList<QString> OQString::ignoreWordsSpanish14 = QList<QString> () << "internationyyy";

QList<QString> OQString::ignoreFirstWords1 = QList<QString> () << "i";
QList<QString> OQString::ignoreFirstWords2 = QList<QString> () << "an" << "do" << "he" << "my" << "ne" << "no" << "nw" << "oh" << "on" << "or" << "pa" << "sa" << "se" << "so" << "sw" << "to" << "up";
QList<QString> OQString::ignoreFirstWords3 = QList<QString> () << "any" << "box" << "due" << "est" << "get" << "god" << "how" << "ils" << "i'd" << "i'm" << "let" << "mum" << "n.e" << "n.w" << "not" << "now" << "one"
                                                               << "rip" << "s.e" << "s.w" << "see" << "she" << "sun" << "tel" << "til" << "two" << "tél" << "web" << "yet";
QList<QString> OQString::ignoreFirstWords4 = QList<QString> () << "aunt" << "back" << "both" << "call" << "care" << "each" << "even" << "ever" << "fils" << "find" << "fond" << "give" << "golf" << "gone" << "good"
                                                               << "half" << "hall" << "here" << "holy" << "i'll" << "join" << "just" << "kind" << "last" << "like" << "live" << "long" << "love"
                                                               << "mama" << "mass" << "more" << "most" << "much" << "need" << "over" << "park" << "part" << "post" << "read" << "rest" << "sign" << "some" << "soon" << "such"
                                                               << "text" << "then" << "thus" << "time" << "upon" << "view" << "wake" << "we'd" << "weep" << "what";
QList<QString> OQString::ignoreFirstWords5 = QList<QString> () << "above" << "again" << "aloha" << "along" << "among" << "après" << "ashes" << "aside" << "aunty" << "being"
                                                               << "cease" << "creek" << "don't" << "dream" << "early" << "email" << "enjoy" << "enter" << "every" << "femme" << "fille"
                                                               << "given" << "happy" << "heart" << "homme" << "later" << "maman" << "merci" << "offer" << "order" << "other" << "phone" << "print" << "prior" << "proud" << "r.i.p" << "right"
                                                               << "sadly" << "salon" << "share" << "since" << "suite" << "their" << "think" << "three" << "today" << "uncle" << "under" << "until" << "visit" << "we're" << "where" << "words";
QList<QString> OQString::ignoreFirstWords6 = QList<QString> () << "adjust" << "anyone" << "asleep" << "before" << "beside" << "beyond" << "called" << "cancer" << "casual" << "centre" << "chapel" << "cheers" << "church" << "clergy" << "deeply"
                                                               << "except" << "filles" << "floral" << "fondly" << "gently" << "guests" << "having" << "heures" << "highly" << "hiking" << "membre" << "people" << "photos" << "prayer" << "public"
                                                               << "rather" << "repose" << "réseau" << "salons" << "should" << "suivra" << "taking" << "things" << "though" << "uncles" << "within"
                                                               << "épouse" << "époux";
QList<QString> OQString::ignoreFirstWords7 = QList<QString> () << "adopted" << "against" << "another" << "baptist" << "because" << "besides" << "between" << "blessed" << "chappel" << "cheques" << "contact" << "cousins" << "dearest" << "deepest" << "despite"
                                                               << "finally" << "forward" << "funeral" << "further" << "golfing" << "goodbye" << "greatly" << "growing" << "happily" << "however" << "instead" << "joining" << "keeping"
                                                               << "leaving" << "liturgy" << "located" << "married" << "nothing" << "parents" << "prairie" << "promise" << "resting" << "retired" << "romance"
                                                               << "service" << "shortly" << "sincere" << "singing" << "summers" << "sunrise" << "viewing" << "webcast" << "website" << "whether";
QList<QString> OQString::ignoreFirstWords8 = QList<QString> () << "awaiting" << "becoming" << "cemetery" << "deceased" << "dementia" << "entering" << "extended" << "everyone" << "farewell" << "good-bye" << "grandson" << "grateful"
                                                               << "heritage" << "honoring" << "involved" << "lifelong" << "location" << "longtime" << "marriage" << "memorial" << "memories" << "messages" << "moreover" << "mourning"
                                                               << "numerous" << "profound" << "remember" << "retiring";
QList<QString> OQString::ignoreFirstWords9 = QList<QString> () << "according" << "afternoon" << "attention" << "cherished" << "countless" << "cremation" << "customers" << "cérémonie"
                                                               << "dedicated" << "diagnosed" << "direction" << "donations" << "extremely" << "gathering" << "graveside" << "honouring" << "initially" << "placement"
                                                               << "published" << "reception" << "résidence" << "returning" << "surviving" << "sustained" << "telephone" << "therefore" << "undaunted";
QList<QString> OQString::ignoreFirstWords10 = QList<QString> () << "additional" << "afterwards" << "challenges" << "charitable" << "cremations" << "entombment" << "eventually" << "everything" << "foundation" << "graduating"
                                                                << "ironically" << "registered" << "respecting" << "retirement" << "thankfully" << "thereafter" << "throughout";
QList<QString> OQString::ignoreFirstWords11 = QList<QString> () << "association" << "celebration" << "condolences" << "crematorium" << "immigrating" << "traditional" ;
QList<QString> OQString::ignoreFirstWords12 = QList<QString> () << "additionally" << "appreciating" << "appeciation" << "celebrations" << "l'inhumation" << "respectfully" << "subsequently";
QList<QString> OQString::ignoreFirstWords13 = QList<QString> () << "contributions" << "remerciements" << "sister-in-law" << "stationnement" << "unfortunately";
QList<QString> OQString::ignoreFirstWords14 = QList<QString> () << "brother-in-law";


// Relationship words need spaces for QString::compare. Ordered (by length) such that combinations starting with otherRelationshipWords
// are more or less last in the list
QList<QString> OQString::relationshipWordsEnglishM = QList<QString> () << "join his "
                                                                       << "rejoin his "
                                                                       << "to mourn wife "
                                                                       << "missed by his "
                                                                       << "mourned by his "
                                                                       << "to cherish him "
                                                                       << "remembering him"
                                                                       << "survived by his "
                                                                       << "cherished by his "
                                                                       << "survived by wife "
                                                                       << "leaves behind his "
                                                                       << "remembered by his "
                                                                       << "surrounded by his "
                                                                       << "remembered by wife "
                                                                       << "predeceased by his "
                                                                       << "predeceased by wife ";
QList<QString> OQString::relationshipWordsEnglishF = QList<QString> () << "join her "
                                                                       << "rejoin her "
                                                                       << "missed by her "
                                                                       << "mourned by her "
                                                                       << "to cherish her "
                                                                       << "remembering her"
                                                                       << "survived by her "
                                                                       << "cherished by her "
                                                                       << "to mourn husband "
                                                                       << "leaves behind her "
                                                                       << "remembered by her "
                                                                       << "surrounded by her "
                                                                       << "predeceased by her "
                                                                       << "survived by husband "
                                                                       << "remembered by husband "
                                                                       << "predeceased by husband ";
QList<QString> OQString::relationshipWordsEnglishU = QList<QString> () << "left with "
                                                                       << "missed by "
                                                                       << "married to "
                                                                       << "mourned by "
                                                                       << "left behind "
                                                                       << "survived by "
                                                                       << "cherished by "
                                                                       << "leaves behind "
                                                                       << "remembered by "
                                                                       << "surrounded by "
                                                                       << "left to adore "
                                                                       << "left to mourn "
                                                                       << "leaves to mourn "
                                                                       << "left to cherish "
                                                                       << "in the hearts of "
                                                                       << "leaving to mourn "
                                                                       << "in the presence of "
                                                                       << "leaving behind to cherish ";
QList<QString> OQString::relationshipWordsFrenchM  = QList<QString> () << "mari de "
                                                                       << "fils de "
                                                                       << "pere de "
                                                                       << "père de "
                                                                       << "époux de"
                                                                       << "précédé par "
                                                                       << "prédécédé par ";
QList<QString> OQString::relationshipWordsFrenchF  = QList<QString> () << "mere de "
                                                                       << "mère de "
                                                                       << "femme de "
                                                                       << "fille de "
                                                                       << "épouse de "
                                                                       << "précédée par "
                                                                       << "prédécédée par ";
QList<QString> OQString::relationshipWordsFrenchU  = QList<QString> () << "laisse dans le deuil" << "restent pour pleurer" << "époux de";
QList<QString> OQString::relationshipWordsSpanishM = QList<QString> () << "dummy";
QList<QString> OQString::relationshipWordsSpanishF = QList<QString> () << "dummy";
QList<QString> OQString::relationshipWordsSpanishU = QList<QString> () << "dummy";

// The words below are repeats of ignoreWords above
QList<QString> OQString::ageWordsEnglish   = QList<QString> () << "age" << "aged" << "birthday" << "year" << "years" ;
QList<QString> OQString::ageWordsFrench    = QList<QString> () << "age" << "an" << "anniversaire" << "ans" << "l'age" << "l'âge" << "âge";
QList<QString> OQString::ageWordsSpanish   = QList<QString> () << "años" << "cumpleaños" << "edad";

QList<QString> OQString::birthWordsEnglish = QList<QString> () << "born" << "entered";
QList<QString> OQString::birthWordsFrench  = QList<QString> () << "nee" << "né" << "née";
QList<QString> OQString::birthWordsSpanish = QList<QString> () << "nacido";

QList<QString> OQString::deathWordsEnglish = QList<QString> () << "age" << "announce" << "breath" << "called" << "crossed" << "death" << "departed" << "departure" << "died"
                                                               << "entered" << "journeyed" << "left" << "loss" << "lost" << "passed" << "passing" << "peacefully" << "suddenly" << "taken" << "took" << "turned"
                                                               << "went";
QList<QString> OQString::deathWordsFrench  = QList<QString> () << "allé" << "allée" << "décès" << "décès" << "décédé"
                                                               << "décédée" << "départ"
                                                               << "mort" << "morte" << "quitté" << "quittée" << "quittés" << "rejoindre" << "souffle"
                                                               << "éteint" << "éteinte";
QList<QString> OQString::deathWordsSpanish = QList<QString> () << "falleció" << "murió";

QList<QString> OQString::serviceWordsEnglish    = QList<QString> () << "celebration" << "funeral" << "memorial" << "service" << "visitation";
QList<QString> OQString::serviceWordsFrench     = QList<QString> () << "celebration" << "célébration" << "funeraille" << "funéraille" << "messe" << "service";
QList<QString> OQString::serviceWordsSpanish    = QList<QString> () << "funeral" << "servicio";

QList<QString> OQString::otherPersonReferenceWordsEnglish = QList<QString> () << "join" << "joined" << "late" << "leaves" << "leaving"
                                                                              << "missed" << "mourn" << "mourned" << "predeceased"
                                                                              << "rejoined" << "remembered" << "survived";
QList<QString> OQString::otherPersonReferenceWordsFrench  = QList<QString> () << "rejoin";
QList<QString> OQString::otherPersonReferenceWordsSpanish = QList<QString> () << "rejoin";

QList<QString> OQString::otherMiscKeeperWordsEnglish     = QList<QString> () << "by" << "in" << "of" << "to"
                                                                             << "god" << "her" << "his" << "son"
                                                                             << "join" << "wife"
                                                                             << "behind" << "father" << "mother"
                                                                             << "husband"
                                                                             << "daughter";
QList<QString> OQString::otherMiscKeeperWordsFrench      = QList<QString> () << "de"
                                                                             << "fils"
                                                                             << "fille"
                                                                             << "époux"
                                                                             << "épouse";
QList<QString> OQString::otherMiscKeeperWordsSpanish     = QList<QString> () << "de";

QList<QString> OQString::parentWordsEnglishM = QList<QString> () << "his parents " << " son of " << "his father" << "his dad " << "grandson of " << "step-son of";
QList<QString> OQString::parentWordsEnglishF = QList<QString> () << "her parents " << " daughter of " << "her father" << "her dad ";
QList<QString> OQString::parentWordsEnglishU = QList<QString> () << "born to " << " parents" << "child of ";
QList<QString> OQString::parentWordsFrenchM  = QList<QString> () << "fils de ";
QList<QString> OQString::parentWordsFrenchF  = QList<QString> () << "fille de ";
QList<QString> OQString::parentWordsFrenchU  = QList<QString> () << "ses parents" << "enfant de " << "son père";
QList<QString> OQString::parentWordsSpanishM = QList<QString> () << "hijo de ";
QList<QString> OQString::parentWordsSpanishF = QList<QString> () << "hija de ";
QList<QString> OQString::parentWordsSpanishU = QList<QString> () << "sus padres";

QList<QString> OQString::brotherWordsEnglish = QList<QString> () << "brother";
QList<QString> OQString::brotherWordsFrench  = QList<QString> () << "frere" << "frère";
QList<QString> OQString::brotherWordsSpanish = QList<QString> () << "hermano";

QList<QString> OQString::childReferencesEnglishM    = QList<QString> () << "his daughter " << "his son " << "his children" << "father of " << "father to " << "dad to " << "dad of " << "daddy to " << "daddy of " << " pa of " << "pa to " << "father-in-law of ";
QList<QString> OQString::childReferencesEnglishF    = QList<QString> () << "her daughter " << "her son " << "her children" << "mother to " << "mother of " << "mom to " << "mom of " << "mommy to " << "mommy of " << " ma of " << "ma to " << "mother-in-law of ";
QList<QString> OQString::childReferencesEnglishU    = QList<QString> () << "their daughter " << "their son " << "their children " << "children:";
QList<QString> OQString::childReferencesFrenchM     = QList<QString> () << "père à " << "père de " << "papa à " << "papa de ";
QList<QString> OQString::childReferencesFrenchF     = QList<QString> () << "mère à " << "mère de " << "maman à " << "maman de ";
QList<QString> OQString::childReferencesFrenchU     = QList<QString> () << "sa fille " << "son fils " << "ses filles" << "ses fils" << "son enfant " << "ses enfants" << "son garçon" << "ses garçons";
QList<QString> OQString::childReferencesSpanishM    = QList<QString> () << "test test M";
QList<QString> OQString::childReferencesSpanishF    = QList<QString> () << "test test F";
QList<QString> OQString::childReferencesSpanishU    = QList<QString> () << "test test U";

QList<QString> OQString::siblingReferencesEnglishM   = QList<QString> () << "brother of " << "brother to " << "his sister " << "his brother " << "brother-in-law of " << "uncle of ";
QList<QString> OQString::siblingReferencesEnglishF   = QList<QString> () << "sister of " << "sister to " << "her sister " << "her brother " << "sister-in-law of " << "aunt of ";
QList<QString> OQString::siblingReferencesEnglishU   = QList<QString> () << "sibling of " << "sibling to " << "brother " << "sister " << "sibling " << "brother:" << "sister:" << "sibling:" << "brother;" << "sister;" << "sibling;";
QList<QString> OQString::siblingReferencesFrenchM    = QList<QString> () << "frère de " << "frère à ";
QList<QString> OQString::siblingReferencesFrenchF    = QList<QString> () << "soeur de " << "soeur à ";
QList<QString> OQString::siblingReferencesFrenchU    = QList<QString> () << "sa soeur " << "ses soeurs " << "son frère " << "ses frères ";
QList<QString> OQString::siblingReferencesSpanishM   = QList<QString> () << "test test M";
QList<QString> OQString::siblingReferencesSpanishF   = QList<QString> () << "test test F";
QList<QString> OQString::siblingReferencesSpanishU   = QList<QString> () << "test test U";

QList<QString> OQString::spousalReferencesEnglishM  = QList<QString> () << "his wife " << "his spouse " << "his partner " << "husband of " << "husband to " ;
QList<QString> OQString::spousalReferencesEnglishF  = QList<QString> () << "her husband " << "her spouse " << "her partner " << "wife of " << "wife to ";
QList<QString> OQString::spousalReferencesEnglishU  = QList<QString> () << "friend of " << "friend to ";
QList<QString> OQString::spousalReferencesFrenchM   = QList<QString> () << "sa femme " << "l'époux de " << "conjoint de";
QList<QString> OQString::spousalReferencesFrenchF   = QList<QString> () << "son mari " << "l'épouse de " << "conjointe de";
QList<QString> OQString::spousalReferencesFrenchU   = QList<QString> () << "son épouse " << "épouse de " << "son conjoint " << "son partenaire " << "sa partenaire ";
QList<QString> OQString::spousalReferencesSpanishM  = QList<QString> () << "test test M";
QList<QString> OQString::spousalReferencesSpanishF  = QList<QString> () << "test test F";
QList<QString> OQString::spousalReferencesSpanishU  = QList<QString> () << "test test U";

QList<QString> OQString::relativeReferencesEnglish  = QList<QString> () << "niece " << "nephew " << "niece; " << "nephew; " << "niece: " << "nephew:" ;
QList<QString> OQString::relativeReferencesFrench   = QList<QString> () << "nièce " << "neveu " << "nièce; " << "neveu; " << "nièce: " << "neveu:" ;
QList<QString> OQString::relativeReferencesSpanish  = QList<QString> () << "test test S ";

QList<QString> OQString::passingReferencesEnglishM    = QList<QString> () << "his passing";
QList<QString> OQString::passingReferencesEnglishF    = QList<QString> () << "her passing";
QList<QString> OQString::passingReferencesEnglishU    = QList<QString> () << "testtest";
QList<QString> OQString::passingReferencesFrenchM     = QList<QString> () << "décédé ";
QList<QString> OQString::passingReferencesFrenchF     = QList<QString> () << "décédée ";
QList<QString> OQString::passingReferencesFrenchU     = QList<QString> () << "décès ";
QList<QString> OQString::passingReferencesSpanishM    = QList<QString> () << "testtest M";
QList<QString> OQString::passingReferencesSpanishF    = QList<QString> () << "testtest F";
QList<QString> OQString::passingReferencesSpanishU    = QList<QString> () << "testtest U";

QList<QString> OQString::deathIndicatorsEnglish           = QList<QString> () << "final" << "found" << "heaven" << "heavenly" << "peace"
                                                                              << "peacefully" << "resting";
QList<QString> OQString::deathIndicatorsFrench            = QList<QString> () << "ciel";
QList<QString> OQString::deathIndicatorsSpanish           = QList<QString> () << "cielo";
