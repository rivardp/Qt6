// dataRecord.cpp

#include "dataRecord.h"
#include "../Include/PMySQL/databaseSearches.h"

dataRecord::dataRecord() :  permanentErrorFlag(false), gender(genderUnknown), YOB(0), YOD(0), deemedYOD(0),
    ageAtDeath(0), DOBfullyCredible(false), DODfullyCredible(false), YOBfullyCredible(false),
    YODfullyCredible(false), ageAtDeathFullyCredible(false), datesLocked(false),
    language(language_unknown), cycle(0), numUnmatched(0), priorUnmatched(0),
    previouslyLoaded(0), neeEtAlEncountered(false), ageNextReference(false), maleHyphenated(false),
    singleYear(0)
{
    wi.clear();
}

dataRecord::~dataRecord()
{
}

void dataRecord::setFamilyNames(QList<OQString> &nameList)
{
    PQString lastName;

    while (!nameList.isEmpty())
    {
        lastName = nameList.takeFirst();
        setFamilyName(lastName);
    }
}

void dataRecord::setFamilyName(const PQString &name)
{
	// Need to strip parentheses if they are present
    PQString formattedName(name);
	formattedName.removeBookEnds();
    formattedName.removeLeading("(");
    formattedName.removeEnding(")");
    if (OQString(formattedName).isNeeEtAl())
        return;

    // Fix issue with French obits where unstructured starts "nee le 11 octobre"
    if ((formattedName.left(2) == PQString("le")) && formattedName.isAlphaNumeric())
        return;

    formattedName = formattedName.proper().left(25);

	NAMEINFO nameInfo;

    if (familyName.getLength() > 0)
	{
        if (familyName.getUnaccentedString() != formattedName.getUnaccentedString())
		{
            nameInfo.name = formattedName;
			nameInfo.type = ntLast;
            setAlternates(nameInfo);
		}
	}
	else
	{
        familyName = formattedName;
	}
}

void dataRecord::setFirstName(const PQString &name, const unsigned int position)
{
    // With default position = 0, checking occurs
    // For explicit calls, fields are overwritten

    switch (position)
    {
    case 0:
        if ((firstName.getLength() > 0) && (firstName != name.proper().left(15)))
        {
            PQString errMsg;
            errMsg << "Encountered inconsistent First Names in reading file: ";
            errMsg << getURL();
            globals.logMsg(ErrorRecord, errMsg);
        }
        else
            firstName = name.proper().left(15);

        break;

    case 1:
        firstName = name.proper().left(15);
        break;

    case 2:
        firstNameAKA1 = name.proper().left(15);
        break;

    case 3:
        firstNameAKA2 = name.proper().left(15);
        break;

    default:
        // Do nothing
        break;
    }
}

void dataRecord::setFirstNames(const PQString &name, const bool forceLoad)
{
    /*databaseSearches dbSearch;

    if (!forceLoad && dbSearch.pureNickNameLookup(name.getString(), &globals))
            return;*/
    Q_UNUSED(forceLoad);

    if ((firstName.getLength() == 0) || (firstName.getUnaccentedString() == name.proper().getUnaccentedString()))
        firstName = name.proper().left(15);
    else
    {
        if ((firstNameAKA1.getLength() == 0) || (firstNameAKA1.getUnaccentedString() == name.proper().getUnaccentedString()))
            firstNameAKA1 = name.proper().left(15);
        else
        {
            if ((firstNameAKA2.getLength() == 0) || (firstNameAKA2.getUnaccentedString() == name.proper().getUnaccentedString()))
                firstNameAKA2 = name.proper().left(15);
            else
                attemptToMakeRoomFor(name.left(15));
        }
    }
}

void dataRecord::attemptToMakeRoomFor(const PQString &name)
{
    databaseSearches dbSearch;

    if (dbSearch.pureNickNameLookup(name.getString(), &globals))
            return;

    if (firstNameAKA2.getLength() == 0)
        setFirstNames(name);
    else
    {
        if (isAMiddleName(firstName))
        {
            firstName = firstNameAKA1;
            firstNameAKA1 = firstNameAKA2;
            firstNameAKA2 = name.getString();
        }
        else
        {
            if (isAMiddleName(firstNameAKA1))
            {
                firstNameAKA1 = firstNameAKA2;
                firstNameAKA2 = name.getString();
            }
            else
            {
                if (isAMiddleName(firstNameAKA2))
                    firstNameAKA2 = name.getString();
            }
        }
    }
}

void dataRecord::setMiddleNames(const PQString &name)
{
    if (name.getLength() == 0)
        return;

    bool stillValid;
    int i, numWords, position;
    OQStream existingWords;
    OQStream newWords;
    PQString existingWord, newWord, modifiedName, errMsg;
    PQString wordA, wordB;
    PQString space(" ");

    modifiedName = name.left(45);
    simplifyInitials(modifiedName);

    if (OQString(modifiedName).isFormalVersionOf(getFirstName().getString(), errMsg))
        setFirstNames(modifiedName);
    else
    {
        numWords = static_cast<int>(modifiedName.countWords());
        if ((middleNames.getLength() == 0) && (numWords == 1))
        {
            // Simple case of nothing to match against
            middleNames = modifiedName;
        }
        else
        {
            // Create a list of middlenames to determine which ones to keep
            QList<PQString> listOfNames;
            existingWords = middleNames;
            while (!existingWords.isEOS())
            {
                existingWord = existingWords.getWord(true);
                listOfNames.append(existingWord);
            }
            newWords = modifiedName;
            while (!newWords.isEOS())
            {
                newWord = newWords.getWord(true);
                listOfNames.append(newWord);
            }

            // Iterate through list to determine which ones to keep
            numWords = listOfNames.size();
            position = 0;

            while (position < numWords)
            {
                i = 0;
                stillValid = true;
                wordA = listOfNames.at(position).lower();

                while ((stillValid) && (i < numWords))
                {
                    if (i != position)
                    {
                        wordB = listOfNames.at(i).lower();
                        if ((wordA == wordB) || ((wordA.getLength() == 1) && (wordA == wordB.left(1))))
                        {
                            stillValid = false;
                            listOfNames.removeAt(position);
                            numWords--;
                        }
                    }
                    i++;
                }
                if (stillValid)
                    position++;
            }

            // Save sanitized list of names
            middleNames.clear();
            for (i = 0; i < numWords; i++)
            {
                if ((middleNames.getLength() + listOfNames.at(i).getLength()) <= 45)
                {
                    middleNames += listOfNames.at(i);
                    middleNames += space;
                }
            }

            middleNames.cleanUpEnds();
        }
    }
}

void dataRecord::setMiddleNameUsedAsFirstName(const PQString &name)
{
    middleNameUsedAsFirstName = name;
}

void dataRecord::setParentsLastName(const PQString &name)
{
    parentsLastName = name;
}

void dataRecord::setMaidenNames(const PQString &names)
{
    if (names.getLength() == 0)
        return;

    OQStream tempStream(names);
    OQString singleName, singleWord;
    OQString space(" ");

    if (tempStream.countWords() == 1)
        addMaidenName(names);
    else
    {
        while(!tempStream.isEOS())
        {
            singleName.clear();
            singleWord = tempStream.getWord(false, DIVIDER);
            singleWord.removeEnding(PUNCTUATION);
            while ((singleWord.isCompoundName() || singleWord.isAboriginalName(tempStream.peekAtWord())) && !tempStream.isEOS())
            {
                singleName += singleWord;
                if (!tempStream.isEOS())
                {
                    singleName += space;
                    singleWord = tempStream.getWord(false, DIVIDER);
                }
            }
            singleName += singleWord;
            addMaidenName(singleName);
        }
    }

    // Update family names
    QString name;
    PQString drName;
    for (int i = 0; i < maidenNames.size(); i++)
    {
        name = maidenNames.at(i).getString();
        drName.clear();
        drName = name;
        setFamilyName(drName);
    }

    // Clear parentsLastName if no match exists
    PQString pln = getParentsLastName();
    bool matched = false;
    if (pln.getLength() > 0)
    {
        for (int i = 0; i < maidenNames.size(); i++)
        {
            if (pln == maidenNames.at(i))
                matched = true;
        }

        if (!matched)
        {
            removeFromLastNames(pln);
            setParentsLastName("");
        }
    }
}

void dataRecord::addMaidenName(const PQString &mn)
{
    bool alreadyThere;
    QString names = mn.getString();
    QList<QString> nameList = names.split(QString("-"), Qt::SkipEmptyParts, Qt::CaseSensitive);

    for (int i = 0; i < nameList.size(); i++)
    {
        OQString cleanName(nameList.at(i));
        alreadyThere = false;

        for (int i = 0; i < maidenNames.size(); i++)
        {
            if (cleanName.getString() == maidenNames.at(i).getString())
                alreadyThere = true;
        }

        if (!alreadyThere)
        {
            maidenNames.append(cleanName);
            if (cleanName.isFoundIn(OQString::problematicFemaleMiddleNames, 1))
                globals.globalDr->wi.nameWarningException = true;
        }
    }
}

void dataRecord::setPrefix(const PQString &pfx)
{
	if ((prefix.getLength() > 0) && (prefix != pfx))
	{
        PQString errMsg;
        errMsg << "Encountered inconsistent Prefixes in reading file: ";
        errMsg << getURL();
        globals.logMsg(ErrorRecord, errMsg);
    }
	else
	{
        prefix = pfx.left(15);
	}
}

void dataRecord::setSuffix(const PQString &sfx)
{
	if ((suffix.getLength() > 0) && (suffix != sfx))
	{
        PQString errMsg;
        errMsg << "Encountered inconsistent Suffixes in reading file: ";
        errMsg << getURL();
        globals.logMsg(ErrorRecord, errMsg);
    }
	else
	{
        suffix = sfx.left(10);
	}
}

void dataRecord::setGender(const GENDER sex, const bool forceOverride)
{
    if ((sex == genderUnknown) && !forceOverride)
        return;

    if ((gender > 0) && (gender != sex))
	{
        if (forceOverride)
        {
            gender = sex;
            workingGender = sex;
        }

        PQString errMsg;
        errMsg << "Encountered inconsistent Genders in reading file: ";
        errMsg << getURL();
        globals.logMsg(ErrorRecord, errMsg);
        globals.globalDr->wi.genderFlag = 3;
    }
	else
    {
		gender = sex;
        workingGender = sex;
    }
}

void dataRecord::setWorkingGender(const GENDER sex)
{
    workingGender = sex;
}

void dataRecord::clearDOB()
{
    QDate newDOB;
    DOB = newDOB;
    DOBfullyCredible = false;
}

void dataRecord::clearDOD()
{
    QDate newDOD;
    DOD = newDOD;
    DODfullyCredible = false;
}

void dataRecord::setDOB(const QDate &dob, const bool forceOverride, const bool fullyCredible)
{
    bool error, dateError, rangeErrorHigh, rangeErrorLow;
    PQString errMsg;

    if (datesLocked)
        return;

    if (dob.isValid() && (dob <= globals.today) && (dob > QDate(1900,1,1)))
	{
		// Match against existing data if it exists
        dateError = DOB.isValid() && (DOB != dob);
        rangeErrorHigh = !DOB.isValid() && maxDOB.isValid() && (dob > maxDOB);
        rangeErrorLow = !DOB.isValid() && minDOB.isValid() && (dob < minDOB);
        error = dateError || rangeErrorHigh || rangeErrorLow;

        if (!error || forceOverride)
		{
			DOB = dob;
			minDOB = dob;
			maxDOB = dob;
            YOB = static_cast<unsigned int>(dob.year());
            DOBfullyCredible = fullyCredible;
            YOBfullyCredible = fullyCredible;

            if (DOD.isValid())
                ageAtDeath = static_cast<unsigned int>(elapse(DOB, DOD));
		}
        else
        {
            if (rangeErrorHigh && ((dob.toJulianDay() - maxDOB.toJulianDay()) <= 366))
            {
                // Implement a reasonable fix if possible based on assumed unstructured writing error, otherwise widen range
                if (ageNextReference && ((YOB == 0) || ((YOB > 0) && maxDOB.year() == dob.year())))
                {
                    DOB = dob;
                    minDOB = dob;
                    maxDOB = dob;
                    YOB = static_cast<unsigned int>(dob.year());
                    DOBfullyCredible = fullyCredible;
                    YOBfullyCredible = fullyCredible;

                    if (DOD.isValid())
                        ageAtDeath = static_cast<unsigned int>(elapse(DOB, DOD));
                }
                else
                {
                    maxDOB = dob;
                    if (maxDOB.year() != minDOB.year())
                        YOB = 0;
                    else
                        YOB = maxDOB.year();
                    ageAtDeath = 0;
                    wi.dateFlag = 6;
                }
            }

            if (rangeErrorLow && ((minDOB.toJulianDay() - dob.toJulianDay()) <= 366))
            {
                // Implement a reasonable fix if possible based on assumed unstructured writing error, otherwise widen range
                if (ageNextReference && ((YOB == 0) || ((YOB > 0) && minDOB.year() == dob.year())))
                {
                    DOB = dob;
                    minDOB = dob;
                    maxDOB = dob;
                    YOB = static_cast<unsigned int>(dob.year());
                    DOBfullyCredible = fullyCredible;
                    YOBfullyCredible = fullyCredible;

                    if (DOD.isValid())
                        ageAtDeath = static_cast<unsigned int>(elapse(DOB, DOD));
                }
                else
                {
                    minDOB = dob;
                    if (minDOB.year() != maxDOB.year())
                        YOB = 0;
                    else
                        YOB = minDOB.year();
                    ageAtDeath = 0;
                    wi.dateFlag = 6;
                }
            }

            if (dateError)
            {
                if ((DOB == DOD) && (dob < DOD))
                {
                    // Assume DOB was an error
                    DOB = dob;
                    DOB = dob;
                    minDOB = DOB;
                    maxDOB = DOB;
                    YOB = static_cast<unsigned int>(DOB.year());
                    if (DOD.isValid())
                        ageAtDeath = static_cast<unsigned int>(elapse(DOB, DOD));
                }
                else
                {
                    // Issue error message, but still modify record
                    errMsg << "Attempt to set mismatched DOB in reading file: ";

                    // Create range in most cases - replace DOB in one instance
                    if (dob < DOB)
                    {
                        if ((DOB.year() == static_cast<int>(YOD)) && (dob == QDate(dob.year(), DOB.month(), DOB.day())))
                        {
                            DOB = dob;
                            minDOB = DOB;
                            maxDOB = DOB;
                            YOB = static_cast<unsigned int>(DOB.year());
                            if (DOD.isValid())
                                ageAtDeath = static_cast<unsigned int>(elapse(DOB, DOD));
                        }
                        else
                        {
                            minDOB = dob;
                            // if difference is < 20 years, assume dob is correct, otherwise hedge bets
                            double diff = elapse(dob, DOB);
                            if (diff > 20)
                            {
                                maxDOB = dob;
                                DOB = dob;
                                YOB = dob.year();
                                if (DOD.isValid())
                                    ageAtDeath = static_cast<unsigned int>(elapse(dob, DOD));
                            }
                            else
                            {
                                maxDOB = DOB;
                                DOB = QDate();
                                if (minDOB.year() == maxDOB.year())
                                {
                                    YOB = static_cast<unsigned int>(dob.year());
                                    if (DOD.isValid() && (((minDOB.month() < DOD.month()) && (maxDOB.month() < DOD.month())) || ((minDOB.month() > DOD.month()) && (maxDOB.month() > DOD.month()))))
                                        ageAtDeath = static_cast<unsigned int>(elapse(dob, DOD));
                                }
                                else
                                {
                                    YOB = 0;
                                    ageAtDeath = 0;
                                }
                            }

                            DOBfullyCredible = false;
                            YOBfullyCredible = false;
                        }
                    }
                    else
                    {
                        if ((dob >= DOD) || ((dob.year() == static_cast<int>(YOD)) && (dob == QDate(YOD, DOB.month(), DOB.day()))))
                        {
                            // leave everything as is
                        }
                        else
                        {
                            minDOB = DOB;
                            maxDOB = dob;
                            DOB = QDate();
                            if (minDOB.year() == maxDOB.year())
                            {
                                YOB = static_cast<unsigned int>(dob.year());
                                if (DOD.isValid() && (((minDOB.month() < DOD.month()) && (maxDOB.month() < DOD.month())) || ((minDOB.month() > DOD.month()) && (maxDOB.month() > DOD.month()))))
                                    ageAtDeath = static_cast<unsigned int>(elapse(dob, DOD));
                            }
                            else
                            {
                                YOB = 0;
                                ageAtDeath = 0;
                            }
                            DOBfullyCredible = false;
                            YOBfullyCredible = false;
                        }
                    }
                }
            }
        }
	}    
    else
    {
        errMsg << "Attempt to set invalid DOB in reading file: ";
    }

    if (errMsg.getLength() > 0)
    {
        errMsg << getURL();
        globals.logMsg(ErrorRecord, errMsg);
        globals.globalDr->wi.dateFlag = 2;
        globals.globalDr->setAgeAtDeath(0, true, true);
    }
}

