#include "../../Projects/Include/PMySQL/pMySQL.h"

bool createConnection(QSqlDatabase *db)
{
    db = &QSqlDatabase::addDatabase("QMYSQL");
    db->setHostName("localhost");
    db->setDatabaseName("death_audits");
    db->setUserName("FHuser");
    db->setPassword("FHuser");
    if (!db->open())
    {
        qDebug() << "Database error occurred";
        qDebug() << db->lastError();
        return false;
    }
    return true;
}

bool createConnection(QSqlDatabase *db, QString user)
{
    db = &QSqlDatabase::addDatabase("QMYSQL");
    db->setHostName("localhost");
    db->setDatabaseName("death_audits");
    db->setUserName(user);
    db->setPassword(user);
    if (!db->open())
    {
        qDebug() << "Database error occurred";
        qDebug() << db->lastError();
        return false;
    }
    return true;
}

bool createConnection(QSqlDatabase *db, QString user, QString pswd)
{
    db = &QSqlDatabase::addDatabase("QMYSQL");
    db->setHostName("localhost");
    db->setDatabaseName("death_audits");
    db->setUserName(user);
    db->setPassword(pswd);
    if (!db->open())
    {
        qDebug() << "Database error occurred";
        qDebug() << db->lastError();
        return false;
    }
    return true;
}

QString getLastExecutedQuery(const QSqlQuery& query)
{
    QString str = query.lastQuery();
    QMapIterator<QString, QVariant> it(query.boundValues());
    while (it.hasNext())
    {
        it.next();
        str.replace(it.key(),it.value().toString());
    }
    return str;
}

bool createTestConnection(QSqlDatabase *db, GLOBALVARS &globals)
{
    QStringList tableList;
    QString table;
    QString computerIdentity = globals.baseDirectory.left(13).getString();

    db = &QSqlDatabase::addDatabase("QMYSQL");
    db->setHostName("localhost");

    if (computerIdentity == QString("C:/Users/Phil"))
    {
        table = QString("death_audits");
        db->setUserName("ProgramExe");
        db->setPassword("TestPswd");
    }
    else
    {
        //table = QString("pmr_test_database");
        table = QString("death_audits");
        db->setUserName("pprivard");
        db->setPassword("b4a5!F894d8");
    }

    db->setDatabaseName(table);

    if (db->open())
    {
        qDebug() << "Database" << table << " opened...";
        globals.logMsg(RunSummary, QString("Database ") + table + QString(" opened..."));
    }
    else
    {
        qDebug() << "Database error occurred";
        qDebug() << db->lastError();
        globals.logMsg(RunSummary, QString("Error opening database ") + table + QString(" - Abort Run"));
        globals.logMsg(RunSummary, QString("Error: ") + db->lastError().text());
        return false;
    }
    return true;
}

bool createDemoConnection(QSqlDatabase *db)
{
    db = &QSqlDatabase::addDatabase("QMYSQL");
    db->setHostName("localhost");
    db->setDatabaseName("death_audits");
    db->setUserName("ProgramExe");
    db->setPassword("TestPswd");
    if (db->open())
    {
        qDebug() << "Database death_audits opened...";
    }
    else
    {
        qDebug() << "Database error occurred";
        qDebug() << db->lastError();
        return false;
    }
    return true;
}
