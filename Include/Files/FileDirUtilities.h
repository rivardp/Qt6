//	FileDirUtilities.h		Version 1.0		February 8, 2015
//
#ifndef FILEDIRUTILITIES_H
#define FILEDIRUTILITIES_H

#include <Windows.h>
#include "../Penser/PString.h"

bool FileExists(const PString &filename);		// Returns true if file found
bool DirectoryExists(const PString &directory);		// Returns true if directory exists

int  GetLastVersion(const PString &filename, PString &nextVersion, int numExist = 0);
	// Assumes original written as "filename.ext" and subsequent versions as "filename (xx).ext"

#endif