void dataRecord::setMinDOB(const QDate &dob)
{
    if (datesLocked)
        return;

    if (dob.isValid() && !DOB.isValid())
	{
		minDOB = dob;
	}
}

void dataRecord::setMaxDOB(const QDate &dob)
{
    if (datesLocked)
        return;

    if (dob.isValid() && !DOB.isValid())
	{
		maxDOB = dob;
	}
}

void dataRecord::setMinMaxDOB()
{
    if (datesLocked)
        return;

    if (DOB.isValid())
    {
        minDOB = DOB;
        maxDOB = DOB;
        return;
    }
    else
    {
        // Set worst case boundries to be tightened where possible
        if (!minDOB.isValid())
            minDOB = QDate(1875, 1, 1);
        if (maxDOB.isValid() && DOD.isValid())
        {
            if (DOD < maxDOB)
                maxDOB = DOD;
        }
        else
        {
            if (DOD.isValid() && (DOD < globals.today))
                maxDOB = DOD;
            else
            {
                if((YOD > 0) && (YOD < static_cast<unsigned int>(globals.today.year())))
                    maxDOB = QDate(static_cast<int>(YOD), 12, 31);
                else
                {
                    maxDOB = globals.today;

                    if (globals.globalDr->getPublishDate().isValid() && (globals.globalDr->getPublishDate() < maxDOB))
                    {
                        maxDOB = globals.globalDr->getPublishDate();
                        if ((maxDOB.month() >= 2) || (maxDOB.day() > 7))
                            YOD = maxDOB.year();
                        else
                            YOD = maxDOB.year() - 1;
                    }

                    if (globals.globalDr->getCommentDate().isValid() && (globals.globalDr->getCommentDate() < maxDOB))
                    {
                        if ((maxDOB.month() >= 2) || (maxDOB.day() > 7))
                            YOD = maxDOB.year();
                        else
                            YOD = maxDOB.year() - 1;
                        maxDOB = globals.globalDr->getCommentDate();
                    }

                    if (globals.globalDr->getDOS().isValid() && (globals.globalDr->getDOS() < maxDOB))
                    {
                        maxDOB = globals.globalDr->getDOS();
                        if ((maxDOB.month() >= 2) || (maxDOB.day() > 10))
                            YOD = maxDOB.year();
                        else
                            YOD = maxDOB.year() - 1;
                    }
                }
            }
        }
    }

    QDate tempDate1;
    QDate tempDate2;

    if ((ageAtDeath == 0) && DOD.isValid())
    {
        if (YOB != 0)
        {
            minDOB = QDate(static_cast<int>(YOB), 1, 1);
            tempDate2 = QDate(static_cast<int>(YOB), 12, 31);
            if ((tempDate2 < globals.today) && (tempDate2 < maxDOB))
                maxDOB = tempDate2;
        }
        return;
    }

    if (!DOD.isValid())
    {
        if (YOB != 0)
        {
            minDOB = QDate(static_cast<int>(YOB), 1, 1);
            tempDate2 = QDate(static_cast<int>(YOB), 12, 31);
            if ((tempDate2 < globals.today) && (tempDate2 < maxDOB))
                maxDOB = tempDate2;
        }
        else
        {
            if ((YOD > 0) && (ageAtDeath > 0))
            {
                minDOB = QDate(static_cast<int>(YOD - ageAtDeath - 1), 1, 2);
                tempDate2 = QDate(static_cast<int>(YOD - ageAtDeath), 12, 31);
                if ((tempDate2 < globals.today) && (tempDate2 < maxDOB))
                    maxDOB = tempDate2;
            }
        }
        return;
    }

    // Latest possible day
    // DOD is valid to get here
    bool leapDay = ((DOD.month() == 2) && (DOD.day() == 29));
    int birthYear = DOD.year() - static_cast<int>(ageAtDeath);
    if (leapDay)
        tempDate1 = QDate(birthYear, 3, 1);
    else
    {
        tempDate1 = QDate(birthYear, DOD.month(), DOD.day());
        //tempDate1 = tempDate1.addDays(1);
    }
    // If the YOB is known, we can narrow the range
    if (YOB != 0)
        tempDate2 = QDate(static_cast<int>(YOB), 12, 31);
    if (YOB && (tempDate2 < tempDate1) && (elapse(tempDate2, tempDate1) < 1.0))
        maxDOB = tempDate2;
    else
        maxDOB = tempDate1;

    // Earliest possible day
    if (ageNextReference && !ageAtDeathFullyCredible && ((YOB == 0) || (YOB < static_cast<unsigned int>(birthYear))))  // Adjust for incorrect language understating age
        birthYear--;
    if (leapDay)
        tempDate1 = QDate(birthYear - 1, 3, 1);
    else
    {
        tempDate1 = QDate(birthYear - 1, DOD.month(), DOD.day());
        tempDate1 = tempDate1.addDays(1);
    }
    // If the YOB is known, we can narrow the range
    if (YOB != 0)
        tempDate2 = QDate(static_cast<int>(YOB), 1, 1);
    if (YOB && (tempDate2 > tempDate1) && (elapse(tempDate1, tempDate2) < 1.0))
        minDOB = tempDate2;
    else
        minDOB = tempDate1;

    // Where ageAtDeath wasn't fully credible and range > 1, reset to 0
    if (ageNextReference && (ageAtDeath > 0) && DOD.isValid())
    {
        int minAge, maxAge;
        minAge = static_cast<int>(elapse(maxDOB, DOD));
        maxAge = static_cast<int>(elapse(minDOB, DOD));
        if (minAge != maxAge)
            ageAtDeath = 0;
    }

    if ((minDOB != maxDOB) && sourceID.publishDate.isValid() && (sourceID.publishDate < maxDOB))
        maxDOB = sourceID.publishDate;
}

void dataRecord::setDOD(const QDate &dod, const bool forceOverride, const bool fullyCredible)
{
	bool error = false;
    bool overrideOverride = false;
    bool memorialObit = false;
    bool priorError = false;
    PQString errMsg;

	if (dod.isValid())
	{
		// Match against existing data if it exists
        error = DOD.isValid() && ((DOD != dod) || (dod > globals.today));
        if (error && (DOD != dod))
        {
            memorialObit = (DOD.year() == globals.today.year()) && (dod.year() < globals.today.year()) && (dod.year() >= (globals.today.year() - 25));
            if ((globals.today.toJulianDay() - DOD.toJulianDay()) < 350)
                memorialObit = false;
            priorError = (DOD.year() < (globals.today.year() - 25)) && (dod < globals.today);

            if (dod > globals.today)
                overrideOverride = true;
        }

        if (!error || (forceOverride && !overrideOverride & memorialObit) || priorError)
        {
			DOD = dod;
            YOD = static_cast<unsigned int>(dod.year());
            DODfullyCredible = fullyCredible;
            YODfullyCredible = fullyCredible;
            setMinMaxDOB();
            if (DOB.isValid())
                setAgeAtDeath(static_cast<int>(elapse(DOB, DOD)));
        }

        if (error || priorError)
            wi.dateFlag = 3;  // Warning only as outcome should be correct in most cases
	}
    else
    {
        errMsg << "Attempt to set invalid DOD in reading file: ";
        wi.dateFlag = 3;
    }


    if (errMsg.getLength() > 0)
    {
        errMsg << getURL();
        globals.logMsg(ErrorRecord, errMsg);
    }
}

void dataRecord::setDOBandDOD(const DATES &dates, const bool forceOverride)
{
    bool problem = false;

    if (datesLocked){
        return;}

	if (dates.potentialDOB.isValid() && dates.potentialDOD.isValid())
	{
		// Match against existing data if it exists
        bool problemDOB = (DOB.isValid() && (DOB != dates.potentialDOB));
        bool problemDOD = (DOD.isValid() && (DOD != dates.potentialDOD));
        problem =  problemDOB || problemDOD;

        if (!problem || forceOverride)
		{
			DOB = dates.potentialDOB;
			DOD = dates.potentialDOD;
		}

       if (problem)
        {
            PQString errMsg;
            errMsg << "Encountered inconsistent dates in reading file: ";
            errMsg << getURL();
            globals.logMsg(ErrorRecord, errMsg);
            if (problemDOB)
                globals.globalDr->wi.dateFlag = 12;
            else
            {
                if (problemDOD && (dates.potentialDOD < DOD))
                    globals.globalDr->wi.dateFlag = 13;
                else
                    globals.globalDr->wi.dateFlag = 3;
            }
        }
    }
	else
	{
		// Try to match a single date

        // First look at unlikely case of potential DOB and blank potential DOD
		if (dates.potentialDOB.isValid() && (dates.potentialDOB != DOD))
		{
            problem = (DOB.isValid() && (DOB != dates.potentialDOB));
            if (!problem)
				DOB = dates.potentialDOB;
			else
			{
				if (!DOD.isValid() && (dates.potentialDOB > DOB))
					DOD = dates.potentialDOB;
				else
				{
                    PQString errMsg;
                    errMsg << "Encountered inconsistent DOBs in reading file: ";
                    errMsg << getURL();
                    //globals.logMsg(ErrorRecord, errMsg);
                    globals.globalDr->wi.dateFlag = 12;
                    globals.globalDr->setAgeAtDeath(0, true, true);
                }
			}
		}
		
        // Second case is much likelier, where single DOD was in header/title as a new potential DOD is slightly different
        // Key is to separate reporting issues from obit notices for babies
        if (dates.potentialDOD.isValid() && (dates.potentialDOD != DOB))
		{
            problem = (DOD.isValid() && (DOD != dates.potentialDOD));
            if (!problem)
				DOD = dates.potentialDOD;
			else
			{
                if (!DOB.isValid() && (dates.potentialDOD < DOD))
					DOB = dates.potentialDOD;
			}

            if (problem)
            {
                //PQString errMsg;
                //errMsg << "Encountered inconsistent DODs in reading file: ";
                //errMsg << getURL();
                //globals.logMsg(ErrorRecord, errMsg);
                globals.globalDr->wi.dateFlag = 13;
            }
		}
	}

    // Update related fields
    if (DOB.isValid())
	{
        YOB = static_cast<unsigned int>(DOB.year());
		minDOB = DOB;
		maxDOB = DOB;
	}

    if (DOD.isValid())
        YOD = static_cast<unsigned int>(DOD.year());
}

void dataRecord::setTypoDOB(const QDate &dob)
{
    typoDOB = dob;
}

void dataRecord::setTypoDOD(const QDate &dod)
{
    typoDOD = dod;
}

void dataRecord::setMonthYearOfBirth(const unsigned int mob, const unsigned int yob)
{
    bool problem = false;

    if (datesLocked || DOB.isValid()){
        return;}

    if ((YOB > 0) && (yob != YOB))
        problem = true;
    else
    {
        if ((mob < 1) || (mob > 12))
            problem = true;
        else
        {
            if ((yob < 1900) || (yob > static_cast<unsigned int>(globals.today.year())))
                problem = true;
        }
    }

    if (!problem)
    {
        YOB = yob;
        QDate lowDate(yob, mob, 1);
        QDate highDate = lowDate.addMonths(1).addDays(-1);
        if (lowDate > minDOB)
            minDOB = lowDate;
        if (highDate < maxDOB)
            maxDOB = highDate;
    }
}

