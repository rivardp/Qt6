// OQStream.h

#ifndef OQSTREAM_H
#define OQSTREAM_H

#include "OQString.h"
#include "PQStream.h"

class OQStream:public PQStream
{
public:
        OQStream();
        OQStream(const OQString &source);
        OQStream(const PQString &source);
        OQStream(const std::wstring &source);
        ~OQStream();

        OQString get();
        OQString getWord(const bool considerParentheses = false);
        OQString peekAtWord(const bool considerParentheses = false);

        OQStream& operator= (const OQStream &rhs);
        OQStream& operator= (const PQStream &rhs);
        OQStream& operator= (const std::wstring &rhs);

protected:

};


#endif
