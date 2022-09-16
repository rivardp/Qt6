#ifndef ENUMS_H
#define ENUMS_H

enum LANGUAGE { language_unknown = 0, multiple_unknown, english, french, spanish, multiple };
enum GENDER {genderUnknown = 0, Male, Female};
enum SEARCHTYPE { deceasedData = 0, listOfDeceased, listOfGroups };
enum NAMETYPE { ntUnknown = 0, ntFirst, ntMiddle, ntLast, ntLikelyFirst, ntLikelyMiddle, ntLikelyLast, ntPrefix, ntSuffix };
enum NAMESKNOWN { nkNone = 0, nkFirstOnly, nkLastOnly, nkFirstAndLast};
enum CHARSETS { UTF_8 = 0, ASCII, Windows_1252, ISO_8859_15, ISO_8859_1, ISO_8859_6, CP1256 };
enum CREDIBILITY { zero = 0, low, medium, high, veryHigh};
enum DATETYPE { notValidDate = 0, numericDate, englishDate, frenchDate, spanishDate };
enum DATEFIELD { dfAll, dfDOB, dfDOD, dfDOS, dfDOP};
enum PROVINCE { provUnknown = 0, BC, AB, SK, MB, ON, QC, NB, NS, PE, NL, YT, NT, NU};

#endif // ENUMS_H
