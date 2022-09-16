// series.h     Version 1.0  March 24, 2003
//						2.0  March 1, 2009 - No change 
//

#ifndef SERIES_H
#define SERIES_H

#ifndef PVector_H
#include "../Actuary/PVector.h"
#endif

#ifndef FREQ_H
#include "../Actuary/freq.h"
#endif

class series : public freq {

protected:

	virtual void expand(const PVector<long double> &)=0;

public:
	// Constructors
	series();
	series(const freq &frequency);

	//Destructor
	virtual ~series();
};

series::series(): freq()
{
}

series::series(const freq &frequency): freq(frequency)
{
}

series::~series()
{
}

#endif
