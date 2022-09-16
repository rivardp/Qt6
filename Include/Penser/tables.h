//	tables.h		Version 1.0		November 20, 2015

//  Used to create tables to be dumped into POF

#ifndef TABLES_H
#define TABLES_H

#ifndef PSTRING_H
#include "PString.h"
#endif

#ifndef DATE_H
#include "date.h"
#endif

#include <vector>

enum COLUMNTYPE { CT_Unknown = 0, CT_Time, CT_Year, CT_Age, CT_Service, CT_AccruedService, CT_Probability, CT_Interest, 
			      CT_Benefit, CT_AccruedBenefit, CT_Hours, CT_Rate, CT_Date };
enum COLUMNSUMMARY { CS_None = 0, CS_Total, CS_Avg, CS_Last, CS_Min, CS_Max };
enum TABLETYPE { TableT_Unknown = 0, TableT_Generic, TableT_tPx, TableT_Service, TableT_WorkHistory, TableT_Credits };

const unsigned int defaultColumnWidth = 12;
const long double NO_SUMMARY_VALUE = 9999999.99;

/**********************************************************************************/
/*                               COLUMN CLASS                                     */
/**********************************************************************************/

class Column
{

public:
	Column();
	Column(const Column &col);
	Column(Column &&col);
	template<class T> Column(const PString &name, const vector <T> &data, COLUMNTYPE type);
	template<class T> Column(const PString &name, const Array <T> &data, COLUMNTYPE type);
	Column(const PString &name, const Array <date> &data, COLUMNTYPE type);
	~Column();
	
	void setStdColumnSpecs(unsigned int type);
	void setColumnType(unsigned int type);
	void setColumnSummary(unsigned int summary);
	void setColumnWidth(unsigned int width);
	void setColumnDec(int decimals);
	void setFormat(int formats);

	unsigned int getColumnType() const;
	unsigned int getColumnWidth() const;
	unsigned int getNumRows() const;
	int getColumnDec() const;
	const TCHAR * getColumnName() const;
	long double getRowValue(unsigned int row) const;
	long double getSummaryValue();
	unsigned int getFormat() const;

private:
	PString colName;
	std::unique_ptr<vector<long double>> pColData;
	COLUMNTYPE colType;
	COLUMNSUMMARY colSummary;
	TABLETYPE tableType;
	long double summaryValue;
	unsigned int numDataRows;
	unsigned int colWidth;
	unsigned int format;
	int numDec;

};

Column::Column()
{
	std::unique_ptr<vector<long double>> pColData (new vector < long double >);
	setColumnType(CT_Unknown);
	setColumnSummary(CS_None);
	setColumnWidth(defaultColumnWidth);
	setColumnDec(0);
	setFormat(NO_FORMAT);
	numDataRows = 0;
	summaryValue = 0;
}

// Copy constructor where the Column parameter passed is an existing Column object
Column::Column(const Column &col) : colName(col.colName),
									numDataRows(col.numDataRows),
									pColData(new vector < long double >(col.numDataRows)),
									summaryValue(col.summaryValue),
									colType(col.colType),
									colSummary(col.colSummary),
									colWidth(col.colWidth),
									format(col.format),
									numDec(col.numDec)
{
	for (unsigned int i = 0; i < numDataRows; i++)
		(*pColData)[i] = long double((*col.pColData)[i]);
}

// Move constructor where the Column parameter passed is a temporary Column object
Column::Column(Column &&col) : colName(col.colName),
							   numDataRows(col.numDataRows),
							   pColData(std::move(col.pColData)),
							   summaryValue(col.summaryValue),
							   colType(col.colType),
							   colSummary(col.colSummary),
							   colWidth(col.colWidth),
							   format(col.format),
							   tableType(col.tableType),
							   numDec(col.numDec)
{
	col.pColData = NULL;
}

template<class T>
Column::Column(const PString &name, const vector <T> &data, COLUMNTYPE type) : colName(name),
																			   numDataRows(data.getSize()),
																			   pColData(new vector < long double >(data.getSize())),
																			   summaryValue(0)
{
	for (unsigned int i = 0; i < numDataRows; i++)
		(*pColData)[i] = long double(data[i]);
	setStdColumnSpecs(type);
}

template<class T>
Column::Column(const PString &name, const Array <T> &data, COLUMNTYPE type) : colName(name),
																			  numDataRows(data.getSize()),
																			  pColData(new vector < long double >(data.getSize())),
																			  summaryValue(0)
{
	for (unsigned int i = 0; i < numDataRows; i++)
		(*pColData)[i] = long double(data[i]);
	setStdColumnSpecs(type);
}

// Specialized constructor for date column, where data is stored as numeric value
Column::Column(const PString &name, const Array <date> &data, COLUMNTYPE type) : colName(name),
																				 numDataRows(data.getSize()),
																				 pColData(new vector < long double >(data.getSize())),
																				 summaryValue(0)
{
	for (unsigned int i = 0; i < numDataRows; i++)
		(*pColData)[i] = long double(data[i].numericValue());
	setStdColumnSpecs(type);
}

