// dataMiningStructure.h
//

#ifndef DATA_MINING_STRUCTURE_H
#define DATA_MINING_STRUCTURE_H

#include <QDate>

#include "../Include/Penser/PQString.h"
#include "../Include/Unstructured/qUnstructuredContent.h"

struct RECORD
{
    unsigned int providerID;
    unsigned int providerKey;
    QString url;
    QString POSTformRequest;
    QDate dob;
    QDate dod;
    unsigned int yob;
    unsigned int yod;
    unsigned int ageAtDeath;
    QDate pubdate;
    QString maidenNames;
    QString ID;
    QString pcKey;
    QString fullName;
    QString snippet;

    void clear(){
        providerID = 0;
        providerKey = 0;
        url.clear();
        POSTformRequest.clear();
        dob.setDate(1900,1,1);
        dod.setDate(1900,1,1);
        yob = 0;
        yod = 0;
        ageAtDeath = 0;
        pubdate.setDate(1900,1,1);
        maidenNames.clear();
        ID.clear();
        pcKey.clear();
        fullName.clear();
        snippet.clear();
    }
};

struct PAGEVARIABLES
{
    unsigned int providerID;
    unsigned int providerKey;
    unsigned int yob;
    unsigned int yod;
    unsigned int ageAtDeath;

    PQString webAddress;
    PQString maidenName;

    QDate currentDOB;
    QDate currentDOD;
    QDate currentPubDate;
    QDate priorDODdate;
    QDate latestDODdate;
    QDate lastRunDate;
    QDate firstAvailableDate;
    QDate cutoffDate;
    QDate queryTargetDate;
    QDate queryStartDate;
    QDate queryEndDate;

    QString POSTformRequest;
    QString ID;
    QString lastID;
    QString lastDate;
    QString fhURLid;
    QString fhURLidDivider;
    QString cutOffID;
    QString pcKey;
    QString fullName;
    QString snippet;

    bool sequentialID;
    bool alphabeticalListing;
    bool paginatedResult;
    bool includeRecord;
    bool usePubDateForCutOff;

    unstructuredContent ucDOB;
    unstructuredContent ucDOD;
    unstructuredContent ucDOBandDOD;
    unstructuredContent ucPubDate;
    unstructuredContent ucVariable;
    DATEORDER numericDateOrder;

    PAGEVARIABLES() : numericDateOrder(doNULL) {}

    void clear(){
        providerID = 0;
        providerKey = 0;
        yob = 0;
        yod = 0;
        ageAtDeath = 0;
        webAddress.clear();
        maidenName.clear();
        pcKey.clear();
        fullName.clear();
        snippet.clear();
        currentDOB.setDate(1900,1,1);
        currentDOD.setDate(1900,1,1);
        currentPubDate.setDate(1900,1,1);
        priorDODdate.setDate(1900,1,1);
        latestDODdate.setDate(1900,1,1);
        lastRunDate.setDate(1900,1,1);
        firstAvailableDate.setDate(1900,1,1);
        POSTformRequest.clear();
        ID.clear();
        lastID.clear();
        lastDate.clear();
        fhURLid.clear();
        fhURLidDivider.clear();
        cutOffID.clear();
        ucDOB.clear();
        ucDOD.clear();
        ucDOBandDOD.clear();
        ucPubDate.clear();
        ucVariable.clear();
        numericDateOrder = doNULL;
    }

    void reset(){
        webAddress.clear();
        maidenName.clear();
        pcKey.clear();
        fullName.clear();
        snippet.clear();
        POSTformRequest.clear();
        ID.clear();
        currentDOB.setDate(1900,1,1);
        currentDOD.setDate(1900,1,1);
        currentPubDate.setDate(1900,1,1);
        ucDOB.clear();
        ucDOD.clear();
        ucDOBandDOD.clear();
        ucPubDate.clear();
        ucVariable.clear();
        yob = 0;
        yod = 0;
        ageAtDeath = 0;
    }
};

enum PARAMTYPE {ptNotSet, ptUint, ptInt, ptQS, ptPQS, ptQTDyear, ptQTDmonth, ptQTDday, ptQTDmask, ptJulian};
enum MASKTYPE  {mtNoMask, mtYYYYMM, mtMMhYYYY, mtDDsMMsYYYY, mtMMsDDsYYYY, mtYYYYhMMhDD, mtYYYY, mtMM, mtDD, mtM, mtD, mtMMMM};

struct URLPARAMS
{
    PQString param1;
    PQString param2;
    PQString param3;
    PQString param4;
    PQString param5;
    PQString param6;

    PARAMTYPE param1Type;
    PARAMTYPE param2Type;
    PARAMTYPE param3Type;
    PARAMTYPE param4Type;
    PARAMTYPE param5Type;
    PARAMTYPE param6Type;

    unsigned int *UIparam1;
    unsigned int *UIparam2;
    unsigned int *UIparam3;
    unsigned int *UIparam4;
    unsigned int *UIparam5;
    unsigned int *UIparam6;

