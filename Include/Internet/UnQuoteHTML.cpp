// unQuoteHTML.cpp

#include "unQuoteHTML.h"

static void WriteUTF8 (std::ostream & Out, unsigned int Ch)
/* writes Ch in UTF-8 encoding to Out. Note this version only deals
with characters up to 16 bits. */
{
	if (Ch >= 0x800)
	{
		Out.put(0xE0 | Ch >> 12 & 0x0F);
		Out.put(0x80 | Ch >> 6 & 0x3F);
		Out.put(0x80 | Ch & 0x3F);
	}
	else if (Ch >= 0x80)
	{
		Out.put(0xC0 | Ch >> 6 & 0x1F);
		Out.put(0x80 | Ch & 0x3F);
	}
	else
	{
		Out.put(Ch);
	} /*if*/
} /*WriteUTF8*/

void UnquoteHTML(std::istream & In,std::ostream & Out)
/* copies In to Out, expanding any HTML entity references into literal
UTF-8 characters. */
{
	enum
	{
		NoMatch,
		MatchBegin,
		MatchName,
		MatchNumber,
		MatchDecimalNumber,
		MatchHexNumber,
	} MatchState;
	std::string MatchingName;
	unsigned int CharCode;
	bool ProcessedChar, GotCharCode;
	MatchState = NoMatch;
	for (;;)
	{
		const unsigned char ThisCh = In.get();
		if (In.eof())
			break;
		ProcessedChar = false; /* to begin with */
		GotCharCode = false; /* to begin with */
		switch (MatchState)
		{
		case MatchBegin:
			if (ThisCh == '#')
			{
				MatchState = MatchNumber;
				ProcessedChar = true;
			}
			else if
				(
				ThisCh >= 'a' && ThisCh <= 'z'
				||
				ThisCh >= 'A' && ThisCh <= 'Z'
				)
			{
				MatchingName.append(1, ThisCh);
				MatchState = MatchName;
				ProcessedChar = true;
			}
			else
			{
				Out.put('&');
				MatchState = NoMatch;
			} /*if*/
			break;
		case MatchName:
			if
				(
				ThisCh >= 'a' && ThisCh <= 'z'
				||
				ThisCh >= 'A' && ThisCh <= 'Z'
				||
				ThisCh >= '0' && ThisCh <= '9'
				)
			{
				MatchingName.append(1, ThisCh);
				ProcessedChar = true;
			}
			else if (ThisCh == ';')
			{
				if (EntityNames.empty())
				{
					/* first use, load EntityNames from StaticEntityNames */
					const EntityNameEntry * ThisEntry;
					ThisEntry = StaticEntityNames;
					for (;;)
					{
						if (ThisEntry->Name == NULL)
							break;
						EntityNames.insert
							(
							EntityNamePair(std::string(ThisEntry->Name), ThisEntry->Value)
							);
						++ThisEntry;
					} /*for*/
				} /*if*/
				const EntityNameMap::const_iterator NameEntry = EntityNames.find(MatchingName);
				if (NameEntry != EntityNames.end())
				{
					CharCode = NameEntry->second;
					ProcessedChar = true;
					GotCharCode = true;
				} /*if*/
			} /*if*/
			if (!ProcessedChar)
			{
				Out.put('&');
				for (unsigned int i = 0; i < MatchingName.size(); ++i)
				{
					Out.put(MatchingName[i]);
				} /*for*/
				MatchState = NoMatch;
			} /*if*/
			break;
		case MatchNumber:
			if (ThisCh == 'x' || ThisCh == 'X')
			{
				ProcessedChar = true;
				MatchState = MatchHexNumber;
				CharCode = 0;
			}
			else if (ThisCh >= '0' && ThisCh <= '9')
			{
				CharCode = ThisCh - '0';
				MatchState = MatchDecimalNumber;
				ProcessedChar = true;
			}
			else
			{
				MatchState = NoMatch;
			} /*if*/
			break;
		case MatchDecimalNumber:
			if (ThisCh >= '0' && ThisCh <= '9')
			{
				CharCode = CharCode * 10 + ThisCh - '0';
				ProcessedChar = true;
			}
			else if (ThisCh == ';')
			{
				ProcessedChar = true;
				GotCharCode = true;
			}
			else
			{
				MatchState = NoMatch;
			} /*if*/
			break;
		case MatchHexNumber:
			if (ThisCh >= '0' && ThisCh <= '9')
			{
				CharCode = CharCode * 16 + ThisCh - '0';
				ProcessedChar = true;
			}
			else if (ThisCh >= 'a' && ThisCh <= 'f')
			{
				CharCode = CharCode * 16 + ThisCh - 'a' + 10;
				ProcessedChar = true;
			}
			else if (ThisCh >= 'A' && ThisCh <= 'F')
			{
				CharCode = CharCode * 16 + ThisCh - 'A' + 10;
				ProcessedChar = true;
			}
			else if (ThisCh == ';')
			{
				ProcessedChar = true;
				GotCharCode = true;
			}
			else
			{
				MatchState = NoMatch;
			} /*if*/
			break;
		} /*switch*/
		if (GotCharCode)
		{
			WriteUTF8(Out, CharCode);
			MatchState = NoMatch;
		}
		else if (!ProcessedChar && MatchState == NoMatch)
		{
			if (ThisCh == '&')
			{
				MatchState = MatchBegin;
				MatchingName.erase();
			}
			else
			{
				Out.put(ThisCh);
			} /*if*/
		} /*if*/
	} /*for*/
} /*UnquoteHTML*/

std::wstring s2ws(const std::string& str)
{
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

std::wstring unQuoteHTML(std::wstring &input)
{
	std::stringstream in;
	std::stringstream out;

	in.str(ws2s(input));
	UnquoteHTML(in, out);
	return s2ws(out.str());
}

QString unQuoteHTML(QString &input)
{
    std::stringstream in;
    std::stringstream out;

    in.str(ws2s(input.toStdWString()));
    UnquoteHTML(in, out);
//    return QString(s2ws(out.str()));
    return QString("dummy to be fixed");
}

void findAndReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
	size_t pos = 0;
	while ((pos = str.find(oldStr, pos)) != std::string::npos)
	{
		str.replace(pos, oldStr.length(), newStr);
		pos += newStr.length();
	}
}

