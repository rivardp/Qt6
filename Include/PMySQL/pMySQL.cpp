#include "../../Projects/Include/PMySQL/pMySQL.h"

bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("death_audits");
    db.setUserName("FHuser");
    db.setPassword("FHuser");
    if (!db.open())
    {
        qDebug() << "Database error occurred";
        qDebug() << db.lastError();
        return false;
    }
    return true;
}

bool createConnection(QString user)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("death_audits");
    db.setUserName(user);
    if (user == "deceasedUpdater")
        db.setPassword("c'9;>:)Kq`=U{T}n");
    else
    {
        if (user == "deceasedLegacyUpdater")
            db.setPassword("zGZc.Bf/U9aT2@C%");
        else
        {
            if (user == "BatchMySQL")
                db.setPassword("zx7<urVS'K+m(q[5");
            else
            {
                if (user == "Phil")
                    db.setPassword("c_XW}bfPCBV53<es");
                else
                    db.setPassword(user);
            }
        }
    }

    if (!db.open())
    {
        qDebug() << "Database error occurred";
        qDebug() << db.lastError();
        return false;
    }
    return true;   
}

bool createConnection(QString user, QString pswd)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("death_audits");
    db.setUserName(user);
    db.setPassword(pswd);
    if (!db.open())
    {
        qDebug() << "Database error occurred";
        qDebug() << db.lastError();
        return false;
    }
    return true;
}

QString getLastExecutedQuery(const QSqlQuery& query)
{
    QString str = query.lastQuery();

    /*QVariantList vList = query.boundValues();
    QList<QString> sList;
    for (int i = 0; i < vList.size(); ++i){
        sList.append(vList.at(i).toString().toUtf8().data());}*/

    return str;
}

bool createTestConnection(QSqlDatabase &db, GLOBALVARS &globals)
{
    QString table;
    QString computerIdentity = globals.baseDirectory.left(13).getString();

    db.addDatabase("QMYSQL");
    db.setHostName("localhost");

    if (computerIdentity == QString("C:/Users/Phil"))
    {
        table = QString("death_audits");
        db.setUserName("ProgramExe");
        db.setPassword("TestPswd");
    }
    else
    {
        //table = QString("pmr_test_database");
        table = QString("death_audits");
        db.setUserName("pprivard");
        db.setPassword("b4a5!F894d8");
    }

    db.setDatabaseName(table);

    if (db.open())
    {
        qDebug() << "Database" << table << " opened...";
        globals.logMsg(RunSummary, QString("Database ") + table + QString(" opened..."));
    }
    else
    {
        qDebug() << "Database error occurred";
        qDebug() << db.lastError();
        globals.logMsg(RunSummary, QString("Error opening database ") + table + QString(" - Abort Run"));
        globals.logMsg(RunSummary, QString("Error: ") + db.lastError().text());
        return false;
    }
    return true;
}

bool createDemoConnection(QSqlDatabase &db)
{
    db.addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("death_audits");
    db.setUserName("ProgramExe");
    db.setPassword("TestPswd");
    if (db.open())
    {
        qDebug() << "Database death_audits opened...";
    }
    else
    {
        qDebug() << "Database error occurred";
        qDebug() << db.lastError();
        return false;
    }
    return true;
}
