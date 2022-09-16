// readCSV.h

#ifndef READCSV_H
#define READCSV_H

#include <QFile>
#include <QSqlQuery>
#include <QSqlError>

#include "../Include/Penser/PQString.h"
#include "../Include/Penser/PQStream.h"
#include "../Include/PMySQL/databaseSearches.h"
#include "../UpdateFuneralHomes/Include/dataRecord.h"
#include "../UpdateFuneralHomes/Include/dataStructure.h"
#include "../UpdateFuneralHomes/Include/globalVars.h"
#include "../UpdateFuneralHomes/Include/matchRecord.h"

void readLine(QFile &filename, dataRecord &dr, GLOBALVARS gv);
bool loadRecord(const dataRecord &dr, MATCHKEY &matchKeyResult, bool &newRec);
bool loadSourceID(const SOURCEID &sid, const GLOBALVARS globals);
bool updateRecord(const dataRecord &dr, const double weight, const PQString updateDate);
void updateFirstNameDB(const dataRecord &dr);
void updateDrToBestOf(dataRecord &dr, PQString &middlenames, PQString &nameAKA1, PQString &nameAKA2, GENDER &gender);
bool validate(QString &name);

#endif
