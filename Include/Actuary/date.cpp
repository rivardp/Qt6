// date.cpp

#include "../Actuary/date.h"

date::date() : yyyy(0), mm(0), dd(0), numericDate(0)
{
}

date::date(const unsigned int &Year, const unsigned int &Month, const unsigned int &Day, errorFlag &Error)
{
	numericDate = yyyy = mm = dd = 0;

	if (Year < 1875)
	{
		Error = 40; // Year cannot be less than 1875
		return;
	}

	if (Year > 2250)
	{
		Error = 41; // Year cannot be greater than 2150
		return;
	}
	
	if ((Month < 1) || (Month > 12) || (Day < 1))
	{
		Error = 42; // Invalid date
		return;
	}

	switch (Month)
	{
	case 4:        // April
	case 6:        // June
	case 9:        // September
	case 11:       // November
		if (Day > 30)
			Error = 42;  // Invalid date
		break;

	case 2:
		if (isLeapYear(Year))
		{
			if (Day > 29)
				Error = 42;  // Invalid date
		}
		else
			if (Day > 28)
				Error = 42;  // Invalid date
		break;

	default:      // All remaining months
		if (Day > 31)
			Error = 42;  // Invalid date
		break;
	
	}

	if (Error) return;

	yyyy = Year;
	mm = Month;
	dd = Day;
	numericDate = convertToNumeric(yyyy, mm, dd);

}

date::date(const unsigned long int &dateNumber, errorFlag &Error)
{
	numericDate = yyyy = mm = dd = 0;

    if (dateNumber < 1875)
	{
		Error = 40; // Year cannot be less than 1875
		return;
	}

	date maxDate(2250,12,31, Error);
	if (dateNumber > maxDate.numericValue())
	{
		Error = 41; // Year cannot be greater than 2150
		return;
	}

	numericDate = dateNumber;
	date temp = convertFromNumeric(dateNumber,Error);
	if (Error) return;
	yyyy = temp.year();
	mm   = temp.month();
	dd   = temp.day();
}

date::date(const date &rhs)
{
	yyyy = rhs.year();
	mm = rhs.month();
	dd = rhs.day();
	numericDate = rhs.numericDate;
}
date::date(const date *rhs)
{
	yyyy = rhs->year();
	mm = rhs->month();
	dd = rhs->day();
	numericDate = rhs->numericDate;
}

date::date(const unsigned int usi) : yyyy(0), mm(0), dd(0), numericDate(0)
{
    Q_UNUSED( usi )
}

/*double elapse(const unsigned long int &nd1, const unsigned long int &nd2, errorFlag &Error)
{
    date dateBeg(nd1, Error);
    if (Error) return 0;
    date dateEnd(nd2, Error);
    if (Error) return 0;

    return elapse(dateBeg, dateEnd, Error);
}

double elapse(const date &dateBeg, const date &dateEnd, errorFlag &Error)
{
    double period;

    if (dateBeg.numericValue() > dateEnd.numericValue())
        return -elapse(dateEnd,dateBeg,Error);

    if (dateBeg.year() == dateEnd.year())
    {
        period = (static_cast<double>(dateEnd.numericValue() - dateBeg.numericValue())) /
                    dateBeg.daysInYear();
    }
    else
    {
        double t1, t2, t3;
        date endOfFirstYear(dateBeg.year()+1,1,1, Error);
        if (Error) return 0;
        date begOfLastYear(dateEnd.year(),1,1, Error);
        if (Error) return 0;
        t1 = (static_cast<double>(endOfFirstYear.numericValue() - dateBeg.numericValue())) /
                    dateBeg.daysInYear();
        t2 = dateEnd.year() - endOfFirstYear.year();
        t3 = (static_cast<double>(dateEnd.numericValue() - begOfLastYear.numericValue())) /
                    dateEnd.daysInYear();
        period = t1 + t2 + t3;
    }

    return static_cast<double>(0.0001 * static_cast<long int>(period * 10000 + 0.5));
}*/

unsigned int date::year() const
{
	return yyyy;
}

unsigned int date::month() const
{
	return mm;
}

unsigned int date::day() const
{
	return dd;
}

unsigned long int date::numericValue() const
{
	return numericDate;
}

unsigned int date::daysInYear() const
{
	return daysInYear(yyyy);
}

bool date::operator==(const date &rhs) const
{
	return (numericDate == rhs.numericValue()) ? true : false;
}

bool date::operator!=(const date &rhs) const
{
	return (numericDate == rhs.numericValue()) ? false : true;
}

bool date::operator<=(const date &rhs) const
{
	return (numericDate <= rhs.numericValue()) ? true : false;
}

bool date::operator< (const date &rhs) const
{
	return (numericDate < rhs.numericValue()) ? true : false;
}

bool date::operator>=(const date &rhs) const
{
	return (numericDate >= rhs.numericValue()) ? true : false;
}

bool date::operator> (const date &rhs) const
{
	return (numericDate > rhs.numericValue()) ? true : false;
}