void dataRecord::setAgeAtDeath(const unsigned int num, const bool fullyCredible, const bool override)
{
    bool consistent;
    unsigned int existingAgeAtDeath = ageAtDeath;

    if (datesLocked)
        return;

    if (ageAtDeathFullyCredible && !override)
        return;

    if (DOB.isValid() && DOD.isValid())
    {
        unsigned int tempAgeAtDeath = static_cast<unsigned int>(elapse(DOB,DOD));

        if (DOBfullyCredible && DODfullyCredible)
        {
            if ((DOB == DOD) && (num > 0))
            {
                // Assume there was an error in structured dates
                if (YOB > (YOD - num))
                {
                    // YOB wrong
                    clearDOB();
                    YOB = 0;
                    ageAtDeath = num;
                    setMinMaxDOB();
                }
                else
                {
                    // YOD wrong
                    clearDOD();
                    YOD = 0;
                }
            }
            else
            {
                if ((num != tempAgeAtDeath) && (num == (tempAgeAtDeath + 20)) && (YOD == (static_cast<uint>(globals.today.year()) - 20)))
                {
                    ageAtDeath = num;
                    DOD = QDate(DOD.year() + 20, DOD.month(), DOD.day());
                    YOD = DOD.year();
                }
                else
                    ageAtDeath = tempAgeAtDeath;

                ageAtDeathFullyCredible = true;
            }
        }
        else
        {
            //if ((ageAtDeath > 0) && (num != ageAtDeath) && (num != tempAgeAtDeath))
            if ((num != ageAtDeath) && (num != tempAgeAtDeath))
            {
                // Attempt to fix occasional read error as part of UpdateDeceased
                if ((DOB.month() == 1) && (DOB.day() == 1) && (num == (tempAgeAtDeath - 1)))
                {
                    DOB = QDate();
                    ageAtDeath = num;
                }
                else
                {
                    if (tempAgeAtDeath == existingAgeAtDeath)
                    {
                        // Likely an interpretation error reading sentences
                        wi.dateFlag = 9;
                    }
                    else
                    {
                        ageAtDeath = tempAgeAtDeath;
                        if (((tempAgeAtDeath - num) == 1) || ((num - tempAgeAtDeath) == 1))
                            wi.dateFlag = 9;
                        else
                            wi.dateFlag = 19;
                    }
                }
            }
            else
            {
                if (num == ageAtDeath)
                    ageAtDeath = num;
                else
                    ageAtDeath = tempAgeAtDeath;
            }

            ageAtDeathFullyCredible = false;
        }
        return;
    }

    if (DOD.isValid() && (YOB > 0))
    {
        QDate tempDate;
        bool leapYearDay = ((DOD.month() == 2) && (DOD.day() == 29));
        unsigned int minAge, maxAge;
        maxAge = YOD - YOB;
        minAge = maxAge - 1;

        consistent = (num >= minAge) && (num <= maxAge);        
        if (consistent)
        {
            ageAtDeath = num;
            ageAtDeathFullyCredible = (ageAtDeath > 0) && (ageAtDeathFullyCredible || (fullyCredible && !ageNextReference));

            if (ageAtDeath == minAge)
            {
                if (leapYearDay)
                    tempDate.setDate(DOD.year() - ageAtDeath, 3, 1);
                else
                    tempDate.setDate(DOD.year() - (ageAtDeath + 1), DOD.month(), DOD.day());

                // Death before birthday
                if (!leapYearDay)
                    tempDate = tempDate.addDays(1);

                if (!minDOB.isValid() || (tempDate > minDOB))
                    minDOB = tempDate;
            }
            else
            {
                if (leapYearDay)
                    tempDate.setDate(DOD.year() - ageAtDeath, 3, 1);
                else
                    tempDate.setDate(DOD.year() - ageAtDeath, DOD.month(), DOD.day());

                // Death before birthday
                if (leapYearDay)
                    tempDate = tempDate.addDays(-1);

                if (!maxDOB.isValid() || (tempDate < maxDOB))
                    maxDOB = tempDate;
            }
        }
        else
        {
            if (ageNextReference && ((num + 1) == minAge))
            {
                ageAtDeath = num + 1;
                ageAtDeathFullyCredible = (ageAtDeath > 0) && (ageAtDeathFullyCredible || fullyCredible);
                ageNextReference = false;
            }
            else
            {
                if (ageAtDeathFullyCredible && fullyCredible)
                {
                    PQString errMsg;
                    errMsg << "Inconsistent age at death encountered for: " << getURL();
                    globals.logMsg(ErrorRecord, errMsg);
                    globals.globalDr->wi.ageFlag = 1;
                }
                else
                {
                    if (fullyCredible)
                    {
                        ageAtDeath = num;
                        if (ageAtDeath > 0)
                            ageAtDeathFullyCredible = true;
                        else
                            ageAtDeath = false;
                    }
                }
            }
        }

        return;
    }

    if (DOD.isValid() && !DOB.isValid() && typoDOB.isValid() && (num > 0))
    {
        QDate tempDate;

        int mthDOD = DOD.month();
        int dayDOD = DOD.day();
        int mthDOB = typoDOB.month();
        int dayDOB = typoDOB.day();

        bool earlierDOB = (mthDOB < mthDOD) || ((mthDOB == mthDOD) && (dayDOB < dayDOD));

        if (earlierDOB)
            tempDate = QDate(YOD - num, mthDOB, dayDOB);
        else
            tempDate = QDate(YOD - num - 1, mthDOB, dayDOB);

        if (tempDate.isValid())
        {
            DOB = tempDate;
            ageAtDeath = num;
            ageAtDeathFullyCredible = false;
        }
    }

    consistent = ((num == ageAtDeath) || (ageAtDeath == 0));
    if (consistent || override)
    {
        ageAtDeath = num;
        ageAtDeathFullyCredible = (ageAtDeath > 0) && (ageAtDeathFullyCredible || fullyCredible);
    }
    else
    {
        if (ageAtDeathFullyCredible && fullyCredible)
        {
            PQString errMsg;
            errMsg << "Inconsistent age at death encountered for: " << getURL();
            globals.logMsg(ErrorRecord, errMsg);
            globals.globalDr->wi.ageFlag = 1;
        }
        else
        {
            if (fullyCredible)
            {
                ageAtDeath = num;
                if (ageAtDeath > 0)
                    ageAtDeathFullyCredible = true;
                else
                    ageAtDeath = false;
            }
        }
    }
}

void dataRecord::setURL(const PQString &url)
{
    if ((sourceID.URL.getLength() > 0) && (sourceID.URL != url))
	{
        PQString errMsg;
        errMsg << "Encountered inconsistent URLs in adding: ";
        errMsg << url.getString();
        errMsg << "  to : ";
        errMsg << getURL();
        globals.logMsg(ErrorRecord, errMsg);
	}
	else
        sourceID.URL = url.left(150);
}

void dataRecord::setLanguage(const LANGUAGE lang)
{
    if (language == language_unknown)
        language = lang;
    else
    {
        if ((language == multiple_unknown) || (language == multiple))
            language = multiple;
        else
        {
            if ((language != lang) && (lang != language_unknown))
                language = multiple;
        }
    }
}

void dataRecord::setAlternates(const NAMEINFO &nameInfo, bool bestOf)
{
    QStringList nonHyphenatedNames;
    PQString name, errMsg;
    PQString space(" ");
    NAMETYPE type;
    databaseSearches dbSearch;

    name = nameInfo.name;
    type = nameInfo.type;

    if ((name.getLength() == 0) || (name == PQString("Retired")) || (name == PQString("Ret'd")) || (name == PQString("Retd")) || (name == PQString("\"\"")) || (name == PQString(",")) || (name == PQString("St.")))
		return;

    LANGUAGE lang = getLanguage();
    GENDER gender = getGender();
    if (gender == genderUnknown){
        gender = getWorkingGender();}

	switch (type)
	{
	case ntLast:
        if (name.isHyphenated())
        {
            OQString checkName = name;
            if (checkName.isHyphenatedName())
            {
                name.removeHyphens();
                name = name.proper();
            }
            else
            {
                // Check for unusual entry such at "St-Jacques"
                // Leverage removeInternalPeriods logic which checks for "saints"
                QString tempString = name.getString();
                tempString.replace(QString("St-"), QString("St."), Qt::CaseSensitive);
                tempString.replace(QString("Saint-"), QString("Saint."), Qt::CaseSensitive);
                tempString.replace(QChar(8211), QChar(46));
                OQString tempNameA = OQString(tempString);
                OQString tempNameB = tempNameA;
                tempNameB.removeInternalPeriods();

                if (tempNameA.isNeeEtAl())
                {
                    name = tempNameB;
                    neeEtAlEncountered = true;
                    if (gender == Male)
                    {
                        gender = Female;
                        wi.genderFlag = 21;
                    }
                }
                else
                {
                    if (tempNameA == tempNameB)
                        name = tempNameA;
                    else
                    {
                        int i;
                        while ((i = tempString.indexOf(QChar(46), 0)) != -1)
                        {
                            if (i > 0)
                            {
                                name = tempString.left(i);
                                name = name.proper();
                                if (!OQString(name).isNeeEtAl())
                                    name.convertToQStringList(nonHyphenatedNames);
                            }
                            tempString.remove(0, i + 1);
                        }
                        name = tempString;
                    }
                }
            }
        }

        name.convertToQStringList(nonHyphenatedNames);

        for (int i = 0; i < nonHyphenatedNames.size(); i++)
        {
            name = nonHyphenatedNames.at(i);

            if ((familyName.getLength() == 0) || (familyName.getUnaccentedString() == name.getUnaccentedString()))
                familyName = name;
            else
            {
                if ((familyNameAlt1.getLength() == 0) || (familyNameAlt1.getUnaccentedString() == name.getUnaccentedString()))
                    familyNameAlt1 = name;
                else
                {
                    if ((familyNameAlt2.getLength() == 0) || (familyNameAlt2.getUnaccentedString() == name.getUnaccentedString()))
                        familyNameAlt2 = name;
                    else
                    {
                        if ((familyNameAlt3.getLength() == 0) || (familyNameAlt3.getUnaccentedString() == name.getUnaccentedString()))
                            familyNameAlt3 = name;
                    }
                }
            }
        }

        if (!bestOf)
        {
            removeFromMiddleNames(name);
            removeFromFirstNames(name);
        }

        if (nameInfo.hadNeeEtAl && !nameInfo.hadFormerly)
        {
            globals.globalDr->setNeeEtAlEncountered(true);
            PQString parentsOrBrotherName = globals.globalDr->getParentsLastName();
            if (parentsOrBrotherName.lower().getUnaccentedString() != name.lower().getUnaccentedString())
                removeFromLastNames(parentsOrBrotherName);
        }

        if ((gender == Male) && (nonHyphenatedNames.size() > 1))
            setMaleHypentated(true);

		break;  // Last name

	case ntFirst:
        if (dbSearch.pureNickNameLookup(name.getString(), &globals) && !bestOf)
        {
            //  Keep "Junior" and "Buck" et al if its the first word
            if ((globals.globalDr->getFirstName().getLength() != 0) || (globals.globalDr->getLastName().getLength() != 0))
                return;
        }

        // Fix case of single initial with a period
        if (name.getLength() == 2)
            name.removeEnding(QString("."));

        if ((firstName.getLength() == 0) || (firstName.getUnaccentedString() == name.getUnaccentedString()) || ((firstName.getLength() == 1) && (firstName == name.left(1))))
            firstName = name;
		else
		{
            if ((firstNameAKA1.getLength() == 0) || (firstNameAKA1.getUnaccentedString() == name.getUnaccentedString()) || ((firstNameAKA1.getLength() == 1) && (firstNameAKA1 == name.left(1))))
			{
                if ((name.getLength() > 1) || ((name.getLength() == 1) && (name != firstName.left(1))))
                    firstNameAKA1 = name;
			}
			else
			{
                if ((firstNameAKA2.getLength() == 0) || (firstNameAKA2.getUnaccentedString() == name.getUnaccentedString()) || ((firstNameAKA2.getLength() == 1) && (firstNameAKA2 == name.left(1))))
                    firstNameAKA2 = name;
                else
                {
                    // Check if the new addition is a single initial matching an existing name
                    if (name.getLength() == 1)
                    {
                        if (name != firstName.left(1))
                        {
                            if (firstNameAKA1.getLength() == 0)
                                firstNameAKA1 = name;
                            else
                            {
                                if ((name != firstNameAKA1.left(1)) && (firstNameAKA2.getLength() == 0))
                                    firstNameAKA2 = name;
                            }
                        }
                    }
                    else
                        attemptToMakeRoomFor(name);
                }
			}
		}

        // If gender is uncertain, update working gender
        if (gender == genderUnknown)
        {
            double unisex = dbSearch.genderLookup(globals.globalDr, &globals);
            if (unisex > 0.75)
                workingGender = Male;
            if (unisex < 0.25)
                workingGender = Female;
        }
		break;  // First name

	case ntPrefix:
        if (prefix.getLength() == 0)
        {
            OQStream tempPrefix = name;
            if(!tempPrefix.isTitle(lang, gender))
                prefix = name;
        }
        else
        {
            OQStream existingNames = prefix;
            OQStream newNames = name;   // Allow for more than a single suffix to be added at a time
            PQString existingName, newName;

            for (unsigned int j = 0; j < name.countWords(); j++)
            {
                newName = newNames.getWord();
                bool matched = false;
                unsigned int i = 0;
                while (!matched && (i < existingNames.countWords()))
                {
                    existingName = existingNames.getWord();
                    matched = (existingName == newName);
                    i++;
                }
                if (!matched)
                {
                    OQStream tempPrefix = name;
                    if(!tempPrefix.isTitle(lang, gender) && ((prefix.getLength() + newName.getLength()) < 15))
                    {
                        prefix += space;
                        prefix += newName;
                    }
                }
            }
        }
        break;  // Prefix

	case ntSuffix:
        if (suffix.getLength() == 0)
        {
            suffix = name;
        }
        else
        {
            OQStream existingNames = suffix;
            OQStream newNames = name;   // Allow for more than a single suffix to be added at a time
            PQString existingName, newName;

            for (unsigned int j = 0; j < name.countWords(); j++)
            {
                newName = newNames.getWord();
                bool matched = false;
                unsigned int i = 0;
                while (!matched && (i < existingNames.countWords()))
                {
                    existingName = existingNames.getWord();
                    matched = (existingName == newName);
                    i++;
                }
                if (!matched && ((suffix.getLength() + newName.getLength() < 10)))
                {
                    suffix += space;
                    suffix += newName;
                }
            }
        }
        break;  // Suffix

	case ntMiddle:
        // Fix case of single initial with a period
        if (name.getLength() == 2)
            name.removeEnding(QString("."));

        if (OQString(name).isFormalVersionOf(getFirstName().getString(), errMsg))
            setFirstNames(name);
        else
        {
            name = name.getString().replace("-", " ", Qt::CaseInsensitive);
            if (middleNames.getLength() == 0)
            {
                // No middle names exist yet
                middleNames = name;
            }
            else
            {
                OQStream existingNames = middleNames;
                OQStream newNames = name;   // Allow for more than a single name to be added at a time
                PQString existingName, newName, removeString;

                for (unsigned int j = 0; j < name.countWords(); j++)
                {
                    newName = newNames.getWord();
                    bool matched = false;
                    unsigned int i = 0;
                    while (!matched && (i < middleNames.countWords()))
                    {
                        existingName = existingNames.getWord();
                        matched = (existingName.getUnaccentedString() == newName.getUnaccentedString());
                        if (!matched && (newName.getLength() == 1))
                            matched = (existingName.left(1) == newName);
                        if (!matched && (existingName.getLength() == 1))
                        {
                            if(existingName == newName.left(1))
                                removeString = space + existingName + space;
                        }
                        i++;
                    }

                    if (!matched)
                    {
                        middleNames += space;
                        middleNames += newName;
                    }

                    if (removeString.getLength() == 3)
                    {
                        int index = middleNames.findPosition(space);
                        if ((index == 1) && (middleNames.left(1) == removeString.middle(1, 1)))
                            middleNames.dropLeft(2);
                        else
                            middleNames.replace(removeString.getString(), QString(""));
                    }
                }
            }
		}
        break;  // Middle

	default:
		// Not sure if this is required - perhaps lastName?
		break;
	}
}

void dataRecord::setAlternates(const QList<NAMEINFO> &nameInfoList, bool bestOf)
{
    QListIterator<NAMEINFO> iter(nameInfoList);
    NAMEINFO ni;

    while (iter.hasNext())
	{
        ni = iter.next();
        setAlternates(ni, bestOf);
	}
}

void dataRecord::setDate(const QDate &day)
{
	lastUpdated = day;
}

