// enterprise.h

#ifndef ENTERPRISE_H
#define ENTERPRISE_H

#include <QDate>
#include <QSqlQuery>
#include <QSqlError>

#include "../PayAlertEnterprise/groupConfig.h"


QList<GROUPCONFIG> getGroupConfigs(unsigned int clientCode);
QList<USERINFO> getAllUsers(unsigned int clientCode);

#endif


