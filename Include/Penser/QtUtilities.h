// QtUtilities.h

#ifndef QTUTILITIES_H
#define QTUTILITIES_H

#include <QString>
#include <QList>


bool caseSensitiveAlphabetical(const QString &s1, const QString &s2);
bool caseSensitiveReverseAlphabetical(const QString &s1, const QString &s2);
bool caseInsensitiveAlphabetical(const QString &s1, const QString &s2);
bool caseInsensitiveReverseAlphabetical(const QString &s1, const QString &s2);
bool byIncreasingLength(const QString &s1, const QString &s2);
bool byDecreasingLength(const QString &s1, const QString &s2);
void sort(QList<QString> &list, bool ascending = true, bool caseSensitive = true, bool removeDuplicates = true, bool byLength = false);


#endif // QTUTILITIES_H
