// freq.h		Version 2.0  February 9, 2003
//					    3.0  March 1, 2009 - Eliminate shorts
//

#ifndef FREQ_H
#define FREQ_H

#ifndef PVector_H
#include "../Actuary/PVector.h"
#endif

#ifndef ERRORFLAG_H
#include "../Actuary/errorFlag.h"
#endif

static const int frequencies[12] = {1,2,12,24,26,52,101,102,112,124,126,152};
static const PVector<int> validFreq(frequencies,12);

class freq {

public:
	// Type define
	enum annuityType {arrears, advance};

	// Constructors
	freq();
	freq(const double frequency, errorFlag &Error);
	freq(const freq &rhs);

	// Methods
	unsigned int getPayFreq() const;
	unsigned int getIntFreq() const;
	double getFreq() const;
	freq::annuityType getTiming() const;
	
	// Assignment Operator
	freq& operator=(const freq &frequency);

	// Destructor
	~freq();

protected:
	unsigned int payFreq;		// Number of payments per year
	unsigned int intFreq;		// Interest is accumulated every intFreq payment
	double payIntFreq;				// Combination of payFreq and intFreq
	annuityType timing;				// Annuity immediate or annuity due

	void validateFrequency(const double frequency, errorFlag &Error) const;
	void setFrequency(const double frequency);

};

freq::freq(): payIntFreq(12), payFreq(12), intFreq(1), timing(advance)
{
}

freq::freq(const double frequency, errorFlag &Error)
{
	validateFrequency(frequency, Error);
	if (!Error)
		setFrequency(frequency);
}

freq::freq(const freq &rhs)
{
	payFreq = rhs.getPayFreq();
	intFreq = rhs.getIntFreq();
	payIntFreq = rhs.getFreq();
	timing = rhs.getTiming();
}

freq& freq::operator=(const freq &rhs)
{
	if (this == &rhs) return *this;
	payFreq = rhs.getPayFreq();
	intFreq = rhs.getIntFreq();
	payIntFreq = rhs.getFreq();
	timing = rhs.getTiming();
	return *this;
}

freq::~freq()
{
}

void freq::validateFrequency(const double frequency, errorFlag &Error) const
{
    int tpayfreq = static_cast<int>(frequency);
    int tintfreq = static_cast<int>(1000*(frequency - tpayfreq) + 0.0001);

	if (!validFreq.isFound(tpayfreq))
		Error = 15; //Invalid payment frequency parameter
	
	if ((frequency != tpayfreq) &&
		  (!validFreq.isFound(tintfreq) || (((tpayfreq % 100) % tintfreq) != 0)))
		Error = 16; //Invalid interest frequency parameter
}

void freq::setFrequency(const double frequency)
{
    timing = ((static_cast<unsigned int>(frequency)) > 100) ? arrears : advance;
	payIntFreq = frequency;
    payFreq = (static_cast<unsigned int>(frequency)) % 100;
    if (frequency == static_cast<int>(frequency))
        intFreq = static_cast<unsigned int>(1);
	else
        intFreq = static_cast<unsigned int>((frequency-static_cast<int>(frequency))*1000 + 0.0001);
}

unsigned int freq::getPayFreq() const
{
	return payFreq;
}

unsigned int freq::getIntFreq() const
{
	return intFreq;
}

double freq::getFreq() const
{
	return payIntFreq;
}

freq::annuityType freq::getTiming() const
{
	return timing;
}

#endif
