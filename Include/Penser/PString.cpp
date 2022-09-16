// PString.cpp

#include "PString.h"


PString::PString()
{
	itsString = new TCHAR[1];
	itsString[0] = '\0';
	itsLength = 0;
}

PString::PString(unsigned int length)
{
	itsString = new TCHAR[length + 1];
	for (unsigned int i = 0; i <= length; i++)
		itsString[i] = '\0';
	itsLength = length;
}

PString::PString(const TCHAR * const cString)
{
    itsLength = static_cast<unsigned int>(_tcslen(cString));
	itsString = new TCHAR[itsLength + 1];
	for (unsigned int i = 0; i<itsLength; i++)
		itsString[i] = cString[i];
	itsString[itsLength] = '\0';
}
/*
PString::PString(const char* const cString)
{
	itsLength = (unsigned int)strlen(cString);
	itsString = new TCHAR[itsLength + 1];
	for (unsigned int i = 0; i<itsLength; i++)
		itsString[i] = cString[i];
	itsString[itsLength] = '\0';
}*/

PString::PString(const PString &rhs)
{
	itsLength = rhs.getLength();
	itsString = new TCHAR[itsLength + 1];
	for (unsigned int i = 0; i<itsLength; i++)
		itsString[i] = rhs[i];
	itsString[itsLength] = '\0';
}

PString::PString(const TCHAR& singleChar)
{
	itsLength = 1;
	itsString = new TCHAR[itsLength + 1];
	itsString[0] = singleChar;
	itsString[itsLength] = '\0';
}

PString::PString(const std::string &stdString)
{
    itsLength = static_cast<unsigned int>(stdString.length());
	itsString = new TCHAR[itsLength + 1];
	for (unsigned int i = 0; i<itsLength; i++)
        itsString[i] = static_cast<TCHAR>(stdString[i]);
	itsString[itsLength] = '\0';
}

PString::PString(const std::wstring &wString)
{
    itsLength = static_cast<unsigned int>(wString.length());
	itsString = new TCHAR[itsLength + 1];
	for (unsigned int i = 0; i<itsLength; i++)
        itsString[i] = static_cast<TCHAR>(wString[i]);
	itsString[itsLength] = '\0';
}

PString::~PString()
{
	delete[] itsString;
    itsString = nullptr;
	itsLength = 0;
}

unsigned int PString::getLength()const
{
	return itsLength;
}

const TCHAR* PString::getString() const
{
	return itsString;
}

std::string PString::getStdString() const
{
	std::string temp;
	for (unsigned int i = 0; i < itsLength; i++)
        temp += static_cast<char>(itsString[i]);
	return temp;
}

std::wstring PString::getWString() const
{
	std::wstring temp;
	for (unsigned int i = 0; i < itsLength; i++)
        temp += static_cast<wchar_t>(itsString[i]);
	return temp;
}

char * PString::convertToCharPtr() const
{
	size_t outputSize = itsLength + 1; // +1 for null terminator
	char *charPtr = new char[outputSize];
	size_t charsConverted = 0;
	wcstombs_s(&charsConverted, charPtr, outputSize, itsString, itsLength);

	return charPtr;
}

char* PString::getQString() const
{
    return convertToCharPtr();
}

PString& PString::operator=(const PString &rhs)
{
	if (this == &rhs)
		return *this;
	delete[] itsString;
	itsLength = rhs.getLength();
	itsString = new TCHAR[itsLength + 1];
	for (unsigned int i = 0; i<itsLength; i++)
		itsString[i] = rhs[i];
	itsString[itsLength] = '\0';
	return *this;
}

TCHAR & PString::operator[](size_t offset)
{
	if (offset > itsLength)
		return itsString[itsLength - 1];
	else
		return itsString[offset];
}

TCHAR PString::operator[](size_t offset) const
{
	if (offset > itsLength)
		return itsString[itsLength - 1];
	else
		return itsString[offset];
}

PString PString::operator+(const PString& rhs)
{
	unsigned int  totalLength = itsLength + rhs.getLength();
	PString temp(totalLength);
	unsigned int i;
	for (i = 0; i<itsLength; i++)
		temp[i] = itsString[i];
	for (unsigned int j = 0; j<rhs.getLength(); j++, i++)
		temp[i] = rhs[j];
	temp[totalLength] = '\0';
	return temp;
}

