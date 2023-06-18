
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
    allUsers = getAllUsers(clientCode);
    GROUPCONFIG initialActiveRecord = m_model->loadInitialData(getGroupConfigs(clientCode));
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

    connect(ui->PB_Exit, SIGNAL(clicked()), qApp, SLOT(close()));

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
    ui->IB_DayOfMonthChosen->setEnabled(false);
    ui->CB_Sunday->setEnabled(false);
    ui->CB_Monday->setEnabled(false);
    ui->CB_Tuesday->setEnabled(false);
    ui->CB_Wednesday->setEnabled(false);
    ui->CB_Thursday->setEnabled(false);
    ui->CB_Friday->setEnabled(false);
    ui->CB_Saturday->setEnabled(false);
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
    int numRowsSelected;
    QModelIndex selection;
    QVariant data;

    if (rowDeselected.size() > 0)
    {
        // Update data modal
        // Table View is updated on the fly or indirectly through scheduling view

        // Update scheduling view
        activeRecord.determineNextRunDate();

        // Update reporting view
        const QModelIndex priorEmailSelectionIndex = ui->tableView->model()->index(rowDeselected.indexes().first().row(), 11);
        ui->tableView->model()->setData(priorEmailSelectionIndex, QVariant(activeRecord.emailRecipients), Qt::EditRole);
    }

    numRowsSelected = ui->tableView->selectionModel()->selectedRows(0).size();
    if (numRowsSelected > 0)
    {
        // Populate window with current selection
        // Populate table view
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

        ui->sourceFile->setText(CSVfileDirectory.path() + QString("/") + activeRecord.groupName + QString(".csv"));

        // Populate scheduling view
        updateSchedulingGroup(*ui, activeRecord);

        // Populate reporting view
        updateReportingGroup(*ui, activeRecord);
    }
    else
    {
        ui->sourceFile->setText("");
        activeRecord = GROUPCONFIG();

        // Populate scheduling view
        updateSchedulingGroup(*ui, activeRecord);

        // Populate reporting view
        updateReportingGroup(*ui, activeRecord);
    }
}

void MainWindow::on_PB_DeleteGroup_clicked()
{
    if (ui->tableView->selectionModel()->hasSelection())
    {
        QModelIndex currentSelection = ui->tableView->selectionModel()->selectedRows(0).first();
        int row = currentSelection.row();
        if ((row >= 0) && (row < ui->tableView->model()->rowCount()))
            ui->tableView->model()->removeRow(row);
    }
}

