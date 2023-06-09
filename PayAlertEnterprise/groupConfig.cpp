#include "groupConfig.h"


QString GROUPCONFIG::name() const
{
    return key;
}

void GROUPCONFIG::formatData()
{
    if (lastRun.isValid())
        lastRunString = lastRun.toString("dddd, MMMM d, yyyy");
}

void GROUPCONFIG::determineNextRunDate()
{
    QDateTime currentDateTime, nextRunTime;
    QDate nextRunDate, lastDayCurrentMonth, lastDayNextMonth;
    DAYSOFWEEK nextRunDayOfWeek;
    int dayNum, daysToAdd;

    currentDateTime.setTimeZone(QTimeZone("UTC-05:00"));
    nextRunTime.setTimeZone(QTimeZone("UTC-05:00"));
    currentDateTime = QDateTime::currentDateTime();
    nextRunTime = QDateTime(QDate().currentDate(),QTime(5,0,0));
    if (nextRunTime < currentDateTime)
        nextRunTime = nextRunTime.addDays(1);
    nextRunDate = QDate(nextRunTime.date());

    switch(groupSchedule.freqOptions)
    {
    case Disabled:
        nextRun = QDateTime();
        nextRunString.clear();
        break;

    case Daily:
        nextRun = nextRunTime;
        break;

    case WeekDays:
        nextRun = nextRunTime;
        dayNum = (nextRunDate.dayOfWeek() % 7) + 1;
        nextRunDayOfWeek = findNextDay(nextRunDate, int(inclMonday|inclTuesday|inclWednesday|inclThursday|inclFriday));
        daysToAdd = (dayNum + 7 - static_cast<int>(nextRunDayOfWeek)) % 7;
        nextRun = nextRun.addDays(daysToAdd);
        break;

    case Weekly:
    case BiWeekly:
        nextRun = nextRunTime;
        dayNum = (nextRunDate.dayOfWeek() % 7) + 1;
        nextRunDayOfWeek = findNextDay(nextRunDate, groupSchedule.weeklyDay);
        daysToAdd = (dayNum + 7 - static_cast<int>(nextRunDayOfWeek)) % 7;
        nextRun = nextRun.addDays(daysToAdd);
        if ((groupSchedule.freqOptions == BiWeekly) && (lastRun.daysTo(nextRun) <= 7))
            nextRun = nextRun.addDays(7);
        break;

    case Custom:
        nextRun = nextRunTime;
        dayNum = (nextRunDate.dayOfWeek() % 7) + 1;
        nextRunDayOfWeek = findNextDay(nextRunDate, groupSchedule.customDaysChosen);
        daysToAdd = (dayNum + 7 - static_cast<int>(nextRunDayOfWeek)) % 7;
        nextRun = nextRun.addDays(daysToAdd);
        break;

    case Monthly:
        nextRun = nextRunTime;
        lastDayCurrentMonth = QDate(nextRunDate.year(), nextRunDate.month() + 1, 1).addDays(-1);
        lastDayNextMonth = QDate(nextRunDate.year(), nextRunDate.month() + 2, 1).addDays(-1);
        if (groupSchedule.monthDayChosen == 31)
            daysToAdd = lastDayCurrentMonth.day() - nextRunDate.day();
        else
        {
            if (groupSchedule.monthDayChosen >= nextRunDate.day())
            {
                if (groupSchedule.monthDayChosen <= lastDayCurrentMonth.day())
                    daysToAdd = groupSchedule.monthDayChosen - nextRunDate.day();
                else
                    daysToAdd = lastDayCurrentMonth.day() - nextRunDate.day();
            }
            else
            {
                if (groupSchedule.monthDayChosen <= lastDayNextMonth.day())
                    daysToAdd = QDate(nextRunDate.year(), nextRunDate.month(), groupSchedule.monthDayChosen).addMonths(1).toJulianDay() - nextRunDate.toJulianDay();
                else
                    daysToAdd = lastDayNextMonth.day() - nextRunDate.day();
            }
        }
        nextRun = nextRun.addDays(daysToAdd);
        break;

    default:
        nextRun = nextRunTime;
        dayNum = (nextRunDate.dayOfWeek() % 7) + 1;
        nextRunDayOfWeek = static_cast<DAYSOFWEEK>(dayNum);
        daysToAdd = (dayNum + 7 - static_cast<int>(nextRunDayOfWeek)) % 7;
        nextRun = nextRun.addDays(daysToAdd);
        break;
    }

    if (nextRun.isValid())
        nextRunString = nextRun.toString("dddd, MMMM d, yyyy");
}

void GROUPCONFIG::setDefaultValues()
{
    groupSchedule.freqOptions = Disabled;
    groupSchedule.weeklyDay = Monday;
    groupSchedule.monthDayChosen = 31;
    groupSchedule.customDaysChosen = inclMonday|inclTuesday|inclWednesday|inclThursday|inclFriday;
    groupMatchesIncluded = High|Good|Reasonable|Possible;
}

DAYSOFWEEK findNextDay(QDate startDate, int daysChosen)
{
    if (daysChosen == 0)
        return Monday;

    bool found = false;
    int dayOfWeek = (startDate.dayOfWeek() % 7) + 1;
    int dayValue;

    while (!found)
    {
        dayValue = enumValue(dayOfWeek);
        if ((dayValue & daysChosen) == dayValue)
            found = true;
        else
            dayOfWeek = (dayOfWeek + 1) % 7;
    }

    return static_cast<DAYSOFWEEK>(dayOfWeek);
}

int enumValue(int dayOfWeek)
{
    int result = 1;
    for (int i = 1; i < dayOfWeek; i++){
        result = result * 2;
    }

    return result;
}

