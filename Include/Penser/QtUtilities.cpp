// QtUtilities.cpp

#include "QtUtilities.h"

void sort(QList<QString> &list, bool ascending, bool caseSensitive, bool removeDuplicates, bool byLength)
{
    if (byLength)
    {
        if (ascending)
            std::sort(list.begin(), list.end(), byIncreasingLength);
        else
            std::sort(list.begin(), list.end(), byDecreasingLength);
    }
    else
    {
        if (caseSensitive)
        {
            if (ascending)
                std::sort(list.begin(), list.end(), caseSensitiveAlphabetical);
            else
                std::sort(list.begin(), list.end(), caseSensitiveReverseAlphabetical);
        }
        else
        {
            if (ascending)
                std::sort(list.begin(), list.end(), caseInsensitiveAlphabetical);
            else
                std::sort(list.begin(), list.end(), caseInsensitiveReverseAlphabetical);
        }

        if (removeDuplicates)
        {
            int i = 0;
            while (i < list.size() - 1)
            {
                if (caseSensitive)
                {
                    if (list.at(i).compare(list.at(i+1),Qt::CaseSensitive) == 0)
                        list.removeAt(i+1);
                    else
                        i++;
                }
                else
                {
                    if (list.at(i).compare(list.at(i+1),Qt::CaseInsensitive) == 0)
                        list.removeAt(i+1);
                    else
                        i++;
                }
            }
        }
    }
}

bool caseSensitiveAlphabetical(const QString &s1, const QString &s2)
{
    return s1 < s2;
}

bool caseSensitiveReverseAlphabetical(const QString &s1, const QString &s2)
{
    return s1 > s2;
}

bool caseInsensitiveAlphabetical(const QString &s1, const QString &s2)
{
    return s1.toLower() < s2.toLower();
}

bool caseInsensitiveReverseAlphabetical(const QString &s1, const QString &s2)
{
    return s1.toLower() > s2.toLower();
}

bool byIncreasingLength(const QString &s1, const QString &s2)
{
    return s1.length() < s2.length();
}

bool byDecreasingLength(const QString &s1, const QString &s2)
{
    return s1.length() > s2.length();
}
