//	FileDirAfterGlobal.h		Version 1.0		February 11, 2015
//
//	All of these functions rely on global variable definitions before the header

//  Get install directory - Uses GetCurrentDirectory from Windows
//							Assumes current directory == install directory
//                          Requires "hInst"

PString GetInstallDir()
{
	const DWORD	bufferSize = MAX_PATH;
	TCHAR	buffer[bufferSize+1];

	GetModuleFileName(hInst, buffer, MAX_PATH);
	PString directory(buffer);
	
	// Drop filename
	unsigned int positionLastSlash = directory.findPosition(_T("\\"), -1, 0, 1);
	unsigned int drop = directory.getLength() - positionLastSlash + 1;
	directory.dropRight(drop);
	
	if (directory.right(5) == _T("Debug"))		// Drops "\Debug" if applicable
		directory.dropRight(6);
	if (directory.right(7) == _T("Release"))	// Drops "\Release" if applicable
		directory.dropRight(8);

	return directory;
}

