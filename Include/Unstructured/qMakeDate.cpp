// qMakeDate.cpp

#include "qMakeDate.h"

QDate MAKEDATE::today = QDate();

MAKEDATE::MAKEDATE() : isValid(false), isFinal(false), analyzeFurther(false), format(DATEORDER(doNULL)),
    conviction(CONVICTION(0)), totalStored(0), numChar1(0), numChar2(0), numChar3(0),
    potentialYMD(false), potentialDMY(false), potentialMDY(false), currentNumber(1)
{
	// num1, num2, and num3 three are intentionally left undefined to catch logic errors
}

MAKEDATE::~MAKEDATE()
{

}

MAKEDATE::MAKEDATE(int num1, int num2, int num3, int num4, int num5, int num6, QDate date)
{
    setToday(date);

    DATEORDER dateOrder;
    int yyyy1, yyyy2;
    int yearPosition;
    bool yearSet,definitive;
    yearSet = false;

    // Start with year
    if ((num3 > 1900) && (num6 > 1900) && (num3 <= today.year()) && (num6 <= today.year()))
    {
        yyyy1 = num3;
        yyyy2 = num6;
        yearSet = true;
        yearPosition = 3;
    }
    else
    {
        if (num6 == (today.year() - 2000))
        {
            yyyy2 = num6 + 2000;
            if (num3 > num6)
                yyyy1 = num3 + 1900;
            else
                yyyy1 = num3 + 2000;
            yearSet = true;
            yearPosition = 3;
        }
    }

    if (!yearSet)
    {
        if ((num1 > 1900) && (num4 > 1900) && (num1 <= today.year()) && (num4 <= today.year()))
        {
            yyyy1 = num1;
            yyyy2 = num4;
            yearSet = true;
            yearPosition = 1;
        }
        else
        {
            if (num4 == (today.year() - 2000))
            {
                yyyy2 = num4 + 2000;
                if (num1 > num4)
                    yyyy1 = num1 + 1900;
                else
                    yyyy1 = num4 + 2000;
                yearSet = true;
                yearPosition = 1;
            }
        }
    }

    if (!yearSet)
        return;

    if (yearPosition == 3)
    {
        if (!((num1 > 0) && (num2 > 0) && (num4 > 0) && (num5 > 0) && (num1 <= 31) && (num2 <= 31) && (num4 <= 31) && (num5 <= 31)))
            return;
    }
    else
    {
        // yearPosition == 1
        if (!((num2 > 0) && (num3 > 0) && (num5 > 0) && (num6 > 0) && (num2 <= 31) && (num3 <= 31) && (num5 <= 31) && (num6 <= 31)))
            return;
    }

    definitive = false;

    if (yearPosition == 3)
    {
        if ((num1 <= 12) && (num4 <= 12) && ((num2 > 12) || (num5 > 12)))
        {
            dateOrder = doMDY;
            definitive = true;
        }
        else
        {
            if ((num2 <= 12) && (num5 <= 12) && ((num1 > 12) || (num4 > 12)))
            {
                dateOrder = doDMY;
                definitive = true;
            }
        }
    }
    else
    {
        // yearPosition == 1
        if ((num2 <= 12) && (num5 <= 12))
        {
            dateOrder = doYMD;
            definitive = true;
        }
    }

    // Logic could be enhanced later to use DOD or ageAtDeath to further refine
    if (!definitive)
    {
        // Default to MDY is all values are valid
        if ((yearPosition == 3) && (num1 <= 12) && (num2 <= 31) && (num4 <= 12) && (num5 <= 31))
        {
            dateOrder = doMDY;
            finalDate = QDate(yyyy1, num1, num2);
            finalDate2 = QDate(yyyy2, num4, num5);
        }
        else
            return;
    }
    else
    {
        switch (dateOrder)
        {
        case doMDY:
        case doMD20Y:
            finalDate = QDate(yyyy1, num1, num2);
            finalDate2 = QDate(yyyy2, num4, num5);
            break;

        case doDMY:
            finalDate = QDate(yyyy1, num2, num1);
            finalDate2 = QDate(yyyy2, num5, num4);
            break;

        case doYMD:
            finalDate = QDate(yyyy1, num2, num3);
            finalDate2 = QDate(yyyy2, num5, num6);
            break;

        default:
            break;
        }
    }

    if (finalDate.isValid() && finalDate2.isValid())
        conviction = cvHIGH;
    else
    {
        finalDate.setDate(0,0,0);
        finalDate2.setDate(0,0,0);
    }
}


void MAKEDATE::countChar()
{
	switch (currentNumber)
	{
	case 1:
		numChar1++;
		break;
	case 2:
		numChar2++;
		break;
	case 3:
		numChar3++;
		break;
	default:
		// do nothing
		break;
	}
}

unsigned int MAKEDATE::getNumChar(unsigned int number) const
{
	unsigned int result;

	switch (number)
	{
	case 1:
		result = numChar1;
		break;
	case 2:
		result = numChar2;
		break;
	case 3:
		result = numChar3;
		break;
	default:
		result = 99;
	}

	return result;
}

