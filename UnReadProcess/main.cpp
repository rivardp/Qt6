#include <QCoreApplication>
#include <QString>
#include <QDir>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Get filename as the second argument
    QString targetFileName, newFileName;
    QStringList argList = QCoreApplication::arguments();
    if (argList.size() != 2)
        return 0;
    else
        targetFileName = argList.at(1);

    //targetFileName = QString("C:\\Obits\\Batch Read\\testing.obitList.processed");

    // Pick off extension
    int index = targetFileName.lastIndexOf(QChar('.'));
    if (index < 0)
        return 0;
    QString ext = targetFileName.mid(index,-1);
    if ((ext != QString(".read")) && (ext != ".processed"))
        return 0;

    newFileName = targetFileName;
    newFileName.chop(ext.size());

    QDir fullPathName;
    fullPathName.rename(targetFileName, newFileName);

    return 0;
}
