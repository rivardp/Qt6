// PStream.cpp

#include "PStream.h"

PStream::PStream() : PString(), EOS(true), position(0)
{
}

PStream::PStream(const PString &source) : PString(source), position(0)
{
	if (itsLength > 0)
		EOS = false;
	else
		EOS = true;
}

PStream::PStream(const std::wstring &source) : PString(source), position(0)
{
	if (itsLength > 0)
		EOS = false;
	else
		EOS = true;
}

PStream::~PStream()
{

}

void PStream::forward(unsigned int numPosition)
{
	unsigned int maxMove = itsLength - position;
	if (numPosition <= maxMove)
		position += numPosition;
	else
		position = itsLength;
	if (position == itsLength)
		EOS = true;
}

void PStream::backward(unsigned int numPosition)
{
	unsigned int maxMove = position;
	if (numPosition <= maxMove)
		position -= numPosition;
	else
		position = 0;
	if (position < itsLength)
		EOS = false;
}

void PStream::beg()
{
	position = 0;
	if (itsLength > 0)
		EOS = false;
	else
		EOS = true;
}

bool PStream::isEOS() const
{
	return EOS;
}


PString PStream::get() 
{
	if (position < itsLength)
		position++;
	else
		EOS = true;
	if (!EOS)
		return PString((TCHAR)itsString[position - 1]);
	else
		return PString(_T("\0"));
}

PString PStream::getWord(const bool considerParentheses)
{
	bool wordStarted = false;
	bool wordEnded = false;
	PString word;
	PString singleChar;
	PString stopChar;
	PString space(_T(" "));
	PString comma(_T(","));
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

PString PStream::peekAtWord(const bool considerParentheses)
{
	PString word;
	unsigned int origPosition = this->position;		// Needed in case stream contains consecutive spaces 
	word = getWord(considerParentheses);
	this->backward(this->position - origPosition);  // EOS reset  by 'backward' if necessary

	return word;
}

PStream& PStream::operator= (const PStream &rhs)
{
	if (this == &rhs)
		return *this;
	
	delete[] itsString;
	itsLength = rhs.getLength();
	itsString = new TCHAR[itsLength + 1];
	for (unsigned int i = 0; i<itsLength; i++)
		itsString[i] = rhs[i];
	itsString[itsLength] = '\0';

	position = rhs.position;
	EOS = rhs.EOS;

	return *this;
}

PStream& PStream::operator= (const std::wstring &rhs)
{
	delete[] itsString;
	itsLength = (unsigned int) rhs.length();
	itsString = new TCHAR[itsLength + 1];
	for (unsigned int i = 0; i<itsLength; i++)
		itsString[i] = rhs[i];
	itsString[itsLength] = '\0';

	position = 0;
	if (itsLength > 0)
		EOS = false;
	else
		EOS = true;

	return *this;
}