    int *IntParam1;
    int *IntParam2;
    int *IntParam3;
    int *IntParam4;
    int *IntParam5;
    int *IntParam6;

    QString *QSparam1;
    QString *QSparam2;
    QString *QSparam3;
    QString *QSparam4;
    QString *QSparam5;
    QString *QSparam6;

    PQString *PQSparam1;
    PQString *PQSparam2;
    PQString *PQSparam3;
    PQString *PQSparam4;
    PQString *PQSparam5;
    PQString *PQSparam6;

    QDate *QTDdate1;
    QDate *QTDdate2;
    QDate *QTDdate3;
    QDate *QTDdate4;
    QDate *QTDdate5;
    QDate *QTDdate6;

    unsigned int *Julian1;
    unsigned int *Julian2;
    unsigned int *Julian3;
    unsigned int *Julian4;
    unsigned int *Julian5;
    unsigned int *Julian6;

    MASKTYPE maskType1;
    MASKTYPE maskType2;
    MASKTYPE maskType3;
    MASKTYPE maskType4;
    MASKTYPE maskType5;
    MASKTYPE maskType6;

    unsigned int numParams;

    URLPARAMS() : UIparam1(nullptr), UIparam2(nullptr), UIparam3(nullptr), UIparam4(nullptr), UIparam5(nullptr), UIparam6(nullptr),
                  IntParam1(nullptr), IntParam2(nullptr), IntParam3(nullptr), IntParam4(nullptr), IntParam5(nullptr), IntParam6(nullptr),
                  QSparam1(nullptr), QSparam2(nullptr), QSparam3(nullptr), QSparam4(nullptr), QSparam5(nullptr),  QSparam6(nullptr),
                  PQSparam1(nullptr), PQSparam2(nullptr), PQSparam3(nullptr), PQSparam4(nullptr), PQSparam5(nullptr), PQSparam6(nullptr),
                  QTDdate1(nullptr), QTDdate2(nullptr), QTDdate3(nullptr), QTDdate4(nullptr), QTDdate5(nullptr),  QTDdate6(nullptr) {}

    void clear(){
        param1.clear();
        param2.clear();
        param3.clear();
        param4.clear();
        param5.clear();
        param6.clear();

        param1Type = ptNotSet;
        param2Type = ptNotSet;
        param3Type = ptNotSet;
        param4Type = ptNotSet;
        param5Type = ptNotSet;
        param6Type = ptNotSet;
    }

