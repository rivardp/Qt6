// OQStream.h

#ifndef OQSTREAM_H
#define OQSTREAM_H

//#include <QTextCodec>
//#include <QDate>
#include <QDateTime>
#include "OQString.h"

class OQStream:public OQString
{

public:
    OQStream();
    OQStream(const OQStream &source);
    OQStream(const OQString source);
    OQStream(const PQString source);
    OQStream(const QString source);
    OQStream(const std::wstring source);

	void forward(unsigned int numPosition = 1);
	void backward(unsigned int numPosition = 1);
	void beg();
    void end();
    unsigned int getPosition() const;
    bool moveTo(const QString &string, int maxMove = 100000000, unsigned int repositionAfter = 0);
    bool moveToEarliestOf(const QString &string1, const QString &string2, int maxMove = 100000000);
    bool moveToEarliestOf(const QString &string1, const QString &string2, const QString &string3, int maxMove = 100000000);
    bool moveBackwardTo(const QString &string, int maxMove = 1000);
    bool conditionalMoveTo(const QString &string, const QString &stopString, unsigned int repositionAfter = 0, int maxMove = 100000000);
    bool consecutiveMovesTo(const int maxMove, const QString &string1, const QString &string2, const QString &string3 = QString(), const QString &string4 = QString());

    bool isEOS() const;
    bool contains(const QString &string, bool ignoreBookendedLetters = false);
    bool hasAllWordsCapitalized(bool mustBeAlpha = true);
    bool streamIsJustDates();
    int firstDifferenceFrom(QString comparisonString);

    OQString get();
    QChar    getQChar();
    OQString getWord(const bool considerParentheses = false, unsigned int secondaryBreakChar = SLASH, const bool secondaryBreakInParentheses = true);
    OQString getCleanWord(const bool considerParentheses = false);
    OQStream getSentence(const LANGUAGE &lang = language_unknown);
    OQStream getSentence(bool &realSentenceEncounteredFlag, const LANGUAGE &lang = language_unknown);
    OQStream getSentence(const QList<QString> &firstNames, bool &realSentenceEncounteredFlag, const LANGUAGE &lang = language_unknown);
    OQStream getSentence(const QList<QString> &firstNames, bool &realSentenceEncounteredFlag, const LANGUAGE &lang, const QStringList &conditionalStopWords);
    //OQStream getSentence(QList<QString> &listOfInitials);
    OQString getNext(unsigned int numChars);
    OQString getUntil(QString qchar, unsigned int maxNumChars = 10000, bool dropLast = true);
    OQString getUntilEarliestOf(QString stop1, QString stop2, unsigned int maxNumChars = 10000, bool dropLast = true);
    OQString peekAtWord(const bool considerParentheses = false, const unsigned int howFar = 1);
    OQString peekAtNext(const unsigned int numChars);
    OQString peekAtNextRealChar();
    OQString readHTMLContent(unsigned int maxChar = 200);
    OQString readNextBetween(unsigned int param);
    OQString readQuotedMetaContent();

    OQStream& removeHTMLtags(int insertPeriods = 0);
    OQStream& removeContentWithin(unsigned int bookends = PARENTHESES | QUOTES);

    OQStream& operator= (const OQStream &rhs);
    OQStream& operator= (const OQString &rhs);
    OQStream& operator= (const PQString &rhs);
    OQStream& operator= (const QString &rhs);
    OQStream& operator= (const std::string &rhs);
    OQStream& operator= (const std::wstring &rhs);
    OQStream& operator= (const QByteArray &rhs);

    bool loadStringValue(const QString from, QString &toString, const bool stopAtRecord);
    bool loadValue(const QString from, QString &toString, const bool stopAtRecord = true);
    bool loadValue(const QString from, PQString &toPQString, const bool stopAtRecord = true);
    bool loadValue(const QString from, int &toInt, const bool stopAtRecord = true);
    bool loadValue(const QString from, unsigned int &toUInt, const bool stopAtRecord = true);
    bool loadValue(const QString from, bool &toBool, const bool stopAtRecord = true);
    bool loadValue(const QString from, QDate &toDate, const QString dateFormat, const bool stopAtRecord = true);

    // Overloaded functions for OQString and PQString necessary to set EOS properly
    // PQString
    void dropLeft(unsigned int num);
    void dropRight(unsigned int num);
    PQString& simplify(bool insertPeriods = false);
    void replace(const QString target, const QString newString, const Qt::CaseSensitivity caseSensitive = Qt::CaseSensitive);    // Replace parts of string with new string
    void replace(const int start, const int numChars, const QString newString);    // Replace parts of string with new string
    void replaceHTMLentities();   // Replaces &nbsp; and other entities with real character
    void removeLinks();           // Remove links and the tag wording
    void removeSpecialChar();     // Remove escape character (\), ampersand (&) and double quote (") since they will interfere with streams
    bool removeHyphens();
    void removeBlankSentences();  // Replaces all ". ." with "."
    bool removeBookEnds(unsigned int bookends = PARENTHESES | QUOTES);
    bool removeLeading(const unsigned int charCode);
    bool removeLeading(const std::wstring target);
    bool removeLeading(const QString target);
    bool removeEnding(const unsigned int charCode);
    bool removeEnding(const std::wstring target);
    bool removeEnding(const QString target);
    bool removeAll(const QString target);
    bool removePossessive();
    bool removeRepeatedLastName();
    bool cleanUpEnds();

    // OQString
    bool removeOrdinal(LANGUAGE language = language_unknown);
    bool removeLeadingNeeEtAl(LANGUAGE language = language_unknown);
    bool removeLeadingAKA();
    bool removeInternalPeriods();

protected:
    int position;
	bool EOS;
};


#endif
