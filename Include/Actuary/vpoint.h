// vpoint.h   version 1.0  March 24, 2003
//			          2.0  March 1, 2009 - Eliminate all shorts
//

#ifndef VPOINT_H
#define VPOINT_H

#ifndef VBASE_H
#include "vbase.h"
#endif

#ifndef ERRORFLAG_H
#include "errorFlag.h"
#endif

class vpoint : public vbase {

private:

	// Methods
	void calcVT();
	long double itsValue() const;

	// Variables
	long double vT;

public:
	// Constructors
	vpoint();
	vpoint(const intDiscV &idv, const double duration, errorFlag &Error);
	vpoint(const vpoint &rhs);

	// Conversion operator
	operator long double(); //provides rhs read value

	// Assignment operator
	vpoint& operator=(vpoint &rhs);

	// Calculation operators
	long double operator*(const vpoint &rhs);
	long double operator/(const vpoint &rhs);

	//Destructor
	~vpoint();
};

vpoint::vpoint() : vbase()
{
	vT = 1;
}

vpoint::vpoint(const vpoint &rhs)
{
	durn = rhs.durn;
	intDiscVec = rhs.intDiscVec;
	vT = rhs.itsValue();
}

vpoint::vpoint(const intDiscV &idv, const double duration, errorFlag &Error)
				: vbase(idv)
{
	validateDuration(duration, Error);
	if (!Error)
		{
		durn = duration;
		calcVT();
		}
}

vpoint::operator long double()
{
	return vT;
}

vpoint & vpoint::operator=(vpoint &rhs)
{
	if (this == &rhs) return *this;
	durn = rhs.durn;
	intDiscVec = rhs.intDiscVec;
	vT = rhs.itsValue();
	return *this;
}

long double vpoint::operator*(const vpoint &rhs)
{
	return vT * rhs.itsValue();
}

long double vpoint::operator/(const vpoint &rhs)
{
	return vT / rhs.itsValue();
}

vpoint::~vpoint()
{
}

void vpoint::calcVT()
{
	unsigned int index = 0;
	unsigned int periods = intDiscVec.getPeriods();
	unsigned int remPeriods = periods;
	long double cumDurn = 0;
	long double periodDurn;

	vT = (long double)1;
	while (cumDurn < durn)
		{
		if (remPeriods == 1)
			periodDurn = durn - cumDurn;
		else
			periodDurn = ((durn-cumDurn) > intDiscVec[index*2 + 1]) ?
											intDiscVec[index*2 + 1] : durn - cumDurn;
		vT *= powl(1 + intDiscVec[index*2],-periodDurn);
		cumDurn += periodDurn;
		index++;
		remPeriods--;
		}

	// Round final answer to 4 decimal places
	vT = (long double)(.0001 * (long int)(vT * 10000 + 0.5));
}

long double vpoint::itsValue() const
{
	return vT;
}
#endif