void MAKEDATE::store(unsigned int tot)
{
	switch (currentNumber)
	{
	case 1:
		num1 = tot;
		break;
	case 2:
		num2 = tot;
		break;
	case 3:
		num3 = tot;
		break;
	default:
		// do nothing
		break;
	}

	totalStored++;
}

void MAKEDATE::nextNum()
{
	currentNumber++;
}

void MAKEDATE::analyzeNumbers(DATEORDER dateOrder)
{
	// For this function to be called, we know that three numbers were read in
	
    unsigned int currentYear = static_cast<unsigned int>(today.year());
    QDate tempDate, dateYMD, dateDMY, dateMDY;
	bool orderEstablished = false;

	// Look for obvious errors
	if ((num1 > currentYear) || (num2 > currentYear) || (num3 > currentYear))
		return;  // Can't be a date in the future
	if ((num1 > 12) && (num2 > 12) && (num3 > 12))
		return;  // At least one number must be a month
	if (num2 > 31)
		return;  // Assume year always first or last
	if (((num1 > 31) && (num2 > 31)) || ((num1 > 31) && (num3 > 31)) || ((num2 > 31) && (num3 > 31)))
		return;  // At least two numbers must be equal to 31 and under

	// Conviction level of outcome dependent on having a yyyy number
	CONVICTION maxConviction;
    if ((numChar1 == 4) || (numChar2 == 4) || (numChar3 == 4) || (dateOrder != doNULL))
		maxConviction = CONVICTION(cvHIGH);
	else
		maxConviction = CONVICTION(cvLIKELY);

    switch (dateOrder)
    {
    case doYMD:
        potentialYMD = true;
        potentialDMY = false;
        potentialMDY = false;
        orderEstablished = true;
        break;

    case doDMY:
        potentialYMD = false;
        potentialDMY = true;
        potentialMDY = false;
        orderEstablished = true;
        break;

    case doMDY:
    case doMD20Y:
        potentialYMD = false;
        potentialDMY = false;
        potentialMDY = true;
        orderEstablished = true;
        break;

    case doNULL:
    default:
        potentialYMD = false;
        potentialDMY = false;
        potentialMDY = false;
        orderEstablished = false;
        break;
    }

	// Adjust data if possible to change yy to yyyy
	bool unadjusted = true;
	if ((numChar1 <= 2) && (numChar2 <= 2) && (numChar3 <= 2))
	{
        switch(dateOrder)
        {
        case doYMD:
            num1 += 1900;
            unadjusted = false;
            break;

        case doDMY:
        case doMDY:
            num3 += 1900;
            unadjusted = false;
            break;

        case doMD20Y:
            num3 += 2000;
            unadjusted = false;
            break;

        case doNULL:
        default:
            if (num1 > 31)
            {
                // We can safely conclude its a year for a DOB in 1900s (risk of 1800s birth)
                num1 += 1900;
                unadjusted = false;
            }
            if (unadjusted && (num3 > 31))
            {
                // We can safely conclude its a year for a DOB in 1900s (risk of 1800s birth)
                num3 += 1900;
                unadjusted = false;
            }
            /*if (unadjusted && ((num1 == (currentYear - 2000)) || (num1 == (currentYear - 2001))) && ((num3 != (currentYear - 2000)) && (num3 != (currentYear - 2001))))
            {
                // We can safely assume first number is from a recent death
                num1 += 2000;
                unadjusted = false;
            }
            if (unadjusted && ((num3 == (currentYear - 2000)) || (num3 == (currentYear - 2001))) && ((num1 != (currentYear - 2000)) && (num1 != (currentYear - 2001))))
            {
                // We can safely assume first number is from a recent death
                num3 += 2000;
                unadjusted = false;
            }*/

            // If a safe assumption has yet to be made, check if any variation of date is within 30 days of current date (i.e., is it a DOD
            if (unadjusted)
            {
                tempDate = QDate(num1 + 2000, num2, num3);
                if (tempDate.isValid() && (tempDate < today))
                    dateYMD = tempDate;

                tempDate = QDate(num3 + 2000, num2, num1);
                if (tempDate.isValid() && (tempDate < today))
                    dateDMY = tempDate;

                tempDate = QDate(num3 + 2000, num1, num2);
                if (tempDate.isValid() && (tempDate < today))
                    dateMDY = tempDate;

                QDate oneMonthAgo = today.addMonths(-1);

                if (dateYMD.isValid() && (dateYMD >= dateDMY) && (dateYMD >= dateMDY) && (dateYMD > oneMonthAgo))
                {
                    num1 += 2000;
                    unadjusted = false;
                    orderEstablished = true;
                    potentialYMD = true;
                }

                if (dateDMY.isValid() && (dateDMY > dateYMD) && (dateDMY >= dateMDY) && (dateDMY > oneMonthAgo))
                {
                    num3 += 2000;
                    unadjusted = false;
                    orderEstablished = true;
                    potentialDMY = true;
                }

                if (dateMDY.isValid() && (dateMDY > dateYMD) && (dateMDY > dateDMY) && (dateMDY > oneMonthAgo))
                {
                    num3 += 2000;
                    unadjusted = false;
                    orderEstablished = true;
                    potentialMDY = true;
                }
            }
            break;
        }

		if (unadjusted)
			analyzeFurther = true;		// Will revisit if the other date can be validated
	}
	
	if (!orderEstablished)
	{
		// Year first position
		if ((num1 >= 1890) && (num1 <= currentYear))
		{
			// Only one possible correct date exists
			potentialYMD = true;
			validatePotentials(maxConviction);
			return;
		}

		// Year second position => ERROR
		if ((num2 >= 1890) && (num2 <= currentYear))
			return;

		// Year third position => both DMY and MDY are possible
		if ((num3 >= 1890) && (num3 <= currentYear))
		{
			// Check for error
			if ((num1 == 0) || (num2 == 0))
				return;		// only year can possibly be zero (i.e. year 2000 with yy format)

			// DMY
			if ((num1 > 12) && (num1 <= 31))
				potentialDMY = true;

			// MDY
			if ((num2 > 12) && (num2 <= 31))
				potentialMDY = true;

			// Inconclusive DMY or MDY
			if ((num1 <= 12) && (num2 <= 12))
			{
				potentialDMY = true;
				potentialMDY = true;
			}

			validatePotentials(maxConviction);
			return;
		}
	} // end if !orderEstablishes

	if (orderEstablished)
	{
		validatePotentials(maxConviction);
		return;
	}
}