void dataRecord::setYOB(const unsigned int yob, const bool forceOverride, const bool fullyCredible)
{
    if (datesLocked)
        return;

    bool errorA = false;
    bool errorB = false;
    unsigned int minYear = 1900;
    unsigned int maxYear = static_cast<unsigned int>(globals.today.year());
    PQString errMsg;
    QDate tempDate, minDate, maxDate;

    if ((yob > minYear) && (yob <= maxYear))
    {
        // Run additional validations under two approaches
        bool ageAtDeathSet = (ageAtDeath > 0) && minDOB.isValid();

        tempDate.setDate(static_cast<int>(yob), 1, 1);
        if (ageAtDeathSet && (minDOB > tempDate))
            minDate = minDOB;
        else
            minDate = tempDate;

        tempDate.setDate(static_cast<int>(yob), 12, 31);
        if (ageAtDeathSet && (maxDOB < tempDate))
            maxDate = maxDOB;
        else
            maxDate = tempDate;


        // Match against existing data if it exists
        errorA = (YOB > 0) && (YOB != yob);
        errorB = ageAtDeathSet && (YOB == 0) && !((static_cast<int>(yob) == minDOB.year()) || (static_cast<int>(yob) == maxDOB.year()));
        if (!errorA && !errorB)
        {
            YOB = yob;
            YOBfullyCredible = fullyCredible;
            if (missingDOB())
            {
                minDOB = minDate;
                maxDOB = maxDate;
            }
        }

        if (errorA || errorB || forceOverride)
        {
            if (forceOverride)
            {
                if (errorA || errorB)
                {
                    if (DOB.isValid())
                    {
                        if (DOB < minDate)
                            minDate = DOB;
                        if (DOB > maxDate)
                            maxDate = DOB;
                        DOB = QDate();
                    }
                    ageAtDeath = 0;

                    errMsg << "Encountered inconsistent YOB in reading file: ";
                    wi.dateFlag = 22;
                }

                YOB = yob;
                minDOB = minDate;
                maxDOB = maxDate;
            }
            else
            {
                if (errorA && (YOB == YOD) && (yob < YOB))
                {
                    // Assume original information is wrong
                    DOB.setDate(0,0,0);
                    YOB = yob;
                    minDOB = minDate;
                    maxDOB = maxDate;
                }
            }
        }
    }
    else
    {
        if (forceOverride)
            YOB = yob;

        if (yob != 0)
        {
            errMsg << "Attempt to set invalid YOB in reading file: ";
            wi.dateFlag = 4;
        }
    }

    if (errMsg.getLength() > 0)
    {
        errMsg << getURL();
        globals.logMsg(ErrorRecord, errMsg);
        globals.globalDr->setAgeAtDeath(0, true, true);
    }
}

void dataRecord::setYOD(const unsigned int yod, const bool forceOverride, const bool fullyCredible)
{
    bool error = false;
    unsigned int minYear = 1900;
    unsigned int maxYear = static_cast<unsigned int>(globals.today.year());
    PQString errMsg;

    if ((yod > minYear) && (yod <= maxYear))
    {
        // Match against existing data if it exists
        error = (YOD > 0) && (YOD != yod);
        if (!error || forceOverride)
        {
            YOD = yod;
            YODfullyCredible = fullyCredible;
        }

        if (error)
            errMsg << "Encountered inconsistent YOD in reading file: ";
    }
    else
    {
        if (yod != 0)
            errMsg << "Attempt to set invalid YOD in reading file: ";
    }

    if (errMsg.getLength() > 0)
    {
        errMsg << getURL();
        globals.logMsg(ErrorRecord, errMsg);
        globals.globalDr->wi.dateFlag = 3;
    }
}

void dataRecord::setTitle(const PQString &Title)
{
    title = Title;
}

void dataRecord::setTitleKey(const PQString &TitleKey)
{
    titleKey = TitleKey;
}

void dataRecord::setFirstLoaded(const QDate & date)
{
    firstRecorded = date;
}

void dataRecord::setDOS(const QDate &date)
{
    DOS = date;
}

void dataRecord::setCommentDate(const QDate &date)
{
    commentDate = date;
}

void dataRecord::setLastUpdated(const QDate &date)
{
    lastUpdated = date;
}

void dataRecord::setNeeEtAlEncountered(const bool flag)
{
    neeEtAlEncountered = flag;
}

void dataRecord::setAgeNextReference(const bool flag)
{
    ageNextReference = flag;
}

void dataRecord::setMaleHypentated(const bool flag)
{
    maleHyphenated = flag;
}

void dataRecord::setSingleYear(const unsigned int year)
{
    singleYear = year;
}

/*void dataRecord::setPotentialFirstName(const QString name)
{
    potentialGivenName = name;
}*/

void dataRecord::setUsedFirstNameFromStructured(const QString name)
{
    usedFirstNameFromStructured = name;
}

void dataRecord::setUsedFirstNameFromUnstructured(const QString name)
{
    usedFirstNameFromUnstructured = name;
}

void dataRecord::setFirstGivenNameUsedInSentence(const QString name)
{
    firstGivenNameUsedInSentence = name;
}

void dataRecord::setSpouseName(const QString name)
{    
    spouseName = name;
    spouseName.replace(",", "");
}

void dataRecord::storeContent(PQString *content, unsigned int fieldType)
{
    PQString tempName;
    QDate tempDate;
    unsigned int fullNumber, y, m, d;

	switch (fieldType)
	{
	case tlFamilyName:
        tempName = content->getString();
		setFamilyName(tempName);
		break;

	case tlFirstName:
        tempName = content->getString();
		setFirstName(tempName);
		break;

	case tlMiddleName:
        tempName = content->getString();
		setMiddleNames(tempName);
		break;
	
	case tlGender:
		break;
		
    case tlDOB:     // Assumes yyyymmdd format
        fullNumber = static_cast<unsigned int>(content->asNumber());
        y = static_cast<unsigned int>(fullNumber / 10000);
        m = static_cast<unsigned int>((fullNumber - y*10000) / 100);
        d = static_cast<unsigned int>((fullNumber - y*10000 - m*100));
        tempDate = QDate(static_cast<int>(y), static_cast<int>(m), static_cast<int>(d));
		if (tempDate.isValid())
			setDOB(tempDate);
		break;

    case tlDOD:     // Assumes yyyymmdd format
        fullNumber = static_cast<unsigned int>(content->asNumber());
        y = static_cast<unsigned int>(fullNumber / 10000);
        m = static_cast<unsigned int>((fullNumber - y*10000) / 100);
        d = static_cast<unsigned int>((fullNumber - y*10000 - m*100));
        tempDate = QDate(static_cast<int>(y), static_cast<int>(m), static_cast<int>(d));
        if (tempDate.isValid())
			setDOD(tempDate);
		break;
		
	case tlURL:
		setURL(*content);
		break;

	default:
		break;
	}
}

void dataRecord::setDeceasedNumber(const unsigned int dn)
{
    sourceID.deceasedNumber = dn;
}

void dataRecord::setPublishDate(const QDate &date)
{
    sourceID.publishDate = date;
}

QString dataRecord::getURL() const
{
    return sourceID.URL.getString();
}

GENDER dataRecord::getGender() const
{
	return gender;
}

GENDER dataRecord::getWorkingGender() const
{
    return workingGender;
}

PROVINCE dataRecord::getProv() const
{
    return postalCodeInfo.getPROVINCE();
}

LANGUAGE dataRecord::getLanguage() const
{
	return language;
}

PQString dataRecord::getFirstName() const
{
    return firstName;
}

PQString dataRecord::getFirstNameAKA1() const
{
    return firstNameAKA1;
}

PQString dataRecord::getFirstNameAKA2() const
{
    return firstNameAKA2;
}

PQString dataRecord::getMiddleNames() const
{
    return middleNames;
}

bool dataRecord::getMiddleNameList(QList<QString> &mnl) const
{
    PQString name;
    OQStream nameStream(getMiddleNames());

    while (!nameStream.isEOS())
    {
        name = nameStream.getWord();
        if (name.getLength() > 0)
            mnl.append(name.getString());
    }

    if (mnl.size() > 0)
        return true;
    else
        return false;
}

PQString dataRecord::getMiddleNameUsedAsFirstName() const
{
    return middleNameUsedAsFirstName;
}

PQString dataRecord::getParentsLastName() const
{
    return parentsLastName;
}

QList<PQString> dataRecord::getMaidenNames() const
{
    return maidenNames;
}

QList<QString> dataRecord::getFirstNameList() const
{
    QList<QString> resultList;
    QString name;

    name = getFirstName().getString();
    if (name.size() > 0)
    {
        resultList.append(name);
        name = getFirstNameAKA1().getString();
        if (name.size() > 0)
        {
            resultList.append(name);
            name = getFirstNameAKA2().getString();
            if (name.size() > 0)
                resultList.append(name);
        }
    }

    return resultList;
}

QList<QString> dataRecord::getGivenNameList(GLOBALVARS *gv) const
{
    QList<QString> resultList, middleNameList;
    QString name;
    databaseSearches dbSearch;

    name = getFirstName().getString();
    if ((name.size() > 0) && dbSearch.givenNameLookup(name, gv, genderUnknown))
        resultList.append(name);

    name = getFirstNameAKA1().getString();
    if ((name.size() > 0) && dbSearch.givenNameLookup(name, gv, genderUnknown))
        resultList.append(name);

    name = getFirstNameAKA2().getString();
    if ((name.size() > 0) && dbSearch.givenNameLookup(name, gv, genderUnknown))
        resultList.append(name);

    if(getMiddleNameList(middleNameList))
    {
        while (middleNameList.size() > 0)
        {
            name = middleNameList.takeFirst();
            if (!resultList.contains(name) && dbSearch.givenNameLookup(name, gv, genderUnknown))
                resultList.append(name);
        }
    }

    return resultList;
}

PQString dataRecord::getLastName() const
{
    return familyName;
}

PQString dataRecord::getLastNameAlt1() const
{
    return familyNameAlt1;
}

PQString dataRecord::getLastNameAlt2() const
{
    return familyNameAlt2;
}

PQString dataRecord::getLastNameAlt3() const
{
    return familyNameAlt3;
}

PQString dataRecord::getFullName() const
{
    PQString sp(" ");
    PQString result = firstName;
    if (firstNameAKA1.getLength() > 0)
        result += sp + firstNameAKA1;
    if (firstNameAKA2.getLength() > 0)
        result += sp + firstNameAKA2;
    if (middleNames.getLength() > 0)
        result += sp + middleNames;
    if (familyName.getLength() > 0)
        result += sp + familyName;

    return result;
}

PQString dataRecord::getPrefix() const
{
	return prefix;
}

PQString dataRecord::getSuffix() const
{
	return suffix;
}

QDate dataRecord::getDOB() const
{
	return DOB;
}

QDate dataRecord::getMinDOB() const
{
    return minDOB;
}

QDate dataRecord::getMaxDOB() const
{
    return maxDOB;
}

QDate dataRecord::getDOD() const
{
	return DOD;
}

unsigned int dataRecord::getYOB() const
{
	return YOB;
}

unsigned int dataRecord::getYOD() const
{
	return YOD;
}

int dataRecord::getDeemedYOD()
{
    if (deemedYOD > 0)
        return deemedYOD;

    if (YOD > 0)
        return YOD;

    unsigned int daysSinceJan1publish, daysSinceJan1service;
    QDate pubDate;

    if (sourceID.publishDate.isValid() && (sourceID.publishDate > QDate(1900,1,1)))
    {
        if (commentDate.isValid() && (commentDate < sourceID.publishDate))
            pubDate = commentDate;
        else
            pubDate = sourceID.publishDate;
    }
    else
    {
        if (commentDate.isValid())
            pubDate = commentDate;
    }

    if (DOS.isValid())
    {
        if (pubDate.isValid())
        {
            if ((DOS.year() == pubDate.year()))
            {
                daysSinceJan1publish = pubDate.toJulianDay() - QDate(pubDate.year(), 1, 1).toJulianDay();
                daysSinceJan1service = DOS.toJulianDay() - QDate(DOS.year(), 1, 1).toJulianDay();
                if ((daysSinceJan1publish < 5) || (daysSinceJan1service < 5))
                    deemedYOD = pubDate.year() - 1;
                else
                    deemedYOD = pubDate.year();
            }
            else
            {
                int diff = DOS.year() - pubDate.year();
                if (diff == 1)
                    deemedYOD = pubDate.year();
                else
                {
                    if (diff == -1)
                        deemedYOD = DOS.year();
                    // Bigger difference likely means one date is just wrong
                }
            }
        }
        else
        {
           daysSinceJan1service = DOS.toJulianDay() - QDate(DOS.year(), 1, 1).toJulianDay();
           if (daysSinceJan1service < 10)
                deemedYOD = DOS.year() - 1;
            else
                deemedYOD = DOS.year();
        }
    }
    else
    {
        if (pubDate.isValid())
        {
            daysSinceJan1publish = pubDate.toJulianDay() - QDate(pubDate.year(), 1, 1).toJulianDay();
            if (daysSinceJan1publish < 5)
                deemedYOD = pubDate.year() - 1;
            else
                deemedYOD = pubDate.year();
        }
    }

    return deemedYOD;
}

unsigned int dataRecord::getAgeAtDeath() const
{
	return ageAtDeath;
}

bool dataRecord::getDOBcredibility() const
{
    return DOBfullyCredible;
}

bool dataRecord::getDODcredibility() const
{
    return DODfullyCredible;
}

bool dataRecord::getYOBcredibility() const
{
    return YOBfullyCredible;
}

bool dataRecord::getYODcredibility() const
{
    return YODfullyCredible;
}

bool dataRecord::getAgeAtDeathCredibility() const
{
    return ageAtDeathFullyCredible;
}

NAMESKNOWN dataRecord::getNamesKnown() const
{
	NAMESKNOWN result;

    if (familyName.getLength() > 0)
	{
        if (firstName.getLength() > 0)
			result = nkFirstAndLast;
		else
			result = nkLastOnly;
	}
	else
	{
        if (firstName.getLength() > 0)
			result = nkFirstOnly;
		else
			result = nkNone;
	}

	return result;
}

unsigned int dataRecord::getNumFamilyNames() const
{
    unsigned int result = 0;
    if (familyName.getLength() > 0)
        result++;
    if (familyNameAlt1.getLength() > 0)
        result++;
    if (familyNameAlt2.getLength() > 0)
        result++;
    if (familyNameAlt3.getLength() > 0)
        result++;

    return result;
}

unsigned int dataRecord::getNumFirstNames() const
{
    unsigned int result = 0;
    if (firstName.getLength() > 0)
        result++;
    if (firstNameAKA1.getLength() > 0)
        result++;
    if (firstNameAKA2.getLength() > 0)
        result++;

    return result;
}

PQString dataRecord::getTitle() const
{
    return title;
}

PQString dataRecord::getTitleKey() const
{
    return titleKey;
}

bool dataRecord::getNeeEtAlEncountered() const
{
    return neeEtAlEncountered;
}

bool dataRecord::getAgeNextReference() const
{
    return ageNextReference;
}

bool dataRecord::getMaleHyphenated() const
{
    return maleHyphenated;
}

unsigned int dataRecord::getSingleYear() const
{
    return singleYear;
}

/*QString dataRecord::getPotentialFirstName() const
{
    return potentialGivenName;
}*/

QString dataRecord::getUsedFirstNameFromStructured() const
{
    return usedFirstNameFromStructured;
}

QString dataRecord::getUsedFirstNameFromUnstructured() const
{
    return usedFirstNameFromUnstructured;
}

QString dataRecord::getFirstGivenNameUsedInSentence() const
{
    return firstGivenNameUsedInSentence;
}

QString dataRecord::getSpouseName() const
{
    return spouseName;
}

POSTALCODE_INFO dataRecord::getPostalCodeInfo() const
{
    return postalCodeInfo;
}

QString dataRecord::getRawFullName() const
{
    return rawFullName;
}

QString dataRecord::getObitSnippet() const
{
    return obitSnippet;
}

