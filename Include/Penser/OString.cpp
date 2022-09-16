// OString.cpp

#include "OString.h"

OString::OString() : PString()
{
}

OString::OString(unsigned int length) : PString(length)
{
}

OString::OString(const TCHAR * const cString) : PString(cString)
{
}

OString::OString(const char* const cString) : PString(cString)
{
}

OString::OString(const PString &rhs) : PString(rhs)
{
}

OString::OString(const OString &rhs) : PString(rhs)
{
}

OString::OString(const TCHAR& singleChar) : PString(singleChar)
{
}

OString::OString(const std::string &stdString) : PString(stdString)
{
}

OString::OString(const std::wstring &wString) : PString(wString)
{
}

OString::~OString()
{
}

OString& OString::operator=(const OString &rhs)
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

OString OString::operator+(const OString& rhs)
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

void OString::operator+=(const OString& rhs)
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

bool OString::operator==(const OString &rhs)
{
	return isMatch(rhs);
}

bool OString::operator!=(const OString &rhs)
{
	return !isMatch(rhs);
}

void OString::insertParameter(PString param)
{
	unsigned int i;
	unsigned int paramLength;
	int position = findPosition(_T("%p%"));
	if (position > 0)
	{
		position--;  // convert to array reference
		paramLength = param.getLength();
		TCHAR *origString = itsString;
		itsLength = itsLength + paramLength - 3;
		itsString = new TCHAR[itsLength + 1];
		for (i = 0; i < (unsigned int)position; i++)
			itsString[i] = origString[i];
		for (i = 0; i < paramLength; i++)
			itsString[position + i] = param[i];
		for (i = 0; i < (itsLength - position - paramLength); i++)
			itsString[position + paramLength + i] = origString[position + 3 + i];
		itsString[itsLength] = '\0';
	}
}


bool OString::removeLeadingNeeEtAl()
{
	bool removed = false;
	bool colonUsed = false;
	bool periodUsed = false;

	if (itsLength < 5)
		return false;

	colonUsed = ((itsString[3] == ':') || (itsString[4] == ':'));
	periodUsed = ((itsString[3] == '.') || (itsString[4] == '.'));

	if (colonUsed)
	{
		if (!removed)
			removed = removeLeading(_T("nee:"));

		if (!removed)
			removed = removeLeading(_T("née:"));

		if (!removed)
			removed = removeLeading(_T("born:"));

		if (!removed)
			removed = removeLeading(_T("Nee:"));

		if (!removed)
			removed = removeLeading(_T("Née:"));

		if (!removed)
			removed = removeLeading(_T("Born:"));

		if (!removed)
			removed = removeLeading(_T("NEE:"));

		if (!removed)
			removed = removeLeading(_T("NÉE:"));

		if (!removed)
			removed = removeLeading(_T("BORN:"));
	}
	else
	{
		if (periodUsed)
		{
			if (!removed)
				removed = removeLeading(_T("nee."));

			if (!removed)
				removed = removeLeading(_T("née."));

			if (!removed)
				removed = removeLeading(_T("born."));

			if (!removed)
				removed = removeLeading(_T("Nee."));

			if (!removed)
				removed = removeLeading(_T("Née."));

			if (!removed)
				removed = removeLeading(_T("Born."));

			if (!removed)
				removed = removeLeading(_T("NEE."));

			if (!removed)
				removed = removeLeading(_T("NÉE."));

			if (!removed)
				removed = removeLeading(_T("BORN."));
		}
		else
		{
			if (!removed)
				removed = removeLeading(_T("nee "));

			if (!removed)
				removed = removeLeading(_T("née "));

			if (!removed)
				removed = removeLeading(_T("born "));

			if (!removed)
				removed = removeLeading(_T("Nee "));

			if (!removed)
				removed = removeLeading(_T("Née "));

			if (!removed)
				removed = removeLeading(_T("Born "));

			if (!removed)
				removed = removeLeading(_T("NEE "));

			if (!removed)
				removed = removeLeading(_T("NÉE "));

			if (!removed)
				removed = removeLeading(_T("BORN "));
		}
	}

	if (removed)
		removeLeading(_T(" "));

	return removed;
}

