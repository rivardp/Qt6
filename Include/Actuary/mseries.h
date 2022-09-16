// mseries.h   Version 1.0  March 24, 2003
//					   3.0  August 15, 2008 - Modifications for generational mortality
//					   3.1  February 21, 2009 - New constructor using DOB
//					   4.0  March 1, 2009 - Eliminate shorts
//

#ifndef MSERIES_H
#define MSERIES_H

#ifndef PVector_H
#include "../Actuary/PVector.h"
#endif

#ifndef SERIES_H
#include "../Actuary/series.h"
#endif

#ifndef MBASE_H
#include "../Actuary/mbase.h"
#endif

#ifndef ELIGDATE_H
#include "../Actuary/eligdate.h"
#endif

#ifndef FREQ_H
#include "freq.h"
#endif 

class mseries : public PVector<long double>,
					 public series,
					 public mbase {

private:

	virtual void expand(const PVector<long double> &baseRates);

public:
	// Constructors
	mseries(const sex &xsex, const double currentAge, const double duration,
			const mbase &mtable, const date &doc, const freq &frequency, 
			errorFlag &Error);

	mseries(const sex &xsex, const date &dob, const date &doc, 
		    const double duration, const mbase &mtable, const freq &frequency, 
			errorFlag &Error);

	//Destructor
	~mseries();
};

mseries::mseries(const sex &xsex, const double currentAge, const double duration, 
				 const mbase &mtable, const date &doc, const freq &frequency, 
				 errorFlag &Error) : PVector<long double>(), series(frequency), 
				 mbase(mtable)
{
    if (static_cast<int>(xsex) != 99)
	{
        setMTable(table,Error);
		if (Error) return;
		msex = xsex;
	}
	else  // User Defined Table
	{
		setUDMTable(xsex,Error);
		if (Error) return;
		msex = 99;
		table = UserDefined;
	}
	age = currentAge;
	validateAge(Error);
	if (Error) return;
	validateDuration(duration,Error);
	if (Error) return;
	setDuration(duration);
	
	if (timing == advance)
        size = 1 + static_cast<unsigned long int>(durn * payFreq - .0001);
	else
        size = static_cast<unsigned long int>(durn * payFreq + .0005);

	if ((mtable.getMortalityType() == generational) || (mtable.getMortalityType() == generational2D))
	{
		date dob(dpoint(doc,-currentAge,1,0,Error));
		if (Error) return;
		setGenInputs(dob,doc,Error);
		if (Error) return;
	}
	loadMortRates(Error);
	if (Error) return;

	PVector<long double> &baseRates = *pbaseRates;
	expand(baseRates);
	delete pbaseRates;
    pbaseRates = nullptr;
}

mseries::mseries(const sex &xsex, const date &dob, const date &doc, 
				 const double duration, const mbase &mtable, const freq &frequency, 
				 errorFlag &Error) : PVector<long double>(), series(frequency), 
				 mbase(mtable)
{
    if (static_cast<int>(xsex) != 99)
    {
        setMTable(table,Error);
		if (Error) return;
		msex = xsex;
	}
	else  // User Defined Table
	{
		setUDMTable(xsex,Error);
		if (Error) return;
		msex = 99;
		table = UserDefined;
	}
	age = elapse(dob,doc,Error);
	if (Error) return;
	validateAge(Error);
	if (Error) return;
	validateDuration(duration,Error);
	if (Error) return;
	setDuration(duration);
	
    if (timing == advance)
        size = 1 + static_cast<unsigned long int>(durn * payFreq - .0001);
    else
        size = static_cast<unsigned long int>(durn * payFreq + .0005);

	if ((mtable.getMortalityType() == generational) || (mtable.getMortalityType() == generational2D))
	{
		setGenInputs(dob,doc,Error);
		if (Error) return;
	}
	loadMortRates(Error);
	if (Error) return;

	PVector<long double> &baseRates = *pbaseRates;
	expand(baseRates);
	delete pbaseRates;
    pbaseRates = nullptr;
}

mseries::~mseries()
{
}

void mseries::expand(const PVector<long double> &baseRates)
{
	data = new long double[size];
    assert (data != nullptr);
	if (timing == advance)
        data[0] = static_cast<long double>(1);

    unsigned int periodsTotal = static_cast<unsigned int>(size - timing);
    unsigned int ageLastBirthday = static_cast<unsigned int>(age);
	long double timeSinceLastBirthday = age - ageLastBirthday;
    unsigned int periodsUntilBirthday = static_cast<unsigned int>(payFreq * (1 - timeSinceLastBirthday));
        long double fractionOfPeriod = (payFreq - periodsUntilBirthday) / static_cast<long double>(payFreq - timeSinceLastBirthday);
	long double a,b;
    if (fractionOfPeriod == static_cast<long double>(0))
		{
		periodsUntilBirthday = 0;
        b = 1 / static_cast<long double>(payFreq);
		a = 0;
		}
	else
		{
		b = fractionOfPeriod;
        a = 1 / static_cast<long double>(payFreq - b);
		}

	long double tPx,sQxt,Qx,QxA,QxB,aQxt,bQxt;
	unsigned int attainedAgeLastBirthdayBeg,attainedAgeLastBirthdayEnd;
    tPx = static_cast<long double>(1);
	long double attainedAgeBeg, attainedAgeEnd;

// Put in check for maximum age where rates provided
	for (unsigned int index=0; index<periodsTotal; index++)
		{
        attainedAgeBeg = age + index / static_cast<long double>(payFreq);
        attainedAgeEnd = age + (index+1) / static_cast<long double>(payFreq);
        attainedAgeLastBirthdayBeg = (attainedAgeBeg > ageLast) ? ageLast : static_cast<unsigned int>(attainedAgeBeg);
        attainedAgeLastBirthdayEnd = (attainedAgeEnd > ageLast) ? ageLast : static_cast<unsigned int>(attainedAgeEnd);
					 // UDD assumed
		if (attainedAgeLastBirthdayBeg != attainedAgeLastBirthdayEnd)
								// Payment period extends over a birthday
			{
			QxA = baseRates[attainedAgeLastBirthdayBeg - fAge];
			QxB = baseRates[attainedAgeLastBirthdayEnd - fAge];
			aQxt = b * QxA / (1 - (1-b) * QxA);
			bQxt = (1 - aQxt) * a * QxB;
			sQxt = aQxt + bQxt;
			}
		else   // Payment period within one age
			{
			Qx = baseRates[attainedAgeLastBirthdayBeg - fAge];
            sQxt = (1 / static_cast<long double>(payFreq)) * Qx /
						 (1 - (attainedAgeBeg - attainedAgeLastBirthdayBeg) * Qx);
			}
		tPx *= (1 - sQxt);
		data[index+timing] = tPx;
		}
}

#endif