void PString::operator+=(const PString& rhs)
{
	unsigned int rhsLength = rhs.getLength();
	unsigned int totalLength = itsLength + rhsLength;
	PString  temp(totalLength);
	unsigned int i;
	for (i = 0; i<itsLength; i++)
		temp[i] = itsString[i];
	for (unsigned int j = 0; j<rhs.getLength(); j++, i++)
		temp[i] = rhs[i - itsLength];
	temp[totalLength] = '\0';
	*this = temp;
}

bool PString::operator==(const PString &rhs)
{
	return isMatch(rhs);
}

bool PString::operator!=(const PString &rhs)
{
	return !isMatch(rhs);
}

PString PString::upper() const
{
	unsigned int currentCharType;
	PString temp(itsLength);
	unsigned int i;
	for (i = 0; i < itsLength; i++)
	{
        currentCharType = ANSI[static_cast<unsigned int>(itsString[i])].charType_defns;
		if ((currentCharType & LOWER) == LOWER)
            temp[i] = static_cast<TCHAR>(ANSI[static_cast<unsigned int>(itsString[i])].lowerUpperMatch);
        else
			temp[i] = itsString[i];
	}
	temp[itsLength] = '\0';
	return temp;
}

PString PString::lower() const
{
	unsigned int currentCharType;
	PString temp(itsLength);
	unsigned int i;
	for (i = 0; i < itsLength; i++)
	{
        currentCharType = ANSI[static_cast<unsigned int>(itsString[i])].charType_defns;
		if ((currentCharType & UPPER) == UPPER)
            temp[i] = static_cast<TCHAR>(ANSI[static_cast<unsigned int>(itsString[i])].lowerUpperMatch);
		else
			temp[i] = itsString[i];
	}
	temp[itsLength] = '\0';
	return temp;
}

PString PString::proper() const
{
	PString singleChar, lastChar, lastTwo, lastThree;
	PString result;
	unsigned int i;

	PString space(" ");
	PString hyphen("-");
    PString apostrophe("'");
	PString tempString(itsString);
	bool charAdded;

	if (itsLength > 0)
	{
		singleChar = tempString[0];
		result += singleChar.upper();
	}

	for (i = 1; i < itsLength; i++)
	{
		// Use lower case with certain exceptions, where input is assumed to be correct
		lastChar = itsString[i - 1];
		singleChar = itsString[i];
		charAdded = false;

        // After an apostophe in the second position (e.g., O'Toole)
        if ((lastChar == apostrophe) && (i == 2))
        {
            result += singleChar.upper();
            charAdded = true;
        }

        // After a space, hyphen, or "Mc" or "Mac", the user input case is used unless the word is allCaps
		if ((lastChar == space) || (lastChar == hyphen))
		{
			result += singleChar;
			charAdded = true;
		}

		if (!charAdded && (i >= 2))
		{
			lastTwo = result.middle(i - 2, 2);
			if ((lastTwo == _T("Mc")) && !this->isAllCaps())
			{
				result += singleChar;
				charAdded = true;
			}
		}

		if (!charAdded && (i >= 3))
		{
			lastThree = result.middle(i - 3, 3);
			if ((lastThree == _T("Mac")) && !this->isAllCaps())
			{
				result += singleChar;
				charAdded = true;
			}
		}

		if (!charAdded)
			result += singleChar.lower();
	}

	return result;
}


PString PString::reciprocal() const
{
	if (itsLength == 1)
        return PString(static_cast<TCHAR>(ANSI[static_cast<unsigned int>(itsString[0])].lowerUpperMatch));
	else
		return *this;
}

PString PString::left(unsigned int num) const
{
	PString temp(num);
	unsigned int i;
	if (num <= itsLength)
	{
		for (i = 0; i<num; i++)
			temp[i] = itsString[i];
		temp[num] = '\0';
	}
	else
	{
		for (i = 0; i<itsLength; i++)
			temp[i] = itsString[i];
		for (i = itsLength; i<num; i++)
			temp[i] = ' ';
		temp[num] = '\0';
	}
	return temp;
}

