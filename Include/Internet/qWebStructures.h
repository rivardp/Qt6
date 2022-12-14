#ifndef QWEBSTRUCTURES_H
#define QWEBSTRUCTURES_H

#include <QString>
#include <QFile>
#include <QUrl>

#include "../UpdateFuneralHomes/Include/globalVars.h"

struct downloadOutputs
{
    QString downloadFileName;

    QString HTMLfileName;
    QString PDFfileName;
    QString ID;
    unsigned int providerID;
    unsigned int providerKey;
    QString dob;
    QString dod;
    QString yob;
    QString yod;
    unsigned int ageAtDeath;
    QString maidenNames;
    QString pubDate;
    QString pcKey;
    QFile* queue;
    bool addToQueue;
    bool fileAlreadyExists;
    bool forceOverrides;

    QString obitListEntry;

    GLOBALVARS *globals;

    void clear()
    {
        downloadFileName.clear();
        HTMLfileName.clear();
        PDFfileName.clear();
        ID.clear();
        providerID = 0;
        providerKey = 0;;
        dob.clear();
        dod.clear();
        yob.clear();
        yod.clear();
        ageAtDeath = 0;
        maidenNames.clear();
        pubDate.clear();
        pcKey.clear();
        addToQueue = false;
        fileAlreadyExists = false;
        forceOverrides = true;
        globals = nullptr;

        obitListEntry.clear();
    }
};

struct downloadInstructions
{
    unsigned int originalOrder;
    quint32 randomOrder;

    PQString url;
    QUrl qUrl;

    bool followRedirects;
    QString POSTformRequest;

    QString verb;
    QString payload;

    // Headers
    QString ContentTypeHeader;

    // RawHeaders
    QString User_Agent;
    QString Accept;
    QString Accept_Encoding;
    QString Accept_Language;
    QString Access_Control_Allow_Origin;
    QString Access_Control_Allow_Methods;
    QString Access_Control_Request_Method;
    QString Access_Control_Request_Headers;
    QString DomainID;
    QString Origin;
    QString Referer;
    QString Instance;
    QString Authorization;
    QString Sec_Fetch_Mode;
    QString Sec_Fetch_Site;
    QString Sec_Fetch_Dest;
    QString X_Requested_With;
    QString X_Origin;

    // Attributes
    QString RedirectPolicyAttribute;

    // Configurations
    QString SSL;

    // Site specific - needed for rare tweaking
    unsigned int providerID;
    unsigned int providerKey;

    void clear()
    {
        originalOrder = 0;
        randomOrder = 0;

        url.clear();
        qUrl.clear();

        followRedirects = false;
        POSTformRequest.clear();

        verb.clear();
        payload.clear();
        ContentTypeHeader.clear();
        User_Agent.clear();
        Accept.clear();
        Accept_Encoding.clear();
        Accept_Language.clear();
        Access_Control_Allow_Origin.clear();
        Access_Control_Allow_Methods.clear();
        Access_Control_Request_Method.clear();
        Access_Control_Request_Headers.clear();
        DomainID.clear();
        Origin.clear();
        Referer.clear();
        Instance.clear();
        Authorization.clear();
        Sec_Fetch_Mode.clear();
        Sec_Fetch_Site.clear();
        Sec_Fetch_Dest.clear();
        X_Requested_With.clear();
        X_Origin.clear();
        RedirectPolicyAttribute.clear();
        SSL.clear();

        providerID = 0;
        providerKey = 0;
    }
};

bool sortIntoRandomOrder(const downloadInstructions &di1, const downloadInstructions &di2);
bool sortIntoOriginalOrder(const downloadInstructions &di1, const downloadInstructions &di2);

struct StringRECORD
{
    OQString providerID;
    OQString providerKey;
    OQString url;
    OQString POSTformRequest;
    OQString dob;
    OQString dod;
    OQString yob;
    OQString yod;
    OQString maidenNames;
    OQString ID;
    OQString pubDate;
    OQString ageAtDeath;
    OQString pcKey;

    void clear(){
        providerID.clear();
        providerKey.clear();
        url.clear();
        POSTformRequest.clear();
        dob.clear();
        dod.clear();
        yob.clear();
        yod.clear();
        maidenNames.clear();
        ID.clear();
        pubDate.clear();
        ageAtDeath.clear();
        pcKey.clear();
    }

};

struct DOWNLOADREQUEST
{
    downloadOutputs outputs;
    downloadInstructions instructions;

    void clear()
    {
        outputs.clear();
        instructions.clear();
    }
};


#endif // QWEBSTRUCTURES_H

