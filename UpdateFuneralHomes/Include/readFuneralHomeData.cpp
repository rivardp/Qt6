// readFuneralHomeData.cpp

#include "readFuneralHomeData.h"

void readFuneralHomeData(HTTPDownloader &downloader, std::vector<PString*> &pURLaddresses)
{

	PString URLaddress, URLaddressTemplate, URLFile;
//	PROVIDER provider;
	SourceFile initialWebfile, sourceFile;
	char* tempWebFileName("tempWebPage.htm");
	PString *newURLaddress;
	std::wstring webAddress;

	PString numberOfFuneralHomes, province;
	unsigned int numFuneralHomes, numProvinces;

	bool success, keepGoing;
	unsigned int page;
	unsigned int entriesPerPage;
	unsigned int numRead, numReadThisPage;

	/***************************/
	/*    LEGACY               */
	/***************************/

	// Initial data gathering query, one province at a time
	numProvinces = sizeof(LegacyProvinces) / sizeof(std::wstring);
	URLaddressTemplate = _T("http://www.legacy.com/funeral-homes/directory/search?City=&StateName=%p%&page=%p%");

	for (unsigned int i = 0; i < numProvinces; i++)
	{
		province = LegacyProvinces[i];
		page = 1;
		createURLaddress(URLaddress, URLaddressTemplate, URLparam(province), URLparam(page));
		downloader.download(URLaddress.getStdString(), tempWebFileName);
		initialWebfile = PString(tempWebFileName);
		success = initialWebfile.consecutiveMovesTo(50, _T("Results for"), _T("of "));
		if (success)
		{
			numberOfFuneralHomes = initialWebfile.readNextWord();
			numFuneralHomes = (unsigned int)numberOfFuneralHomes.extractFirstNumber();

			entriesPerPage = 10;
			numRead = 0;
			numReadThisPage = 0;
			keepGoing = true;

			while ((numRead < numFuneralHomes) && keepGoing)
			{
				keepGoing = initialWebfile.moveTo(_T("fhname"));
				if (keepGoing)
				{
					keepGoing = initialWebfile.moveTo(_T("href"));
					if (keepGoing)
					{
						webAddress = unQuoteHTML(initialWebfile.readNextBetween(QUOTES));
						newURLaddress = new PString(webAddress);
						pURLaddresses.push_back(newURLaddress);
						numRead++;
						numReadThisPage++;
						if ((numReadThisPage == entriesPerPage) && (numRead < numFuneralHomes))
						{
							initialWebfile.close();
							numReadThisPage = 0;
							page++;
							createURLaddress(URLaddress, URLaddressTemplate, URLparam(province), URLparam(page));
							downloader.download(URLaddress.getStdString(), tempWebFileName);
							initialWebfile = PString(tempWebFileName);
						}
					}
				}
			} // end while looping within a province
			initialWebfile.close();
		}
	} // end of for loop iterating through all provinces

}  // end of funeral homes
