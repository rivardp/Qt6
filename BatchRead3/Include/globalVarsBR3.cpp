#include "../UpdateFuneralHomes/Include/globalVars.h"

GLOBALVARS::GLOBALVARS()
{
}

GLOBALVARS::~GLOBALVARS()
{
}

bool GLOBALVARS::setupDirectoryStructure()
{
    // Client setup occurs as files are read in
    return setupBaseDirectoryStructure();
}

bool GLOBALVARS::setupBaseDirectoryStructure()
{
    QDir directory;

    // Input Directories
    readDirectory = PQString("C:\\Obits\\Batch Read");
    if (!directory.exists(readDirectory.getString()))
        directory.mkdir(readDirectory.getString());
    directory.setCurrent(readDirectory.getString());

    // General Output Directories
    logsDirectory = PQString("C:\\Obits\\Error Logs");
    reportsDirectory = PQString("C:\\Obits\\Batch Read");

    // Operating Directories
    baseDirectory = PQString("C:\\Obits\\Providers");
    if (!directory.exists(baseDirectory.getString()))
        directory.mkdir(baseDirectory.getString());
    directory.setCurrent(baseDirectory.getString());

    return true;     // For backwards compatibility
}

bool GLOBALVARS::setupClientDirectoryStructure(PQString clientSubDir)
{
    PQString subDir, currentDirectory;
    QDir directory, origDirectory;
    bool success = true;

    if (clientSubDir.getLength() == 0)
        return false;

    // Save original directory and reset it at end
    origDirectory = QDir::current();

    // Make sure client directory structure exists below base structure
    // baseDirectory includes "\\Providers" in this implementation
    directory.setCurrent(baseDirectory.getString());
    if (!directory.exists(clientSubDir.getString()))
        directory.mkdir(clientSubDir.getString());
    currentDirectory = baseDirectory + PQString("\\") + clientSubDir;
    directory.setCurrent(currentDirectory.getString());

    subDir = PQString("Obituaries");
    if (!directory.exists(subDir.getString()))
        directory.mkdir(subDir.getString());
    if ((run == Obits) || (run == BatchRead))
        reportsDirectory = baseDirectory + PQString("\\") + clientSubDir + PQString("\\") + subDir;

    subDir = PQString("PDFs");
    if (!directory.exists(subDir.getString()))
        directory.mkdir(subDir.getString());
    PDFdirectory = baseDirectory + PQString("\\") + clientSubDir + PQString("\\PDFs");

    directory.setCurrent(origDirectory.absolutePath());

   return success;  // Just for backwards compatibility
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

    root = QString("BatchRead3runTimeError");
    FHrunTimeError = new QFile((root + stringdate + extension).getString());
    if(FHrunTimeError->open(QIODevice::WriteOnly | QIODevice::Text))
        FHrunTimeError->close();
    else
        success = false;

    root = QString("BatchRead3urlError");
    FHurlError = new QFile((root + stringdate + extension).getString());
    if(FHurlError->open(QIODevice::WriteOnly | QIODevice::Text))
        FHurlError->close();
    else
        success = false;

    root = QString("BatchRead3connectionError");
    FHconnectionError = new QFile((root + stringdate + extension).getString());
    if(FHconnectionError->open(QIODevice::WriteOnly | QIODevice::Text))
        FHconnectionError->close();
    else
        success = false;

    root = QString("BatchRead3sqlError");
    FHsqlError = new QFile((root + stringdate + extension).getString());
    if(FHsqlError->open(QIODevice::WriteOnly | QIODevice::Text))
        FHsqlError->close();
    else
        success = false;

    root = QString("BatchRead3actionRequired");
    FHactionRequired = new QFile((root + stringdate + extension).getString());
    if(FHactionRequired->open(QIODevice::WriteOnly | QIODevice::Text))
        FHactionRequired->close();
    else
        success = false;

    root = QString("BatchRead3runSummary");
    FHrunSummary = new QFile((root + stringdate + extension).getString());
    if(FHrunSummary->open(QIODevice::WriteOnly | QIODevice::Text))
        FHrunSummary->close();
    else
        success = false;

    root = QString("BatchRead3auditListing");
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
    QTextStream *outputStream = nullptr;
    QFile *eliminateWarning = new QFile;

    switch (messageType)
    {
    case ErrorSQL:
        if(FHsqlError->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHsqlError);
        else
            outputStream = new QTextStream(eliminateWarning);

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

        *outputStream << "SQL" << AdditionalInfo << msg << Qt::endl;
        FHsqlError->close();
        break;

    case ErrorURL:
        if(FHurlError->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHurlError);
        else
            outputStream = new QTextStream(eliminateWarning);

        *outputStream << "URL Error: " << msg << Qt::endl;
        FHurlError->close();
        break;

    case ErrorRunTime:
        if(FHrunTimeError->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHrunTimeError);
        else
            outputStream = new QTextStream(eliminateWarning);

        *outputStream << "Run Time Error: " << msg << Qt::endl;
        FHrunTimeError->close();
        break;

    case ErrorConnection:
        if(FHconnectionError->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHconnectionError);
        else
            outputStream = new QTextStream(eliminateWarning);

        *outputStream << "Connection Error: " << msg << Qt::endl;
        FHconnectionError->close();
        break;

    case ErrorRecord:
        if(FHrecordError->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHrecordError);
        else
            outputStream = new QTextStream(eliminateWarning);

        *outputStream << "Individual Record Error: " << msg << Qt::endl;
        FHrecordError->close();
        break;

    case ActionRequired:
        if(FHactionRequired->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHactionRequired);
        else
            outputStream = new QTextStream(eliminateWarning);

        switch(msgInfo)
        {
        case 1:
            AdditionalInfo = " - Nothing defined yet: ";
            break;

        default:
            AdditionalInfo = ": ???";
        }

        *outputStream << "Action Required" << AdditionalInfo << msg << Qt::endl;
        FHactionRequired->close();
        break;

    case RunSummary:
        if(FHrunSummary->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHrunSummary);
        else
            outputStream = new QTextStream(eliminateWarning);

        *outputStream << "BatchRead2 stat: " << msg << Qt::endl;        
        FHrunSummary->close();
        break;

    case AuditListing:
        if(FHauditListing->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            outputStream = new QTextStream(FHauditListing);
        else
            outputStream = new QTextStream(eliminateWarning);

        *outputStream << msg << Qt::endl;
        FHauditListing->close();
        break;

    case NoMsg:
        outputStream = new QTextStream(eliminateWarning);
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
    delete eliminateWarning;
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
        //QMessageBox::warning(nullptr, "Setup Problem", "Unable to retrieve current date", "Ok");
        QMessageBox mb(QMessageBox::Warning, "Setup Error", "Unable to retrieve current date");

    todaySQL << today.toString("yyyy/MM/dd") << QString(" 0:0:0");

    successfulStep = setupDirectoryStructure();
    if (!successfulStep)
        //QMessageBox::warning(nullptr, "Setup Problem", "Unable to establish directory structure", "Ok");
        QMessageBox mb(QMessageBox::Warning, "Setup Error", "Unable to establish directory structure");
    setupSuccessful = setupSuccessful && successfulStep;

    successfulStep = createAllOutputFiles(today);
    if (!successfulStep)
        //QMessageBox::warning(nullptr, "Setup Problem", "Unable to create output files", "Ok");
        QMessageBox mb(QMessageBox::Warning, "Setup Error", "Unable to create output files");
    setupSuccessful = setupSuccessful && successfulStep;

    batchRunFlag = determineIfBatchRun();

    return setupSuccessful;
}

bool GLOBALVARS::determineIfBatchRun()
{
    bool isBatchRun = false;

    QDateTime batchStart, thisStart, dateForName;

    dateForName = QDateTime::currentDateTime();
    if (dateForName.time() > QTime(22, 55))
        dateForName = dateForName.addDays(1);

    QString targetFileName(QString("C:\\Obits\\Batch Read\\Nightly Obit Run Log for "));
    targetFileName += dateForName.toString("yyyyMMdd");
    targetFileName += QString(".log");

    if(QFileInfo::exists(targetFileName))
    {
        QFileInfo fi(targetFileName);
        batchStart = fi.birthTime();
        thisStart = QDateTime::currentDateTime();
        int seconds = batchStart.secsTo(thisStart);
        double mins = seconds / 60;
        if (mins < 240)
        {
            isBatchRun = true;
            batchRunFlag = true;
            batchRunFileName = targetFileName;
        }
    }

    return isBatchRun;
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