PString PString::right(unsigned int num) const
{
	PString temp(num);
	unsigned int i;
	if (num <= itsLength)
	{
		for (i = 0; i<num; i++)
			temp[i] = itsString[itsLength - num + i];
		temp[num] = '\0';
	}
	else
	{
		for (i = 0; i<itsLength; i++)
			temp[i] = itsString[i];
		for (i = itsLength; i<num; i++)
			temp[i] = ' ';
		temp[num] = '\0';
	}
	return temp;
}

PString PString::middle(unsigned int offset, unsigned int num) const
{
	PString temp(num);
	unsigned int i, maxOffset;
	maxOffset = (itsLength < offset) ? itsLength : offset;
	if (num <= (itsLength - maxOffset))
	{
		for (i = 0; i<num; i++)
			temp[i] = itsString[i + maxOffset];
		temp[num] = '\0';
	}
	else
	{
		for (i = 0; i<(itsLength - maxOffset); i++)
			temp[i] = itsString[i + maxOffset];
		for (i = (itsLength - maxOffset); i<num; i++)
			temp[i] = ' ';
		temp[num] = '\0';
	}
	return temp;
}

PString& PString::clear()
{
	delete[] itsString;
	itsString = new TCHAR[1];
	itsString[0] = '\0';
	itsLength = 0;
	return *this;
}

void PString::dropLeft(unsigned int num)
{
	if (num > 0)
	{
		TCHAR *origString = itsString;
		if (num < itsLength)
		{
			itsLength -= num;
			itsString = new TCHAR[itsLength + 1];
			for (unsigned int i = 0; i<itsLength; i++)
				itsString[i] = origString[num + i];
			itsString[itsLength] = '\0';
			delete[]origString;
		}
		else
		{
			itsLength = 0;
			itsString = new TCHAR[itsLength + 1];
			itsString[itsLength] = '\0';
			delete[]origString;
		}
	}
}

void PString::dropRight(unsigned int num)
{
	if (num > 0)
	{
		TCHAR *origString = itsString;
		if (num < itsLength)
		{
			itsLength -= num;
			itsString = new TCHAR[itsLength + 1];
			for (unsigned int i = 0; i<itsLength; i++)
				itsString[i] = origString[i];
			itsString[itsLength] = '\0';
			delete[]origString;
		}
		else
		{
			itsLength = 0;
			itsString = new TCHAR[itsLength + 1];
			itsString[itsLength] = '\0';
			delete[]origString;
		}
	}
}


void PString::removeSpecialChar()
{
	// Remove escape character (\), ampersand (&) and double quote (") since they will interfere with streams
	PString newString;
	PString currentChar;
	unsigned int i;

	for (i = 0; i < itsLength; i++)
	{
		currentChar = itsString[i];
		if ((itsString[i] != '"') && (itsString[i] != '&') && (itsString[i] != '\\'))
			newString += currentChar;
	}
	TCHAR *origString = itsString;
	itsLength = newString.getLength();
	itsString = new TCHAR[itsLength + 1];
	for (i = 0; i<itsLength; i++)
		itsString[i] = newString[i];
	itsString[itsLength] = '\0';
	delete[]origString;
}

bool PString::hasBookEnds(unsigned int bookends)
{
	if (itsLength == 0)
		return false;

	bool hasParentheses, hasQuotes;
	PString firstChar = itsString[0];
	PString lastChar = itsString[itsLength - 1];

	hasParentheses = ((bookends & PARENTHESES) == PARENTHESES) && ((firstChar.getCharType() & PARENTHESES) == PARENTHESES)
		&& ((lastChar.getCharType() & PARENTHESES) == PARENTHESES);
	hasQuotes = ((bookends & QUOTES) == QUOTES) && ((firstChar.getCharType() & QUOTES) == QUOTES)
		&& ((lastChar.getCharType() & QUOTES) == QUOTES);

	return hasParentheses || hasQuotes;
}


bool PString::removeBookEnds(unsigned int bookends)
{
	if (itsLength == 0)
		return false;

	bool hasParentheses, hasQuotes;

	hasParentheses = hasBookEnds(bookends & PARENTHESES);
	hasQuotes = hasBookEnds(bookends & QUOTES);

	if (hasParentheses || hasQuotes)
	{
		dropLeft(1);
		dropRight(1);
	}

	return hasParentheses || hasQuotes;
}

