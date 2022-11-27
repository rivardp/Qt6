#include "postalCodeInfo.h"


POSTALCODE_INFO::POSTALCODE_INFO() : valid(false)
{
}

POSTALCODE_INFO::POSTALCODE_INFO(const QString init_postal_code, const QString init_city, const QString init_province, const QString init_province_abbr, const int init_province_enum,
                                 const int init_time_zone, const double init_latitude, const double init_longitude)
{
    setData(init_postal_code, init_city, init_province, init_province_abbr, init_province_enum, init_time_zone, init_latitude, init_longitude);
    valid = true;
}

POSTALCODE_INFO::POSTALCODE_INFO(const POSTALCODE_INFO &rhs)
{
    setData(rhs.postal_code, rhs.city, rhs.province, rhs.province_abbr, rhs.province_enum, rhs.time_zone, rhs.latitude, rhs.longitude);
    validate();
}

void POSTALCODE_INFO::setData(const QString init_postal_code, const QString init_city, const QString init_province, const QString init_province_abbr, const int init_province_enum,
                              const int init_time_zone, const double init_latitude, const double init_longitude)
{
    clear();

    postal_code = init_postal_code;
    city = init_city;
    province = init_province;
    province_abbr = init_province_abbr;
    province_enum = static_cast<PROVINCE>(init_province_enum);
    time_zone = init_time_zone;
    latitude = init_latitude;
    longitude = init_longitude;

    validate();
}

void POSTALCODE_INFO::validate()
{
    if (((postal_code.length() == 7) || (postal_code.length() == 3)) && (province.length() > 0) && (province_enum != provUnknown) && (latitude != 0) && (longitude != 0))
        valid = true;
    else
        valid = false;
}

void POSTALCODE_INFO::setValid(bool status)
{
    valid = status;
}

QString POSTALCODE_INFO::getPostalCode() const
{
    return postal_code;
}

QString POSTALCODE_INFO::getCity() const
{
    return city;
}

QString POSTALCODE_INFO::getProvince() const
{
    return province;
}

QString POSTALCODE_INFO::getProv() const
{
    return province_abbr;
}

PROVINCE POSTALCODE_INFO::getPROVINCE() const
{
    if (valid)
        return province_enum;
    else
        return PROVINCE::provUnknown;
}

int POSTALCODE_INFO::getTimeZone() const
{
    if (valid)
        return time_zone;
    else
        return -1;
}

double POSTALCODE_INFO::getLatitude() const
{
    if (valid)
        return latitude;
    else
        return 0;
}

double POSTALCODE_INFO::getLongitude() const
{
    if (valid)
        return longitude;
    else
        return 0;
}

bool POSTALCODE_INFO::isValid() const
{
    return valid;
}

POSTALCODE_INFO& POSTALCODE_INFO::operator= (const POSTALCODE_INFO &rhs)
{
    this->clear();
    if (rhs.isValid())
    {
        postal_code = rhs.postal_code;
        city = rhs.city;
        province = rhs.province;
        province_abbr = rhs.province_abbr;
        province_enum = rhs.province_enum;
        time_zone = rhs.time_zone;
        latitude = rhs.latitude;
        longitude = rhs.longitude;

        valid = rhs.valid;
    }

    return *this;
}

bool POSTALCODE_INFO::operator== (const POSTALCODE_INFO &rhs)
{
    return postal_code == rhs.postal_code;
}

bool POSTALCODE_INFO::operator!= (const POSTALCODE_INFO &rhs)
{
    return postal_code != rhs.postal_code;
}

void POSTALCODE_INFO::clear()
{
    postal_code.clear();
    city.clear();
    province.clear();
    province_abbr.clear();
    province_enum = provUnknown;
    time_zone = -1;
    latitude = 0;
    longitude = 0;

    valid = false;
}

