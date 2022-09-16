// sla.h   Version 1.0  March 24, 2003
//				   2.0  August 15, 2008 - Modified to handle generational mortality
//				   2.1  February 21, 2008 - Add constructor using DOB
//				   3.0  March 1, 2009 - Eliminate shorts
//

#ifndef SLA_H
#define SLA_H

#ifndef PVector_H
#include "../Actuary/PVector.h"
#endif

#ifndef VSERIES_H
#include "../Actuary/vseries.h"
#endif

#ifndef VPOINT_H
#include "../Actuary/vpoint.h"
#endif

#ifndef MSERIES_H
#include "../Actuary/mseries.h"
#endif

#ifndef MPOINT_H
#include "../Actuary/mpoint.h"
#endif

#ifndef SEX_H
#include "../Actuary/sex.h"
#endif

#ifndef FREQ_H
#include "../Actuary/freq.h"
#endif

#ifndef ERRORFLAG_H
#include "../Actuary/errorFlag.h"
#endif

class sla{

private:
	long double annuityFactor;

public:
	// Constructor
	sla(const sex &sexX, const long double &UIage, const intDiscV &intRateVec,
		const long double &UIguarPrd, const long double &UIdefd,
		const long double &UIduration, const mbase &UImtable, const date &doc,
		const intDiscV &COLA, const freq &UIfreq, errorFlag &Error);

	sla(const sex &sexX, const date &dob, const date &doc, 
		const intDiscV &intRateVec, const long double &UIguarPrd, 
		const long double &UIdefd, const long double &UIduration, 
		const mbase &UImtable, const intDiscV &COLA, 
		const freq &UIfreq, errorFlag &Error);

	// Conversion operator
	operator long double();  // provides rhs read value

	//Destructor
	~sla();

};

sla::sla(const sex &sexX, const long double &UIage, const intDiscV &intRateVec,
		 const long double &UIguarPrd, const long double &UIdefd,
		 const long double &UIduration, const mbase &UImtable, const date &doc,
		 const intDiscV &COLA, const freq &UIfreq, errorFlag &Error)
{
	// Set up Mortality PVector over Payment Period
	dpoint annStart(doc,UIdefd,1,0,Error);
	if (Error) return;
	mseries tPxVec(sexX, UIage+UIdefd, UIduration, UImtable, annStart, UIfreq, Error);
	if (Error) return;

	// Set up Interest Discount PVector over Payment Period
	intDiscV payIntRateVec(intRateVec.drop(UIdefd),Error);
	if (Error) return;
    unsigned int vTfreq = static_cast<unsigned int>(UIfreq.getFreq());
	freq vTFreq(vTfreq, Error);
	if (Error) return;
	long double mduration = tPxVec.getDuration();
	vseries vTVec(payIntRateVec, mduration, vTFreq, Error);
	if (Error) return;

	// Set up Guarantee Period
	if (UIguarPrd < 0)
	{
		Error = 50;
		return;
	}
	unsigned long int pymtNum = vTVec.getSize();
	unsigned int Freq = UIfreq.getPayFreq();
	unsigned int guarPymts;
	if (UIfreq.getTiming()== freq::advance)
        guarPymts = 1 + static_cast<unsigned int>(Freq * UIguarPrd - .0001);
	else
        guarPymts = static_cast<unsigned int>(Freq * UIguarPrd + .0005);
	PVector<long double> guarPymtVec(pymtNum);
	if (guarPymts > 0)
		{
		guarPymtVec.reSize(guarPymts);
        guarPymtVec = static_cast<long double>(1);
		guarPymtVec.reSize(pymtNum);
		}

	// Set up COLA
	intDiscV payCOLA(COLA.drop(UIdefd));
	vseries payCOLAVec(payCOLA, mduration, UIfreq, Error);
	if (Error) return;

	// Calculate Payment Annuity Factor
	PVector<long double> annuityVec = vTVec * vmax(tPxVec, guarPymtVec) * !payCOLAVec;
	long double annuity = annuityVec.sum() / Freq;

	//Check and set up Mortality discount on deferral
	mpoint tPx;
	if ((tPxVec.getDiscountOnDeferral() == mbase::interestAndMortalityX) ||
		(tPxVec.getDiscountOnDeferral() == mbase::interestAndMortalityXY))
	{
		mpoint ttPx(sexX, UIage, UIdefd, UImtable, doc, Error);
		if (Error) return;
		tPx = ttPx;
	}
	
	//Check and set up Interest discount on deferral
	vpoint vTDiscount(intRateVec, UIdefd, Error);
	if (Error) return;
	vpoint vTIndex(COLA, UIdefd, Error);
	if (Error) return;
	long double vT = vTDiscount / vTIndex;

	// Calculate final annuity factor (rounded to 4 decimal places)
    annuityFactor = .0001 * static_cast<long int>(vT * tPx * annuity * 10000 + 0.5);
	
/*	if (TESTMODE)
	{
		std::ofstream myfile;
		myfile.open("dump.txt");
		myfile << "Writing this to a file.\n";

		myfile << "Sex:     " << sexX << std::endl;
		myfile << "Age:     " << UIage << std::endl;
		myfile << "Ret Age: " << UIage + UIdefd << std::endl;
		myfile << "GP:      " << UIguarPrd << std::endl;
		myfile << "# Pymts: " << pymtNum << std::endl;
		myfile << "vT:      " << vT << std::endl;
		myfile << "tPx:     " << tPx << std::endl;
		myfile << "Annuity: " << annuity << std::endl;
		myfile << "APV:     " << annuityFactor << std::endl << std::endl;

		myfile << "Pymt        Age      vT       tPx       GP      COLA      APV" << std::endl;
		myfile << "----        ---    ------    ------   ------   ------    ------" << std::endl;
		myfile.setf(std::ios::showpoint);
		for (int index = 0; index < vTVec.getSize(); index++)
		{
			myfile << std::setw(3) << index + 1 << std::setw(6) << "  ";
			myfile << UIage + UIdefd + float(index) / Freq << "  " << vTVec[index];
			myfile << "  " << tPxVec[index] << "  " << guarPymtVec[index] << "  ";
			myfile << ((!payCOLAVec)[index]) << "  " << annuityVec[index] << std::endl;
		}
		myfile.close();
	}*/
}

