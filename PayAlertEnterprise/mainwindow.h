
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qitemselectionmodel.h"
#include <qstringlistmodel.h>
#include <QMainWindow>
#include <QCloseEvent>
#include <QColumnView>
#include <QDir>
#include <QFileInfo>

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

    // TableView methods
    void setActiveRecord(GROUPCONFIG &ar);
    GROUPCONFIG getActiveRecord() const;

    void initTableView();
    void initScheduling(Ui::MainWindow *ui, GROUPCONFIG &activeRecord);
    void initReporting(QList<USERINFO> &allUsers, QStringList &allEligibleEmails, QStringList &allChosenEmails);

    void updateModel();
    void updateModel(QModelIndex &indexToUpdate);
    void updateSourceReference();

    // Scheduling Methods
    void setupWeeklyDropDown(Ui::MainWindow &ui, GROUPCONFIG &activeRecord);
    void setupDayPicks(Ui::MainWindow &ui, GROUPCONFIG &activeRecord);
    void updateSchedulingGroup(Ui::MainWindow &ui, GROUPCONFIG &activeRecord);
    void disableDayPicks(Ui::MainWindow *ui);
    void enableDayPicks(Ui::MainWindow *ui);

    // Reporting methods
    void updateReportingGroup(Ui::MainWindow &ui, GROUPCONFIG &activeRecord);

    // Other methods
    QModelIndexList matchColumnValues(QColumnView &cv, QList<QString> &toBeMatched);
    QModelIndexList matchColumnValues(QColumnView &cv, QString toBeMatched);

    void closeEvent (QCloseEvent *event);
    void save();

protected slots:
    void groupSelected(const QItemSelection &rowSelected, const QItemSelection &rowDeselected);
    void onGroupNameEdit(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private slots:
    void on_PB_DeleteGroup_clicked();
    void on_PB_AddGroup_clicked();

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

    void on_CB_Sunday_clicked();
    void on_CB_Monday_clicked();
    void on_CB_Tuesday_clicked();
    void on_CB_Wednesday_clicked();
    void on_CB_Thursday_clicked();
    void on_CB_Friday_clicked();
    void on_CB_Saturday_clicked();

    void on_CB_High_clicked();
    void on_CB_Good_clicked();
    void on_CB_Reasonable_clicked();
    void on_CB_Possible_clicked();

    void on_PB_Exit_clicked();
    void on_PB_SaveAndExit_clicked();
    void on_PB_Save_clicked();


    void on_RB_FullHistory_clicked();

    void on_RB_IncrementalOnly_clicked();

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


