#ifndef GROUPCONFIG_H
#define GROUPCONFIG_H

#include <QString>
#include <QDate>
#include <QDateTime>
#include <QTimeZone>

enum DAYSOFWEEK {Sunday = 1, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday};
enum FREQOPTIONS {Disabled, Daily, WeekDays, Weekly, BiWeekly, Monthly, Custom};
enum CONFIDENCE { High = 1, Good = 2, Reasonable = 4, Possible = 8};
enum INCLUDEDDAYS { inclSunday = 1, inclMonday = 2, inclTuesday = 4, inclWednesday = 8, inclThursday = 16, inclFriday = 32, inclSaturday = 64};

struct SCHEDULING {
    FREQOPTIONS freqOptions;
    DAYSOFWEEK weeklyDay;
    int customDaysChosen;
    int monthDayChosen;
};

class GROUPCONFIG
{
public:
    QString key;
    unsigned int clientCode;
    QString groupName;
    QDateTime lastRun;
    QDateTime nextRun;
    QString lastRunString;
    QString nextRunString;
    SCHEDULING groupSchedule;
    QString emailRecipients;
    int groupMatchesIncluded;

    QString name() const;
    void formatData();
    void determineNextRunDate();
    void setDefaultValues();
    GROUPCONFIG createNewGroupConfig(unsigned int clientCode);
    GROUPCONFIG createNewGroupConfig(unsigned int clientCode, QStringList &existingGroupNames);
};

DAYSOFWEEK findNextDay(QDate startDate, int dayChosen);
DAYSOFWEEK findNextIncludedDay(QDate startDate, int daysChosen);
int enumValue(int dayOfWeek);

class USERINFO
{
public:
    QString key;
    unsigned int clientCode;
    QString emailAddress;
    QString adminRights;
    QString name;
    QString password;
};

#endif // GROUPCONFIG_H
