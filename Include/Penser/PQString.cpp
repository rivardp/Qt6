// PQString.cpp

#include "PQString.h"

// CONSTRUCTORS

PQString::PQString()
{
}

PQString::PQString(const char * const cString)
{
    itsString = QString::fromUtf8(cString);
}

PQString::PQString(const PQString &pqstring)
{
    itsString = pqstring.getString();
}

PQString::PQString(const QString &qstring)
{
    itsString = qstring;
}

PQString::PQString(const PString &pstring)
{
    itsString = pstring.getQString();
}

PQString::PQString(const QChar& singleChar)
{
    itsString = singleChar;
}

PQString::PQString(const std::string &stdString)
{
    itsString = QString::fromStdString(stdString);
}

PQString::PQString(const std::wstring &wString)
{
    itsString = QString::fromStdWString(wString);
}

PQString::PQString(const unsigned int ui)
{
    itsString = QString::number(ui);
}

PQString::PQString(const int i)
{
    itsString = QString::number(i);
}


// DESTRUCTOR

PQString::~PQString()
{
}

// OPERATORS

QString PQString::operator[](unsigned int offset) const
{
    int offSet = static_cast<int>(offset);

    if (itsString.length() == 0)
        return itsString;
    else
    {
        if (offSet >= itsString.length())
            return itsString.at(itsString.length() - 1);
        else
            return itsString.at(offSet);
    }
}

PQString PQString::operator+(const PQString& rhs)
{
    QString temp(itsString);
    temp.append(rhs.itsString);
    return temp;
}

PQString PQString::operator+(const QString& rhs)
{
    QString temp(itsString);
    temp.append(rhs);
    return temp;
}

PQString PQString::operator+(const QChar& rhs)
{
    QString temp(itsString);
    temp.append(rhs);
    return temp;
}

void PQString::operator+=(const PQString& rhs)
{
    itsString.append(rhs.itsString);
}

void PQString::operator+=(const QString& rhs)
{
    itsString.append(rhs);
}

void PQString::operator+=(const QChar& rhs)
{
    itsString.append(rhs);
}

PQString& PQString::operator=(const PQString& rhs)
{
    if (this == &rhs)
        return *this;
    itsString = QString(rhs.getString());
    return *this;
}

PQString& PQString::operator=(const QString& rhs)
{
    itsString = rhs;
    return *this;
}

PQString& PQString::operator=(const QChar& rhs)
{
    itsString = QString(rhs);
    return *this;
}

bool PQString::operator==(const PQString& rhs) const
{
    if (itsString.compare(rhs.itsString, Qt::CaseSensitive) == 0)
        return true;
    else
        return false;
}

bool PQString::operator==(const QString& rhs) const
{
    if (itsString.compare(rhs, Qt::CaseSensitive) == 0)
        return true;
    else
        return false;
}

bool PQString::operator==(const QChar& rhs) const
{
    if (itsString.compare(QString(rhs), Qt::CaseSensitive) == 0)
        return true;
    else
        return false;
}

bool PQString::operator!=(const PQString &rhs) const
{
    if (itsString.compare(rhs.itsString, Qt::CaseSensitive) == 0)
        return false;
    else
        return true;
}

bool PQString::operator!=(const QString &rhs) const
{
    if (itsString.compare(rhs, Qt::CaseSensitive) == 0)
        return false;
    else
        return true;
}

bool PQString::operator!=(const QChar &rhs) const
{
    if (itsString.compare(QString(rhs), Qt::CaseSensitive) == 0)
        return false;
    else
        return true;
}

PQString& PQString::operator<<(const PQString& rhs)
{
    itsString.append(rhs.itsString);
    return *this;
}

// ACCESSORS

unsigned int PQString::getLength()const
{
    return static_cast<unsigned int>(itsString.length());
}

QString PQString::getString() const
{
    return itsString;
}

QString PQString::getUnaccentedString() const
{
    QChar c;
    int dIndex;
    QString result = "";
    int j = itsString.length();

    for (int i = 0; i < j; i++)
    {
        c = itsString[i];
        dIndex = diacriticLetters.indexOf(c);
        if (dIndex < 0)
            result.append(c);
        else
            result.append(noDiacriticLetters[dIndex]);
    }

    return result;
}

QString PQString::replaceLigatures() const
{
    if (itsString.length() == 0)
        return itsString;

    QVector<uint> intVector = itsString.toUcs4();
    uint maxInt = *std::max_element(intVector.begin(), intVector.end());
    if (maxInt < 64256)
        return itsString;
    else
    {
        int i = 0;
        while (i < intVector.size())
        {
            if ((intVector.at(i) >= 64256) && (intVector.at(i) <= 65279))
            {
                switch(intVector.at(i))
                {
                case 64256:     // ff
                    intVector[i] = 102;
                    intVector.insert(i+1, 102);
                    i++;
                    break;

                case 64257:     // fi
                    intVector[i] = 102;
                    intVector.insert(i+1, 105);
                    i++;
                    break;

                case 64258:     // fl
                    intVector[i] = 102;
                    intVector.insert(i+1, 108);
                    i++;
                    break;

                case 64259:     // ffi
                    intVector[i] = 102;
                    intVector.insert(i+1, 102);
                    intVector.insert(i+2, 105);
                    i+=2;
                    break;

                case 64260:     // ffl
                    intVector[i] = 102;
                    intVector.insert(i+1, 102);
                    intVector.insert(i+2, 108);
                    i+=2;
                    break;

                case 64261:     // ft
                    intVector[i] = 102;
                    intVector.insert(i+1, 116);
                    i++;
                    break;

                case 64279:     // non breaking space
                    intVector[i] = 32;
                    break;

                default:
                    // Do nothing
                    break;
                }
            }
            i++;
        }
    }

    QString newString;
    for (int i = 0; i < intVector.size(); i++)
    {
        int cappedI = intVector.at(i);
        if (cappedI > 65535)
            cappedI = 65533;
        newString += QString(QChar(cappedI));
    }

    return newString;
}

std::string PQString::getStdString() const
{
    return itsString.toStdString();
}

std::wstring PQString::getWString() const
{
    return itsString.toStdWString();
}

char * PQString::convertToCharPtr() const
{
    QByteArray ba = itsString.toLocal8Bit();

    return ba.data();
}

