// eligdate.h     Version 1.0    March 24, 2003
//						  1.1    February 21, 2009 - Added operator= (assign a date)
//					      1.2    February 22, 2009 - Added "latest of" option
//												   - Fix "already met condition" error
//					      2.0    March 1, 2009 - Eliminate shorts
//                        2.1    January 20, 2019 - Adjust for elapse.h
//

#ifndef ELIGDATE_H
#define ELIGDATE_H

#ifndef DATE_H
#include "../Actuary/date.h"
#endif

#ifndef ERRORFLAG_H
#include "../Actuary/errorFlag.h"
#endif

#ifndef ELAPSE_H
#include "../Actuary/elapse.h"
#endif

class dpoint : public date {

protected:
	
	// Methods
	void validateEligReq(const double &eligReq, errorFlag &Error) const;
	void validateIncr(const double &incr, errorFlag &Error) const;

public:

	// Constructors
	dpoint(const date &DOB, const double &ageReq, const double &incr,
		   const unsigned int &rndOption, errorFlag &Error);

	dpoint(const date &DOB, const double &ageReq, const date &DOH,
		   const double &SVAcc, const double &SVReq, const double &SVIncr,
		   const double &ptsAcc, const double &ptsReq, const double &ptsSVIncr,
		   const unsigned int &rndOption, errorFlag &Error);

	dpoint(const date &DOB, const double &ageReq, const date &DOH,
		   const double &SVAcc, const double &SVReq, const double &SVIncr,
		   const double &ptsAcc, const double &ptsReq, const double &ptsSVIncr,
		   const unsigned int &rndOption, const unsigned int &flOption,
		   errorFlag &Error);

	// Methods
	dpoint& operator= (const date &rhs);

};


dpoint::dpoint(const date &DOB, const double &ageReq, const double &incr,
			   const unsigned int &rndOption, errorFlag &Error) : date()
{
	validateEligReq(ageReq, Error);  // <=999
	if (Error) return;
	validateIncr(incr, Error);		// >=0 and <= 5
	if (Error) return;
    validateOption(rndOption, Error);	// 0, 1, 2, 3, 4
	if (Error) return;

	long double time, timeToEOY, timeRemaining;
	unsigned int timeYears, daysRemaining;
	int sign;

	sign = (ageReq >= 0) ? 1 : -1;

    if (static_cast<int>(ageReq) == 0)
		time = 0;
	else
	{
        if (static_cast<int>(incr) == 0)
			time = sign * 120;
		else
            time = static_cast<long double>(ageReq) / incr;
	}
	if ((time/sign) > 120)
		time = sign * 120;
    timeYears = static_cast<unsigned int>(time + sign * .000001);
	timeRemaining = time - timeYears;

	date temp1(DOB.year()+timeYears, DOB.month(), DOB.day(), Error);
	if (Error) return;
	
	if ((sign * timeRemaining) > 0)
	{
		date EOY;
		if (sign == 1)
		{
			date tEOY(DOB.year()+timeYears+1,1,1, Error);
			EOY = tEOY;
		}
		else
		{
			date tEOY(DOB.year()+timeYears-1,12,31, Error);
			EOY = tEOY;
		}
		if (Error) return;
		timeToEOY = elapse(temp1,EOY,Error);
		if (Error) return;
		if ((sign*timeRemaining) > (sign*timeToEOY))
		{
			temp1 = EOY;
			timeRemaining -= timeToEOY;
		}
        daysRemaining = static_cast<unsigned int>(timeRemaining * temp1.daysInYear() + sign * 0.5);
		date temp2(temp1.numericValue()+daysRemaining, Error);
		if (Error) return;
		temp1 = temp2;
	}
	
	unsigned int roundingOption = (ageReq > 0) ? rndOption : 0;
	date rounded;
	rounded = roundUp(temp1,roundingOption,Error);
	if (Error) return;
	yyyy = rounded.year();
	mm   = rounded.month();
	dd   = rounded.day();
	numericDate = rounded.numericValue();
}