    void updateParams(){

        QDate tempDate;

        if (numParams >= 1)
        {
            switch (param1Type)
            {
            case ptUint:
                param1 = PQString(*UIparam1);
                break;

            case ptInt:
                param1 = PQString(*IntParam1);
                break;

            case ptQS:
                param1 = PQString(*QSparam1);
                break;

            case ptPQS:
                param1 = *PQSparam1;
                break;

            case ptQTDyear:
                tempDate = *QTDdate1;
                param1 = PQString(tempDate.year());
                break;

            case ptQTDmonth:
                tempDate = *QTDdate1;
                param1 = PQString(tempDate.month());
                break;

            case ptQTDday:
                tempDate = *QTDdate1;
                param1 = PQString(tempDate.day());
                break;

            case ptQTDmask:
                tempDate = *QTDdate1;
                switch(maskType1)
                {
                case mtYYYYMM:
                    param1 = PQString(tempDate.toString("yyyyMM"));
                    break;

                case mtMMhYYYY:
                    param1 = PQString(tempDate.toString("MM-yyyy"));
                    break;

                case mtDDsMMsYYYY:
                    param1 = PQString(tempDate.toString("dd/MM/yyyy"));
                    break;

                case mtMMsDDsYYYY:
                    param1 = PQString(tempDate.toString("MM/dd/yyyy"));
                    break;

                case mtYYYYhMMhDD:
                    param1 = PQString(tempDate.toString("yyyy-MM-dd"));
                    break;

                default:
                    break;
                }
                break;

            case ptJulian:
                tempDate = QDate::fromJulianDay(*Julian1);
                switch(maskType1)
                {
                case mtYYYY:
                    param1 = PQString(tempDate.toString("yyyy"));
                    break;

                case mtMM:
                    param1 = PQString(tempDate.toString("MM"));
                    break;

                case mtDD:
                    param1 = PQString(tempDate.toString("dd"));
                    break;

                case mtM:
                    param1 = PQString(tempDate.toString("M"));
                    break;

                case mtD:
                    param1 = PQString(tempDate.toString("d"));
                    break;

                case mtMMMM:
                    param1 = PQString(tempDate.toString("MMMM"));
                    break;

                default:
                break;
                }

            default:
            break;
            }
        }

        if (numParams >= 2)
        {
            switch (param2Type)
            {
            case ptUint:
                param2 = PQString(*UIparam2);
                break;

            case ptInt:
                param2 = PQString(*IntParam2);
                break;

            case ptQS:
                param2 = PQString(*QSparam2);
                break;

            case ptPQS:
                param2 = *PQSparam2;
                break;

            case ptQTDyear:
                tempDate = *QTDdate2;
                param2 = PQString(tempDate.year());
                break;

            case ptQTDmonth:
                tempDate = *QTDdate2;
                param2 = PQString(tempDate.month());
                break;

            case ptQTDday:
                tempDate = *QTDdate2;
                param2 = PQString(tempDate.day());
                break;

            case ptQTDmask:
                tempDate = *QTDdate2;
                switch(maskType2)
                {
                case mtYYYYMM:
                    param2 = PQString(tempDate.toString("yyyyMM"));
                    break;

                case mtMMhYYYY:
                    param2 = PQString(tempDate.toString("MM-yyyy"));
                    break;

                case mtDDsMMsYYYY:
                    param2 = PQString(tempDate.toString("dd/MM/yyyy"));
                    break;

                case mtMMsDDsYYYY:
                    param2 = PQString(tempDate.toString("MM/dd/yyyy"));
                    break;

                case mtYYYYhMMhDD:
                    param2 = PQString(tempDate.toString("yyyy-MM-dd"));
                    break;

                default:
                    break;
                }
                break;

            case ptJulian:
                tempDate = QDate::fromJulianDay(*Julian2);
                switch(maskType2)
                {
                case mtYYYY:
                    param2 = PQString(tempDate.toString("yyyy"));
                    break;

                case mtMM:
                    param2 = PQString(tempDate.toString("MM"));
                    break;

                case mtDD:
                    param2 = PQString(tempDate.toString("dd"));
                    break;

                case mtM:
                    param2 = PQString(tempDate.toString("M"));
                    break;

                case mtD:
                    param2 = PQString(tempDate.toString("d"));
                    break;

                case mtMMMM:
                    param2 = PQString(tempDate.toString("MMMM"));
                    break;

                default:
                break;
                }

            default:
                break;
            }
        }

        if (numParams >= 3)
        {
            switch (param3Type)
            {
            case ptUint:
                param3 = PQString(*UIparam3);
                break;

            case ptInt:
                param3 = PQString(*IntParam3);
                break;

            case ptQS:
                param3 = PQString(*QSparam3);
                break;

            case ptPQS:
                param3 = *PQSparam3;
                break;

            case ptQTDyear:
                tempDate = *QTDdate3;
                param3 = PQString(tempDate.year());
                break;

            case ptQTDmonth:
                tempDate = *QTDdate3;
                param3 = PQString(tempDate.month());
                break;

            case ptQTDday:
                tempDate = *QTDdate3;
                param3 = PQString(tempDate.day());
                break;

            case ptQTDmask:
                tempDate = *QTDdate3;
                switch(maskType3)
                {
                case mtYYYYMM:
                    param3 = PQString(tempDate.toString("yyyyMM"));
                    break;

                case mtMMhYYYY:
                    param3 = PQString(tempDate.toString("MM-yyyy"));
                    break;

                case mtDDsMMsYYYY:
                    param3 = PQString(tempDate.toString("dd/MM/yyyy"));
                    break;

                case mtMMsDDsYYYY:
                    param3 = PQString(tempDate.toString("MM/dd/yyyy"));
                    break;

                case mtYYYYhMMhDD:
                    param3 = PQString(tempDate.toString("yyyy-MM-dd"));
                    break;

                default:
                    break;
                }
                break;

            case ptJulian:
                tempDate = QDate::fromJulianDay(*Julian3);
                switch(maskType3)
                {
                case mtYYYY:
                    param3 = PQString(tempDate.toString("yyyy"));
                    break;

                case mtMM:
                    param3 = PQString(tempDate.toString("MM"));
                    break;

                case mtDD:
                    param3 = PQString(tempDate.toString("dd"));
                    break;

                case mtM:
                    param3 = PQString(tempDate.toString("M"));
                    break;

                case mtD:
                    param3 = PQString(tempDate.toString("d"));
                    break;

                case mtMMMM:
                    param3 = PQString(tempDate.toString("MMMM"));
                    break;

                default:
                break;
                }

            default:
                break;
            }
        }

        if (numParams >= 4)
        {
            switch (param4Type)
            {
            case ptUint:
                param4 = PQString(*UIparam4);
                break;

            case ptInt:
                param4 = PQString(*IntParam4);
                break;

            case ptQS:
                param4 = PQString(*QSparam4);
                break;

            case ptPQS:
                param4 = *PQSparam4;
                break;

            case ptQTDyear:
                tempDate = *QTDdate4;
                param4 = PQString(tempDate.year());
                break;

            case ptQTDmonth:
                tempDate = *QTDdate4;
                param4 = PQString(tempDate.month());
                break;

            case ptQTDday:
                tempDate = *QTDdate4;
                param4 = PQString(tempDate.day());
                break;

            case ptQTDmask:
                tempDate = *QTDdate4;
                switch(maskType4)
                {
                case mtYYYYMM:
                    param4 = PQString(tempDate.toString("yyyyMM"));
                    break;

                case mtMMhYYYY:
                    param4 = PQString(tempDate.toString("MM-yyyy"));
                    break;

                case mtDDsMMsYYYY:
                    param4 = PQString(tempDate.toString("dd/MM/yyyy"));
                    break;

                case mtMMsDDsYYYY:
                    param4 = PQString(tempDate.toString("MM/dd/yyyy"));
                    break;

                case mtYYYYhMMhDD:
                    param4 = PQString(tempDate.toString("yyyy-MM-dd"));
                    break;

                default:
                    break;
                }
                break;

            case ptJulian:
                tempDate = QDate::fromJulianDay(*Julian4);
                switch(maskType4)
                {
                case mtYYYY:
                    param4 = PQString(tempDate.toString("yyyy"));
                    break;

                case mtMM:
                    param4 = PQString(tempDate.toString("MM"));
                    break;

                case mtDD:
                    param4 = PQString(tempDate.toString("dd"));
                    break;

                case mtM:
                    param4 = PQString(tempDate.toString("M"));
                    break;

                case mtD:
                    param4 = PQString(tempDate.toString("d"));
                    break;

                case mtMMMM:
                    param4 = PQString(tempDate.toString("MMMM"));
                    break;

                default:
                break;
                }

            default:
                break;
            }
        }

        if (numParams >= 5)
        {
            switch (param5Type)
            {
            case ptUint:
                param5 = PQString(*UIparam5);
                break;

            case ptInt:
                param5 = PQString(*IntParam5);
                break;

            case ptQS:
                param5 = PQString(*QSparam5);
                break;

            case ptPQS:
                param5 = *PQSparam5;
                break;

            case ptQTDyear:
                tempDate = *QTDdate5;
                param5 = PQString(tempDate.year());
                break;

            case ptQTDmonth:
                tempDate = *QTDdate5;
                param5 = PQString(tempDate.month());
                break;

            case ptQTDday:
                tempDate = *QTDdate5;
                param5 = PQString(tempDate.day());
                break;

            case ptQTDmask:
                tempDate = *QTDdate5;
                switch(maskType5)
                {
                case mtYYYYMM:
                    param5 = PQString(tempDate.toString("yyyyMM"));
                    break;

                case mtMMhYYYY:
                    param5 = PQString(tempDate.toString("MM-yyyy"));
                    break;

                case mtDDsMMsYYYY:
                    param5 = PQString(tempDate.toString("dd/MM/yyyy"));
                    break;

                case mtMMsDDsYYYY:
                    param5 = PQString(tempDate.toString("MM/dd/yyyy"));
                    break;

                case mtYYYYhMMhDD:
                    param5 = PQString(tempDate.toString("yyyy-MM-dd"));
                    break;

                default:
                    break;
                }
                break;

            case ptJulian:
                tempDate = QDate::fromJulianDay(*Julian5);
                switch(maskType5)
                {
                case mtYYYY:
                    param5 = PQString(tempDate.toString("yyyy"));
                    break;

                case mtMM:
                    param5 = PQString(tempDate.toString("MM"));
                    break;

                case mtDD:
                    param5 = PQString(tempDate.toString("dd"));
                    break;

                case mtM:
                    param5 = PQString(tempDate.toString("M"));
                    break;

                case mtD:
                    param5 = PQString(tempDate.toString("d"));
                    break;

                case mtMMMM:
                    param5 = PQString(tempDate.toString("MMMM"));
                    break;

                default:
                break;
                }

            default:
                break;
            }
        }

        if (numParams >= 6)
        {
            switch (param6Type)
            {
            case ptUint:
                param6 = PQString(*UIparam6);
                break;

            case ptInt:
                param6 = PQString(*IntParam6);
                break;

            case ptQS:
                param6 = PQString(*QSparam6);
                break;

            case ptPQS:
                param6 = *PQSparam6;
                break;

            case ptQTDyear:
                tempDate = *QTDdate6;
                param6 = PQString(tempDate.year());
                break;

            case ptQTDmonth:
                tempDate = *QTDdate6;
                param6 = PQString(tempDate.month());
                break;

            case ptQTDday:
                tempDate = *QTDdate6;
                param6 = PQString(tempDate.day());
                break;

            case ptQTDmask:
                tempDate = *QTDdate6;
                switch(maskType6)
                {
                case mtYYYYMM:
                    param6 = PQString(tempDate.toString("yyyyMM"));
                    break;

                case mtMMhYYYY:
                    param6 = PQString(tempDate.toString("MM-yyyy"));
                    break;

                case mtDDsMMsYYYY:
                    param6 = PQString(tempDate.toString("dd/MM/yyyy"));
                    break;

                case mtMMsDDsYYYY:
                    param6 = PQString(tempDate.toString("MM/dd/yyyy"));
                    break;

                case mtYYYYhMMhDD:
                    param6 = PQString(tempDate.toString("yyyy-MM-dd"));
                    break;

                default:
                    break;
                }
                break;

            case ptJulian:
                tempDate = QDate::fromJulianDay(*Julian6);
                switch(maskType6)
                {
                case mtYYYY:
                    param6 = PQString(tempDate.toString("yyyy"));
                    break;

                case mtMM:
                    param6 = PQString(tempDate.toString("MM"));
                    break;

                case mtDD:
                    param6 = PQString(tempDate.toString("dd"));
                    break;

                case mtM:
                    param6 = PQString(tempDate.toString("M"));
                    break;

                case mtD:
                    param6 = PQString(tempDate.toString("d"));
                    break;

                case mtMMMM:
                    param6 = PQString(tempDate.toString("MMMM"));
                    break;

                default:
                break;
                }

            default:
                break;
            }
        }
    }
};