unsigned int PQString::convertCharToANSI()
{
    unsigned int ANSINum;
    ushort unicodeNum = itsString.at(0).unicode();

    if (unicodeNum < 256)
        return unicodeNum;

    switch(unicodeNum)
    {
    case 256:   // Ā
    case 258:   // Ă
    case 260:   // Ą
    case 461:   // Ǎ
        ANSINum = 65;
        break;

    case 257:   // ā
    case 259:   // ă
    case 261:   // ą
    case 462:   // ǎ
        ANSINum = 97;
        break;

    case 262:   // Ć
    case 264:   // Ĉ
    case 266:   // Ċ
    case 268:   // Č
        ANSINum = 67;
        break;

    case 263:   // ć
    case 265:   // ĉ
    case 267:   // ċ
    case 269:   // č
        ANSINum = 99;
        break;

    case 270:   // Ď
    case 272:   // Đ
        ANSINum = 68;
        break;

    case 271:   // ď
    case 273:   // đ
        ANSINum = 100;
        break;

    case 274:   // Ē
    case 276:   // Ĕ
    case 278:   // Ė
    case 280:   // Ę
    case 282:   // Ě
        ANSINum = 69;
        break;

    case 275:   // ē
    case 277:   // ĕ
    case 279:   // ė
    case 281:   // ę
    case 283:   // ě
        ANSINum = 101;
        break;

    case 284:   // Ĝ
    case 286:   // Ğ
    case 288:   // Ġ
    case 290:   // Ģ
    case 486:   // Ǧ
        ANSINum = 71;
        break;

    case 285:   // ĝ
    case 287:   // ğ
    case 289:   // ġ
    case 291:   // ģ
    case 487:   // ǧ
        ANSINum = 103;
        break;

    case 292:   // Ĥ
    case 294:   // Ħ
    case 542:   // Ȟ
        ANSINum = 72;
        break;

    case 293:   // ĥ
    case 295:   // ħ
    case 543:   // ȟ
        ANSINum = 104;
        break;

    case 296:   // Ĩ
    case 298:   // Ī
    case 300:   // Ĭ
    case 302:   // Į
    case 304:   // İ
    case 463:   // Ǐ
        ANSINum = 73;
        break;

    case 297:   // ĩ
    case 299:   // ī
    case 301:   // ĭ
    case 303:   // į
    case 305:   // i̇
    case 464:   // ǐ
        ANSINum = 105;
        break;

    case 308:   // Ĵ
    case 495:   // J̌
        ANSINum = 74;
        break;

    case 309:   // ĵ
    case 496:   // ǰ
        ANSINum = 106;
        break;

    case 310:   // Ķ
    case 488:   // Ǩ
        ANSINum = 75;
        break;

    case 311:   // ķ
    case 312:   // ĸ
    case 489:   // ǩ
        ANSINum = 107;
        break;

    case 313:   // Ĺ
    case 315:   // Ļ
    case 317:   // Ľ
    case 319:   // Ŀ
    case 321:   // Ł
        ANSINum = 76;
        break;

    case 314:   // ĺ
    case 316:   // ļ
    case 318:   // ľ
    case 320:   // ŀ
    case 322:   // ł
        ANSINum = 108;
        break;

    case 323:   // Ń
    case 325:   // Ņ
    case 327:   // Ň
    case 330:   // Ŋ
        ANSINum = 78;
        break;

    case 324:   // ń
    case 326:   // ņ
    case 328:   // ň
    case 329:   // ņ
    case 331:   // ŋ
        ANSINum = 110;
        break;

    case 332:   // Ō
    case 334:   // Ŏ
    case 336:   // Ő
    case 465:   // Ǒ
        ANSINum = 79;
        break;

    case 333:   // ō
    case 335:   // ŏ
    case 337:   // ő
    case 466:   // ǒ
        ANSINum = 111;
        break;

    case 340:   // Ŕ
    case 342:   // Ŗ
    case 344:   // Ř
        ANSINum = 82;
        break;

    case 341:   // ŕ
    case 343:   // ŗ
    case 345:   // ř
        ANSINum = 114;
        break;

    case 346:   // Ś
    case 348:   // Ŝ
    case 350:   // Ş
    case 352:   // Š
        ANSINum = 83;
        break;

    case 347:   // ś
    case 349:   // ŝ
    case 351:   // ş
    case 353:   // š
        ANSINum = 115;
        break;

    case 354:   // Ţ
    case 356:   // Ť
    case 358:   // Ŧ
        ANSINum = 84;
        break;

    case 355:   // ţ
    case 357:   // ť
    case 359:   // ŧ
        ANSINum = 116;
        break;

    case 360:   // Ũ
    case 362:   // Ū
    case 364:   // Ŭ
    case 366:   // Ů
    case 368:   // Ű
    case 370:   // Ų
    case 467:   // Ǔ
        ANSINum = 85;
        break;

    case 361:   // ũ
    case 363:   // ū
    case 365:   // ŭ
    case 367:   // ů
    case 369:   // ű
    case 371:   // ų
    case 468:   // ǔ
        ANSINum = 117;
        break;

    case 372:   // Ŵ
        ANSINum = 87;
        break;

    case 373:   // ŵ
        ANSINum = 119;
        break;

    case 374:   // Ŷ
    case 376:   // ÿ
        ANSINum = 89;
        break;

    case 375:   // ŷ
        ANSINum = 121;
        break;

    case 377:   // Ź
    case 379:   // Ż
    case 381:   // Ž
        ANSINum = 90;
        break;

    case 378:   // ź
    case 380:   // ż
    case 382:   // ž
        ANSINum = 122;
        break;

    case 8211:      // –	0x96	U+2013	&ndash
        ANSINum = 150;
        break;

    case 8212:      // —	0x97	U+2014	&mdash
        ANSINum = 151;
        break;

    case 8216:      // ‘	0x91	U+2018	&lsquo
        ANSINum = 145;
        break;

    case 8217:      // ’	0x92	U+2019	&rsquo
        ANSINum = 146;
        break;

    case 8218:      // ‚	0x82	U+201A	&sbquo
        ANSINum = 130;
        break;

    case 8220:      // “	0x93	U+201C	&ldquo
        ANSINum = 147;
        break;

    case 8221:      // ”	0x94	U+201D	&rdquo
        ANSINum = 148;
        break;

    case 8222:      // „	0x84	U+201E	&bdquo
        ANSINum = 132;
        break;

    case 8224:      // †	0x86	U+2020	&dagger
        ANSINum = 134;
        break;

    case 8225:      // ‡	0x87	U+2021	&Dagger
        ANSINum = 135;
        break;

    case 8226:      // •	0x95	U+2022	&bull
        ANSINum = 149;
        break;

    case 8230:      // …	0x85	U+2026	&hellip
        ANSINum = 133;
        break;

    case 8240:      // ‰	0x89	U+2030	&permil
        ANSINum = 137;
        break;

    case 8249:      // ‹	0x8B	U+2039	&lsaquo
        ANSINum = 139;
        break;

    case 8250:      // ›	0x9B	U+203A	&rsaquo
        ANSINum = 155;
        break;

    case 8364:      // €	0x80	U+20AC	&euro
        ANSINum = 128;
        break;

    case 8482:      // ™	0x99	U+2122	&trade
        ANSINum = 153;
        break;

    case 65279:      // Zero width, non-breaking space - Replace with space
        ANSINum = 32;
        break;

    default:
        ANSINum = 191;  // Character outside of ANSI range, so replace with upside down question mark
    }

    QChar newChar(ANSINum);
    itsString = newChar;

    return ANSINum;
}

void PQString::convertToQStringList(QStringList &nameList)
{
    if (!isHyphenated())
        nameList.append(itsString);
    else
    {
        if (itsString.contains(QChar(45), Qt::CaseSensitive))
            nameList = itsString.split(QChar(45), Qt::SkipEmptyParts, Qt::CaseSensitive);
        else
            nameList = itsString.split(QChar(8211), Qt::SkipEmptyParts, Qt::CaseSensitive);
    }

    PQString name;
    for (int i = 0; i < nameList.size(); i++)
    {
        name = nameList[i];
        nameList[i] = name.proper().getString();
    }
}

