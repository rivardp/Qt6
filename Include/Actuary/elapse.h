// elapse.h   Version 1.0     January 13, 2020 - Extracted from prior date.h

#ifndef ELAPSE_H
#define ELAPSE_H

#ifndef DATE_H
#include "../Actuary/date.h"
#endif

#ifndef ERRORFLAG_H
#include "../Actuary/errorFlag.h"
#endif

double elapse(const unsigned long int &nd1, const unsigned long int &nd2, errorFlag &Error);
double elapse(const date &d1, const date &d2, errorFlag &Error);

#endif