void MAKEDATE::validatePotentials(CONVICTION maxConviction)
{
    QDate tempDate;
	
	unsigned int numPotentials = 0;

	if (potentialYMD)
	{
		numPotentials++;
        tempDate = QDate(num1, num2, num3);
        if (tempDate.isValid())
			potentialDateYMD = tempDate;
	}

	if (potentialDMY)
	{
		numPotentials++;
        tempDate = QDate(num3, num2, num1);
        if (tempDate.isValid())
            potentialDateDMY = tempDate;
	}

	if (potentialMDY)
	{
		numPotentials++;
        tempDate = QDate(num3, num1, num2);
        if (tempDate.isValid())
            potentialDateMDY = tempDate;
	}

	switch (numPotentials)
	{
	case 1:
		if (potentialYMD)
		{
			finalDate = potentialDateYMD;
			format = DATEORDER(doYMD);
			conviction = maxConviction;
		}

		if (potentialDMY)
		{
			finalDate = potentialDateDMY;
			format = DATEORDER(doDMY);
			conviction = maxConviction;
		}

		if (potentialMDY)
		{
			finalDate = potentialDateMDY;
			format = DATEORDER(doMDY);
			conviction = maxConviction;
		}
		
		isValid = true;
		isFinal = true;
		break;

	case 2:
		// Only final if day == month as both MDY and DMY result in same date
		if (potentialDateMDY.isValid() && (potentialDateMDY == potentialDateDMY))
		{
			isFinal = true;
			isValid = true;
			conviction = maxConviction;
			finalDate = potentialDateMDY;
			// format remains ambiguous
		}
		else
		{
			conviction = cvPOSSIBLE;
			isValid = true;
		}
		break;

	case 3:
		// Not possible
		conviction = cvPOSSIBLE;
		isValid = true;
		break;

	default:
		break;
	}

}

void MAKEDATE::revisitAssuming(DATEORDER dateOrder)
{
	// Either i) Original year would only have been yy
	// Or     ii) There were more than one valid possibilities

	// Use lower threshold if "analyzeFurther" was set to true

    unsigned int currentYear = static_cast<unsigned int>(today.year());

	bool adjusted = false;
    QDate tempDate;

	switch (dateOrder)
	{
	case doYMD:
		if ((numChar1 <= 2) && (num1 > (currentYear - 2000)))
		{
			num1 += 1900;
			adjusted = true;
		}
		else
		{
			if ((numChar1 <= 2) && (num1 <= (currentYear - 2000)))
			{
				num1 += 2000;
				adjusted = true;
			}
		}
		break;

	case doDMY:
	case doMDY:
		if ((numChar3 <= 2) && (num3 > (currentYear - 2000)))
		{
			num3 += 1900;
			adjusted = true;
		}
		else
		{
			if ((numChar3 <= 2) && (num3 <= (currentYear - 2000)))
			{
				num3 += 2000;
				adjusted = true;
			}
		}
		break;

	default:
		break;
	}

	if (adjusted || isValid)
	{
		switch (dateOrder)
		{
		case doYMD:
            tempDate = QDate(num1, num2, num3);
			break;

		case doDMY:
            tempDate = QDate(num3, num2, num1);
			break;

		case doMDY:
            tempDate = QDate(num3, num1, num2);
			break;

        default:
            break;
		}

        if (tempDate.isValid())
		{
			finalDate = tempDate;
			format = dateOrder;
			isValid = true;
			isFinal = true;
			conviction = cvPOSSIBLE;
		}
	}
}

void MAKEDATE::setToday(QDate &date)
{
    today = date;
}