struct PAYLOADPARAMS
{
    QByteArray param1;
    QByteArray param2;
    QByteArray param3;
    QByteArray param4;
    QByteArray param5;
    QByteArray param6;

    PARAMTYPE param1Type;
    PARAMTYPE param2Type;
    PARAMTYPE param3Type;
    PARAMTYPE param4Type;
    PARAMTYPE param5Type;
    PARAMTYPE param6Type;

    unsigned int *UIparam1;
    unsigned int *UIparam2;
    unsigned int *UIparam3;
    unsigned int *UIparam4;
    unsigned int *UIparam5;
    unsigned int *UIparam6;

    int *IntParam1;
    int *IntParam2;
    int *IntParam3;
    int *IntParam4;
    int *IntParam5;
    int *IntParam6;

    QString *QSparam1;
    QString *QSparam2;
    QString *QSparam3;
    QString *QSparam4;
    QString *QSparam5;
    QString *QSparam6;

    QByteArray *QBAparam1;
    QByteArray *QBAparam2;
    QByteArray *QBAparam3;
    QByteArray *QBAparam4;
    QByteArray *QBAparam5;
    QByteArray *QBAparam6;

    QDate *QTDdate1;
    QDate *QTDdate2;
    QDate *QTDdate3;
    QDate *QTDdate4;
    QDate *QTDdate5;
    QDate *QTDdate6;

