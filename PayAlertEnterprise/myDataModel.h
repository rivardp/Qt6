
#ifndef MYDATAMODEL_H
#define MYDATAMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QColor>

#include "groupConfig.h"

//! [0]

inline QDataStream &operator<<(QDataStream &stream, const GROUPCONFIG &groupConfig)
{
    stream << groupConfig.key << groupConfig.clientCode << groupConfig.groupName;
    stream << groupConfig.lastRun << groupConfig.nextRun << groupConfig.lastRunString << groupConfig.nextRunString;
    stream << groupConfig.groupSchedule.freqOptions << groupConfig.groupSchedule.weeklyDay << groupConfig.groupSchedule.customDaysChosen << groupConfig.groupSchedule.monthDayChosen;
    stream << groupConfig.emailRecipients;
    stream << groupConfig.groupMatchesIncluded;

    return stream;
}

inline QDataStream &operator>>(QDataStream &stream, GROUPCONFIG &groupConfig)
{
    stream >> groupConfig.key >> groupConfig.clientCode >> groupConfig.groupName;
    stream >> groupConfig.lastRun >> groupConfig.nextRun >> groupConfig.lastRunString >> groupConfig.nextRunString;
    stream >> groupConfig.groupSchedule.freqOptions >> groupConfig.groupSchedule.weeklyDay >> groupConfig.groupSchedule.customDaysChosen >> groupConfig.groupSchedule.monthDayChosen;
    stream >> groupConfig.emailRecipients;
    stream >> groupConfig.groupMatchesIncluded;

    return stream;
}

class myDataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    myDataModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

    GROUPCONFIG loadInitialData(const QList<GROUPCONFIG> &groupConfig);
    void updateGroupRecord(const QModelIndex &index, const GROUPCONFIG &groupConfig);
    void refreshRunDateTimes();
    const QList<GROUPCONFIG> &getAllGroupConfigs() const;
    //bool getActiveRecord(GROUPCONFIG &gc, const QModelIndex &index);
    QStringList getExistingGroupNames();

protected slots:
    void addHoverRow(const QModelIndex &index);
    void removeHoverRow();

private:
    QList<GROUPCONFIG> groupConfigs;
    QList<int> hoverRows;
    QModelIndex savedIndex;
};
//! [0]

#endif // MYDATAMODEL_H