void dataRecord::xport(QString filename, int extraOptParam)
{
    Q_UNUSED(extraOptParam);
    QString comma(",");
    PQString outputFile(filename);

    globals.output = new QFile((globals.batchDirectory + PQString("\\") + outputFile).getString());
    if(globals.output->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        QTextStream outputStream(globals.output);
        outputStream.setEncoding(QStringEncoder::Utf8);

        // Before exporting, check for problematic double quotes in names
        // checkNames deletes any double quotes found
        if (checkNames())
        {
            PQString errMsg;
            errMsg << "Unexpected double quotes found in:  " << sourceID.URL.getString();
            globals.logMsg(ErrorRecord, errMsg);
        }

        outputStream << familyName.getString() << comma;
        outputStream << familyNameAlt1.getString() << comma;
        outputStream << familyNameAlt2.getString() << comma;
        outputStream << familyNameAlt3.getString() << comma;
        outputStream << firstName.getString() << comma;
        outputStream << firstNameAKA1.getString() << comma;
        outputStream << firstNameAKA2.getString() << comma;
        outputStream << middleNames.getString() << comma;
        outputStream << prefix.getString() << comma;
        outputStream << suffix.getString() << comma;
        outputStream << gender << comma;
        outputStream << DOB.toString("yyyyMMdd") << comma;
        outputStream << DOD.toString("yyyyMMdd") << comma;
        outputStream << minDOB.toString("yyyyMMdd") << comma;
        outputStream << maxDOB.toString("yyyyMMdd") << comma;
        outputStream << YOB << comma;
        outputStream << YOD << comma;
        outputStream << ageAtDeath << comma;
        outputStream << sourceID.URL.getString() << comma;
        outputStream << lastUpdated.toString("yyyyMMdd") << comma;
        outputStream << language << comma;
        switch (language)
        {
        case english:
            outputStream << "English" << comma;
            break;

        case french:
            outputStream << "French" << comma;
            break;

        case spanish:
            outputStream << "Spanish" << comma;
            break;

        case multiple:
            outputStream << "Multiple" << comma;
            break;

        case multiple_unknown:
            outputStream << "Multiple unknown" << comma;
            break;

        default:
            outputStream << "Language unknown" << comma;
        }

        outputStream << sourceID.provider << comma << sourceID.providerKey << comma << sourceID.ID.getString() << comma;
        outputStream << DOS.toString("yyyyMMdd") << comma << sourceID.publishDate.toString("yyyyMMdd") << comma;
        outputStream << spouseName << comma;
        outputStream << wi.ageFlag << comma << wi.dateFlag << comma << wi.genderFlag << comma << wi.nameReversalFlag << comma << wi.nameFlagGeneral << comma;
        outputStream << wi.doubleMemorialFlag << comma << wi.outstandingErrMsg << comma;
        if (getMaleHyphenated())
            outputStream << "1" << comma;
        else
            outputStream << "0" << comma;
        //outputStream << wi.memorialFlag << comma;
        outputStream << wi.futureUse << comma;
        outputStream << wi.validated << comma ;
        outputStream << wi.confirmTreatmentName << comma;
        outputStream << postalCodeInfo.getPostalCode() << comma << priorUnmatched << comma << previouslyLoaded << comma;
        //outputStream << extraOptParam << comma;
        outputStream << firstGivenNameUsedInSentence << comma;
        outputStream << Qt::endl;

        globals.output->close();
    }
}

bool dataRecord::isAFirstName(const PQString &name) const
{
    if ((name.getLength() == 0) || (sourceID.providerKey == 0))  // second condition captures undefined dr record
        return false;

    if ((firstName.getLength() > 0) && (firstName.lower().getUnaccentedString() == name.lower().getUnaccentedString()))
		return true;
	else
	{
        if ((firstNameAKA1.getLength() > 0) && (firstNameAKA1.lower().getUnaccentedString() == name.lower().getUnaccentedString()))
			return true;
		else
		{
            if ((firstNameAKA2.getLength() > 0) && (firstNameAKA2.lower().getUnaccentedString() == name.lower().getUnaccentedString()))
				return true;
			else
			{
				return false;
			}
		}
	}
}

bool dataRecord::isALastName(const PQString &name) const
{
    if ((name.getLength() == 0) || (sourceID.providerKey == 0))  // second conditions addresses undefined dr record
        return false;

    if ((familyName.getLength() > 0) && (familyName.lower().getUnaccentedString() == name.lower().getUnaccentedString()))
		return true;
	else
	{
        if ((familyNameAlt1.getLength() > 0) && (familyNameAlt1.lower().getUnaccentedString() == name.lower().getUnaccentedString()))
			return true;
		else
		{
            if ((familyNameAlt2.getLength() > 0) && (familyNameAlt2.lower().getUnaccentedString() == name.lower().getUnaccentedString()))
				return true;
			else
			{
                if ((familyNameAlt3.getLength() > 0) && (familyNameAlt3.lower().getUnaccentedString() == name.lower().getUnaccentedString()))
					return true;
				else
				{
					return false;
				}
			}
		}
	}
}

bool dataRecord::isAMiddleName(const PQString &name) const
{
    if ((name.getLength() == 0) || (sourceID.providerKey == 0))  // second conditions addresses undefined dr record
        return false;

    bool matched = false;

    OQStream temp(middleNames);
    for (unsigned int i = 0; i < middleNames.countWords(); i++)
	{
        if (name.lower().getUnaccentedString() == temp.getWord(true).lower().getUnaccentedString())
			matched = true;
	}
	
	return matched;
}

bool dataRecord::isASuffix(const PQString &name) const
{
    if (name.getLength() == 0)
        return false;

    if (name.lower().getUnaccentedString() == suffix.lower().getUnaccentedString())
		return true;
	else
		return false;
}

bool dataRecord::isAPrefix(const PQString &name) const
{
    if (name.getLength() == 0)
        return false;

    if (name.lower().getUnaccentedString() == prefix.lower().getUnaccentedString())
		return true;
	else
		return false;
}

bool dataRecord::isAnInitial(const PQString &letter) const
{
    if (letter.getLength() != 1)
        return false;

    PQString initial;
    OQStream tempStream;

    if (firstName.getLength() > 0)
    {
        initial = firstName.left(1);
        if (letter.lower().getUnaccentedString() == initial.lower().getUnaccentedString())
            return true;
    }

    if (firstNameAKA1.getLength() > 0)
    {
        initial = firstNameAKA1.left(1);
        if (letter.lower().getUnaccentedString() == initial.lower().getUnaccentedString())
            return true;
    }

    if (firstNameAKA2.getLength() > 0)
    {
        initial = firstNameAKA2.left(1);
        if (letter.lower().getUnaccentedString() == initial.lower().getUnaccentedString())
            return true;
    }

    tempStream = middleNames;
    if (middleNames.getLength() > 0)
    {
        while (!tempStream.isEOS())
        {
            initial = tempStream.getWord().left(1);
            if (letter.lower().getUnaccentedString() == initial.lower().getUnaccentedString())
                return true;
        }
    }

    // Default answer if no hits
    return false;
}

bool dataRecord::hasFirstNameInitial() const
{
    bool found = false;

    if (firstName.getLength() == 1)
        found = true;

    if (!found && (firstNameAKA1.getLength() == 1))
        found = true;

    if (!found && (firstNameAKA2.getLength() == 1))
        found = true;

    return found;
}

bool dataRecord::hasMiddleNameInitial() const
{
    bool found = false;
    QString name;

    QList<QString> nameList = middleNames.getString().split(" ");
    while (!found && (nameList.count() > 0))
    {
        name = nameList.takeFirst();
        if (name.length() == 1)
            found = true;
    }

    return found;
}

bool dataRecord::replaceFirstNameInitial(const PQString &name)
{
    bool replaced = false;

    if ((firstName.getLength() == 1) && (firstName.lower().getUnaccentedString() == name.left(1).lower().getUnaccentedString()))
    {
        firstName = name.proper();
        replaced = true;
    }

    if (!replaced && (firstNameAKA1.getLength() == 1) && (firstNameAKA1.lower().getUnaccentedString() == name.left(1).lower().getUnaccentedString()))
    {
        firstNameAKA1 = name.proper();
        replaced = true;
    }

    if (!replaced && (firstNameAKA2.getLength() == 1) && (firstNameAKA2.lower().getUnaccentedString() == name.left(1).lower().getUnaccentedString()))
    {
        firstNameAKA2 = name.proper();
        replaced = true;
    }

    return replaced;
}

bool dataRecord::replaceMiddleNameInitial(const PQString &name)
{
    bool replaced = false;

    QString existingName;
    QList<QString> newNameList;
    QList<QString> existingNameList = middleNames.getString().split(" ");

    for (int i = 0; i < existingNameList.count(); i++)
    {
        existingName = existingNameList.at(i);
        if ((existingName.length() == 1) && (existingName.toLower() == name.left(1).lower().getUnaccentedString()))
        {
            existingName = name.proper().getString();
            replaced = true;
        }
        newNameList.append(existingName);
    }

    middleNames = PQString(newNameList.join(" "));

    return replaced;
}

NAMETYPE dataRecord::isAName(const PQString &name)
{
    if ((name.getLength() == 0) || (sourceID.providerKey == 0))  // second conditions addresses undefined dr record
        return ntUnknown;

    NAMETYPE result;

    result = isASavedName(name);
    if (static_cast<bool>(result))
        return result;
    else
    {
        if (isANickName(name))
            return ntFirst;
        else
            return ntUnknown;
    }
}

NAMETYPE dataRecord::isASavedName(const PQString &name)
{
    if ((name.getLength() == 0) || (sourceID.providerKey == 0))  // second conditions addresses undefined dr record
        return ntUnknown;

    if (isAFirstName(name))
		return ntFirst;
	else
	{
		if (isALastName(name))
			return ntLast;
		else
		{
			if (isAMiddleName(name))
				return ntMiddle;
			else
			{
                if (isAPrefix(name))
                    return ntPrefix;
                else
                {
                    if (isASuffix(name))
                        return ntSuffix;
                    else
                        return ntUnknown;
                }
			}
		}
	}
}

bool dataRecord::isANickName(const OQString &name)
{
    if (name.getLength() == 0)
        return false;

    bool matched = false;
    PQString warningMessage;

    matched = name.isInformalVersionOf(getFirstName().lower().getString(), warningMessage);

    if (!matched  && (warningMessage.getLength() == 0))
        matched = name.isInformalVersionOf(getFirstNameAKA1().lower().getString(), warningMessage);

    if (!matched  && (warningMessage.getLength() == 0))
        matched = name.isInformalVersionOf(getFirstNameAKA2().lower().getString(), warningMessage);

    if (!matched  && (warningMessage.getLength() == 0))
    {
        QList<QString> middleNameList;
        if (getMiddleNameList(middleNameList))
        {
            while (!matched && (middleNameList.size() > 0)  && (warningMessage.getLength() == 0))
            {
                matched = name.isInformalVersionOf(middleNameList.takeFirst(), warningMessage);
            }
        }
    }

    if (warningMessage.getLength() > 0)
        globals.logMsg(ErrorSQL, warningMessage);

    return matched;

}

bool dataRecord::isAFormalName(const OQString &name)
{
    if (name.getLength() == 0)
        return false;

    bool matched = false;
    PQString warningMessage;
    OQString potentialNickName;
    QString potentialFormalName = name.lower().getUnaccentedString();

    potentialNickName = getFirstName().lower().getUnaccentedString();
    matched = potentialNickName.isInformalVersionOf(potentialFormalName, warningMessage);

    if (!matched  && (warningMessage.getLength() == 0))
    {
        potentialNickName = getFirstNameAKA1().lower().getUnaccentedString();
        matched = potentialNickName.isInformalVersionOf(potentialFormalName, warningMessage);
    }

    if (!matched  && (warningMessage.getLength() == 0))
    {
        potentialNickName = getFirstNameAKA2().lower().getUnaccentedString();
        matched = potentialNickName.isInformalVersionOf(potentialFormalName, warningMessage);
    }

    if (!matched  && (warningMessage.getLength() == 0))
    {
        QList<QString> middleNameList;
        if (getMiddleNameList(middleNameList))
        {
            while (!matched && (middleNameList.size() > 0)  && (warningMessage.getLength() == 0))
            {
                potentialNickName = middleNameList.takeFirst();
                matched = potentialNickName.isInformalVersionOf(potentialFormalName, warningMessage);
            }
        }
    }

    if (warningMessage.getLength() > 0)
        globals.logMsg(ErrorSQL, warningMessage);

    return matched;

}

bool dataRecord::isANameVersion(const OQString &name, const bool expandedCheck)
{
    bool matched;

    matched = isAFirstName(name) || isANickName(name) || isAFormalName(name);

    if (!matched && expandedCheck)
        matched = isASimilarName(name);

    return matched;
}

bool dataRecord::isASimilarName(const OQString &name)
{
    // First while loop looks for same three starting chars
    // Second while loop checks if one name contains the other

    bool matched = false;
    PQString firstPortion = name.left(3).lower().getUnaccentedString();
    PQString target;
    OQStream middles(middleNames);

    int i = 0;
    while (!matched && (i < 30))
    {
        target.clear();

        switch(i)
        {
        case 0:
            target = firstName.lower().getUnaccentedString();
            break;

        case 1:
            target = firstNameAKA1.lower().getUnaccentedString();
            break;

        case 2:
            target = firstNameAKA2.lower().getUnaccentedString();
            i = 9;
            break;

        case 10:
        case 11:
        case 12:
        case 13:
            target = middles.getWord().lower().getUnaccentedString();
            break;

        case 14:
            target.clear();
            i = 19;
            break;

        case 20:
            target = familyName.lower().getUnaccentedString();
            break;

        case 21:
            target = familyNameAlt1.lower().getUnaccentedString();
            break;

        case 22:
            target = familyNameAlt2.lower().getUnaccentedString();
            break;

        case 23:
            target = familyNameAlt3.lower().getUnaccentedString();
            break;
        }

        if (target.left(3) != QString("van"))
            target = target.left(3);

        if (target.getLength() == 0)
            i =  10 * static_cast<int>((i + 10) / 10);
        else
        {
            if (firstPortion == target)
                matched = true;
            else
                i++;
        }
    }

    i = 0;
    middles.beg();
    OQString searchIn;
    while (!matched && (i < 30))
    {
        target.clear();
        searchIn.clear();

        switch(i)
        {
        case 0:
            if (firstName.getLength() > 0)
            {
                if (name.getLength() > firstName.getLength())
                {
                    target = firstName;
                    searchIn = name;
                }
                else
                {
                    target = name;
                    searchIn = firstName;
                }
            }
            break;

        case 1:
            if (firstNameAKA1.getLength() > 0)
            {
                if (name.getLength() > firstNameAKA1.getLength())
                {
                    target = firstNameAKA1;
                    searchIn = name;
                }
                else
                {
                    target = name;
                    searchIn = firstNameAKA1;
                }
            }
            break;

        case 2:
            if (firstNameAKA2.getLength() > 0)
            {
                if (name.getLength() > firstNameAKA2.getLength())
                {
                    target = firstNameAKA2;
                    searchIn = name;
                }
                else
                {
                    target = name;
                    searchIn = firstNameAKA2;
                }
            }
            i = 9;
            break;

        case 10:
        case 11:
        case 12:
        case 13:
            target = middles.getWord().left(3).lower().getUnaccentedString();
            if (target.getLength() > 0)
            {
                if (name.getLength() > target.getLength())
                {
                    target = target.getString();
                    searchIn = name;
                }
                else
                {
                    searchIn = target;
                    target = name;
                }
            }
            break;

        case 14:
            i = 19;
            break;

        case 20:
            if (familyName.getLength() > 0)
            {
                if (name.getLength() > familyName.getLength())
                {
                    target = familyName;
                    searchIn = name;
                }
                else
                {
                    target = name;
                    searchIn = familyName;
                }
            }
            break;

        case 21:
            if (familyNameAlt1.getLength() > 0)
            {
                if (name.getLength() > familyNameAlt1.getLength())
                {
                    target = familyNameAlt1;
                    searchIn = name;
                }
                else
                {
                    target = name;
                    searchIn = familyNameAlt1;
                }
            }
            break;

        case 22:
            if (familyNameAlt2.getLength() > 0)
            {
                if (name.getLength() > familyNameAlt2.getLength())
                {
                    target = familyNameAlt2;
                    searchIn = name;
                }
                else
                {
                    target = name;
                    searchIn = familyNameAlt2;
                }
            }
            break;

        case 23:
            if (familyNameAlt3.getLength() > 0)
            {
                if (name.getLength() > familyNameAlt3.getLength())
                {
                    target = familyNameAlt3;
                    searchIn = name;
                }
                else
                {
                    target = name;
                    searchIn = familyNameAlt3;
                }
            }
            break;
        }

        if (target.getLength() == 0)
            i =  10 * static_cast<int>((i + 10) / 10);
        else
        {
            if ((target.getLength() >= 3) && searchIn.getString().contains(target.getString(), Qt::CaseInsensitive))
                matched = true;
            else
                i++;
        }
    }

    return matched;
}

