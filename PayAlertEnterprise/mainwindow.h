
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qitemselectionmodel.h"
#include <qstringlistmodel.h>
#include <QMainWindow>
#include <QColumnView>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#include "myDataModel.h"

class MainWindow : public QMainWindow

{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setActiveRecord(GROUPCONFIG &ar);
    GROUPCONFIG getActiveRecord() const;

    void initTableView();
    void initScheduling(Ui::MainWindow *ui, GROUPCONFIG &activeRecord);
    void initReporting(QList<USERINFO> &allUsers, QStringList &allEligibleEmails, QStringList &allChosenEmails);

    void updateModel();

    // Scheduling Methods
    void setupWeeklyDropDown(Ui::MainWindow &ui, GROUPCONFIG &activeRecord);
    void setupDayPicks(Ui::MainWindow &ui, GROUPCONFIG &activeRecord);
    void updateSchedulingGroup(Ui::MainWindow &ui, GROUPCONFIG &activeRecord);
    void disableDayPicks(Ui::MainWindow *ui);
    void enableDayPicks(Ui::MainWindow *ui);

    // Reporting methods
    void updateReportingGroup(Ui::MainWindow &ui, GROUPCONFIG &activeRecord);

    // Other methods
    QModelIndexList matchColumnValues(QColumnView &cv, QList<QString>& toBeMatched);

protected slots:
    void groupSelected(const QItemSelection &rowSelected, const QItemSelection &rowDeselected);

private slots:
    void on_RB_Disabled_clicked();
    void on_RB_Daily_clicked();
    void on_RB_Weekdays_clicked();
    void on_RB_Weekly_clicked();
    void on_RB_BiWeekly_clicked();
    void on_RB_Monthly_clicked();
    void on_RB_Custom_clicked();
    void on_IB_DayOfMonthChosen_textChanged(const QString &arg1);
    void on_DD_WeeklyChosen_currentIndexChanged(int index);

    void on_PB_Add_clicked();
    void on_PB_Remove_clicked();

private:
    Ui::MainWindow *ui;
    myDataModel *m_model;
    GROUPCONFIG activeRecord;
    QList<USERINFO> allUsers;
    QStringList allEligibleEmails, allChosenEmails;
    QStringListModel *stringListModelAll;
    QStringListModel *stringListModelIncl;
};

#endif // MAINWINDOW_H