Column::~Column()
{
}

void Column::setStdColumnSpecs(unsigned int type)
{
	switch (type)
	{
	case CT_Time:
		setColumnType(CT_Time);
		setColumnSummary(CS_None);
		setColumnWidth(4);
		setColumnDec(0);
		setFormat(NO_FORMAT);
		break;

	case CT_Year:
		setColumnType(CT_Year);
		setColumnSummary(CS_None);
		setColumnWidth(7);
		setColumnDec(0);
		setFormat(NO_FORMAT);
		break;

	case CT_Age:
		setColumnType(CT_Age);
		setColumnSummary(CS_None);
		setColumnWidth(5);
		setColumnDec(0);
		setFormat(NO_FORMAT);
		break;

	case CT_Service:
		setColumnType(CT_Service);
		setColumnSummary(CS_Total);
		setColumnWidth(9);
		setColumnDec(2);
		setFormat(NO_FORMAT);
		break;

	case CT_AccruedService:
		setColumnType(CT_Service);
		setColumnSummary(CS_Last);
		setColumnWidth(9);
		setColumnDec(2);
		setFormat(NO_FORMAT);
		break;

	case CT_Probability:
		setColumnType(CT_Probability);
		setColumnSummary(CS_None);
		setColumnWidth(8);
		setColumnDec(0);
		setFormat(NO_FORMAT);
		break;

	case CT_Interest:
		setColumnType(CT_Interest);
		setColumnSummary(CS_None);
		setColumnWidth(8);
		setColumnDec(4);
		setFormat(NO_FORMAT);
		break;

	case CT_Benefit:
		setColumnType(CT_Benefit);
		setColumnSummary(CS_Total);
		setColumnWidth(14);
		setColumnDec(2);
		setFormat(COMMA);
		break;

	case CT_AccruedBenefit:
		setColumnType(CT_AccruedBenefit);
		setColumnSummary(CS_Last);
		setColumnWidth(14);
		setColumnDec(2);
		setFormat(COMMA);
		break;

	case CT_Hours:
		setColumnType(CT_Hours);
		setColumnSummary(CS_Total);
		setColumnWidth(12);
		setColumnDec(2);
		setFormat(COMMA);
		break;

	case CT_Rate:
		setColumnType(CT_Rate);
		setColumnSummary(CS_None);
		setColumnWidth(10);
		setColumnDec(2);
		setFormat(NO_FORMAT);
		break;

	case CT_Date:
		setColumnType(CT_Date);
		setColumnSummary(CS_None);
		setColumnWidth(11);
		setColumnDec(0);
		setFormat(DATE | YMD | FORWARD_SLASH);
		break;

	default:
		setColumnType(CT_Unknown);
		setColumnSummary(CS_None);
		setColumnWidth(defaultColumnWidth);
		setColumnDec(2);
		setFormat(NO_FORMAT);
		break;
	}
}

void Column::setColumnType(unsigned int type)
{
	colType = COLUMNTYPE(type);
}

void Column::setColumnSummary(unsigned int summary)
{
	colSummary = COLUMNSUMMARY(summary);
}


void Column::setColumnWidth(unsigned int width)
{
	colWidth = width;
}

void Column::setColumnDec(int decimals)
{
	numDec = decimals;
}

void Column::setFormat(int formats)
{
	format = formats;
}

unsigned int Column::getColumnType() const
{
	return colType;
}

unsigned int Column::getColumnWidth() const
{
	return colWidth;
}

unsigned int Column::getNumRows() const
{
	return numDataRows;
}

int Column::getColumnDec() const
{
	return numDec;
}

const TCHAR * Column::getColumnName() const
{
	return colName.getString();
}

long double Column::getRowValue(unsigned int row) const
{
	return (*pColData)[row <= numDataRows ? row : numDataRows];
}

long double Column::getSummaryValue()
{
	if (numDataRows == 0)
		return NO_SUMMARY_VALUE;

	switch (colSummary)
	{
	case CS_None:
		return NO_SUMMARY_VALUE;
		break;

	case CS_Total:
		return (*pColData).sum();
		break;

	case CS_Avg:
		return (*pColData).sum() / numDataRows;
		break;

	case CS_Last:
		return (*pColData)[numDataRows - 1];
		break;

	case CS_Min:
		return (*pColData).minValue();
		break;

	case CS_Max:
		return (*pColData).maxValue();
		break;

	default:
		return NO_SUMMARY_VALUE;
	}
}

unsigned int Column::getFormat() const
{
	return format;
}

/**********************************************************************************/
/*                               TABLE CLASS                                      */
/**********************************************************************************/

class Table
{
public:
	Table();
	~Table();

	void addColumn(const Column &col);
	void exportTable(std::basic_fstream<TCHAR> &stream, unsigned int inclSummary = 1) const;
	void exportHeadings(std::basic_fstream<TCHAR> &stream) const;
	void exportData(std::basic_fstream<TCHAR> &stream) const;
	void exportSummaries(std::basic_fstream<TCHAR> &stream) const;
	bool isValidData() const; 

