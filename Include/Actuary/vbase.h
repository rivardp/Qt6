// vbase.h   Version 1.0   March 24, 2003
//					 2.0   March 1, 2009 - No change
//

#ifndef VBASE_H
#define VBASE_H

#ifndef INTDISCV_H
#include "../Actuary/intdiscv.h"
#endif

#ifndef ERRORFLAG_H
#include "../Actuary/errorFlag.h"
#endif

class vbase {

protected:
	double durn; 		 // Duration of stream in years
	intDiscV intDiscVec; // Interest discount PVector

	void validateDuration(const double duration, errorFlag &Error) const;

public:
	// Constructors
	vbase();
	vbase(const PVector<long double> &shortForm, errorFlag &Error);
	vbase(const long double intRate, errorFlag &Error);
	vbase(const intDiscV &idv);

	//Destructor
	~vbase();
};

vbase::vbase() : durn(0) , intDiscVec()
{
}

vbase::vbase(const PVector<long double> &shortForm, errorFlag &Error) 
				: durn(0) , intDiscVec(shortForm, Error)
{
}

vbase::vbase(const long double intRate, errorFlag &Error)
				: durn(0) , intDiscVec(intRate, Error)
{
}

vbase::vbase(const intDiscV &idv) : durn(0) , intDiscVec(idv)
{
}

vbase::~vbase()
{
}


void vbase::validateDuration(const double duration, errorFlag &Error) const
{
	if (duration < 0)
		Error = 4; // Interest/Indexing period cannot be < 0
}

#endif
