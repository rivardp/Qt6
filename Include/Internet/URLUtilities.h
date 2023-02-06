// URLUtilities.h

#ifndef URLUTILITIES_H
#define URLUTILITIES_H

#include "../Include/Penser/PQString.h"
#include "../UpdateFuneralHomes/Include/dataMiningStructure.h"

void createURLaddress(PQString &result, const PQString &URLtemplate, const PQString p1 = QString(""), const PQString p2 = QString(""),
                      const PQString p3 = QString(""), const PQString p4 = QString(""), const PQString p5 = QString(""), const PQString p6 = QString(""));

void createURLaddress(PQString &result, const PQString &URLtemplate, URLPARAMS &urlParams);
void createURLaddress(DOWNLOADREQUEST &downloadRequest, const PQString &URLtemplate, URLPARAMS &urlParams);
void createURLaddress(DOWNLOADREQUEST &downloadRequest, const PQString &URLtemplate, URLPARAMS &urlParams, PAYLOADPARAMS &payloadParams);

PQString URLparam(const PQString &param);
PQString URLparam(const unsigned int &param);

bool validateURL(PQString &HTTP, PQString &WWW, PQString &url);


#endif
