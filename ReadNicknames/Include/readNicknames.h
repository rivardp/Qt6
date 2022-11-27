// readNicknames.h

#ifndef READNICKNAMES_H
#define READNICKNAMES_H

#include <QFile>
#include <QSqlQuery>
#include <QSqlError>

#include "../Include/PMySQL/databaseSearches.h"
#include "../Include/Penser/PQStream.h"
#include "../UpdateFuneralHomes/Include/globalVars.h"
#include "../PostalCodes/postalCodeInfo.h"
//#include "../PostalCodes/postalcodes.h"

void readLine(QFile &filename, QString &formalname, QString &nickname);
void updateFirstNameDB(const QString &formalname, const QString &nickname, GLOBALVARS *globals);
void updateNickNameDB(const QString &formalname, const QString &nickname, GLOBALVARS *globals);


#endif