    unsigned int *Julian1;
    unsigned int *Julian2;
    unsigned int *Julian3;
    unsigned int *Julian4;
    unsigned int *Julian5;
    unsigned int *Julian6;

    MASKTYPE maskType1;
    MASKTYPE maskType2;
    MASKTYPE maskType3;
    MASKTYPE maskType4;
    MASKTYPE maskType5;
    MASKTYPE maskType6;

    unsigned int numParams;

    PAYLOADPARAMS() : UIparam1(nullptr), UIparam2(nullptr), UIparam3(nullptr), UIparam4(nullptr), UIparam5(nullptr), UIparam6(nullptr),
                      IntParam1(nullptr), IntParam2(nullptr), IntParam3(nullptr), IntParam4(nullptr), IntParam5(nullptr), IntParam6(nullptr),
                      QSparam1(nullptr), QSparam2(nullptr), QSparam3(nullptr), QSparam4(nullptr), QSparam5(nullptr),  QSparam6(nullptr),
                      QBAparam1(nullptr), QBAparam2(nullptr), QBAparam3(nullptr), QBAparam4(nullptr), QBAparam5(nullptr), QBAparam6(nullptr),
                      QTDdate1(nullptr), QTDdate2(nullptr), QTDdate3(nullptr), QTDdate4(nullptr), QTDdate5(nullptr),  QTDdate6(nullptr) {}

    void clear(){
        param1.clear();
        param2.clear();
        param3.clear();
        param4.clear();
        param5.clear();
        param6.clear();

        param1Type = ptNotSet;
        param2Type = ptNotSet;
        param3Type = ptNotSet;
        param4Type = ptNotSet;
        param5Type = ptNotSet;
        param6Type = ptNotSet;
    }

