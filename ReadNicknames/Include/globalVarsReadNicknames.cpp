#include "../UpdateFuneralHomes/Include/globalVars.h"

GLOBALVARS::GLOBALVARS()
{
}

GLOBALVARS::~GLOBALVARS()
{
}

bool GLOBALVARS::setupDirectoryStructure()
{
    PQString subDir;
    QDir directory;
    bool success = true;

    setupBaseDirectoryStructure();

    subDir = PQString("Error Logs");
    if (!directory.exists(subDir.getString()))
        directory.mkdir(subDir.getString());
    logsDirectory = baseDirectory + PQString("\\") + subDir;

    subDir = PQString("Reports");
    if (!directory.exists(subDir.getString()))
        directory.mkdir(subDir.getString());
    reportsDirectory = baseDirectory + PQString("\\") + subDir;

    success = directory.exists(reportsDirectory.getString()) && directory.exists(logsDirectory.getString());

    return success;
}

bool GLOBALVARS::setupBaseDirectoryStructure()
{
    QDir directory;

    baseDirectory = PQString("C:\\Obits");
    if (!directory.exists(baseDirectory.getString()))
        directory.mkdir(baseDirectory.getString());
    directory.setCurrent(baseDirectory.getString());

    return true;
}

bool GLOBALVARS::setupClientDirectoryStructure(PQString clientSubDir)
{
    Q_UNUSED(clientSubDir);

    return true;
}

bool GLOBALVARS::createAllOutputFiles(QDate &today)
{
    bool success = true;
    PQString extension(QString(".log"));
    PQString root;
    PQString stringdate(today.toString("yyyyMMdd"));
    QDir directory;

    /*********************/
    /*       Logs        */
    /*********************/

    directory.setCurrent(logsDirectory.getString());

    root = QString("ReadNickNamesCSVrunTimeError");
    FHrunTimeError = new QFile((root + stringdate + extension).getString());
    if(FHrunTimeError->open(QIODevice::WriteOnly | QIODevice::Text))
        FHrunTimeError->close();
    else
        success = false;

    root = QString("ReadNickNamesCSVurlError");
    FHurlError = new QFile((root + stringdate + extension).getString());
    if(FHurlError->open(QIODevice::WriteOnly | QIODevice::Text))
        FHurlError->close();
    else
        success = false;

    root = QString("ReadNickNamesCSVconnectionError");
    FHconnectionError = new QFile((root + stringdate + extension).getString());
    if(FHconnectionError->open(QIODevice::WriteOnly | QIODevice::Text))
        FHconnectionError->close();
    else
        success = false;

    root = QString("ReadNickNamesCSVsqlError");
    FHsqlError = new QFile((root + stringdate + extension).getString());
    if(FHsqlError->open(QIODevice::WriteOnly | QIODevice::Text))
        FHsqlError->close();
    else
        success = false;

    root = QString("ReadNickNamesCSVactionRequired");
    FHactionRequired = new QFile((root + stringdate + extension).getString());
    if(FHactionRequired->open(QIODevice::WriteOnly | QIODevice::Text))
        FHactionRequired->close();
    else
        success = false;

    root = QString("ReadNickNamesCSVrunSummary");
    FHrunSummary = new QFile((root + stringdate + extension).getString());
    if(FHrunSummary->open(QIODevice::WriteOnly | QIODevice::Text))
        FHrunSummary->close();
    else
        success = false;

    root = QString("ReadNickNamesCSVauditListing");
    FHauditListing = new QFile((root + stringdate + extension).getString());
    if(FHauditListing->open(QIODevice::WriteOnly | QIODevice::Text))
        FHauditListing->close();
    else
        success = false;

    /*********************/
    /*      Output       */
    /*********************/

    // Setup as part of the mining process

    return success;
}

