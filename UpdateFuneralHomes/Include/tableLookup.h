// tableLookup.h

#ifndef TABLELOOKUP_H
#define TABLELOOKUP_H

#include <string>
#include <tchar.h>
#include <fstream>

#define tlFamilyName         1
#define tlFirstName          2
#define tlMiddleName         3
#define tlFullName           4
#define tlGender             5
#define tlDOB                6
#define tlDOD                7
#define tlCity               8
#define tlProvince           9
#define tlCountry           10
#define tlSpouseName        11
#define tlURL               12
#define tlPublisherName     13
#define tlPublisherCity     14
#define tlPublisherProvince 15
#define tlPublisherCountry  16
#define tlCauseOfDeath      17

struct TableRow{
	std::wstring fieldname;
	unsigned int rowValue;
};

class fieldNames {

public:

	bool setupLookupTable();
	int find(const std::wstring &target) const;

private:
	std::wifstream inFile;
	unsigned int numFields;
	TableRow *tableRow;

};









#endif
