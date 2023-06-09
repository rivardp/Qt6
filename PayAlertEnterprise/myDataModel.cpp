#include "myDataModel.h"

#include <QDebug>
#include <QTextStream>
#include <QVariant>

//! [0]
myDataModel::myDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}
//! [0]

//! [1]
int myDataModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : groupConfigs.size();
}

int myDataModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 13;
}
//! [1]

//! [2]
QVariant myDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= groupConfigs.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &groupConfig = groupConfigs.at(index.row());

        switch (index.column()) {
        case 0:
            return groupConfig.key;
            break;
        case 1:
            return groupConfig.clientCode;
            break;
        case 2:
            return groupConfig.groupName;
            break;
        case 3:
            return groupConfig.lastRun;
            break;
        case 4:
            return groupConfig.nextRun;
            break;
        case 5:
            return groupConfig.lastRunString;
            break;
        case 6:
            return groupConfig.nextRunString;
            break;
        case 7:
            return groupConfig.groupSchedule.freqOptions;
            break;
        case 8:
            return groupConfig.groupSchedule.weeklyDay;
            break;
        case 9:
            return groupConfig.groupSchedule.customDaysChosen;
            break;
        case 10:
            return groupConfig.groupSchedule.monthDayChosen;
            break;
        case 11:
            return groupConfig.emailRecipients;
            break;
        case 12:
            return groupConfig.groupMatchesIncluded;
            break;
        default:
            break;
        }
    }

    if (role == Qt::TextAlignmentRole){
        if ((index.column() == 5) || (index.column() == 6))
            return int(Qt::AlignRight | Qt::AlignVCenter);
        else
            return int(Qt::AlignLeft | Qt::AlignVCenter);
    }

    return QVariant();
}
//! [2]

//! [3]
QVariant myDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Key");
        case 1:
            return tr("Client Code");
        case 2:
            return tr("Name");
        case 3:
            return tr("Last Run Date");
        case 4:
            return tr("Next Run Date");
        case 5:
            return tr("Last Run Date");
        case 6:
            return tr("Next Run Date");
        case 7:
            return tr("Freq Opt");
        case 8:
            return tr("Weekly Day");
        case 9:
            return tr("Custom Days");
        case 10:
            return tr("Monthly Day");
        case 11:
            return tr("Emails");
        case 12:
            return tr("Reports Incl");
        default:
            break;
        }
    }
    return QVariant();
}
//! [3]

//! [4]
bool myDataModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        groupConfigs.insert(position, { QString(), (int)0, QString(), QDateTime(), QDateTime(), QString(), QString(), {(FREQOPTIONS)0, (DAYSOFWEEK)0, (int)0, (int)0}, QString(), (int)0 });

    endInsertRows();
    return true;
}
//! [4]

//! [5]
bool myDataModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        groupConfigs.removeAt(position);

    endRemoveRows();
    return true;
}
//! [5]

//! [6]
bool myDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        const int row = index.row();
        auto groupConfig = groupConfigs.value(row);
        int temp;

        switch (index.column()) {
        case 0:
            groupConfig.key = value.toString();
        case 1:
            groupConfig.clientCode = value.toUInt();
        case 2:
            groupConfig.groupName = value.toString();
            break;
        case 3:
            groupConfig.lastRun = value.toDateTime();
            break;
        case 4:
            groupConfig.nextRun = value.toDateTime();
            break;
        case 5:
            groupConfig.lastRunString = value.toString();
            break;
        case 6:
            groupConfig.nextRunString = value.toString();
            break;
        case 7:
            groupConfig.groupSchedule.freqOptions = static_cast<FREQOPTIONS>(value.toInt());
            break;
        case 8:
            temp = value.toInt();
            if (temp > 0)
                temp -= 1;
            groupConfig.groupSchedule.weeklyDay  = static_cast<DAYSOFWEEK>(temp);
            break;
        case 9:
            groupConfig.groupSchedule.customDaysChosen = value.toInt();
            break;
        case 10:
            groupConfig.groupSchedule.monthDayChosen = value.toInt();
            break;
        case 11:
            groupConfig.emailRecipients = value.toString();
            break;
        case 12:
            groupConfig.groupMatchesIncluded = value.toInt();
            break;
        default:
            break;
        }

        groupConfigs.replace(row, groupConfig);
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

        return true;
    }

    return false;
}
//! [6]

//! [7]
Qt::ItemFlags myDataModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;

    if(index.column() == 2)
        return QAbstractTableModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    else
        return QAbstractTableModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//! [7]

//! [8]
const QList<GROUPCONFIG> &myDataModel::getAllGroupConfigs() const
{
    return groupConfigs;
}
//! [8]

GROUPCONFIG myDataModel::loadInitialData(const QList<GROUPCONFIG> &groupConfig)
{
    groupConfigs = groupConfig;
    refreshRunDateTimes();

    GROUPCONFIG initialActiveRecord;
    if (groupConfigs.size() > 0)
        initialActiveRecord = groupConfigs.at(0);

    return initialActiveRecord;
}

void myDataModel::updateGroupRecord(const QModelIndex &index, const GROUPCONFIG &groupConfig)
{
    const int row = index.row();
    if (index.isValid())
    {
        groupConfigs.replace(row, groupConfig);
        const QModelIndex firstCol = index.sibling(row, 0);
        const QModelIndex lastCol = index.sibling(row, 12);
        emit dataChanged(firstCol, lastCol, {Qt::DisplayRole, Qt::EditRole});
    }
}

void myDataModel::refreshRunDateTimes()
{
    typedef QList<GROUPCONFIG>::Iterator Iterator;

    Iterator start = groupConfigs.begin();
    Iterator end = groupConfigs.end();
    for (Iterator it = start; it != end; ++it) {
        it->formatData();
        it->determineNextRunDate();
    }
}

