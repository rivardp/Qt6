// mostCommonName.h
//

#ifndef MOST_COMMON_NAME_H
#define MOST_COMMON_NAME_H

#include <QString>
#include <QList>

#include "../UpdateFuneralHomes/Include/globalVars.h"
#include "../UpdateFuneralHomes/Include/dataStructure.h"
#include "../UpdateFuneralHomes/Include/dataRecord.h"
#include "../Include/PMySQL/databaseSearches.h"

class mostCommonName
{

public:
    mostCommonName();
    ~mostCommonName();

    QList<QString> listOfFirstWords;
    unsigned int nameFreq;

    NAMEINFO readMostCommonName(GLOBALVARS *gv);
    QString readPotentialFirstName(GLOBALVARS *gv);
    GENDER startsWithGenderWord(GLOBALVARS *gv);

    bool setupCompleted() const;
    void clear();


private:
    QList<unsigned int> frequencyCounts;

    void cleanUpList();
    void sortOnWords();
    void sortOnFrequency();
    void consolidateWords();

    bool SetupCompleted;
};

#endif
