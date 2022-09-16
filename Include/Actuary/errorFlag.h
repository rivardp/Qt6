// errorflag.h   Version 1.0  March 24, 2003
//						 2.0  March 1, 2009 - Eliminate all shorts
//						 2.1  February 12, 2015 - Change to accomodate UNICODE
//                       3.0  November 16, 2019 - Moved over to Qt
//

#ifndef ERRORFLAG_H
#define ERRORFLAG_H

#include <tchar.h>

class errorFlag {

protected:
	unsigned int errorNum;	// Numeric code for error
    TCHAR *errorDesc;
	
public:
	// Constructors
	errorFlag();
	
	// Methods
    TCHAR* getErrorDesc();

	// Conversion Operator
	operator unsigned int();  // provides rhs read value
	
	// Assignment Operator
	errorFlag& operator=(const unsigned int ErrorNum);

	// Destructor
	~errorFlag();
};


#endif
