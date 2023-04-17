#ifndef PQSTRING_H
#define PQSTRING_H

#include <QString>
#include <QTextDocument>
#include <QRegularExpression>
#include <QVector>
#include <vector>
#include <algorithm>
#include "PString.h"
#include <tchar.h>

#include "../Include/Penser/qchartype.h"

class PQString
{
public:
	// Constructors
    PQString();
    PQString(const char* const);			// Convert character array to PQString
    PQString(const PQString &);
    PQString(const QString &);
    PQString(const PString &);
    PQString(const QChar& singleChar);		// created for get()
    PQString(const std::string &);
    PQString(const std::wstring &);
    PQString(const unsigned int);
    PQString(const int);

	// Destructor
    ~PQString();

	// operators
    QString operator[](unsigned int offset) const;
    //QCharRef operator[](unsigned int offset);
    PQString operator+(const PQString &rhs);
    PQString operator+(const QString &rhs);
    PQString operator+(const QChar &rhs);
    void operator+=(const PQString &rhs);
    void operator+=(const QString &rhs);
    void operator+=(const QChar &rhs);
    PQString& operator= (const PQString &rhs);
    PQString& operator= (const QString &rhs);
    PQString& operator= (const QChar &rhs);
    bool operator== (const PQString &rhs) const;
    bool operator== (const QString &rhs) const;
    bool operator== (const QChar &rhs) const;
    bool operator!= (const PQString &rhs) const;
    bool operator!= (const QString &rhs) const;
    bool operator!= (const QChar &rhs) const;
    PQString& operator<< (const PQString &rhs);

	// accessors
	unsigned int getLength() const;
    QString getString() const;
    QString getUnaccentedString() const;
    QString replaceLigatures() const;
	std::string getStdString() const;
	std::wstring getWString() const;
	char* convertToCharPtr() const;
    unsigned int convertCharToANSI();

	// generic methods
    PQString left(unsigned int num) const;
    PQString right(unsigned int num) const;
    PQString middle(unsigned int offset, unsigned int num) const;
    void clear();

	void dropLeft(unsigned int num);
	void dropRight(unsigned int num);
    void purge(QList<QString> &purgeList);

    void parseContentInto(std::vector<PQString*> &wordVector);

    bool isMatch(const PQString &rhs) const;
    bool isMatch(const wchar_t* wString) const;
    int findPosition(const wchar_t* wString, int direction = 1, unsigned int skip = 0, unsigned int occurrence = 1) const;
    int findPosition(const PQString &pString, int direction = 1, unsigned int skip = 0, unsigned int occurrence = 1) const;
    int firstDifference(const PQString &pString) const;

	long double asNumber() const;
	long double extractFirstNumber() const;
    QString extractFirstWord() const;

    // URL utility support
    void insertParameter(const PQString param);	// replace %p% with param

	// methods dependent on chartype.h
	unsigned int getCharType(unsigned int elemNum = 0, unsigned int numWordsRead = 0) const;

    PQString upper() const;
    PQString lower() const;
    PQString proper() const;
    PQString preHyphen() const;
    PQString postHyphen() const;
    PQString reciprocal() const;
    PQString& simplify(bool insertPeriods = false);
    PQString& JSONsimplify();
    PQString& unescapeJSONformat();
    PQString& replaceIneligibleURLchars();

	bool isAlpha() const;
	bool isNumeric() const;
	bool isAlphaNumeric() const;
    bool isDeemedSpace() const;
	bool isCapitalized() const;
    bool isAllCaps(bool mustAllBeAlpha = true) const;
    bool isHyphenated() const;
    bool isHyphen() const;
    bool isSingleQuote() const;
    bool isDoubleQuote() const;
    bool isQuoteMark() const;
    bool isForeignLanguage() const;
    bool containsVowel() const;
    bool startsWithClick(bool includeVoided = false) const;

    void replace(const QString target, const QString newString, const Qt::CaseSensitivity caseSensitive = Qt::CaseSensitive);    // Replace parts of string with new string
    void replace(const int start, const int numChars, const QString newString);    // Replace parts of string with new string
    void replaceHTMLentities();   // Replaces &nbsp; and other entities with real character
    void decodeHTMLchars();       // Replaces &#233 et al. with actual character
    void removeSpecialChar();     // Remove escape character (\), ampersand (&) and double quote (") since they will interfere with streams
    void removeQuotes();          // Removes all instances of double quotes - QChar(34)
    bool removeHyphens();
    void removeBlankSentences(bool firstPass = true);  // Replaces all ". ." with "."
    void convertToQStringList(QStringList &nameList);
    QString unQuoteHTML();
    bool hasBookEnds(const unsigned int bookends);
    bool removeBookEnds(unsigned int bookends = PARENTHESES | QUOTES, bool runRecursive = false);
    bool removeLeading(const unsigned int charCode);
    bool removeLeading(const std::wstring target, const Qt::CaseSensitivity cs = Qt::CaseSensitive);
    bool removeLeading(const QString target, const Qt::CaseSensitivity cs = Qt::CaseSensitive);
    bool removeEnding(const unsigned int charCode);
    bool removeEnding(const std::wstring target, const Qt::CaseSensitivity cs = Qt::CaseSensitive);
    bool removeEnding(const QString target, const Qt::CaseSensitivity cs = Qt::CaseSensitive);
    bool removeAll(const QString target, const Qt::CaseSensitivity cs = Qt::CaseSensitive);
    bool removeLineFormatting();
    bool removePossessive();
    bool removeCelebrations();
    bool cleanUpEnds();

    unsigned int countWords(unsigned int delimiter = SPACE, unsigned int WORDTYPE = ALPHA) const;
	unsigned int drawOutNumber() const;

    static QList<QString> MONTHS;

protected:
    QString itsString;

    static QString diacriticLetters;
    static QList<QString> noDiacriticLetters;
};

#endif
