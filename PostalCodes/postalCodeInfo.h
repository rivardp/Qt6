#ifndef POSTALCODEINFO_H
#define POSTALCODEINFO_H

#include <QString>
#include "../UpdateFuneralHomes/Include/enums.h"

class POSTALCODE_INFO
{
public:
    POSTALCODE_INFO();
    // Next constructor is strictly for use with hash
    POSTALCODE_INFO(const QString postal_code, const QString city, const QString province, const QString province_abbr, const int province_enum,
                    const int time_zone, const double latitude, const double longitude);

    QString getPostalCode() const;
    QString getCity() const;
    QString getProvince() const;
    QString getProv() const;
    PROVINCE getPROVINCE() const;
    int getTimeZone() const;
    double getLatitude() const;
    double getLongitude() const;

    bool setPostalCode(QString pc);
    void clear();

    bool isValid() const;

    void convertPCtoProv(QString postalCode, PROVINCE &prov);
    void convertPCtoProv(QString postalCode, QString &prov, bool abbreviated = false);
    double distanceTo(POSTALCODE_INFO &pc2) const;
    double distanceBetween(POSTALCODE_INFO &pc1, POSTALCODE_INFO &pc2) const;

    POSTALCODE_INFO& operator= (const POSTALCODE_INFO &rhs);
    bool operator== (const POSTALCODE_INFO &rhs);
    bool operator!= (const POSTALCODE_INFO &rhs);

    friend class PostalCodes;


private:
    QString postal_code;
    QString city;
    QString province;
    QString province_abbr;
    PROVINCE province_enum;
    int time_zone;
    double latitude;
    double longitude;

    bool valid;

    void setValid(bool status);
};


#endif // POSTALCODEINFO_H
