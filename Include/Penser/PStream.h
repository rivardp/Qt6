// PStream.h

#ifndef PSTREAM_H
#define PSTREAM_H

#include "PString.h"

class PStream:public PString
{
	friend class OStream;

public:
	PStream();
	PStream(const PString &source);
	PStream(const std::wstring &source);
	~PStream();

	void forward(unsigned int numPosition = 1);
	void backward(unsigned int numPosition = 1);
	void beg();
	bool isEOS() const;
	PString get();
	PString getWord(const bool considerParentheses = false);
	PString peekAtWord(const bool considerParentheses = false);

	PStream& operator= (const PStream &rhs);
	PStream& operator= (const std::wstring &rhs);


protected:
	unsigned int position;
	bool EOS;
};


#endif