bool OString::isPrefix() const
{
	if (itsLength == 0)
		return false;

	PString target = this->lower();
	target.removeEnding(_T("."));

	if ((target == _T("dr")) || (target == _T("mr")) || (target == _T("mrs")) || (target == _T("ms")) || (target == _T("mst")) || (target == _T("mr")))
		return true;

	if ((target == _T("adm")) || (target == _T("sgt")) || (target == _T("maj")) || (target == _T("gen")) || (target == _T("cdt")) || (target == _T("col")) || (target == _T("pvt")) || (target == _T("lt")))
		return true;

	if ((target == _T("sen")) || (target == _T("gov")) || (target == _T("hon")) || (target == _T("rep")))
		return true;

	return false;
}

bool OString::isSuffix() const
{
	if (itsLength == 0)
		return false;

	PString target = this->lower();
	target.removeEnding(_T("."));

	if ((target == _T("jr")) || (target == _T("sr")))
		return true;

	if ((target == _T("i")) || (target == _T("ii")) || (target == _T("iii")) || (target == _T("iv")) || (target == _T("v")))
		return true;

	if ((target == _T("rn")) || (target == _T("r.n")) || (target == _T("md")) || (target == _T("m.d")) || (target == _T("phd")))
		return true;

	if ((target == _T("qc")) || (target == _T("q.c")))
		return true;

	if ((target == _T("p. eng")) || (target == _T("p.eng")) || (target == _T("p eng")))
		return true;

	if ((target == _T("b. a")) || (target == _T("b.a")) || (target == _T("ba")))
		return true;

	if ((target == _T("b. comm")) || (target == _T("b.comm")) || (target == _T("bcomm")))
		return true;

	if ((target == _T("fca")) || (target == _T("cma")) || (target == _T("ca")))
		return true;

	if ((target == _T("fcia")) || (target == _T("fsa")))
		return true;


	return false;
}

bool OString::isSuffixAllCaps() const
{
	if (itsLength == 0)
		return false;

	PString target = this->lower();

	if ((target == _T("i")) || (target == _T("ii")) || (target == _T("iii")))
		return true;

	if ((target == _T("rn")) || (target == _T("r.n.")))
		return true;

	if ((target == _T("qc")) || (target == _T("q.c.")))
		return true;

	return false;
}

bool OString::isTitle() const
{
	if (itsLength == 0)
		return false;

	PString target = this->lower();

	if ((target == _T("mr")) || (target == _T("mr.")) || (target == _T("mrs")) || (target == _T("mrs.")))
		return true;

	return false;
}

bool OString::isSaint() const
{
	if (itsLength == 0)
		return false;

	PString target = this->lower();

	if ((target == _T("st")) || (target == _T("st.")))
		return true;

	return false;
}

unsigned int OString::isWrittenMonth(const LANGUAGE &lang)
{
	unsigned int numWords, i;
	PString lowerWord;
	PString *months, *word;
	bool matched;

	switch (lang)
	{
	case french:
		numWords = sizeof(monthsFrench) / sizeof(monthInfo);
		months = new PString[numWords];
		for (i = 0; i < numWords; i++)
			months[i] = monthsFrench[i].monthAlpha;
		break;

	case spanish:
		numWords = sizeof(monthsSpanish) / sizeof(monthInfo);
		months = new PString[numWords];
		for (i = 0; i < numWords; i++)
			months[i] = monthsSpanish[i].monthAlpha;
		break;

	case english:
	default:
		numWords = sizeof(monthsEnglish) / sizeof(monthInfo);
		months = new PString[numWords];
		for (i = 0; i < numWords; i++)
			months[i] = monthsEnglish[i].monthAlpha;
		break;
	}

	word = new PString(itsString);		// pull next word from content
	lowerWord = word->lower();
	matched = false;
	i = 0;
	while (!matched && i < numWords)
	{
		if (lowerWord.isMatch(months[i]))
			matched = true;
		i++;
	}

	if (matched)
		return 1;
	else
		return 0;
}
