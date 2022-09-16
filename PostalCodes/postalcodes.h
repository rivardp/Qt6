#ifndef POSTALCODES_H
#define POSTALCODES_H

#include <QString>
#include "postalCodeHash.h"
#include "postalCodeInfo.h"

class PostalCodes
{
public:
    PostalCodes();
    ~PostalCodes();
    POSTALCODE_INFO lookup(QString);
    PostalCodes* getPostalCodesClassPointer();

    friend POSTALCODE_INFO;

private:
    PostalCodes *p;
};

//extern "C" POSTALCODES_EXPORT PostalCodes * getPostalCodesClassPointer();
typedef PostalCodes* (*GetPostalCodes)();

#endif // POSTALCODES_H
