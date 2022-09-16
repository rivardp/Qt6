#include "postalCodeInfoDLL.h"


POSTALCODE_INFO::POSTALCODE_INFO()
{
    valid = false;
}

POSTALCODE_INFO::POSTALCODE_INFO(const QString init_postal_code, const QString init_city, const QString init_province, const QString init_province_abbr, const int init_province_enum,
                                 const int init_time_zone, const double init_latitude, const double init_longitude)
{
    valid = true;

    postal_code = init_postal_code;
    city = init_city;
    province = init_province;
    province_abbr = init_province_abbr;
    province_enum = static_cast<PROVINCE>(init_province_enum);
    time_zone = init_time_zone;
    latitude = init_latitude;
    longitude = init_longitude;
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
        return 99999999;
}

double POSTALCODE_INFO::distanceBetween(POSTALCODE_INFO &pc1, POSTALCODE_INFO &pc2) const
{
    return pc1.distanceTo(pc2);
}

void POSTALCODE_INFO::convertPCtoProv(QString postalCode, PROVINCE &prov)
{
    QChar firstLetter = postalCode.at(1);
    //enum PROVINCE { provUnknown = 0, BC, AB, SK, MB, ON, QC, NB, NS, PEI, NL, YT, NT, NU};

    switch (firstLetter.digitValue())
    {
    case 'A':
        prov = NL;
        break;

    case 'B':
        prov = NS;
        break;

    case 'C':
        prov = PE;
        break;

    case 'E':
        prov = NB;
        break;

    case 'G':
    case 'H':
    case 'I':
    case 'J':
        prov = QC;
        break;

    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
        prov = ON;
        break;

    case 'R':
        prov = MB;
        break;

    case 'S':
        prov = SK;
        break;

    case 'T':
        prov = AB;
        break;

    case 'V':
        prov = BC;
        break;

    case 'Y':
        prov = YT;
        break;

    case 'Z':
        prov = NS;
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

    if (abbreviated)
        prov = static_cast<QString>(enumProv);
    else
    {
        switch(enumProv)
        {
        case BC:
            prov = "British Columbia";
            break;

        case AB:
            prov = "Alberta";
            break;

        case SK:
            prov = "Saskatchewan";
            break;

        case MB:
            prov = "Manitoba";
            break;

        case ON:
            prov = "Ontario";
            break;

        case QC:
            prov = "Québec";
            break;

        case NB:
            prov = "New Brunswick";
            break;

        case NS:
            prov = "Nova Scotia";
            break;

        case PE:
            prov = "Prince Edward Island";
            break;

        case NL:
            prov = "Newfoundland";
            break;

        case YT:
            prov = "Yukon";
            break;

        case NT:
            prov = "NorthWest Territories";
            break;

        case NU:
            prov = "Nunavet";
            break;

        default:
            prov = "";
            break;
        }
    }
}

