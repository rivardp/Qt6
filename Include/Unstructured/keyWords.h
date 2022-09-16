// keyWords.h
//
// Words are also used as source to determine language of website
// Words are ordered in expected frequency for efficiency

#ifndef WORDS_TO_IGNORE_H
#define WORDS_TO_IGNORE_H

#include <string>
#include <tchar.h>

static const std::wstring ignoreWordsEnglish1[] = { _T("a") };
static const std::wstring ignoreWordsEnglish2[] = { _T("in"), _T("of"), _T("be"), _T("we"), _T("it"), _T("is"), _T("to"), _T("on"), _T("at"), _T("by") };
static const std::wstring ignoreWordsEnglish3[] = { _T("are"), _T("sad"), _T("our"), _T("the"), _T("say"), _T("and"), _T("was"), _T("age"), _T("son"), _T("god") };
static const std::wstring ignoreWordsEnglish4[] = { _T("will"), _T("with"), _T("wife"), _T("that"), _T("died"), _T("left"), _T("born"), _T("lost"), _T("home"), _T("side"),
													_T("once"), _T("gift"), _T("year")};
static const std::wstring ignoreWordsEnglish5[] = { _T("great"), _T("after"), _T("heavy"), _T("death"), _T("while"), _T("years") };
static const std::wstring ignoreWordsEnglish6[] = { _T("loving"), _T("memory"), _T("missed"), _T("hearts"), _T("father"), _T("mother"), _T("sister"), _T("friend"),
													_T("monday"), _T("friday"), _T("sunday"), _T("battle"), _T("heaven"), _T("family"), _T("sorrow"), _T("passed") };
static const std::wstring ignoreWordsEnglish7[] = { _T("beloved"), _T("brother"), _T("husband"), _T("sadness"), _T("illness"), _T("goodbye"), _T("tuesday"), _T("passing"),
													_T("crossed"), _T("hospice") };
static const std::wstring ignoreWordsEnglish8[] = { _T("suddenly"), _T("memoriam"), _T("announce"), _T("lovingly"), _T("peaceful"), _T("thursday"), _T("saturday"),
													_T("daughter"), _T("hospital") };
static const std::wstring ignoreWordsEnglish9[] = { _T("wednesday"), _T("departure") };
static const std::wstring ignoreWordsEnglish10[] = { _T("peacefully"), _T("courageous") };
static const std::wstring ignoreWordsEnglish11[] = { _T("remembrance"), _T("remembering"), _T("grandfather"), _T("grandmother"), _T("celebration"), _T("brotherhood") };
static const std::wstring ignoreWordsEnglish12[] = { _T("unexpectedly") };
static const std::wstring ignoreWordsEnglish13[] = { _T("international"), _T("unexpectedly") };
static const std::wstring ignoreWordsEnglish14[] = { _T("xxxxxxxxxxxxxx") };

static const std::wstring ignoreWordsFrench1[] = { _T("dummy") };
static const std::wstring ignoreWordsFrench2[] = { _T("le") };
static const std::wstring ignoreWordsFrench3[] = { _T("xxx"), _T("xxx") };
static const std::wstring ignoreWordsFrench4[] = { _T("dummy") };
static const std::wstring ignoreWordsFrench5[] = { _T("dummy") };
static const std::wstring ignoreWordsFrench6[] = { _T("dummy") };
static const std::wstring ignoreWordsFrench7[] = { _T("dummy") };
static const std::wstring ignoreWordsFrench8[] = { _T("dummy") };
static const std::wstring ignoreWordsFrench9[] = { _T("dummy") };
static const std::wstring ignoreWordsFrench10[] = { _T("dummy") };
static const std::wstring ignoreWordsFrench11[] = { _T("dummy") };
static const std::wstring ignoreWordsFrench12[] = { _T("dummy") };
static const std::wstring ignoreWordsFrench13[] = { _T("international") };
static const std::wstring ignoreWordsFrench14[] = { _T("internationale") };

static const std::wstring ignoreWordsSpanish1[] = { _T("dummy") };
static const std::wstring ignoreWordsSpanish2[] = { _T("spanishdummy") };
static const std::wstring ignoreWordsSpanish3[] = { _T("dummy") };
static const std::wstring ignoreWordsSpanish4[] = { _T("dummy") };
static const std::wstring ignoreWordsSpanish5[] = { _T("dummy") };
static const std::wstring ignoreWordsSpanish6[] = { _T("dummy") };
static const std::wstring ignoreWordsSpanish7[] = { _T("dummy") };
static const std::wstring ignoreWordsSpanish8[] = { _T("dummy") };
static const std::wstring ignoreWordsSpanish9[] = { _T("dummy") };
static const std::wstring ignoreWordsSpanish10[] = { _T("dummy") };
static const std::wstring ignoreWordsSpanish11[] = { _T("dummy") };
static const std::wstring ignoreWordsSpanish12[] = { _T("dummy") };
static const std::wstring ignoreWordsSpanish13[] = { _T("international") };
static const std::wstring ignoreWordsSpanish14[] = { _T("internationale") };

static const std::wstring genderWordsEnglishM[] = { _T("he"), _T("his") };
static const std::wstring genderWordsEnglishF[] = { _T("she"), _T("her") };
static const std::wstring genderWordsFrenchM[] = { _T("il"), _T("son") };
static const std::wstring genderWordsFrenchF[] = { _T("elle"), _T("sa") };
static const std::wstring genderWordsSpanishM[] = { _T("el"), _T("él"), _T("su") };
static const std::wstring genderWordsSpanishF[] = { _T("ella"), _T("su") };

static const std::wstring uncapitalizedNames[] = { _T("de"), _T("la"), _T("di"), _T("van"), _T("der"), _T("st"), _T("st.") };

// The words below are repeats of ignoreWords above
static const std::wstring deathWords[] = { _T("passed"), _T("passing"), _T("died"), _T("death"), _T("left"), _T("crossed"), _T("lost"), _T("age"), _T("departure"), _T("announce") };
static const std::wstring birthWords[] = { _T("born") };
static const std::wstring ageWords[] = { _T("age"), _T("years"), _T("year") };


#endif