#include "readNicknames.h"

void readLine(QFile &filename, QString &formalname, QString &nickname)
{
    // CSV format   Formal name, Nickname

    QString comma(",");
    char buffer[1024];

    qint64 lineLength = filename.readLine(buffer, sizeof(buffer));
    if (lineLength != -1)
    {
        // The line is available in buf
        PQStream line(buffer);

        // Clear anything left in the prior variable names
        formalname.clear();
        nickname.clear();

        formalname = line.getUntil(comma).getString();
        nickname = line.getUntil(comma).getString();
    }
}

void updateFirstNameDB(const QString &formalname, const QString &nickname, GLOBALVARS *globals)
{
    // Don't do anything if information incomplete
    if ((formalname.length() == 0) || (nickname.length() == 0))
        return;

    QSqlQuery query;
    QSqlError error;

    PQString errorMessage;
    bool success;

    // Load nickname where appropriate
    query.prepare("SELECT altNames FROM firstnames WHERE name = :name");
    query.bindValue(":name", QVariant(formalname));
    success = query.exec();
    query.next();

    if (!success || (query.size() > 1))
    {
        error = query.lastError();

        errorMessage << QString("SQL error trying to determine if firstnames already contains: ") << formalname;
        globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }
    else
    {
        if (query.size() == 0)  // new name to be loaded
        {
            unsigned int maleCount = 0;
            unsigned int femaleCount = 0;
            double malePct = 0;

            query.prepare("INSERT INTO firstnames(name, maleCount, femaleCount, malePct, altNames) VALUES (:name, :maleCount, :femaleCount, :malePct, :altNames)");
            query.bindValue(":name", QVariant(formalname));
            query.bindValue(":maleCount", QVariant(maleCount));
            query.bindValue(":femaleCount", QVariant(femaleCount));
            query.bindValue(":malePct", QVariant(malePct));
            query.bindValue(":altNames", QVariant(nickname));
            success = query.exec();
            query.next();

            if (!success)
            {
                error = query.lastError();

                errorMessage << QString("SQL error trying to add new nickname: ") << nickname;
                globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
            }

            query.clear();

        }
        else    // name exists
        {
            bool nameListUpdated = false;
            bool handled = false;
            int i = 0;
            int compareResult;

            // Pull current nicknames
            QString currentNames = query.value(0).toString();
            QList<QString> listOfNames = currentNames.split(QString(" "), QString::SkipEmptyParts, Qt::CaseInsensitive);
            if ((listOfNames.size() == 1) && (listOfNames.at(0).length() == 0))
                listOfNames.clear();
            int numNames = listOfNames.size();

            if (numNames == 0)
            {
                listOfNames.append(nickname);
                handled = true;
                nameListUpdated = true;
            }

            while (!handled && (i < numNames))
            {
                compareResult = nickname.compare(listOfNames.at(i), Qt::CaseInsensitive);
                if (compareResult == 0)
                    handled = true;
                else
                {
                    if (compareResult < 0)
                    {
                        listOfNames.insert(i, nickname);
                        handled = true;
                        nameListUpdated = true;
                    }
                    else
                    {
                        // compareResult >= 1
                        i++;
                        if (i == numNames)
                        {
                            // No more possible matches
                            listOfNames.append(nickname);
                            handled = true;
                            nameListUpdated = true;
                        }
                    }
                }
            }

            if (nameListUpdated)
            {
                QString nicknames;
                numNames = listOfNames.size();

                for (i = 0 ; i < numNames; i++)
                {
                    nicknames += listOfNames.at(i);
                    if (i < (numNames - 1))
                        nicknames += QString(" ");
                }

                query.prepare("UPDATE firstnames SET altNames = :nicknames WHERE name = :name");
                query.bindValue(":nicknames", QVariant(nicknames));
                query.bindValue(":name", QVariant(formalname));
                success = query.exec();
                query.next();

                if (!success)
                {
                    error = query.lastError();

                    errorMessage << QString("SQL error trying to add nickname: ") << nickname << QString(" to first name: ") << formalname;
                    globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
                }
            }
        }
    }
}

void updateNickNameDB(const QString &formalname, const QString &nickname, GLOBALVARS *globals)
{
    // Don't do anything if information incomplete
    if ((formalname.length() == 0) || (nickname.length() == 0))
        return;

    QSqlQuery query;
    QSqlError error;

    PQString errorMessage;
    bool success;

    // Load nickname where appropriate
    query.prepare("SELECT formalNames FROM nicknames WHERE nickname = :nickname");
    query.bindValue(":nickname", QVariant(nickname));
    success = query.exec();
    query.next();

    if (!success || (query.size() > 1))
    {
        error = query.lastError();

        errorMessage << QString("SQL error trying to determine if nicknames already contains: ") << nickname;
        globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
    }
    else
    {
        if (query.size() == 0)  // new name to be loaded
        {
            query.prepare("INSERT INTO nicknames(nickname, formalNames) VALUES (:nickname, :formalNames)");
            query.bindValue(":nickname", QVariant(nickname));
            query.bindValue(":formalNames", QVariant(formalname));
            success = query.exec();
            query.next();

            if (!success)
            {
                error = query.lastError();

                errorMessage << QString("SQL error trying to add new nickname: ") << nickname;
                globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
            }

            query.clear();

        }
        else    // name exists
        {
            bool nameListUpdated = false;
            bool handled = false;
            int i = 0;
            int compareResult;

            // Pull current formalnames
            QString currentNames = query.value(0).toString();
            QList<QString> listOfNames = currentNames.split(QString(" "), QString::SkipEmptyParts, Qt::CaseInsensitive);
            if ((listOfNames.size() == 1) && (listOfNames.at(0).length() == 0))
                listOfNames.clear();
            int numNames = listOfNames.size();

            if (numNames == 0)
            {
                listOfNames.append(formalname);
                handled = true;
                nameListUpdated = true;
            }

            while (!handled && (i < numNames))
            {
                compareResult = formalname.compare(listOfNames.at(i), Qt::CaseInsensitive);
                if (compareResult == 0)
                    handled = true;
                else
                {
                    if (compareResult < 0)
                    {
                        listOfNames.insert(i, formalname);
                        handled = true;
                        nameListUpdated = true;
                    }
                    else
                    {
                        // compareResult >= 1
                        i++;
                        if (i == numNames)
                        {
                            // No more possible matches
                            listOfNames.append(formalname);
                            handled = true;
                            nameListUpdated = true;
                        }
                    }
                }
            }

            if (nameListUpdated)
            {
                QString formalNames;
                numNames = listOfNames.size();

                for (i = 0 ; i < numNames; i++)
                {
                    formalNames += listOfNames.at(i);
                    if (i < (numNames - 1))
                        formalNames += QString(" ");
                }

                query.prepare("UPDATE nicknames SET formalNames = :formalNames WHERE nickname = :nickname");
                query.bindValue(":formalNames", QVariant(formalNames));
                query.bindValue(":nickname", QVariant(nickname));
                success = query.exec();
                query.next();

                if (!success)
                {
                    error = query.lastError();

                    errorMessage << QString("SQL error trying to add formal name: ") << formalname << QString(" to nickname: ") << nickname;
                    globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
                }
            }
        }
    }
}