unsigned int dataRecord::getNumSavedNames() const
{
    return getNumFirstNames() + middleNames.countWords() + getNumFamilyNames();
}

bool dataRecord::missingDOB() const
{
    if (DOB.isValid())
		return false;
	else
		return true;
}

bool dataRecord::missingDOD() const
{
    if (DOD.isValid())
		return false;
	else
		return true;
}

void dataRecord::setGlobals(GLOBALVARS &gv)
{
    globals = gv;
}

void dataRecord::clear()
{
 familyName.clear();
 familyNameAlt1.clear();
 familyNameAlt2.clear();
 familyNameAlt3.clear();
 firstName.clear();
 firstNameAKA1.clear();
 firstNameAKA2.clear();
 middleNames.clear();
 middleNameUsedAsFirstName.clear();
 parentsLastName.clear();
 maidenNames.clear();
 suffix.clear();
 prefix.clear();
 gender = genderUnknown;
 workingGender = genderUnknown;
 DOB.setDate(0,0,0);
 DOD.setDate(0,0,0);
 typoDOB.setDate(0,0,0);
 typoDOD.setDate(0,0,0);
 YOB = 0;
 YOD = 0;
 deemedYOD = 0;
 ageAtDeath = 0;
 DOBfullyCredible = false;
 DODfullyCredible = false;
 YOBfullyCredible = false;
 YODfullyCredible = false;
 ageAtDeathFullyCredible = false;
 datesLocked = false;
 minDOB.setDate(0,0,0);
 maxDOB.setDate(0,0,0);
 sourceID.clear();
 firstRecorded.setDate(0,0,0);
 lastUpdated.setDate(0,0,0);
 language = language_unknown;
 DOS.setDate(0,0,0);
 commentDate.setDate(0,0,0);
 numUnmatched = 0;
 priorUnmatched = 0;
 previouslyLoaded = 0;
 title.clear();
 titleKey.clear();
 neeEtAlEncountered = false;
 ageNextReference = false;
 maleHyphenated = false;
 singleYear = 0;
 usedFirstNameFromUnstructured.clear();
 usedFirstNameFromStructured.clear();
 firstGivenNameUsedInSentence.clear();
 spouseName.clear();
 wi.clear();
 postalCodeInfo.clear();
 rawFullName.clear();
 obitSnippet.clear();
 permanentErrorFlag = false;
}

void dataRecord::clearDates()
{
 DOB.setDate(0,0,0);
 DOD.setDate(0,0,0);
 typoDOB.setDate(0,0,0);
 typoDOD.setDate(0,0,0);
 YOB = 0;
 YOD = 0;
 deemedYOD = 0;
 ageAtDeath = 0;
 DOBfullyCredible = false;
 DODfullyCredible = false;
 YOBfullyCredible = false;
 YODfullyCredible = false;
 ageAtDeathFullyCredible = false;
 datesLocked = false;
 minDOB.setDate(0,0,0);
 maxDOB.setDate(0,0,0);
 DOS.setDate(0,0,0);
 commentDate.setDate(0,0,0);
}

void dataRecord::clearLastNames()
{
    familyName.clear();
    familyNameAlt1.clear();
    familyNameAlt2.clear();
    familyNameAlt3.clear();
}

void dataRecord::clearFirstNames()
{
    firstName.clear();
    firstNameAKA1.clear();
    firstNameAKA2.clear();
}

void dataRecord::clearMiddleNames()
{
    middleNames.clear();
}

void dataRecord::removeUnnecessaryInitials()
{
    PQString initial;

    // Clean up first names first
    if (firstName.getLength() == 1)
    {
        initial = firstName;
        if ((firstNameAKA1.getLength() > 0) && (firstNameAKA1.countWords() == 1))
        {
            if (firstNameAKA1.left(1) == initial)
            {
                firstName = firstNameAKA1;
                firstNameAKA1 = firstNameAKA2;
                firstNameAKA2.clear();

                // Repeat test based on new structure with max of two names remaining
                if (firstName.getLength() == 1)
                {
                    initial = firstName;
                    if ((firstNameAKA1.getLength() > 0) && (firstNameAKA1.countWords() == 1))
                    {
                        if (firstNameAKA1.left(1) == initial)
                        {
                            firstName = firstNameAKA1;
                            firstNameAKA1.clear();
                        }
                    }
                }
            }
        }
    }

    if (firstNameAKA1.getLength() == 1)
    {
        initial = firstNameAKA1;
        if ((firstName.getLength() > 0) && (firstName.countWords() == 1))
        {
            if (firstName.left(1) == initial)
            {
                firstNameAKA1 = firstNameAKA2;
                firstNameAKA2.clear();

                // Repeat test based on new structure with last remaining name
                if (firstNameAKA1.getLength() == 1)
                {
                    initial = firstNameAKA1;
                    if ((firstName.getLength() > 0) && (firstName.countWords() == 1))
                    {
                        if (firstName.left(1) == initial)
                           firstNameAKA1.clear();
                    }
                }
            }
        }
    }

    if (firstNameAKA2.getLength() == 1)
    {
        initial = firstNameAKA2;
        if ((firstName.getLength() > 0) && (firstName.countWords() == 1))
        {
            if (firstName.left(1) == initial)
                firstNameAKA2.clear();
        }

        if ((firstNameAKA1.getLength() > 0) && (firstNameAKA1.countWords() == 1))
        {
            if (firstNameAKA1.left(1) == initial)
                firstNameAKA2.clear();
        }
    }

    // Clean up middle names second
    if (middleNames.countWords() >= 2)
    {
        bool potentialIssue = false;
        OQString singleName;
        OQStream allNames = middleNames;
        QList<OQString> listOfNames;

        // Create list of middle names and flag if an initial is present
        while (!allNames.isEOS())
        {
            listOfNames.append(singleName = allNames.getWord());
            if (singleName.getLength() == 1)
                potentialIssue = true;
        }

        // At least one initial
        if (potentialIssue)
        {
            int i;
            int rotationsRemaining = listOfNames.size() - 1;
            bool noDeletion;

            while ((rotationsRemaining >= 0) && (listOfNames.size() >= 2))
            {
                singleName = listOfNames.at(0);
                noDeletion = true;
                if (singleName.getLength() == 1)
                {
                   initial = singleName;
                   i = 1;
                   while ((i < listOfNames.size()) && noDeletion)
                   {
                       if (listOfNames.at(i).left(1) == initial)
                       {
                           listOfNames.removeFirst();
                           rotationsRemaining--;
                           noDeletion = false;
                       }
                       else
                           i++;
                   }
                }

                if (noDeletion)
                {
                    singleName = listOfNames.takeFirst();
                    listOfNames.append(singleName);
                    rotationsRemaining--;
                }
            }
        }
    }
}

void dataRecord::setID(const PQString &id)
{
    PQString tempID = id;
    if (id.left(1) == PQString("-"))
        tempID.dropLeft(1);
    sourceID.ID = tempID.left(85);
}

void dataRecord::setProvider(const PROVIDER &pvdr)
{
    sourceID.provider = pvdr;
}

void dataRecord::setProviderKey(const unsigned int pk)
{
    sourceID.providerKey = pk;
}

void dataRecord::setUnmatched(const unsigned int num)
{
    numUnmatched = num;
}

void dataRecord::setPriorUnmatched(const unsigned int num)
{
    priorUnmatched = num;
}

void dataRecord::setPreviouslyLoaded(const unsigned int num)
{
    previouslyLoaded = num;
}

PROVIDER dataRecord::getProvider() const
{
    return sourceID.provider;
}

unsigned int dataRecord::getProviderKey() const
{
    return sourceID.providerKey;
}

PQString dataRecord::getID() const
{
    return sourceID.ID;
}

QDate dataRecord::getPublishDate() const
{
    return sourceID.publishDate;
}

unsigned int dataRecord::getDeceasedNumber() const
{
    return sourceID.deceasedNumber;
}

SOURCEID dataRecord::getSourceID() const
{
    return sourceID;
}

QDate dataRecord::getFirstRecorded() const
{
    return firstRecorded;
}

QDate dataRecord::getDOS() const
{
    return DOS;
}

QDate dataRecord::getCommentDate() const
{
    return commentDate;
}

unsigned int dataRecord::getCycle() const
{
    return cycle;
}

unsigned int dataRecord::getPreviouslyLoaded() const
{
    return previouslyLoaded;
}

bool dataRecord::checkNames()
{
    // Checks for any double quotes before exporting to CSV file

    bool quotesFound = false;

    if (familyName.getString().contains(QChar(34)))
    {
        familyName.removeQuotes();
        quotesFound = true;
    }

    if (familyNameAlt1.getString().contains(QChar(34)))
    {
        familyNameAlt1.removeQuotes();
        quotesFound = true;
    }

    if (familyNameAlt2.getString().contains(QChar(34)))
    {
        familyNameAlt2.removeQuotes();
        quotesFound = true;
    }

    if (familyNameAlt3.getString().contains(QChar(34)))
    {
        familyNameAlt3.removeQuotes();
        quotesFound = true;
    }

    if (firstName.getString().contains(QChar(34)))
    {
        firstName.removeQuotes();
        quotesFound = true;
    }

    if (firstNameAKA1.getString().contains(QChar(34)))
    {
        firstNameAKA1.removeQuotes();
        quotesFound = true;
    }

    if (firstNameAKA2.getString().contains(QChar(34)))
    {
        firstNameAKA2.removeQuotes();
        quotesFound = true;
    }

    if (middleNames.getString().contains(QChar(34)))
    {
        middleNames.removeQuotes();
        quotesFound = true;
    }

    return quotesFound;
}

void dataRecord::simplifyInitials(PQString &word)
{
    if ((word.getLength() == 3) && (word.middle(1,1) == PQString(".")))
    {
        if (isAnInitial(word.left(1)))
        {
            if (isAnInitial(word.right(1)))
                word.clear();
            else
                word = word.right(1).proper();
        }
        else
        {
            if (isAnInitial(word.right(1)))
                word = word.left(1).proper();
            else
                word = word.left(1) + PQString(" ") + word.right(1);
        }
    }

    return;
}

void dataRecord::adjustHyphenatedLastNames()
{
    bool adjustmentRequired = false;
    if (familyName.isHyphenated() ||
            ((familyNameAlt1.getLength() > 0) && (familyNameAlt1.isHyphenated())) ||
            ((familyNameAlt2.getLength() > 0) && (familyNameAlt2.isHyphenated())) ||
            ((familyNameAlt3.getLength() > 0) && (familyNameAlt3.isHyphenated())))
        adjustmentRequired = true;

    if (adjustmentRequired)
    {
        // Create list of names
        QList<PQString> names;

        addSingleNamesToList(names, familyName);
        if (familyNameAlt1.getLength() > 0)
            addSingleNamesToList(names, familyNameAlt1);
        if (familyNameAlt2.getLength() > 0)
            addSingleNamesToList(names, familyNameAlt2);
        if (familyNameAlt3.getLength() > 0)
            addSingleNamesToList(names, familyNameAlt3);

        // Remove any duplicate names (drop second reference to a single name
        int numWords = names.size();
        int i, j;
        bool stillValid;
        PQString nameA, nameB;

        int position = 0;
        while (position < (numWords - 1))
        {
            i = position + 1;
            stillValid = true;
            nameA = names.at(position).lower().getUnaccentedString();

            while ((stillValid) && (i < numWords))
            {
                nameB = names.at(i).lower().getUnaccentedString();
                if (nameA == nameB)
                {
                    stillValid = false;
                    names.removeAt(i);
                    numWords--;
                }
                i++;
            }
            position++;
        }

        // Save new list of names back to datarecord
        PQString tempName;
        familyName.clear();
        familyNameAlt1.clear();
        familyNameAlt2.clear();
        familyNameAlt3.clear();
        j = numWords;
        if (numWords > 4)
            j = 4;
        for (i = 0; i < j; i++)
        {
            tempName = names.at(i);
            this->setFamilyName(tempName);
        }
    }
}

void dataRecord::addSingleNamesToList(QList<PQString> &list, PQString fullName)
{
    QString firstPart, secondPart, FullName;
    PQString newName;
    OQString checkName;
    int i;

    if (fullName.isHyphenated())
    {
        checkName = fullName;
        if (checkName.isHyphenatedName())
        {
            checkName.removeHyphens();
            checkName = checkName.proper();
            list.append(checkName.proper());
        }
        else
        {
            FullName = fullName.getString();
            i = FullName.indexOf(QChar(45));
            if (i == -1)
                i = FullName.indexOf(QChar(8211));
            firstPart = FullName.left(i);
            secondPart = FullName.right(static_cast<int>(fullName.getLength()) - (i+1));
            newName = PQString(firstPart);
            newName.cleanUpEnds();
            list.append(newName);
            newName = PQString(secondPart);
            newName.cleanUpEnds();
            list.append(newName);
        }
    }
    else
        list.append(fullName);
}

void dataRecord::pullGivenNames(QList<QString> &names) const
{
    // Order is quite deliberate, as more weight accorded to earlier entries

    if (firstName.getLength() > 0)
        names.append(firstName.getString());

    if (middleNames.getLength() > 0)
    {
        OQStream temp(middleNames);
        PQString word;
        while (!temp.isEOS())
        {
            word = temp.getWord();
            names.append(word.getString());
        }
    }

    if (firstNameAKA1.getLength() > 0)
        names.append(firstNameAKA1.getString());

    if (firstNameAKA2.getLength() > 0)
        names.append(firstNameAKA2.getString());
}

