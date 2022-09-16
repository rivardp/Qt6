#include "qWebPage.h"

WebPage::WebPage(QWebEngineProfile *profile, QObject *parent) : QWebEnginePage(profile, parent)
{
    downloadFinished = false;
    HTMLfinished = false;
    PDFfinished = false;
    queueFinished = false;

    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(finishProcessing(bool)));
    connect(this, &QWebEnginePage::loadFinished, [this](bool success) {downloadFinished = success;});
    connect(this, SIGNAL(HTMLready(QString)), this, SLOT(saveHTML(QString)));
    connect(this, SIGNAL(finishedAllTasks(WebPage*)), this->parent(), SLOT(cleanUp(WebPage*)));

}

WebPage::~WebPage()
{
//    delete PDFprinter;
}

void WebPage::finishProcessing(bool success)
{
    qDebug() << "LoadFinished() signal in WebPage to execute finishProcessing()";

    if (success)
    {
        createHTMLfile();   // Task 1
        createPDFfile();    // Task 2
        addFileToQueue();   // Task 3
    }
    else
    {
        qDebug() << "Download Error: " << "missing error string";
        emit finishedTask(1);
        emit finishedTask(2);
        emit finishedTask(3);
    }
}

void WebPage::createHTMLfile()
{
    qDebug() << "Made it here webPage";
    toHtml([this](const QString& webPageContents) mutable {emit HTMLready(webPageContents);});
    emit finishedTask(1);
}

void WebPage::createPDFfile()
{
    // The next two lines do the job, but the PDF file size averaged 750KB
    //printToPdf(outputs.PDFfileName, QPageLayout(QPageSize(QPageSize::A2), QPageLayout::Portrait, QMarginsF()));
    //emit finishedTask(2);

    // These two lines also worked (with the extra functions), but again average file size was 750KB
    //PDFprinter->setOutputFileName(outputs.PDFfileName);
    //print(PDFprinter, [this] (const bool success) mutable {printingFinished(success);});

    // The third approach...
    QSizeF pageSize;

    QPrinter PDFprinter;
    pageSize = PDFprinter.pageRect().size();    // page size in pixels
    PDFprinter.setOutputFormat(QPrinter::PdfFormat);
    pageSize = PDFprinter.pageRect().size();    // page size in pixels
//    PDFprinter.setResolution(QPrinter::HighResolution);
//    PDFprinter.setFullPage(true);
    pageSize = PDFprinter.pageRect().size();    // page size in pixels
    PDFprinter.setPaperSize(QPrinter::A6);
    pageSize = PDFprinter.pageRect().size();    // page size in pixels
    PDFprinter.setOrientation(QPrinter::Portrait);
    PDFprinter.setPageMargins(0,0,0,0, QPrinter::Millimeter);
    pageSize = PDFprinter.pageRect().size();    // page size in pixels
    PDFprinter.setOutputFileName(outputs.PDFfileName);
    pageSize = PDFprinter.pageRect().size();    // page size in pixels

    QWebEnginePage tempWebPage(this);
    QWebEngineView webView;
    this->setView(webView);
    webView.setPage(&tempWebPage);
    webView.setMaximumSize(960,10000);
    webView.setMinimumWidth(960);
    webView.update();
    webView.show();  // Causes update event, which is required to avoid handle error
    pageSize = webView.size();
    QPixmap pixmap(webView.size());
//    QPixmap pixmap("C:\\Users\\USER\\Documents\\Qt\\Projects\\img0.jpg");
    webView.page()->view()->render(&pixmap);
    QSize PDFsize(PDFprinter.width(), PDFprinter.height());
    QPainter painter;
    painter.begin(&PDFprinter);
    painter.drawPixmap(0,0,pixmap.scaled(PDFsize, Qt::KeepAspectRatio));
    painter.end();
    emit finishedTask(2);
}

void WebPage::addFileToQueue()
{
    QTextStream outputStream(outputs.queue);
    QString entry(outputs.PDFfileName);
    entry.append("||");
    entry.append(requestedUrl().toString());
    outputStream << entry << endl;

    emit finishedTask(3);
}

void WebPage::saveHTML(QString HTMLstring)
{
    QFile *qFile;

    qFile = new QFile(outputs.HTMLfileName);
    if (qFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream outputStream(qFile);
        outputStream << HTMLstring;
        qFile->close();
    }
}

void WebPage::setOutputs(downloadOutputs &targets)
{
    outputs = targets;

    if (!outputs.addToQueue)
        queueFinished = true;
}

void WebPage::trackTasks(int taskFinished)
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

   if (HTMLfinished && PDFfinished && queueFinished)
   {
       emit allTasksCompleted();
       emit finishedAllTasks(this);
   }
}

void WebPage::logTimeout()
{
    PQString errMsg;
    errMsg << "Download timeout occurred with: " << requestedUrl().toString();
    outputs.globals->logMsg(ErrorRunTime, errMsg);
}

void WebPage::printingFinished(bool success)
{
    if (success)
        emit finishedTask(2);
}

