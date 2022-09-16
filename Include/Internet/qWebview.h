#ifndef QWEBVIEW_H
#define QWEBVIEW_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTextStream>
//#include <QtWebEngineWidgets/QtWebEngineWidgets>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QSize>
#include <QUrl>
#include <QDebug>
#include <QPrinter>
//#include <QPainter>
//#include <QPixmap>
#include <QTimer>
#include <QApplication>
//#include <QEvent>

//#include <QGraphicsScene>
//#include <QGraphicsView>
//#include <QGraphicsPixmapItem>

#include "../Include/Internet/qWebStructures.h"
#include "../Include/PMySQL/databaseSearches.h"

class WebView : public QWebEngineView
{
    Q_OBJECT

public:
    explicit WebView(QObject *parentDM, QWidget *parent = nullptr);
    ~WebView();

    void setOutputs(downloadOutputs &targets);
    void setURL(QUrl &url);
    bool stillProcessing();
    void printingFinished(bool success);

    bool skipHTMLcreation;
    bool includeInList;

signals:
    void HTMLready(QString file);
    void PDFready();
    void finishedTask(int);
    void allTasksCompleted();
    void allDoneWith(WebView*);
    void loadingComplete(bool success);
    void updateFinished();

protected slots:
    void finishProcessing(bool success);
    void finishedPDFprinting(const QString &file, bool success);
    void saveHTML(QString file);
    void trackTasks(int);
    void logTimeout();

private:    
    void createHTMLfile();
    void createPDFfile();
    void addFileToQueue();

    downloadOutputs outputs;
    QUrl qUrl;

    bool downloadFinished;      // Reserved for future lamda use (since it actually works)
    bool HTMLfinished;
    bool PDFfinished;
    bool queueFinished;

    QPrinter *PDFprinter;

protected:
//    bool eventFilter(QObject* object, QEvent* event);

};

#endif // QWEBVIEW_H

