#include "../Include/Internet/URLUtilities.h"
#include "../Include/Penser/PQString.h"

void createURLaddress(PQString &result, const PQString &URLtemplate, const PQString p1, const PQString p2,
    const PQString p3, const PQString p4, const PQString p5, const PQString p6)
{
	result = URLtemplate;
	if (p1.getLength() > 0)
		result.insertParameter(p1);
	if (p2.getLength() > 0)
		result.insertParameter(p2);
	if (p3.getLength() > 0)
		result.insertParameter(p3);
	if (p4.getLength() > 0)
		result.insertParameter(p4);
	if (p5.getLength() > 0)
		result.insertParameter(p5);
    if (p6.getLength() > 0)
        result.insertParameter(p6);
}

void createURLaddress(PQString &result, const PQString &URLtemplate, URLPARAMS &urlParams)
{
    urlParams.updateParams();
    createURLaddress(result, URLtemplate, urlParams.param1, urlParams.param2, urlParams.param3, urlParams.param4, urlParams.param5, urlParams.param6);
}

void createURLaddress(DOWNLOADREQUEST &downloadRequest, const PQString &URLtemplate, URLPARAMS &urlParams)
{
    PQString result;
    createURLaddress(result, URLtemplate, urlParams);
    downloadRequest.instructions.url = result;
    downloadRequest.instructions.qUrl = QUrl(downloadRequest.instructions.url.getString());
}

PQString URLparam(const PQString &param)
{
	return param;
}

PQString URLparam(const unsigned int &param)
{
    return QString::number(param);
}

bool validateURL(PQString &HTTP, PQString &WWW, PQString &url)
{
    PQString origString = url;
    PQString subString;
    QString tempString;

    int position;

    // Check for more than one double slash occurrence
    position = url.findPosition(PString("//"), 1, 0, 2);
    if (position != -1)
        return false;

    // Check to ensure www. is present
    position = url.findPosition(PString("//"), 1, 0, 1);
    PQString www("www.");
    if (position >= 0)
    {
        unsigned int uPosition = static_cast<unsigned int>(position);
        subString = url.middle(uPosition + 2, 4);
        if (subString != www)
            url = origString.left(uPosition) + www + origString.right(origString.getLength() - uPosition);
    }
    else
    {
        subString = url.left(4);
        if (subString != www)
            url = www + origString;
    }

    // Check to ensure http:// is present
    position = url.findPosition(HTTP, 1, 0, 1);
    if (position == -1)
        url = HTTP + PQString("//") + url;
    else
    {
        if (position > 0)
            return false;
    }

    // Replace "www." with WWW provide
    tempString = url.getString();
    tempString.replace(QString("www."), WWW.getString());
    url = tempString;

    return true;
}