	void setTableType(unsigned int type);
	unsigned int getTableType() const;
	
private:
	std::vector<Column*> pColumns;
	TABLETYPE tableType;
	PString tableTag;
};

Table::Table()
{
}

Table::~Table()
{
/*	for (unsigned int i = 0; i < pColumns.size(); ++i) {
		delete pColumns[i]; 
	}
	pColumns.clear();*/
	for (std::vector<Column*>::iterator iter = pColumns.begin(); iter != pColumns.end(); iter++ )
	{
		delete *iter; 
		iter = pColumns.erase(iter);
	}
}

void Table::addColumn(const Column &col)
{
	pColumns.push_back(new Column(col));
}

void Table::exportTable(std::basic_fstream<TCHAR> &stream, unsigned int inclSummary) const
{
	stream << _T("<table>\n");
	std::vector<Column*>::const_iterator iter = pColumns.begin();
	if (pColumns.size() > 0)
	{
		exportHeadings(stream);
		if ((*iter)->getNumRows() > 0)
		{
			stream << _T("   <tbody>\n");
			exportData(stream);
			if (inclSummary)
				exportSummaries(stream);
			stream << _T("   </tbody>\n");
		}
	}
	stream << _T("</table>\n");
}

void Table::exportHeadings(std::basic_fstream<TCHAR> &stream) const
{
	unsigned int colWidth;

	stream << _T("   <thead>\n");
	stream << _T("      <tr>\n");
	for (std::vector<Column*>::const_iterator iter = pColumns.begin(); iter < pColumns.end(); iter++)
	{
		colWidth = (*iter)->getColumnWidth() * 6;  // Estimate of with of column in pixals
		stream << _T("        <th style=\"width:"); 
		stream << ConvertNumberToString(colWidth, 0).getString();
		stream << _T("px\">");
		stream << (*iter)->getColumnName();
		stream << _T("</th>\n");
	}
	stream << _T("      </tr>\n");
	stream << _T("   </thead>\n");
}

void Table::exportData(std::basic_fstream<TCHAR> &stream) const
{
	// Don't process anything if there is a problem with the data
	if (!this->isValidData())
	{
		stream << _T("      <tr><td>Problem with data</td></tr>\n");
		return;
	}

	// Variables needed to handle columns of dates
	errorFlag error;
	date rowDate;

	// Determine if any rows are to be grouped
	unsigned int priorYear = 0;  // Used for work history table to tag new years for formatting
	vector<unsigned int> rowsSelected;
	if (this->tableType == TableT_WorkHistory)
	{
		// Group monthly records by year for years >=1992
		// Date column will always be the first column
		for (unsigned int row = 0; row < pColumns[0]->getNumRows(); row++)
		{
			rowDate = date((unsigned long int)pColumns[0]->getRowValue(row), error);
			if ((rowDate.year() > 1992) && (rowDate.year() != priorYear))
				rowsSelected.add(row);
			priorYear = rowDate.year();
		}
	}
	
	// Start outputting rows
	for (unsigned int row = 0; row < pColumns[0]->getNumRows(); row++)
	{
		if (rowsSelected.isFound(row))
			stream << _T("      <tr class=\"newGroupStarted\">");
		else
			stream << _T("      <tr>");
		
		for (std::vector<Column*>::const_iterator iter = pColumns.begin(); iter < pColumns.end(); iter++)
		{
			stream << _T("<td>");
			if ((*iter)->getColumnType() == CT_Date)
			{
				rowDate = date((unsigned long int)pColumns[0]->getRowValue(row), error);
				stream << rowDate.stringOutput(DATE | YMD | FORWARD_SLASH).getString();
			}
			else
				stream << ConvertNumberToString((*iter)->getRowValue(row), (*iter)->getColumnDec(), (*iter)->getFormat()).getString();
			stream << _T("</td>");
		}
		stream << _T("</tr>\n");
	}
}

void Table::exportSummaries(std::basic_fstream<TCHAR> &stream) const
{
	stream << _T("      <tr class=\"summaryRow\">");
	for (std::vector<Column*>::const_iterator iter = pColumns.begin(); iter < pColumns.end(); iter++)
	{
		stream << _T("<td>");
		if ((*iter)->getSummaryValue() == NO_SUMMARY_VALUE)
			stream << _T(" ");
		else
			stream << ConvertNumberToString((*iter)->getSummaryValue(), (*iter)->getColumnDec(), (*iter)->getFormat()).getString();
		stream << _T("</td>");
	}
	stream << _T("</tr>\n");
}

bool Table::isValidData() const
{
	bool isValid = true;

	// Check if all columns have the same number of rows
	unsigned int firstColnumRows = pColumns[0]->getNumRows();
	for (std::vector<Column*>::const_iterator iter = pColumns.begin(); iter < pColumns.end(); iter++)
	{
		isValid = isValid && ((*iter)->getNumRows() == firstColnumRows);
	}

	return isValid;
}

void Table::setTableType(unsigned int type)
{
	tableType = TABLETYPE(type);
}

unsigned int Table::getTableType() const
{
	return tableType;
}


#endif