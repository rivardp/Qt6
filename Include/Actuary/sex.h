// sex.h   Version 1.1  October 25, 2007   - Change char* parameter to const char*
//		           1.2  July 20, 2008 - Added validity check on UserDefinedLx Vecotr
//				   2.0  March 1, 2009 - Eliminate shorts
//

#ifndef SEX_H
#define SEX_H

#ifndef ERRORFLAG_H
#include "../Actuary/errorFlag.h"
#endif

#ifndef PVector_H
#include "../Actuary/PVector.h"
#endif

class sex {

protected:

	double sexParameter;  // 1=Male, 2=Female, 0 to 1=Unisex
	PVector<long double> *userDefinedLx;

	int match(const char *s1, const char *s2);

public:
	// Constructors
	sex();
	sex(const long double inputSex, errorFlag &Error);
	sex(const char* inputSex, errorFlag &Error);
	sex(const PVector<long double> &udmv, errorFlag &Error);
	sex(sex &sx);

	// Methods
	PVector<long double>& getUserDefinedLx() const;

	// Conversion operator
	operator double() const;  // provides rhs read value

	// Assignment operator
	sex& operator=(const sex &rhs);

	// Destructor
	~sex();

};

PVector<long double>& sex::getUserDefinedLx() const
{
	return *userDefinedLx;
}

sex::operator double() const
{
	return sexParameter;
}

sex& sex::operator=(const sex &rhs)
{
	if (this == &rhs) return *this;
	if (userDefinedLx)
	{
		delete userDefinedLx;
        userDefinedLx = nullptr;
	}
	sexParameter = rhs;
    if (static_cast<int>(sexParameter) == 99)
	{
		userDefinedLx = new PVector<long double>(rhs.getUserDefinedLx());
        assert (userDefinedLx != nullptr);
	}
	return *this;
}

sex::~sex()
{
	delete userDefinedLx;
	userDefinedLx = 0;
}

sex::sex() : sexParameter(1) , userDefinedLx(nullptr)
{
}

sex::sex(const long double inputSex, errorFlag &Error) : userDefinedLx(nullptr)
{
	if ((inputSex < 0) || ((inputSex > 1) && (inputSex != 2)))
		Error = 20; // Invalid sex parameter
	if (!Error)
        sexParameter = static_cast<double>(inputSex);
}


sex::sex(const char *inputSex, errorFlag &Error) : userDefinedLx(nullptr)
{
	const char *stemp;
	unsigned int length=0;
	stemp = inputSex;

	while ((*stemp != '\0') && (length <= 7))
		{
		length++;
		stemp++;
		}

	switch (length)
		{
		case 7:
		Error = 20; // Invalid sex parameter
		break;

		case 1:
			if (match(inputSex,"M\n") || match(inputSex,"m\n"))
                sexParameter = static_cast<double>(1);
			else
				if (match(inputSex,"F\n") || match(inputSex,"f\n"))
                    sexParameter = static_cast<double>(2);
				else
					Error = 20; // Invalid sex parameter
			break;

		case 4:
			if (match(inputSex,"MALE\n") || match(inputSex,"Male\n") || match(inputSex,"male\n"))
                sexParameter = static_cast<double>(1);
			else
				Error = 20; // Invalid sex parameter
			break;

		case 6:
			if (match(inputSex,"FEMALE\n") || match(inputSex,"Female\n") || match(inputSex,"female\n"))
                sexParameter = static_cast<double>(2);
			else
				Error = 20; // Invalid sex parameter
			break;

		default: 	// if no match by here, invalid table
				Error = 20; // Invalid sex parameter
      }
}

sex::sex(const PVector<long double> &udmv, errorFlag &Error)
{
	sexParameter = 99;
	userDefinedLx = new PVector<long double>(udmv);
    assert (userDefinedLx != nullptr);

	unsigned int size = userDefinedLx->getSize();
	bool OK = (size >= 2);

	unsigned int i = 0;
	while (OK && (i<(size-2)))
	{
		OK = (*userDefinedLx)[i] >= (*userDefinedLx)[i+1];
		i++;
	}
	if (!OK)
		Error = 32;
}

sex::sex(sex &sx) : sexParameter(sx), userDefinedLx(0)
{
    if (static_cast<int>(sx) == 99)
	{
		userDefinedLx = new PVector<long double>(sx.getUserDefinedLx());
        assert (userDefinedLx != nullptr);
	}
}

int sex::match(const char *s1, const char *s2)
{
	while (*s1 != '\0')
		{
		if (*s1++ != *s2++) return 0;
		}
	return 1;
}

#endif
