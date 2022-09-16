#include <QCoreApplication>
#include <QApplication>
#include <QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QTextCodec>
#include <QDebug>

#include "../UpdateFuneralHomes/Include/globalVars.h"
#include "../Include/PMySQL/pMySQL.h"
#include "Include/readNicknames.h"



int main(int argc, char *argv[])
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

    /*******************************************/
    /*           Set Locale to UTF-8           */
    /*******************************************/

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    /*******************************************/
    /*           Run initial setup             */
    /*******************************************/

    GLOBALVARS globals;

    // Setup the MySQL connection with database manager 'db'
    if (!createConnection(globals.db, QString("deceasedUpdater")))
    {
        qDebug() << "Error connecting to SQL database 'death_audits'";
        globals.logMsg(ErrorConnection, QString("Error connecting to SQL database death_audits"));
        return 0;
    }
    else
    {
        qDebug() << "Connected to 'death_audits'...";
    }

    // Finish setup
    if (!globals.runSetups())
    {
        qDebug() << "Error in setup process";
        globals.logMsg(ErrorRunTime, QString("Error in setup process"));
        return 0;
    }
    else
    {
        qDebug() << "Setup completed successfully...";
    }

    /*******************************************/
    /*           Open and read file            */
    /*******************************************/

    QString filename("C:\\Obits\\Reports\\NickNamesUpdater.csv");
    QFile CSVfile(filename);

    QString formalName, nickName;
    if(CSVfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!CSVfile.atEnd())
        {
            readLine(CSVfile, formalName, nickName);
            updateFirstNameDB(formalName, nickName, &globals);
            updateNickNameDB(formalName, nickName, &globals);
        }
    }
    else
    {
        PQString errorMessage;
        globals.logMsg(ErrorRunTime, errorMessage << "Could not open file: " << filename);
    }

    return 0;
}
