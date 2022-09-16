// PQDate.h

#ifndef DATE_H
#define DATE_H

#define Q_UNUSED(x) (void)x;

#include <QDate>
#include <QDateTime>

/*
static const unsigned int leapYearRange[91] = {
1876,1880,1884,1888,1892,1896,1904,1908,1912,1916,1920,1924,1928,1932,1936,1940,1944,1948,
1952,1956,1960,1964,1968,1972,1976,1980,1984,1988,1992,1996,2000,2004,2008,2012,2016,2020,
2024,2028,2032,2036,2040,2044,2048,2052,2056,2060,2064,2068,2072,2076,2080,2084,2088,2092,
2096,2104,2108,2112,2116,2120,2124,2128,2132,2136,2140,2144,2148,2152,2156,2160,2164,2168,
2172,2176,2180,2184,2188,2192,2196,2204,2208,2212,2216,2220,2224,2228,2232,2236,2240,2244,
2248};

static const PVector<unsigned int> leapYears(leapYearRange,91);
static const unsigned int cumDays[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};
static const unsigned int cumDaysLeapYear[13] = {0,31,60,91,121,152,182,213,244,274,305,335,366};

enum DATEFORMAT { DF_Unknown = 0, yyyymmdd, mmddyyyy, ddmmyyyy, yyyymmddns }; 


class date{

protected:
	unsigned int yyyy;
	unsigned int mm;
	unsigned int dd;

	unsigned long int numericDate;

	// Methods
	unsigned int isLeapYear(const unsigned int &Year) const;
	unsigned int countLeapDaysPriorTo(const unsigned int &Year) const;
	unsigned int daysInYear(const unsigned int &Year) const;

	unsigned long int convertToNumeric(const unsigned int &Year, const unsigned int &Month, const unsigned int &Day);
	date convertFromNumeric(const unsigned long int &nd, errorFlag &Error);

	friend void validateOption(const unsigned int &option, errorFlag &Error);

public:
	date();
	date(const unsigned int &Year, const unsigned int &Month, const unsigned int &Day, errorFlag &Error);
	date(const unsigned long int &dateNumber, errorFlag &Error);
	date(const date &rhs);
	date(const date *rhs);
	date(const unsigned int usi);  // Added for default initialization in Array<>

	unsigned int year() const;
	unsigned int month() const;
	unsigned int day() const;
	unsigned long int numericValue() const;
	unsigned int daysInYear() const;
	long int xport() const;
	void clear();
	unsigned int isValid() const;
	
	// comparison operators
	bool operator==(const date &d1) const;
	bool operator!=(const date &d1) const;
	bool operator<=(const date &d1) const;
	bool operator< (const date &d1) const;
	bool operator>=(const date &d1) const;
	bool operator> (const date &d1) const;

	// calculation operators
		   date operator+(const unsigned long int &days) const;
	friend date operator+(const unsigned long int &days, const date &d1){return d1 + days;}
	       date operator-(const unsigned long int &days) const;
	
	// assignment operator
	date& operator=(const date &rhs);

	// calculation and assignment operators
	date& operator+=(const unsigned long int &days);
	date& operator-=(const unsigned long int &days);

	// Methods
	friend double elapse(const unsigned long int &nd1, const unsigned long int &nd2, errorFlag &Error);
	friend double elapse(const date &d1, const date &d2, errorFlag &Error);
	
	friend date roundUp(const date &d1, const unsigned int &option, errorFlag &Error);

	friend date dmax(const date &d1, const date &d2);
	friend date dmax(const date &d1, const date &d2, const date &d3);
	friend date dmax(const date &d1, const date &d2, const date &d3, const date &d4);
	friend date dmax(const date &d1, const date &d2, const date &d3, const date &d4, const date &d5);
	friend date dmin(const date &d1, const date &d2);
	friend date dmin(const date &d1, const date &d2, const date &d3);
	friend date dmin(const date &d1, const date &d2, const date &d3, const date &d4);
	friend date dmin(const date &d1, const date &d2, const date &d3, const date &d4, const date &d5);

	// String output methods
	PString stringOutput(const unsigned int format) const;
};*/

double elapse(const QDate &d1, const QDate &d2);


#endif