    void updateParams(){

        QDate tempDate;

        if (numParams >= 1)
        {
            switch (param1Type)
            {
            case ptUint:
                param1 = QByteArray::number(*UIparam1);
                break;

            case ptInt:
                param1 = QByteArray::number(*IntParam1);
                break;

            case ptQS:
                param1 = (*QSparam1).toUtf8();
                break;

            case ptPQS:
                param1 = *QBAparam1;
                break;

            case ptQTDyear:
                tempDate = *QTDdate1;
                param1 = QByteArray::number(tempDate.year());
                break;

            case ptQTDmonth:
                tempDate = *QTDdate1;
                param1 = QByteArray::number(tempDate.month());
                break;

            case ptQTDday:
                tempDate = *QTDdate1;
                param1 = QByteArray::number(tempDate.day());
                break;

            case ptQTDmask:
                tempDate = *QTDdate1;
                switch(maskType1)
                {
                case mtYYYYMM:
                    param1 = (tempDate.toString("yyyyMM")).toUtf8();
                    break;

                case mtMMhYYYY:
                    param1 = (tempDate.toString("MM-yyyy")).toUtf8();
                    break;

                case mtDDsMMsYYYY:
                    param1 = (tempDate.toString("dd/MM/yyyy")).toUtf8();
                    break;

                case mtMMsDDsYYYY:
                    param1 = (tempDate.toString("MM/dd/yyyy")).toUtf8();
                    break;

                case mtYYYYhMMhDD:
                    param1 = (tempDate.toString("yyyy-MM-dd")).toUtf8();
                    break;

                default:
                    break;
                }
                break;

            case ptJulian:
                tempDate = QDate::fromJulianDay(*Julian1);
                switch(maskType1)
                {
                case mtYYYY:
                    param1 = (tempDate.toString("yyyy")).toUtf8();
                    break;

                case mtMM:
                    param1 = (tempDate.toString("MM")).toUtf8();
                    break;

                case mtDD:
                    param1 = (tempDate.toString("dd")).toUtf8();
                    break;

                case mtM:
                    param1 = (tempDate.toString("M")).toUtf8();
                    break;

                case mtD:
                    param1 = (tempDate.toString("d")).toUtf8();
                    break;

                case mtMMMM:
                    param1 = (tempDate.toString("MMMM")).toUtf8();
                    break;

                default:
                break;
                }

            default:
            break;
            }
        }

        if (numParams >= 2)
        {
            switch (param2Type)
            {
            case ptUint:
                param2 = QByteArray::number(*UIparam2);
                break;

            case ptInt:
                param2 = QByteArray::number(*IntParam2);
                break;

            case ptQS:
                param2 = (*QSparam2).toUtf8();
                break;

            case ptPQS:
                param2 = *QBAparam2;
                break;

            case ptQTDyear:
                tempDate = *QTDdate2;
                param2 = QByteArray::number(tempDate.year());
                break;

            case ptQTDmonth:
                tempDate = *QTDdate2;
                param2 = QByteArray::number(tempDate.month());
                break;

            case ptQTDday:
                tempDate = *QTDdate2;
                param2 = QByteArray::number(tempDate.day());
                break;

            case ptQTDmask:
                tempDate = *QTDdate2;
                switch(maskType2)
                {
                case mtYYYYMM:
                    param2 = (tempDate.toString("yyyyMM")).toUtf8();
                    break;

                case mtMMhYYYY:
                    param2 = (tempDate.toString("MM-yyyy")).toUtf8();
                    break;

                case mtDDsMMsYYYY:
                    param2 = (tempDate.toString("dd/MM/yyyy")).toUtf8();
                    break;

                case mtMMsDDsYYYY:
                    param2 = (tempDate.toString("MM/dd/yyyy")).toUtf8();
                    break;

                case mtYYYYhMMhDD:
                    param2 = (tempDate.toString("yyyy-MM-dd")).toUtf8();
                    break;

                default:
                    break;
                }
                break;

            case ptJulian:
                tempDate = QDate::fromJulianDay(*Julian2);
                switch(maskType2)
                {
                case mtYYYY:
                    param2 = (tempDate.toString("yyyy")).toUtf8();
                    break;

                case mtMM:
                    param2 = (tempDate.toString("MM")).toUtf8();
                    break;

                case mtDD:
                    param2 = (tempDate.toString("dd")).toUtf8();
                    break;

                case mtM:
                    param2 = (tempDate.toString("M")).toUtf8();
                    break;

                case mtD:
                    param2 = (tempDate.toString("d")).toUtf8();
                    break;

                case mtMMMM:
                    param2 = (tempDate.toString("MMMM")).toUtf8();
                    break;

                default:
                break;
                }

            default:
                break;
            }
        }

        if (numParams >= 3)
        {
            switch (param3Type)
            {
            case ptUint:
                param3 = QByteArray::number(*UIparam3);
                break;

            case ptInt:
                param3 = QByteArray::number(*IntParam3);
                break;

            case ptQS:
                param3 = (*QSparam3).toUtf8();
                break;

            case ptPQS:
                param3 = *QBAparam3;
                break;

            case ptQTDyear:
                tempDate = *QTDdate3;
                param3 = QByteArray::number(tempDate.year());
                break;

            case ptQTDmonth:
                tempDate = *QTDdate3;
                param3 = QByteArray::number(tempDate.month());
                break;

            case ptQTDday:
                tempDate = *QTDdate3;
                param3 = QByteArray::number(tempDate.day());
                break;

            case ptQTDmask:
                tempDate = *QTDdate3;
                switch(maskType3)
                {
                case mtYYYYMM:
                    param3 = (tempDate.toString("yyyyMM")).toUtf8();
                    break;

                case mtMMhYYYY:
                    param3 = (tempDate.toString("MM-yyyy")).toUtf8();
                    break;

                case mtDDsMMsYYYY:
                    param3 = (tempDate.toString("dd/MM/yyyy")).toUtf8();
                    break;

                case mtMMsDDsYYYY:
                    param3 = (tempDate.toString("MM/dd/yyyy")).toUtf8();
                    break;

                case mtYYYYhMMhDD:
                    param3 = (tempDate.toString("yyyy-MM-dd")).toUtf8();
                    break;

                default:
                    break;
                }
                break;

            case ptJulian:
                tempDate = QDate::fromJulianDay(*Julian3);
                switch(maskType3)
                {
                case mtYYYY:
                    param3 = (tempDate.toString("yyyy")).toUtf8();
                    break;

                case mtMM:
                    param3 = (tempDate.toString("MM")).toUtf8();
                    break;

                case mtDD:
                    param3 = (tempDate.toString("dd")).toUtf8();
                    break;

                case mtM:
                    param3 = (tempDate.toString("M")).toUtf8();
                    break;

                case mtD:
                    param3 = (tempDate.toString("d")).toUtf8();
                    break;

                case mtMMMM:
                    param3 = (tempDate.toString("MMMM")).toUtf8();
                    break;

                default:
                break;
                }

            default:
                break;
            }
        }

        if (numParams >= 4)
        {
            switch (param4Type)
            {
            case ptUint:
                param4 = QByteArray::number(*UIparam4);
                break;

            case ptInt:
                param4 = QByteArray::number(*IntParam4);
                break;

            case ptQS:
                param4 = (*QSparam4).toUtf8();
                break;

            case ptPQS:
                param4 = *QBAparam4;
                break;

            case ptQTDyear:
                tempDate = *QTDdate4;
                param4 = QByteArray::number(tempDate.year());
                break;

            case ptQTDmonth:
                tempDate = *QTDdate4;
                param4 = QByteArray::number(tempDate.month());
                break;

            case ptQTDday:
                tempDate = *QTDdate4;
                param4 = QByteArray::number(tempDate.day());
                break;

            case ptQTDmask:
                tempDate = *QTDdate4;
                switch(maskType4)
                {
                case mtYYYYMM:
                    param4 = (tempDate.toString("yyyyMM")).toUtf8();
                    break;

                case mtMMhYYYY:
                    param4 = (tempDate.toString("MM-yyyy")).toUtf8();
                    break;

                case mtDDsMMsYYYY:
                    param4 = (tempDate.toString("dd/MM/yyyy")).toUtf8();
                    break;

                case mtMMsDDsYYYY:
                    param4 = (tempDate.toString("MM/dd/yyyy")).toUtf8();
                    break;

                case mtYYYYhMMhDD:
                    param4 = (tempDate.toString("yyyy-MM-dd")).toUtf8();
                    break;

                default:
                    break;
                }
                break;

            case ptJulian:
                tempDate = QDate::fromJulianDay(*Julian4);
                switch(maskType4)
                {
                case mtYYYY:
                    param4 = (tempDate.toString("yyyy")).toUtf8();
                    break;

                case mtMM:
                    param4 = (tempDate.toString("MM")).toUtf8();
                    break;

                case mtDD:
                    param4 = (tempDate.toString("dd")).toUtf8();
                    break;

                case mtM:
                    param4 = (tempDate.toString("M")).toUtf8();
                    break;

                case mtD:
                    param4 = (tempDate.toString("d")).toUtf8();
                    break;

                case mtMMMM:
                    param4 = (tempDate.toString("MMMM")).toUtf8();
                    break;

                default:
                break;
                }

            default:
                break;
            }
        }

        if (numParams >= 5)
        {
            switch (param5Type)
            {
            case ptUint:
                param5 = QByteArray::number(*UIparam5);
                break;

            case ptInt:
                param5 = QByteArray::number(*IntParam5);
                break;

            case ptQS:
                param5 = (*QSparam5).toUtf8();
                break;

            case ptPQS:
                param5 = *QBAparam5;
                break;

            case ptQTDyear:
                tempDate = *QTDdate5;
                param5 = QByteArray::number(tempDate.year());
                break;

            case ptQTDmonth:
                tempDate = *QTDdate5;
                param5 = QByteArray::number(tempDate.month());
                break;

            case ptQTDday:
                tempDate = *QTDdate5;
                param5 = QByteArray::number(tempDate.day());
                break;

            case ptQTDmask:
                tempDate = *QTDdate5;
                switch(maskType5)
                {
                case mtYYYYMM:
                    param5 = (tempDate.toString("yyyyMM")).toUtf8();
                    break;

                case mtMMhYYYY:
                    param5 = (tempDate.toString("MM-yyyy")).toUtf8();
                    break;

                case mtDDsMMsYYYY:
                    param5 = (tempDate.toString("dd/MM/yyyy")).toUtf8();
                    break;

                case mtMMsDDsYYYY:
                    param5 = (tempDate.toString("MM/dd/yyyy")).toUtf8();
                    break;

                case mtYYYYhMMhDD:
                    param5 = (tempDate.toString("yyyy-MM-dd")).toUtf8();
                    break;

                default:
                    break;
                }
                break;

            case ptJulian:
                tempDate = QDate::fromJulianDay(*Julian5);
                switch(maskType5)
                {
                case mtYYYY:
                    param5 = (tempDate.toString("yyyy")).toUtf8();
                    break;

                case mtMM:
                    param5 = (tempDate.toString("MM")).toUtf8();
                    break;

                case mtDD:
                    param5 = (tempDate.toString("dd")).toUtf8();
                    break;

                case mtM:
                    param5 = (tempDate.toString("M")).toUtf8();
                    break;

                case mtD:
                    param5 = (tempDate.toString("d")).toUtf8();
                    break;

                case mtMMMM:
                    param5 = (tempDate.toString("MMMM")).toUtf8();
                    break;

                default:
                break;
                }

            default:
                break;
            }
        }

        if (numParams >= 6)
        {
            switch (param6Type)
            {
            case ptUint:
                param6 = QByteArray::number(*UIparam6);
                break;

            case ptInt:
                param6 = QByteArray::number(*IntParam6);
                break;

            case ptQS:
                param6 = (*QSparam6).toUtf8();
                break;

            case ptPQS:
                param6 = *QBAparam6;
                break;

            case ptQTDyear:
                tempDate = *QTDdate6;
                param6 = QByteArray::number(tempDate.year());
                break;

            case ptQTDmonth:
                tempDate = *QTDdate6;
                param6 = QByteArray::number(tempDate.month());
                break;

            case ptQTDday:
                tempDate = *QTDdate6;
                param6 = QByteArray::number(tempDate.day());
                break;

            case ptQTDmask:
                tempDate = *QTDdate6;
                switch(maskType6)
                {
                case mtYYYYMM:
                    param6 = (tempDate.toString("yyyyMM")).toUtf8();
                    break;

                case mtMMhYYYY:
                    param6 = (tempDate.toString("MM-yyyy")).toUtf8();
                    break;

                case mtDDsMMsYYYY:
                    param6 = (tempDate.toString("dd/MM/yyyy")).toUtf8();
                    break;

                case mtMMsDDsYYYY:
                    param6 = (tempDate.toString("MM/dd/yyyy")).toUtf8();
                    break;

                case mtYYYYhMMhDD:
                    param6 = (tempDate.toString("yyyy-MM-dd")).toUtf8();
                    break;

                default:
                    break;
                }
                break;

            case ptJulian:
                tempDate = QDate::fromJulianDay(*Julian6);
                switch(maskType6)
                {
                case mtYYYY:
                    param6 = (tempDate.toString("yyyy")).toUtf8();
                    break;

                case mtMM:
                    param6 = (tempDate.toString("MM")).toUtf8();
                    break;

                case mtDD:
                    param6 = (tempDate.toString("dd")).toUtf8();
                    break;

                case mtM:
                    param6 = (tempDate.toString("M")).toUtf8();
                    break;

                case mtD:
                    param6 = (tempDate.toString("d")).toUtf8();
                    break;

                case mtMMMM:
                    param6 = (tempDate.toString("MMMM")).toUtf8();
                    break;

                default:
                break;
                }

            default:
                break;
            }
        }
    }
};

