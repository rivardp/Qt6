// tableLookup.cpp

#include "tableLookup.h"

bool fieldNames::setupLookupTable()
{
	inFile = std::wifstream ("TableLookupValues.txt", std::ios::in);
	if (!inFile.is_open())
		return false;

	std::wstring word;
	int matched;
	bool OK = true;

	while (!inFile.eof())
	{
		std::getline(inFile, word);
		if (word.size() > 0)
			numFields++;
	}

	tableRow = new TableRow[numFields];

	inFile.clear();
	inFile.seekg(0, std::ios::beg);

	for (unsigned int i = 0; i < numFields; i++)
	{
		std::getline(inFile, tableRow[i].fieldname, L' ');
		std::getline(inFile, word);

		matched = word.compare(L"firstName");
		if (matched == 0)
		{
			tableRow[i].rowValue = tlFirstName;
		}
		else
		{
			matched = word.compare(L"familyName");
			if (matched == 0)
			{
				tableRow[i].rowValue = tlFamilyName;
			}
			else
			{
				matched = word.compare(L"fullName");
				if (matched == 0)
				{
					tableRow[i].rowValue = tlFullName;
				}
				else
				{
					matched = word.compare(L"dateOfBirth");
					if (matched == 0)
					{
						tableRow[i].rowValue = tlDOB;
					}
					else
					{
						matched = word.compare(L"dateOfDeath");
						if (matched == 0)
						{
							tableRow[i].rowValue = tlDOD;
						}
						else
						{
							matched = word.compare(L"city");
							if (matched == 0)
							{
								tableRow[i].rowValue = tlCity;
							}
							else
							{
								matched = word.compare(L"province");
								if (matched == 0)
								{
									tableRow[i].rowValue = tlProvince;
								}
								else
								{
									matched = word.compare(L"country");
									if (matched == 0)
									{
										tableRow[i].rowValue = tlCountry;
									}
									else
									{
										matched = word.compare(L"publisher.name");
									    if (matched == 0)
									    {
										    tableRow[i].rowValue = tlPublisherName;
									    }
									    else
									    {
										    matched = word.compare(L"publisher.city");
										    if (matched == 0)
										    {
											    tableRow[i].rowValue = tlPublisherCity;
										    }
										    else
										    {
											    matched = word.compare(L"publisher.province");
											    if (matched == 0)
											    {
												    tableRow[i].rowValue = tlPublisherProvince;
											    }
											    else
											    {
												    matched = word.compare(L"publisher.country");
												    if (matched == 0)
												    {
													    tableRow[i].rowValue = tlPublisherCountry;
												    }
												    else
												    {
													    matched = word.compare(L"spouseName");
														if (matched == 0)
														{
															tableRow[i].rowValue = tlSpouseName;
														}
														else
														{
															matched = word.compare(L"dthCert.causeOfDeath");
															if (matched == 0)
															{
																tableRow[i].rowValue = tlCauseOfDeath;
															}
															else
																OK = false;
														}
													}
											    }
										    }
									    }
                                    }
								}
							}
						}
					}
				}
			}
		}
	}
	return OK;
}

int fieldNames::find(const std::wstring &target) const
{
	int result;

    bool matched = false;
	bool passed = false;
	unsigned int i = 0;
	unsigned int compareResult;

	while (!matched && !passed && (i < numFields))
	{
		compareResult = target.compare(tableRow[i].fieldname);
		if (compareResult == 0)
		{
			matched = true;
			result = i;
		}
		else
		{
			if (compareResult < 0)
				passed = true;
		}
		i++;
	}

	if (matched)
		return tableRow[result].rowValue;
	else
		return -1;
}
