#include "CheckMonitored.h"
#include "globals.h"

void createListOfDeceasedIDs(QString filename, QDate cutOffDate, GLOBALVARS gv)
{
    QSqlQuery query;
    QSqlError error;

    PQString errorMessage, cutOffDateSQL;
    QString comma(",");
    bool success;

    QFile *CSVfile = new QFile(filename);
    if(!CSVfile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        return;

    QTextStream outputStream(CSVfile);

    cutOffDateSQL << cutOffDate.toString("yyyy/MM/dd") << QString(" 0:0:0");

    success = query.prepare("SELECT deceasedNumber, lastName FROM deceased WHERE lastUpdated >= :lastUpdated");
    if (!success)
        qDebug() << "Error with SQL statement in CheckMonitored - createListOfDeceasedIDs";
    query.bindValue(":lastUpdated", QVariant(cutOffDateSQL.getString()));
    success = query.exec();

    if (!success)
    {
        error = query.lastError();

        errorMessage << QString("SQL problem trying to create list of recent deceasedIDs to be checked");
        gv.logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));

        return;
    }
    else
    {
        while (query.next())
        {
            outputStream << query.value(0).toInt() << comma;
            outputStream << query.value(1).toString() << comma;
            outputStream << Qt::endl;
        }
    }

    CSVfile->close();

    return;
}


void readLine(QFile &filename, dataRecord &dr)
{
    // CSV format   ID, Lastname

    PQString word;
    PQString name;
    QString comma(",");
    char buffer[1024];

    qint64 lineLength = filename.readLine(buffer, sizeof(buffer));
    if (lineLength != -1)
    {
        // The line is available in buf
        PQStream line(buffer);

        // Clear anything left in the prior dr record
        dr.clear();

        word = line.getUntil(comma);
        if (word.getLength() > 0)
            dr.setDeceasedNumber(static_cast<unsigned int>(word.asNumber()));

        name = line.getUntil(comma);
        if (name.getLength() > 0)
           dr.setFamilyName(name);
    }
}