bool PString::removeLeading(const std::wstring target)
{
    unsigned int length = static_cast<unsigned int>(target.size());
	bool matched = true;

	for (unsigned int i = 0; i < length; i++)
		matched = matched && (itsString[i] == target[i]);

	if (matched)
	{
		dropLeft(length);
		// If a single char target, look for repeated chars
		removeLeading(target);
	}

	return matched;
}


bool PString::removeEnding(const std::wstring target)
{
    unsigned int length = static_cast<unsigned int>(target.size());
    bool matched = true;

	for (unsigned int i = 0; i < length; i++)
		matched = matched && (itsString[itsLength - length + i] == target[i]);

	if (matched)
	{
		dropRight(length);
		// check if multiple targets existed (e.g. trailing blanks)
		if (length == 1)
			removeEnding(target);
	}

	return matched;
}

bool PString::removeEnding(const unsigned int charCode)
{
	PString lastChar = right(1);
	if ((lastChar.getCharType() & charCode) == charCode)
	{
		dropRight(1);
		return true;
	}

	return false;
}

bool PString::cleanUpEnds()
{
	bool changesMade;

	changesMade = removeLeading(_T(" "));
	changesMade = removeLeading(_T("\n")) || changesMade;
	changesMade = removeLeading(_T(" ")) || changesMade;
	changesMade = removeEnding(_T(" ")) || changesMade;

	return changesMade;
}


bool PString::isMatch(const PString &rhs) const
{
	if (this == &rhs)
		return 1;
	if (itsLength != rhs.getLength())
		return 0;
	bool match = true;
	unsigned int i = 0;
	while (match && (i<itsLength))
	{
		match = (itsString[i] == rhs[i]) ? true : false;
		i++;
	}
	return match;
}

bool PString::isMatch(const TCHAR* cString) const
{
	PString target(cString);
	return isMatch(target);
}

bool PString::isAlpha() const
{
	return (getCharType() & ALPHA) == ALPHA;
}

bool PString::isNumeric() const
{
	return (getCharType() & NUMERICAL) == NUMERICAL;
}

bool PString::isAlphaNumeric() const
{
	return (getCharType() & ALPHANUMERIC) == ALPHANUMERIC;
}

bool PString::isCapitalized() const
{
	if (itsLength == 0)
		return false;

	PString firstChar(itsString[0]);

	return (firstChar.getCharType() & UPPER) == UPPER;
}

bool PString::isAllCaps() const
{
	// Only checks true alpha characters and isn't invalidated by other characters

	bool stillOK = true;
	unsigned int i = 0;
	unsigned int charType;
	PString singleChar;

	while (stillOK && (i < itsLength))
	{
		singleChar = itsString[i];
		charType = singleChar.getCharType();
		if (((charType & ALPHA) == ALPHA) && ((charType & SPACE) != SPACE) && ((charType & PUNCTUATION) != PUNCTUATION))
			stillOK = (charType & UPPER) == UPPER;
		i++;
	}
	return stillOK;
}

long double PString::asNumber() const
{
	long double answer = 0;
	bool OK = true;
	bool noMoreNumbers = false;
	unsigned int decimal = 0;
	unsigned int i = 0;
	unsigned int denominator = 1;
	unsigned int blankCount = 0;
	unsigned int cValue;
	int sign = 1;
	TCHAR c;

	while (OK && (i < itsLength))
	{
		c = itsString[i];
		switch (c)
		{
		case 45:	// Negative sign
		case 150:	// Dash
			if ((i == 0) || (i == blankCount))
				sign = -1;
			else
				OK = false;
			break;

		case 32:	// Space
			blankCount++;
			if ((answer > 0) || (sign != 1) || (decimal > 0))
				noMoreNumbers = true;
			break;

		case 46:	// Decimal point
			decimal++;
			if (decimal > 1)
				OK = false;
			break;

		case 48:	// 0
		case 49:	// 1
		case 50:	// 2
		case 51:	// 3
		case 52:	// 4
		case 53:	// 5
		case 54:	// 6
		case 55:	// 7
		case 56:	// 8
		case 57:	// 9
			if (noMoreNumbers)
				OK = false;
			else
			{
				cValue = (unsigned int)c - 48;
				answer = cValue + answer * 10;
				if (decimal)
					denominator *= 10;
			}
			break;

        case 36:		// Currency sign '$' Dollar
        case 163:		// Currency sign '£' Pound
        case 164:		// Currency sign '¤'
        case 165:		// Currency sign '¥' Yen
        case 8364:      // Currency sign '€' Euro
        case 44:		// Comma ','
            break;		// Ignore character and continue

        case 37:		// Percentage '%'
        case 162:		// Cents '¢'
			noMoreNumbers = true;
			denominator /= 100;
			break;

		default:
			OK = false;
		}
		i++;
	}
	if (OK)
		return sign * answer / denominator;
	else
		return 0;
}

