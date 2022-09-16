// XLL Utility.h
// Helper Functions

#include "../PenserXLL/XLW/Include/xlw/xlw.h"
#include "../PenserXLL/XLW/Include/xlw/xlcall32.h"

using namespace xlw;

bool IsNumVector(XlfOper input)
{
    // Function returns "true" if the input is a numeric vector
	
	// Check for a single number
	if (input.IsNumber())
		return true;

    // If not a single number, check for vector of numbers
	if (!input.IsMulti())
		return false;

	// Must be either "1 x c" or "r x 1"
	if ((input.rows() != 1) && (input.columns() != 1))
		return false;

	// Validate individual cells
	bool OK = true;
    for (int i = 0; i < input.rows(); i++)
    {
        for (int j = 0; j < input.columns(); j++)
		{
            OK = OK && input(i, j).IsNumber();
			if (!OK)
				return false;
		}
    }
	return true;
}

// Create function to convert between "Excel dates" and "Penser dates"
// The base Excel date can be either 1900/1/1 or 1904/1/1
// The base Penser date is 1875/1/1
// If 1875/1/1 == 1, then 1900/1/1 == 9131
// If 1875/1/1 == 1, then 2000/1/1 == 45656

unsigned int ExcelDateConversion(double inputDate, int direction)
{
	// direction == 1 is for Excel to Penser
	// direction == -1 is for Penser to Excel
	
	XLOPER12 xResult;
	XLOPER12 xYear, xMonth, xDay;
    unsigned int date20000101, adjustment;

	xYear.xltype = xltypeNum;
	xMonth.xltype = xltypeNum;
	xDay.xltype = xltypeNum;
	xYear.val.num = 2000;
	xMonth.val.num = 1;
	xDay.val.num = 1;

	if (Excel12(xlfDate, &xResult, 3, &xYear, &xMonth, &xDay) == xlretSuccess)
        date20000101 = static_cast<unsigned int>(xResult.val.num);
	else
        date20000101 = static_cast<unsigned int>(36526);  // Assume Excel is set to 1900/01/01 = 1

	adjustment = 9130 + (36526 - date20000101);

    return static_cast<unsigned int>(inputDate + static_cast<unsigned int>(direction) * adjustment);
};
