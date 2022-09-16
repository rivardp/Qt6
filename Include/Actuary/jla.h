// jla.h   Version 1.0  March 24, 2003
//				   2.0  August 15, 2008 - Modified to handle generational mortality
//				   2.1  February 21, 2009 - Added constructor using DOB
//				   3.0  March 1, 2009 - Eliminate shorts
//				   3.1  March 25, 2009 - Add separate mortality for spouse
//

#ifndef JLA_H
#define JLA_H

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

class jla{

private:
	long double annuityFactor;

	// Methods
	void validateJntPct(const long double &UIjntPct, errorFlag &Error);
	void validateRednOpt(const long double &UIrednOpt, errorFlag &Error);

public:
	// Constructor
	jla(const sex &sexX, const long double &UIageX, const sex &sexY,
		const long double &UIageY, const intDiscV &intRateVec,
		const long double &jntPct, const long double &UIguarPrd,
		const long double &UIdefd, const long double &UIduration,
		const mbase &UImtable, const date &doc, const intDiscV &COLA, 
		const freq &UIfreq,	const unsigned int &UIrednOpt, 
		errorFlag &Error);

	jla(const sex &sexX, const date &dobX, const sex &sexY,
		const date &dobY, const date &doc, const intDiscV &intRateVec,
		const long double &jntPct, const long double &UIguarPrd,
		const long double &UIdefd, const long double &UIduration,
		const mbase &UImtable, const intDiscV &COLA, 
		const freq &UIfreq,	const unsigned int &UIrednOpt, 
		errorFlag &Error);

	jla(const sex &sexX, const date &dobX, const sex &sexY,
		const date &dobY, const date &doc, const intDiscV &intRateVec,
		const long double &jntPct, const long double &UIguarPrd,
		const long double &UIdefd, const long double &UIduration,
		const mbase &UImtableX, const mbase &UImtableY, const intDiscV &COLA, 
		const freq &UIfreq,	const unsigned int &UIrednOpt, 
		errorFlag &Error);

	// Conversion operator
	operator long double();  // provides rhs read value

	//Destructor
	~jla();

};

