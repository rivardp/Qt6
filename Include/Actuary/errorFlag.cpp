// errorflag.cpp
//				
#include "errorflag.h"

errorFlag::errorFlag(): errorNum(0)
{
}

errorFlag::operator unsigned int()
{
	return errorNum;
}

errorFlag& errorFlag::operator=(const unsigned int ErrorNum)
{
    errorNum = ErrorNum;
	return *this;
}

errorFlag::~errorFlag()
{
}

TCHAR* errorFlag::getErrorDesc()
{
    errorDesc = new TCHAR(60);

    switch(errorNum)
    {
    case 0:
        _tcscpy_s(errorDesc, 61, _T("No error exists"));
        break;
    case 1:
        _tcscpy_s(errorDesc, 61, _T("Interest/Indexing rate cannot be < 0%"));
        break;
    case 2:
        _tcscpy_s(errorDesc, 61, _T("Interest/Indexing rate cannot be > 100%"));
        break;
    case 3:
        _tcscpy_s(errorDesc, 61, _T("Interest/Indexing PVector elements not odd"));
        break;
    case 4:
        _tcscpy_s(errorDesc, 61, _T("Interest/Indexing period cannot be < 0"));
        break;
    case 5:
        _tcscpy_s(errorDesc, 61, _T("Invalid interest/indexing rate parameter"));
        break;
    case 6:
        _tcscpy_s(errorDesc, 61, _T("Invalid period parameter"));
        break;
    case 7:
        _tcscpy_s(errorDesc, 61, _T("Invalid payment period parameter"));
        break;
    case 10:
        _tcscpy_s(errorDesc, 61, _T("Period cannot be < 0"));
        break;
    case 11:
        _tcscpy_s(errorDesc, 61, _T("Payment period cannot be < 0"));
        break;
    case 12:
        _tcscpy_s(errorDesc, 61, _T("Deferral period cannot be < 0"));
        break;
    case 13:
        _tcscpy_s(errorDesc, 61, _T("Invalid deferral parameter"));
        break;
    case 14:
        _tcscpy_s(errorDesc, 61, _T("Invalid guarantee parameter"));
        break;
    case 15:
        _tcscpy_s(errorDesc, 61, _T("Invalid payment frequency parameter"));
        break;
    case 16:
        _tcscpy_s(errorDesc, 61, _T("Invalid interest frequency parameter"));
        break;
    case 20:
        _tcscpy_s(errorDesc, 61, _T("Invalid sex parameter"));
        break;
    case 24:
        _tcscpy_s(errorDesc, 61, _T("Invalid joint percentage"));
        break;
    case 25:
        _tcscpy_s(errorDesc, 61, _T("Joint percentage cannot be < 0"));
        break;
    case 26:
        _tcscpy_s(errorDesc, 61, _T("Joint percentage cannot be > 500%"));
        break;
    case 27:
        _tcscpy_s(errorDesc, 61, _T("Invalid reduction option on death"));
        break;
    case 30:
        _tcscpy_s(errorDesc, 61, _T("Invalid mortality table"));
        break;
    case 31:
        _tcscpy_s(errorDesc, 61, _T("Final Qx in user defined table <> 1"));
        break;
    case 32:
        _tcscpy_s(errorDesc, 61, _T("Invalid user defined mortality table"));
        break;
    case 33:
        _tcscpy_s(errorDesc, 61, _T("Invalid mortality projection"));
        break;
    case 34:
        _tcscpy_s(errorDesc, 61, _T("Generational mortality inputs not set"));
        break;
    case 35:
        _tcscpy_s(errorDesc, 61, _T("Age less than table minimum"));
        break;
    case 36:
        _tcscpy_s(errorDesc, 61, _T("Age greater than table maximum"));
        break;
    case 37:
        _tcscpy_s(errorDesc, 61, _T("Invalid age parameter"));
        break;
    case 38:
        _tcscpy_s(errorDesc, 61, _T("Invalid generational mortality date"));
        break;
    case 39:
        _tcscpy_s(errorDesc, 61, _T("Generational mortality requires a calculation date"));
        break;
    case 40:
        _tcscpy_s(errorDesc, 61, _T("Year cannot be less than 1875"));
        break;
    case 41:
        _tcscpy_s(errorDesc, 61, _T("Year cannot be greater than 2250"));
        break;
    case 42:
        _tcscpy_s(errorDesc, 61, _T("Invalid date"));
        break;
    case 43:
        _tcscpy_s(errorDesc, 61, _T("Invalid date rounding option"));
        break;
    case 44:
        _tcscpy_s(errorDesc, 61, _T("Invalid eligibility requirement"));
        break;
    case 45:
        _tcscpy_s(errorDesc, 61, _T("Invalid increment parameter"));
        break;
    case 46:
        _tcscpy_s(errorDesc, 61, _T("DOH/DOC must be >= DOB"));
        break;
    case 47:
        _tcscpy_s(errorDesc, 61, _T("Invalid calculation date"));
        break;
    case 50:
        _tcscpy_s(errorDesc, 61, _T("Guarantee period cannot be < 0"));
        break;
    case 51:
        _tcscpy_s(errorDesc, 61, _T("Year cannot be less than 2000"));
        break;
    case 52:
        _tcscpy_s(errorDesc, 61, _T("Projection year cannot be less than base year"));
        break;
    case 53:
        _tcscpy_s(errorDesc, 61, _T("Invalid rounding option"));
        break;

    default:
        _tcscpy_s(errorDesc, 61, _T("Unidentified error"));
        break;
    }


    _tcscpy_s(errorDesc, 61, _T("No "));

    return errorDesc;
}