long double PString::extractFirstNumber() const
{
	PString excerpt;
	unsigned int i = 0;
	TCHAR c;
	bool started = false;
	bool keepGoing = true;

	while (keepGoing && (i < itsLength))
	{
		c = itsString[i];
		switch (c)
		{
		case 45:	// Negative sign
		case 150:	// Dash
			if (started)
				keepGoing = false;
			else
			{
				started = true;
				excerpt += c;
			}
			break;

			// Keep the following characters
		case 46:	// Decimal point
		case 48:	// 0
		case 49:	// 1
		case 50:	// 2
		case 51:	// 3
		case 52:	// 4
		case 53:	// 5
		case 54:	// 6
		case 55:	// 7
		case 56:	// 8
		case 57:	// 9
			started = true;
			excerpt += c;
			break;

			// Ignore the following characters
        case 36:		// Currency sign '$' Dollar
        case 163:		// Currency sign '£' Pound
        case 164:		// Currency sign '¤'
        case 165:		// Currency sign '¥' Yen
        case 8364:      // Currency sign '€' Euro
        case 44:		// Comma ','
            break;		// Ignore character and continue

			// Ignore all other characters unless number is started, in which case break
		default:
			if (started)
				keepGoing = false;
			break;
		}
		i++;
	}

	return excerpt.asNumber();
}


int PString::findPosition(const PString &criteria, int direction, unsigned int skip, unsigned int occurrence) const
{
	// Function returns -1 if match not made, the position of the first character otherwise (not the array element)
	// Function relies on recursion if occurrence > 1

	// direction =  1 => means forwards or left to right
	// direction = -1 => means backwards or right to left

	// Return immediately on parameter errors
	if ((criteria.itsLength == 0) || (occurrence == 0) || ((direction != 1) && (direction != -1)))
		return -1;

	// Redefine target and return if no match possible
	PString target(itsString);
	if (direction == 1)
		target.dropLeft(skip);
	else
		target.dropRight(skip);
	if (criteria.itsLength > target.itsLength)
		return -1;

	// Start actual search
    unsigned int numMatched, i, j;
    int firstCharPos;							// must go to -1 in backwards search
    bool initialMatch, matched;
	numMatched = 0;
	initialMatch = matched = false;

	if (direction == 1)	// forward
	{
		i = 0;
		while (!matched && (criteria.itsLength <= (target.itsLength - i)))
		{
			j = 0;
			initialMatch = (criteria[j] == target[i + j]);
			while (!matched && initialMatch && (j < criteria.itsLength))
			{
				j++;
				if (j == criteria.itsLength)
					matched = true;
				else
					initialMatch = (criteria[j] == target[i + j]);
			}  // end of inner while loop
			i++;
		}  // end of outer while loop

		if (matched)
		{
			numMatched++;
			if (numMatched == occurrence)
                return static_cast<int>(skip + i);
			else
				return findPosition(criteria, direction, skip + i, occurrence - 1);
		}
		else
			return -1;
	}
	else   // search backwards
	{
		i = target.itsLength - criteria.itsLength;
        firstCharPos = static_cast<int>(i);
        while (!matched && (firstCharPos >= 0))
		{
			j = 0;
			initialMatch = (criteria[j] == target[i + j]);
			while (!matched && initialMatch && (j < criteria.itsLength))
			{
				j++;
				if (j == criteria.itsLength)
					matched = true;
				else
					initialMatch = (criteria[j] == target[i + j]);
			}  // end of inner while loop
			i--;
            firstCharPos--;
		}  // end of outer while loop

		if (matched)
		{
			numMatched++;
			if (numMatched == occurrence)
                return static_cast<int>(i + 2);
			else
				return findPosition(criteria, direction, itsLength - i + 1, occurrence - 1);
		}
		else
			return -1;
	}
}

