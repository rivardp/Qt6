//	num_to_string.cpp
//
#include "NumToString.h"

TCHAR ConvertDigitToString(const unsigned int digit)
{
    TCHAR buf[16];
    _stprintf(buf,_T("%d"), digit);
    TCHAR* result = static_cast<TCHAR*>(buf);

    return *result;
}

PString ConvertNumberToString(const long double &num, const int &numDec, const int format)
{
	bool inclCommas = (format & nfCOMMA) == nfCOMMA;
	bool inclCurrency = (format & nfCURRENCY) == nfCURRENCY;
	bool inclPct = (format & nfPERCENT) == nfPERCENT;
	bool isSIN = (format & nfSIN) == nfSIN;
	unsigned int numLeft;
	PString stringNum;

	unsigned int numChar = 0;
	int sign = 1;
	unsigned int i, digit, numBeforeDec, numAfterDec;
    long double temp;
	PString space = _T(" ");

	if (isSIN)
	{
		temp = num;
		while ((temp >= 1) && (numChar < 9))
		{
			numChar++;
			temp /= 10;
		}
		temp = num;
		for (i = 1; i <= numChar; i++)
		{
            digit = static_cast<unsigned int>(temp / powl(10, numChar - i));
			temp -= digit * powl(10, numChar - i);
			stringNum += ConvertDigitToString(digit);
			numLeft = numChar - i;
			if ((numLeft == 3) || (numLeft == 6))
				stringNum += space;
		}
		return stringNum;
	}

	if (inclCurrency)
		stringNum += _T("$");
	
	// Process simple case if it exists
	if ((numDec == 0) && (num >= 0) && (num < 10))
	{
        stringNum += ConvertDigitToString(static_cast<const unsigned int>(num));
		return stringNum;
	}
	
	// More complex numbers

	if (num < 0)
		sign = -1;
    long truncated = static_cast<long>((sign * num * powl(10,numDec)) + 0.5);
	temp = truncated;
	while (temp >= 1)
	{
		numChar++;
		temp /= 10;
	}

    numAfterDec = (numDec > 0) ? static_cast<unsigned int>(numDec) : 0;
	if (numChar <= numAfterDec)
		numBeforeDec = 1;		// Always print a leading zero
	else
		numBeforeDec = numChar - numAfterDec;
	
	temp = truncated / powl(10,numDec);

    if (sign == -1)
	{
		if (inclCurrency)
			stringNum += _T("(");
		else
			stringNum += _T("-");
	}

	for (i=1; i<=numBeforeDec; i++)
	{
        digit = static_cast<unsigned int>(temp / powl(10,numBeforeDec-i));
		temp -= digit * powl(10,numBeforeDec-i);
		stringNum += ConvertDigitToString(digit);
		numLeft = numBeforeDec - i;
		if (inclCommas && (numLeft > 0) && ((numLeft%3)==0))
			stringNum += _T(",");
	}
	if (numDec > 0)
		stringNum += _T(".");
	for (i=1; i<=numAfterDec; i++)
	{
        digit = static_cast<unsigned int>((temp * powl(10,i)) + 0.000000000001);
		temp -= digit / powl(10,i);
		stringNum += ConvertDigitToString(digit);
	}

	if ((sign == -1) && inclCurrency)
		stringNum += _T(")");

	if (inclPct)
		stringNum += _T("%");

	return stringNum;
}