dpoint::dpoint(const date &DOB, const double &ageReq, const date &DOH,
			   const double &SVAcc, const double &SVReq, const double &SVIncr,
			   const double &ptsAcc, const double &ptsReq, const double &ptsSVIncr,
			   const unsigned int &rndOption, errorFlag &Error) : date()
{
	date result;

	if (DOB.numericValue() > DOH.numericValue())
	{
		Error = 46;   // DOH/DOC must be >= DOB
		return;
	}

	double ageIncr = 1;
	dpoint eligAge(DOB,ageReq,ageIncr,0,Error);
	if (Error) return;
	result = eligAge;

    if (static_cast<int>(SVReq) != 999)
	{
		double remSVReq = (SVAcc > SVReq) ? 0 : SVReq - SVAcc;
		dpoint eligSV(DOH,remSVReq,SVIncr,0,Error);
		if (Error) return;
		result = dmin(result,eligSV);
	}

    if (static_cast<int>(ptsReq) != 999)
	{
		double initPtsAge = elapse(DOB,DOH,Error);
		if (Error) return;
		double remPtsReq = (ptsReq > (initPtsAge + ptsAcc)) ?
								ptsReq - (initPtsAge + ptsAcc) : 0;
		double ptsIncr = ageIncr + ptsSVIncr;
		dpoint eligPts(DOH,remPtsReq,ptsIncr,0,Error);
		if (Error) return;
		result = dmin(result,eligPts);
	}

	date rounded;
	rounded = roundUp(result,rndOption,Error);
	if (Error) return;
	yyyy = rounded.year();
	mm   = rounded.month();
	dd   = rounded.day();
	numericDate = rounded.numericValue();
}

dpoint::dpoint(const date &DOB, const double &ageReq, const date &DOH,
			   const double &SVAcc, const double &SVReq, const double &SVIncr,
			   const double &ptsAcc, const double &ptsReq, const double &ptsSVIncr,
			   const unsigned int &rndOption, const unsigned int &flOption,
			   errorFlag &Error) : date()
{
	date result;

	if (DOB.numericValue() > DOH.numericValue())
	{
		Error = 46;   // DOH/DOC must be >= DOB
		return;
	}

	double ageIncr = 1;
	dpoint eligAge(DOB,ageReq,ageIncr,0,Error);
	if (Error) return;
	result = eligAge;

    if (static_cast<int>(SVReq) != 999)
    {
		double remSVReq = (SVAcc > SVReq) ? 0 : SVReq - SVAcc;
		dpoint eligSV(DOH,remSVReq,SVIncr,0,Error);
		if (Error) return;
		if (flOption)
			result = dmax(result,eligSV);
		else
			result = dmin(result,eligSV);
	}

    if (static_cast<int>(ptsReq) != 999)
    {
		double initPtsAge = elapse(DOB,DOH,Error);
		if (Error) return;
		double remPtsReq = (ptsReq > (initPtsAge + ptsAcc)) ?
								ptsReq - (initPtsAge + ptsAcc) : 0;
		double ptsIncr = ageIncr + ptsSVIncr;
		dpoint eligPts(DOH,remPtsReq,ptsIncr,0,Error);
		if (Error) return;
		if (flOption)
			result = dmax(result,eligPts);
		else
			result = dmin(result,eligPts);
	}

	date rounded;
	rounded = roundUp(result,rndOption,Error);
	if (Error) return;
	yyyy = rounded.year();
	mm   = rounded.month();
	dd   = rounded.day();
	numericDate = rounded.numericValue();
}

dpoint& dpoint::operator = (const date &rhs)
{
	yyyy = rhs.year();
	mm   = rhs.month();
	dd   = rhs.day();
	numericDate = rhs.numericValue();

	return *this;
}


void dpoint::validateEligReq(const double &eligReq, errorFlag &Error) const
{
	if (eligReq > 999)
		Error = 44;		// Invalid eligibility requirement
}


void dpoint::validateIncr(const double &incr, errorFlag &Error) const
{
	if ((incr < 0) || (incr > 5))
		Error = 45;		// Invalid increment parameter
}

#endif
