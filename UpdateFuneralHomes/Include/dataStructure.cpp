// dataStructure.cpp

#include "dataStructure.h"

CONTENTREAD::CONTENTREAD()
{

}

CONTENTREAD::~CONTENTREAD()
{

}

OQString CONTENTREAD::select(const PQString &targetWord)
{
    if (firstBlock.getString().contains(targetWord.getString(), Qt::CaseInsensitive))
        return firstBlock;

    if (secondBlock.getString().contains(targetWord.getString(), Qt::CaseInsensitive))
        return secondBlock;

    if (thirdBlock.getString().contains(targetWord.getString(), Qt::CaseInsensitive))
        return thirdBlock;

    return allContent;
}

void CONTENTREAD::cleanUpEnds()
{
    allContent.cleanUpEnds();
    firstBlock.cleanUpEnds();
    secondBlock.cleanUpEnds();
    thirdBlock.cleanUpEnds();
}

void CONTENTREAD::clear()
{
    allContent.clear();
    firstBlock.clear();
    secondBlock.clear();
    thirdBlock.clear();
}

void NAMESTATS::determineCredibility()
{
    unsigned int totalCount = maleCount + femaleCount;
    bool unanimous = (totalCount > 0) && ((maleCount == 0) || (femaleCount == 0));

    if (totalCount == 0)
        credibility = zero;
    else
    {
        if ((totalCount <= 10) && !unanimous)
            credibility = low;
        else
        {
            if ((totalCount <= 50) && !(unanimous && (totalCount >= 5)))
                credibility = medium;
            else
            {
                if ((totalCount <= 250) && !(unanimous && (totalCount >= 100)))
                    credibility = high;
                else
                    credibility = veryHigh;
            }
        }
    }
}

WARNINGINFO::WARNINGINFO()
{
}

WARNINGINFO::~WARNINGINFO()
{
}

void WARNINGINFO::clear()
{
    dateAgeError = 0;
    ageFlag = 0;
    dateFlag = 0;
    genderFlag = 0;
    nameReversalFlag = 0;
    nameFlagGeneral = 0;
    doubleMemorialFlag = 0;
    bilingualFlag = 0;
    futureUse = 0;
    outstandingErrMsg = 0;
    memorialFlag = 0;
    validated = 0;

    nameWarningException = false;

    spouseFirtName.clear();
    checkParentsName.clear();
    checkInclName.clear();
    checkExclName.clear();
    confirmTreatmentName.clear();
    confirmMiddleName.clear();
}

void WARNINGINFO::resetNonDateValidations()
{
    genderFlag = 0;
    //nameReversalFlag = 0;
    nameFlagGeneral = 0;
    //doubleMemorialFlag = 0;
    bilingualFlag = 0;
    futureUse = 0;
    outstandingErrMsg = 0;
    memorialFlag = 0;
    validated = 0;

    nameWarningException = false;

    spouseFirtName.clear();
    checkParentsName.clear();
    checkInclName.clear();
    checkExclName.clear();
    //confirmTreatmentName.clear();
    confirmMiddleName.clear();
}

FUNERALHOME::FUNERALHOME()
{
}

FUNERALHOME::~FUNERALHOME()
{
}

void FUNERALHOME::clear()
{
    listerKey = 0;
    listerID = 0;
    providerID = 0;
    providerKey = 0;

    fhName.clear();
    fhCity.clear();
    fhProvince.clear();
    fhPostalCode.clear();
    fhURL.clear();

    fhRunStatus = 0;
    fhURLparam1.clear();
    fhURLparam1.clear();
    fhHTTP.clear();
    fhWWW.clear();
}
