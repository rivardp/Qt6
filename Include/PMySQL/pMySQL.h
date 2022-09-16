#ifndef PMYSQL_H
#define PMYSQL_H

#include <QString>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMapIterator>
#include <QDebug>
#include "../UpdateFuneralHomes/Include/globalVars.h"

bool createConnection(QSqlDatabase *db);
bool createConnection(QSqlDatabase *db, QString user);
bool createConnection(QSqlDatabase *db, QString user, QString pswd);
QString getLastExecutedQuery(const QSqlQuery& query);

bool createTestConnection(QSqlDatabase *db, GLOBALVARS &globals);
bool createDemoConnection(QSqlDatabase *db);

#endif // PMYSQL_H