void MainWindow::on_PB_AddGroup_clicked()
{
    ui->tableView->model()->insertRow(0);
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

void MainWindow::updateModel(QModelIndex &indexToUpdate)
{
    m_model->updateGroupRecord(indexToUpdate, getActiveRecord());
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
    ui.DD_WeeklyChosen->addItem("Sunday");
    ui.DD_WeeklyChosen->addItem("Monday");
    ui.DD_WeeklyChosen->addItem("Tuesday");
    ui.DD_WeeklyChosen->addItem("Wednesday");
    ui.DD_WeeklyChosen->addItem("Thursday");
    ui.DD_WeeklyChosen->addItem("Friday");
    ui.DD_WeeklyChosen->addItem("Saturday");

    ui.DD_WeeklyChosen->setCurrentIndex(static_cast<int>(activeRecord.groupSchedule.weeklyDay) - 1);
    ui.DD_WeeklyChosen->setEnabled(false);
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
    ui.DD_WeeklyChosen->setCurrentIndex(static_cast<int>(activeRecord.groupSchedule.weeklyDay) - 1);
    ui.IB_DayOfMonthChosen->setText(QString::number(activeRecord.groupSchedule.monthDayChosen));

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

    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::updateReportingGroup(Ui::MainWindow &ui, GROUPCONFIG &activeRecord)
{
    QList<QString> selectedRecipients = activeRecord.emailRecipients.split("|");
    QString nameToMove;
    QModelIndexList foundIndices;
    QModelIndex oldIndex;
    bool putOnLeftSide, onLeftSide;

    // Deal with Recipients
    for (int i = 0; i < allUsers.size(); i++)
    {
        putOnLeftSide = !selectedRecipients.contains(allUsers.at(i).emailAddress);
        foundIndices = matchColumnValues(*ui.CV_RegisteredUsers, allUsers.at(i).emailAddress);
        onLeftSide = foundIndices.size() > 0;

        if (onLeftSide)
        {
            if (putOnLeftSide)
            {
                // Nothing to do
            }
            else
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
        else
        {
            // on right side
            if (putOnLeftSide)
            {
                // Move to left side
                foundIndices = matchColumnValues(*ui.CV_ReportRecipients, allUsers.at(i).emailAddress);
                oldIndex = foundIndices.first();

                if (oldIndex.isValid())
                {
                    nameToMove = ui.CV_ReportRecipients->model()->data(oldIndex, Qt::DisplayRole).toString();
                    ui.CV_ReportRecipients->model()->removeRow(oldIndex.row());

                    int rowCount = ui.CV_RegisteredUsers->model()->rowCount();
                    ui.CV_RegisteredUsers->model()->insertRow(rowCount);
                    const QModelIndex newIndex = ui.CV_RegisteredUsers->model()->index(rowCount, 0);
                    ui.CV_RegisteredUsers->model()->setData(newIndex, QVariant(nameToMove), Qt::EditRole);
                }
            }
            else
            {
                // Nothing to do
            }
        }
    }

    // Deal with match types
    if ((activeRecord.groupMatchesIncluded & static_cast<int>(High)) == static_cast<int>(High))
        ui.CB_High->setCheckState(Qt::Checked);
    else
        ui.CB_High->setCheckState(Qt::Unchecked);

    if ((activeRecord.groupMatchesIncluded & static_cast<int>(Good)) == static_cast<int>(Good))
        ui.CB_Good->setCheckState(Qt::Checked);
    else
        ui.CB_Good->setCheckState(Qt::Unchecked);

    if ((activeRecord.groupMatchesIncluded & static_cast<int>(Reasonable)) == static_cast<int>(Reasonable))
        ui.CB_Reasonable->setCheckState(Qt::Checked);
    else
        ui.CB_Reasonable->setCheckState(Qt::Unchecked);

    if ((activeRecord.groupMatchesIncluded & static_cast<int>(Possible)) == static_cast<int>(Possible))
        ui.CB_Possible->setCheckState(Qt::Checked);
    else
        ui.CB_Possible->setCheckState(Qt::Unchecked);
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
    if (arg1.length() == 0)
        return;

    int input = arg1.toInt();
    if (input < 1)
        input = 1;
    if (input > 31)
        input = 31;
    ui->IB_DayOfMonthChosen->setText(QString::number(input));
    activeRecord.groupSchedule.monthDayChosen = input;
    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_DD_WeeklyChosen_currentIndexChanged(int index)
{
    activeRecord.groupSchedule.weeklyDay = static_cast<DAYSOFWEEK>(index + 1);
    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_CB_Sunday_clicked()
{
    if (ui->CB_Sunday->isChecked())
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen | inclSunday;
    else
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen & ~inclSunday;

    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_CB_Monday_clicked()
{
    if (ui->CB_Monday->isChecked())
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen | inclMonday;
    else
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen & ~inclMonday;

    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_CB_Tuesday_clicked()
{
    if (ui->CB_Tuesday->isChecked())
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen | inclTuesday;
    else
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen & ~inclTuesday;

    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_CB_Wednesday_clicked()
{
    if (ui->CB_Wednesday->isChecked())
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen | inclWednesday;
    else
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen & ~inclWednesday;

    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_CB_Thursday_clicked()
{
    if (ui->CB_Thursday->isChecked())
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen | inclThursday;
    else
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen & ~inclThursday;

    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_CB_Friday_clicked()
{
    if (ui->CB_Friday->isChecked())
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen | inclFriday;
    else
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen & ~inclFriday;

    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_CB_Saturday_clicked()
{
    if (ui->CB_Saturday->isChecked())
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen | inclSaturday;
    else
        activeRecord.groupSchedule.customDaysChosen = activeRecord.groupSchedule.customDaysChosen & ~inclSaturday;

    activeRecord.determineNextRunDate();
    updateModel();
}

void MainWindow::on_CB_High_clicked()
{
    if (ui->CB_High->isChecked())
        activeRecord.groupMatchesIncluded = activeRecord.groupMatchesIncluded | High;
    else
        activeRecord.groupMatchesIncluded = activeRecord.groupMatchesIncluded & ~High;

    updateModel();
}

void MainWindow::on_CB_Good_clicked()
{
    if (ui->CB_Good->isChecked())
        activeRecord.groupMatchesIncluded = activeRecord.groupMatchesIncluded | Good;
    else
        activeRecord.groupMatchesIncluded = activeRecord.groupMatchesIncluded & ~Good;

    updateModel();
}

void MainWindow::on_CB_Reasonable_clicked()
{
    if (ui->CB_Reasonable->isChecked())
        activeRecord.groupMatchesIncluded = activeRecord.groupMatchesIncluded | Reasonable;
    else
        activeRecord.groupMatchesIncluded = activeRecord.groupMatchesIncluded & ~Reasonable;

    updateModel();
}

void MainWindow::on_CB_Possible_clicked()
{
    if (ui->CB_Possible->isChecked())
        activeRecord.groupMatchesIncluded = activeRecord.groupMatchesIncluded | Possible;
    else
        activeRecord.groupMatchesIncluded = activeRecord.groupMatchesIncluded & ~Possible;

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

    activeRecord.emailRecipients.append(QString("|") + nameToMove);
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

    activeRecord.emailRecipients.remove(QString("|") + nameToMove);
    activeRecord.emailRecipients.remove(nameToMove);
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

QModelIndexList MainWindow::matchColumnValues(QColumnView &cv, QString toBeMatched)
{
    QList<QString> qListToBeMatched;
    qListToBeMatched.append(toBeMatched);

    return matchColumnValues(cv, qListToBeMatched);
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "PayAlert",
                                                               tr("Are you sure? All unsaved changes will be lost permanently.\n"),
                                                               QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                               QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::on_PB_Exit_clicked()
{
    close();
}

void MainWindow::on_PB_SaveAndExit_clicked()
{
    save();
    close();
}

void MainWindow::on_PB_Save_clicked()
{
    save();
}

void MainWindow::save()
{
    // Trigger update of model
    const QModelIndex selection = ui->tableView->selectionModel()->selectedRows(0).first();
    QItemSelection itemSelected;
    itemSelected.select(selection, selection);
    groupSelected(itemSelected, itemSelected);

    saveGroupConfigs(m_model->getAllGroupConfigs());
}


