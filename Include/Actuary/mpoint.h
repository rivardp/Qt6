// mpoint.h   Version 1.0  March 24, 2003
//					  2.2  August 23, 2008
//					  2.3  February 21, 2009 - Add constructor using DOB
//					  3.0  March 1, 2009 - Eliminate shorts
//					  3.1  May 12, 215 - Updated to handle two dimensional improvement scale
//

#ifndef MPOINT_H
#define MPOINT_H

#ifndef PVector_H
#include "../Actuary/PVector.h"
#endif

#ifndef MBASE_H
#include "../Actuary/mbase.h"
#endif

#ifndef SEX_H
#include "../Actuary/sex.h"
#endif

#ifndef ELIGDATE_H
#include "../Actuary/eligdate.h"
#endif

#ifndef ERRORFLAG_H
#include "../Actuary/errorFlag.h"
#endif

class mpoint : public mbase {

private:
	// inherited from mbase.h
	//	double durn;       // Duration of stream in years
	//	double sex;        // 1=Male, 2=Female, 0 to 1=Unisex
	//	double age;			 // Age at calculation date
	//	unsigned short int ageFirst;  // First age of mortality table
	//	unsigned short int ageLast;   // Last age of mortality table
	//	unsigned short int fAge;  // First age where Qx required
	//	unsigned short int lAge;  // Last age where Qx required
	//	enum mTableList {unassigned,GAM71,GAM83,UP94,GAM94} table;
	//	void validateDuration(double duration) const;
	//	void setDuration(double duration);
	//	void validateMTable(char *mtable);
	//	void setMTable(mTableList table);
	//	void loadMortRates();
	//	PVector<long double> *pbaseRates;

	// Constructors

	// Methods
	void calcMort(const PVector<long double> &baseRates);
	long double itsValue() const;

	// Variables
	long double tPx;


public:
	// Constructors
	mpoint();
	mpoint(const sex &xsex, const double currentAge, const double duration,
		   const mbase &mtable, const date &doc, errorFlag &Error);
	mpoint(const sex &xsex, const date &dob, const date &doc, const double duration,
		   const mbase &mtable, errorFlag &Error);
	mpoint(const mpoint &rhs);
	
	// Assignment operator
	mpoint& operator=(const mpoint &rhs);

	// Conversion operators
	operator long double() const; // provides rhs read value

	// Calculation operators
	long double operator*(const mpoint &rhs);
	long double operator/(const mpoint &rhs);

	//Destructor
	~mpoint();
};

mpoint::mpoint() : mbase()
{
	tPx = 1;
}

mpoint::mpoint(const sex &xsex, const double currentAge, const double duration,
			   const mbase &mtable, const date &doc, errorFlag &Error) : mbase(mtable)
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
	if ((mtable.getMortalityType() == generational) || (mtable.getMortalityType() == generational2D))
	{
		if (doc.numericValue() == 0)
		{
			Error = 39;  // Generational mortality requires a calculation date
			return;
		}
		date dob(dpoint(doc,-currentAge,1,0,Error));
		if (Error) return;
		setGenInputs(dob,doc,Error);
		if (Error) return;
	}
	loadMortRates(Error);
	if (Error) return;

	PVector<long double> &baseRates = *pbaseRates;
	calcMort(baseRates);
}

mpoint::mpoint(const sex &xsex, const date &dob, const date &doc, const double duration,
			   const mbase &mtable, errorFlag &Error) : mbase(mtable)
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
	if ((mtable.getMortalityType() == generational) || (mtable.getMortalityType() == generational2D))
	{
		if (doc.numericValue() == 0)
		{
			Error = 39;  // Generational mortality requires a calculation date
			return;
		}
		setGenInputs(dob,doc,Error);
		if (Error) return;
	}
	loadMortRates(Error);
	if (Error) return;

	PVector<long double> &baseRates = *pbaseRates;
	calcMort(baseRates);
}

mpoint::mpoint(const mpoint &rhs) : mbase(rhs)
{
	tPx = rhs.itsValue();
}

mpoint& mpoint::operator=(const mpoint &rhs)
{
	if (this == &rhs) return *this;
	durn = rhs.durn;
	msex = rhs.msex;
	age = rhs.age;
	ageFirst = rhs.ageFirst;
	ageLast = rhs.ageLast;
	fAge = rhs.fAge;
	lAge = rhs.lAge;
	table = rhs.table;
	discountOnDeferral = rhs.getDiscountOnDeferral();
	baseYear = rhs.baseYear;
	projYear = rhs.projYear;
	TTBD = rhs.TTBD;
	tPx = rhs.itsValue();
	return *this;
}

long double mpoint::operator*(const mpoint &rhs)
{
	return tPx * rhs.itsValue();
}

long double mpoint::operator/(const mpoint &rhs)
{
	return tPx / rhs.itsValue();
}

mpoint::~mpoint()
{
}

void mpoint::calcMort(const PVector<long double> &baseRates)
{
	long double timeSinceLastBirthday = age - fAge;
	if ((age + durn) > (ageLast + 1))
        tPx = static_cast<long double>(0);
	else
		{
		long double timePastFinalBirthday = age + durn - lAge;

		long double Qx,y,t;

        tPx = static_cast<long double>(1);

		// First period up to but not exceeding next birthday (yQx+t) - UDD assumed
		t = timeSinceLastBirthday;
		y = (durn > (1 - t)) ? (1 - t) : durn;
		Qx = baseRates[0];
		tPx *= 1 - (y * Qx) / (1 - t * Qx);

		// Loop through any full integer ages
		for (unsigned int index=1; index<(lAge-fAge); index++)
			{
			Qx = baseRates[index];
			tPx *= 1 - Qx;
			}

		// Pick up any final piece if required
		if ((lAge > fAge) && (timePastFinalBirthday > 0))
			{
			t = timePastFinalBirthday;
			Qx = baseRates[lAge-fAge];
			tPx *= 1 - (t * Qx);
			}
	}

    // Round final answer to six decimal places
    tPx = static_cast<long double>(.000001 * static_cast<long int>(tPx * 1000000 + 0.5));
}

long double mpoint::itsValue() const
{
	return tPx;
}

mpoint::operator long double() const
{
	return tPx;
}

#endif
