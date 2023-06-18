// enterprise.cpp

#include "enterprise.h"
#include "qdebug.h"

unsigned int clientCode = 0;
QDir installDirectory = QDir();
QDir CSVfileDirectory = QDir();

unsigned int getClientCode()
{    
    QString fileName = installDirectory.path() + QString("/PayAlert.config");
    QFile configFile(fileName);

    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        unsigned int parameter;
        getParameter(configFile, parameter, QString("Client Code"));
        clientCode = parameter;
    }

    return clientCode;
}

bool getParameter(QFile &file, unsigned int &parameter, QString target)
{
    QString resultRead;
    if (readParameter(file, resultRead, target))
    {
        parameter = resultRead.toUInt();
        return true;
    }
    else
        return false;
}

bool readParameter(QFile &file, QString &parameter, QString target)
{
    bool found = false;
    QTextStream in(&file);
    QString completeTarget = QString("[") + target + QString("]: ");

    while (!found && !in.atEnd()) {
        QString line = in.readLine();
        found = (line.left(completeTarget.length()) == completeTarget);
        if (found)
            parameter = line.remove(completeTarget);
    }

    return found;
}

QList<GROUPCONFIG> getGroupConfigs(unsigned int clientCode)
{
    QSqlQuery query;
    QSqlError error;
    bool success;

    QList<GROUPCONFIG> result;
    GROUPCONFIG record;

    success = query.prepare("SELECT clientCode, groupName, lastRun, freqOption, weeklyDay, monthlyDay, customDay, reportLevel, emails "
                            "FROM client_info.groups WHERE clientCode = :clientCode");
    if (!success)
        qDebug() << "Problem with SQL statement formulation";
    query.bindValue(":clientCode", QVariant(clientCode));

    success = query.exec();

    if (success)
    {
        while(query.next())
        {
            record.clientCode = query.value(0).toUInt();
            record.groupName = query.value(1).toString();
            record.lastRun = query.value(2).toDateTime();
            record.groupSchedule.freqOptions = static_cast<FREQOPTIONS>(query.value(3).toInt());
            record.groupSchedule.weeklyDay = static_cast<DAYSOFWEEK>(query.value(4).toInt());
            record.groupSchedule.monthDayChosen = query.value(5).toInt();
            record.groupSchedule.customDaysChosen = query.value(6).toInt();
            record.groupMatchesIncluded = query.value(7).toInt();
            record.emailRecipients = query.value(8).toString();
            record.key = QString::number(record.clientCode) + record.groupName;

            result.append(record);
        }
    }

    if (result.size() == 0)
        result.append(record.createNewGroupConfig(clientCode));

    return result;
}

QList<USERINFO> getAllUsers(unsigned int clientCode)
{
    QSqlQuery query;
    QSqlError error;
    bool success;

    QList<USERINFO> result;
    USERINFO record;

    success = query.prepare("SELECT clientCode, email, administratorRights, name, password "
                            "FROM client_info.users WHERE clientCode = :clientCode");
    if (!success)
        qDebug() << "Problem with SQL statement formulation";
    query.bindValue(":clientCode", QVariant(clientCode));

    success = query.exec();

    if (success)
    {
        while(query.next())
        {
            record.clientCode = query.value(0).toUInt();
            record.emailAddress = query.value(1).toString();
            record.adminRights = query.value(2).toString();
            record.name = query.value(3).toString();
            record.password = query.value(4).toString();
            record.key = QString::number(record.clientCode) + record.emailAddress;

            result.append(record);
        }
    }

    return result;
}

void saveGroupConfigs(const QList<GROUPCONFIG> &gc)
{
    QSqlQuery query;
    QSqlError error;
    bool success;

    QList<GROUPCONFIG> result;
    GROUPCONFIG record;

    success = query.prepare("SELECT clientCode, groupName, lastRun, freqOption, weeklyDay, monthlyDay, customDay, reportLevel, emails "
                            "FROM client_info.groups WHERE clientCode = :clientCode");
    if (!success)
        qDebug() << "Problem with SQL statement formulation";

    // Delete existing
    // Replace with new


    query.bindValue(":clientCode", QVariant(clientCode));

    success = query.exec();

    if (success)
    {
        while(query.next())
        {
            record.clientCode = query.value(0).toUInt();
            record.groupName = query.value(1).toString();
            record.lastRun = query.value(2).toDateTime();
            record.groupSchedule.freqOptions = static_cast<FREQOPTIONS>(query.value(3).toInt());
            record.groupSchedule.weeklyDay = static_cast<DAYSOFWEEK>(query.value(4).toInt());
            record.groupSchedule.monthDayChosen = query.value(5).toInt();
            record.groupSchedule.customDaysChosen = query.value(6).toInt();
            record.groupMatchesIncluded = query.value(7).toInt();
            record.emailRecipients = query.value(8).toString();
            record.key = QString::number(record.clientCode) + record.groupName;

            result.append(record);
        }
    }
}


