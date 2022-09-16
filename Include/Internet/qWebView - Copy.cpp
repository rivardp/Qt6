#include "qWebView.h"

WebView::WebView(QObject *parentDM, QWidget *parent) : QWebEngineView(parent)
{
    Q_UNUSED(parentDM);

    QApplication::instance()->installEventFilter(this);

    downloadFinished = false;
    HTMLfinished = false;
    PDFfinished = false;
    queueFinished = false;

    connect(this, SIGNAL(loadingComplete(bool)), this, SLOT(finishProcessing(bool)));
    connect(this, &QWebEngineView::loadFinished, [this](bool success) {downloadFinished = success;});
    connect(this, SIGNAL(HTMLready(QString)), this, SLOT(saveHTML(QString)));
    connect(this, SIGNAL(finishedTask(int)), this, SLOT(trackTasks(int)));

    // Settings to permit capturing of webpage screen shot - part of render solution
    // render() only works after show() called, first two settings hide the window and clean up afterwards
    //setAttribute(Qt::WA_DontShowOnScreen);
    //setAttribute(Qt::WA_DeleteOnClose);
    //settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    //settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
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
        //createHTMLfile();   // Task 1
        emit finishedTask(1);
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

void WebView::createHTMLfile()
{
    qDebug() << "Made it here createHTMLfile";

    // The FunctorOrLambda function only calls back once the asynchronous function is complete
    page()->toHtml([this](const QString& webViewContents) mutable {emit HTMLready(webViewContents);});
    emit finishedTask(1);
}

void WebView::createPDFfile()
{

    qDebug() << "Made it to createPDFfile  " << qUrl.toString();

    // This line works, but average file size is 750KB
    //page()->printToPdf(outputs.PDFfileName, QPageLayout(QPageSize(QPageSize::A2), QPageLayout::Portrait, QMarginsF()));
    emit finishedTask(2);

    /*  This section is awaiting a fix to support rendering
    show();
    QEventLoop asyncWait;
    QTimer::singleShot(5000, &asyncWait, SLOT(quit()));
    asyncWait.exec();

    hide();

    // Create and setup PDF printer
    PDFprinter = new QPrinter(QPrinter::HighResolution);
    PDFprinter->setOutputFormat(QPrinter::PdfFormat);
    PDFprinter->setResolution(100);
    PDFprinter->setFullPage(true);
    //PDFprinter->setOrientation(QPrinter::Portrait);
    //PDFprinter->setOrientation(QPrinter::Landscape);
    PDFprinter->setPaperSize(QPrinter::A4);
    PDFprinter->setPageMargins(0,0,0,0, QPrinter::Millimeter);
    PDFprinter->setOutputFileName(outputs.PDFfileName);
    //QSize PDFsize(PDFprinter.width(), PDFprinter.height());

    QPainter painter;
    painter.begin(PDFprinter);
    int printx = PDFprinter->pageRect().width();
    int printy = PDFprinter->pageRect().height();
    int pagex = width();
    int pagey = height();
    double xscale = PDFprinter->pageRect().width()/double(width());
    double yscale = PDFprinter->pageRect().height()/double(height());
    double scale = qMin(xscale, yscale);
    painter.translate(PDFprinter->paperRect().x() + PDFprinter->pageRect().width()/2,
                       PDFprinter->paperRect().y() + PDFprinter->pageRect().height()/2);
    painter.scale(scale, scale);
    painter.translate(-width()/2, -height()/2);

    render(PDFprinter);
    emit finishedTask(2);
    painter.end();*/

    // Short term solution trying to use PDFprinter, but output wasn't scalable
    //page()->print(PDFprinter, [this] (const bool success) mutable {printingFinished(success);});

    /*QPixmap pixmap(this->size());
    page()->view()->show();
    page()->view()->hide();
    this->render(&pixmap);
    if (pixmap.isNull())
        qDebug() << "Houston we have a problem";

    QGraphicsScene scene;
    QGraphicsView view(&scene);
    QGraphicsPixmapItem item(pixmap);
    scene.addItem(&item);
    view.show();

    QPainter painter;
    painter.begin(&PDFprinter);
    //painter.drawPixmap(0,0,pixmap.scaled(PDFsize, Qt::KeepAspectRatio));
    painter.drawPixmap(0,0,pixmap);
    emit finishedTask(2);
    painter.end();*/

    /* More potential coding that is dependent on render() being fixed
    qDebug() << "Timer in createPDFfile started...";
    QEventLoop asyncWait;
    QTimer::singleShot(20, &asyncWait, SLOT(quit()));
    asyncWait.exec();
    qDebug() << "Timer ended";

    // The next line triggers an update request, which is required to avoid a handle error
    this->show();
    this->hide();

    QPixmap pixmap(size());
    //painter->set.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    page()->view()->render(&pixmap);
    QSize PDFsize(PDFprinter.width(), PDFprinter.height());

    QGraphicsScene scene;
    QGraphicsView view(&scene);
    QGraphicsPixmapItem item(pixmap);
    scene.addItem(&item);
    view.show();


    view.hide();

    if (pixmap.isNull())
        qDebug() << "Houston we have a problem";

    QPainter painter;
    painter.begin(&PDFprinter);
    painter.drawPixmap(0,0,pixmap.scaled(PDFsize, Qt::KeepAspectRatio));
    painter.end();
    emit finishedTask(2);*/
}

void WebView::addFileToQueue()
{
    qDebug() << "Made it to addFileToQueue " << qUrl.toString();

    QTextStream outputStream(outputs.queue);
    QString entry(outputs.PDFfileName);
    entry.append("||");
    entry.append(qUrl.toString());
    outputStream << entry << endl;

    emit finishedTask(3);
}

void WebView::saveHTML(QString HTMLstring)
{
    QFile *qFile;

    qFile = new QFile(outputs.HTMLfileName);
    if (qFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream outputStream(qFile);
        outputStream << HTMLstring.toUtf8();
        qFile->close();
    }
    delete qFile;
    qFile = 0;
}

void WebView::setOutputs(downloadOutputs &targets)
{
    outputs = targets;

    if (!outputs.addToQueue)
        queueFinished = true;
}

void WebView::setUrl(QUrl url)
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
       emit allTasksCompleted();
   }
}

void WebView::logTimeout()
{
    PQString errMsg;
    errMsg << "Download timeout occurred with: " << qUrl.toString();
    outputs.globals->logMsg(ErrorRunTime, errMsg);
}

void WebView::printingFinished(bool success)
{
    if (success)
        emit finishedTask(2);

    // TO DO: Finalize or eliminate depending on final render solution/requirement
    if (PDFprinter)
        delete PDFprinter;
}

bool WebView::eventFilter(QObject* object, QEvent* event)
{
   QEvent::Type eventType = event->type();
   QString objectName = object->objectName();

    if (eventType == QEvent::UpdateRequest)
   {
       emit updateFinished();
   }
   return false;
}

