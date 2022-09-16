#ifndef GLOBALVARS_H
#define GLOBALVARS_H

#include <QApplication>
#include <QString>
#include <QTextStream>
#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QFile>
#include <QDir>
#include <QSqlDatabase>
#include <QMessageBox>
#include "../Include/Penser/OQString.h"
#include "../../UpdateFuneralHomes/Include/providers.h"

enum msgType { NoMsg, ErrorSQL, ErrorURL, ErrorRunTime, ErrorConnection, ErrorRecord, DefdErrorRecord, ActionRequired, RunSummary, AuditListing };
enum msgSQL { NoError, ConnectionError, StatementError, TransactionError, UnknownError };
enum msgToDo { NoAction, NewFH };

enum runType {runNotDefined, Obits, URLhistory, URLcurrent, BatchRead, BatchProcess, BatchLoad};

class DEFDERRORMESSAGES
{
public:
    DEFDERRORMESSAGES();
    ~DEFDERRORMESSAGES();

    msgType mt;
    QString msg;
    int msgInfo;

    void clear();
};

class OQString;
class dataRecord;
class readObit;
class unstructuredContent;
class MATCHRECORD;

class GLOBALVARS {

public:

    GLOBALVARS();
    ~GLOBALVARS();

    QDate today;
    PQString todaySQL;
    runType run;
    PQString clientBaseDir;
    PROVIDER providerID;
    unsigned int clientKey;
    int runStatus;
    int totalDownloads;
    bool noMajorIssues;
    bool batchRunFlag;
    QString batchRunFileName;

    QSqlDatabase *db;

    bool setupDirectoryStructure();
    bool setupBaseDirectoryStructure();
    bool setupClientDirectoryStructure(PQString dir);
    bool createAllOutputFiles(QDate &today);
    bool determineIfBatchRun();

    void logMsg(msgType mt, QString msg, int msgInfo = 0);
    void logMsg(msgType mt, PQString msg, int msgInfo = 0);
    void logMsg(msgType mt, OQString msg, int msgInfo = 0);

    bool runSetups();
    void cleanUp();

    PQString baseDirectory;
    PQString reportsDirectory;
    PQString PDFdirectory;
    PQString readDirectory;
    PQString batchDirectory;
    QFile *output;

    // Record specific
    dataRecord *globalDr;
    readObit *globalObit;
    unstructuredContent *uc;
    unstructuredContent *justInitialNamesUC;
    QStringList websiteLocationWords;

    QList<DEFDERRORMESSAGES> defdErrorMessages;

private:
    QFile *FHrunTimeError;
    QFile *FHrecordError;
    QFile *FHurlError;
    QFile *FHconnectionError;
    QFile *FHsqlError;
    QFile *FHactionRequired;
    QFile *FHrunSummary;
    QFile *FHauditListing;

    PQString installDirectory;
    PQString workingDirectory;
    PQString logsDirectory;
    PQString actionDirectory;

    bool setupSuccessful;


};

#endif // GLOBALVARS_H

