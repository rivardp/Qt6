// enterprise.h

#ifndef ENTERPRISE_H
#define ENTERPRISE_H

#include <QDate>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QDir>

#include "../PayAlertEnterprise/groupConfig.h"

unsigned int getClientCode();
QList<GROUPCONFIG> getGroupConfigs(unsigned int clientCode);
void saveGroupConfigs(const QList<GROUPCONFIG> &gc);
QList<USERINFO> getAllUsers(unsigned int clientCode);

bool readParameter(QFile &file, QString &parameter, QString target);
bool getParameter(QFile &file, unsigned int &parameter, QString target);


extern unsigned int clientCode;
extern QDir installDirectory;
extern QDir CSVfileDirectory;

#endif