int PString::findPosition(const TCHAR* cString, int direction, unsigned int skip, unsigned int occurrence) const
{
	PString pString(cString);
	return findPosition(pString, direction, skip, occurrence);
}

unsigned int PString::getCharType(unsigned int elemNum, unsigned int numWordsRead) const
{
	// Assess types of characters included in string
	// Uses recursion if string length > 1
	if (elemNum >= itsLength)
		return UNDEFINED;

	unsigned int finalType = UNDEFINED;
    unsigned int currentCharType = ANSI[static_cast<unsigned int>(itsString[elemNum])].charType_defns;

	if (elemNum == (itsLength - 1))
	{
		return currentCharType;
	}

	unsigned int remainingCharType = this->getCharType(elemNum + 1, numWordsRead);

	if ((currentCharType & (ALPHA | HYPHENATED | PARENTHESES)) && (remainingCharType & (ALPHA | HYPHENATED | PARENTHESES)))
		finalType += ALPHA;

	if ((currentCharType & UPPER) && (remainingCharType & UPPER))
		finalType += UPPER;

	if ((currentCharType & LOWER) && (remainingCharType & LOWER))
		finalType += LOWER;

	if (((currentCharType & (NUMERICAL | NUMERIC_FORMAT)) && (remainingCharType & NUMERICAL)) || ((currentCharType & NUMERICAL) && (remainingCharType & NUMERIC_FORMAT)))
		finalType += NUMERICAL;

	if ((currentCharType & NUMERIC_FORMAT) && (remainingCharType & NUMERIC_FORMAT))
		finalType += NUMERIC_FORMAT;

	if ((currentCharType & UPPER) && (currentCharType & ALPHA) && (remainingCharType & LOWER) && (remainingCharType & ALPHA))
		finalType += PROPER;

	if (((currentCharType & ALPHA) && (remainingCharType & MIXED)) || ((currentCharType & ALPHA) && !(remainingCharType & (UPPER | LOWER | PROPER))))
		finalType += MIXED;

	if (((finalType & ALPHA) || (finalType & NUMERICAL)) ||
		(((currentCharType & ALPHA) || (currentCharType & NUMERICAL)) && ((remainingCharType & ALPHANUMERIC) || (remainingCharType & ALPHA) || (remainingCharType & NUMERICAL))))
		finalType += ALPHANUMERIC;


	return finalType;

}

unsigned int PString::countWords(unsigned int delim)
{
	unsigned int count = 0;
	if (itsLength == 0)
		return 0;

	bool wordStarted = false;
	PString singleChar, delimiter;

	switch (delim)
	{
	case COMMA:
		delimiter = _T(",");
		break;

	case HYPHENATED:
		delimiter = _T("-");
		break;

	case PERIOD:
		delimiter = _T(".");
		break;

	case SPACE:
	default:
		delimiter = _T(" ");
		break;

	}

	for (unsigned int i = 0; i < itsLength; i++)
	{
		singleChar = itsString[i];
		if (singleChar.isAlpha() && (singleChar != delimiter))
			wordStarted = true;

		if (wordStarted && ((singleChar == delimiter) || (i == (itsLength - 1))))
		{
			count++;
			wordStarted = false;
		}
	}
	return count;
}

unsigned int PString::drawOutNumber() const
{
	unsigned int result = 0;
	PString singleChar;

	for (unsigned int i = 0; i < itsLength; i++)
	{
		singleChar = itsString[i];
		if ((singleChar.getCharType() & NUMERICAL) == NUMERICAL)
            result = result * 10 + static_cast<unsigned int>(singleChar.asNumber());
	}

	return result;
}
