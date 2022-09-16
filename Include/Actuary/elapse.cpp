// elapse.cpp    Version 1.0   Extracted from date.cpp

#include "../Actuary/elapse.h"

double elapse(const unsigned long int &nd1, const unsigned long int &nd2, errorFlag &Error)
{
    date dateBeg(nd1, Error);
    if (Error) return 0;
    date dateEnd(nd2, Error);
    if (Error) return 0;

    return elapse(dateBeg, dateEnd, Error);
}

double elapse(const date &dateBeg, const date &dateEnd, errorFlag &Error)
{
    double period;

    if (dateBeg.numericValue() > dateEnd.numericValue())
        return -elapse(dateEnd,dateBeg,Error);

    if (dateBeg.year() == dateEnd.year())
    {
        period = (static_cast<double>(dateEnd.numericValue() - dateBeg.numericValue())) /
                    dateBeg.daysInYear();
    }
    else
    {
        double t1, t2, t3;
        date endOfFirstYear(dateBeg.year()+1,1,1, Error);
        if (Error) return 0;
        date begOfLastYear(dateEnd.year(),1,1, Error);
        if (Error) return 0;
        t1 = (static_cast<double>(endOfFirstYear.numericValue() - dateBeg.numericValue())) /
                    dateBeg.daysInYear();
        t2 = dateEnd.year() - endOfFirstYear.year();
        t3 = (static_cast<double>(dateEnd.numericValue() - begOfLastYear.numericValue())) /
                    dateEnd.daysInYear();
        period = t1 + t2 + t3;
    }

    return static_cast<double>(0.0001 * static_cast<long int>(period * 10000 + 0.5));
}
