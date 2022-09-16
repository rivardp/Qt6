// PQDate.cpp

#include "PQDate.h"


double elapse(const QDate &dateBeg, const QDate &dateEnd)
{
    if (!(dateBeg.isValid() && dateEnd.isValid()))
        return -99999;

    return (dateEnd.year() - dateBeg.year()) + (dateEnd.month() - dateBeg.month())/12.0 + (dateEnd.day() - dateBeg.day())/360.0;
}