void GLOBALVARS::logMsg(msgType messageType, QString msg, int msgInfo)
{
    QString AdditionalInfo;
    QTextStream *outputStream;

    switch (messageType)
    {
    case ErrorSQL:
        if(FHsqlError->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHsqlError);

        switch(msgInfo)
        {
        case 1:
            AdditionalInfo = " - Connection Error: ";
            break;

        case 2:
            AdditionalInfo = " - Statement Error: ";
            break;

        case 3:
            AdditionalInfo = " - Transaction Error: ";
            break;

        case 4:
            AdditionalInfo = " - Unknown Error: ";
            break;

        default:
            AdditionalInfo = ": ";
        }

        *outputStream << "SQL" << AdditionalInfo << msg << endl;
        FHsqlError->close();
        break;

    case ErrorURL:
        if(FHurlError->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHurlError);

        *outputStream << "URL Error: " << msg << endl;
        FHurlError->close();
        break;

    case ErrorRunTime:
        if(FHrunTimeError->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHrunTimeError);

        *outputStream << "Run Time Error: " << msg << endl;
        FHrunTimeError->close();
        break;

    case ErrorConnection:
        if(FHconnectionError->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHconnectionError);

        *outputStream << "Connection Error: " << msg << endl;
        FHconnectionError->close();
        break;

    case ErrorRecord:
        if(FHrecordError->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHrecordError);

        *outputStream << "Individual Record Error: " << msg << endl;
        FHrecordError->close();
        break;

    case ActionRequired:
        if(FHactionRequired->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHactionRequired);
        switch(msgInfo)
        {
        case 1:
            AdditionalInfo = " - Nothing defined yet: ";
            break;

        default:
            AdditionalInfo = ": ???";

        }

        *outputStream << "Action Required" << AdditionalInfo << msg << endl;
        FHactionRequired->close();
        break;

    case RunSummary:
        if(FHrunSummary->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHrunSummary);

        *outputStream << "ReadNickNamesCSV stat: " << msg << endl;
        FHrunSummary->close();
        break;

    case AuditListing:
        if(FHauditListing->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHauditListing);

        *outputStream << msg << endl;
        FHauditListing->close();
        break;

    case DefdErrorRecord:
        // This one is not like the others, the error message is rechecked at the end to see if it still applies
        DEFDERRORMESSAGES dem;
        dem.mt = ErrorRecord;
        dem.msg = msg;
        dem.msgInfo = msgInfo;
        defdErrorMessages.append(dem);
        break;

    }

    delete outputStream;
}

void GLOBALVARS::logMsg(msgType messageType, PQString msg, int msgInfo)
{
    logMsg(messageType, msg.getString(), msgInfo);
}

void GLOBALVARS::logMsg(msgType messageType, OQString msg, int msgInfo)
{
    logMsg(messageType, msg.getString(), msgInfo);
}

bool GLOBALVARS::runSetups()
{
    bool successfulStep;

    today = QDate::currentDate();
    setupSuccessful = today.isValid();
    if (!setupSuccessful)
        QMessageBox::warning(nullptr, "Setup Problem", "Unable to retrieve current date", "Ok");
    todaySQL << today.toString("yyyy/MM/dd") << QString(" 0:0:0");

    successfulStep = setupDirectoryStructure();
    if (!successfulStep)
        QMessageBox::warning(nullptr, "Setup Problem", "Unable to establish directory structure", "Ok");
    setupSuccessful = setupSuccessful && successfulStep;

    successfulStep = createAllOutputFiles(today);
    if (!successfulStep)
        QMessageBox::warning(nullptr, "Setup Problem", "Unable to create output files", "Ok");
    setupSuccessful = setupSuccessful && successfulStep;

    return setupSuccessful;
}

DEFDERRORMESSAGES::DEFDERRORMESSAGES()
{};

DEFDERRORMESSAGES::~DEFDERRORMESSAGES()
{};

void DEFDERRORMESSAGES::clear()
{
    mt = NoMsg;
    msg.clear();
    msgInfo = 0;
}
