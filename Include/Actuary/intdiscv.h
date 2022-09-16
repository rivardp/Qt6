// intdiscv.h		Version 1.0  March 24, 2003
//						    2.0  March 1, 2009 - Eliminate shorts 
//

#ifndef INTDISCV_H
#define INTDISCV_H

#ifndef PVector_H
#include "../Actuary/PVector.h"
#endif

#ifndef ERRORFLAG_H
#include "../Actuary/errorFlag.h"
#endif

class intDiscV : public PVector<long double> {

private:
	unsigned int numPeriods; // Number of different interest rates

	void validateInterestRate(const PVector<long double> &shortform, errorFlag &Error) const;
	void setInterestRate(const PVector<long double> &shortform);

public:
	// Constructors
	intDiscV();
	intDiscV(const intDiscV &idv);
	intDiscV(const long double intRateScalar, errorFlag &Error);
	intDiscV(const long double *intRateArray, const int arraySize, errorFlag &Error);
	intDiscV(const PVector<long double> &intRateVec, errorFlag &Error);

	//Destructor
	~intDiscV();

	//Methods
	int getPeriods() const;
	intDiscV drop(long double period) const;

	//Assignment Operators
	intDiscV& operator=(const intDiscV &idv);

};

intDiscV::intDiscV() : PVector<long double>(), numPeriods(0)
{
}


intDiscV::intDiscV(const intDiscV &rhs) : PVector<long double>(rhs.getSize()),
				 					      numPeriods(rhs.getPeriods())
{
	for (unsigned long int i=0; i < size; i++)
		data[i] = rhs.data[i];
}

intDiscV::intDiscV(const long double intRateScalar, errorFlag &Error)
{
	PVector<long double> tempVec(1);
	tempVec = intRateScalar;
	validateInterestRate(tempVec, Error);
	if (!Error) setInterestRate(tempVec);
}

intDiscV::intDiscV(const long double *intRateArray, const int arraySize, errorFlag &Error)
{
    PVector<long double> tempVec(intRateArray,static_cast<unsigned long>(arraySize));
	validateInterestRate(tempVec, Error);
	if (!Error) setInterestRate(tempVec);
}

intDiscV::intDiscV(const PVector<long double> &intRateVec, errorFlag &Error)
{
	validateInterestRate(intRateVec, Error);
	if (!Error) setInterestRate(intRateVec);
}

intDiscV::~intDiscV()
{
}

void intDiscV::validateInterestRate(const PVector<long double> &shortForm, errorFlag &Error) const
{
	if (shortForm.getSize()%2 != 1)
		Error = 3; // Interest/Indexing PVector elements not odd
	if (shortForm.minValue() < 0)
		Error = 5; // Invalid interest/indexing rate parameter

	unsigned long int maxIndex=shortForm.getSize();
	for (unsigned long int index=0; index<maxIndex; index+=2){
		if (shortForm[index]>1)
			Error = 2; // Interest/Indexing rate cannot be > 100%
		}
}

void intDiscV::setInterestRate(const PVector<long double> &intRate)
{
	size = intRate.getSize();
	data = new long double[size];
    assert (data != nullptr);
	for (unsigned long int i=0; i<size ; i++)
			data[i]=intRate[i];
	if (size == 0)
		numPeriods = 0;
	else
        numPeriods = 1 + static_cast<unsigned int>(size / 2);
}


int intDiscV::getPeriods() const
{
    return static_cast<int>(numPeriods);
}

intDiscV intDiscV::drop(long double period) const
{
	 if (numPeriods == 1)
	 {
		 intDiscV newidv(*this);
		 return newidv;
	 }

	 unsigned int periodsDropped = 0;
	 long double cumDurn = 0;

	 while((periodsDropped<(numPeriods-1)) && (cumDurn <= period))
		 {
		 cumDurn+=data[2*periodsDropped+1];
		 periodsDropped++;
		 }
		 if (cumDurn > period)
			  periodsDropped--;

		 unsigned long int newSize = (numPeriods-periodsDropped)*2 - 1;
		 PVector<long double> tempVec(newSize);
		 for (unsigned long int index = 0; index<newSize; index++)
				tempVec[index] = data[index + periodsDropped*2];
		 if (newSize > 1)
				tempVec[1] = cumDurn - period;
		 errorFlag Error;
		 Error = 0;
		 intDiscV newidv(tempVec, Error);
		 return newidv;
}

intDiscV& intDiscV::operator=(const intDiscV &rhs)
{
	if (this == &rhs)return *this;

	delete[]data;
	size = rhs.getSize();
	data = new long double[size];
    assert (data != nullptr);
	for (unsigned long int i=0; i < size; i++)
		data[i] = rhs.data[i];
    numPeriods = static_cast<unsigned int>(rhs.getPeriods());
	return *this;
}

#endif


