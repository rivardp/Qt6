// PQStream.cpp

#include "PQStream.h"

PQStream::PQStream() : PQString(), EOS(true), position(0)
{
}

PQStream::PQStream(const PQString &source) : PQString(source), position(0)
{
    if (itsString.length() > 0)
		EOS = false;
	else
		EOS = true;
}

PQStream::PQStream(const std::wstring &source) : PQString(source), position(0)
{
    if (itsString.length() > 0)
		EOS = false;
	else
		EOS = true;
}

PQStream::~PQStream()
{
}

void PQStream::forward(unsigned int numPosition)
{
    unsigned int itsLength = itsString.length();

    unsigned int maxMove = itsLength - position;
	if (numPosition <= maxMove)
		position += numPosition;
	else
		position = itsLength;
	if (position == itsLength)
		EOS = true;
}

void PQStream::backward(unsigned int numPosition)
{
	unsigned int maxMove = position;
	if (numPosition <= maxMove)
		position -= numPosition;
	else
		position = 0;
    if (position < (unsigned int) itsString.length())
		EOS = false;
}

void PQStream::beg()
{
	position = 0;
    if (itsString.length() > 0)
		EOS = false;
	else
		EOS = true;
}

bool PQStream::moveTo(const QString &string)
{
    long int result = itsString.indexOf(string, position, Qt::CaseSensitive);
    if (result == -1)
    {
        position = itsString.length();
        EOS = true;
        return false;
    }
    else
    {
        position = result + string.length();
        return true;
    }
}

bool PQStream::consecutiveMovesTo(const unsigned int maxMove, const QString &string1, const QString &string2, const QString &string3, const QString &string4)
{
    // maxMove is measured from point first string is matched

    unsigned int initialPosition, endingPosition;

    bool success = moveTo(string1);
    if (success)
    {
        initialPosition = position;
        success = success && moveTo(string2);
        endingPosition = position;
    }

    if (success && (string3.size() > 0))
    {
        success = success && moveTo(string3);
        endingPosition = position;
    }
    if (success && (string4.size() > 0))
    {
        success = success && moveTo(string4);
        endingPosition = position;
    }

    if (success)
        success = (maxMove > (endingPosition - initialPosition));

    return success;
}

bool PQStream::isEOS() const
{
	return EOS;
}

PQString PQStream::get()
{
    if (position < (unsigned int) itsString.length())
		position++;
	else
		EOS = true;
	if (!EOS)
        return itsString.at(position - 1);
	else
        return PQString(QString("\0"));
}

QChar PQStream::getQChar()
{
    if (position < (unsigned int) itsString.length())
        position++;
    else
        EOS = true;
    if (!EOS)
        return itsString.at(position - 1);
    else
        return QChar('\0');
}

PQString PQStream::getWord(const bool considerParentheses)
{
	bool wordStarted = false;
	bool wordEnded = false;
    PQString word;
    PQString singleChar;
    PQString stopChar;
    PQString space(QString(" "));
    PQString comma(QString(","));
	unsigned int singleCharType;	

	// Get first non-space and non-slash character 
	while (!wordStarted && !EOS)
	{
		singleChar = get();
		singleCharType = singleChar.getCharType() & SLASH;
		if ((singleChar != space) && (singleCharType != SLASH))
		{
			wordStarted = true;
			word = singleChar;
		}
	}

	// Set end point (assuming a SLASH is not encountered)
	if (considerParentheses)
	{
		if ((singleChar.getCharType() & (OPENING | PARENTHESES)) == (OPENING | PARENTHESES))
			stopChar = singleChar.reciprocal();
		else
		{
			if ((singleChar.getCharType() & (OPENING | QUOTES)) == (OPENING | QUOTES))
				stopChar = singleChar.reciprocal();
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
		singleCharType = singleChar.getCharType() & SLASH;
		if ((singleChar != stopChar) && (singleCharType != SLASH) && !EOS)
			word += singleChar;
		else
		{
			wordEnded = true;
			// Copy ending parentheses or quotes
			if ((stopChar != space) && (singleCharType != SLASH) && !EOS)
				word += stopChar;
		}
	}

	return word;
}

PQString PQStream::getNext(const unsigned int numChars)
{
    PQString word;
    PQString singleChar;
    unsigned int count = 0;

    while (!EOS && (count < numChars))
    {
        singleChar = get();
        word += singleChar;
        count++;
    }

    return word;
}

PQString PQStream::getUntil(QString qchar, bool dropLast)
{
    PQString word;
    PQString singleChar;
    PQString target(qchar);
    bool matched = false;

    while (!EOS && !matched)
    {
        singleChar = get();
        matched = (singleChar == target);
        if (!matched || !dropLast)
            word += singleChar;
    }

    if (word.right(1) == PQString("\n"))
            word.dropRight(1);

    return word;
}

PQString PQStream::peekAtWord(const bool considerParentheses)
{
    PQString word;
	unsigned int origPosition = this->position;		// Needed in case stream contains consecutive spaces 
	word = getWord(considerParentheses);
	this->backward(this->position - origPosition);  // EOS reset  by 'backward' if necessary

	return word;
}

PQString PQStream::peekAtNext(const unsigned int numChars)
{
    PQString word;
    unsigned int origPosition = this->position;		// Needed in case stream contains consecutive spaces
    word = getNext(numChars);
    this->backward(this->position - origPosition);  // EOS reset  by 'backward' if necessary

    return word;
}

PQString PQStream::readHTMLContent(unsigned int maxChar)
{
    // Looks for and picks out first set of wording between "quotes" or >parentheses<

    bool endReached = false;
    bool started = false;

    PQString heldSoFar;
    PQString singleChar, lastChar;
    PQString space(' ');
    PQString quote('"');
    PQString openTag('<');
    PQString closeTag('>');
    PQString comma(',');

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
                else
                {
                // Disregard
                };
            }
        }
        lastChar = singleChar;
    } // End while

    return heldSoFar.unQuoteHTML();
}

PQString PQStream::readNextBetween(unsigned int param)
{
    QChar singleChar;
    PQString heldSoFar;
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
        endPoint = (currentCharType & param) == param;
        if (endPoint)
            numParam++;
        if ((numParam == 1) && !endPoint)
            heldSoFar += singleChar;
        if (numParam == 2)
            finished = true;
    }
    return heldSoFar;
}

PQStream& PQStream::operator= (const PQStream &rhs)
{
	if (this == &rhs)
		return *this;
	
    itsString = QString(rhs.getString());
	position = rhs.position;
	EOS = rhs.EOS;

	return *this;
}

PQStream& PQStream::operator= (const std::string &rhs)
{
    itsString = QString::fromStdString(rhs);

    position = 0;
    if (itsString.length() > 0)
        EOS = false;
    else
        EOS = true;

    return *this;
}

PQStream& PQStream::operator= (const std::wstring &rhs)
{
    itsString = QString::fromStdWString(rhs);

    position = 0;
    if (itsString.length() > 0)
		EOS = false;
	else
		EOS = true;

	return *this;
}

PQStream& PQStream::operator= (const QByteArray &rhs)
{
    //itsString = QString(QTextCodec::codecForMib(106)->toUnicode(rhs));
    itsString = QString(rhs);

    position = 0;
    if (itsString.length() > 0)
        EOS = false;
    else
        EOS = true;

    return *this;
}


