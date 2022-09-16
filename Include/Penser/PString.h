// PString.h

#ifndef PSTRING_H
#define PSTRING_H

#include <string>
#include <tchar.h>

#include "../Include/Penser/qchartype.h"
#include "../Include/Penser/QtUtilities.h"

class PString
{
public:
	// Constructors
	PString();
    PString(const TCHAR* const);		// Convert character array to PString
//	PString(const char* const);
	PString(const PString &);
    PString(const TCHAR& singleChar);	// created for get()
    PString(unsigned int);				// Required for array initializations only
	PString(const std::string &);
	PString(const std::wstring &);

	// Destructor
	~PString();

	// operators
	TCHAR & operator[](size_t offset);
	TCHAR operator[](size_t offset) const;
	PString operator+(const PString &rhs);
	void operator+=(const PString &rhs);
	PString& operator= (const PString &rhs);
	bool operator== (const PString &rhs);
	bool operator!= (const PString &rhs);

	// accessors
	unsigned int getLength() const;
	const TCHAR * getString() const;
	std::string getStdString() const;
	std::wstring getWString() const;
	char* convertToCharPtr() const;
    char* getQString() const;

	// generic methods
	PString left(unsigned int num) const;
	PString right(unsigned int num) const;
	PString middle(unsigned int offset, unsigned int num) const;
	PString& clear();

	void dropLeft(unsigned int num);
	void dropRight(unsigned int num);

	bool isMatch(const PString &rhs) const;
	bool isMatch(const TCHAR* cString) const;
	int findPosition(const TCHAR* cString, int direction = 1, unsigned int skip = 0, unsigned int occurrence = 1) const;
	int findPosition(const PString &pString, int direction = 1, unsigned int skip = 0, unsigned int occurrence = 1) const;

	long double asNumber() const;
	long double extractFirstNumber() const;

	// methods dependent on chartype.h
	unsigned int getCharType(unsigned int elemNum = 0, unsigned int numWordsRead = 0) const;

	PString upper() const;
	PString lower() const;
	PString proper() const;
	PString reciprocal() const;

	bool isAlpha() const;
	bool isNumeric() const;
	bool isAlphaNumeric() const;
	bool isCapitalized() const;
	bool isAllCaps() const;

	void removeSpecialChar();		// Remove escape character (\), ampersand (&) and double quote (") since they will interfere with streams
	bool hasBookEnds(unsigned int bookends);
	bool removeBookEnds(unsigned int bookends = PARENTHESES | QUOTES);
	bool removeLeading(const std::wstring target);
    bool removeEnding(const unsigned int charCode);
	bool removeEnding(const std::wstring target);
	bool cleanUpEnds();

	unsigned int countWords(unsigned int delimiter = SPACE);
	unsigned int drawOutNumber() const;


protected:
	TCHAR * itsString;
	unsigned int itsLength;
};

#endif