PQString PQString::upper() const
{
    unsigned int currentCharType, numANSI;
    int length = itsString.length();
    QString temp(length,' ');
    QChar qChar;
    for (int i = 0; i < length; i++)
	{
        qChar = itsString.at(i);
        numANSI = qChar.unicode();

        // Address differences between ANSI and UNICODE below 256
        if(qChar.unicode() >= 256)
        {
            PQString newSingle(qChar);
            numANSI = newSingle.convertCharToANSI();
        }

        currentCharType = ANSI[numANSI].charType_defns;
		if ((currentCharType & LOWER) == LOWER)
            temp[i] = QChar(ANSI[numANSI].lowerUpperMatch);
        else
            temp[i] = itsString.at(i);
	}
	return temp;
}

PQString PQString::lower() const
{
    unsigned int currentCharType, numANSI;
    int length = itsString.length();
    QString temp(length,' ');
    QChar qChar;
    for (int i = 0; i < length; i++)
	{
        qChar = itsString.at(i);
        numANSI = qChar.unicode();

        // Address differences between ANSI and UNICODE below 256
        if(qChar.unicode() >= 256)
        {
            PQString newSingle(qChar);
            numANSI = newSingle.convertCharToANSI();
        }

        currentCharType = ANSI[numANSI].charType_defns;
        if ((currentCharType & UPPER) == UPPER)
            temp[i] = QChar(ANSI[numANSI].lowerUpperMatch);
		else
            temp[i] = itsString.at(i);
	}
	return temp;
}

PQString PQString::proper() const
{
    PQString singleChar, priorChar, priorTwo, priorThree;
    PQString result;

    PQString space(" ");
    PQString hyphen("-");
    PQString apostrophe("'");
    PQString period(".");
    PQString tempString(itsString);
    bool charAdded, inParentheses, inQuotes;

    inParentheses = tempString.removeBookEnds(PARENTHESES);
    inQuotes = tempString.removeBookEnds(QUOTES);

    unsigned int length = tempString.getLength();

    if (length > 0)
	{
		singleChar = tempString[0];
		result += singleChar.upper();
	}

    for (unsigned int i = 1; i < length; i++)
	{
		// Use lower case with certain exceptions, where input is assumed to be correct
        priorChar = tempString[i - 1];
        singleChar = tempString[i];
		charAdded = false;

        // After an apostrophe in the second position, capitalize (eg. O'Toole)
        if ((i == 2) && (priorChar == apostrophe))
        {
            result += singleChar.upper();
            charAdded = true;
        }

        // After a period, capitalize the next letter (used with compression to obtain "St.Vincent")
        if (priorChar == period)
        {
            result += singleChar.upper();
            charAdded = true;
        }

        // After a space, hyphen or "Mc" or "Mac", the user input case is used unless the word is allCaps
        if ((priorChar == space) || (priorChar == hyphen))
		{
			result += singleChar;
			charAdded = true;
		}

        // After a slash, capitalize the next letter (used for multiple names "Smith/Johnson")
        if ((priorChar.getCharType() & SLASH) == SLASH)
        {
            result += singleChar.upper();
            charAdded = true;
        }

        if (!charAdded && (i >= 2))
		{
            priorTwo = result.middle(static_cast<unsigned int>(i - 2), 2);
            if (priorTwo == QString("Mc"))
			{
                // Third char always capitalized unless written like "Mcdonald"
                result += singleChar;
				charAdded = true;
			}
		}

		if (!charAdded && (i >= 3))
		{
            priorThree = result.middle(static_cast<unsigned int>(i - 3), 3);
            if ((priorThree == QString("Mac")) && (length > 4))
			{
                // Fourth char always capitalized unless written like "Macdonald" or "Mack"
                result += singleChar;
				charAdded = true;
			}
		}

		if (!charAdded)
			result += singleChar.lower();
	}

    if (inParentheses){
        result = PQString("(") + result + PQString(")");}
    if (inQuotes){
        result = PQString("\"") + result + PQString("\"");}

	return result;
}

PQString PQString::preHyphen() const
{
    int index = findPosition(PQString("-"), -1);

    if (index == -1)
        return PQString(itsString);
    else
        return PQString(itsString.left(index));
}

PQString PQString::postHyphen() const
{
    if (!((itsString.at(0) == QString("-")) || isHyphenated()))
        return PQString(itsString);
    else
    {
        int index = findPosition(PQString("-"), -1);
        return PQString(itsString.right(itsString.length() - (index + 1)));
    }
}

PQString PQString::reciprocal() const
{
    if (itsString.length() == 1)
    {
        QChar qChar = itsString.at(0);
        unsigned int numANSI = qChar.unicode();

        // Address differences between ANSI and UNICODE below 256
        if(qChar.unicode() >= 256)
        {
            PQString newSingle(qChar);
            numANSI = newSingle.convertCharToANSI();
            unsigned int recipANSI = ANSI[numANSI].lowerUpperMatch;
            return PQString(QChar(ANSI[recipANSI].unicode));
        }

        return PQString(QChar(ANSI[numANSI].lowerUpperMatch));
    }
	else
		return *this;
}

PQString& PQString::simplify(bool insertPeriods)
{    
    // Use "¤" instead of actual "." to avoid messing up content for a few sites that use "<span>.</span>" to create sentences.
    if (insertPeriods)
    {
        while (itsString.contains("\n\n"))
        {
            itsString.replace("\n\n", "\n");
        }
        itsString.replace(QString("\n"), QString("¤ "));
    }

    itsString = itsString.simplified();

    while (itsString.contains("¤ ¤"))
    {
        itsString.replace("¤ ¤", "¤");
    }

    while (itsString.contains(" ¤"))
    {
        itsString.replace(" ¤", "¤");
    }

    while (itsString.contains("¤¤"))
    {
        itsString.replace("¤¤", "¤");
    }

    itsString.replace(QString(">¤ "), QString(">"));
    itsString.replace(QString(">¤"), QString(">"));
    itsString.replace(QString(".¤"), QString("."));
    itsString.replace(QString("¤."), QString("."));
    itsString.replace(QString("¤ "), QString(" "));

    return *this;
}

PQString& PQString::JSONsimplify()
{
    itsString.replace("\\n", "");
    itsString.replace("\\t", "");

    unescapeJSONformat();
    replaceHTMLentities();

    return *this;
}


PQString& PQString::unescapeJSONformat()
{
    itsString.replace(QString("\\\\"), QString("\\"));
    itsString.replace(QString("\\/"), QString("/"));
    itsString.replace(QString("\\'"), QString("\'"));
    itsString.replace(QString("\\\""), QString("\""));
    itsString.replace(QString("\\?"), QString("?"));

    return *this;
}

PQString& PQString::replaceIneligibleURLchars()
{
    itsString.replace(QString(","), QString("%2c"));
    itsString.replace(QString(" "), QString("%20"));
    itsString.replace(QString("\""), QString("%22"));
    itsString.replace(QString("'"), QString("%27"));
    itsString.replace(QString(":"), QString("%3a"));
    itsString.replace(QString("‘"), QString("%91"));
    itsString.replace(QString("’"), QString("%92"));
    itsString.replace(QString("“"), QString("%93"));
    itsString.replace(QString("”"), QString("%94"));

    // Undo colon to reinstate https://
    itsString.replace(QString("%3a//"), QString("://"));

    return *this;
}

PQString PQString::left(unsigned int num) const
{
    return itsString.left(static_cast<int>(num));
}