sla::sla(const sex &sexX, const date &dob, const date &doc, 
		 const intDiscV &intRateVec, const long double &UIguarPrd, 
		 const long double &UIdefd, const long double &UIduration, 
		 const mbase &UImtable, const intDiscV &COLA, const freq &UIfreq, errorFlag &Error)
{
	// Set up Mortality PVector over Payment Period
	dpoint annStart(doc,UIdefd,1,0,Error);
	if (Error) return;
	mseries tPxVec(sexX, dob, annStart, UIduration, UImtable, UIfreq, Error);
	if (Error) return;

	// Set up Interest Discount PVector over Payment Period
	intDiscV payIntRateVec(intRateVec.drop(UIdefd),Error);
	if (Error) return;
    unsigned int vTfreq = static_cast<unsigned int>(UIfreq.getFreq());
	freq vTFreq(vTfreq, Error);
	if (Error) return;
	long double mduration = tPxVec.getDuration();
	vseries vTVec(payIntRateVec, mduration, vTFreq, Error);
	if (Error) return;

	// Set up Guarantee Period
	if (UIguarPrd < 0)
	{
		Error = 50;
		return;
	}
	unsigned long int pymtNum = vTVec.getSize();
	unsigned int Freq = UIfreq.getPayFreq();
	unsigned int guarPymts;
	if (UIfreq.getTiming()== freq::advance)
        guarPymts = 1 + static_cast<unsigned int>(Freq * UIguarPrd - .0001);
	else
        guarPymts = static_cast<unsigned int>(Freq * UIguarPrd + .0005);
	PVector<long double> guarPymtVec(pymtNum);
	if (guarPymts > 0)
		{
		guarPymtVec.reSize(guarPymts);
        guarPymtVec = static_cast<long double>(1);
		guarPymtVec.reSize(pymtNum);
		}

	// Set up COLA
	intDiscV payCOLA(COLA.drop(UIdefd));
	vseries payCOLAVec(payCOLA, mduration, UIfreq, Error);
	if (Error) return;

	// Calculate Payment Annuity Factor
	PVector<long double> annuityVec = vTVec * vmax(tPxVec, guarPymtVec) * !payCOLAVec;
	long double annuity = annuityVec.sum() / Freq;

	//Check and set up Mortality discount on deferral
	mpoint tPx;
	if ((tPxVec.getDiscountOnDeferral() == mbase::interestAndMortalityX) ||
		(tPxVec.getDiscountOnDeferral() == mbase::interestAndMortalityXY))
	{
		mpoint ttPx(sexX, dob, doc, UIdefd, UImtable, Error);
		if (Error) return;
		tPx = ttPx;
	}
	
	//Check and set up Interest discount on deferral
	vpoint vTDiscount(intRateVec, UIdefd, Error);
	if (Error) return;
	vpoint vTIndex(COLA, UIdefd, Error);
	if (Error) return;
	long double vT = vTDiscount / vTIndex;

	// Calculate final annuity factor (rounded to 4 decimal places)
    annuityFactor = .0001 * static_cast<long int>(vT * tPx * annuity * 10000 + 0.5);
/*
	std::cout << "Sex:     " << sexX << std::endl;
	std::cout << "Age:     " << UIage << std::endl;
	std::cout << "Ret Age: " << UIage + UIdefd << std::endl;
	std::cout << "GP:      " << UIguarPrd << std::endl;
	std::cout << "# Pymts: " << pymtNum << std::endl;
	std::cout << "vT:      " << vT << std::endl;
	std::cout << "tPx:     " << tPx << std::endl;
	std::cout << "Annuity: " << annuity << std::endl;
	std::cout << "APV:     " << annuityFactor << std::endl << std::endl;

	std::cout << "Pymt        Age      vT       tPx       GP      COLA      APV" << std::endl;
	std::cout << "----        ---    ------    ------   ------   ------    ------" << std::endl;
	std::cout.setf(std::ios::showpoint);
	for (int index=0; index<vTVec.getSize(); index++)
	{
		std::cout << std::setw(3) << index + 1 << std::setw(6) << "  ";
		std::cout << UIage + UIdefd + float(index) / Freq << "  " << vTVec[index];
		std::cout << "  " << tPxVec[index] << "  " << guarPymtVec[index] << "  ";
		std::cout << ((!payCOLAVec)[index]) << "  " << annuityVec[index] << std::endl;
	}
*/
}

sla::operator long double()
{
	return annuityFactor;
}

sla::~sla()
{
}

#endif
