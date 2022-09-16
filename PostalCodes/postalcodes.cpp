#include "postalcodes.h"

PostalCodes::PostalCodes()
{
    p = nullptr;
}

PostalCodes::~PostalCodes()
{
    delete p;
}

POSTALCODE_INFO PostalCodes::lookup(QString pc)
{
    POSTALCODE_INFO lookupResult;
    QString postalCode = pc.left(3);

    if ((postalCode.length() == 3) && pcHash.contains(postalCode))
    {
        lookupResult = pcHash.value(postalCode);
        lookupResult.setValid(true);
    }

    return lookupResult;
}

PostalCodes* PostalCodes::getPostalCodesClassPointer()
{
    if (p)
        return p;
    else
    {
        p = new PostalCodes();
        return p;
    }
}
