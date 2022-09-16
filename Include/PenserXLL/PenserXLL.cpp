// Penser XLL.cpp		Version 2.0 - May 16, 2015
//

#include <xlw\xlw.h>
#include <vector>
#include <tchar.h>

#define TESTMODE 0
#ifdef TESTMODE
#include <ostream>
#include <fstream>
#endif

#ifndef PENSER_TOOLKIT_H
#include <penser_toolkit.h>
#endif

#include "XLL Utility.h"

using namespace xlw;

extern "C"
{
	// vT Function

	LPXLFOPER EXCEL_EXPORT xlvT(LPXLFOPER inInterest, LPXLFOPER inPeriod)
	{
		EXCEL_BEGIN;
		XlfOper Interest(inInterest);
		XlfOper Period(inPeriod);

		errorFlag Error;
		intDiscV idv;

		// Validate period parameter
		if (!Period.IsNumber())
		{
			Error = 6; //Invalid period parameter
			return XlfOper(Error.getErrorDesc());
		}

		// Validate IDV parameter
		if (!IsNumVector(Interest))
		{
			Error = 5; // Invalid interest/indexing rate parameter
			return XlfOper(Error.getErrorDesc());
		}

		// Check for interest type and set idv accordingly
		if (Interest.IsNumber())
		{
			intDiscV tidv(Interest.AsDouble(), Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			idv = tidv;
		}
		else  // Must be an array
		{
			unsigned short int vectorSize = Interest.rows() * Interest.columns();
			std::vector<double> stdInputVector(vectorSize);
			vector<long double> penserInputVector(vectorSize);
			stdInputVector = Interest.AsDoubleVector();
			for (unsigned short int i = 0; i < vectorSize; i++)
				penserInputVector[i] = (long double)stdInputVector[i];
			intDiscV tidv(penserInputVector, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			idv = tidv;
		}

		vpoint vp(idv, Period.AsDouble(), Error);
		if (Error)
			return XlfOper(Error.getErrorDesc());

		double ret = (double)vp;
		return XlfOper(ret);
		EXCEL_END;
	}

	// tPx Function

	LPXLFOPER EXCEL_EXPORT xltPx(LPXLFOPER inSex, LPXLFOPER inAge, LPXLFOPER inPeriod, LPXLFOPER inMortality, LPXLFOPER inCalcDate)
	{
		EXCEL_BEGIN;
		XlfOper Sex(inSex);
		XlfOper Age(inAge);
		XlfOper Period(inPeriod);
		XlfOper Mortality(inMortality);
		XlfOper CalcDate(inCalcDate);

		errorFlag Error;
		sex sexX;
		mbase mtable;
		date valCalcDate;

		// Validate Sex parameter
		if (Sex.IsString())
		{
			PString sexPString = Sex.AsString();
			unsigned short int strlength = sexPString.getLength();
			char *sexString = new char[strlength + 1];
			for (unsigned short int i = 0; i < strlength; i++)
				sexString[i] = (char)sexPString[i];
			sexString[strlength] = '\0';
			sex tsexX(sexString, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			sexX = tsexX;
		}
		else
		{
			if (Sex.IsNumber())
			{
				sex tsexX(Sex.AsDouble(), Error);
				if (Error)
					return XlfOper(Error.getErrorDesc());
				sexX = tsexX;
			}
			else
			{
				if (Sex.IsMulti())  // User Defined Lx - Includes setup of mortality
				{
					if (!IsNumVector(Sex))
					{
						Error = 20; // Invalid sex parameter
						return XlfOper(Error.getErrorDesc());
					}
					unsigned short int vectorSize = Sex.rows() * Sex.columns();
					std::vector<double> stdInputVector(vectorSize);
					vector<long double> penserInputVector(vectorSize);
					stdInputVector = Sex.AsDoubleVector();
					for (unsigned short int i = 0; i < vectorSize; i++)
						penserInputVector[i] = (long double)stdInputVector[i];
					sex tsexX(penserInputVector, Error);
					if (Error)
						return XlfOper(Error.getErrorDesc());
					sexX = tsexX;
					mbase tmtable; // Default table which will be ignored
					mtable = tmtable;
				}
				else
				{
					Error = 20;  // Invalid sex parameter
					return XlfOper(Error.getErrorDesc());
				}
			}
		}

		// Validate Age parameter
		if (!Age.IsNumber())
		{
			Error = 37; //Invalid age parameter
			return XlfOper(Error.getErrorDesc());
		}

		// Validate period parameter
		if (!Period.IsNumber())
		{
			Error = 6; //Invalid period parameter
			return XlfOper(Error.getErrorDesc());
		}

		// Validate mortality table
		if (sexX != 99)
		{
			PString valMortality = "UP94";
			if (!(Mortality.IsMissing() || Mortality.IsNil()))
			{
				if (Mortality.IsString())
				{
					valMortality = PString(Mortality.AsString());
					valMortality = valMortality.upper();
				}
				else
				{
					Error = 30; // Invalid mortality table
					return XlfOper(Error.getErrorDesc());
				}
			}
			unsigned short int strlength = valMortality.getLength();
			char *mtString = new char[strlength + 1];
			for (unsigned short int i = 0; i < strlength; i++)
				mtString[i] = (char)valMortality[i];
			mtString[strlength] = '\0';
			mbase tmtable(mtString, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			mtable = tmtable;
		}

		//	Check for Calculation Date for Generational Mortality
		if (CalcDate.IsMissing() || CalcDate.IsNil())
		{
			if ((mtable.getMortalityType() == mbase::generational) || (mtable.getMortalityType() == mbase::generational2D))
			{
				Error = 39;
				return XlfOper(Error.getErrorDesc());
			}
			date defaultCalcDate(mtable.getBaseYear(), 1, 1, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			valCalcDate = defaultCalcDate;
		}
		else
		{
			if (CalcDate.IsNumber())
			{
				date inputCalcDate(ExcelDateConversion(CalcDate.AsDouble(), 1), Error);
				if (Error)
					return XlfOper(Error.getErrorDesc());
				valCalcDate = inputCalcDate;
			}
			else
			{
				Error = 47; // Invalid calculation date
				return XlfOper(Error.getErrorDesc());
			}
		}

		mpoint mp(sexX, Age.AsDouble(), Period.AsDouble(), mtable, valCalcDate, Error);
		if (Error)
			return XlfOper(Error.getErrorDesc());

		double ret = (double)mp;
		return XlfOper(ret);
		EXCEL_END;
	}
	
	// LIFE Function

	LPXLFOPER EXCEL_EXPORT xlLIFE(LPXLFOPER inSex, LPXLFOPER inAge, LPXLFOPER inInterest, LPXLFOPER inGuarantee, LPXLFOPER inDeferral, 
		LPXLFOPER inPaid, LPXLFOPER inMortality, LPXLFOPER inCalcDate, LPXLFOPER inIndexing, LPXLFOPER inFrequency)
	{
		EXCEL_BEGIN;
		XlfOper Sex(inSex);
		XlfOper Age(inAge);
		XlfOper Interest(inInterest);
		XlfOper Guarantee(inGuarantee);
		XlfOper Deferral(inDeferral);
		XlfOper Paid(inPaid);
		XlfOper Mortality(inMortality);
		XlfOper CalcDate(inCalcDate);
		XlfOper Indexing(inIndexing);
		XlfOper Frequency(inFrequency);

		errorFlag Error;
		sex sexX;
		mbase mtable;
		intDiscV idv;
		intDiscV colav;
		date valCalcDate;

		// Validate Sex parameter
		if (Sex.IsString())
		{
			PString sexPString = Sex.AsString();
			unsigned short int strlength = sexPString.getLength();
			char *sexString = new char[strlength + 1];
			for (unsigned short int i = 0; i < strlength; i++)
				sexString[i] = (char)sexPString[i];
			sexString[strlength] = '\0';
			sex tsexX(sexString, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			sexX = tsexX;
		}
		else
		{
			if (Sex.IsNumber())
			{
				sex tsexX(Sex.AsDouble(), Error);
				if (Error)
					return XlfOper(Error.getErrorDesc());
				sexX = tsexX;
			}
			else
			{
				if (Sex.IsMulti())  // User Defined Lx - Includes setup of mortality
				{
					if (!IsNumVector(Sex))
					{
						Error = 20; // Invalid sex parameter
						return XlfOper(Error.getErrorDesc());
					}
					unsigned short int vectorSize = Sex.rows() * Sex.columns();
					std::vector<double> stdInputVector(vectorSize);
					vector<long double> penserInputVector(vectorSize);
					stdInputVector = Sex.AsDoubleVector();
					for (unsigned short int i = 0; i < vectorSize; i++)
						penserInputVector[i] = (long double)stdInputVector[i];
					sex tsexX(penserInputVector, Error);
					if (Error)
						return XlfOper(Error.getErrorDesc());
					sexX = tsexX;
					mbase tmtable; // Default table which will be ignored
					mtable = tmtable;
				}
				else
				{
					Error = 20;  // Invalid sex parameter
					return XlfOper(Error.getErrorDesc());
				}
			}
		}

		// Validate Age parameter
		if (!Age.IsNumber())
		{
			Error = 37; //Invalid age parameter
			return XlfOper(Error.getErrorDesc());
		}

		// Validate IDV parameter
		if (!IsNumVector(Interest))
		{
			Error = 5; // Invalid interest/indexing rate parameter
			return XlfOper(Error.getErrorDesc());
		}

		// Check for interest type and set idv accordingly
		if (Interest.IsNumber())
		{
			intDiscV tidv(Interest.AsDouble(), Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			idv = tidv;
		}
		else  // Must be an array
		{
			unsigned short int vectorSize = Interest.rows() * Interest.columns();
			std::vector<double> stdInputVector(vectorSize);
			vector<long double> penserInputVector(vectorSize);
			stdInputVector = Interest.AsDoubleVector();
			for (unsigned short int i = 0; i < vectorSize; i++)
				penserInputVector[i] = (long double)stdInputVector[i];
			intDiscV tidv(penserInputVector, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			idv = tidv;
		}

		// Verify and set guarantee period
		long double valGuarantee = 0;
		if (!(Guarantee.IsMissing() || Guarantee.IsNil()))
		{
			if (Guarantee.IsNumber())
				valGuarantee = Guarantee.AsDouble();
			else
			{
				Error = 14; // Invalid Guarantee parameter
				return XlfOper(Error.getErrorDesc());
			}
		}

		// Verify and set guarantee period
		long double valDeferral = 0;
		if (!(Deferral.IsMissing() || Deferral.IsNil()))
		{
			if (Deferral.IsNumber())
				valDeferral = Deferral.AsDouble();
			else
			{
				Error = 13; // Invalid Deferral parameter
				return XlfOper(Error.getErrorDesc());
			}
		}

		// Verify and set paid period
		long double valPaid = 999;
		if (!(Paid.IsMissing() || Paid.IsNil()))
		{
			if (Paid.IsNumber())
				valPaid = Paid.AsDouble();
			else
			{
				Error = 7; // Invalid Payment period parameter
				return XlfOper(Error.getErrorDesc());
			}
		}

		// Validate mortality table - Run even if UserDefinedMortality is being used
		// in order to setup pickup the pre-retirement setting
		PString valMortality = "UP94";
		if (!(Mortality.IsMissing() || Mortality.IsNil()))
		{
			if (Mortality.IsString())
			{
				valMortality = PString(Mortality.AsString());
				valMortality = valMortality.upper();
			}
			else
			{
				Error = 30; // Invalid mortality table
				return XlfOper(Error.getErrorDesc());
			}
		}
		unsigned short int strlength = valMortality.getLength();
		char *mtString = new char[strlength + 1];
		for (unsigned short int i = 0; i < strlength; i++)
			mtString[i] = (char)valMortality[i];
		mtString[strlength] = '\0';
		mbase tmtable(mtString, Error);
		if (Error)
			return XlfOper(Error.getErrorDesc());
		mtable = tmtable;

		//	Check Calculation Date for Generational Mortality
		if (CalcDate.IsMissing() || CalcDate.IsNil())
		{
			if ((mtable.getMortalityType() == mbase::generational) || (mtable.getMortalityType() == mbase::generational2D))
			{
				Error = 39;
				return XlfOper(Error.getErrorDesc());
			}
			date defaultCalcDate(mtable.getBaseYear(), 1, 1, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			valCalcDate = defaultCalcDate;
		}
		else
		{
			if (CalcDate.IsNumber())
			{
				date inputCalcDate(ExcelDateConversion(CalcDate.AsDouble(), 1), Error);
				if (Error)
					return XlfOper(Error.getErrorDesc());
				valCalcDate = inputCalcDate;
			}
			else
			{
				Error = 47; // Invalid calculation date
				return XlfOper(Error.getErrorDesc());
			}
		}

		// Check for COLA input and set colav accordingly
		if ((Indexing.IsMissing() || Indexing.IsNil()))
		{
			intDiscV tcolav(0, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			colav = tcolav;
		}
		else
		{
			if (Indexing.IsNumber())
			{
				intDiscV tcolav(Indexing.AsDouble(), Error);
				if (Error)
					return XlfOper(Error.getErrorDesc());
				colav = tcolav;
			}
			else  // Must be an array
			{
				unsigned short int vectorSize = Indexing.rows() * Indexing.columns();
				std::vector<double> stdInputVector(vectorSize);
				vector<long double> penserInputVector(vectorSize);
				stdInputVector = Indexing.AsDoubleVector();
				for (unsigned short int i = 0; i < vectorSize; i++)
					penserInputVector[i] = (long double)stdInputVector[i];
				intDiscV tcolav(penserInputVector, Error);
				if (Error)
					return XlfOper(Error.getErrorDesc());
				colav = tcolav;
			}
		}

		// Setup frequency of payment and interest variable
		// Verify and set guarantee period
		long double valFrequency = 12;
		if (!(Frequency.IsMissing() || Frequency.IsNil()))
		{
			if (Frequency.IsNumber())
				valFrequency = Frequency.AsDouble();
			else
			{
				Error = 15; // Invalid Frequency parameter
				return XlfOper(Error.getErrorDesc());
			}
		}
		freq frequency(valFrequency, Error);
		if (Error)
			return XlfOper(Error.getErrorDesc());

		// Call life function with all parameters
		sla ax(sexX, Age.AsDouble(), idv, valGuarantee, valDeferral, valPaid,
			mtable, valCalcDate, colav, frequency, Error);
		if (Error)
			return XlfOper(Error.getErrorDesc());
		double ret = (double)ax;
		return XlfOper(ret);
		EXCEL_END;
	}

	// JOINT Function

	LPXLFOPER EXCEL_EXPORT xlJOINT(LPXLFOPER inSexX, LPXLFOPER inAgeX, LPXLFOPER inSexY, LPXLFOPER inAgeY, LPXLFOPER inInterest, 
		LPXLFOPER inJointPct, LPXLFOPER inGuarantee, LPXLFOPER inDeferral, LPXLFOPER inPaid, LPXLFOPER inMortality, 
		LPXLFOPER inCalcDate, LPXLFOPER inIndexing, LPXLFOPER inFrequency, LPXLFOPER inRednOption)
	{
		EXCEL_BEGIN;
		XlfOper SexX(inSexX);
		XlfOper AgeX(inAgeX);
		XlfOper SexY(inSexY);
		XlfOper AgeY(inAgeY);
		XlfOper Interest(inInterest);
		XlfOper JointPct(inJointPct);
		XlfOper Guarantee(inGuarantee);
		XlfOper Deferral(inDeferral);
		XlfOper Paid(inPaid);
		XlfOper Mortality(inMortality);
		XlfOper CalcDate(inCalcDate);
		XlfOper Indexing(inIndexing);
		XlfOper Frequency(inFrequency);
		XlfOper RednOption(inRednOption);

		errorFlag Error;
		sex sexX, sexY;
		mbase mtable;
		intDiscV idv;
		intDiscV colav;
		date valCalcDate;

		// Validate SexX parameter
		if (SexX.IsString())
		{
			PString sexPStringX = SexX.AsString();
			unsigned short int strlength = sexPStringX.getLength();
			char *sexStringX = new char[strlength + 1];
			for (unsigned short int i = 0; i < strlength; i++)
				sexStringX[i] = (char)sexPStringX[i];
			sexStringX[strlength] = '\0';
			sex tsexX(sexStringX, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			sexX = tsexX;
		}
		else
		{
			if (SexX.IsNumber())
			{
				sex tsexX(SexX.AsDouble(), Error);
				if (Error)
					return XlfOper(Error.getErrorDesc());
				sexX = tsexX;
			}
			else
			{
				if (SexX.IsMulti())  // User Defined Lx - Includes setup of mortality
				{
					if (!IsNumVector(SexX))
					{
						Error = 20; // Invalid sex parameter
						return XlfOper(Error.getErrorDesc());
					}
					unsigned short int vectorSize = SexX.rows() * SexX.columns();
					std::vector<double> stdInputVector(vectorSize);
					vector<long double> penserInputVector(vectorSize);
					stdInputVector = SexX.AsDoubleVector();
					for (unsigned short int i = 0; i < vectorSize; i++)
						penserInputVector[i] = (long double)stdInputVector[i];
					sex tsexX(penserInputVector, Error);
					if (Error)
						return XlfOper(Error.getErrorDesc());
					sexX = tsexX;
					mbase tmtable; // Default table which will be ignored
					mtable = tmtable;
				}
				else
				{
					Error = 20;  // Invalid sex parameter
					return XlfOper(Error.getErrorDesc());
				}
			}
		}

		// Validate AgeX parameter
		if (!AgeX.IsNumber())
		{
			Error = 37; //Invalid age parameter
			return XlfOper(Error.getErrorDesc());
		}

		// Validate SexY parameter
		if (SexY.IsString())
		{
			PString sexPStringY = SexY.AsString();
			unsigned short int strlength = sexPStringY.getLength();
			char *sexStringY = new char[strlength + 1];
			for (unsigned short int i = 0; i < strlength; i++)
				sexStringY[i] = (char)sexPStringY[i];
			sexStringY[strlength] = '\0';
			sex tsexY(sexStringY, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			sexY = tsexY;
		}
		else
		{
			if (SexY.IsNumber())
			{
				sex tsexY(SexY.AsDouble(), Error);
				if (Error)
					return XlfOper(Error.getErrorDesc());
				sexY = tsexY;
			}
			else
			{
				if (SexY.IsMulti())  // User Defined Lx - Includes setup of mortality
				{
					if (!IsNumVector(SexY))
					{
						Error = 20; // Invalid sex parameter
						return XlfOper(Error.getErrorDesc());
					}
					unsigned short int vectorSize = SexY.rows() * SexY.columns();
					std::vector<double> stdInputVector(vectorSize);
					vector<long double> penserInputVector(vectorSize);
					stdInputVector = SexY.AsDoubleVector();
					for (unsigned short int i = 0; i < vectorSize; i++)
						penserInputVector[i] = (long double)stdInputVector[i];
					sex tsexY(penserInputVector, Error);
					if (Error)
						return XlfOper(Error.getErrorDesc());
					sexY = tsexY;
					mbase tmtable; // Default table which will be ignored
					mtable = tmtable;
				}
				else
				{
					Error = 20;  // Invalid sex parameter
					return XlfOper(Error.getErrorDesc());
				}
			}
		}

		// Validate AgeY parameter
		if (!AgeY.IsNumber())
		{
			Error = 37; //Invalid age parameter
			return XlfOper(Error.getErrorDesc());
		}

		// Validate IDV parameter
		if (!IsNumVector(Interest))
		{
			Error = 5; // Invalid interest/indexing rate parameter
			return XlfOper(Error.getErrorDesc());
		}

		// Check for interest type and set idv accordingly
		if (Interest.IsNumber())
		{
			intDiscV tidv(Interest.AsDouble(), Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			idv = tidv;
		}
		else  // Must be an array
		{
			unsigned short int vectorSize = Interest.rows() * Interest.columns();
			std::vector<double> stdInputVector(vectorSize);
			vector<long double> penserInputVector(vectorSize);
			stdInputVector = Interest.AsDoubleVector();
			for (unsigned short int i = 0; i < vectorSize; i++)
				penserInputVector[i] = (long double)stdInputVector[i];
			intDiscV tidv(penserInputVector, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			idv = tidv;
		}

		// Verify Joint Percentage
		if (!JointPct.IsNumber())
		{
			Error = 24; //Invalid joint percentage
			return XlfOper(Error.getErrorDesc());
		}

		// Verify and set guarantee period
		long double valGuarantee = 0;
		if (!(Guarantee.IsMissing() || Guarantee.IsNil()))
		{
			if (Guarantee.IsNumber())
				valGuarantee = Guarantee.AsDouble();
			else
			{
				Error = 14; // Invalid Guarantee parameter
				return XlfOper(Error.getErrorDesc());
			}
		}

		// Verify and set guarantee period
		long double valDeferral = 0;
		if (!(Deferral.IsMissing() || Deferral.IsNil()))
		{
			if (Deferral.IsNumber())
				valDeferral = Deferral.AsDouble();
			else
			{
				Error = 13; // Invalid Deferral parameter
				return XlfOper(Error.getErrorDesc());
			}
		}

		// Verify and set paid period
		long double valPaid = 999;
		if (!(Paid.IsMissing() || Paid.IsNil()))
		{
			if (Paid.IsNumber())
				valPaid = Paid.AsDouble();
			else
			{
				Error = 7; // Invalid Payment period parameter
				return XlfOper(Error.getErrorDesc());
			}
		}

		// Validate mortality table - Run even if UserDefinedMortality is being used
		// in order to setup pickup the pre-retirement setting
		PString valMortality = "UP94";
		if (!(Mortality.IsMissing() || Mortality.IsNil()))
		{
			if (Mortality.IsString())
			{
				valMortality = PString(Mortality.AsString());
				valMortality = valMortality.upper();
			}
			else
			{
				Error = 30; // Invalid mortality table
				return XlfOper(Error.getErrorDesc());
			}
		}
		unsigned short int strlength = valMortality.getLength();
		char *mtString = new char[strlength + 1];
		for (unsigned short int i = 0; i < strlength; i++)
			mtString[i] = (char)valMortality[i];
		mtString[strlength] = '\0';
		mbase tmtable(mtString, Error);
		if (Error)
			return XlfOper(Error.getErrorDesc());
		mtable = tmtable;

		//	Check for Calculation Date for Generational Mortality
		if (CalcDate.IsMissing() || CalcDate.IsNil())
		{
			if ((mtable.getMortalityType() == mbase::generational) || (mtable.getMortalityType() == mbase::generational2D))
			{
				Error = 39;
				return XlfOper(Error.getErrorDesc());
			}
			date defaultCalcDate(mtable.getBaseYear(), 1, 1, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			valCalcDate = defaultCalcDate;
		}
		else
		{
			if (CalcDate.IsNumber())
			{
				date inputCalcDate(ExcelDateConversion(CalcDate.AsDouble(), 1), Error);
				if (Error)
					return XlfOper(Error.getErrorDesc());
				valCalcDate = inputCalcDate;
			}
			else
			{
				Error = 47; // Invalid calculation date
				return XlfOper(Error.getErrorDesc());
			}
		}

		// Check for COLA input and set colav accordingly
		if ((Indexing.IsMissing() || Indexing.IsNil()))
		{
			intDiscV tcolav(0, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			colav = tcolav;
		}
		else
		{
			if (Indexing.IsNumber())
			{
				intDiscV tcolav(Indexing.AsDouble(), Error);
				if (Error)
					return XlfOper(Error.getErrorDesc());
				colav = tcolav;
			}
			else  // Must be an array
			{
				unsigned short int vectorSize = Indexing.rows() * Indexing.columns();
				std::vector<double> stdInputVector(vectorSize);
				vector<long double> penserInputVector(vectorSize);
				stdInputVector = Indexing.AsDoubleVector();
				for (unsigned short int i = 0; i < vectorSize; i++)
					penserInputVector[i] = (long double)stdInputVector[i];
				intDiscV tcolav(penserInputVector, Error);
				if (Error)
					return XlfOper(Error.getErrorDesc());
				colav = tcolav;
			}
		}

		// Setup frequency of payment and interest variable
		// Verify and set guarantee period
		long double valFrequency = 12;
		if (!(Frequency.IsMissing() || Frequency.IsNil()))
		{
			if (Frequency.IsNumber())
				valFrequency = Frequency.AsDouble();
			else
			{
				Error = 15; // Invalid Frequency parameter
				return XlfOper(Error.getErrorDesc());
			}
		}
		freq frequency(valFrequency, Error);
		if (Error)
			return XlfOper(Error.getErrorDesc());

		// Verify and Reduction Option
		unsigned short int valRednOption = 1;
		if (!(RednOption.IsMissing() || RednOption.IsNil()))
		{
			if (RednOption.IsNumber())
				valRednOption = RednOption.AsInt();
			else
			{
				Error = 27; // Invalid reduction option on death
				return XlfOper(Error.getErrorDesc());
			}
		}

		// Call joint life function with all parameters
		jla axy(sexX, AgeX.AsDouble(), sexY, AgeY.AsDouble(), idv, JointPct.AsDouble(),
			valGuarantee, valDeferral, valPaid, mtable, valCalcDate, colav,
			frequency, valRednOption, Error);
		if (Error)
			return XlfOper(Error.getErrorDesc());
		double ret = (double)axy;
		return XlfOper(ret);
		EXCEL_END;
	}

	// ELAPSE Function

	LPXLFOPER EXCEL_EXPORT xlELAPSE(LPXLFOPER inDate1, LPXLFOPER inDate2)
	{
		EXCEL_BEGIN;
		XlfOper Date1(inDate1);
		XlfOper Date2(inDate2);

		errorFlag Error;

		// Validate date parameters
		if (!(Date1.IsNumber() && Date2.IsNumber()))
		{
			Error = 42; //Invalid date parameter
			return XlfOper(Error.getErrorDesc());
		}

		// Adjust dates for 1875/01/01 start instead of Excel's 1900/01/01 (9131 days)
		unsigned long int date1 = ExcelDateConversion((unsigned long int)Date1.AsDouble(), 1);
		unsigned long int date2 = ExcelDateConversion((unsigned long int)Date2.AsDouble(), 1);

		double answer = elapse(date1, date2, Error);
		if (Error)
			return XlfOper(Error.getErrorDesc());

		return XlfOper(answer);
		EXCEL_END;
	}

	// ELIGDATE Function

	LPXLFOPER EXCEL_EXPORT xlELIGDATE(LPXLFOPER inDOB_Age, LPXLFOPER inAgeRequirement, LPXLFOPER inDOH_DOC, 
		LPXLFOPER inServiceAccrued, LPXLFOPER inServiceRequirement, LPXLFOPER inServiceIncr, LPXLFOPER inPtsServiceAccrued, 
		LPXLFOPER inPtsRequirement, LPXLFOPER inPtsServiceIncr, LPXLFOPER inOption)
	{
		EXCEL_BEGIN;
		XlfOper DOB_Age(inDOB_Age);
		XlfOper AgeRequirement(inAgeRequirement);
		XlfOper DOH_DOC(inDOH_DOC);
		XlfOper ServiceAccrued(inServiceAccrued);
		XlfOper ServiceRequirement(inServiceRequirement);
		XlfOper ServiceIncr(inServiceIncr);
		XlfOper PtsServiceAccrued(inPtsServiceAccrued);
		XlfOper PtsRequirement(inPtsRequirement);
		XlfOper PtsServiceIncr(inPtsServiceIncr);
		XlfOper Option(inOption);

		errorFlag Error;

		// Validate mandatory parameters
		if (!(DOB_Age.IsNumber() && DOH_DOC.IsNumber()))
		{
			Error = 44; //Invalid eligibility requirement
			return XlfOper(Error.getErrorDesc());
		}

		// Validate and setup Age, Service and Points Checks
		long double valAgeRequirement = 999;
		if (!(AgeRequirement.IsMissing() || AgeRequirement.IsNil()))
		{
			if (AgeRequirement.IsNumber())
				valAgeRequirement = AgeRequirement.AsDouble();
			else
			{
				Error = 44; //Invalid eligibility requirement
				return XlfOper(Error.getErrorDesc());
			}
		}

		long double valServiceAccrued = 0;
		if (!(ServiceAccrued.IsMissing() || ServiceAccrued.IsNil()))
		{
			if (ServiceAccrued.IsNumber())
				valServiceAccrued = ServiceAccrued.AsDouble();
			else
			{
				Error = 44; //Invalid eligibility requirement
				return XlfOper(Error.getErrorDesc());
			}
		}

		long double valServiceRequirement = 999;
		if (!(ServiceRequirement.IsMissing() || ServiceRequirement.IsNil()))
		{
			if (ServiceRequirement.IsNumber())
				valServiceRequirement = ServiceRequirement.AsDouble();
			else
			{
				Error = 44; //Invalid eligibility requirement
				return XlfOper(Error.getErrorDesc());
			}
		}

		long double valServiceIncr = 1;
		if (!(ServiceIncr.IsMissing() || ServiceIncr.IsNil()))
		{
			if (ServiceIncr.IsNumber())
				valServiceIncr = ServiceIncr.AsDouble();
			else
			{
				Error = 44; //Invalid eligibility requirement
				return XlfOper(Error.getErrorDesc());
			}
		}

		long double valPtsServiceAccrued = 0;
		if (!(PtsServiceAccrued.IsMissing() || PtsServiceAccrued.IsNil()))
		{
			if (PtsServiceAccrued.IsNumber())
				valPtsServiceAccrued = PtsServiceAccrued.AsDouble();
			else
			{
				Error = 44; //Invalid eligibility requirement
				return XlfOper(Error.getErrorDesc());
			}
		}

		long double valPtsRequirement = 999;
		if (!(PtsRequirement.IsMissing() || PtsRequirement.IsNil()))
		{
			if (PtsRequirement.IsNumber())
				valPtsRequirement = PtsRequirement.AsDouble();
			else
			{
				Error = 44; //Invalid eligibility requirement
				return XlfOper(Error.getErrorDesc());
			}
		}

		long double valPtsServiceIncr = 1;
		if (!(PtsServiceIncr.IsMissing() || PtsServiceIncr.IsNil()))
		{
			if (PtsServiceIncr.IsNumber())
				valPtsServiceIncr = PtsServiceIncr.AsDouble();
			else
			{
				Error = 44; //Invalid eligibility requirement
				return XlfOper(Error.getErrorDesc());
			}
		}

		unsigned short int valOption = 1;
		if (!(Option.IsMissing() || Option.IsNil()))
		{
			if (Option.IsNumber())
				valOption = Option.AsInt();
			else
			{
				Error = 44; //Invalid eligibility requirement
				return XlfOper(Error.getErrorDesc());
			}
		}

		// Calculate and return result
		date DOC(ExcelDateConversion((unsigned long int)DOH_DOC.AsDouble(), 1), Error);
		if (Error)
			return XlfOper(Error.getErrorDesc());

		date DOB;
		if (DOB_Age.AsDouble() < 125)  // Value entered was an age
		{
			dpoint tempDOB(DOC, -DOB_Age.AsDouble(), 1, 0, Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			DOB = tempDOB;
		}
		else  // Value entered was a date
		{
			date tempDOB(ExcelDateConversion((unsigned long int)DOB_Age.AsDouble(), 1), Error);
			if (Error)
				return XlfOper(Error.getErrorDesc());
			DOB = tempDOB;
		}

		dpoint ED(DOB, valAgeRequirement, DOC, valServiceAccrued, valServiceRequirement,
			valServiceIncr, valPtsServiceAccrued, valPtsRequirement, valPtsServiceIncr,
			valOption, Error);
		if (Error)
			return XlfOper(Error.getErrorDesc());

		double answer = (double)ExcelDateConversion(ED.numericValue(), -1);
		return XlfOper(answer);
		EXCEL_END;
	}

}

namespace
{

	// Register the function vT.

	XLRegistration::Arg vTArgs[] =
	{
		{ "interest", "Interest", "XLF_OPER" },
		{ "period", "Discount period", "XLF_OPER" }
	};

	XLRegistration::XLFunctionRegistrationHelper registervT(
		"xlvT", "vT", "Calculate the interest discount factor",
		"Penser", vTArgs, 2);

	// Register the function vT4.

	XLRegistration::Arg vT4Args[] =
	{
		{ "interest", "Interest", "P" },
		{ "period", "Discount period", "P" }
	};

	XLRegistration::XLFunctionRegistrationHelper registervT4(
		"xlvT4", "vT4", "Calculate the interest discount factor",
		"Penser", vT4Args, 2, false, false, "P");

	// Register the function vT12.

	XLRegistration::Arg vT12Args[] =
	{
		{ "interest", "Interest", "Q" },
		{ "period", "Discount period", "Q" }
	};

	XLRegistration::XLFunctionRegistrationHelper registervT12(
		"xlvT12", "vT12", "Calculate the interest discount factor",
		"Penser", vT12Args, 2, false, false, "Q");

	// Register the function tPx.

	XLRegistration::Arg tPxArgs[] =
	{
		{ "sex", "Sex", "XLF_OPER" },
		{ "age", "Age", "XLF_OPER" },
		{ "period", "Period", "XLF_OPER" },
		{ "mortality", "Mortality table", "XLF_OPER" },
		{ "calc date", "Calculation date", "XLF_OPER" }
	};

	XLRegistration::XLFunctionRegistrationHelper registertPx(
		"xltPx", "tPx", "Returns the probability of survival tPx",
		"Penser", tPxArgs, 5);

	// Register the function tPx4.

	XLRegistration::Arg tPx4Args[] =
	{
		{ "sex", "Sex", "P" },
		{ "age", "Age", "P" },
		{ "period", "Period", "P" },
		{ "mortality", "Mortality table", "P" },
		{ "calc date", "Calculation date", "P" }
	};

	XLRegistration::XLFunctionRegistrationHelper registertPx4(
		"xltPx4", "tPx4", "Returns the probability of survival tPx",
		"Penser", tPx4Args, 5, false, false, "P");

	// Register the function tPx12.

	XLRegistration::Arg tPx12Args[] =
	{
		{ "sex", "Sex", "Q" },
		{ "age", "Age", "Q" },
		{ "period", "Period", "Q" },
		{ "mortality", "Mortality table", "Q" },
		{ "calc date", "Calculation date", "Q" }
	};

	XLRegistration::XLFunctionRegistrationHelper registertPx12(
		"xltPx12", "tPx12", "Returns the probability of survival tPx",
		"Penser", tPx12Args, 5, false, false, "Q");

	// Register the function LIFE

	XLRegistration::Arg LIFEArgs[] =
	{
		{ "sex", "Sex", "XLF_OPER" },
		{ "age", "Age", "XLF_OPER" },
		{ "interest", "Interest", "XLF_OPER" },
		{ "guarantee", "Guarantee period", "XLF_OPER" },
		{ "deferral", "Deferral period", "XLF_OPER" },
		{ "payment", "Payment period", "XLF_OPER" },
		{ "mortality", "Mortality table", "XLF_OPER" },
		{ "calculation date", "Calculation date", "XLF_OPER" },
		{ "indexing", "Indexing", "XLF_OPER" },
		{ "frequency", "Frequency", "XLF_OPER" }
	};

	XLRegistration::XLFunctionRegistrationHelper registerLIFE(
		"xlLIFE", "LIFE", "Single life annuity factor",
		"Penser", LIFEArgs, 10);

	// Register the function LIFE4

	XLRegistration::Arg LIFE4Args[] =
	{
		{ "sex", "Sex", "P" },
		{ "age", "Age", "P" },
		{ "interest", "Interest", "P" },
		{ "guarantee", "Guarantee period", "P" },
		{ "deferral", "Deferral period", "P" },
		{ "payment", "Payment period", "P" },
		{ "mortality", "Mortality table", "P" },
		{ "calculation date", "Calculation date", "P" },
		{ "indexing", "Indexing", "P" },
		{ "frequency", "Frequency", "P" }
	};

	XLRegistration::XLFunctionRegistrationHelper registerLIFE4(
		"xlLIFE4", "LIFE4", "Single life annuity factor",
		"Penser", LIFE4Args, 10, false, false, "P");

	// Register the function LIFE12

	XLRegistration::Arg LIFE12Args[] =
	{
		{ "sex", "Sex", "Q" },
		{ "age", "Age", "Q" },
		{ "interest", "Interest", "Q" },
		{ "guarantee", "Guarantee period", "Q" },
		{ "deferral", "Deferral period", "Q" },
		{ "payment", "Payment period", "Q" },
		{ "mortality", "Mortality table", "Q" },
		{ "calculation date", "Calculation date", "Q" },
		{ "indexing", "Indexing", "Q" },
		{ "frequency", "Frequency", "Q" }
	};

	XLRegistration::XLFunctionRegistrationHelper registerLIFE12(
		"xlLIFE12", "LIFE12", "Single life annuity factor",
		"Penser", LIFE12Args, 10, false, false, "Q");

	// Register the function JOINT

	XLRegistration::Arg JOINTArgs[] =
	{
		{ "sex X", "Sex of member", "XLF_OPER" },
		{ "age X", "Age of member", "XLF_OPER" },
		{ "sex Y", "Sex of spouse", "XLF_OPER" },
		{ "age Y", "Age of spouse", "XLF_OPER" },
		{ "interest", "Interest", "XLF_OPER" },
		{ "joint %", "Joint percentage", "XLF_OPER" },
		{ "guarantee", "Guarantee period", "XLF_OPER" },
		{ "deferral", "Deferral period", "XLF_OPER" },
		{ "payment", "Payment period", "XLF_OPER" },
		{ "mortality", "Mortality table", "XLF_OPER" },
		{ "calculation date", "Calculation date", "XLF_OPER" },
		{ "indexing", "Indexing", "XLF_OPER" },
		{ "frequency", "Frequency", "XLF_OPER" },
		{ "reduction option", "Reduction option", "XLF_OPER" }
	};

	XLRegistration::XLFunctionRegistrationHelper registerJOINT(
		"xlJOINT", "JOINT", "Joint life annuity factor",
		"Penser", JOINTArgs, 14);

	// Register the function JOINT4

	XLRegistration::Arg JOINT4Args[] =
	{
		{ "sex X", "Sex of member", "P" },
		{ "age X", "Age of member", "P" },
		{ "sex Y", "Sex of spouse", "P" },
		{ "age Y", "Age of spouse", "P" },
		{ "interest", "Interest", "P" },
		{ "joint %", "Joint percentage", "P" },
		{ "guarantee", "Guarantee period", "P" },
		{ "deferral", "Deferral period", "P" },
		{ "payment", "Payment period", "P" },
		{ "mortality", "Mortality table", "P" },
		{ "calculation date", "Calculation date", "P" },
		{ "indexing", "Indexing", "P" },
		{ "frequency", "Frequency", "P" },
		{ "reduction option", "Reduction option", "P" }
	};

	XLRegistration::XLFunctionRegistrationHelper registerJOINT4(
		"xlJOINT4", "JOINT4", "Joint life annuity factor",
		"Penser", JOINT4Args, 14, false, false, "P");

	// Register the function JOINT12

	XLRegistration::Arg JOINT12Args[] =
	{
		{ "sex X", "Sex of member", "Q" },
		{ "age X", "Age of member", "Q" },
		{ "sex Y", "Sex of spouse", "Q" },
		{ "age Y", "Age of spouse", "Q" },
		{ "interest", "Interest", "Q" },
		{ "joint %", "Joint percentage", "Q" },
		{ "guarantee", "Guarantee period", "Q" },
		{ "deferral", "Deferral period", "Q" },
		{ "payment", "Payment period", "Q" },
		{ "mortality", "Mortality table", "Q" },
		{ "calculation date", "Calculation date", "Q" },
		{ "indexing", "Indexing", "Q" },
		{ "frequency", "Frequency", "Q" },
		{ "reduction option", "Reduction option", "Q" }
	};

	XLRegistration::XLFunctionRegistrationHelper registerJOINT12(
		"xlJOINT12", "JOINT12", "Joint life annuity factor",
		"Penser", JOINT12Args, 14, false, false, "Q");

	// Register the function ELAPSE.

	XLRegistration::Arg ELAPSEArgs[] =
	{
		{ "date 1", "Date 1", "XLF_OPER" },
		{ "date 2", "Date 2", "XLF_OPER" }
	};

	XLRegistration::XLFunctionRegistrationHelper registerELAPSE(
		"xlELAPSE", "ELAPSE", "Calculate the elapsed time (in years) between two dates",
		"Penser", ELAPSEArgs, 2);

	// Register the function ELAPSE4.

	XLRegistration::Arg ELAPSE4Args[] =
	{
		{ "date 1", "Date 1", "P" },
		{ "date 2", "Date 2", "P" }
	};

	XLRegistration::XLFunctionRegistrationHelper registerELAPSE4(
		"xlELAPSE4", "ELAPSE4", "Calculate the elapsed time (in years) between two dates",
		"Penser", ELAPSE4Args, 2, false, false, "P");

	// Register the function ELAPSE12.

	XLRegistration::Arg ELAPSE12Args[] =
	{
		{ "date 1", "Date 1", "Q" },
		{ "date 2", "Date 2", "Q" }
	};

	XLRegistration::XLFunctionRegistrationHelper registerELAPSE12(
		"xlELAPSE12", "ELAPSE12", "Calculate the elapsed time (in years) between two dates",
		"Penser", ELAPSE12Args, 2, false, false, "Q");

	// Register the function ELIGDATE

	XLRegistration::Arg ELIGDATEArgs[] =
	{
		{ "Either DOB or Age", "Either DOB or Age", "XLF_OPER" },
		{ "Age requirement", "Age requirement", "XLF_OPER" },
		{ "Either DOC or DOH", "Either DOC or DOH (DOC req'd with age)", "XLF_OPER" },
		{ "Service accrued at DOC or DOH", "Service accrued at DOC or DOH", "XLF_OPER" },
		{ "Service requirement", "Service requirement", "XLF_OPER" },
		{ "Service increment", "Service increment", "XLF_OPER" },
		{ "Service accrued for points requirement", "Service accrued for points requirement", "XLF_OPER" },
		{ "Points requirement", "Points requirement", "XLF_OPER" },
		{ "Service increments for points", "Service increment for points", "XLF_OPER" },
		{ "Option", "Option", "XLF_OPER" }
	};

	XLRegistration::XLFunctionRegistrationHelper registerELIGDATE(
		"xlELIGDATE", "ELIGDATE",
		"Returns the earliest date where any of the age, service, or points elig req'ts are met",
		"Penser", ELIGDATEArgs, 10);

	// Register the function ELIGDATE4

	XLRegistration::Arg ELIGDATE4Args[] =
	{
		{ "Either DOB or Age", "Either DOB or Age", "P" },
		{ "Age requirement", "Age requirement", "P" },
		{ "Either DOC or DOH", "Either DOC or DOH (DOC req'd with age)", "P" },
		{ "Service accrued at DOC or DOH", "Service accrued at DOC or DOH", "P" },
		{ "Service requirement", "Service requirement", "P" },
		{ "Service increment", "Service increment", "P" },
		{ "Service accrued for points requirement", "Service accrued for points requirement", "P" },
		{ "Points requirement", "Points requirement", "P" },
		{ "Service increments for points", "Service increment for points", "P" },
		{ "Option", "Option", "P" }
	};

	XLRegistration::XLFunctionRegistrationHelper registerELIGDATE4(
		"xlELIGDATE4", "ELIGDATE4",
		"Returns the earliest date where any of the age, service, or points elig req'ts are met",
		"Penser", ELIGDATE4Args, 10, false, false, "P");

	// Register the function ELIGDATE12

	XLRegistration::Arg ELIGDATE12Args[] =
	{
		{ "Either DOB or Age", "Either DOB or Age", "Q" },
		{ "Age requirement", "Age requirement", "Q" },
		{ "Either DOC or DOH", "Either DOC or DOH (DOC req'd with age)", "Q" },
		{ "Service accrued at DOC or DOH", "Service accrued at DOC or DOH", "Q" },
		{ "Service requirement", "Service requirement", "Q" },
		{ "Service increment", "Service increment", "Q" },
		{ "Service accrued for points requirement", "Service accrued for points requirement", "Q" },
		{ "Points requirement", "Points requirement", "Q" },
		{ "Service increments for points", "Service increment for points", "Q" },
		{ "Option", "Option", "Q" }
	};

	XLRegistration::XLFunctionRegistrationHelper registerELIGDATE12(
		"xlELIGDATE12", "ELIGDATE12",
		"Returns the earliest date where any of the age, service, or points elig req'ts are met",
		"Penser", ELIGDATE12Args, 10, false, false, "Q");

}