date date::operator+(const unsigned long int &days) const
{
	errorFlag Error;
	date temp(numericDate + days, Error);
	return temp;
}

date date::operator-(const unsigned long int &days) const
{
	errorFlag Error;
	date temp(numericDate - days, Error);
	return temp;
}

date& date::operator=(const date &rhs)
{
	yyyy = rhs.year();
	mm   = rhs.month();
	dd   = rhs.day();
	numericDate = rhs.numericValue();

	return *this;
}

date& date::operator+=(const unsigned long int &days)
{
	errorFlag Error;
	date temp(numericDate + days, Error);
	yyyy = temp.year();
	mm   = temp.month();
	dd   = temp.day();
	numericDate = temp.numericValue();
	return *this;
}

date& date::operator-=(const unsigned long int &days)
{
	errorFlag Error;
	date temp(numericDate - days, Error);
	yyyy = temp.year();
	mm   = temp.month();
	dd   = temp.day();
	numericDate = temp.numericValue();
	return *this;
}

unsigned int date::isLeapYear(const unsigned int &Year) const
{
	return leapYears.isFound(Year);
}

unsigned int date::isValid() const
{
	if (numericDate > 0)
		return 1;
	else
		return 0;
}

unsigned int date::countLeapDaysPriorTo(const unsigned int &Year) const
{
	unsigned int leapDays = 0;
	for (unsigned int i = 1875; i < Year; i++)
		leapDays += isLeapYear(i);
	return leapDays;
}

unsigned int date::daysInYear(const unsigned int &d1) const
{
	return 365 + isLeapYear(d1);
}

unsigned long int date::convertToNumeric(const unsigned int &Year, const unsigned int &Month, const unsigned int &Day)
{
	unsigned long int nd;
	nd = (Year - 1875) * 365 + countLeapDaysPriorTo(Year) + cumDays[Month-1] + Day;
	if (isLeapYear(Year) && (Month > 2))
		nd += 1;
	return nd;
}

date date::convertFromNumeric(const unsigned long int &nd, errorFlag &Error)
{
	if (nd == 0)
	{
		date uninitialized;
		return uninitialized;
	}
	
	// Note assumes input from Excel has been previously adjusted to different scale,
	// thus 1875/01/01 = 1 and 1900/01/01 = 9132;
    unsigned int baseYear = 1875 + static_cast<unsigned int>(nd / 366);
	date temp(baseYear,1,1,Error);
	if (Error) return temp;
	while ((nd - temp.numericValue()) >= daysInYear(baseYear))
	{
		temp.yyyy += 1;
		temp.numericDate += daysInYear(baseYear);
		baseYear++;
	}
    unsigned int remDays = static_cast<unsigned int>(nd - temp.numericValue() + 1);
	unsigned int i = 1; 
	if (isLeapYear(temp.year()))
	{
		while (remDays > cumDaysLeapYear[i])
			i++;
		temp.mm = i;
		temp.dd = remDays - cumDaysLeapYear[i-1];
	}
	else
	{
		while (remDays > cumDays[i])
			i++;
		temp.mm = i;
		temp.dd = remDays - cumDays[i-1];
	}

	return temp;
}

long int date::xport() const
{
    return static_cast<long>(10000 * yyyy + 100 * mm + dd);
}

void date::clear()
{
	yyyy = 0;
	mm = 0;
	dd = 0;
	numericDate = 0;
}

date date::roundUp(const date &d1, const unsigned int &option, errorFlag &Error)
{
	date result;
	validateOption(option, Error);
	if (Error)
		return result;
	
	switch (option)
	{
		case 0:				// No rounding
			result = d1;
			break;
		case 1:				// 1st of month next or coincident with
			if (d1.day() == 1)
				result = d1;
			else
			{
				if (d1.month() == 12)
				{
					date temp(d1.year()+1,1,1,Error);
					if (!Error)
						result = temp;
				}
				else
				{
					date temp(d1.year(),d1.month()+1,1,Error);
					if (!Error)
						result = temp;
				}
			}
			break;
		case 2:				// 1st of month next
			if (d1.month() == 12)
			{
                date temp(d1.year()+1,1,1,Error);
				if (!Error)
					result = temp;
			}
			else
			{
                date temp(d1.year(),d1.month()+1,1,Error);
				if (!Error)
					result = temp;
			}
			break;
		case 3:				// Last of month next or coincident with
			if (d1.month() == 12)
			{
				date temp1(d1.year()+1,1,1,Error);
				if (!Error)
				{
                    date temp2(temp1.numericValue()-1,Error);
					if (!Error)
						result = temp2;
				}
			}
			else
			{
                date temp1(d1.year(),d1.month()+1,1,Error);
				if (!Error)
				{
                    date temp2(temp1.numericValue()-1,Error);
					if (!Error)
						result = temp2;
				}
			}
			break;
		case 4:				// Last of month next
			date temp1(d1.numericValue()+1,Error);
			if (!Error)
			{
                if (temp1.day() == 1)
					result = roundUp(temp1,3,Error);
				else
					result = roundUp(d1,3,Error);
			}
			break;
	}

	return result;
}

