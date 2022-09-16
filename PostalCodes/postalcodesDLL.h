#ifndef POSTALCODES_H
#define POSTALCODES_H

#include <QString>
#include "postalCodeInfoDLL.h"

#if defined(POSTALCODES_LIBRARY)
    #define POSTALCODES_EXPORT __declspec(dllexport)
#else
    #define POSTALCODES_EXPORT __declspec(dllimport)
#endif

class POSTALCODES_EXPORT PostalCodes
{
public:
    PostalCodes();
    virtual ~PostalCodes();
    virtual POSTALCODE_INFO lookup(QString);
    virtual PostalCodes* getPostalCodesClassPointer();

private:
    PostalCodes *p;
};

extern "C" POSTALCODES_EXPORT PostalCodes * getPostalCodesClassPointer();
typedef PostalCodes* (*GetPostalCodes)();

#endif // POSTALCODES_H
