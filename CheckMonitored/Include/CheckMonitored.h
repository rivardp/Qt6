// CheckMonitored.h

#ifndef CHECKMONITORED_H
#define CHECKMONITORED_H

#include <QFile>
#include <QSqlQuery>
#include <QSqlError>

#include "../Include/Penser/PQString.h"
#include "../Include/Penser/PQStream.h"
#include "../Include/PMySQL/databaseSearches.h"
#include "../UpdateFuneralHomes/Include/dataStructure.h"
#include "../UpdateFuneralHomes/Include/globalVars.h"
#include "../UpdateFuneralHomes/Include/matchRecord.h"
#include "../Include/Penser/QtUtilities.h"

void createListOfDeceasedIDs(QString filename, QDate cutOffDate, GLOBALVARS gv);
void readLine(QFile &filename, dataRecord &dr);


#endif
