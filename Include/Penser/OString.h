// OString.h - PString with some added functionality for Obits
// 


#ifndef OSTRING_H
#define OSTRING_H

#include "PString.h"

#ifndef LANGUAGE_ENUM
#define LANGUAGE_ENUM
enum LANGUAGE { language_unknown = 0, english, french, spanish };
#endif

class OString :public PString
{
public:
	// Constructors
	OString();
	OString(const TCHAR* const);		// Convert character array to PString
	OString(const char* const);
	OString(const PString &);
	OString(const OString &);
	OString(const TCHAR& singleChar);	// created for get()
	OString(unsigned int);				// Required for array initializations only
	OString(const std::string &);
	OString(const std::wstring &);
	
	// Destructor
	~OString();

	// operators
	OString& operator= (const OString &rhs);
	OString operator+(const OString &rhs);
	void operator+=(const OString &rhs);
	bool operator== (const OString &rhs);
	bool operator!= (const OString &rhs);

	// Methods
	void insertParameter(PString param);	// replace %p% with param
	bool removeLeadingNeeEtAl();

	bool isPrefix() const;
	bool isSuffix() const;
	bool isSuffixAllCaps() const;
	bool isTitle() const;
	bool isSaint() const;

	unsigned int isWrittenMonth(const LANGUAGE &lang);

};


#endif