void dataRecord::pullUniqueGivenNames(QList<QString> &names) const
{
    QList<QString> tempList;
    pullGivenNames(tempList);

    PQString errMsg;
    bool include;
    int i, j;

    j = 0;
    while (j < tempList.count())
    {
        include = true;
        i = 0;
        while (include && (i < tempList.count()))
        {
            if (i != j)
                include = !OQString(tempList[i]).isInformalVersionOf(tempList[j], errMsg);
            if (!include &&  (j < i))
                include = true;
            i++;
        }
        if (include)
            names.append(tempList[j]);
        j++;
    }
}

void dataRecord::incorporateParentsLastName()
{
    if (parentsLastName.getLength() == 0)
        return;

    PQString name;
    name = parentsLastName;

    setFamilyName(name);
    if (name.countWords() == 1)
        removeFromMiddleNames(parentsLastName);
}

void dataRecord::removeFromMiddleNames(PQString &name)
{
    if ((middleNames.getLength() == 0) || (name.getLength() == 0))
        return;

    QString names = middleNames.getString();
    QString toBeRemoved = name.getString();
    int index = names.indexOf(toBeRemoved, 0, Qt::CaseInsensitive);
    if (index >= 0)
    {
        OQString tempNames;

        names.remove(index, toBeRemoved.size());
        names.replace(QString("  "), QString(" "));
        tempNames = names;
        tempNames.cleanUpEnds();

        middleNames = tempNames.getString();
    }
}

void dataRecord::removeFromFirstNames(PQString &name)
{
    if (firstName.getLength() == 0)
        return;

    if (firstName == name)
    {
        firstName = firstNameAKA1;
        firstNameAKA1 = firstNameAKA2;
        firstNameAKA2.clear();
    }
    else
    {
        if (firstNameAKA1.getLength() == 0)
            return;

        if (firstNameAKA1 == name)
        {
            firstNameAKA1 = firstNameAKA2;
            firstNameAKA2.clear();
        }
        else
        {
            if (firstNameAKA2 == name)
                firstNameAKA2.clear();
        }
    }

}

void dataRecord::removeFromLastNames(PQString &name)
{
    if (familyName.getLength() == 0)
        return;

    if (familyName == name)
    {
        familyName = familyNameAlt1;
        familyNameAlt1 = familyNameAlt2;
        familyNameAlt2 = familyNameAlt3;
        familyNameAlt3.clear();
    }
    else
    {
        if (familyNameAlt1.getLength() == 0)
            return;

        if (familyNameAlt1 == name)
        {
            familyNameAlt1 = familyNameAlt2;
            familyNameAlt2 = familyNameAlt3;
            familyNameAlt3.clear();
        }
        else
        {
            if (familyNameAlt2.getLength() == 0)
                return;

            if (familyNameAlt2 == name)
            {
                familyNameAlt2 = familyNameAlt3;
                familyNameAlt3.clear();
            }
            else
            {
                if (familyNameAlt3 == name)
                    familyNameAlt3.clear();
            }
        }
    }
}

void dataRecord::updateToBestOf(dataRecord &dbRecord)
{    
    PQString tempName;
    NAMEINFO nameInfo;
    QList<NAMEINFO> nameInfoList;
    databaseSearches dbSearch;

    // Deal with first names
    nameInfoList.clear();
    nameInfo.type = ntFirst;
    nameInfo.name = dbRecord.getFirstName();
    nameInfoList.append(nameInfo);

    tempName = dbRecord.getFirstNameAKA1();
    if (tempName.getLength() > 0)
    {
        nameInfo.name = tempName;
        nameInfoList.append(nameInfo);

        tempName = dbRecord.getFirstNameAKA2();
        if (tempName.getLength() > 0)
        {
            nameInfo.name = tempName;
            nameInfoList.append(nameInfo);
        }
    }

    if (firstName.getLength() > 0)
    {
        bool matched = false;
        int i = 0;
        while (!matched && (i < nameInfoList.size()))
        {
            matched = (firstName.getUnaccentedString() == nameInfoList.at(i).name.getUnaccentedString());
            i++;
        }
        if (!matched)
        {
            nameInfo.name = firstName;
            nameInfoList.append(nameInfo);
        }
    }

    if (firstNameAKA1.getLength() > 0)
    {
        bool matched = false;
        int i = 0;
        while (!matched && (i < nameInfoList.size()))
        {
            matched = (firstNameAKA1.getUnaccentedString() == nameInfoList.at(i).name.getUnaccentedString());
            i++;
        }
        if (!matched)
        {
            nameInfo.name = firstNameAKA1;
            nameInfoList.append(nameInfo);
        }
    }

    if (firstNameAKA2.getLength() > 0)
    {
        bool matched = false;
        int i = 0;
        while (!matched && (i < nameInfoList.size()))
        {
            matched = (firstNameAKA2.getUnaccentedString() == nameInfoList.at(i).name.getUnaccentedString());
            i++;
        }
        if (!matched)
        {
            nameInfo.name = firstNameAKA2;
            nameInfoList.append(nameInfo);
        }
    }

    // Remove pure nicknames if list too long
    int i = 0;
    while((nameInfoList.size() > 3) && (i < nameInfoList.size()))
    {
        if (dbSearch.pureNickNameLookup(nameInfoList.at(i).name.getUnaccentedString(), &dbRecord.globals))
            nameInfoList.removeAt(i);
        else
            i++;
    }

    // Re-sort for nicknames
    int listSize = nameInfoList.size();
    OQString OQname;
    PQString errMsg;
    if (listSize > 1)
    {
        for (int i = 0; i < (listSize - 1); i++)
        {
            for (int j = i+1; j < listSize; j++)
            {
                OQname = nameInfoList.at(i).name;
                if (OQname.isInformalVersionOf(nameInfoList.at(j).name.getUnaccentedString(), errMsg))
                {
                    nameInfo = nameInfoList.takeAt(j);
                    nameInfoList.insert(i, nameInfo);
                }
            }
        }
    }

    // Push pure nicknames to end
    if (listSize > 1)
    {
        for (int i = 0; i < (listSize - 1); i++)
        {
            if (dbSearch.pureNickNameLookup(nameInfoList.at(i).name.getUnaccentedString(), &globals))
            {
                nameInfo = nameInfoList.takeAt(i);
                nameInfoList.append(nameInfo);
            }
        }
    }

    if (nameInfoList.size() > 3)
    {
        PQString errMsg;
        errMsg << "Loss of first name in 'best of' merge for: " << dbRecord.getID();
        dbRecord.globals.logMsg(ErrorRecord, errMsg);

        while (nameInfoList.size() > 3)
            nameInfoList.removeLast();
    }

    clearFirstNames();
    setAlternates(nameInfoList, true);


    // Deal with easy case of Middlenames - Will automatically just add new names
    nameInfoList.clear();
    nameInfo.clear();
    nameInfo.type = ntMiddle;
    OQStream tempStream(dbRecord.getMiddleNames());
    while (!tempStream.isEOS())
    {
        nameInfo.name = tempStream.getWord();
        nameInfoList.append(nameInfo);
    }
    setAlternates(nameInfoList, true);

    // Deal with last names
    nameInfoList.clear();
    nameInfo.clear();
    nameInfo.type = ntLast;

    nameInfo.name = dbRecord.getLastName();
    nameInfoList.append(nameInfo);

    tempName = dbRecord.getLastNameAlt1();
    if (tempName.getLength() > 0)
    {
        nameInfo.name = tempName;
        nameInfoList.append(nameInfo);

        tempName = dbRecord.getLastNameAlt2();
        if (tempName.getLength() > 0)
        {
            nameInfo.name = tempName;
            nameInfoList.append(nameInfo);

            tempName = dbRecord.getLastNameAlt3();
            if (tempName.getLength() > 0)
            {
                nameInfo.name = tempName;
                nameInfoList.append(nameInfo);
            }
        }
    }
    setAlternates(nameInfoList, true);

    // Deal with prefixes
    tempName = dbRecord.getPrefix();
    if (tempName.getLength() > 0)
    {
        nameInfoList.clear();
        nameInfo.clear();
        nameInfo.type = ntPrefix;

        nameInfo.name = tempName;
        nameInfoList.append(nameInfo);

        setAlternates(nameInfoList, true);
    }

    // Deal with suffixes
    tempName = dbRecord.getSuffix();
    if (tempName.getLength() > 0)
    {
        nameInfoList.clear();
        nameInfo.clear();
        nameInfo.type = ntSuffix;

        nameInfo.name = tempName;
        nameInfoList.append(nameInfo);

        setAlternates(nameInfoList, true);
    }

    // Deal with gender
    if (gender == genderUnknown)
        gender = dbRecord.getGender();
    else
    {
        if ((dbRecord.getGender() != genderUnknown) && (gender != dbRecord.getGender()))
        {
            PQString errMsg;
            errMsg << "Mismatched genders in 'best of' merge for: " << dbRecord.getID();
            dbRecord.globals.logMsg(ErrorRecord, errMsg);
        }
    }

    // Deal with DOB
    if (!DOB.isValid())
        DOB = dbRecord.getDOB();
    else
    {
        if (dbRecord.getDOB().isValid() && (DOB != dbRecord.getDOB()))
        {
            PQString errMsg;
            errMsg << "Mismatched DOBs in 'best of' merge for: " << dbRecord.getID();
            dbRecord.globals.logMsg(ErrorRecord, errMsg);
        }
    }

    // Deal with YOB
    if (DOB.isValid())
        YOB = static_cast<unsigned int>(DOB.year());
    else
    {
        if (YOB == 0)
            YOB = dbRecord.YOB;
        else
        {
            if ((YOB != dbRecord.YOB) && (dbRecord.YOB > 0))
            {
                PQString errMsg;
                errMsg << "Mismatched YOBs in 'best of' merge for: " << dbRecord.getID();
                dbRecord.globals.logMsg(ErrorRecord, errMsg);
            }
        }
    }

    // Deal with DOD
    if (!DOD.isValid())
        DOD = dbRecord.getDOD();
    else
    {
        if (dbRecord.getDOD().isValid() && (DOD != dbRecord.getDOD()))
        {
            PQString errMsg;
            errMsg << "Mismatched DODs in 'best of' merge for: " << dbRecord.getID();
            dbRecord.globals.logMsg(ErrorRecord, errMsg);
        }
    }

    // Deal with YOD
    if (DOD.isValid())
        YOD = static_cast<unsigned int>(DOD.year());
    else
    {
        if (YOD == 0)
            YOD = dbRecord.YOD;
        else
        {
            if ((YOD != dbRecord.YOD) && (dbRecord.YOD > 0))
            {
                PQString errMsg;
                errMsg << "Mismatched YODs in 'best of' merge for: " << dbRecord.getID();
                dbRecord.globals.logMsg(ErrorRecord, errMsg);
            }
        }
    }

    // Deal with Age at Death
    if (DOB.isValid() && DOD.isValid())
        ageAtDeath = static_cast<unsigned int>(elapse(DOB, DOD));
    else
    {
        if (ageAtDeath == 0)
            ageAtDeath = dbRecord.ageAtDeath;
        else
        {
            if ((ageAtDeath != dbRecord.ageAtDeath) && (dbRecord.ageAtDeath > 0))
            {
                PQString errMsg;
                errMsg << "Mismatched AgeAtDeath in 'best of' merge for: " << dbRecord.getID();
                dbRecord.globals.logMsg(ErrorRecord, errMsg);
            }
        }
    }

    // Deal with DOS
    if (!DOS.isValid())
        DOS = dbRecord.getDOS();
    else
    {
        if (dbRecord.getDOS().isValid() && (DOS != dbRecord.getDOS()))
        {
            PQString errMsg;
            errMsg << "Mismatched DOSs in 'best of' merge for: " << dbRecord.getID();
            dbRecord.globals.logMsg(ErrorRecord, errMsg);
        }
    }

    // Deal with min/max DOB
    setMinMaxDOB();
    if (dbRecord.minDOB > minDOB)
        minDOB = dbRecord.minDOB;
    if (dbRecord.maxDOB < maxDOB)
        maxDOB = dbRecord.maxDOB;

    sourceID.deceasedNumber = dbRecord.sourceID.deceasedNumber;

    // Deal with spouse's name
    if (spouseName.length() == 0)
        spouseName = dbRecord.spouseName;

    // Deal with Postal Code
    if (!postalCodeInfo.isValid())
        postalCodeInfo = dbRecord.postalCodeInfo;
    else
    {
        if (dbRecord.postalCodeInfo.isValid() && (postalCodeInfo != dbRecord.postalCodeInfo))
        {
            bool dbIsBetter = false;

            if ((globals.globalDr->getProvider() < 1000) && (dbRecord.getProvider() >= 1000) && (dbRecord.getProvider() != 1021) && (dbRecord.getProvider() != 2147))
                dbIsBetter = true;
            else
            {
                if ((globals.globalDr->getProvider() >= 3000) && (dbRecord.getProvider() >= 1000) && (dbRecord.getProvider() != 1021) && (dbRecord.getProvider() != 2147))
                    dbIsBetter = true;
                else
                {
                    if (((globals.globalDr->getProvider() == 1021) || (globals.globalDr->getProvider() == 2147)) && (dbRecord.getProvider() >= 1000) && (dbRecord.getProvider() < 3000))
                       dbIsBetter = true;
                }
            }

            if (dbIsBetter)
                postalCodeInfo = dbRecord.postalCodeInfo;
        }
    }
}

