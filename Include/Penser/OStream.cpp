// OStream.cpp
// The PString member of PStream is used for nearly all functionality
// The PString member of OString is essentially redundant and only used to access OString member functions

#include "OStream.h"

OStream::OStream() : PStream()
{
}

OStream::OStream(const OString &source) : PStream(source)
{
}

OStream::OStream(const PString &source) : PStream(source)
{
}

OStream::OStream(const std::wstring &source) : PStream(source)
{
}

OStream::~OStream()
{
}

OString OStream::get()
{
	PString pWord = PStream::get();
	OString oWord(pWord);
	return oWord;
}

OString OStream::getWord(const bool considerParentheses)
{
	PString pWord = PStream::getWord(considerParentheses);
	OString oWord(pWord);
	return oWord;
}

OString OStream::peekAtWord(const bool considerParentheses)
{
	PString pWord = PStream::peekAtWord(considerParentheses);
	OString oWord(pWord);
	return oWord;
}

OStream& OStream::operator= (const OStream &rhs)
{
	if (this == &rhs)
		return *this;

	delete[] itsString;
	itsLength = rhs.getLength();
	itsString = new TCHAR[itsLength + 1];
	for (unsigned int i = 0; i < itsLength; i++)
		itsString[i] = rhs[i];
	itsString[itsLength] = '\0';

	position = rhs.position;
	EOS = rhs.EOS;

	return *this;
}

OStream& OStream::operator= (const PStream &rhs)
{
	if (this == &rhs)
		return *this;

	delete[] itsString;
	itsLength = rhs.getLength();
	itsString = new TCHAR[itsLength + 1];
	for (unsigned int i = 0; i < itsLength; i++)
		itsString[i] = rhs[i];
	itsString[itsLength] = '\0';

	position = rhs.position;
	EOS = rhs.EOS;

	return *this;
}

OStream& OStream::operator= (const std::wstring &rhs)
{
	delete[] itsString;
	itsLength = (unsigned int)rhs.length();
	itsString = new TCHAR[itsLength + 1];
	for (unsigned int i = 0; i < itsLength; i++)
		itsString[i] = rhs[i];
	itsString[itsLength] = '\0';

	position = 0;
	if (itsLength > 0)
		EOS = false;
	else
		EOS = true;

	return *this;
}

