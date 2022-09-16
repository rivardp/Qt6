#ifndef QWEBPAGE_H
#define QWEBPAGE_H

#include <QObject>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QSize>
#include <QUrl>
#include <QDebug>
#include <QPrinter>
#include <QPrinterInfo>
#include <QPixmap>

#include "../Include/Internet/qWebStructures.h"

class WebPage : public QWebEnginePage
{
    Q_OBJECT

public:
    WebPage(QWebEngineProfile* profile, QObject *parent = nullptr);
    ~WebPage();

    void setOutputs(downloadOutputs &targets);
    bool stillProcessing();
    void printingFinished(bool success);

//    QPrinter *PDFprinter;

signals:
    void HTMLready(QString file);
    void PDFready();
    void finishedTask(int);
    void finishedAllTasks(WebPage *);
    void allTasksCompleted();

protected slots:
    void finishProcessing(bool success);
    void saveHTML(QString file);
    void trackTasks(int);
    void logTimeout();

private:    
    void createHTMLfile();
    void createPDFfile();
    void addFileToQueue();

    downloadOutputs outputs;

    bool downloadFinished;      // Reserved for future lamda use (since it actually works)
    bool HTMLfinished;
    bool PDFfinished;
    bool queueFinished;


};

#endif // QWEBPAGE_H

