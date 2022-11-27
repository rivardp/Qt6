#include "../Internet/qWebview.h"

WebView::WebView(QObject *parentDM, QWidget *parent) : QWebEngineView(parent)
{
    Q_UNUSED(parentDM);

//    QApplication::instance()->installEventFilter(this);

    downloadFinished = false;
    HTMLfinished = false;
    PDFfinished = false;
    queueFinished = false;

    connect(this, SIGNAL(loadingComplete(bool)), this, SLOT(finishProcessing(bool)));
    connect(this, &QWebEngineView::loadFinished, [this](bool success) {downloadFinished = success;});
    connect(this->page(), SIGNAL(pdfPrintingFinished(QString,bool)), this, SLOT(finishedPDFprinting(QString,bool)));
    connect(this, SIGNAL(HTMLready(QString)), this, SLOT(saveHTML(QString)));
    connect(this, SIGNAL(finishedTask(int)), this, SLOT(trackTasks(int)));

}

WebView::~WebView()
{
    //delete PDFprinter;
}

void WebView::finishProcessing(bool success)
{
    qDebug() << "Received loadingComplete() signal in WebView to execute finishProcessing() " << qUrl.toString();

    if (success)
    {
        if (!skipHTMLcreation)
            createHTMLfile();   // Task 1
        else
            emit finishedTask(1);

        createPDFfile();    // Task 2

        if (includeInList)
            addFileToQueue();   // Task 3
        else
            emit finishedTask(3);
    }
    else
    {
        qDebug() << "Download Error: " << "missing error string";
        emit finishedTask(1);
        emit finishedTask(2);
        emit finishedTask(3);
    }
}

void WebView::createHTMLfile()
{
    qDebug() << "Made it here createHTMLfile";

    // The FunctorOrLambda function only calls back once the asynchronous function is complete
    page()->toHtml([this](const QString& webViewContents) mutable {emit HTMLready(webViewContents);});
}

void WebView::createPDFfile()
{

    qDebug() << "Made it to createPDFfile  " << qUrl.toString();
    emit finishedTask(2);

    // This line works, but average file size is 750KB
    //page()->printToPdf(outputs.PDFfileName, QPageLayout(QPageSize(QPageSize::A2), QPageLayout::Portrait, QMarginsF()));


}

void WebView::finishedPDFprinting(const QString &file, bool success)
{
    Q_UNUSED(file);
    Q_UNUSED(success);

    emit finishedTask(2);
}

void WebView::addFileToQueue()
{
    qDebug() << "Made it to addFileToQueue " << qUrl.toString();

    bool exception = (outputs.providerID == 2015);

    QTextStream outputStream(outputs.queue);
    QString entry(outputs.PDFfileName);
    entry.append("||");
    if (exception)
        entry.append(QString("https://schuler-lefebvrefuneralchapel.com/index.html"));
    else
        entry.append(qUrl.toString());
    entry.append("||");
    entry.append(outputs.dob);
    entry.append("||");
    entry.append(outputs.dod);
    entry.append("||");
    entry.append(outputs.maidenNames);
    entry.append("||");
    entry.append(outputs.pubDate);
    outputStream << entry << Qt::endl;

    emit finishedTask(3);
}

void WebView::saveHTML(QString HTMLstring)
{
    QFile *qFile;

    qFile = new QFile(outputs.HTMLfileName);
    if (qFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream outputStream(qFile);
        outputStream << PQString(HTMLstring).replaceLigatures();
        qFile->close();
    }
    delete qFile;
    qFile = nullptr;

    emit finishedTask(1);
}

void WebView::setOutputs(downloadOutputs &targets)
{
    outputs = targets;

    if (!outputs.addToQueue)
        queueFinished = true;
}

void WebView::setURL(QUrl &url)
{
    qUrl = url;
}


void WebView::trackTasks(int taskFinished)
{
    switch (taskFinished)
    {
    case 1:
        HTMLfinished = true;
        break;

    case 2:
        PDFfinished = true;
        break;

    case 3:
        queueFinished = true;
        break;

    default:
        // Do nothing
        break;
    }

    qDebug() << "Task " << taskFinished << " completed for: " << outputs.ID;

   if (HTMLfinished && PDFfinished && queueFinished)
   {
       qDebug() << "allTasksCompleted for:  " << outputs.ID;
       emit allDoneWith(this);
       emit allTasksCompleted();
   }
}

void WebView::logTimeout()
{
    PQString errMsg;
    errMsg << "Download timeout occurred with: " << qUrl.toString();
    outputs.globals->logMsg(ErrorRunTime, errMsg);

    // Stop processing for this provider as something isn't working properly
    outputs.globals->noMajorIssues = false;

    qDebug() << "************* Run terminated early and did not process all the records ************";
}

void WebView::printingFinished(bool success)
{
    if (success)
        emit finishedTask(2);

    // TO DO: Finalize or eliminate depending on final render solution/requirement
    if (PDFprinter)
        delete PDFprinter;
}