double POSTALCODE_INFO::distanceTo(POSTALCODE_INFO &pc2) const
{
    double lat1, long1, lat2, long2;
    double dlat, dlong;
    double result;

    if (this->isValid() && pc2.isValid())
    {
        // Convert degrees to radians
        lat1  = this->getLatitude()  * 3.1415926535897932384626433832795028841971 / 180;
        long1 = this->getLongitude() * 3.1415926535897932384626433832795028841971 / 180;
        lat2  = pc2.getLatitude()  * 3.1415926535897932384626433832795028841971 / 180;
        long2 = pc2.getLongitude() * 3.1415926535897932384626433832795028841971 / 180;

        // Haversine Formula
        dlong = long2 - long1;
        dlat = lat2 - lat1;

        result = pow(sin(dlat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlong / 2), 2);
        result = 2 * asin(sqrt(result));

        // Radius of Earth in Kilometers, R = 6371 - Use R = 3956 for miles
        double R = 6371;
        result = result * R;

        return result;
    }
    else
        return 9999999;
}

double POSTALCODE_INFO::distanceBetween(POSTALCODE_INFO &pc1, POSTALCODE_INFO &pc2) const
{
    return pc1.distanceTo(pc2);
}

void POSTALCODE_INFO::convertPCtoProv(QString postalCode, PROVINCE &prov)
{
    QChar firstLetter = postalCode.at(0);
    //enum PROVINCE { provUnknown = 0, BC, AB, SK, MB, ON, QC, NB, NS, PEI, NL, YT, NT, NU};

    switch (firstLetter.unicode())
    {
    case 66:    // A
        prov = NL;
        break;

    case 67:    // B
        prov = NS;
        break;

    case 68:    // C
        prov = PE;
        break;

    case 70:    // E:
        prov = NB;
        break;

    case 72:    // G
    case 73:    // H
    case 74:    // I
    case 75:    // J
        prov = QC;
        break;

    case 76:    // K
    case 77:    // L
    case 78:    // M
    case 79:    // N
    case 80:    // O
    case 81:    // P
        prov = ON;
        break;

    case 82:    // R
        prov = MB;
        break;

    case 83:    // S
        prov = SK;
        break;

    case 84:    // T
        prov = AB;
        break;

    case 86:    // V
        prov = BC;
        break;

    case 88:    // X
        prov = NT;
        break;

    case 89:    // Y
        prov = YT;
        break;

    default:
        prov = provUnknown;
        break;
    }
}

void POSTALCODE_INFO::convertPCtoProv(QString postalCode, QString &prov, bool abbreviated)
{
    PROVINCE enumProv;
    convertPCtoProv(postalCode, enumProv);

    switch(enumProv)
    {
    case BC:
        if (abbreviated)
            prov = "BC";
        else
            prov = "British Columbia";
        break;

    case AB:
        if (abbreviated)
            prov = "AB";
        else
            prov = "Alberta";
        break;

    case SK:
        if (abbreviated)
            prov = "SK";
        else
            prov = "Saskatchewan";
        break;

    case MB:
        if (abbreviated)
            prov = "MB";
        else
            prov = "Manitoba";
        break;

    case ON:
        if (abbreviated)
            prov = "ON";
        else
            prov = "Ontario";
        break;

    case QC:
        if (abbreviated)
            prov = "QC";
        else
            prov = "Québec";
        break;

    case NB:
        if (abbreviated)
            prov = "NB";
        else
            prov = "New Brunswick";
        break;

    case NS:
        if (abbreviated)
            prov = "NS";
        else
            prov = "Nova Scotia";
        break;

    case PE:
        if (abbreviated)
            prov = "PE";
        else
            prov = "Prince Edward Island";
        break;

    case NL:
        if (abbreviated)
            prov = "NL";
        else
            prov = "Newfoundland";
        break;

    case YT:
        if (abbreviated)
            prov = "YT";
        else
            prov = "Yukon";
        break;

    case NT:
        if (abbreviated)
            prov = "NT";
        else
            prov = "NorthWest Territories";
        break;

    case NU:
        if (abbreviated)
            prov = "NU";
        else
            prov = "Nunavet";
        break;

    default:
        prov = "";
        break;
    }
}