enum FLOWTYPE {startToEnd, sequential, singleListing, dateRange, lastValue, alphabetical, monthly};

struct FLOWPARAMETERS
{
    FLOWTYPE flowType;
    unsigned int currentPosition;
    unsigned int endingPosition;
    unsigned int initialPosition;
    unsigned int indexOffset;
    unsigned int obitsPerPage;
    int flowIncrement;
    QString lastValueParam;
    QString currentLetter;
    unsigned int letterCurrent;
    unsigned int letterEnd;
    QString currentMonth;
    unsigned int monthCurrent;
    unsigned int monthEnd;

    int numBlankDownloads;
    int numBlankDownloadLimit;
    int numRecordsRead;

    bool keepDownloading;
    bool initialSetup;
};

struct UDrequestParam
{
    unsigned int providerID;
    unsigned int providerKey;
    QString fhHTTP;
    QString fhWWW;
    QString fhURLforUpdate;
    QString fhParam1;
    QString fhParam2;
    QString fhURLid;
    QString fhURLidDivider;
    unsigned int fhSpecialCode;
    QDate fhFirstObit;
    QDate fhLastRun;
    QString fhSequentialID;
    QString fhAlphabeticalListing;
    QString fhFollowRedirects;

    unsigned int originalOrder;
    quint32 randomOrder;

    void clear()
    {
        providerID = 0;
        providerKey = 0;
        fhHTTP.clear();
        fhWWW.clear();
        fhURLforUpdate.clear();
        fhParam1.clear();
        fhParam2.clear();
        fhURLid.clear();
        fhURLidDivider.clear();
        fhSpecialCode = 0;
        fhFirstObit = QDate();
        fhLastRun = QDate();
        fhSequentialID.clear();
        fhAlphabeticalListing.clear();
        fhFollowRedirects.clear();

        originalOrder = 0;
        randomOrder = 0;
    }
};


#endif
