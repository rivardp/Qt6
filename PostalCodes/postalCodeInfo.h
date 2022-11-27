#ifndef POSTALCODEINFO_H
#define POSTALCODEINFO_H

#include <QString>
#include "../UpdateFuneralHomes/Include/enums.h"

class POSTALCODE_INFO
{
public:
    POSTALCODE_INFO();
    POSTALCODE_INFO(const POSTALCODE_INFO &);
    POSTALCODE_INFO(const QString postal_code, const QString city, const QString province, const QString province_abbr, const int province_enum,
                    const int time_zone, const double latitude, const double longitude);  // Constructor assumes valid data - use setData() otherwise

    QString getPostalCode() const;
    QString getCity() const;
    QString getProvince() const;
    QString getProv() const;
    PROVINCE getPROVINCE() const;
    int getTimeZone() const;
    double getLatitude() const;
    double getLongitude() const;

    bool setPostalCode(QString pc);
    void setData(const QString postal_code, const QString city, const QString province, const QString province_abbr, const int province_enum,
                 const int time_zone, const double latitude, const double longitude);
    void clear();

    bool isValid() const;

    void convertPCtoProv(QString postalCode, PROVINCE &prov);
    void convertPCtoProv(QString postalCode, QString &prov, bool abbreviated = false);
    double distanceTo(POSTALCODE_INFO &pc2) const;
    double distanceBetween(POSTALCODE_INFO &pc1, POSTALCODE_INFO &pc2) const;

    POSTALCODE_INFO& operator= (const POSTALCODE_INFO &rhs);
    bool operator== (const POSTALCODE_INFO &rhs);
    bool operator!= (const POSTALCODE_INFO &rhs);


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
    void validate();
};


#endif // POSTALCODEINFO_H
