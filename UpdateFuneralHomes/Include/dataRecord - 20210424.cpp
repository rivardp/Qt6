// dataRecord.cpp

#include "dataRecord.h"


dataRecord::dataRecord() :  gender(genderUnknown), language(language_unknown), numUnmatched(0),
                            priorUnmatched(0), previouslyLoaded(0), neeEtAlEncountered(false), YOB(0), YOD(0), deemedYOD(0),
                            DOBfullyCredible(false), DODfullyCredible(false), YOBfullyCredible(false),
                            YODfullyCredible(false), ageAtDeathFullyCredible(false), datesLocked(false), ageNextReference(false),
                            maleHyphenated(false), singleYear(0), ageAtDeath(0), cycle(0), prov(provUnknown)
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

    // Fix issue with French obits where unstructured starts "nee le 11 octobre"
    if ((formattedName.left(2) == PQString("le")) && formattedName.isAlphaNumeric())
        return;

	formattedName = formattedName.proper();

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
        if ((firstName.getLength() > 0) && (firstName != name.proper()))
        {
            PQString errMsg;
            errMsg << "Encountered inconsistent First Names in reading file: ";
            errMsg << getURL();
            globals.logMsg(ErrorRecord, errMsg);
        }
        else
            firstName = name.proper();

        break;

    case 1:
        firstName = name.proper();
        break;

    case 2:
        firstNameAKA1 = name.proper();
        break;

    case 3:
        firstNameAKA2 = name.proper();
        break;

    default:
        // Do nothing
        break;
    }
}

void dataRecord::setFirstNames(const PQString &name, const bool forceLoad)
{
    if (!forceLoad && pureNickNameLookup(name.getString(), &globals))
            return;

    if ((firstName.getLength() == 0) || (firstName.getUnaccentedString() == name.proper().getUnaccentedString()))
        firstName = name.proper();
    else
    {
        if ((firstNameAKA1.getLength() == 0) || (firstNameAKA1.getUnaccentedString() == name.proper().getUnaccentedString()))
            firstNameAKA1 = name.proper();
        else
        {
            if ((firstNameAKA2.getLength() == 0) || (firstNameAKA2.getUnaccentedString() == name.proper().getUnaccentedString()))
                firstNameAKA2 = name.proper();
            else
                attemptToMakeRoomFor(name);
        }
    }
}

