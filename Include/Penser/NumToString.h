//	num_to_string.h		Version 1.0		February 23, 2009
//
//  Converts numbers in PStrings

#ifndef NUM_TO_STRING_H
#define NUM_TO_STRING_H

#define nfNO_FORMAT 0
#define nfCOMMA 1
#define nfCURRENCY 2
#define nfPERCENT 4
#define nfDATE 8
#define nfFORWARD_SLASH 16
#define nfLONG_FORM 32
#define nfYMD 64
#define nfDMY 128
#define nfMDY 256
#define nfSIN 512


#ifndef PSTRING_H
#include "../Penser/PString.h"
#endif

TCHAR ConvertDigitToString(const unsigned int digit);		// Converts a single digit into a TCHAR
PString ConvertNumberToString(const long double &num, const int &numDec, const int format = 0);

#endif