void date::validateOption(const unsigned int &option, errorFlag &Error)
{
    if (option > 4)
		Error = 43; // Invalid date rounding option
}

date dmax(const date &d1, const date &d2, const date &d3, const date &d4, const date &d5)
{
	return dmax(d1, dmax(d2,d3,d4,d5));
}

date dmax(const date &d1, const date &d2, const date &d3, const date &d4)
{
	return dmax(d1, dmax(d2,d3,d4));
}

date dmax(const date &d1, const date &d2, const date &d3)
{
	return dmax(d1, dmax(d2,d3));
}

date dmax(const date &d1, const date &d2)
{
	return (d1.numericValue() > d2.numericValue()) ? d1 : d2;
}

date dmin(const date &d1, const date &d2, const date &d3, const date &d4, const date &d5)
{
	return dmin(d1, dmin(d2,d3,d4,d5));
}

date dmin(const date &d1, const date &d2, const date &d3, const date &d4)
{
	return dmin(d1, dmin(d2,d3,d4));
}

date dmin(const date &d1, const date &d2, const date &d3)
{
	return dmin(d1, dmin(d2,d3));
}

date dmin(const date &d1, const date &d2)
{
	return (d1.numericValue() < d2.numericValue()) ? d1 : d2;
}

PString date::stringOutput(const unsigned int format = 0) const
{
	// Coded to be backwards compatible
    // If format equals 0 (default), 1, or 2, then it's the old format code using ENUM
    // If format equals 8 or higher, then it's the new approach using DEFINE and bitwise operators

	unsigned int order = 0;
    PString result;
	PString comma = _T(", ");
	PString slash = _T("/");

	bool inclFwdSlash = ((format & nfFORWARD_SLASH) == nfFORWARD_SLASH) ||
                        (format == yyyymmdd) || (format == ddmmyyyy) || (format == mmddyyyy);
	bool showLongForm = ((format & nfLONG_FORM) == nfLONG_FORM) || (format == 0);
    
	if ((format & nfDMY) == nfDMY)
		order = 3;

	if ((format & nfMDY) == nfMDY)
		order = 2;

	// Default order if not long form is yyyymmdd
	if (((format & nfYMD) == nfYMD) || (order == 0))
		order = 1;

	if (showLongForm)  // Overrides all other choices
		order = 0;

	switch(order)
	{
	case 1:		//  yyyymmdd
	case 4:		//  yyyymmdd no slash
		result += ConvertNumberToString(yyyy, 0);

		if (inclFwdSlash)
			result += slash;

		if (mm < 10)
		{
			result += ConvertDigitToString(0);
			result += ConvertDigitToString(mm);
		}
		else
			result += ConvertNumberToString(mm, 0);

		if (inclFwdSlash)
			result += slash;

		if (dd < 10)
		{
			result += ConvertDigitToString(0);
			result += ConvertDigitToString(dd);
		}
		else
			result += ConvertNumberToString(dd,0);

		break;

	case 2:		//  mmddyyyy
		if (mm < 10)
		{
			result += ConvertDigitToString(0);
			result += ConvertDigitToString(mm);
		}
		else
			result += ConvertNumberToString(mm, 0);

		if (inclFwdSlash)
			result += slash;

		if (dd < 10)
		{
			result += ConvertDigitToString(0);
			result += ConvertDigitToString(dd);
		}
		else
			result += ConvertNumberToString(dd, 0);

		if (inclFwdSlash)
			result += slash;

		result += ConvertNumberToString(yyyy, 0);

		break;

	case 3:		//  ddmmyyyy
		if (dd < 10)
		{
			result += ConvertDigitToString(0);
			result += ConvertDigitToString(dd);
		}
		else
			result += ConvertNumberToString(dd, 0);

		if (inclFwdSlash)
			result += slash;

		if (mm < 10)
		{
			result += ConvertDigitToString(0);
			result += ConvertDigitToString(mm);
		}
		else
			result += ConvertNumberToString(mm, 0);

		if (inclFwdSlash)
			result += slash;

		result += ConvertNumberToString(yyyy, 0);
        
		break;

	case 0:			// mmmm dd, yyyy
	default:
		switch (mm)
		{
		case 1:
			result = _T("January ");
			break;

		case 2:
			result = _T("February ");
			break;

		case 3:
			result = _T("March ");
			break;

		case 4:
			result = _T("April ");
			break;

		case 5:
			result = _T("May ");
			break;

		case 6:
			result = _T("June ");
			break;

		case 7:
			result = _T("July ");
			break;

		case 8:
			result = _T("August ");
			break;

		case 9:
			result = _T("September ");
			break;

		case 10:
			result = _T("October ");
			break;

		case 11:
			result = _T("November ");
			break;

		case 12:
			result = _T("December ");
			break;

		} // End of switch on month

		result += ConvertNumberToString(dd,0) + comma + ConvertNumberToString(yyyy,0);

		break;

	}  // End of switch on format

	return result;
}