PQString PQString::right(unsigned int num) const
{
    return itsString.right(static_cast<int>(num));
}

PQString PQString::middle(unsigned int offset, unsigned int num) const
{
    return itsString.mid(static_cast<int>(offset), static_cast<int>(num));
}

void PQString::clear()
{
    itsString.clear();
}

void PQString::dropLeft(unsigned int num)
{
    itsString.remove(0, static_cast<int>(num));
}

void PQString::dropRight(unsigned int num)
{
    itsString.chop(static_cast<int>(num));
}

void PQString::purge(QList<QString> &purgeList)
{
    int listSize = purgeList.size();

    for (int i = 0; i < listSize; i++)
        itsString.replace(purgeList.at(i), QString(""), Qt::CaseInsensitive);

    itsString.replace(QString("()"), QString(""), Qt::CaseInsensitive);
}


void PQString::parseContentInto(std::vector<PQString*> &wordVector)
{
    // Assume no breaking on spaces
    bool slash, parentheses, space;
    PQString singleChar, word;
    bool wordStarted = false;

    int itsLength = itsString.length();

    for (int i = 0; i < itsLength; i++)
    {
        singleChar = itsString.at(i);
        slash = (singleChar.getCharType() & SLASH) == SLASH;
        parentheses = (singleChar.getCharType() & PARENTHESES) == PARENTHESES;
        space = (singleChar.getCharType() & SPACE) == SPACE;

        if (wordStarted)
        {
            if (slash || parentheses)
            {
                wordStarted = false;
                word.removeEnding(_T(" "));
                wordVector.push_back(new PQString(word));
                word.clear();
            }
            else
                word += singleChar;
        }
        else
        {
            if (!space && !slash && !parentheses)
            {
                wordStarted = true;
                word += singleChar;
            }
        }

        // complete last word if last character read
        if (wordStarted && (i == (itsString.length() - 1)))
        {
            word.removeEnding(_T(" "));
            wordVector.push_back(new PQString(word));
        }
    }
}

void PQString::removeSpecialChar()
{
	// Remove escape character (\), ampersand (&) and double quote (") since they will interfere with streams
    itsString.remove(QChar('"'));
    itsString.remove(QChar('&'));
    itsString.remove(QChar('\\'));
}

void PQString::removeQuotes()
{
    itsString.remove(QChar('"'));
}

bool PQString::removeHyphens()
{
    QString orig(itsString);

    itsString.remove(QChar(45));
    itsString.remove(QChar(8211));

    return (itsString != orig);
}

void PQString::removeBlankSentences()
{
    while (itsString.contains(QString(". .")))
    {
        itsString.replace(QRegularExpression("\\. \\."),". ");
    }
}

void PQString::decodeHTMLchars()
{
    static QRegularExpression re("&#([0-9]+);");
    static QRegularExpressionMatch match;
    int number;

    match = re.match(itsString);
    while (match.hasMatch())
    {
        number = match.captured(1).toInt(0,10);
        if (number <= 65535)
            itsString.replace(match.captured(0), QChar(number));
        else
            itsString.replace(match.captured(0), "");
        match = re.match(itsString);
    }
}

