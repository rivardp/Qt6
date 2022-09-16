// PQStream.h

#ifndef PQSTREAM_H
#define PQSTREAM_H

//#include <QTextCodec>
#include "PQString.h"

class PQStream:public PQString
{

public:
    PQStream();
    PQStream(const PQString &source);
    PQStream(const std::wstring &source);
    ~PQStream();

	void forward(unsigned int numPosition = 1);
	void backward(unsigned int numPosition = 1);
	void beg();
    bool moveTo(const QString &string);
    bool consecutiveMovesTo(const unsigned int maxMove, const QString &string1, const QString &string2, const QString &string3 = QString(' '), const QString &string4 = QString(' '));

    bool isEOS() const;

    PQString get();
    QChar getQChar();
    PQString getWord(const bool considerParentheses = false);
    PQString getNext(const unsigned int numChars);
    PQString getUntil(QString qchar, bool dropLast = true);
    PQString peekAtWord(const bool considerParentheses = false);
    PQString peekAtNext(const unsigned int numChars);
    PQString readHTMLContent(unsigned int maxChar = 200);
    PQString readNextBetween(unsigned int param);

    PQStream& operator= (const PQStream &rhs);
    PQStream& operator= (const std::string &rhs);
    PQStream& operator= (const std::wstring &rhs);
    PQStream& operator= (const QByteArray &rhs);


protected:
	unsigned int position;
	bool EOS;
};


#endif