jla::jla(const sex &sexX, const long double &UIageX, const sex &sexY,
	  	 const long double &UIageY, const intDiscV &intRateVec,
		 const long double &UIjntPct, const long double &UIguarPrd,
		 const long double &UIdefd, const long double &UIduration,
		 const mbase &UImtable, const date &doc, const intDiscV &COLA, 
		 const freq &UIfreq, const unsigned int &UIrednOpt, 
		 errorFlag &Error)
{
	// Verify joint annuitant percentage
	validateJntPct(UIjntPct, Error);
	if (Error) return;

	// Verify reduction option
	validateRednOpt(UIrednOpt, Error);
	if (Error) return;

	// Set up Mortality PVectors over Payment Period
	dpoint annStart(doc,UIdefd,1,0,Error);
	if (Error) return;
	mseries tPxVec(sexX, UIageX+UIdefd, UIduration, UImtable, annStart, UIfreq, Error);
	if (Error) return;
	mseries tPyVec(sexY, UIageY+UIdefd, UIduration, UImtable, annStart, UIfreq, Error);
	if (Error) return;
	
	unsigned long int maxSize = tPxVec.getSize();
	long double mduration = tPxVec.getDuration();
	if (tPyVec.getSize() < maxSize)
		tPyVec.reSize(maxSize);
	if (tPyVec.getSize() > maxSize)
	{
		mduration = tPyVec.getDuration();
		maxSize = tPyVec.getSize();
		tPxVec.reSize(maxSize);
	}

	// Set up Interest Discount PVector over Payment Period
	intDiscV payIntRateVec(intRateVec.drop(UIdefd),Error);
	if (Error) return;
    unsigned int vTfreq = static_cast<unsigned int>(UIfreq.getFreq());
	freq vTFreq(vTfreq, Error);
	if (Error) return;
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
	PVector<long double> annuityVec;
	switch(UIrednOpt)
	{
		case 1:  // Guarantee X is alive over G period
			annuityVec = vTVec * (!payCOLAVec) * vmax(guarPymtVec,
		  					tPxVec + UIjntPct * (tPyVec - tPxVec * tPyVec));
			// Can also be calculated as (but less efficient)
			//     = vTVec * (!payCOLAVec) * (vmax(guarPymtVec,tPxVec) +
			//		 UIjntPct * (tPyVec - vmax(guarPymtVec,tPxVec) * tPyVec)))
			break;
		case 2:  // Guarantee Y is alive over G period
			annuityVec = vTVec * (!payCOLAVec) * vmax(guarPymtVec,
							tPyVec + UIjntPct * (tPxVec - tPxVec * tPyVec));
			// Can also be calculated as (but less efficient)
			//     = vTVec * (!payCOLAVec) * (vmax(guarPymtVec,tPyVec) +
			//		 UIjntPct * (tPxVec - vmax(guarPymtVec,tPyVec) * tPxVec)))
			break;
		case 3:  // Guarantee both X and Y are alive over G period
			annuityVec = vTVec * (!payCOLAVec) * vmax(guarPymtVec,
							tPxVec * tPyVec + UIjntPct * (tPxVec + tPyVec - 2 * tPxVec * tPyVec));
			// Can substitute 'tPxVec + tPyVec - 2 * tPxVec * tPyVec' with
			//                '(1 - (1 - tPxVec) * (1 - tPyVec)) - tPxVec * tPyVec'
			break;
		case 4:  // Guarantee Y is alive over G period
			annuityVec = vTVec * (!payCOLAVec) * (tPxVec + UIjntPct *
							(vmax(guarPymtVec,tPyVec) - tPxVec * vmax(guarPymtVec,tPyVec)));
			break;
		case 5:  // Guarantee X is alive over G period
			annuityVec = vTVec * (!payCOLAVec) * (tPyVec + UIjntPct *
							(vmax(guarPymtVec,tPxVec) - tPyVec * vmax(guarPymtVec,tPxVec)));
			break;
		case 6:  // Guarantee either X or Y is alive over G period
			annuityVec = vTVec * (!payCOLAVec) * (tPxVec * tPyVec + UIjntPct *
							(vmax(guarPymtVec,(1 - (1 - tPxVec) * (1 - tPyVec))) - tPxVec * tPyVec));
			break;
	}
	long double annuity = annuityVec.sum() / Freq;

	//Check and set up Mortality discount on deferral
	mpoint tPx;
	mpoint tPy;
	if ((tPxVec.getDiscountOnDeferral() == mbase::interestAndMortalityX) ||
		(tPxVec.getDiscountOnDeferral() == mbase::interestAndMortalityXY))
	{
		mpoint ttPx(sexX, UIageX, UIdefd, UImtable, doc, Error);
		if (Error) return;
		tPx = ttPx;
	}
	if ((tPyVec.getDiscountOnDeferral() == mbase::interestAndMortalityY) ||
		(tPyVec.getDiscountOnDeferral() == mbase::interestAndMortalityXY))
	{
		mpoint ttPy(sexY, UIageY, UIdefd, UImtable, doc, Error);
		if (Error) return;
		tPy = ttPy;
	}

	//Check and set up Interest discount on deferral
	vpoint vTDiscount(intRateVec, UIdefd, Error);
	if (Error) return;
	vpoint vTIndex(COLA, UIdefd, Error);
	if (Error) return;
	long double vT = vTDiscount / vTIndex;

	// Calculate final annuity factor (rounded to 4 decimal places)
    annuityFactor = .0001 * static_cast<long int>(vT * tPx * tPy * annuity * 10000 + 0.5);
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

jla::jla(const sex &sexX, const date &dobX, const sex &sexY,
	  	 const date &dobY, const date &doc, const intDiscV &intRateVec,
		 const long double &UIjntPct, const long double &UIguarPrd,
		 const long double &UIdefd, const long double &UIduration,
		 const mbase &UImtable, const intDiscV &COLA, 
		 const freq &UIfreq, const unsigned int &UIrednOpt, 
		 errorFlag &Error)
{
	jla same(sexX, dobX, sexY, dobY, doc, intRateVec, UIjntPct, UIguarPrd,UIdefd, 
		     UIduration, UImtable, UImtable, COLA, UIfreq, UIrednOpt,Error);

	annuityFactor = same;
}

jla::jla(const sex &sexX, const date &dobX, const sex &sexY,
	  	 const date &dobY, const date &doc, const intDiscV &intRateVec,
		 const long double &UIjntPct, const long double &UIguarPrd,
		 const long double &UIdefd, const long double &UIduration,
		 const mbase &UImtableX, const mbase &UImtableY, const intDiscV &COLA, 
		 const freq &UIfreq, const unsigned int &UIrednOpt, 
		 errorFlag &Error)
{
	// Verify joint annuitant percentage
	validateJntPct(UIjntPct, Error);
	if (Error) return;

	// Verify reduction option
	validateRednOpt(UIrednOpt, Error);
	if (Error) return;

	// Set up Mortality PVectors over Payment Period
	dpoint annStart(doc,UIdefd,1,0,Error);
	if (Error) return;
	mseries tPxVec(sexX, dobX, annStart, UIduration, UImtableX, UIfreq, Error);
	if (Error) return;
	mseries tPyVec(sexY, dobY, annStart, UIduration, UImtableY, UIfreq, Error);
	if (Error) return;
	
	unsigned long int maxSize = tPxVec.getSize();
	long double mduration = tPxVec.getDuration();
	if (tPyVec.getSize() < maxSize)
		tPyVec.reSize(maxSize);
	if (tPyVec.getSize() > maxSize)
	{
		mduration = tPyVec.getDuration();
		maxSize = tPyVec.getSize();
		tPxVec.reSize(maxSize);
	}

	// Set up Interest Discount PVector over Payment Period
	intDiscV payIntRateVec(intRateVec.drop(UIdefd),Error);
	if (Error) return;
	unsigned int vTfreq = (unsigned int) UIfreq.getFreq();
	freq vTFreq(vTfreq, Error);
	if (Error) return;
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
		guarPymtVec = (long double)1;
		guarPymtVec.reSize(pymtNum);
		}

	// Set up COLA
	intDiscV payCOLA(COLA.drop(UIdefd));
	vseries payCOLAVec(payCOLA, mduration, UIfreq, Error);
	if (Error) return;

	// Calculate Payment Annuity Factor
	PVector<long double> annuityVec;
	switch(UIrednOpt)
	{
		case 1:  // Guarantee X is alive over G period
			annuityVec = vTVec * (!payCOLAVec) * vmax(guarPymtVec,
		  					tPxVec + UIjntPct * (tPyVec - tPxVec * tPyVec));
			// Can also be calculated as (but less efficient)
			//     = vTVec * (!payCOLAVec) * (vmax(guarPymtVec,tPxVec) +
			//		 UIjntPct * (tPyVec - vmax(guarPymtVec,tPxVec) * tPyVec)))
			break;
		case 2:  // Guarantee Y is alive over G period
			annuityVec = vTVec * (!payCOLAVec) * vmax(guarPymtVec,
							tPyVec + UIjntPct * (tPxVec - tPxVec * tPyVec));
			// Can also be calculated as (but less efficient)
			//     = vTVec * (!payCOLAVec) * (vmax(guarPymtVec,tPyVec) +
			//		 UIjntPct * (tPxVec - vmax(guarPymtVec,tPyVec) * tPxVec)))
			break;
		case 3:  // Guarantee both X and Y are alive over G period
			annuityVec = vTVec * (!payCOLAVec) * vmax(guarPymtVec,
							tPxVec * tPyVec + UIjntPct * (tPxVec + tPyVec - 2 * tPxVec * tPyVec));
			// Can substitute 'tPxVec + tPyVec - 2 * tPxVec * tPyVec' with
			//                '(1 - (1 - tPxVec) * (1 - tPyVec)) - tPxVec * tPyVec'
			break;
		case 4:  // Guarantee Y is alive over G period
			annuityVec = vTVec * (!payCOLAVec) * (tPxVec + UIjntPct *
							(vmax(guarPymtVec,tPyVec) - tPxVec * vmax(guarPymtVec,tPyVec)));
			break;
		case 5:  // Guarantee X is alive over G period
			annuityVec = vTVec * (!payCOLAVec) * (tPyVec + UIjntPct *
							(vmax(guarPymtVec,tPxVec) - tPyVec * vmax(guarPymtVec,tPxVec)));
			break;
		case 6:  // Guarantee either X or Y is alive over G period
			annuityVec = vTVec * (!payCOLAVec) * (tPxVec * tPyVec + UIjntPct *
							(vmax(guarPymtVec,(1 - (1 - tPxVec) * (1 - tPyVec))) - tPxVec * tPyVec));
			break;
	}
	long double annuity = annuityVec.sum() / Freq;

	//Check and set up Mortality discount on deferral
	mpoint tPx;
	mpoint tPy;
	if ((tPxVec.getDiscountOnDeferral() == mbase::interestAndMortalityX) ||
		(tPxVec.getDiscountOnDeferral() == mbase::interestAndMortalityXY))
	{
		mpoint ttPx(sexX, dobX, doc, UIdefd, UImtableX, Error);
		if (Error) return;
		tPx = ttPx;
	}
	if ((tPyVec.getDiscountOnDeferral() == mbase::interestAndMortalityY) ||
		(tPyVec.getDiscountOnDeferral() == mbase::interestAndMortalityXY))
	{
		mpoint ttPy(sexY, dobY, doc, UIdefd, UImtableY, Error);
		if (Error) return;
		tPy = ttPy;
	}

	//Check and set up Interest discount on deferral
	vpoint vTDiscount(intRateVec, UIdefd, Error);
	if (Error) return;
	vpoint vTIndex(COLA, UIdefd, Error);
	if (Error) return;
	long double vT = vTDiscount / vTIndex;

	// Calculate final annuity factor (rounded to 4 decimal places)
    annuityFactor = .0001 * static_cast<long int>(vT * tPx * tPy * annuity * 10000 + 0.5);
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

void jla::validateJntPct(const long double &UIjntPct, errorFlag &Error)
{
	if (UIjntPct < 0)
		Error = 25;
	if (UIjntPct > 5)
		Error = 26;
}

void jla::validateRednOpt(const long double &UIrednOpt, errorFlag &Error)
{
	unsigned int option = (unsigned int)(UIrednOpt + .0001);
	if ((option < 1) || (option > 6))
		Error = 27;
}

jla::operator long double()
{
	return annuityFactor;
}

jla::~jla()
{
}

#endif
