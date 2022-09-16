#include "postalcodes.h"
#include "postalCodeHash.h"

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

    if ((pc.length() == 7) && pcHash.contains(pc))
    {
        lookupResult = pcHash.value(pc);
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