int dataRecord::runDateValidations()
{
    // Massages data for minor differences, flags other issues

    int diff;

    if (DOB.isValid())
    {
        if (!minDOB.isValid() || !maxDOB.isValid() || (DOB < minDOB) || (DOB > maxDOB))
            wi.dateFlag = 16;
    }
    else
    {
        if (DOD.isValid() && ((maxDOB.toJulianDay() - minDOB.toJulianDay()) < 365) && (ageAtDeath == 0))
        {
            int minAge, maxAge;
            maxAge = static_cast<int>(elapse(minDOB, DOD));
            minAge = static_cast<int>(elapse(maxDOB, DOD));
            if (minAge == maxAge)
                ageAtDeath = minAge;
        }
    }

    if ((wi.dateFlag != 16) && minDOB.isValid() && (YOB > 0))
    {
        diff = minDOB.year() - static_cast<int>(YOB);
        if (diff > 0)
        {
            if (diff < 4)
            {
                wi.dateFlag = 20;
                minDOB = QDate(static_cast<int>(YOB), 1, 1);
                YOB = 0;
                ageAtDeath = 0;
            }
            else
                wi.dateFlag = 15;
        }
    }

    if ((wi.dateFlag != 16) && maxDOB.isValid() && (YOB > 0))
    {
        diff = static_cast<int>(YOB) - maxDOB.year();
        if (diff > 0)
        {
            if (diff < 4)
            {
                wi.dateFlag = 20;
                maxDOB = QDate(static_cast<int>(YOB), 12, 31);
                YOB = 0;
                ageAtDeath = 0;
            }
            else
                wi.dateFlag = 15;
        }
    }

    if ((wi.dateFlag == 0) && (YOB > 0) && (YOD > 0) && (ageAtDeath > 0))
    {
        unsigned int maxAge = YOD - YOB;
        unsigned int minAge = maxAge - 1;
        if ((ageAtDeath != minAge) && (ageAtDeath != maxAge))
        {
            if (globals.globalDr->ageNextReference && (ageAtDeath == (minAge - 1)))
                ageAtDeath++;
            else
            {
                if (ageAtDeath < minAge)
                    ageAtDeath = 0;
                else
                    wi.dateFlag = 15;
            }
        }
    }

    // Fix dates where reasonable interpretations can be made
    if ((wi.dateFlag == 15) && !DOB.isValid() && (YOD > 0) && (ageAtDeath > 0))
    {
        // Assume an incorrect number was read is as ageAtDeath, resulting in higher DOB than YOB
        if (((maxDOB.toJulianDay() - minDOB.toJulianDay()) <= 365) && ((minDOB.year() - YOB) >= 10))
        {
            ageAtDeath = 0;
            minDOB = QDate(YOB, 1, 1);
            maxDOB = QDate(YOB, 12, 31);
            wi.dateFlag = 17;
        }
    }

    // Check for reversed dates
    if (DOB.isValid() && DOD.isValid())
    {
        if(DOB > DOD)
        {
            QDate temp(DOB);
            DOB = DOD;
            DOD = temp;

            minDOB = DOB;
            maxDOB = DOB;
            YOB = static_cast<unsigned int>(DOB.year());
            YOD = static_cast<unsigned int>(DOD.year());
            ageAtDeath = static_cast<unsigned int>(elapse(DOB, DOD));

            wi.dateFlag = 7;
        }
        else
        {
            unsigned int checkNum = static_cast<unsigned int>(elapse(DOB, DOD));
            if (checkNum != ageAtDeath)
            {
                // Attempt to fix occasional read error as part of UpdateDeceased
                if ((DOB.month() == 1) && (DOB.day() == 1) && (checkNum == (ageAtDeath + 1)))
                    DOB = QDate();
                else
                {
                    if (((ageAtDeath - checkNum) == 1) || ((checkNum - ageAtDeath) == 1))
                    {
                        ageAtDeath = checkNum;
                        wi.dateFlag = 9;
                    }
                    else
                        wi.dateFlag = 19;
                }

                ageAtDeathFullyCredible = false;
            }
        }
    }
    else
    {
        if (YOD < YOB)
            wi.dateFlag = 18;
    }

    if (DOD.isValid() && (DOD > globals.today))
        wi.dateFlag = 10;

    // Check Date of Service
    if (DOD.isValid() && DOS.isValid())
    {
        if(DOS <= DOD)
            DOS.setDate(0,0,0);
    }

    if (!DOD.isValid() && (ageAtDeath > 0) && (ageAtDeath <= 31))
        wi.ageFlag = 11;

    // Flag adjustments for initial runs to "UNDO" fix below for older records
    if ((YOB == 0) && (YOD == 0) && (ageAtDeath > 0))
    {
        YOD = globals.today.addDays(-7).year();
        maxDOB = QDate(globals.today.year() - ageAtDeath, globals.today.month(),globals.today.day());
        minDOB = QDate(globals.today.year() - ageAtDeath - 2, globals.today.month(),globals.today.day());
        wi.dateFlag = 1;
    }

    if ((YOB == YOD) && DOB.isValid() && !DOD.isValid() && (ageAtDeath > 31))
    {
        DOD = DOB;
        DOB = QDate(0,0,0);
        globals.globalDr->setMinMaxDOB();
    }

    // Legacy common issues
    if ((sourceID.provider == Legacy3000) && !DOB.isValid() && sourceID.publishDate.isValid() && (ageAtDeath > 0) && (static_cast<int>(YOD + ageAtDeath) == sourceID.publishDate.year()))
    {
        ageAtDeath = 0;
        minDOB = QDate(1875,1,1);
        maxDOB = QDate(YOD,12,31);
    }

    return wi.dateFlag;
}

void dataRecord::createFirstNameList(QList<QString> &resultList) const
{
    PQString name;
    QString tempName;

    name = getFirstName().getUnaccentedString();
    resultList.append(name.getString());
    if (name.isHyphenated())
    {
        resultList.append(name.preHyphen().getString());
        resultList.append(name.postHyphen().getString());
    }

    name = getFirstNameAKA1();
    if (name.getLength() > 0)
    {
        resultList.append(name.getUnaccentedString());
        if (name.isHyphenated())
        {
            resultList.append(name.preHyphen().getString());
            resultList.append(name.postHyphen().getString());
        }
    }

    name = getFirstNameAKA2();
    if (name.getLength() > 0)
    {
        resultList.append(name.getUnaccentedString());
        if (name.isHyphenated())
        {
            resultList.append(name.preHyphen().getString());
            resultList.append(name.postHyphen().getString());
        }
    }

    return;
}

void dataRecord::createMiddleNameList(QList<QString> &resultList) const
{
    OQStream middlenames(getMiddleNames());
    PQString name;

    while (!middlenames.isEOS())
    {
        name = middlenames.getWord();
        name.cleanUpEnds();
        if (name.getLength() > 0)
            resultList.append(name.getUnaccentedString());
    }

    return;
}

void dataRecord::setDatesLocked(bool dl)
{
    datesLocked = dl;
}

bool dataRecord::getDatesLocked() const
{
    return datesLocked;
}

void dataRecord::clearNames()
{
    familyName.clear();
    familyNameAlt1.clear();
    familyNameAlt2.clear();
    familyNameAlt3.clear();
    firstName.clear();
    firstNameAKA1.clear();
    firstNameAKA2.clear();
    middleNames.clear();
    prefix.clear();
    suffix.clear();
}

void dataRecord::copyNames(dataRecord &source)
{
    clearNames();

    familyName = source.familyName;
    familyNameAlt1 = source.familyNameAlt1;
    familyNameAlt2 = source.familyNameAlt2;
    familyNameAlt3 = source.familyNameAlt3;
    firstName= source.firstName;
    firstNameAKA1 = source.firstNameAKA1;
    firstNameAKA2 = source.firstNameAKA2;
    middleNames = source.middleNames;
    prefix = source.prefix;
    suffix = source.suffix;
}

int dataRecord::countNames()
{
    QList<QString> nameList;
    addToList(nameList, familyName);
    addToList(nameList, familyNameAlt1);
    addToList(nameList, familyNameAlt2);
    addToList(nameList, familyNameAlt3);
    addToList(nameList, firstName);
    addToList(nameList, firstNameAKA1);
    addToList(nameList, firstNameAKA2);
    if (middleNames.getLength() > 0)
        addToList(nameList, middleNames.getString().split(" "));

    return nameList.size();
}

void dataRecord::addToList(QList<QString> &list, PQString &name)
{
    if (name.getLength() > 0)
    {
        if (!list.contains(name.getString()))
            list.append(name.getString());
    }
}

void dataRecord::addToList(QList<QString> &list, QStringList names)
{
    QString name;

    while (names.size() > 0)
    {
        name = names.takeFirst();
        if (!list.contains(name))
            list.append(name);
    }
}

void dataRecord::sortFirstNames()
{
    OQString temp, firstName, alt1Name, alt2Name;
    PQString warningMessage;
    bool switchNames;
    PQString name;

    alt1Name = getFirstNameAKA1().lower();
    if (alt1Name.getLength() == 0)
        return;

    firstName = getFirstName().lower();
    switchNames = firstName.isInformalVersionOf(alt1Name.getString(), warningMessage);

    if (switchNames)
    {
        setFirstName(alt1Name, 1);
        setFirstName(firstName, 2);
    }
    else
    {
        alt2Name = getFirstNameAKA2().lower();
        if (alt2Name.getLength() == 0)
            return;

        switchNames = firstName.isInformalVersionOf(alt2Name.getString(), warningMessage);

        if (switchNames)
        {
            setFirstName(alt2Name, 1);
            setFirstName(firstName, 2);
            setFirstName(alt1Name, 3);
        }
        else
        {
            switchNames = alt2Name.isInformalVersionOf(firstName.getString(), warningMessage);

            if (switchNames)
            {
                setFirstName(alt2Name, 2);
                setFirstName(alt1Name, 3);
            }
            else
            {
                // Check if last two names are flipped
                switchNames = alt1Name.isInformalVersionOf(alt2Name.getString(), warningMessage);

                if (switchNames)
                {
                    setFirstName(alt2Name, 1);
                    setFirstName(alt1Name, 2);
                    setFirstName(firstName, 3);
                }
            }
        }
    }
}

void dataRecord::setPostalCode(POSTALCODE_INFO &pc)
{
    postalCodeInfo = pc;
}

bool dataRecord::setPostalCode(QString &pc)
{
    POSTALCODE_INFO newPCinfo;

    databaseSearches dbSearch;
    dbSearch.fillInPostalCodeInfo(&globals, newPCinfo, pc);
    if (newPCinfo.isValid())
        postalCodeInfo = newPCinfo;

    return postalCodeInfo.isValid();
}

void dataRecord::setRawFullName(const QString &fullName)
{
    rawFullName = fullName;
}

void dataRecord::setObitSnippet(const QString &snippet)
{
    obitSnippet = snippet;
}

void dataRecord::standardizeBadRecord()
{
    familyName = QString("BR_") + QString::number(sourceID.provider) + QString("_") + QString::number(sourceID.providerKey);
    familyNameAlt1.clear();
    familyNameAlt2.clear();
    familyNameAlt3.clear();
    firstName = familyName;
    firstNameAKA1.clear();
    firstNameAKA2.clear();
    middleNames.clear();
    middleNameUsedAsFirstName.clear();
    parentsLastName.clear();
    maidenNames.clear();
    suffix.clear();
    prefix.clear();
    gender = genderUnknown;
    workingGender = genderUnknown;
    DOB = QDate(1875,1,1);
    DOD = QDate(1875,1,1);
    YOB = 0;
    YOD = 0;
    deemedYOD = 0;
    ageAtDeath = 0;
    minDOB = QDate(1875,1,1);;
    maxDOB = QDate(1875,1,1);;
    DOBfullyCredible = false;
    DODfullyCredible = false;
    YOBfullyCredible = false;
    YODfullyCredible = false;
    ageAtDeathFullyCredible = false;
    datesLocked = true;
    // URL;
    // firstRecorded;
    // lastUpdated;
    language = language_unknown;
    // provider;
    // providerKey;
    // ID;
    // publishDate;
    // altPublishDate;
    // altPublishDate2;
    // DOS;
    // commentDate;
    cycle = 0;
    // previouslyLoaded;
}

int dataRecord::reorderLee()
{
    int result = 0;

    // Only make change in higher likelihood scenarios
    if ((middleNames.getLength() > 0) || (familyNameAlt2.getLength() == 0) || (gender == Male))
        return result;

    bool reorder = false;
    NAMESTATS nameStats;
    databaseSearches dbSearch;
    dbSearch.nameStatLookup(familyName.getString(), &globals, nameStats, Female);
    reorder = nameStats.isLikelyGivenName;

    if ((middleNames.getLength() == 0) && (familyNameAlt2.getLength() > 0) && ((familyName == PQString("Lee")) || reorder))
    {
        result = 1;

        middleNames = familyName;
        familyName = familyNameAlt1;
        familyNameAlt1 = familyNameAlt2;
        familyNameAlt2 = familyNameAlt3;
        familyNameAlt3.clear();
    }
    else
    {
        if ((middleNames.getLength() == 0) && (familyNameAlt2.getLength() > 0) && (familyNameAlt1 == PQString("Lee")))
        {
            result = 1;

            middleNames = familyName;
            familyNameAlt1 = familyNameAlt2;
            familyNameAlt2 = familyNameAlt3;
            familyNameAlt3.clear();
        }
    }

    return result;
}

bool dataRecord::removeExtraneousCommas()
{
    bool removed = false;
    PQString *pField = nullptr;
    QString field;

    for(int i = 0; i <= 7; i++)
    {
        switch(i)
        {
        case 0:
            pField = &familyName;
            break;

        case 1:
            pField = &familyNameAlt1;
            break;

        case 2:
            pField = &familyNameAlt2;
            break;

        case 3:
            pField = &familyNameAlt3;
            break;

        case 4:
            pField = &firstName;
            break;

        case 5:
            pField = &firstNameAKA1;
            break;

        case 6:
            pField = &firstNameAKA2;
            break;

        case 7:
            pField = &middleNames;
            break;
        }

        field = pField->getString();
        if (field.contains(","))
        {
            field.replace(",", "");
            *pField = field;
            removed = true;
            permanentErrorFlag = true;
        }
    }

    return removed;
}

void dataRecord::drDoubles(QList<dataRecord> &listDoubles, QString obitSnippet)
{
    if (wi.doubleMemorialFlag != 10)
        return;

    if (getProvider() != Legacy3000)
        return;

    dataRecord drCopy(*this);

    QString word;
    QList<QString> middles;
    getMiddleNameList(middles);
    bool matched = false;

    while (!matched && (middles.size() >= 2))
    {
        word = middles.takeFirst();
        if (word.toLower() == "and")
            matched = true;
    }

    if (matched)
    {
        word = middles.takeFirst();
        middleNames.clear();
        drCopy.middleNames.clear();
        setSpouseName(word);
        drCopy.clearFirstNames();
        drCopy.setFirstNames(word);
        drCopy.setSpouseName(getFirstName().getString());
    }

    if (gender == Male)
        drCopy.setGender(Female, true);
    else
    {
        if (gender == Female)
            drCopy.setGender(Male, true);
    }
    drCopy.sourceID.ID = PQString("9") + sourceID.ID;

    OQString tempString(obitSnippet);
    tempString.fixBasicErrors();
    obitSnippet = tempString.getString();
    QRegularExpression targetS;
    targetS.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);
    targetS.setPattern("\\((January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4}) - (January|February|March|April|May|June|July|August|September|October|November|December) (\\d+, \\d{4})\\)");
    QRegularExpressionMatch match;
    QRegularExpressionMatchIterator i = targetS.globalMatch(obitSnippet);
    bool adjusted = false;

    if (i.hasNext())
    {
        match = i.next();
        if (i.hasNext())  // second match
        {
            match = i.next();
            QString stringDOB = match.captured(1) + QString(" ") + match.captured(2);
            QString stringDOD = match.captured(3) + QString(" ") + match.captured(4);
            QDate dob = QDate::fromString(stringDOB, "MMMM d, yyyy");
            QDate dod = QDate::fromString(stringDOD, "MMMM d, yyyy");
            if (dob.isValid())
                drCopy.setDOB(dob, true);
            if (dod.isValid())
                drCopy.setDOD(dod, true);
            adjusted = true;
        }
    }

    if (!adjusted && DOB.isValid() && DOD.isValid() && (ageAtDeath <= 30))
    {
        drCopy.DOB = QDate();
        drCopy.minDOB = QDate(1875,1,1);
        drCopy.maxDOB = drCopy.DOD;
        drCopy.YOB = 0;
        drCopy.ageAtDeath = 0;

        DOD = DOB;
        DOB = QDate();
        minDOB = QDate(1875,1,1);
        maxDOB = DOD;
        YOB = 0;
        YOD = DOD.year();
        ageAtDeath = 0;
    }

    listDoubles.append(drCopy);
}


bool SOURCEID::operator ==(SOURCEID const& newSource) const
{
    return  (provider == newSource.provider) && (providerKey == newSource.providerKey) && (ID == newSource.ID) && (URL == newSource.URL) && (deceasedNumber == newSource.deceasedNumber);
}

void SOURCEID::clear()
{
    provider = static_cast<PROVIDER>(0);
    providerKey = 0;
    ID.clear();
    URL.clear();
    publishDate = QDate();

    deceasedNumber = 0;
}

