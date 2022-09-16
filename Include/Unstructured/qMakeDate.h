// qMakeDate.h

#ifndef MAKEDATE_H
#define MAKEDATE_H

#include <QDate>

enum DATEORDER { doNULL = 0, doYMD, doDMY, doMDY, doMD20Y };
enum CONVICTION { cvNONE = 0, cvLOW, cvPOSSIBLE, cvLIKELY, cvHIGH };

class MAKEDATE{

public:

	MAKEDATE();
    MAKEDATE(int num1, int num2, int num3, int num4, int num5, int num6, QDate date);
	~MAKEDATE();

    QDate finalDate;
    QDate finalDate2;
	bool isValid;
	bool isFinal;
	bool analyzeFurther;
	DATEORDER format;

    QDate potentialDateYMD;
    QDate potentialDateDMY;
    QDate potentialDateMDY;

	CONVICTION conviction;

	unsigned int totalStored;

	void countChar();
	void store(unsigned int tot);
	void nextNum();
	unsigned int getNumChar(unsigned int number) const;
    void analyzeNumbers(DATEORDER dateOrder = doNULL);
	void validatePotentials(CONVICTION max);
	void revisitAssuming(DATEORDER dateOrder);

    void setToday(QDate &date);

private:

	unsigned int num1;
	unsigned int num2;
	unsigned int num3;

	unsigned int numChar1;
	unsigned int numChar2;
	unsigned int numChar3;

	bool potentialYMD;
	bool potentialDMY;
	bool potentialMDY;

	unsigned int currentNumber;

    static QDate today;

};

#endif
