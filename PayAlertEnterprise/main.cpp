
#include "mainwindow.h"
#include "enterprise.h"
#include "../Include/PMySQL/pMySQL.h"

#include <QApplication>
#include <QScreen>

#include <QLocale>
#include <QTranslator>
#include <QDir>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    // Translator
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "PayAlertEnterprise_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    // Initiate settings
    installDirectory = QCoreApplication::applicationDirPath();
    CSVfileDirectory.setPath(installDirectory.path() + QString("/CSVfiles"));
    clientCode = getClientCode();
    if (clientCode == 0)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Missing or invalid PayAlert.config file");
        msgBox.setInformativeText("Call (xxx) xxx-xxxx or email support@payalert.ca for assistance");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return 0;
    }

    // MySQL
    // Setup the MySQL connection with database manager 'db'
    if (!createEnterpriseConnection(QString("voyager1")))
    {
        qDebug() << "Error connecting to SQL database 'clientInfo'";
        return 0;
    }
    else
    {
        qDebug() << "Connected to 'clientInfo'...";
    }

    MainWindow mainWindow;

    // Set initial width to the size of the form
    const QRect availableGeometry = mainWindow.screen()->availableGeometry();
    const QRect appGeometry = mainWindow.geometry();
    int width, height, formWidth, formHeight;
    width = availableGeometry.width();
    height = availableGeometry.height();
    formWidth = appGeometry.width();
    formHeight = appGeometry.height();
    mainWindow.resize(formWidth < width ? formWidth: width, formHeight < height ? formHeight : height);
    mainWindow.setWindowTitle(QObject::tr("PayAlert - Enterprise Version"));

    mainWindow.show();

    return app.exec();
}
