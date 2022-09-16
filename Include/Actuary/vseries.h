// vseries.h	Version 2.0		February 9, 2003
//                      3.0     March 1, 2009 - Eliminate shorts
//

#ifndef VSERIES_H
#define VSERIES_H

#ifndef PVector_H
#include "../Actuary/PVector.h"
#endif

#ifndef SERIES_H
#include "../Actuary/series.h"
#endif

#ifndef VBASE_H
#include "../Actuary/vbase.h"
#endif

#ifndef FREQ_H
#include "../Actuary/freq.h"
#endif


class vseries : public PVector<long double>,
					 public series,
					 public vbase {

private:
// Inherited from series.h (freq.h)
//  unsigned int payFreq;		// Number of payments per year
//  unsigned int intFreq;		// Interest is accumulated every intFreq payment
//  double payIntFreq;				// Combination of payFreq and intFreq
//  enum annuityType {arrears, advance} timing; // Annuity immediate or annuity due
//  void validateFrequency(const double frequency, errorFlag &Error) const;
//  void setFrequency(const double frequency);

// Inherited from vbase.h
//	double durn; 		 // Duration of stream in years
//	intDiscV intDiscVec; // Interest discount PVector
//	void validateDuration(const double duration, errorFlag &Error) const;

	virtual void expand(const PVector<long double> &shortForm);

public:
	// Constructors
	vseries(const intDiscV &idv, const double duration, const freq &frequency, errorFlag &Error);

	//Destructor
	~vseries();
};

vseries::vseries(const intDiscV &idv, const double duration, const freq &frequency,
		 errorFlag &Error) : PVector<long double>(),series(frequency),vbase(idv)
{
	validateDuration(duration, Error);
	if (!Error)
	{
		durn = duration;
		if (timing == advance)
            size = 1 + static_cast<unsigned long int>(durn * payFreq - .0001);
		else
            size = static_cast<unsigned long int>(durn * payFreq + .0005);
		expand(intDiscVec);
	}
}

vseries::~vseries()
{
}

void vseries::expand(const PVector<long double> &shortForm)
{
	unsigned int index;
	unsigned int periods;		// Number of distinct interest periods
	unsigned int *breaks;		// Payment #(s) after which interest rate changes
	long double *breakCum;      // Cumulative break points stated in years
	long double *discountRates;

	periods = (unsigned int)(shortForm.getSize()/2)+1;
	breaks = new unsigned int[periods];
    assert (breaks != nullptr);
	breakCum = new long double[periods];
    assert (breakCum != nullptr);
	discountRates = new long double[periods];
    assert (discountRates != nullptr);
	data = new long double[size];
    assert (data != nullptr);
    breakCum[0] = static_cast<long double>(0);

	for (index=0; index < (periods - 1); index++)
	{
		breakCum[index+1] = breakCum[index] + shortForm[index*2+1];
        breaks[index] = static_cast<unsigned int>(breakCum[index+1] * payFreq + .0001);
		discountRates[index] = shortForm[index*2];
	}
	breaks[periods-1] = 65535;
	discountRates[periods-1] = shortForm[(periods-1)*2];
	if (timing == advance)
        data[0] = static_cast<long double>(1);

	long double r, rCum, rCumPeriod;
	unsigned int periodIndex, jumps;
	periodIndex = 0;
	rCum = 1;
	index = 0;

	while (index < (size-timing))
	{
		rCumPeriod = 1;
		jumps = 0;
		// Count # of times interest rate changes before next payment
		while (index >= breaks[periodIndex])
		{
			periodIndex++;
			jumps++;
		}
		if ((periodIndex>0) && (index == breaks[periodIndex-1]))
			// Complicated case where more than one interest rate applies between two payments
		{
			unsigned int a, b;
			a = periodIndex - jumps;
			b = a + 1;
			// Calculate discount for first applicable interest rate
			rCumPeriod *= powl(1 + discountRates[a],
					 -(breakCum[b] * payFreq - breaks[a]) / payFreq);
			// Loop through as necessary if >2 interest rate periods or
			// if >1 jumps exist
			for (unsigned int indexJump=1; indexJump<jumps; indexJump++)
			{
				a = periodIndex - jumps + indexJump;
				b = a + 1;
				rCumPeriod *= powl(1 + discountRates[a],-(breakCum[b] - breakCum[a]));
			}
			// Calculate discount for last applicable interest rate
			rCumPeriod *= powl(1 + discountRates[periodIndex],
					 -(breaks[periodIndex-1]+1 - breakCum[periodIndex]*payFreq) / payFreq);
			r = rCumPeriod;
		}
		else  // Simple case where one interest rate applies over entire period between pymts
            r = powl(1 + discountRates[periodIndex],static_cast<long double>(-1) / payFreq);
		rCum *= r;
		if ((((index + timing) % intFreq) == 0) && ((index != 0) || (intFreq == 1)))   // Normal case of constant interest
			data[index+timing] = rCum;
		else    // Defer application of interest until every intFreq pymts
			if ((timing == arrears) && (index == 0))
                data[index+timing] = static_cast<long double>(1);
			else
				data[index+timing] = data[index+timing-1];
		index++;
	}
	delete breaks;
	delete breakCum;
	delete discountRates;
}

#endif
