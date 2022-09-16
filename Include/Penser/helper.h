//	helper.h		Version 1.0		January 10, 2015

//  Helper Functions

#ifndef HELPER_H
#define HELPER_H

#include "PString.h"

BOOL FileExists(PString filename)
{
	LPCTSTR szFile = filename.getString();
	DWORD dwAttrib = GetFileAttributes(szFile);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int GetLastVersion(PString filename, PString &nextVersion, int numExist)
{
	// Assumes original written as "filename.ext" and subsequent versions as "filename (xx).ext"

	// Get extension (assumes length of either 3 or 4)
	int extLength;
	PString period(".");
	PString temp(filename);
	temp.dropRight(3);
	PString checkForPeriod = temp.right(1);
	if (checkForPeriod.isMatch(period))
		extLength = 3;
	else
		extLength = 4;
	PString ext(filename.right(extLength + 1));		// include leading period

	// Get filename without extension
	PString fname(filename.left(filename.getLength() - (extLength + 1)));

	// Build pieces of the full filename and setup necessary variables for search
	int target = numExist + 1;
	PString verNum;
	PString front(" (");
	PString back(")");
	PString checkName;
	bool lastVersionFound = false;

	// Find last version
	while (!lastVersionFound)
	{
		verNum = PString(std::to_string(target));
		checkName = fname + front + verNum + back + ext;
		if (FileExists(checkName))
			target++;
		else
			lastVersionFound = true;
	}
	nextVersion = checkName;
	return target-1;
}



#endif