void dataRecord::attemptToMakeRoomFor(const PQString &name)
{
    if (pureNickNameLookup(name.getString(), &globals))
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

    modifiedName = name;
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
                middleNames += listOfNames.at(i);
                middleNames += space;
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
    QList<QString> nameList = names.split(QString("-"), QString::SkipEmptyParts, Qt::CaseSensitive);

    for (int i = 0; i < nameList.size(); i++)
    {
        OQString cleanName(nameList.at(i));
        alreadyThere = false;

        for (int i = 0; i < maidenNames.size(); i++)
        {
            if (cleanName == maidenNames.at(i))
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
		prefix = pfx;
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
		suffix = sfx;
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
        globals.globalDr->wi.genderFlag = 1;
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

void dataRecord::setProv(PROVINCE province)
{
    prov = province;
}

void dataRecord::clearDOB()
{
    QDate newDOB;
    DOB = newDOB;
    DOBfullyCredible = false;
}

void dataRecord::setDOB(const QDate &dob, const bool forceOverride, const bool fullyCredible)
{
    bool error, dateError, rangeErrorHigh, rangeErrorLow;
    PQString errMsg;

    if (datesLocked)
        return;

    if (dob.isValid() && (dob <= globals.today))
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
                        DOBfullyCredible = false;
                        YOBfullyCredible = false;
                    }
                }
                else
                {
                    if ((dob.year() == static_cast<int>(YOD)) && (dob == QDate(YOD, DOB.month(), DOB.day())))
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
                        maxDOB = globals.globalDr->getPublishDate();
                    if (globals.globalDr->getCommentDate().isValid() && (globals.globalDr->getCommentDate() < maxDOB))
                        maxDOB = globals.globalDr->getCommentDate();
                    if (globals.globalDr->getDOS().isValid() && (globals.globalDr->getDOS() < maxDOB))
                        maxDOB = globals.globalDr->getDOS();
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
}

void dataRecord::setDOD(const QDate &dod, const bool forceOverride, const bool fullyCredible)
{
	bool error = false;
    PQString errMsg;

	if (dod.isValid())
	{
		// Match against existing data if it exists
        error = DOD.isValid() && ((DOD != dod) || (dod > globals.today)) ;
        if (!error || forceOverride)
        {
			DOD = dod;
            YOD = static_cast<unsigned int>(dod.year());
            DODfullyCredible = fullyCredible;
            YODfullyCredible = fullyCredible;
            setMinMaxDOB();
            if (DOB.isValid())
                setAgeAtDeath(static_cast<int>(elapse(DOB, DOD)));
        }

        if (error && !forceOverride)
            wi.dateFlag = 3;
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
        globals.globalDr->wi.dateFlag = 3;
    }
}

void dataRecord::setDOBandDOD(const DATES &dates, const bool forceOverride)
{
    bool problem = false;

    if (datesLocked)
        return;

	if (dates.potentialDOB.isValid() && dates.potentialDOD.isValid())
	{
		// Match against existing data if it exists
        problem = (DOB.isValid() && (DOB != dates.potentialDOB)) || (DOD.isValid() && (DOD != dates.potentialDOD));

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
            globals.globalDr->wi.dateFlag = 1;
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
                    globals.globalDr->wi.dateFlag = 2;
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
                globals.globalDr->wi.dateFlag = 3;
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

void dataRecord::setAgeAtDeath(const unsigned int num, const bool fullyCredible, const bool override)
{
    bool consistent;

    if (datesLocked)
        return;

    if (ageAtDeathFullyCredible && !override)
        return;

    if (DOB.isValid() && DOD.isValid())
    {
        unsigned int tempAgeAtDeath = static_cast<unsigned int>(elapse(DOB,DOD));

        if (DOBfullyCredible && DODfullyCredible)
        {
            ageAtDeath = tempAgeAtDeath;
            ageAtDeathFullyCredible = true;
        }
        else
        {
            if ((ageAtDeath > 0) && (num != ageAtDeath) && (num != tempAgeAtDeath))
                wi.dateFlag = 9;

            ageAtDeath = tempAgeAtDeath;
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
	if ((URL.getLength() > 0) && (URL != url))
	{
        PQString errMsg;
        errMsg << "Encountered inconsistent URLs in adding: ";
        errMsg << url.getString();
        errMsg << "  to : ";
        errMsg << getURL();
        globals.logMsg(ErrorRecord, errMsg);
	}
	else
		URL = url;
}

void dataRecord::setAltURL(const PQString &url)
{
    if ((altURL.getLength() > 0) && (altURL != url))
    {
        PQString errMsg;
        errMsg << "Encountered inconsistent altURLs in adding: ";
        errMsg << url.getString();
        errMsg << "  to : ";
        errMsg << getURL();
        globals.logMsg(ErrorRecord, errMsg);
    }

    // Load it either way however in case we have reached a third URL
    altURL = url;
}

void dataRecord::setAltURL2(const PQString &url)
{
    if ((altURL2.getLength() > 0) && (altURL2 != url))
    {
        PQString errMsg;
        errMsg << "Encountered inconsistent altURLs in adding: ";
        errMsg << url.getString();
        errMsg << "  to : ";
        errMsg << getURL();
        globals.logMsg(ErrorRecord, errMsg);
    }

    // Load it either way however in case we have reached a fourth URL
    altURL2 = url;
}

void dataRecord::setLanguage(const LANGUAGE lang)
{
    if (language == language_unknown)
        language = lang;
    else
    {
        if (lang == language_unknown);
            // Do nothing
        else
        {
            if (language != lang)
            {
                PQString errMsg;
                errMsg << "Encountered inconsistent language determinations for: ";
                errMsg << getURL();
                //globals.logMsg(ErrorRecord, errMsg);

                language = lang;
            }
        }
    }
}

void dataRecord::setAlternates(const NAMEINFO &nameInfo, bool bestOf)
{
    QStringList nonHyphenatedNames;
    PQString name, errMsg;
    PQString space(" ");
    NAMETYPE type;

    name = nameInfo.name;
    type = nameInfo.type;

    if ((name.getLength() == 0) || (name == PQString("Retired")) || (name == PQString("Ret'd")) || (name == PQString("Retd")))
		return;

    LANGUAGE lang = getLanguage();
    GENDER gender = getGender();
    if (gender == genderUnknown)
        gender = getWorkingGender();

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
                    name = tempNameB;
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
        if (pureNickNameLookup(name.getString(), &globals))
             return;

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
            double unisex = genderLookup(globals.globalDr, &globals);
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
                    if(!tempPrefix.isTitle(lang, gender))
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
                if (!matched)
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
                    wi.dateFlag = 2;
                }

                YOB = yob;
                minDOB = minDate;
                maxDOB = maxDate;
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
            wi.dateFlag = 2;
        }
    }

    if (errMsg.getLength() > 0)
    {
        errMsg << getURL();
        globals.logMsg(ErrorRecord, errMsg);
        globals.globalDr->wi.dateFlag = 2;
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

void dataRecord::setDeceasedNumber(const unsigned int dn)
{
    deceasedNumber = dn;
}

void dataRecord::setCycle(const unsigned int num)
{
    cycle = num;
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

void dataRecord::setPublishDate(const QDate &date)
{
    publishDate = date;
}

void dataRecord::setAltPublishDate(const QDate &date)
{
    altPublishDate = date;
}

void dataRecord::setAltPublishDate2(const QDate &date)
{
    altPublishDate2 = date;
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

void dataRecord::setPotentialFirstName(const QString name)
{
    potentialGivenName = name;
}

void dataRecord::setSpouseName(const QString name)
{
    spouseName = name;
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

QString dataRecord::getURL() const
{
    return URL.getString();
}

QString dataRecord::getAltURL() const
{
    return altURL.getString();
}

QString dataRecord::getAltURL2() const
{
    return altURL2.getString();
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
    return prov;
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

    name = getFirstName().getString();
    if ((name.size() > 0) && givenNameLookup(name, gv, genderUnknown))
        resultList.append(name);

    name = getFirstNameAKA1().getString();
    if ((name.size() > 0) && givenNameLookup(name, gv, genderUnknown))
        resultList.append(name);

    name = getFirstNameAKA2().getString();
    if ((name.size() > 0) && givenNameLookup(name, gv, genderUnknown))
        resultList.append(name);

    if(getMiddleNameList(middleNameList))
    {
        while (middleNameList.size() > 0)
        {
            name = middleNameList.takeFirst();
            if (!resultList.contains(name) && givenNameLookup(name, gv, genderUnknown))
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

    if (publishDate.isValid() && (publishDate > QDate(1900,1,1)))
    {
        if (commentDate.isValid() && (commentDate < publishDate))
            pubDate = commentDate;
        else
            pubDate = publishDate;
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

QString dataRecord::getPotentialFirstName() const
{
    return potentialGivenName;
}

QString dataRecord::getSpouseName() const
{
    return spouseName;
}

void dataRecord::xport(QString filename, int extraOptParam)
{
    QString comma(",");
    PQString outputFile(filename);

    globals.output = new QFile((globals.batchDirectory + PQString("\\") + outputFile).getString());
    if(globals.output->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        QTextStream outputStream(globals.output);
        outputStream.setCodec("UTF-8");

        // Before exporting, check for problematic double quotes in names
        // checkNames deletes any double quotes found
        if (checkNames())
        {
            PQString errMsg;
            errMsg << "Unexpected double quotes found in:  " << URL.getString();
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
        switch (gender)
        {
        case Male:
            outputStream << "Male" << comma;
            break;

        case Female:
            outputStream << "Female" << comma;
            break;

        default:
            outputStream << "gender unknown" << comma;
        }
        outputStream << DOB.toString("yyyyMMdd") << comma;
        outputStream << DOD.toString("yyyyMMdd") << comma;
        outputStream << minDOB.toString("yyyyMMdd") << comma;
        outputStream << maxDOB.toString("yyyyMMdd") << comma;
        outputStream << YOB << comma;
        outputStream << YOD << comma;
        outputStream << ageAtDeath << comma;
        outputStream << URL.getString() << comma;
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

        default:
            outputStream << "Language unknown" << comma;
        }

        outputStream << provider << comma << providerKey << comma << ID.getString() << comma;
        outputStream << DOS.toString("yyyyMMdd") << comma << publishDate.toString("yyyyMMdd") << comma;
        outputStream << wi.ageFlag << comma << wi.dateFlag << comma << wi.genderFlag << comma << wi.nameReversalFlag << comma << wi.nameFlagGeneral << comma;
        outputStream << wi.doubleMemorialFlag << comma << wi.outstandingErrMsg << comma;
        if (getMaleHyphenated())
            outputStream << "1" << comma;
        else
            outputStream << "0" << comma;
        outputStream << wi.memorialFlag << comma;
        outputStream << wi.validated << comma ;
        outputStream << spouseName << comma;
        outputStream << wi.checkParentsName << comma << wi.checkInclName << comma << wi.checkExclName << comma << wi.confirmTreatmentName << comma << wi.confirmMiddleName << comma ;
        outputStream << priorUnmatched << comma << previouslyLoaded << comma;
        outputStream << extraOptParam << comma;
        outputStream << endl;

        globals.output->close();
    }
}

bool dataRecord::isAFirstName(const PQString &name) const
{
    if ((name.getLength() == 0) || (providerKey == 0))  // second condition captures undefined dr record
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
    if ((name.getLength() == 0) || (providerKey == 0))  // second conditions addresses undefined dr record
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
    if ((name.getLength() == 0) || (providerKey == 0))  // second conditions addresses undefined dr record
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
    if ((name.getLength() == 0) || (providerKey == 0))  // second conditions addresses undefined dr record
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
    if ((name.getLength() == 0) || (providerKey == 0))  // second conditions addresses undefined dr record
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
                    target = target;
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
 prov = provUnknown;
 URL.clear();
 altURL.clear();
 altURL2.clear();
 firstRecorded.setDate(0,0,0);
 lastUpdated.setDate(0,0,0);
 language = language_unknown;
 provider = Undefined;
 altProvider = Undefined;
 altProvider2 = Undefined;
 providerKey = 0;
 altProviderKey = 0;
 altProviderKey2 = 0;
 ID.clear();
 altID.clear();
 altID2.clear();
 publishDate.setDate(0,0,0);
 altPublishDate.setDate(0,0,0);
 altPublishDate2.setDate(0,0,0);
 DOS.setDate(0,0,0);
 commentDate.setDate(0,0,0);
 deceasedNumber = 0;
 numUnmatched = 0;
 priorUnmatched = 0;
 previouslyLoaded = 0;
 title.clear();
 titleKey.clear();
 neeEtAlEncountered = false;
 ageNextReference = false;
 maleHyphenated = false;
 singleYear = 0;
 potentialGivenName.clear();
 spouseName.clear();
 wi.clear();
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

dataRecord& dataRecord::operator= (const dataRecord &rhs)
{
    this->clear();

    familyName = rhs.familyName;
    familyNameAlt1 = rhs.familyNameAlt1;
    familyNameAlt2 = rhs.familyNameAlt2;
    familyNameAlt3 = rhs.familyNameAlt3;
    firstName = rhs.firstName;
    firstNameAKA1 = rhs.firstNameAKA1;
    firstNameAKA2 = rhs.firstNameAKA2;
    middleNames = rhs.middleNames;
    middleNameUsedAsFirstName = rhs.middleNameUsedAsFirstName;
    parentsLastName = rhs.middleNameUsedAsFirstName;
    maidenNames = rhs.maidenNames;
    suffix = rhs.suffix;
    prefix = rhs.prefix;
    gender = rhs.gender;
    workingGender = rhs.workingGender;
    DOB = rhs.DOB;
    DOD = rhs.DOD;
    YOB = rhs.YOB;
    YOD = rhs.YOD;
    deemedYOD = rhs.deemedYOD;
    ageAtDeath = rhs.ageAtDeath;
    minDOB = rhs.minDOB;
    maxDOB = rhs.maxDOB;
    DOBfullyCredible = rhs.DOBfullyCredible;
    DODfullyCredible = rhs.DODfullyCredible;
    YOBfullyCredible = rhs.YOBfullyCredible;
    YODfullyCredible = rhs.YODfullyCredible;
    ageAtDeathFullyCredible = rhs.ageAtDeathFullyCredible;
    datesLocked = rhs.datesLocked;
    prov = rhs.prov;
    URL = rhs.URL;
    altURL = rhs.altURL;
    altURL2 = rhs.altURL2;
    firstRecorded = rhs.firstRecorded;
    lastUpdated = rhs.lastUpdated;
    language = rhs.language;
    provider = rhs.provider;
    altProvider = rhs.altProvider;
    altProvider2 = rhs.altProvider2;
    providerKey = rhs.providerKey;
    altProviderKey = rhs.altProviderKey;
    altProviderKey2 = rhs.altProviderKey2;
    ID = rhs.ID;
    altID = rhs.altID;
    altID2 = rhs.altID2;
    publishDate = rhs.publishDate;
    altPublishDate = rhs.altPublishDate;
    altPublishDate2 = rhs.altPublishDate2;
    DOS = rhs.DOS;
    commentDate = rhs.commentDate;
    deceasedNumber = rhs.deceasedNumber;
    cycle = rhs.cycle;
    numUnmatched = rhs.numUnmatched;
    priorUnmatched = rhs.priorUnmatched;
    previouslyLoaded = rhs.previouslyLoaded;
    title = rhs.title;
    titleKey = rhs.titleKey;
    neeEtAlEncountered = rhs.neeEtAlEncountered;
    ageNextReference = rhs.ageNextReference;
    maleHyphenated = rhs.maleHyphenated;
    spouseName = rhs.spouseName;

    globals = rhs.globals;

    return *this;
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

void dataRecord::setProvider(const PROVIDER &pvdr)
{
    provider = pvdr;
}

void dataRecord::setAltProvider(const PROVIDER &pvdr)
{
    altProvider = pvdr;
}

void dataRecord::setAltProvider2(const PROVIDER &pvdr)
{
    altProvider2 = pvdr;
}

void dataRecord::setProviderKey(const unsigned int pk)
{
    providerKey = pk;
}

void dataRecord::setAltProviderKey(const unsigned int pk)
{
    altProviderKey = pk;
}

void dataRecord::setAltProviderKey2(const unsigned int pk)
{
    altProviderKey2 = pk;
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

void dataRecord::setID(const PQString &id)
{
    ID = id;
}

void dataRecord::setAltID(const PQString &id)
{
    altID = id;
}

void dataRecord::setAltID2(const PQString &id)
{
    altID2 = id;
}

PROVIDER dataRecord::getProvider() const
{
    return provider;
}

PROVIDER dataRecord::getAltProvider() const
{
    return altProvider;
}

PROVIDER dataRecord::getAltProvider2() const
{
    return altProvider2;
}

unsigned int dataRecord::getProviderKey() const
{
    return providerKey;
}

unsigned int dataRecord::getAltProviderKey() const
{
    return altProviderKey;
}

unsigned int dataRecord::getAltProviderKey2() const
{
    return altProviderKey2;
}

PQString dataRecord::getID() const
{
    return ID;
}

PQString dataRecord::getAltID() const
{
    return altID;
}

PQString dataRecord::getAltID2() const
{
    return altID2;
}

QDate dataRecord::getFirstRecorded() const
{
    return firstRecorded;
}

QDate dataRecord::getPublishDate() const
{
    return publishDate;
}

QDate dataRecord::getAltPublishDate() const
{
    return altPublishDate;
}

QDate dataRecord::getAltPublishDate2() const
{
    return altPublishDate2;
}

QDate dataRecord::getDOS() const
{
    return DOS;
}

QDate dataRecord::getCommentDate() const
{
    return commentDate;
}

unsigned int dataRecord::getDeceasedNumber() const
{
    return deceasedNumber;
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

void dataRecord::updateToBestOf(dataRecord &newSource)
{
    PQString tempName;
    NAMEINFO nameInfo;
    QList<NAMEINFO> nameInfoList;

    // Deal with first names
    nameInfoList.clear();
    nameInfo.type = ntFirst;
    nameInfo.name = newSource.getFirstName();
    nameInfoList.append(nameInfo);

    tempName = newSource.getFirstNameAKA1();
    if (tempName.getLength() > 0)
    {
        nameInfo.name = tempName;
        nameInfoList.append(nameInfo);

        tempName = newSource.getFirstNameAKA2();
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
            matched = (firstName == nameInfoList.at(i).name);
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
            matched = (firstNameAKA1 == nameInfoList.at(i).name);
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
            matched = (firstNameAKA2 == nameInfoList.at(i).name);
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
        if (pureNickNameLookup(nameInfoList.at(i).name.getString(), &newSource.globals))
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
                if (OQname.isInformalVersionOf(nameInfoList.at(j).name.getString(), errMsg))
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
            if (pureNickNameLookup(nameInfoList.at(i).name.getString(), &globals))
            {
                nameInfo = nameInfoList.takeAt(i);
                nameInfoList.append(nameInfo);
            }
        }
    }

    if (nameInfoList.size() > 3)
    {
        PQString errMsg;
        errMsg << "Loss of first name in 'best of' merge for: " << newSource.getID();
        newSource.globals.logMsg(ErrorRecord, errMsg);

        while (nameInfoList.size() > 3)
            nameInfoList.removeLast();
    }

    clearFirstNames();
    setAlternates(nameInfoList, true);


    // Deal with easy case of Middlenames - Will automatically just add new names
    nameInfoList.clear();
    nameInfo.clear();
    nameInfo.type = ntMiddle;
    OQStream tempStream(newSource.getMiddleNames());
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

    nameInfo.name = newSource.getLastName();
    nameInfoList.append(nameInfo);

    tempName = newSource.getLastNameAlt1();
    if (tempName.getLength() > 0)
    {
        nameInfo.name = tempName;
        nameInfoList.append(nameInfo);

        tempName = newSource.getLastNameAlt2();
        if (tempName.getLength() > 0)
        {
            nameInfo.name = tempName;
            nameInfoList.append(nameInfo);

            tempName = newSource.getLastNameAlt3();
            if (tempName.getLength() > 0)
            {
                nameInfo.name = tempName;
                nameInfoList.append(nameInfo);
            }
        }
    }
    setAlternates(nameInfoList, true);

    // Deal with prefixes
    tempName = newSource.getPrefix();
    if (tempName.getLength() > 0)
    {
        nameInfoList.clear();
        nameInfo.clear();
        nameInfo.type = ntSuffix;

        nameInfo.name = tempName;
        nameInfoList.append(nameInfo);

        setAlternates(nameInfoList, true);
    }

    // Deal with suffixes
    tempName = newSource.getSuffix();
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
        gender = newSource.getGender();
    else
    {
        if ((newSource.getGender() != genderUnknown) && (gender != newSource.getGender()))
        {
            PQString errMsg;
            errMsg << "Mismatched genders in 'best of' merge for: " << newSource.getID();
            newSource.globals.logMsg(ErrorRecord, errMsg);
        }
    }

    // Deal with DOB
    if (!DOB.isValid())
        DOB = newSource.getDOB();
    else
    {
        if (newSource.getDOB().isValid() && (DOB != newSource.getDOB()))
        {
            PQString errMsg;
            errMsg << "Mismatched DOBs in 'best of' merge for: " << newSource.getID();
            newSource.globals.logMsg(ErrorRecord, errMsg);
        }
    }

    // Deal with YOB
    if (DOB.isValid())
        YOB = static_cast<unsigned int>(DOB.year());
    else
    {
        if (YOB == 0)
            YOB = newSource.YOB;
        else
        {
            if (YOB != newSource.YOB)
            {
                PQString errMsg;
                errMsg << "Mismatched YOBs in 'best of' merge for: " << newSource.getID();
                newSource.globals.logMsg(ErrorRecord, errMsg);
            }
        }
    }

    // Deal with DOD
    if (!DOD.isValid())
        DOD = newSource.getDOD();
    else
    {
        if (newSource.getDOD().isValid() && (DOD != newSource.getDOD()))
        {
            PQString errMsg;
            errMsg << "Mismatched DODs in 'best of' merge for: " << newSource.getID();
            newSource.globals.logMsg(ErrorRecord, errMsg);
        }
    }

    // Deal with YOD
    if (DOD.isValid())
        YOD = static_cast<unsigned int>(DOD.year());
    else
    {
        if (YOD == 0)
            YOD = newSource.YOD;
        else
        {
            if (YOD != newSource.YOD)
            {
                PQString errMsg;
                errMsg << "Mismatched YODs in 'best of' merge for: " << newSource.getID();
                newSource.globals.logMsg(ErrorRecord, errMsg);
            }
        }
    }

    // Deal with Age at Death
    if (DOB.isValid() && DOD.isValid())
        ageAtDeath = static_cast<unsigned int>(elapse(DOB, DOD));
    else
    {
        if (ageAtDeath == 0)
            ageAtDeath = newSource.ageAtDeath;
        else
        {
            if (ageAtDeath != newSource.ageAtDeath)
            {
                PQString errMsg;
                errMsg << "Mismatched AgeAtDeath in 'best of' merge for: " << newSource.getID();
                newSource.globals.logMsg(ErrorRecord, errMsg);
            }
        }
    }

    // Deal with DOS
    if (!DOS.isValid())
        DOS = newSource.getDOS();
    else
    {
        if (newSource.getDOS().isValid() && (DOS != newSource.getDOS()))
        {
            PQString errMsg;
            errMsg << "Mismatched DOSs in 'best of' merge for: " << newSource.getID();
            newSource.globals.logMsg(ErrorRecord, errMsg);
        }
    }

    // Deal with min/max DOB
    setMinMaxDOB();
    if (newSource.minDOB > minDOB)
        minDOB = newSource.minDOB;
    if (newSource.maxDOB < maxDOB)
        maxDOB = newSource.maxDOB;

    // Set up for altUrl and altID load on forced overrides
    SOURCEID sourceID, tempSourceID;
    QList<SOURCEID> matchedSourceID, unmatchedSourceID;

    sourceID.provider = provider;
    sourceID.providerKey = providerKey;
    sourceID.URL = URL;
    sourceID.ID = ID.getString();
    sourceID.publishDate = publishDate;

    if (newSource.provider > 0)
    {
        tempSourceID.provider = newSource.provider;
        tempSourceID.providerKey = newSource.providerKey;
        tempSourceID.URL = newSource.URL;
        tempSourceID.ID = newSource.ID.getString();
        tempSourceID.publishDate = newSource.publishDate;

        if (tempSourceID == sourceID)
            matchedSourceID.append(tempSourceID);
        else
            unmatchedSourceID.append(tempSourceID);
    }

    if (newSource.altProvider > 0)
    {
        tempSourceID.provider = newSource.altProvider;
        tempSourceID.providerKey = newSource.altProviderKey;
        tempSourceID.URL = newSource.altURL;
        tempSourceID.ID = newSource.altID.getString();
        tempSourceID.publishDate = newSource.altPublishDate;

        if (tempSourceID == sourceID)
            matchedSourceID.append(tempSourceID);
        else
            unmatchedSourceID.append(tempSourceID);
    }

    if (newSource.altProvider2 > 0)
    {
        tempSourceID.provider = newSource.altProvider2;
        tempSourceID.providerKey = newSource.altProviderKey2;
        tempSourceID.URL = newSource.altURL2;
        tempSourceID.ID = newSource.altID2.getString();
        tempSourceID.publishDate = newSource.altPublishDate2;

        if (tempSourceID == sourceID)
            matchedSourceID.append(tempSourceID);
        else
            unmatchedSourceID.append(tempSourceID);
    }

    unsigned int loaded = 1; // which is the current new record read in from CSV

    while ((loaded < 3) && (unmatchedSourceID.size() > 0))
    {
       tempSourceID = unmatchedSourceID.takeFirst();

       switch (loaded)
       {
       case 1:
           altProvider = tempSourceID.provider;
           altProviderKey = tempSourceID.providerKey;
           altURL = tempSourceID.URL;
           altID = tempSourceID.ID.getString();
           altPublishDate = tempSourceID.publishDate;
           break;

       case 2:
           altProvider2 = tempSourceID.provider;
           altProviderKey2 = tempSourceID.providerKey;
           altURL2 = tempSourceID.URL;
           altID2 = tempSourceID.ID.getString();
           altPublishDate2 = tempSourceID.publishDate;
           break;
       }

       loaded++;
    }

    // Deal with spouse's name
    if (spouseName.length() == 0)
        spouseName = newSource.spouseName;

    // Set identification
    deceasedNumber = newSource.deceasedNumber;
    cycle = newSource.cycle;
}


int dataRecord::runDateValidations()
{
    // Massages data for minor differences, flags other issues

    int diff;

    if (DOB.isValid())
    {
        if (!minDOB.isValid() || !maxDOB.isValid() || (DOB < minDOB) || (DOB > maxDOB))
            wi.dateFlag = 6;
    }

    if ((wi.dateFlag != 6) && minDOB.isValid() && (YOB > 0))
    {
        diff = minDOB.year() - static_cast<int>(YOB);
        if (diff > 0)
        {
            if (diff < 3)
            {
                wi.dateFlag = 4;
                minDOB = QDate(static_cast<int>(YOB), 1, 1);
                YOB = 0;
                ageAtDeath = 0;
            }
            else
                wi.dateFlag = 5;
        }
    }

    if ((wi.dateFlag != 6) && maxDOB.isValid() && (YOB > 0))
    {
        diff = static_cast<int>(YOB) - maxDOB.year();
        if (diff > 0)
        {
            if (diff < 3)
            {
                wi.dateFlag = 4;
                maxDOB = QDate(static_cast<int>(YOB), 12, 31);
                YOB = 0;
                ageAtDeath = 0;
            }
            else
                wi.dateFlag = 5;
        }
    }

    if ((wi.dateFlag == 0) && (YOB > 0) && (YOD > 0) && (ageAtDeath > 0))
    {
        unsigned int maxAge = YOD - YOB;
        unsigned int minAge = maxAge - 1;
        if ((ageAtDeath != minAge) && (ageAtDeath != maxAge))
            wi.dateFlag = 5;
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
                wi.dateFlag = 9;
        }
    }
    else
    {
        if (YOD < YOB)
            wi.dateFlag = 8;
    }

    if (DOD.isValid() && (DOD > globals.today))
        wi.dateFlag = 10;

    // Check Date of Service
    if (DOD.isValid() && DOS.isValid())
    {
        if(DOS <= DOD)
            DOS.setDate(0,0,0);
    }

    return wi.dateFlag;
}

void dataRecord::createFirstNameList(QList<QString> &resultList) const
{
    PQString name;

    resultList.append(getFirstName().getUnaccentedString());
    name = getFirstNameAKA1();
    if (name.getLength() > 0)
        resultList.append(name.getUnaccentedString());
    name = getFirstNameAKA2();
    if (name.getLength() > 0)
        resultList.append(name.getUnaccentedString());

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

void dataRecord::standardizeBadRecord()
{
    familyName = QString("BR_") + QString::number(provider) + QString("_") + QString::number(providerKey);
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
    altURL.clear();
    altURL2.clear();
    // firstRecorded;
    // lastUpdated;
    language = language_unknown;
    // provider;
    // providerKey;
    altProvider = (PROVIDER)0;
    altProviderKey = 0;
    altProvider2 = (PROVIDER)0;
    altProviderKey2 = 0;
    // ID;
    altID.clear();
    altID2.clear();
    // publishDate;
    // altPublishDate;
    // altPublishDate2;
    // DOS;
    // commentDate;
    cycle = 0;
    // previouslyLoaded;
}

bool SOURCEID::operator ==(SOURCEID const& newSource) const
{
    return  (provider == newSource.provider) && (providerKey == newSource.providerKey) && (URL == newSource.URL) && (ID == newSource.ID);
}

SOURCEID& SOURCEID::operator =(SOURCEID newSource)
{
    provider = newSource.provider;
    providerKey = newSource.providerKey;
    URL = newSource.URL;
    ID = newSource.ID.getString();
    publishDate = newSource.publishDate;

    return *this;
}

