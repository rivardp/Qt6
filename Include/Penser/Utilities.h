//	Utilities.h		Version 1.0		January 11, 2009
//

// Converts a boolean into "Yes" or "No"

TCHAR* YesNo(const bool YES)
{
	TCHAR *result;
	if (YES)
		result = _T("    Yes");
	else
		result = _T("    No");
	return result;
}


// Reads the contents of an edit box and converts it into a PString

PString GetUserStringInput(const HWND hEB)
{
	unsigned int iLength;
	unsigned int minLength;		// Required to ensure you can enter size of buffer
	TCHAR *pUserInput = NULL;
	PString result;

	iLength = (unsigned int) SendMessage(hEB, EM_LINELENGTH, 0, 0);
	if (iLength > 0)
	{
		minLength = (iLength > 3) ? iLength : 4;
		pUserInput = (TCHAR*) calloc(minLength + 1, sizeof(TCHAR));
		*((LPWORD)pUserInput) = (WORD) minLength;
		SendMessage(hEB, EM_GETLINE, 0, (LPARAM) pUserInput);
		pUserInput[iLength] = '\0';
		result = pUserInput;
		free (pUserInput);
		pUserInput = NULL;
	}
	return result;
}

// Reads the contents of an edit box and converts it into a number

double GetUserNumberInput(const HWND hEB)
{
	PString stringNum = GetUserStringInput(hEB);
	return stringNum.asNumber();
}

// Reads in the setting of a checkbox and returns a bool
bool GetCheckBoxInput(const HWND hCB)
{
	if (SendMessage(hCB, BM_GETCHECK, 0, 0))
		return TRUE;
	else
		return FALSE;
}



