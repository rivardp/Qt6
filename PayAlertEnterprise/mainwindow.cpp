
#include "mainwindow.h"
#include "myDataModel.h"
#include "enterprise.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QApplication>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QTableView>

#include <QItemSelectionModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_model(new myDataModel(this))
{
    allUsers = getAllUsers(1000);
    GROUPCONFIG initialActiveRecord = m_model->loadInitialData(getGroupConfigs(1000));
    setActiveRecord(initialActiveRecord);

    ui = new Ui::MainWindow;
    ui->setupUi(this);

    // Table View
    ui->tableView->setModel(m_model);
    initTableView();

    connect(ui->tableView->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
        SLOT(groupSelected(const QItemSelection &, const QItemSelection &))
        );

    // Scheduling
    initScheduling(ui, activeRecord);

    // Reporting
    initReporting(allUsers, allEligibleEmails, allChosenEmails);

    // Create initial selection and view
    if (ui->tableView->model()->rowCount() > 0)
        ui->tableView->selectRow(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initTableView()
{
    ui->tableView->hideColumn(0);   // key
    ui->tableView->hideColumn(1);   // clientCode
    ui->tableView->hideColumn(3);   // lastRunDate
    ui->tableView->hideColumn(4);   // nextRunDate
    ui->tableView->hideColumn(7);   // freqOptions
    ui->tableView->hideColumn(8);   // weeklyDay
    ui->tableView->hideColumn(9);   // customDayChosen
    ui->tableView->hideColumn(10);  // monthlyDayChosen
    ui->tableView->hideColumn(11);  // emailRecipients
    ui->tableView->hideColumn(12);  // groupMatchesIncluded

    ui->tableView->setColumnWidth(2, 160);   // name
    ui->tableView->setColumnWidth(5, 200);;  // lastRunDateString
    ui->tableView->setColumnWidth(6, 200);   // nextRunDateString

    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    //ui->tableView->setStyleSheet("QHeaderView::section { background-color: #B9E1FD; border: 0.5px solid #6c6c6c; }");

    QPalette p = palette();
    p.setColor(QPalette::Inactive, QPalette::Highlight, p.color(QPalette::Active, QPalette::Highlight));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText, p.color(QPalette::Active, QPalette::HighlightedText));
    setPalette(p);

    QPalette hp = ui->tableView->horizontalHeader()->palette();
    hp.setColor( QPalette::Normal, QPalette::Highlight, Qt::darkBlue );
    hp.setColor( QPalette::Normal, QPalette::HighlightedText, Qt::white );
    ui->tableView->horizontalHeader()->setPalette(hp);
}

void MainWindow::initScheduling(Ui::MainWindow *ui, GROUPCONFIG &activeRecord)
{
    setupWeeklyDropDown(*ui, activeRecord);
    updateSchedulingGroup(*ui, activeRecord);

    QValidator *validator = new QIntValidator(0, 31, this);
    ui->IB_DayOfMonthChosen->setValidator(validator);
    ui->IB_DayOfMonthChosen->setAlignment(Qt::AlignCenter);
    ui->IB_DayOfMonthChosen->setToolTip("Enter a day between 1 and 31. Use '31' for last day of month");
    ui->IB_DayOfMonthChosen->setText(QString::number(activeRecord.groupSchedule.monthDayChosen));
}

void MainWindow::initReporting(QList<USERINFO> &allUsers, QStringList &allEligibleEmails, QStringList &allChosenEmails)
{
    for (int i = 0; i < allUsers.size(); i++){
        allEligibleEmails.append(allUsers.at(i).emailAddress);
    }

    stringListModelAll = new QStringListModel();
    stringListModelIncl = new QStringListModel();
    stringListModelAll->setStringList(allEligibleEmails);
    stringListModelIncl->setStringList(allChosenEmails);
    ui->CV_RegisteredUsers->setModel(stringListModelAll);
    ui->CV_ReportRecipients->setModel(stringListModelIncl);
    QList<int> colWidths;
    colWidths.append(ui->CV_RegisteredUsers->geometry().width() - 3);
    colWidths.append(0);
    ui->CV_RegisteredUsers->setColumnWidths(colWidths);
    ui->CV_ReportRecipients->setColumnWidths(colWidths);
    ui->CV_RegisteredUsers->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->CV_ReportRecipients->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->CV_RegisteredUsers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->CV_ReportRecipients->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->CV_RegisteredUsers->setResizeGripsVisible(false);
    ui->CV_ReportRecipients->setResizeGripsVisible(false);
}

void MainWindow::groupSelected(const QItemSelection &rowSelected, const QItemSelection &rowDeselected)
{
    Q_UNUSED(rowSelected);
    Q_UNUSED(rowDeselected);
    QModelIndex selection;
    QVariant data;

    for (int col = 0; col < 13; col++)
    {
        selection = ui->tableView->selectionModel()->selectedRows(col).first();

        if (!selection.isValid())
            return;

        if (selection.row() < 0)
            return;

        data = ui->tableView->model()->data(selection);
        switch (col)
        {
        case 0:
            activeRecord.key = data.toString();
            break;
        case 1:
            activeRecord.clientCode = data.toInt();
            break;
        case 2:
            activeRecord.groupName = data.toString();
            break;
        case 3:
            activeRecord.lastRun = data.toDateTime();
            break;
        case 4:
            activeRecord.nextRun = data.toDateTime();
            break;
        case 5:
            activeRecord.lastRunString = data.toString();
            break;
        case 6:
            activeRecord.nextRunString = data.toString();
            break;
        case 7:
            activeRecord.groupSchedule.freqOptions = static_cast<FREQOPTIONS>(data.toInt());
            break;
        case 8:
            activeRecord.groupSchedule.weeklyDay = static_cast<DAYSOFWEEK>(data.toInt());
            break;
        case 9:
            activeRecord.groupSchedule.customDaysChosen = data.toInt();
            break;
        case 10:
            activeRecord.groupSchedule.monthDayChosen = data.toInt();
            break;
        case 11:
            activeRecord.emailRecipients = data.toString();
            break;
        case 12:
            activeRecord.groupMatchesIncluded = data.toInt();
            break;
        default:
            break;
        }
    }

    updateSchedulingGroup(*ui, activeRecord);
    updateReportingGroup(*ui, activeRecord);
}

void MainWindow::updateModel()
{
    if (ui->tableView->selectionModel()->hasSelection())
    {
        QModelIndex currentSelection = ui->tableView->selectionModel()->selectedRows(0).first();
        int row = currentSelection.row();
        if ((row >= 0) && (row < ui->tableView->model()->rowCount()))
            m_model->updateGroupRecord(currentSelection, getActiveRecord());
    }
}

void MainWindow::setActiveRecord(GROUPCONFIG &ar)
{
    activeRecord = ar;
}

GROUPCONFIG MainWindow::getActiveRecord() const
{
    return activeRecord;
}

void MainWindow::setupWeeklyDropDown(Ui::MainWindow &ui, GROUPCONFIG &activeRecord)
{
    ui.DD_WeeklyChosen->insertItem(0, "Sunday");
    ui.DD_WeeklyChosen->insertItem(1, "Monday");
    ui.DD_WeeklyChosen->insertItem(2, "Tuesday");
    ui.DD_WeeklyChosen->insertItem(3, "Wednesday");
    ui.DD_WeeklyChosen->insertItem(4, "Thursday");
    ui.DD_WeeklyChosen->insertItem(5, "Friday");
    ui.DD_WeeklyChosen->insertItem(6, "Saturday");

    ui.DD_WeeklyChosen->setCurrentIndex(activeRecord.groupSchedule.weeklyDay - 1);
}

void MainWindow::setupDayPicks(Ui::MainWindow &ui, GROUPCONFIG &activeRecord)
{
    if ((inclSunday & activeRecord.groupSchedule.customDaysChosen) == inclSunday)
        ui.CB_Sunday->setCheckState(Qt::Checked);
    if ((inclMonday & activeRecord.groupSchedule.customDaysChosen) == inclMonday)
        ui.CB_Monday->setCheckState(Qt::Checked);
    if ((inclTuesday & activeRecord.groupSchedule.customDaysChosen) == inclTuesday)
        ui.CB_Tuesday->setCheckState(Qt::Checked);
    if ((inclWednesday & activeRecord.groupSchedule.customDaysChosen) == inclWednesday)
        ui.CB_Wednesday->setCheckState(Qt::Checked);
    if ((inclThursday & activeRecord.groupSchedule.customDaysChosen) == inclThursday)
        ui.CB_Thursday->setCheckState(Qt::Checked);
    if ((inclFriday & activeRecord.groupSchedule.customDaysChosen) == inclFriday)
        ui.CB_Friday->setCheckState(Qt::Checked);
    if ((inclSaturday & activeRecord.groupSchedule.customDaysChosen) == inclSaturday)
        ui.CB_Saturday->setCheckState(Qt::Checked);
}

void MainWindow::updateSchedulingGroup(Ui::MainWindow &ui, GROUPCONFIG &activeRecord)
{
    setupDayPicks(ui, activeRecord);

    switch(activeRecord.groupSchedule.freqOptions)
    {
    case Disabled:
        ui.RB_Disabled->setChecked(true);
        break;

    case Daily:
        ui.RB_Daily->setChecked(true);
        break;

    case WeekDays:
        ui.RB_Weekdays->setChecked(true);
        break;

    case Weekly:
        ui.RB_Weekly->setChecked(true);
        break;

    case BiWeekly:
        ui.RB_BiWeekly->setChecked(true);
        break;

    case Monthly:
        ui.RB_Monthly->setChecked(true);
        break;

    case Custom:
        ui.RB_Custom->setChecked(true);
        break;
    }
}

void MainWindow::updateReportingGroup(Ui::MainWindow &ui, GROUPCONFIG &activeRecord)
{
    QList<QString> selectedRecipients = activeRecord.emailRecipients.split("|");
    QString nameToMove;
    QModelIndexList foundIndices;
    QModelIndex oldIndex;

    for (int i = 0; i < selectedRecipients.size(); i++)
    {
        /*void MyComboBox::setValor(const QVariant& myValue, int column = 0, int role = Qt::DisplayRole, const QModelIndex& parent=QModelIndex())
        {
            if(myValue.isNull()) return;
            const int rowCount= model()->rowCount(parent);
            for(int i=0;i<rowCount;++i){
                const QVariant currVal = model()->index(i,column,parent ).data(role );
                if(currVal.isNull()) continue;
                if(currVal == myValue)
                    return setCurrentIndex(i);
            }
        }*/

        foundIndices = matchColumnValues(*ui.CV_RegisteredUsers, selectedRecipients);
        //foundIndices = ui.CV_RegisteredUsers->model()->match(QModelIndex(), Qt::DisplayRole, QVariant(selectedRecipients.at(i)));
        if (foundIndices.size() > 0)
        {
            oldIndex = foundIndices.first();

            if (oldIndex.isValid())
            {
                nameToMove = ui.CV_RegisteredUsers->model()->data(oldIndex, Qt::DisplayRole).toString();
                ui.CV_RegisteredUsers->model()->removeRow(oldIndex.row());

                int rowCount = ui.CV_ReportRecipients->model()->rowCount();
                ui.CV_ReportRecipients->model()->insertRow(rowCount);
                const QModelIndex newIndex = ui.CV_ReportRecipients->model()->index(rowCount, 0);
                ui.CV_ReportRecipients->model()->setData(newIndex, QVariant(nameToMove), Qt::EditRole);
            }
        }
    }

}

void MainWindow::disableDayPicks(Ui::MainWindow *ui)
{
    ui->CB_Sunday->setDisabled(true);
    ui->CB_Monday->setDisabled(true);
    ui->CB_Tuesday->setDisabled(true);
    ui->CB_Wednesday->setDisabled(true);
    ui->CB_Thursday->setDisabled(true);
    ui->CB_Friday->setDisabled(true);
    ui->CB_Saturday->setDisabled(true);
}

void MainWindow::enableDayPicks(Ui::MainWindow *ui)
{
    ui->CB_Sunday->setDisabled(false);
    ui->CB_Monday->setDisabled(false);
    ui->CB_Tuesday->setDisabled(false);
    ui->CB_Wednesday->setDisabled(false);
    ui->CB_Thursday->setDisabled(false);
    ui->CB_Friday->setDisabled(false);
    ui->CB_Saturday->setDisabled(false);
}

void MainWindow::on_RB_Disabled_clicked()
{
    ui->DD_WeeklyChosen->setDisabled(true);
    ui->IB_DayOfMonthChosen->setDisabled(true);
    disableDayPicks(ui);
    activeRecord.groupSchedule.freqOptions = Disabled;
    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_RB_Daily_clicked()
{
    ui->DD_WeeklyChosen->setDisabled(true);
    ui->IB_DayOfMonthChosen->setDisabled(true);
    disableDayPicks(ui);
    activeRecord.groupSchedule.freqOptions = Daily;
    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_RB_Weekdays_clicked()
{
    ui->DD_WeeklyChosen->setDisabled(true);
    ui->IB_DayOfMonthChosen->setDisabled(true);
    disableDayPicks(ui);
    activeRecord.groupSchedule.freqOptions = WeekDays;
    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_RB_Weekly_clicked()
{
    ui->DD_WeeklyChosen->setDisabled(false);
    ui->IB_DayOfMonthChosen->setDisabled(true);
    disableDayPicks(ui);
    activeRecord.groupSchedule.freqOptions = Weekly;
    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_RB_BiWeekly_clicked()
{
    ui->DD_WeeklyChosen->setDisabled(false);
    ui->IB_DayOfMonthChosen->setDisabled(true);
    disableDayPicks(ui);
    activeRecord.groupSchedule.freqOptions = BiWeekly;
    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_RB_Monthly_clicked()
{
    ui->DD_WeeklyChosen->setDisabled(true);
    ui->IB_DayOfMonthChosen->setDisabled(false);
    disableDayPicks(ui);
    activeRecord.groupSchedule.freqOptions = Monthly;
    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_RB_Custom_clicked()
{
    ui->DD_WeeklyChosen->setDisabled(true);
    ui->IB_DayOfMonthChosen->setDisabled(true);
    enableDayPicks(ui);
    activeRecord.groupSchedule.freqOptions = Custom;
    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_IB_DayOfMonthChosen_textChanged(const QString &arg1)
{
    int initialInput = arg1.toInt();
    if (initialInput < 1)
    {
        initialInput = 1;
        ui->IB_DayOfMonthChosen->setText("1");
    }
    if (initialInput > 31)
    {
        initialInput = 31;
        ui->IB_DayOfMonthChosen->setText("31");
    }
    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_DD_WeeklyChosen_currentIndexChanged(int index)
{
    activeRecord.groupSchedule.weeklyDay = static_cast<DAYSOFWEEK>(index + 1);
    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_PB_Add_clicked()
{
    if (!ui->CV_RegisteredUsers->selectionModel()->hasSelection())
        return;

    QString nameToMove;
    const QModelIndex oldIndex = ui->CV_RegisteredUsers->selectionModel()->selectedRows(0).first();
    nameToMove = ui->CV_RegisteredUsers->model()->data(oldIndex, Qt::DisplayRole).toString();
    ui->CV_RegisteredUsers->model()->removeRow(oldIndex.row());

    int rowCount = ui->CV_ReportRecipients->model()->rowCount();
    ui->CV_ReportRecipients->model()->insertRow(rowCount);
    const QModelIndex newIndex = ui->CV_ReportRecipients->model()->index(rowCount, 0);
    ui->CV_ReportRecipients->model()->setData(newIndex, QVariant(nameToMove), Qt::EditRole);
}

void MainWindow::on_PB_Remove_clicked()
{
    if (!ui->CV_ReportRecipients->selectionModel()->hasSelection())
        return;

    QString nameToMove;
    const QModelIndex oldIndex = ui->CV_ReportRecipients->selectionModel()->selectedRows(0).first();
    nameToMove = ui->CV_ReportRecipients->model()->data(oldIndex, Qt::DisplayRole).toString();
    ui->CV_ReportRecipients->model()->removeRow(oldIndex.row());

    int rowCount = ui->CV_RegisteredUsers->model()->rowCount();
    ui->CV_RegisteredUsers->model()->insertRow(rowCount);
    const QModelIndex newIndex = ui->CV_RegisteredUsers->model()->index(rowCount, 0);
    ui->CV_RegisteredUsers->model()->setData(newIndex, QVariant(nameToMove), Qt::EditRole);
}

QModelIndexList MainWindow::matchColumnValues(QColumnView &cv, QList<QString>& toBeMatched)
{
    QModelIndexList result;
    int numPotentialMatches = toBeMatched.size();
    if (numPotentialMatches == 0)
        return result;

    const QModelIndex& parent=QModelIndex();
    int CVrows = cv.model()->rowCount();

    for (int i = 0; i < CVrows; i++)
    {
        const QModelIndex currentIndex = cv.model()->index(i, 0);
        const QString currValue = cv.model()->index(i, 0).data(Qt::DisplayRole).toString();

        for (int j = 0; j < numPotentialMatches; j++)
        {
            if (currValue == toBeMatched.at(j))
                result.append(currentIndex);
        }
    }

    return result;
}