void PQString::replaceHTMLentities()
{
    decodeHTMLchars();

    itsString.replace(QString("&dash;"), QString("-"), Qt::CaseInsensitive);
    itsString.replace(QString("&mdash;"), QString("-"), Qt::CaseInsensitive);
    itsString.replace(QString("&ndash;"), QString("-"), Qt::CaseInsensitive);
    itsString.replace(QString("&nbsp;"), QString(" "), Qt::CaseInsensitive);
    itsString.replace(QString("&lt;"), QString("<"), Qt::CaseInsensitive);
    itsString.replace(QString("&gt;"), QString(">"), Qt::CaseInsensitive);
    itsString.replace(QString("&amp;"), QString("&"), Qt::CaseInsensitive);
    itsString.replace(QString("&quot;"), QString("\""), Qt::CaseInsensitive);
    itsString.replace(QString("&apos;"), QString("'"), Qt::CaseInsensitive);
    itsString.replace(QString("&quot;"), QString(""""), Qt::CaseSensitive);
    itsString.replace(QString("&euro;"), QString("€"), Qt::CaseSensitive);
    itsString.replace(QString("&sbquo;"), QString("‚"), Qt::CaseSensitive);
    itsString.replace(QString("&fnof;"), QString("ƒ"), Qt::CaseSensitive);
    itsString.replace(QString("&bdquo;"), QString("„"), Qt::CaseSensitive);
    itsString.replace(QString("&hellip;"), QString("…"), Qt::CaseSensitive);
    itsString.replace(QString("&dagger;"), QString("†"), Qt::CaseSensitive);
    itsString.replace(QString("&Dagger;"), QString("‡"), Qt::CaseSensitive);
    itsString.replace(QString("&circ;"), QString("ˆ"), Qt::CaseSensitive);
    itsString.replace(QString("&permil;"), QString("‰"), Qt::CaseSensitive);
    itsString.replace(QString("&Scaron;"), QString("Š"), Qt::CaseSensitive);
    itsString.replace(QString("&lsaquo;"), QString("‹"), Qt::CaseSensitive);
    itsString.replace(QString("&OElig;"), QString("Œ"), Qt::CaseSensitive);
    itsString.replace(QString("&lsquo;"), QString("'"), Qt::CaseSensitive);
    itsString.replace(QString("&rsquo;"), QString("'"), Qt::CaseSensitive);
    itsString.replace(QString("&ldquo;"), QString("'"), Qt::CaseSensitive);
    itsString.replace(QString("&rdquo;"), QString("'"), Qt::CaseSensitive);
    itsString.replace(QString("&bull;"), QString("•"), Qt::CaseSensitive);
    itsString.replace(QString("&ndash;"), QString("–"), Qt::CaseSensitive);
    itsString.replace(QString("&mdash;"), QString("—"), Qt::CaseSensitive);
    itsString.replace(QString("&tilde;"), QString("˜"), Qt::CaseSensitive);
    itsString.replace(QString("&trade;"), QString("™"), Qt::CaseSensitive);
    itsString.replace(QString("&scaron;"), QString("š"), Qt::CaseSensitive);
    itsString.replace(QString("&rsaquo;"), QString("›"), Qt::CaseSensitive);
    itsString.replace(QString("&oelig;"), QString("œ"), Qt::CaseSensitive);
    itsString.replace(QString("&Yuml;"), QString("Ÿ"), Qt::CaseSensitive);
    itsString.replace(QString("&iexcl;"), QString("¡"), Qt::CaseSensitive);
    itsString.replace(QString("&cent;"), QString("¢"), Qt::CaseSensitive);
    itsString.replace(QString("&pound;"), QString("£"), Qt::CaseSensitive);
    itsString.replace(QString("&curren;"), QString("¤"), Qt::CaseSensitive);
    itsString.replace(QString("&yen;"), QString("¥"), Qt::CaseSensitive);
    itsString.replace(QString("&brvbar;"), QString("¦"), Qt::CaseSensitive);
    itsString.replace(QString("&sect;"), QString("§"), Qt::CaseSensitive);
    itsString.replace(QString("&uml;"), QString("¨"), Qt::CaseSensitive);
    itsString.replace(QString("&copy;"), QString("©"), Qt::CaseSensitive);
    itsString.replace(QString("&ordf;"), QString("ª"), Qt::CaseSensitive);
    itsString.replace(QString("&laquo;"), QString("«"), Qt::CaseSensitive);
    itsString.replace(QString("&not;"), QString("¬"), Qt::CaseSensitive);
    itsString.replace(QString("&shy;"), QString("­"), Qt::CaseSensitive);
    itsString.replace(QString("&reg;"), QString("®"), Qt::CaseSensitive);
    itsString.replace(QString("&macr;"), QString("¯"), Qt::CaseSensitive);
    itsString.replace(QString("&deg;"), QString("°"), Qt::CaseSensitive);
    itsString.replace(QString("&plusmn;"), QString("±"), Qt::CaseSensitive);
    itsString.replace(QString("&sup2;"), QString("²"), Qt::CaseSensitive);
    itsString.replace(QString("&sup3;"), QString("³"), Qt::CaseSensitive);
    itsString.replace(QString("&acute;"), QString("´"), Qt::CaseSensitive);
    itsString.replace(QString("&micro;"), QString("µ"), Qt::CaseSensitive);
    itsString.replace(QString("&para;"), QString("¶"), Qt::CaseSensitive);
    itsString.replace(QString("&middot;"), QString("·"), Qt::CaseSensitive);
    itsString.replace(QString("&cedil;"), QString("¸"), Qt::CaseSensitive);
    itsString.replace(QString("&sup1;"), QString("¹"), Qt::CaseSensitive);
    itsString.replace(QString("&ordm;"), QString("º"), Qt::CaseSensitive);
    itsString.replace(QString("&raquo;"), QString("»"), Qt::CaseSensitive);
    itsString.replace(QString("&frac14;"), QString("¼"), Qt::CaseSensitive);
    itsString.replace(QString("&frac12;"), QString("½"), Qt::CaseSensitive);
    itsString.replace(QString("&frac34;"), QString("¾"), Qt::CaseSensitive);
    itsString.replace(QString("&iquest;"), QString("¿"), Qt::CaseSensitive);
    itsString.replace(QString("&Agrave;"), QString("À"), Qt::CaseSensitive);
    itsString.replace(QString("&Aacute;"), QString("Á"), Qt::CaseSensitive);
    itsString.replace(QString("&Acirc;"), QString("Â"), Qt::CaseSensitive);
    itsString.replace(QString("&Atilde;"), QString("Ã"), Qt::CaseSensitive);
    itsString.replace(QString("&Auml;"), QString("Ä"), Qt::CaseSensitive);
    itsString.replace(QString("&Aring;"), QString("Å"), Qt::CaseSensitive);
    itsString.replace(QString("&AElig;"), QString("Æ"), Qt::CaseSensitive);
    itsString.replace(QString("&Ccedil;"), QString("Ç"), Qt::CaseSensitive);
    itsString.replace(QString("&Egrave;"), QString("È"), Qt::CaseSensitive);
    itsString.replace(QString("&Eacute;"), QString("É"), Qt::CaseSensitive);
    itsString.replace(QString("&Ecirc;"), QString("Ê"), Qt::CaseSensitive);
    itsString.replace(QString("&Euml;"), QString("Ë"), Qt::CaseSensitive);
    itsString.replace(QString("&Igrave;"), QString("Ì"), Qt::CaseSensitive);
    itsString.replace(QString("&Iacute;"), QString("Í"), Qt::CaseSensitive);
    itsString.replace(QString("&Icirc;"), QString("Î"), Qt::CaseSensitive);
    itsString.replace(QString("&Iuml;"), QString("Ï"), Qt::CaseSensitive);
    itsString.replace(QString("&ETH;"), QString("Ð"), Qt::CaseSensitive);
    itsString.replace(QString("&Ntilde;"), QString("Ñ"), Qt::CaseSensitive);
    itsString.replace(QString("&Ograve;"), QString("Ò"), Qt::CaseSensitive);
    itsString.replace(QString("&Oacute;"), QString("Ó"), Qt::CaseSensitive);
    itsString.replace(QString("&Ocirc;"), QString("Ô"), Qt::CaseSensitive);
    itsString.replace(QString("&Otilde;"), QString("Õ"), Qt::CaseSensitive);
    itsString.replace(QString("&Ouml;"), QString("Ö"), Qt::CaseSensitive);
    itsString.replace(QString("&times;"), QString("×"), Qt::CaseSensitive);
    itsString.replace(QString("&Oslash;"), QString("Ø"), Qt::CaseSensitive);
    itsString.replace(QString("&Ugrave;"), QString("Ù"), Qt::CaseSensitive);
    itsString.replace(QString("&Uacute;"), QString("Ú"), Qt::CaseSensitive);
    itsString.replace(QString("&Ucirc;"), QString("Û"), Qt::CaseSensitive);
    itsString.replace(QString("&Uuml;"), QString("Ü"), Qt::CaseSensitive);
    itsString.replace(QString("&Yacute;"), QString("Ý"), Qt::CaseSensitive);
    itsString.replace(QString("&THORN;"), QString("Þ"), Qt::CaseSensitive);
    itsString.replace(QString("&szlig;"), QString("ß"), Qt::CaseSensitive);
    itsString.replace(QString("&agrave;"), QString("à"), Qt::CaseSensitive);
    itsString.replace(QString("&aacute;"), QString("á"), Qt::CaseSensitive);
    itsString.replace(QString("&acirc;"), QString("â"), Qt::CaseSensitive);
    itsString.replace(QString("&atilde;"), QString("ã"), Qt::CaseSensitive);
    itsString.replace(QString("&auml;"), QString("ä"), Qt::CaseSensitive);
    itsString.replace(QString("&aring;"), QString("å"), Qt::CaseSensitive);
    itsString.replace(QString("&aelig;"), QString("æ"), Qt::CaseSensitive);
    itsString.replace(QString("&ccedil;"), QString("ç"), Qt::CaseSensitive);
    itsString.replace(QString("&egrave;"), QString("è"), Qt::CaseSensitive);
    itsString.replace(QString("&eacute;"), QString("é"), Qt::CaseSensitive);
    itsString.replace(QString("&ecirc;"), QString("ê"), Qt::CaseSensitive);
    itsString.replace(QString("&euml;"), QString("ë"), Qt::CaseSensitive);
    itsString.replace(QString("&igrave;"), QString("ì"), Qt::CaseSensitive);
    itsString.replace(QString("&iacute;"), QString("í"), Qt::CaseSensitive);
    itsString.replace(QString("&icirc;"), QString("î"), Qt::CaseSensitive);
    itsString.replace(QString("&iuml;"), QString("ï"), Qt::CaseSensitive);
    itsString.replace(QString("&eth;"), QString("ð"), Qt::CaseSensitive);
    itsString.replace(QString("&ntilde;"), QString("ñ"), Qt::CaseSensitive);
    itsString.replace(QString("&ograve;"), QString("ò"), Qt::CaseSensitive);
    itsString.replace(QString("&oacute;"), QString("ó"), Qt::CaseSensitive);
    itsString.replace(QString("&ocirc;"), QString("ô"), Qt::CaseSensitive);
    itsString.replace(QString("&otilde;"), QString("õ"), Qt::CaseSensitive);
    itsString.replace(QString("&ouml;"), QString("ö"), Qt::CaseSensitive);
    itsString.replace(QString("&divide;"), QString("÷"), Qt::CaseSensitive);
    itsString.replace(QString("&oslash;"), QString("ø"), Qt::CaseSensitive);
    itsString.replace(QString("&ugrave;"), QString("ù"), Qt::CaseSensitive);
    itsString.replace(QString("&uacute;"), QString("ú"), Qt::CaseSensitive);
    itsString.replace(QString("&ucirc;"), QString("û"), Qt::CaseSensitive);
    itsString.replace(QString("&uuml;"), QString("ü"), Qt::CaseSensitive);
    itsString.replace(QString("&yacute;"), QString("ý"), Qt::CaseSensitive);
    itsString.replace(QString("&thorn;"), QString("þ"), Qt::CaseSensitive);
    itsString.replace(QString("&yuml;"), QString("ÿ"), Qt::CaseSensitive);

    itsString.replace(QString("&#8203;"), QString(""), Qt::CaseInsensitive);
    itsString.replace(QString("&#x2F;"), QString("/"), Qt::CaseInsensitive);
}

void PQString::replace(const QString target, const QString newString, const Qt::CaseSensitivity caseSensitive)
{
    itsString.replace(target, newString, caseSensitive);
}

void PQString::replace(const int start, const int numChars, const QString newString)
{
    itsString.replace(start, numChars, newString);
}

QString PQString::unQuoteHTML()
{
    /*QTextDocument text;
    text.setHtml(itsString);
    itsString = text.toPlainText();
    // Run a second time to pick up unusual coding like "&amp;quot;"
    text.setHtml(itsString);
    itsString = text.toPlainText();*/

    static QRegularExpression re("\\\\u([0-9a-fA-F]{4})");
    static QRegularExpressionMatch match;

    match = re.match(itsString);
    while (match.hasMatch())
    {
        itsString.replace(match.captured(0), QChar(match.captured(1).toUShort(nullptr, 16)));
        match = re.match(itsString);
    }

    // Second process to catch anything left
    int idx = -1;
    while ((idx = itsString.indexOf("\\u")) != -1)
    {
        int nHex = itsString.mid(idx + 2, 4).toInt(0, 16);
        itsString.replace(idx, 6, QChar(nHex));
    }

    return itsString;
}

bool PQString::hasBookEnds(const unsigned int bookends)
{
    int length = itsString.length();

    if (length == 0)
		return false;

	bool hasParentheses, hasQuotes;
    PQString firstChar = itsString.at(0);
    PQString lastChar = itsString.at(length - 1);

	hasParentheses = ((bookends & PARENTHESES) == PARENTHESES) && ((firstChar.getCharType() & PARENTHESES) == PARENTHESES)
		&& ((lastChar.getCharType() & PARENTHESES) == PARENTHESES);
	hasQuotes = ((bookends & QUOTES) == QUOTES) && ((firstChar.getCharType() & QUOTES) == QUOTES)
		&& ((lastChar.getCharType() & QUOTES) == QUOTES);

	return hasParentheses || hasQuotes;
}


bool PQString::removeBookEnds(unsigned int bookends, bool runRecursive)
{
    if (itsString.length() == 0)
		return false;

	bool hasParentheses, hasQuotes;

	hasParentheses = hasBookEnds(bookends & PARENTHESES);
	hasQuotes = hasBookEnds(bookends & QUOTES);

	if (hasParentheses || hasQuotes)
	{
		dropLeft(1);
		dropRight(1);
        removeLeading(" ");
        removeEnding (" ");

        if (runRecursive)
            removeBookEnds(bookends, runRecursive);
    }

	return hasParentheses || hasQuotes;
}

bool PQString::removeLeading(const std::wstring target)
{
    int length = static_cast<int>(target.size());
    if (length > itsString.length())
        return false;

    bool matched = true;

    for (int i = 0; i < length; i++)
        matched = matched && (itsString.at(i) == target[static_cast<unsigned long long>(i)]);

    if (matched)
    {
        dropLeft(static_cast<unsigned int>(length));
        // If a single char target, look for repeated chars
        removeLeading(target);
    }

    return matched;
}

bool PQString::removeLeading(const unsigned int charCode)
{
    if (itsString.length() < 1)
        return false;

    PQString firstChar = left(1);
    // if ((firstChar.getCharType() & charCode) == charCode)  // original code worked, but limited to single charCode
    if ((firstChar.getCharType() & charCode) > 0)
    {
        dropLeft(1);
        // check if multiple targets existed (e.g. trailing blanks)
        removeLeading(charCode);
        return true;
    }

    return false;
}

bool PQString::removeLeading(const QString target)
{
    int length = static_cast<int>(target.size());
    if (length > itsString.length())
        return false;

    bool matched = true;

    for (int i = 0; i < length; i++)
        matched = matched && (itsString.at(i) == target.at(i));

    if (matched)
    {
        dropLeft(static_cast<unsigned int>(length));
        // If a single or multiple char target, look for repeated target
        removeLeading(target);
    }

    return matched;
}

bool PQString::removeEnding(const std::wstring target)
{
    int itsLength = itsString.length();
    int removeLength = static_cast<int>(target.size());
    if (removeLength > itsLength){
        return false;}

	bool matched = true;

    for (int i = 0; i < removeLength; i++){
        matched = matched && (itsString.at(itsLength - removeLength + i) == target[static_cast<unsigned long long>(i)]);}

	if (matched)
	{
        dropRight(static_cast<unsigned int>(removeLength));
		// check if multiple targets existed (e.g. trailing blanks)
        if (removeLength == 1)
			removeEnding(target);
	}

	return matched;
}

bool PQString::removeEnding(const unsigned int charCode)
{
    if (itsString.length() < 1)
        return false;

    PQString lastChar = right(1);
    // if ((lastChar.getCharType() & charCode) == charCode)  // original code worked, but limited to single charCode
    if (((lastChar.getCharType() & charCode) > 0) || (lastChar == PQString(QString(QChar(65533)))))
    {
		dropRight(1);
        // check if multiple targets existed (e.g. trailing blanks)
        removeEnding(charCode);
        return true;
	}

	return false;
}

bool PQString::removeEnding(const QString target)
{
    int itsLength = itsString.length();
    int removeLength = static_cast<int>(target.size());
    if (removeLength > itsLength)
        return false;

    bool matched = true;

    for (int i = 0; i < removeLength; i++)
        matched = matched && (itsString.at(itsLength - removeLength + i) == target.at(i));

    if (matched)
    {
        dropRight(static_cast<unsigned int>(removeLength));
        // check if multiple targets existed (e.g. trailing blanks)
        if (removeLength == 1)
            removeEnding(target);
    }

    return matched;
}

bool PQString::removeAll(const QString target)
{
    bool matched = itsString.contains(target);
    if (matched)
    {
        QString nothing;
        itsString.replace(target, nothing);
    }

    return matched;
}

bool PQString::removeLineFormatting()
{
    bool removed;
    removed = removeAll("\t");
    removed = removed || removeAll("\n");

    return removed;
}

bool PQString::removePossessive()
{
    if (itsString.length() < 3)
        return false;

    bool changesMade = false;
    PQString lastChar, penUltimateChar;

    // Two potential cases, Paul's and Chris'
    lastChar = itsString.right(1);
    penUltimateChar = itsString.mid(itsString.length() - 2, 1);

    if ((lastChar == PQString("s")) && (penUltimateChar.isSingleQuote()))
    {
        itsString.chop(2);
        changesMade = true;
    }
    else
    {
        bool inQuotes = hasBookEnds(QUOTES);
        if (!inQuotes && lastChar.isSingleQuote())
        {
            itsString.chop(1);
            changesMade = true;
        }
    }

    return changesMade;
}

bool PQString::cleanUpEnds()
{
    bool changesMade = false;
    PQString firstChar, lastChar;

    firstChar = itsString.left(1);

    while ((firstChar == QString(" ")) || (firstChar == QString("\n")) || (firstChar == QString("\t")) || (firstChar == QString(".")) || (firstChar == QString("¤")))
    {
        itsString.remove(0, 1);
        firstChar.clear();
        firstChar = itsString.left(1);
        changesMade = true;
    }

    lastChar = itsString.right(1);
    while ((lastChar == QString(" ")) || (lastChar == QString("\n")) || (lastChar == QString("\t")) || (lastChar == QString(".")) || (lastChar == QString("¤")))
    {
        itsString.chop(1);
        lastChar.clear();
        lastChar = itsString.right(1);
        changesMade = true;
    }

	return changesMade;
}

bool PQString::isMatch(const PQString &rhs) const
{
    if (itsString.compare(rhs.getString(), Qt::CaseSensitive) == 0)
        return true;
    else
        return false;
}

bool PQString::isMatch(const wchar_t* wString) const
{
    PQString target(QString::fromStdWString(wString));
	return isMatch(target);
}

bool PQString::isAlpha() const
{
	return (getCharType() & ALPHA) == ALPHA;
}

bool PQString::isNumeric() const
{
	return (getCharType() & NUMERICAL) == NUMERICAL;
}

bool PQString::isAlphaNumeric() const
{
	return (getCharType() & ALPHANUMERIC) == ALPHANUMERIC;
}

bool PQString::isCapitalized() const
{
    if (itsString.length() == 0)
		return false;

    PQString firstChar(itsString.at(0));

	return (firstChar.getCharType() & UPPER) == UPPER;
}

bool PQString::isAllCaps(bool mustAllBeAlpha) const
{
	// Only checks true alpha characters and isn't invalidated by other characters

	bool stillOK = true;
    int i = 0;
	unsigned int charType;
    PQString singleChar;

    int itsLength = itsString.length();

	while (stillOK && (i < itsLength))
	{
        singleChar = itsString.at(i);
		charType = singleChar.getCharType();
        if ((charType & ALPHA) == ALPHA)
        {
            if ((charType & LOWER) == LOWER)
                stillOK = false;
            else
                stillOK = true;
        }
        else
            stillOK = !mustAllBeAlpha;

		i++;
	}
	return stillOK;
}

bool PQString::isHyphenated() const
{
    // Returns true if at least one hyphen is contained within the string, except in first or last position
    QString newString = itsString.mid(1, itsString.size() - 2);

    return (newString.contains(QChar(45)) || newString.contains(QChar(8211)) || newString.contains(QChar(126)));
}

bool PQString::isHyphen() const
{
    if (getLength() != 1)
        return false;

    return (itsString.contains(QChar(45)) || itsString.contains(QChar(8211)) || itsString.contains(QChar(126)));
}

bool PQString::isSingleQuote() const
{
    if (getLength() != 1)
        return false;

    bool matched = false;
    QChar qChar = itsString.at(0);

    switch (qChar.unicode())
    {
    // Potentials not included are accent grave and accent aigu  (96 and 180)
    case 39:
    case 146:
    case 8216:
    case 8217:
        matched = true;
        break;

    default:
        break;

    }

    return matched;
}

bool PQString::isDoubleQuote() const
{
    if (getLength() != 1)
        return false;

    bool matched = false;
    QChar qChar = itsString.at(0);

    switch (qChar.unicode())
    {
    // Potentials not included are accent grave and accent aigu  (96 and 180)
    case 34:
    case 147:
    case 148:
    case 8220:
    case 8221:
        matched = true;
        break;

    default:
        break;

    }

    return matched;
}

bool PQString::isQuoteMark() const
{
    if (getLength() != 1)
        return false;

    return isSingleQuote() || isDoubleQuote();
}

bool PQString::isForeignLanguage() const
{
    bool foreign = false;
    int i = 0;
    int length = itsString.length();

    while (!foreign && (i < length))
    {
        foreign = itsString.at(i).unicode() >= 19968;
        i++;
    }

    return foreign;
}

bool PQString::startsWithClick(bool includeVoided) const
{
    QString firstWord;
    QStringList linkWords = QString("click|view|video|watch|http|livestream|stream|book|reserve|blankedoutsentence").split("|");
    if (includeVoided)
        linkWords.append("voidedoutsentence");

    int index = itsString.indexOf(" ");
    if ((index == -1) && (itsString.length() > 0))
        index = itsString.length();
    if (index > 0)
    {
        firstWord = itsString.left(index).toLower();
        if (linkWords.contains(firstWord))
            return true;
        else
            return false;
    }
    else
        return false;
}

bool  PQString::containsVowel() const
{
    bool hasVowel = false;
    int i = 0;
    int j = 0;
    int length = itsString.length();
    QString vowels("aeiouéèô");

    while (!hasVowel && (i < length))
    {
        j = 0;
        while (!hasVowel && (j < 8))
        {
            hasVowel = (itsString.toLower().at(i) == vowels.at(j));
            j++;
        }
        i++;
    }

    return hasVowel;
}


long double PQString::asNumber() const
{
	long double answer = 0;
	bool OK = true;
	bool noMoreNumbers = false;
	unsigned int decimal = 0;
    int i = 0;
	unsigned int denominator = 1;
    int blankCount = 0;
	unsigned int cValue;
	int sign = 1;
    QChar qChar;

    int itsLength = itsString.length();

	while (OK && (i < itsLength))
	{
        qChar = itsString.at(i);
        switch (static_cast<unsigned int>(qChar.unicode()))
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
                cValue = static_cast<unsigned int>(qChar.unicode() - 48);
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

long double PQString::extractFirstNumber() const
{
    PQString excerpt;
    int i = 0;
    QChar qChar;
	bool started = false;
	bool keepGoing = true;

    int itsLength = itsString.length();

    while (keepGoing && (i < itsLength))
	{
        qChar = itsString.at(i);
        switch (qChar.unicode())
		{
		case 45:	// Negative sign
		case 150:	// Dash
			if (started)
				keepGoing = false;
			else
			{
				started = true;
                excerpt += QChar(qChar.unicode());
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
            excerpt += QChar(qChar.unicode());
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

QString PQString::extractFirstWord() const
{
    int index, lowestIndex;
    QString result;

    lowestIndex = itsString.length();

    index = itsString.indexOf(",");
    if ((index >= 0) && (index < lowestIndex))
        lowestIndex = index;

    index = itsString.indexOf("/");
    if ((index >= 0) && (index < lowestIndex))
        lowestIndex = index;

    if (lowestIndex >= 0)
        result = itsString.left(lowestIndex);
    else
        result = itsString;

    return result;
}


void PQString::insertParameter(const PQString param)
{
    QString oldString("%p%");
    itsString.replace(itsString.indexOf(oldString), oldString.size(), param.getString());
}

int PQString::findPosition(const PQString &criteria, int direction, unsigned int skip, unsigned int occurrence) const
{
	// Function returns -1 if match not made, the position of the first character otherwise (not the array element)
	// Function relies on recursion if occurrence > 1

	// direction =  1 => means forwards or left to right
	// direction = -1 => means backwards or right to left

	// Return immediately on parameter errors
    unsigned int criteriaLength = criteria.getLength();
    if ((criteriaLength == 0) || (occurrence == 0) || ((direction != 1) && (direction != -1)))
		return -1;

	// Redefine target and return if no match possible
    PQString target(itsString);
	if (direction == 1)
		target.dropLeft(skip);
	else
		target.dropRight(skip);
    unsigned int targetLength = target.getLength();
    if (criteriaLength > targetLength)
		return -1;

	// Start actual search
    unsigned int numMatched, i, j;
    int firstCharPos;							// must go to -1 in backwards search
    bool initialMatch, matched;
	numMatched = 0;
    matched = false;

	if (direction == 1)	// forward
	{
		i = 0;
        while (!matched && (criteriaLength <= (targetLength - i)))
		{
			j = 0;
			initialMatch = (criteria[j] == target[i + j]);
            while (!matched && initialMatch && (j < criteriaLength))
			{
				j++;
                if (j == criteriaLength)
					matched = true;
				else
					initialMatch = (criteria[j] == target[i + j]);
			}  // end of inner while loop
            if (!matched)
                i++;
		}  // end of outer while loop

		if (matched)
		{
			numMatched++;
			if (numMatched == occurrence)
                return static_cast<int>(skip + i);
			else
                return findPosition(criteria, direction, skip + i + 1, occurrence - 1);
		}
		else
			return -1;
	}
	else   // search backwards
	{
        i = targetLength - criteriaLength;
        firstCharPos = static_cast<int>(i);
        while (!matched && (firstCharPos >= 0))
		{
			j = 0;
			initialMatch = (criteria[j] == target[i + j]);
            while (!matched && initialMatch && (j < criteriaLength))
			{
				j++;
                if (j == criteriaLength)
					matched = true;
				else
					initialMatch = (criteria[j] == target[i + j]);
			}  // end of inner while loop
            if (!matched)
            {
                i--;
                firstCharPos--;
            }
		}  // end of outer while loop

		if (matched)
		{
			numMatched++;
			if (numMatched == occurrence)
                return static_cast<int>(i);
			else
                return target.findPosition(criteria, direction, targetLength - i, occurrence - 1);
		}
		else
			return -1;
	}
}

int PQString::findPosition(const wchar_t* wString, int direction, unsigned int skip, unsigned int occurrence) const
{
    return findPosition(PQString(QString::fromStdWString(wString)), direction, skip, occurrence);
}

int PQString::firstDifference(const PQString &pString) const
{
    int result = -1;
    int max = static_cast<int>(pString.getLength());
    bool differenceFound = false;

    int i = 0;
    while (!differenceFound && (i < itsString.length()))
    {
        if ((i >= max) || (itsString[i] != pString[static_cast<unsigned int>(i)]))
        {
            differenceFound = true;
            result = i;
        }
        i++;
    }

    return result;
}


unsigned int PQString::getCharType(unsigned int elemNum, unsigned int numWordsRead) const
{
	// Assess types of characters included in string
	// Uses recursion if string length > 1
    unsigned int itsLength = static_cast<unsigned int>(itsString.length());
	if (elemNum >= itsLength)
		return UNDEFINED;

    QChar qChar;
    unsigned int numANSI;
    unsigned int finalType = UNDEFINED;

    qChar = itsString.at(static_cast<int>(elemNum));
    numANSI = qChar.unicode();

    // Address Conditional CRLF
    if (numANSI == 65533)
        return CCRLF;

    // Address differences between ANSI and UNICODE below 256
    if(numANSI >= 256)
    {
        PQString newSingle(qChar);
        numANSI = newSingle.convertCharToANSI();
    }

    unsigned int currentCharType = ANSI[numANSI].charType_defns;  // out of range returns 191, which is type OTHER

	if (elemNum == (itsLength - 1))
	{
		return currentCharType;
	}

    unsigned int remainingCharType = this->getCharType(elemNum + 1, numWordsRead);

    if ((currentCharType & (ALPHA | HYPHENATED | PARENTHESES | QUOTES)) && (remainingCharType & (ALPHA | HYPHENATED | PARENTHESES | QUOTES)))
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

unsigned int PQString::countWords(unsigned int delim, unsigned int WORDTYPE) const
{
	unsigned int count = 0;
    int itsLength = itsString.length();
	if (itsLength == 0)
		return 0;

	bool wordStarted = false;
    PQString singleChar, delimiter;

	switch (delim)
	{
	case COMMA:
        delimiter = QChar(',');
		break;

	case HYPHENATED:
        delimiter = QChar('-');
		break;

	case PERIOD:
        delimiter = QChar('.');
		break;

	case SPACE:
	default:
        delimiter = QChar(' ');
		break;

	}

    for (int i = 0; i < itsLength; i++)
	{
        singleChar = itsString.at(i);
        if (((singleChar.getCharType() & WORDTYPE) == WORDTYPE) && (singleChar != delimiter))
			wordStarted = true;

		if (wordStarted && ((singleChar == delimiter) || (i == (itsLength - 1))))
		{
			count++;
			wordStarted = false;
		}
	}
	return count;

}

unsigned int PQString::drawOutNumber() const
{
	unsigned int result = 0;
    PQString singleChar;

    int itsLength = itsString.length();

    for (int i = 0; i < itsLength; i++)
	{
        singleChar = itsString.at(i);
		if ((singleChar.getCharType() & NUMERICAL) == NUMERICAL)
            result = result * 10 + static_cast<unsigned int>(singleChar.asNumber());
	}

	return result;
}

//QString PQString::diacriticLetters = QString::fromLatin1("ŠŒŽšœžŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýÿ") + QChar(263) + QChar(265) + QChar(269) + QChar(0x0160);
QString PQString::diacriticLetters = QString("ŠŒŽšœžŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýÿ") + QChar(263) + QChar(265) + QChar(269) + QChar(0x0160);

QList<QString> PQString::noDiacriticLetters = QList<QString>() <<"S"<<"OE"<<"Z"<<"s"<<"oe"<<"z"<<"Y"<<"Y"<<"u"<<"A"<<"A"<<"A"<<"A"<<"A"<<"A"<<"AE"<<"C"<<"E"
                                                               <<"E"<<"E"<<"E"<<"I"<<"I"<<"I"<<"I"<<"D"<<"N"<<"O"<<"O"<<"O"<<"O"<<"O"<<"O"<<"U"<<"U"<<"U"<<"U"
                                                               <<"Y"<<"s"<<"a"<<"a"<<"a"<<"a"<<"a"<<"a"<<"ae"<<"c"<<"e"<<"e"<<"e"<<"e"<<"i"<<"i"<<"i"<<"i"<<"o"
                                                               <<"n"<<"o"<<"o"<<"o"<<"o"<<"o"<<"o"<<"u"<<"u"<<"u"<<"u"<<"y"<<"y"<<"c"<<"c"<<"c"<<"s";

QList<QString> PQString::MONTHS = QString("MonthNotSpecified|January|February|March|April|May|June|July|August|September|October|November|December").split("|");
