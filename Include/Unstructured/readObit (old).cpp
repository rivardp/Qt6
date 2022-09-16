// readObit.cpp

#include "../Include/Unstructured/readObit.h"

readObit::readObit() : style(0)
{
}

readObit::~readObit()
{
}

void readObit::read()
{
    bool SetStyle, SetLanguage, ObitTitle, ObitDates, ObitText, ReadStructured, ReadUnstructured, TitleInfo, LookupFields;
    bool StructuredInfoFullyCredible;
    PQString errMsg;
    godReference = false;

    // General strategy is to start with low hanging fruit and get more involved if necessary. Steps are as follows:
    // 1.  Read in title
    // 2.  Read the unstructured obit to determine language and gender if possible
    // 3.  Read in any predefined fields
    // 4.  Read in any structured data
    // 5.  Read in any information contained in headers or titles
    // 6.  Finish with read of unstructured obit

    switch (globals->globalDr->getProvider())
    {
    case Legacy:
        SetStyle = false;
        SetLanguage = false;
        ObitTitle = false;
        ObitDates = false;
        ObitText = false;
        LookupFields = false;
        ReadStructured = false;
        StructuredInfoFullyCredible = true;
        TitleInfo = false;
        ReadUnstructured = false;
        /*
        // No fields for DOB or DOD -> pull out of title

        // TO DO - SQL call to retrieve appropriate search parameters
        searchParam.searchType = deceasedData;
        searchParam.provider = Provider;
        searchParam.tableLookupValue = _T("addPageTarget");
        searchParam.tableLookupDelimiter = QUOTES;
        searchParam.tagType = _T("meta");
        searchParam.tagID = _T("property");
        searchParam.tagContents = _T("og:url");

        // Initial name search to bookend unstructured searches
        // Not required here

        // Update search parameters to start pulling content
        searchParam.tagType = _T("meta");
        searchParam.tagID = _T("name");
        searchParam.tagContents = _T("description");

        success = readLookupFields(searchParam, fn, tempRecord, true);
        success = readUnstructuredContent(searchParam, tempRecord, gcLANGUAGE | gcGENDER | gcDATES | gcALTERNATES | gcREMAINING, true);
        if (success)
        {
            tempRecord.xport(outputCSV);
            std::wcout << tempRecord.getLastName().getString() << _T(", ") << tempRecord.getFirstName().getString() << std::endl;
        }*/
        break;

    case Passages:
        SetStyle = false;
        SetLanguage = false;
        ObitTitle = false;
        ObitDates = false;
        ObitText = false;
        LookupFields = false;
        ReadStructured = false;
        StructuredInfoFullyCredible = true;
        TitleInfo = false;
        ReadUnstructured = false;
        /*
        // Need to read in lastname and first name through a search
        // Look for Born:  and  Date of Passing: after names processed but prior to Pass1 dates

        // Get URL
        URL = this->getSource();
        if (URL.getLength() > 0)
            tempRecord.setURL(URL);
        else;
        //  If commented out, no URL in download file
        //	success = readURL(searchParam, tempRecord, true);


        // TO DO - SQL call to retrieve appropriate search parameters
        searchParam.searchType = deceasedData;
        searchParam.provider = Provider;
        searchParam.tagType = _T("meta");
        searchParam.tagID = _T("name");
        searchParam.tagContents = _T("description");

//		success = readLookupFields(searchParam, fn, tempRecord, true);  NOT REQUIRED FOR THIS PROVIDER
        success = readUnstructuredContent(searchParam, tempRecord, gcLANGUAGE | gcGENDER | gcDATES | gcALTERNATES | gcREMAINING, true);
        if (success)
        {
            tempRecord.xport(outputCSV);
            std::wcout << tempRecord.getLastName().getString() << _T(", ") << tempRecord.getFirstName().getString() << std::endl;
        }*/
        break;

    case LifeMoments:
        SetStyle = false;
        SetLanguage = false;
        ObitTitle = false;
        ObitDates = false;
        ObitText = false;
        LookupFields = false;
        ReadStructured = false;
        StructuredInfoFullyCredible = true;
        TitleInfo = false;
        ReadUnstructured = false;
        /*
        // Need to read in lastname and first name through a search

        // Get URL
        URL = this->getSource();
        if (URL.getLength() > 0)
            tempRecord.setURL(URL);
        else;
        //	success = readURL(searchParam, tempRecord, true);

        // TO DO - SQL call to retrieve appropriate search parameters
        searchParam.searchType = deceasedData;
        searchParam.provider = Provider;
        searchParam.tagType = _T("meta");
        searchParam.tagID = _T("name");
        searchParam.tagContents = _T("description");

        // Read any lookup fields that are available (if any)
        //		success = readLookupFields(searchParam, fn, tempRecord, true);  NOT REQUIRED FOR THIS PROVIDER

        // Read unstructured content - searchParam contains the default parameters should the coded search in the function fail
        success = readUnstructuredContent(searchParam, tempRecord, gcLANGUAGE | gcGENDER | gcDATES | gcALTERNATES | gcREMAINING, true);
        if (success)
        {
            tempRecord.xport(outputCSV);
            std::wcout << tempRecord.getLastName().getString() << _T(", ") << tempRecord.getFirstName().getString() << std::endl;
        }*/
        break;
               
    case SIDS:
        SetStyle = false;
        SetLanguage = false;
        ObitTitle = false;
        ObitDates = false;
        ObitText = false;
        LookupFields = false;
        ReadStructured = false;
        StructuredInfoFullyCredible = true;
        TitleInfo = false;
        ReadUnstructured = false;
        break;
        
    case Batesville:
        // Name, DOB and DOD available in structured form

        SetStyle = true;
        SetLanguage = false;
        ObitTitle = true;
        ObitDates = false;
        ObitText = true;
        LookupFields = false;
        ReadStructured = true;
        StructuredInfoFullyCredible = true;
        TitleInfo = false;
        ReadUnstructured = true;
        break;

    case BrowseAll:
        // Name, DOB and DOD available in structured form

        SetStyle = false;
        SetLanguage = false;
        ObitTitle = true;
        ObitDates = false;
        ObitText = true;
        LookupFields = false;
        ReadStructured = true;
        StructuredInfoFullyCredible = true;
        TitleInfo = false;
        ReadUnstructured = true;
        break;

    case DignityMemorial:
        // DOB and DOD should be included within the *.obitList file most of the time
        SetStyle = false;
        SetLanguage = true;
        ObitTitle = true;
        ObitDates = true;
        ObitText = true;
        LookupFields = false;
        ReadStructured = true;
        StructuredInfoFullyCredible = true;
        TitleInfo = false;
        ReadUnstructured = true;
        break;

    case ConnellyMcKinley:
        // DOB and DOD should be included within the *.obitList file most of the time
        SetStyle = false;
        SetLanguage = true;
        ObitTitle = true;
        ObitDates = false;
        ObitText = true;
        LookupFields = false;
        ReadStructured = true;
        StructuredInfoFullyCredible = true;
        TitleInfo = false;
        ReadUnstructured = true;
        break;

    case CFS:
        // Name, DOB and DOD available in structured form

        SetStyle = true;
        SetLanguage = false;
        ObitTitle = true;
        ObitDates = false;
        ObitText = true;
        LookupFields = false;
        ReadStructured = true;
        StructuredInfoFullyCredible = true;
        TitleInfo = false;
        ReadUnstructured = true;
        break;

    case Frazer:
        // DOB and DOD should be included within the *.obitList file most of the time
        SetStyle = false;
        SetLanguage = false;
        ObitTitle = true;
        ObitDates = false;
        ObitText = true;
        LookupFields = false;
        ReadStructured = true;
        StructuredInfoFullyCredible = true;
        TitleInfo = false;
        ReadUnstructured = true;
        break;

    case Alternatives:
        // DOD should be included within the *.obitList file most of the time
        SetStyle = false;
        SetLanguage = false;
        ObitTitle = true;
        ObitDates = false;
        ObitText = true;
        LookupFields = false;
        ReadStructured = true;
        StructuredInfoFullyCredible = false;
        TitleInfo = false;
        ReadUnstructured = true;
        break;

    case FuneralTech:
        // DOD should be included within the *.obitList file most of the time
        SetStyle = false;
        SetLanguage = false;
        ObitTitle = true;
        ObitDates = false;
        ObitText = true;
        LookupFields = false;
        ReadStructured = true;
        StructuredInfoFullyCredible = false;
        TitleInfo = false;
        ReadUnstructured = true;
        break;

    case WordPress:
        // DOD should be included within the *.obitList file most of the time
        SetStyle = false;
        SetLanguage = false;
        ObitTitle = true;
        ObitDates = false;
        ObitText = true;
        LookupFields = false;
        ReadStructured = true;
        StructuredInfoFullyCredible = false;
        TitleInfo = false;
        ReadUnstructured = true;
        break;

    case FrontRunner:
        // DOB and DOD should be included within the *.obitList file most of the time
        SetStyle = true;
        SetLanguage = false;
        ObitTitle = true;
        ObitDates = false;
        ObitText = true;
        LookupFields = false;
        ReadStructured = true;
        StructuredInfoFullyCredible = false;
        TitleInfo = false;
        ReadUnstructured = true;
        break;

    default:
        // Can only get here due to an oversight
        errMsg << QString("Read Obit switch criteria not set up for provider: ") << globals->globalDr->getProvider();
        globals->logMsg(ErrorRunTime, errMsg);

        SetStyle = false;
        SetLanguage = false;
        ObitTitle = false;
        ObitDates = false;
        ObitText = false;
        LookupFields = false;
        ReadStructured = false;
        StructuredInfoFullyCredible = true;
        TitleInfo = false;
        ReadUnstructured = false;
        break;

    } // end of switch by provider

    // Execute based on settings
    if (SetStyle)
        readStyle();
    if (SetLanguage)
        readLanguage();
    if (ObitTitle)
        readInObitTitle();
    if (ObitDates)
        readInObitDates();
    if (ObitText)
        readInObitText();
    if (1)
        determineLanguageAndGender();  // Also creates justInitialNamesUC
//    if (LookupFields)
//        readLookupFields();
    if (ReadStructured)
        readStructuredContent();
    if (TitleInfo)
        readTitleInfo();
    if ((globals->globalDr->getGender() != Male) && !globals->globalDr->getNeeEtAlEncountered()
                                                 && (globals->globalDr->getMaidenNames().size() == 0))
        readParentsLastName();
    if (ReadUnstructured)
         readUnstructuredContent();

}

void readObit::readLanguage()
{
    // Language is unknown at this stage
    LANGUAGE language = language_unknown;
    OQString langText;

    beg();

    switch(globals->globalDr->getProvider())
    {
    case Legacy:
        break;

    case Passages:
        break;

    case LifeMoments:
        break;

    case SIDS:
        break;

    case Batesville:
    case BrowseAll:
        break;

    case DignityMemorial:
        if (consecutiveMovesTo(20, "og:locale", "content="))
        {
            langText = readNextBetween(DOUBLE_QUOTES);
            langText = langText.right(2);

            if (langText == OQString("en"))
                language = english;
            else
            {
                if (langText == OQString("fr"))
                    language = french;
                else
                {
                    if (langText == OQString("es"))
                        language = spanish;
                }
            }
        }
        break;

    case ConnellyMcKinley:
        break;

    default:
        PQString errMsg;
        errMsg << "Reading in of obit language field not coded for: " << globals->globalDr->getProvider();
        globals->logMsg(ErrorRunTime, errMsg);

    }

    // Common processing for all providers
    if (language != language_unknown)
         globals->globalDr->setLanguage(language);
}


void readObit::readInObitText()
{
    // Basic read specified for each provider, with standard processing for all coded at end

    beg();
    uc.clear();
    unstructuredContent checkContent;

    QString target;

    switch(globals->globalDr->getProvider())
    {
    case Legacy:
        break;

    case Passages:
        break;

    case LifeMoments:
        break;

    case Batesville:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(10, "s:272 (273) liqpLiquid -->", "<p>"))
                uc = getUntil("<style>", 5000, false);

            // Do backup read if necessary (jpeg or no content)
            if (uc.getLength() == 0)
            {
                beg();
                if (consecutiveMovesTo(10, "s:272 (951) liqpLiquid -->", "<p>"))
                    uc = getUntil("<style>", 5000, false);

                // Do second backup read if necessary (jpeg or no content)
                if (uc.getLength() == 0)
                {
                    beg();
                    if (consecutiveMovesTo(20, "og:description", "content="))
                        uc = readNextBetween(DOUBLE_QUOTES);
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(150, "article class=\"obituary\"", "section id=\"obit-text\"", ">"))
                uc = getUntil("</section>", 5000, false);

            // Do backup read if necessary (jpeg or no content)
            if (uc.getLength() == 0)
            {
                beg();
                if (consecutiveMovesTo(20, "og:description", "content="))
                    uc = readNextBetween(DOUBLE_QUOTES);
            }
            break;

        }
        break;

    case SIDS:
        break;

    case BrowseAll:
        if (consecutiveMovesTo(50, "div class=\"content\"", ">"))
        {
            switch(globals->globalDr->getProviderKey())
            {
            case 12538:
                uc = getUntil("</p></div>");
                break;

            case 13823:
            case 14031:
            default:
                uc = getUntil("<P></a>");
                break;
            }
        }
        break;

    case DignityMemorial:
        if (moveTo("class=\"row\""))
        {
            if (consecutiveMovesTo(3000, ">OBITUARY<", "class=\"text-center mt-30\"", ">"))
                uc = getUntil("</div>");
            else
            {
                beg();
                if (consecutiveMovesTo(50, "short-bio text", ">"))
                    uc = getUntil("</div>");
            }
        }
        break;

    case ConnellyMcKinley:
        if (moveTo("<h1>Obituary</h1>"))
        {
            uc = getUntil("</div>");
            uc.simplify();
        }
        break;

    case CFS:
        if (globals->globalDr->getProviderKey() > 90000)
        {
            // Old style obit
            target = QString("id=\"obituary\"");
            if (consecutiveMovesTo(400, target, ">"))
            {
                uc = getUntil("<script");
                uc.simplify();
                uc.removeHTMLtags();
            }
        }
        else
        {
            // New style obit

            switch(style)
            {
                case 0:
                    target = QString("id=\"obituary\"");
                    break;

                case 1:
                    target = QString("id=\"obtext\"");
                    break;
            }

            if (consecutiveMovesTo(50, target, ">"))
            {
                unsigned int position = getPosition();
                bool found = true;
                while (moveTo("<w:WordDocument>", 2500))
                {
                    found = moveTo("</style>");
                    if (found)
                        position = getPosition();
                }
                beg();
                forward(position);
                uc = getUntil("div class=\"row\"", 10000);
            }
            uc.simplify();

            // Verify if a backup read should be attempted
            checkContent = uc;
            checkContent.removeHTMLtags();
            if (checkContent.getLength() < 10)
            {
                beg();
                if (consecutiveMovesTo(50, target, ">"))
                {
                    moveTo("</div>");
                    uc = getUntil("</div>");
                    uc.simplify();
                }
            }
        }

        break;

    case Frazer:
        if (consecutiveMovesTo(100, "id=\"descr\"", "itemprop=\"description\">"))
        {
            uc = getUntil("</p>");
            uc.simplify();
        }
        break;

    case Alternatives:
        checkContent = getString();
        checkContent.simplify();

        if (checkContent.consecutiveMovesTo(2000, "class=\"page-title obit-title\"", "</div> </div>"))
        {
            if(checkContent.conditionalMoveTo("<img", "</div>", 0))
                checkContent.moveTo("</div>");
            uc = checkContent.getUntil("name=\"messages:</h3>");
        }
        break;

    case FuneralTech:
        if (consecutiveMovesTo(300, "obituary-text", "Obituary of", "</h1>"))
        {
            uc = getUntil("</div>");
            uc.simplify();
        }
        break;

    case WordPress:

        if (moveTo("article id=\"post-"))
        {
            if (consecutiveMovesTo(1500, "meta", "class=\"entry-content\">"))
            {
                uc = getUntil("<!-- .entry-content -->");
                uc.simplify();
            }
        }
        break;

    case FrontRunner:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(1000, "bom-obituary-wrapper", "<div>"))
            {
                uc = getUntil("</div>");
                uc.simplify();
            }
            else
            {
                beg();
                if (consecutiveMovesTo(100, "og:description", "content="))
                {
                    uc = readNextBetween(QUOTES);
                    uc.simplify();
                }
            }

            break;

        case 1:
            if (consecutiveMovesTo(2000, "class=\"obituary-title\"", "<div>"))
            {
                uc = getUntil("</div>");
                uc.simplify();
            }
            break;

        case 2:
            if (moveTo(">Obituary<"))
            {
                if (moveTo("class=\"read-more-content\">"))
                {
                    uc = getUntil("</div>");
                    uc.simplify();
                }
            }
        }
        break;
    }

    default:
        PQString errMsg;
        errMsg << "Reading in of obit text not coded for: " << globals->globalDr->getProvider();
        globals->logMsg(ErrorRunTime, errMsg);

    }

    // Common processing
    uc.runStdProcessing(blockedContent, mcn.listOfFirstWords);

}

void readObit::readInObitTitle()
{
    // Language is unknown at this stage
    OQStream titleStream;
    OQString titleText;

    beg();

    switch(globals->globalDr->getProvider())
    {
    case Legacy:
        break;

    case Passages:
        break;

    case LifeMoments:
        break;

    case SIDS:
        break;

    case Batesville:
    case BrowseAll:
    case CFS:
    case Frazer:
    case Alternatives:
    case FuneralTech:
    case WordPress:
    case FrontRunner:
        if (moveTo("<title>"))
            titleStream = getUntil("</title>");
        break;

    case DignityMemorial:
        if (consecutiveMovesTo(20, "og:title", "content="))
            titleStream = readNextBetween(DOUBLE_QUOTES);
        break;

    case ConnellyMcKinley:
        if (moveTo("\"obit_name\">"))
        {
            titleStream = getUntil("</td>");
            titleStream.removeHTMLtags(false);
            titleStream.simplify();
        }
        break;

    default:
        PQString errMsg;
        errMsg << "Reading in of obit title not coded for: " << globals->globalDr->getProvider();
        globals->logMsg(ErrorRunTime, errMsg);

    }

    // Common processing for all providers
    if (titleStream.getLength() > 0)
    {
        if (titleStream.fixQuotes())
        {
            PQString errMsg;
            errMsg << "Potential issue with mismatched quotes for: " << globals->globalDr->getURL();
            globals->logMsg(ErrorRecord, errMsg);
        }
        titleStream.fixBasicErrors();
        titleStream.cleanUpEnds();
        globals->globalDr->setTitle(titleStream.getString());

        titleText = titleStream.getCleanWord();
        while (!titleStream.isEOS() && ((titleText.getLength() == 0) || titleText.isRecognized()))
        {
            titleText = titleStream.getCleanWord();
        }

        if ((titleText.getLength() > 0) && !titleText.isRecognized())
            globals->globalDr->setTitleKey(titleText);
    }
}

void readObit::readInObitDates()
{
    // Language is unknown at this stage

    PQString errMsg;
    OQString datesText;
    unstructuredContent ucDates;
    DATES dates;
    QDate currentDOB, currentDOD;

    beg();

    switch(globals->globalDr->getProvider())
    {
    case Legacy:
        break;

    case Passages:
        break;

    case LifeMoments:
        break;

    case SIDS:
        break;

    case Batesville:
    case BrowseAll:
        break;

    case DignityMemorial:
        break;

    case ConnellyMcKinley:
        if (consecutiveMovesTo(40, "\"Copyright\"", "<br>"))
        {
            datesText = getUntil("</td>");
            datesText.simplify();
            ucDates = datesText;
            dates = ucDates.readDates(globals->globalDr->getLanguage());
        }
        break;

    case CFS:
        break;

    default:
        errMsg << "Reading in of obit title not coded for: " << globals->globalDr->getProvider();
        globals->logMsg(ErrorRunTime, errMsg);

    }

    // Common processing for all providers
    // Deal with DOB
    if (dates.potentialDOB.isValid())
    {
        currentDOB = globals->globalDr->getDOB();
        if (!currentDOB.isValid())
            globals->globalDr->setDOB(dates.potentialDOB);
        else
        {
            if (currentDOB != dates.potentialDOB)
            {
                globals->globalDr->setDOB(dates.potentialDOB);
                errMsg << "Read in inconsistent DOBs for: " << globals->globalDr->getURL();
                globals->logMsg(ErrorRecord, errMsg);
            }
        }
    }

    // Deal with DOD
    if (dates.potentialDOD.isValid())
    {
        currentDOD = globals->globalDr->getDOD();
        if (!currentDOD.isValid())
            globals->globalDr->setDOD(dates.potentialDOD);
        else
        {
            if (currentDOD != dates.potentialDOD)
            {
                globals->globalDr->setDOD(dates.potentialDOD);
                errMsg << "Read in inconsistent DODs for: " << globals->globalDr->getURL();
                globals->logMsg(ErrorRecord, errMsg);
            }
        }
    }
}

void readObit::readLookupFields(const SEARCHPARAM &sp, const fieldNames &FieldNames)
{
    Q_UNUSED(sp);
    Q_UNUSED(FieldNames);

    /*    std::wstring fieldRead;
	PString fieldContent;
	int fieldType;
	bool success = false;
    unstructuredContent *tempContent = nullptr;  // only needed for date fields

    beg();

    while (!isEOS())
	{
		fieldRead = readFieldFollowing(sp);
		fieldType = FieldNames.find(fieldRead);  // returns -1 if no match found
		if (fieldType >= 0)
		{
			fieldContent = PString(readNextBetween(sp.tableLookupDelimiter));
			if ((fieldType == tlDOB) || (fieldType == tlDOD))
			{
				tempContent = new unstructuredContent(fieldContent.getString());
				date tempDate = tempContent->readDateField(DataRecord);
				fieldContent = ConvertNumberToString(tempDate.numericValue(), 0);
				delete tempContent;
				tempContent = 0;
			}
			// Additional pre-processing for certain field types
			std::vector<PString*> words;
			switch (fieldType)
			{
			case tlFamilyName:
			case tlFirstName:
			case tlMiddleName:
				parseContentFromTo(fieldContent, words);
				break;
				
			// Do nothing for remaining field types for now
			case tlFullName:
			case tlGender:
			case tlDOB:
			case tlDOD:
			case tlCity:
			case tlProvince:
			case tlCountry:
			case tlSpouseName:
			case tlURL:
			case tlPublisherName:
			case tlPublisherCity:
			case tlPublisherProvince:
			case tlPublisherCountry:
			case tlCauseOfDeath:
			default:
				words.push_back(new PString(fieldContent));
				break;
			}

			for (std::vector<PString*>::iterator iter = words.begin(); iter != words.end(); iter++)
			{
				DataRecord.storeContent(*iter, fieldType);
			}

			// clean up vector
			for (std::vector<PString*>::iterator iter = words.begin(); iter != words.end(); )
			{
				delete *iter;
				iter = words.erase(iter);
			}

			success = true;   // deemed ok if at least one field is matched and loaded
		}
	}
*/
}

void readObit::validateJustInitialNames()
{
    // If none of the first first three words in justInitialNames match a saved name from the structured
    // read, assume the unstructured content doesn't begin with the deceased's name.
    if (globals->globalDr->getNumSavedNames() > 0)
    {
        bool noMatch = true;
        int i = 0;
        int cutOff = static_cast<int>(justInitialNamesUC.countWords());
        if (cutOff > 3)
            cutOff = 3;
        if (cutOff < 2)
            cutOff = 0; // Skip any attempt to match

        NAMETYPE nameType;
        OQStream checkStream(justInitialNamesUC);
        OQString checkWord, tempWord;
        bool potentialGenderMarker = false;
        GENDER potentialGender = globals->globalDr->getGender();

        while (noMatch && (i < cutOff))
        {
            checkWord = checkStream.getWord().getString();
            if (potentialGender == genderUnknown)
            {
                if (checkWord.isTitle())
                {
                    potentialGenderMarker = true;
                    if (checkWord.isMaleTitle())
                        potentialGender = Male;
                    else
                        potentialGender = Female;
                }
            }
            tempWord = checkWord;
            while (checkWord.isCompoundName())
            {
                checkWord = checkStream.getWord().getString();
                tempWord += OQString(" ");
                tempWord += checkWord;
            }
            checkWord = tempWord;
            checkWord.removeEnding(PUNCTUATION);
            nameType = globals->globalDr->isAName(checkWord);
            if ((nameType >= 1) && (nameType <= 3))
            {
                noMatch = false;
                if (potentialGenderMarker)
                    globals->globalDr->setGender(potentialGender);
            }
            i++;
        }
        if (noMatch || (cutOff < 2))
            justInitialNamesUC.clear();
    }

    // Replace justInitialNamesUC if appropriate
    if ((justInitialNamesUC.getLength() == 0) && (blockedContent.firstBlock.getLength() > 1))
    {
        OQStream tempStream(blockedContent.firstBlock);
        OQString checkWord;
        unsigned int position = 0;
        unsigned int wordLength = 0;
        unsigned int numWords = 0;
        bool notRecognized = true;
        while (notRecognized && !tempStream.isEOS() && (numWords < 15))
        {
            checkWord = tempStream.getWord().getString();
            wordLength = checkWord.getLength();
            checkWord.removeBookEnds();
            checkWord.cleanUpEnds();
            checkWord.removeEnding(",");
            notRecognized = checkWord.isRecognized();
            position = tempStream.getPosition();
            numWords++;
        }
        NAMETYPE nameType = globals->globalDr->isAName(checkWord);
        if ((nameType >= 1) && (nameType <= 3))
        {
            int startFrom = static_cast<int>(position - wordLength - 1);
            justInitialNamesUC = blockedContent.firstBlock.getString().mid(startFrom, -1);
            justInitialNamesUC.pickOffNames();
        }
        else
        {
            bool tryNextWord = givenNameLookup(checkWord.lower().getString(), globals, globals->globalDr->getGender());
            if (tryNextWord)
            {
                checkWord = tempStream.getWord().getString();
                checkWord.removeBookEnds();
                nameType = globals->globalDr->isAName(checkWord);
                if ((nameType >= 1) && (nameType <= 3))
                {
                    justInitialNamesUC = blockedContent.firstBlock;
                    justInitialNamesUC.pickOffNames();
                }

            }
        }
    }
}

void readObit::readUnstructuredContent()
{
    //uc = blockedContent.allContent;
    LANGUAGE lang = globals->globalDr->getLanguage();

    // STEP 1 - Fill in names

    validateJustInitialNames();

    // Look for alternate first and last names
    if (justInitialNamesUC.getLength() > 0)
    {
        justInitialNamesUC.setContentLanguage(lang);
        globals->globalDr->setAlternates(justInitialNamesUC.readAlternates(PARENTHESES | QUOTES, false));
    }
    cleanedUpUC.setContentLanguage(lang);
    globals->globalDr->setAlternates(cleanedUpUC.readAlternates(PARENTHESES | QUOTES, true));

    // Look for most common name used within the obituary
    globals->globalDr->setAlternates(mcn.readMostCommonName(globals));

    // Look for middle name(s), if any, as a single string and sets other names along the way if they are encountered
    globals->globalDr->setMiddleNames(justInitialNamesUC.processAllNames());

    // Where middlename is used as a first name, add it as an AKA as well
    NAME tempName;
    tempName.name = globals->globalDr->getMiddleNameUsedAsFirstName();
    if (tempName.name.getLength() > 0)
    {
        tempName.numWords = tempName.name.countWords();
        globals->globalDr->setFirstNames(tempName);
    }

    // Sort firstNames to have formal ones first
    sortFirstNames();

    // STEP 2 - Fill in gender

    addressUnknownGender();

    // STEP 3 - Fill in dates
    if (missingDateInfo())
    {
        // First pass will see if pure complete dates are at front of string
        // Second pass will look at just years at front (eg. 1966 - 2016)
        // Third pass will review one sentence at a time for dates and key words (high credibility for first two sentences only)
        // Forth pass - Try to find DOB or YOB (different actions depending how far match occurs)
        // Fifth pass is a deeper dive (full content by sentence) looking for two consecutive dates, with potential messages generated
        // Sixth pass is to look for age at death (in first two sentences only)
        // Seventh pass is to look for the first word in the cleanedUpUC to be a number.
        // Eighth pass looks for any two dates in first three sentences, but only keeps if DODs available and match
        // Use date published as a proxy if still missing DOD
        // Finally, set min and max DOB if DOD is known and age at death is known

        fillInDatesFirstPass();
        if (missingDateInfo())
            fillInDatesSecondPass();
        if (missingDateInfo())
            fillInDatesThirdPass(cleanedUpUC);
        if (globals->globalDr->getYOB() == 0)
            fillInDatesFourthPass();

        // Passes 5+ don't require as much information in the content
        cleanedUpUC.removeAllKnownNames();
        if (cleanedUpUC.countWords() < 3)
        {
            cleanedUpUC = blockedContent.allContent;
            cleanedUpUC.setContentLanguage(lang);
            cleanedUpUC.removeAllKnownNames();
        }

        if (missingDateInfo())
            fillInDatesFifthPass();
        if (globals->globalDr->missingDOB() && !globals->globalDr->missingDOD())
            fillInDatesSixthPass();
        if (globals->globalDr->missingDOB() && !globals->globalDr->missingDOD())
            fillInDatesSeventhPass();
        if (globals->globalDr->missingDOB() && !globals->globalDr->missingDOD())
            fillInDatesEighthPass();
        if (globals->globalDr->missingDOD() && globals->globalDr->getDatePublished().isValid())
            globals->globalDr->setDOD(globals->globalDr->getDatePublished());

    }	// end getDates

    // Fill in missing info if possible
    if (globals->globalDr->getAgeAtDeath() == 0)
    {
        if (globals->globalDr->getDOB().isValid() && globals->globalDr->getDOD().isValid())
            globals->globalDr->setAgeAtDeath(static_cast<unsigned int>(elapse(globals->globalDr->getDOB(), globals->globalDr->getDOD())));
        globals->globalDr->setMinMaxDOB();
    }
}

bool readObit::fillInDatesStructured(unstructuredContent &uc)
{
    // This function is only designed to be called with a clean short stream of complete dates
    DATES dates;
    bool newInfoFound = false;
    uc.beg();

    dates = uc.readDOBandDOD();
    newInfoFound = processNewDateInfo(dates, 0);

    if (!newInfoFound && (globals->globalDr->missingDOB() || globals->globalDr->missingDOD()))
    {
        dates = uc.readYears();
        newInfoFound = processNewDateInfo(dates, 2);

        if (!newInfoFound && (globals->globalDr->missingDOB() || globals->globalDr->missingDOD()))
            fillInDatesThirdPass(uc);
    }

    return newInfoFound;
}


void readObit::fillInDatesFirstPass()
{
    // Looks for two complete dates at beginning of content
    DATES dates;

    bool limitWords = true;
    dates = cleanedUpUC.readDates(globals->globalDr->getLanguage(), limitWords);
    processNewDateInfo(dates, 1);
}

void readObit::fillInDatesSecondPass()
{
    // Looks for two complete years at beginning of content (YYYY - YYYY)
    DATES dates;

    dates = cleanedUpUC.readYears();
    processNewDateInfo(dates, 2);
}

void readObit::fillInDatesThirdPass(unstructuredContent &uc)
{
    // Looks for key words and dates in sentences
    DATES dates;
    QList<QString> firstNameList = globals->globalDr->getFirstNameList();

    unsigned int maxSentences = 99;
    dates = uc.contentKeyWordsAndDates(firstNameList, globals->globalDr->getLanguage(), maxSentences);
    processNewDateInfo(dates, 3);
}

void readObit::fillInDatesFourthPass()
{
    // Try to find DOB or YOB (different actions depending how far match occurs)
    DATES dates;

    dates = cleanedUpUC.contentReadBornYear(globals->globalDr->getLanguage());
    processNewDateInfo(dates, 4);
}

void readObit::fillInDatesFifthPass()
{
    // Look for two consecutive dates in any one sentence
    DATES dates;

    dates = cleanedUpUC.contentPullBackToBackDates(globals->globalDr->getLanguage());
    processNewDateInfo(dates, 5);
}


void readObit::fillInDatesSixthPass()
{
    // Look for age at death (in first two sentences only)
    // If a valid age at death (over 15) is located and a discrepancy in DODs is found (title/header vs text), DOD in text is used and dataRecord is updated
    unsigned int maxSentences = 2;
    cleanedUpUC.contentReadAgeAtDeath(maxSentences);
}

void readObit::fillInDatesSeventhPass()
{
    // Look for the first word in the cleanedUpUC to be a number
    cleanedUpUC.beg();
    if (globals->globalDr->missingDOB() && !globals->globalDr->missingDOD() && (globals->globalDr->getAgeAtDeath() == 0))
    {
        PQString word = cleanedUpUC.getWord();
        word.removeBookEnds(PARENTHESES);
        word.removeEnding(PUNCTUATION);
        if (word.isNumeric() && !word.isHyphenated())
        {
            int potentialAge = static_cast<int>(word.asNumber());
            if ((potentialAge >= 0) && (potentialAge <= 125))
            {
                bool fullyCredible = true;
                globals->globalDr->setAgeAtDeath(static_cast<unsigned int>(potentialAge), fullyCredible);
                globals->globalDr->setMinMaxDOB();
            }
        }
    }
}

void readObit::fillInDatesEighthPass()
{
    // Looks for key words and dates in sentences
    DATES dates;

    unsigned int maxSentences = 3;
    dates = cleanedUpUC.sentencePullOutDates(globals->globalDr->getLanguage(), maxSentences);
    processNewDateInfo(dates, 8);
}

void readObit::readStructuredContent()
{
    beg();
    bool fixHyphens, DOBfound, DODfound;
    unsigned int position;
    QDate qdate;
    QString target;
    OQString cleanString;

    DOBfound = false;
    DODfound = false;

    switch(globals->globalDr->getProvider())
    {
    case Legacy:
        break;

    case Passages:
        if (moveTo("Born:"))
        {
            QList<QDate> dateList;
            uc = getUntil("<", 50, true);
            if (uc.pullOutDates(globals->globalDr->getLanguage(), dateList, 1, cleanString, true))
                 globals->globalDr->setDOB(dateList.takeFirst());
        }
        else
            beg();

        if (moveTo("Date of Passing:"))
        {
            QList<QDate> dateList;
            uc = getUntil("<", 50, true);
            if (uc.pullOutDates(globals->globalDr->getLanguage(), dateList, 1, cleanString, true))
                globals->globalDr->setDOB(dateList.takeFirst());
        }
        break;

    case LifeMoments:
        break;

    case Batesville:
        switch (style)
        {
        case 0:
            if (moveTo("obit_name"))
            {
                // Read names (First Name first expected)
                uc = readNextBetween(BRACKETS);
                fixHyphens = true;      // Must be confident content contains only names
                uc.processStructuredNames(fixHyphens);

                // Read DOB and DOD in form (mmm dd, yyyy - mmm dd, yyyy)
                if (moveTo("lifespan"))
                {
                    uc = readNextBetween(BRACKETS);
                    fillInDatesStructured(uc);

                    // If still no go, assume that if field contains only a single date, then it is DOD
                    if ((globals->globalDr->getYOB() == 0) && (globals->globalDr->getYOD() == 0))
                    {
                        if (uc.getLength() <= 21)
                        {
                            QDate potentialDate = uc.readDateField();
                            if (potentialDate.isValid())
                                globals->globalDr->setDOD(potentialDate);
                        }
                    }
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(150, "text-container", "<h1>"))
            {
                // Read names (First Name first expected)
                uc = getUntil("</h1>", 100);
                fixHyphens = true;      // Must be confident content contains only names
                uc.processStructuredNames(fixHyphens);

                // Read DOB and DOD in form (mmm dd, yyyy - mmm dd, yyyy)
                if (moveTo("<h5>"))
                {
                    uc = getUntil("</h5>", 200);
                    uc.removeHTMLtags();
                    uc.unQuoteHTML();
                    fillInDatesStructured(uc);
                }
            }
            break;
        }

        break;

    case SIDS:
        break;

    case BrowseAll:
        if (consecutiveMovesTo(25, "class=\"obitname\"", "strong" ))
        {
            // Read names (First Name first expected)
            uc = readNextBetween(BRACKETS);
            fixHyphens = true;      // Must be confident content contains only names
            uc.processStructuredNames(fixHyphens);

            // Read DOB and DOD in form (mmm dd, yyyy - mmm dd, yyyy)
            if (moveTo("obitlink\">"))
            {
                uc = readNextBetween(PARENTHESES);
                fillInDatesStructured(uc);
            }
        }
        break;

    case DignityMemorial:
        if (consecutiveMovesTo(500, "class=\"w-content\"", ">OBITUARY<", "itemprop=\"name\""))
        {
            // Read names (First Name first expected)
            uc = readNextBetween(BRACKETS);
            fixHyphens = true;      // Must be confident content contains only names
            uc.processStructuredNames(fixHyphens);

            // Read DOB and DOD in form (mmm dd, yyyy - mmm dd, yyyy), split into two separate fields
            // Error messages will be logged if new dates are inconsistent with prior information
            if (moveTo("itemprop=\"birthDate\""))
            {

                uc = readNextBetween(PARENTHESES);
                qdate = uc.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOB(qdate);
            }
            else
            {
                // Reset read position to read in DOD
                beg();
                consecutiveMovesTo(500, "class=\"w-content\"", ">OBITUARY<", "itemprop=\"name\"");
            }

            if (moveTo("itemprop=\"deathDate\""))
            {

                uc = readNextBetween(PARENTHESES);
                qdate = uc.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOD(qdate);
            }

            // If still no valid DOD, potential date included in a header
            if (!globals->globalDr->getDOD().isValid())
            {
                beg();
                if(consecutiveMovesTo(500, "class=\"w-content\"", ">OBITUARY<", "class=\"header-subtitle\""))
                {
                    QList<QDate> dateList;
                    uc = readNextBetween(BRACKETS);
                    if(uc.pullOutDates(globals->globalDr->getLanguage(), dateList, 1, cleanString, true))
                        qdate = dateList.takeFirst();
                    if (qdate > QDate(2009,12,31))
                         globals->globalDr->setDOD(qdate);
                }
            }
        }
        break;

    case ConnellyMcKinley:
        if (moveTo("\"obit_name\">"))
        {
            uc = getUntil("</td>");
            uc.removeHTMLtags(false);
            uc.simplify();

            // Read names (First Name first expected)
            // For this site, maiden names are separated by a comma  (e.g., "Charlotte Adams	(Maiden: Fitzpatrick, Harr)")
            QString newString;
            QString tempString = uc.getString();
            int index = tempString.indexOf("Maiden:", 0, Qt::CaseSensitive);
            if (index >= 0)
            {
                int newIndex = tempString.indexOf(", ", index + 7, Qt::CaseSensitive);
                if (newIndex >= 0)
                {
                    newString = tempString.left(newIndex);
                    newString += QString("/");
                    newString += tempString.right(tempString.length() - newIndex - 2);
                    uc = newString;
                }
            }

            // This site also has problematic double quotes
            newString = uc.getString();
            while (newString.indexOf("''") >= 0)
                newString.replace(QString("''"), QString("'"), Qt::CaseInsensitive);
            if (newString[0] == QChar(' '))  // get rid of any leading space
                newString.remove(0, 1);
            uc = newString;

            fixHyphens = true;      // Must be confident content contains only names
            uc.processStructuredNames(fixHyphens);
        }
        break;

    case CFS:

        if (globals->globalDr->getProviderKey() > 90000)
        {
            // Old style of obits
            if (consecutiveMovesTo(500, "class=\"obituary\"", "style=\"font-size:120%;\""))
            {
                // Read name first
                uc = readNextBetween(BRACKETS);
                fixHyphens = true;      // Must be confident content contains only names
                uc.processStructuredNames(fixHyphens);

                // Move on to DOB and DOD
                if (consecutiveMovesTo(25, "Born:", ","))
                {
                    QList<QDate> dateList;
                    uc = getUntil("<", 50, true);
                    if (uc.pullOutDates(globals->globalDr->getLanguage(), dateList, 1, cleanString, true))
                         globals->globalDr->setDOB(dateList.takeFirst());
                }
                else
                    beg();

                if (consecutiveMovesTo(25, "Died:", ","))
                {
                    QList<QDate> dateList;
                    uc = getUntil("<", 50, true);
                    if (uc.pullOutDates(globals->globalDr->getLanguage(), dateList, 1, cleanString, true))
                        globals->globalDr->setDOD(dateList.takeFirst());
                }
            }
        }
        else
        {
            // New style of obits

            // Get structured name
            switch(style)
            {
            case 0:
                target = QString("obittitle-v2");
                break;

            case 1:
                target = QString("obitnameV3");
                break;

            }

            if (moveTo(target))
            {
                uc = readNextBetween(BRACKETS);
                fixHyphens = true;      // Must be confident content contains only names
                uc.processStructuredNames(fixHyphens);
            }

            // Get dates and age
            switch(style)
            {
            case 0:
                target = QString("class=\"obittitle-dates\"");
                break;

            case 1:
                target = QString("class=\"obitdates\"");
            }

            if (moveTo(target))
            {
                position = getPosition();
                if (moveTo("class=\"dob\""))
                {
                    uc = readNextBetween(BRACKETS);
                    qdate = uc.readDateField();
                    if (qdate.isValid())
                    {
                        globals->globalDr->setDOB(qdate);
                        DOBfound = true;
                    }
                }

                backward(getPosition() - position);

                if (moveTo("class=\"dod\""))
                {
                    uc = readNextBetween(BRACKETS);
                    qdate = uc.readDateField();
                    if (qdate.isValid())
                    {
                        globals->globalDr->setDOD(qdate);
                        DODfound = true;
                    }
                }

                backward(getPosition() - position);

                if (!(DOBfound && DODfound) && moveTo("class=\"ob-age\""))
                {
                    uc = readNextBetween(PARENTHESES);
                    PQString age = getWord();  // Indirect way of dropping "age" at front
                    age = getWord();
                    if (age.isNumeric())
                        globals->globalDr->setAgeAtDeath(static_cast<unsigned int>(age.asNumber()));
                }
            }
        }

        break;

    case Frazer:
        if (consecutiveMovesTo(500, "class=\"obituariesDetailsPage\"", "itemprop=\"name\""))
        {
            // Get name, dropping any reference to location at end
            uc = readNextBetween(BRACKETS);
            PQString singleChar = uc.right(1);
            if ((singleChar.getCharType() & PARENTHESES) == PARENTHESES)
            {
                int index = uc.findPosition(PQString("("), -1);
                uc.dropRight(uc.getLength() - static_cast<unsigned int>(index) + 1);
                if (uc.getString().right(1) == QString(" "))
                    uc.dropRight(1);
            }
            uc.processStructuredNames();

            // Read dates if provided
            position = getPosition();
            if (moveTo("itemprop=\"birthDate\""))
            {
                uc = readNextBetween(BRACKETS);
                qdate = uc.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOB(qdate);
            }

            backward(getPosition() - position);

            if (moveTo("itemprop=\"deathDate\""))
            {
                uc = readNextBetween(BRACKETS);
                qdate = uc.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOD(qdate);
            }
        }

        break;

    case Alternatives:
        if (moveTo("class=\"page-title obit-title\""))
        {
            // Get name
            uc = readNextBetween(BRACKETS);
            uc.processStructuredNames();

            // Read DOD only
            if (moveTo("class=\"obit-dod\""))
            {
                uc = readNextBetween(BRACKETS);
                // On occasion field will contain both DOB and DOD
                DATES dates = uc.readDOBandDOD();
                if (dates.hasDateInfo())
                    processNewDateInfo(dates, 1);
                else
                {
                    qdate = uc.readDateField();
                    if (qdate.isValid())
                        globals->globalDr->setDOD(qdate);
                }
            }
        }

        break;

    case FuneralTech:
        if (moveTo("deceased-info"))
        {
            moveTo("<h2");
            // Read names (First Name first expected)
            uc = readNextBetween(BRACKETS);
            uc.simplify();
            fixHyphens = true;      // Must be confident content contains only names
            uc.processStructuredNames(fixHyphens);

            // Read DOB and DOD in form (mmm dd, yyyy - mmm dd, yyyy)
            if (moveTo("<h3"))
            {
                uc = readNextBetween(PARENTHESES);
                uc.simplify();
                fillInDatesStructured(uc);
            }
        }
        break;

    case WordPress:
        // Structured content written as "Smith, John Stanley  1968 - 2015"
        if (moveTo("entry-title hidden"))
        {
            OQString begString, endString, word;
            OQString space(" ");
            uc = readNextBetween(BRACKETS);
            bool numEncountered = false;

            while (!uc.isEOS())
            {
                word = uc.getWord(true);
                if (!numEncountered && word.isNumeric())
                    numEncountered = true;
                if (!numEncountered)
                {
                    begString += word;
                    begString += space;
                }
                else
                {
                    endString += word;
                    endString += space;
                }
            }
            begString.removeEnding(SPACE);
            endString.removeEnding(SPACE);

            uc = unstructuredContent(begString);
            fixHyphens = false;      // Must be confident content contains only names
            uc.processStructuredNames(fixHyphens);

            // Read YOB and YOD in form (yyyy - yyyy)
            uc = unstructuredContent(endString);
            fillInDatesStructured(uc);

            // Read date published as some obits don't have DOD
            consecutiveMovesTo(500, "date published", ">");
            backward();
            uc = readNextBetween(BRACKETS);
            globals->globalDr->setDatePublished(uc.readDateField());
        }
        break;

    case FrontRunner:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(500, "bom-in-memory-name", ">"))
            {
                // Read name
                uc = getUntil("</div>");
                uc.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                uc.processStructuredNames(fixHyphens);

                // Read YOB and YOD (YYYY - YYYY)
                if (moveTo("bom-in-memory-date"))
                {
                    uc = readNextBetween(BRACKETS);
                    uc.simplify();
                    fillInDatesStructured(uc);
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(500, "class=\"bom-header-info\"", "a href", "php\">"))
            {
                // Read name
                uc = getUntil("</a>");
                uc.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                uc.processStructuredNames(fixHyphens);

                // Read DOB and DOD
                if (moveTo("<p>"))
                {
                    uc = getUntil("</p>");
                    uc.simplify();
                    fillInDatesStructured(uc);
                }
            }
            break;

        case 2:
            if (moveTo("<!-- obit name -->"))
            {
                moveTo("<h1");
                // Read names (First Name first expected)
                uc = readNextBetween(BRACKETS);
                uc.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                uc.processStructuredNames(fixHyphens);

                // Read DOB and DOD
                if (moveTo("<!-- obit date -->"))
                {
                    position = getPosition();

                    if (moveTo("class=\"dateofBirth\""))
                    {
                        uc = readNextBetween(BRACKETS);
                        qdate = uc.readDateField();
                        if (qdate.isValid())
                            globals->globalDr->setDOB(qdate);
                    }

                    backward(getPosition() - position);

                    if (moveTo("class=\"dateofDeath\""))
                    {
                        uc = readNextBetween(BRACKETS);
                        qdate = uc.readDateField();
                        if (qdate.isValid())
                            globals->globalDr->setDOD(qdate);
                    }
                }
            }
            break;
        }
        break;


    default:
        PQString errMsg;
        errMsg << "Structured content not coded for: " << globals->globalDr->getProvider();
        globals->logMsg(ErrorRunTime, errMsg);

    }
}

void readObit::readTitleInfo()
{
    // Read in any information about deceased's name from header and title sections of web page
    beg();
    switch (globals->globalDr->getProvider())
    {
    case Legacy:
        // Nothing available
        break;

    case Passages:
        // Initial name search from title and header sections to bookend unstructured searches
        // Format is always LASTNAME FIRSTNAME -

        if (moveTo("<title>"))
        {
            NAME firstName, lastName;
            OQString name;
            unsigned int numNamesRemaining;

            uc = getUntil(" - ", 75, true);
            if (uc.getLength() == 0)
            {
                // Backup read
                // TODO
            }
            uc.cleanUpEnds();
            uc.beg();
            numNamesRemaining = uc.countWords();
            firstName.numWords = 1;
            lastName.numWords = numNamesRemaining - firstName.numWords;
            while (numNamesRemaining > 0)
            {
                name = uc.getWord();
                if (numNamesRemaining > 1)
                    lastName.name += name.proper();
                else
                    firstName.name = name.proper();
                if (numNamesRemaining > 2)
                    lastName.name += PQString(" ");
                numNamesRemaining--;
            }
            globals->globalDr->setFamilyName(lastName);
            globals->globalDr->setFirstName(firstName);
        }

        break;

    case LifeMoments:
        // Initial name search from title and header sections to bookend unstructured searches
        // No fixed order between last name and first name

        if (moveTo("<meta name=\"description\" content=\""))
        {
            uc = getUntil(" - ", 75, true);
            if (uc.getLength() == 0)
            {
                // Backup read
                beg();
                if (moveTo("<title>"))
                        uc = getUntil(" - ", 50, true);
            }
            uc.cleanUpEnds();
            uc.beg();
            if (uc.countWords() > 1)
            {
                if (uc.contains(","))
                    uc.readLastNameFirst();
                else
                    uc.readFirstNameFirst();
            }
        }

        break;

    case Batesville:
        // Nothing available
        break;

    default:
        // Can only get here due to an oversight
        PQString errMsg;
        errMsg << QString("ReadTitleInfo switch criteria not set up for provider: ") << globals->globalDr->getProvider();
        globals->logMsg(ErrorRunTime, errMsg);

    }
}

void readObit::readStyle()
{
    style = 0;
    OQString tempString;

    beg();
    switch (globals->globalDr->getProvider())
    {
        case Batesville:
            if (moveTo("text-container"))
                style = 1;
        break;

    case CFS:
        if(moveTo("initV3Obit()"))
            style = 1;
        else
        {
            beg();
            if(moveTo("initV2Obit()"))
                style = 0;
            else
                style = 99;
        }
        break;

    case FrontRunner:
    {
        unsigned int ID = static_cast<unsigned int>(globals->globalDr->getID().asNumber());
        style = 0;

        if (ID > 3612282)
            style++;

        if (ID > 3667415)
            style++;

        break;
    }

    default:
        // Do nothing

        break;

    }
}

void readObit::determineLanguageAndGender()
{
    if (uc.getLength() > 0)
    {
        uc.determineLanguageAndGender(cleanedUpUC, justInitialNamesUC);
        if ((uc.getLanguage() != language_unknown) && (uc.getGender() != genderUnknown))
        {
            globals->globalDr->setLanguage(uc.getLanguage());
            globals->globalDr->setGender(uc.getGender());
        }
        else
        {
            // Try again using all content (i.e., not just block with titleKey)
            unstructuredContent allContent, dummy1, dummy2;
            allContent = blockedContent.allContent;
            allContent.determineLanguageAndGender(dummy1, dummy2);

            // Update language first where required
            if (globals->globalDr->getLanguage() == language_unknown)
            {
                if (uc.getLanguage() == language_unknown)
                    globals->globalDr->setLanguage(allContent.getLanguage());
                else
                {
                    if (uc.getLanguage() == allContent.getLanguage())
                        globals->globalDr->setLanguage(uc.getLanguage());
                    else
                    {
                        // Non conclusive, so use most likely outcome, but flag record to check
                        globals->globalDr->setLanguage(uc.getLanguage());
                        PQString errMsg;
                        errMsg << "Mismatched language conclusion:  "  << globals->globalDr->getURL();
                        globals->logMsg(ErrorRunTime, errMsg);
                    }
                }
            }

            // Update gender second where required
            if (globals->globalDr->getGender() == genderUnknown)
            {
                if (uc.getGender() == genderUnknown)
                    globals->globalDr->setGender(allContent.getGender());
                else
                {
                    if (uc.getGender() == allContent.getGender())
                        globals->globalDr->setGender(uc.getGender());
                    else
                    {
                        // Non conclusive, so use most likely outcome, but flag record to check
                        globals->globalDr->setGender(uc.getGender());
                        PQString errMsg;
                        errMsg << "Mismatched gender conclusion:  "  << globals->globalDr->getURL();
                        globals->logMsg(ErrorRunTime, errMsg);
                    }
                }
            }
        }
    }

    // If gender unknown, check if any sentences start with a gender pronoun
    bool unknownGender = (globals->globalDr->getGender() == genderUnknown);
    if (unknownGender)
    {
        GENDER potentialGender = mcn.startsWithGenderWord(globals);
        if ((potentialGender == Male) && (godReference == false))
            globals->globalDr->setGender(Male);
        else
        {
            if (potentialGender == Female)
                globals->globalDr->setGender(Female);
        }
    }

    // If gender still unknown, check first three sentences for "relationship words"
    unknownGender = (globals->globalDr->getGender() == genderUnknown);
    bool checkedAllContent = false;
    if (unknownGender)
    {
        uc = blockedContent.select(globals->globalDr->getTitleKey());
        if (uc.countWords() < 8)
        {
            uc = blockedContent.allContent;
            checkedAllContent = true;
        }

        uc.beg();
        unsigned int i = 0;
        unsigned int maxSentences = 3;  // information pulled from fourth or later sentence could be anything, so stop reading after two sentences
        while (unknownGender && !uc.isEOS() && (i < maxSentences))
        {
            sentence = uc.getSentence();
            unknownGender = sentence.genderRelationalReferences();
            i++;
        }

        if (unknownGender && !checkedAllContent)
        {
            uc = blockedContent.allContent;
            uc.beg();
            unsigned int i = 0;
            unsigned int maxSentences = 3;  // information pulled from fourth or later sentence could be anything, so stop reading after two sentences
            while (unknownGender && !uc.isEOS() && (i < maxSentences))
            {
                sentence = uc.getSentence();
                unknownGender = sentence.genderRelationalReferences();
                i++;
            }
        }
    }
}

void readObit::parseContentFromTo(PString &content, std::vector<PString*> &wordVector)
{
	// Assume no breaking on spaces
	bool slash, parentheses, space;
	PString singleChar, word;
	bool wordStarted = false;

	for (unsigned int i = 0; i < content.getLength(); i++)
	{
		singleChar = content[i];
		slash = (singleChar.getCharType() & SLASH) == SLASH;
		parentheses = (singleChar.getCharType() & PARENTHESES) == PARENTHESES;
		space = (singleChar.getCharType() & SPACE) == SPACE;

		if (wordStarted)
		{
			if (slash || parentheses)
			{
				wordStarted = false;
				word.removeEnding(_T(" "));
				wordVector.push_back(new PString(word));
				word.clear();
			}
			else
				word += singleChar;
		}
		else
		{
			if (!space && !slash && !parentheses)
			{
				wordStarted = true;
				word += singleChar;
			}
		}

		// complete last word if last character read
		if (wordStarted && (i == (content.getLength()-1)))
		{
			word.removeEnding(_T(" "));
			wordVector.push_back(new PString(word));
		}
	}
}

unsigned int readObit::countFrequency(QString word, Qt::CaseSensitivity caseSensitivity) const
{
    return cleanedUpUC.countFrequency(word, caseSensitivity);
}

unsigned int readObit::countFrequencyFirst(QString word, Qt::CaseSensitivity caseSensitivity) const
{
    return cleanedUpUC.countFrequencyFirst(word, caseSensitivity);
}

void readObit::setGlobalVars(GLOBALVARS &gv)
{
    globals = &gv;
    uc.setGlobalVars(*globals);
    // Remaining unstructuredContent set automatically as globals is static
}

void readObit::setGodReference(bool bl)
{
    godReference = bl;
}

bool readObit::hasGodReference() const
{
    return godReference;
}

void readObit::setNumMaleWords(int numWords)
{
    numMaleWords = numWords;
}

void readObit::setNumFemaleWords(int numWords)
{
    numFemaleWords = numWords;
}

int readObit::getNumMaleWords() const
{
    return numMaleWords;
}

int readObit::getNumFemaleWords() const
{
    return numFemaleWords;
}

unstructuredContent* readObit::getCleanedUpUCaddress()
{
    return &cleanedUpUC;
}

void readObit::readParentsLastName()
{
    if (blockedContent.allContent.getLength() == 0)
        return;

    // TODO: Modify coding to capture "born in Winnipeg to Norm and Estelle Rivard"

    bool found = false;
    QList<QString> targetPhrases = OQString::getParentReferences(globals->globalDr->getLanguage());
    QList<QString> parentNames;
    QString allContent = blockedContent.allContent.getString();
    QString targetText;
    unstructuredContent content;
    int index, wordCount, wordCountAfterAnd;
    double firstUnisex, secondUnisex, unisex;
    OQString word, name, savedName, lastChar, nextWord, priorWord, maleLastName, femaleLastName;
    OQString space(" ");
    OQString period(".");
    OQString semicolon(";");
    bool keepGoing, wordIsAnd, andEncountered, compoundName, endOfNames, nextWordIsSurname, isInitial;
    bool isSurname, isGivenName;
    bool highConfidence = false;
    bool maleNameEncountered = false;
    bool femaleNameEncountered = false;
    wordCount = 0;
    firstUnisex = secondUnisex = 0.5;
    GENDER currentGender;

    while (!found && targetPhrases.size() > 0)
    {
        targetText = targetPhrases.takeFirst();
        index = allContent.indexOf(targetText, 0, Qt::CaseInsensitive);
        if (index >= 0)
        {
            keepGoing = true;
            andEncountered = false;
            endOfNames = false;
            nextWordIsSurname = false;
            femaleNameEncountered = false;
            maleNameEncountered = false;
            currentGender = genderUnknown;
            wordCount = 0;
            wordCountAfterAnd = 0;
            name.clear();
            savedName.clear();
            parentNames.clear();
            firstUnisex = 0.5;
            secondUnisex = 0.5;
            content = allContent.mid(index + targetText.size() + 1, 100);

            while (keepGoing && !found && !highConfidence && !content.isEOS())
            {
                priorWord = word;
                word = content.getWord(true);
                nextWord = content.peekAtWord(true);
                nextWord.removeEnding(PUNCTUATION);

                if ((word.lower() == OQString("and")) || (word == OQString("&")))
                {
                    wordIsAnd = true;
                    andEncountered = true;
                    currentGender = genderUnknown;
                    parentNames.clear();
                }
                else
                    wordIsAnd = false;

               // Analyze word
                compoundName = word.isCompoundName();
                lastChar = word.right(1);
                endOfNames = word.removeEnding(PUNCTUATION);
                isInitial = (word.getLength() == 1);
                endOfNames = endOfNames && !isInitial;
                nextWordIsSurname = !endOfNames && surnameLookup(nextWord.getString(), globals);

                // Determine if this is the last word to be considered
                if (((lastChar == period) && !word.isSaint() && !isInitial) || (lastChar == semicolon))
                    keepGoing = false;

                if (!word.hasBookEnds(PARENTHESES))
                {

                    if (word.isCapitalized() && !wordIsAnd)
                    {
                        // Stand alone analysis based on gender lookups
                        isGivenName = givenNameLookup(word.getString(), globals, genderUnknown);
                        isSurname = surnameLookup(word.getString(), globals);

                        if (isGivenName && !isSurname)
                        {
                            parentNames.append(word.getString());

                            isGivenName = givenNameLookup(nextWord.getString(), globals, genderUnknown);
                            isSurname = surnameLookup(nextWord.getString(), globals);

                            if (isGivenName && !isSurname)
                                parentNames.append(nextWord.getString());

                            unisex = genderLookup(parentNames, globals);

                            if (unisex >= 0.9)
                            {
                                maleNameEncountered = true;
                                currentGender = Male;
                                if (!isGivenName && isSurname)
                                {
                                    maleLastName = nextWord;
                                    if (femaleNameEncountered && (maleLastName == femaleLastName))
                                    {
                                        highConfidence = true;
                                        name = word;
                                    }
                                }
                            }
                            else
                            {
                                if (unisex <= 0.1)
                                {
                                    femaleNameEncountered = true;
                                    currentGender = Female;
                                    if (!isGivenName && isSurname)
                                    {
                                        femaleLastName = nextWord;
                                        if (maleNameEncountered && (femaleLastName == maleLastName))
                                        {
                                            highConfidence = true;
                                            name = word;
                                        }
                                    }
                                }
                            }
                        }

                        // Resume normal process
                        if ((wordCount >= 2) && (wordCountAfterAnd >= 1) && !nextWordIsSurname && !highConfidence)
                        {
                            name += word;
                            if (!compoundName )
                                found = true;
                        }

                        if (!compoundName)
                            wordCount++;
                        if (andEncountered && !compoundName)
                            wordCountAfterAnd++;

                    }
                    else
                    {
                        if ((wordCount >= 2) && (compoundName || word.isSaint()))
                        {
                            name += word;
                            name += space;
                            if (!compoundName)
                                wordCount++;
                            if (andEncountered && !compoundName)
                                wordCountAfterAnd++;
                        }
                        else
                        {
                            if (wordIsAnd)
                                keepGoing = true;
                            else
                            {
                                if (word.isCapitalized() && nextWordIsSurname)
                                    keepGoing = true;
                                else
                                    keepGoing = false;
                            }
                        }
                    }

                    // Retain stats and run some checks
                    if (!wordIsAnd)
                    {
                        if (wordCount == 1)
                        {
                            QList<QString> firstName;
                            firstName.append(word.getString());
                            firstUnisex = genderLookup(firstName, globals);
                        }

                        if (andEncountered && (wordCountAfterAnd == 1))
                        {
                            QList<QString> secondName;
                            secondName.append(word.getString());
                            secondUnisex = genderLookup(secondName, globals);
                        }

                        if ((wordCount >= 2) && !andEncountered)
                        {
                            savedName = word;
                            if (globals->globalDr->isALastName(word))
                                return;
                        }

                        if ((wordCountAfterAnd >= 2) && globals->globalDr->isALastName(word))
                            return;
                    }

                }
            }   // end while running through text excerpt
        }
    }   // end while looping through potential parent references

    if (!highConfidence)
    {
        if (maleNameEncountered && (maleLastName.getLength() == 0) && femaleNameEncountered && (femaleLastName.getLength() > 0))
        {
            name = femaleLastName;
            highConfidence = true;
        }
    }

    if ((wordCount >= 4) && !highConfidence)
    {
        PQString errMsg;
        errMsg << "Verify inclusion of parents name for: " << globals->globalDr->getURL();
        globals->logMsg(ErrorRecord, errMsg);
    }

    if (found || highConfidence)
    {
        NAME temp;
        if (highConfidence)
            temp.name = name;
        else
        {
            if ((firstUnisex > 0.5) && (secondUnisex < 0.5) && (wordCount >= 4) && !priorWord.hasBookEnds(PARENTHESES))
                temp.name = savedName;
            else
                temp.name = name;
        }

        globals->globalDr->setParentsLastName(temp.name);
        globals->globalDr->removeFromMiddleNames(temp.name);
        if (globals->globalDr->getGender() != Male)
        {
            temp.numWords = temp.name.countWords();
            globals->globalDr->setFamilyName(temp);
        }
    }
}

void readObit::processMaidenNames()
{
    QList<PQString> nameList = globals->globalDr->getMaidenNames();
    if (nameList.size() == 0)
        return;

    QString name;
    NAME drName;
    for (int i = 0; i < nameList.size(); i++)
    {
        name = nameList.at(i).getString();
        while (name.indexOf("''") >= 0)
            name.replace(QString("''"), QString("'"), Qt::CaseInsensitive);
        if (name[0] == QChar(' '))  // get rid of any leading space
            name.remove(0, 1);
        if (i == 0)
            globals->globalDr->setParentsLastName(name);
        drName.clear();
        drName.name = name;
        drName.numWords = drName.name.countWords();
        globals->globalDr->setFamilyName(drName);
    }
}

void readObit::sortFirstNames()
{
    OQString temp, firstName, alt1Name, alt2Name;
    PQString warningMessage;
    bool switchNames;
    NAME name;

    alt1Name = globals->globalDr->getFirstNameAKA1().lower();
    if (alt1Name.getLength() == 0)
        return;

    firstName = globals->globalDr->getFirstName().lower();
    switchNames = firstName.isInformalVersionOf(alt1Name.getString(), warningMessage);

    if (switchNames)
    {
        name.name = alt1Name;
        name.numWords = alt1Name.countWords();
        globals->globalDr->setFirstName(name, 1);

        name.name = firstName;
        name.numWords = firstName.countWords();
        globals->globalDr->setFirstName(name, 2);
    }
    else
    {
        alt2Name = globals->globalDr->getFirstNameAKA2().lower();
        if (alt2Name.getLength() == 0)
            return;

        switchNames = firstName.isInformalVersionOf(alt2Name.getString(), warningMessage);

        if (switchNames)
        {
            name.name = alt2Name;
            name.numWords = alt2Name.countWords();
            globals->globalDr->setFirstName(name, 1);

            name.name = firstName;
            name.numWords = firstName.countWords();
            globals->globalDr->setFirstName(name, 2);

            name.name = alt1Name;
            name.numWords = alt1Name.countWords();
            globals->globalDr->setFirstName(name, 3);
        }
    }
}

void readObit::clear()
{
    filename.clear();
    uc.clear();
    blockedContent.clear();
    cleanedUpUC.clear();
    justInitialNamesUC.clear();
    sentence.clear();
    mcn.clear();

    godReference = false;
    numMaleWords = 0;
    numFemaleWords = 0;
    style = 0;
}

void readObit::operator = (QFile &file)
{
    setSource(file);
}

bool readObit::setSource(QFile &file)
{
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        filename = file.fileName();
        itsString = file.readAll();
        file.close();
        return true;
    }
    else
    {
        PQString errMsg;
        errMsg << "Could not open file for processing: " << file.fileName();
        globals->logMsg(ErrorConnection, errMsg);
        return false;
    }
}

PQString readObit::getSource() const
{
    return filename;
}

bool readObit::processNewDateInfo(DATES &dates, unsigned int pass)
{
    // Perform some validations and process as appropriate
    // For pass 2 and most pass 4, potential YYYY are provided instead of QDate(yyyy,mm,d)

    // Pass flag highlights where the information came from
    // Zero pass from a structured read
    // First pass will see if pure complete dates are at front of string
    // Second pass will look at just years at front (eg. 1966 - 2016)
    // Third pass will review one sentence at a time for dates and key words (high credibility for first two sentences only)
    // Forth pass - Try to find DOB or YOB (different actions depending how far match occurs)
    // Fifth pass is a deeper dive (full content by sentence) looking for two consecutive dates, with potential messages generated
    // Sixth pass is to look for age at death (in first two sentences only)
    // Seventh pass is to look for the first word in the cleanedUpUC to be a number.
    // Eighth pass looks for any two dates in first three sentences, but only keeps if DODs available and match
    // Finally, set min and max DOB if DOD is known and age at death is known

    bool fixable, implementSolution;
    bool forceDODoverride, forceDOBoverride, forceYODoverride, forceYOBoverride;
    bool newDOBinfo, newDODinfo, newYODinfo, newYOBinfo;
    bool DOBissueExists, DODissueExists, YODissueExists, YOBissueExists, YOBindirectIssueExists;
    bool onlyYearInfo, excludeInfo, nonFixableDOBerror;
    bool fullyCredible;
    qint64 daysDifferent;


    LANGUAGE lang  = globals->globalDr->getLanguage();
    QDate DOD = globals->globalDr->getDOD();
    QDate DOB = globals->globalDr->getDOB();
    int YOD = static_cast<int>(globals->globalDr->getYOD());
    int YOB = static_cast<int>(globals->globalDr->getYOB());
    newDODinfo = dates.potentialDOD.isValid();
    newYODinfo = !newDODinfo && (dates.potentialYOD > 0);
    newDOBinfo = dates.potentialDOB.isValid();
    newYOBinfo = !newDOBinfo && (dates.potentialYOB > 0);
    forceDODoverride = false;
    forceDOBoverride = false;
    forceYODoverride = false;
    forceYOBoverride = false;
    DODissueExists = false;
    DOBissueExists = false;
    YODissueExists = false;
    YOBissueExists = false;
    YOBindirectIssueExists = false;
    fullyCredible = false;

    excludeInfo = ((pass == 8) && (dates.potentialDOD != DOD)) || !dates.hasDateInfo();
    nonFixableDOBerror = (newDOBinfo && !newDODinfo && DOD.isValid() && (dates.potentialDOB > DOD)) ||
                         (newYOBinfo && !newYODinfo && (((DOD.isValid() && (dates.potentialYOB > DOD.year())) || ((YOD > 0) && (dates.potentialYOB > YOD)))));
    if ((excludeInfo && dates.hasDateInfo()) || nonFixableDOBerror)
    {
        PQString errMsg;
        errMsg << "Encountered issues in reading dates for: " << globals->globalDr->getURL();
        globals->logMsg(ErrorRecord, errMsg);
        return false;
    }

    if (!(newDODinfo || newDOBinfo || newYODinfo || newYOBinfo))
        return false;

    YODissueExists = newYODinfo && (YOD > 0) && (YOD != dates.potentialYOD);
    YOBissueExists = newYOBinfo && (YOB > 0) && (YOB != dates.potentialYOB);
    DODissueExists = newDODinfo && ((DOD.isValid() && (dates.potentialDOD != DOD)) || (!DOD.isValid() && YODissueExists));
    DOBissueExists = newDOBinfo && ((DOB.isValid() && (dates.potentialDOB != DOB)) || (!DOB.isValid() && YOBissueExists));

    // Assess if indirect YOB issue exists where ageAtDeath set
    if (newYOBinfo && (YOB == 0) && (globals->globalDr->getAgeAtDeath() > 0))
    {
        int minYOB = globals->globalDr->getMinDOB().year();
        int maxYOB = globals->globalDr->getMaxDOB().year();
        bool consistent = ((dates.potentialYOB >= minYOB) && (dates.potentialYOB <= maxYOB));
        if (!consistent)
            YOBindirectIssueExists = true;
    }

    // Implement fixes where reasonably possible and reliable
    onlyYearInfo = (pass == 2) || ((pass == 4) && (dates.potentialYOB > 0));

    // If inconsistent DOD found, attempts to fix issue if first DOD likely reported in header/title does not match DOD provided in unstructured content
    // Where two dates are read into structured content, or where DOD is within first two sentences with a key word, assume unstructured content is correct
    if (DODissueExists && !onlyYearInfo)
    {
        // Validation #1 - Check where two potential dates exist, but only where nothing read in for DOB to date
        // The two dates read in at the same time will be assumed to be correct
        // The existing DOD will be assumed to be from header/title and incorrect

        implementSolution = (newDOBinfo && !DOB.isValid()) || (pass == 3) || (pass == 9);
        if (implementSolution)
        {
            daysDifferent = DOD.daysTo(dates.potentialDOD);
            fixable = (daysDifferent >= -366) && (daysDifferent <= 366);
            if (fixable)
            {
                forceDODoverride = true;
                DODissueExists = false;
            }
        }
    }

    if (DODissueExists && !onlyYearInfo && (pass != 3) && (pass != 9))
    {
        // Validation #2 - Look for sentence with valid key words and dates
        // Essentially same as third pass in readUnstructured (need to avoid recursive call)

        DATES altDates;
        QList<QString> firstNameList = globals->globalDr->getFirstNameList();
        unsigned int maxSentences = 3;  // information pulled from fourth or later sentence could be anything, so stop reading after three sentences

        altDates = cleanedUpUC.contentKeyWordsAndDates(firstNameList, lang, maxSentences);
        if (altDates.potentialDOD.isValid() && (altDates.potentialDOD == dates.potentialDOD))
        {
            forceDODoverride = true;
            DODissueExists = false;
        }
    }

    // Process updates (and trigger potential error messages)
    if (newDODinfo)
    {
        if (DODissueExists)
        {
            if (dates.fullyCredible && !globals->globalDr->getDODcredibility())
                fullyCredible = true;
            else
                fullyCredible = false;
        }
        else
            fullyCredible = dates.fullyCredible;

        if (fullyCredible)
            forceDODoverride = true;

        globals->globalDr->setDOD(dates.potentialDOD, forceDODoverride, fullyCredible);
    }

    if (newDOBinfo)
    {
        if (DOBissueExists)
        {
            if (dates.fullyCredible && !globals->globalDr->getDOBcredibility())
                fullyCredible = true;
            else
                fullyCredible = false;
        }
        else
            fullyCredible = dates.fullyCredible;

        if (fullyCredible)
            forceDOBoverride = true;

        globals->globalDr->setDOB(dates.potentialDOB, forceDOBoverride, fullyCredible);
    }

    if (newYODinfo)
    {
        if (YODissueExists)
        {
            if (dates.fullyCredible && !globals->globalDr->getYODcredibility())
                fullyCredible = true;
            else
                fullyCredible = false;
        }
        else
            fullyCredible = dates.fullyCredible;

        if (fullyCredible)
            forceYODoverride = true;

        globals->globalDr->setYOD(static_cast<unsigned int>(dates.potentialYOD), forceYODoverride, fullyCredible);
    }

    if (newYOBinfo)
    {
        if (YOBissueExists)
        {
            if (dates.fullyCredible && !globals->globalDr->getYOBcredibility())
                fullyCredible = true;
            else
                fullyCredible = false;
        }
        else
            fullyCredible = dates.fullyCredible;

        if (YOBindirectIssueExists)
        {
            if (dates.fullyCredible && !globals->globalDr->getAgeAtDeathCredibility())
                fullyCredible = true;
            else
                fullyCredible = false;
        }
        else
            fullyCredible = dates.fullyCredible;

        if (fullyCredible)
            forceYOBoverride = true;

        globals->globalDr->setYOB(static_cast<unsigned int>(dates.potentialYOB), forceYOBoverride, fullyCredible);
    }

    return newDODinfo || newDOBinfo || newYODinfo || newYOBinfo;
}

bool readObit::missingDateInfo() const
{
    return (globals->globalDr->missingDOB() || globals->globalDr->missingDOD());
}

void readObit::addressUnknownGender()
{
    // If gender unknown, check first three sentences for gender titles
    bool unknownGender = (globals->globalDr->getGender() == genderUnknown);
    if (unknownGender)
    {
        uc = blockedContent.select(globals->globalDr->getTitleKey());
        uc.beg();

        OQString word;
        GENDER potentialGender = genderUnknown;
        unsigned int i = 0;
        unsigned int maxSentences = 3;  // information pulled from fourth or later sentence could be anything, so stop reading after two sentences
        while (unknownGender && !uc.isEOS() && (i < maxSentences))
        {
            sentence = uc.getSentence();
            word = sentence.getWord();
            if (word.isMaleTitle())
                potentialGender = Male;
            else
            {
                if (word.isFemaleTitle())
                    potentialGender = Female;
            }

            if (potentialGender != genderUnknown)
            {
                word = sentence.getWord();
                if (globals->globalDr->isAFirstName(word))
                {
                    globals->globalDr->setGender(potentialGender);
                    unknownGender = false;
                }
            }
            i++;
        }
    }

    // If gender unknown, but altName exists, assume female
    if (globals->globalDr->getGender() == genderUnknown)
    {
        if (globals->globalDr->getLastNameAlt1().getLength() > 0)
            globals->globalDr->setGender(Female);
    }

    // Run database searches to determine or validate gender if necessary
    double unisex = genderLookup(globals->globalDr, globals);

    if (globals->globalDr->getGender() == genderUnknown)
    {
        if (unisex >= 0.9)
            globals->globalDr->setGender(Male);
        else
        {
            if (unisex <= 0.1)
                globals->globalDr->setGender(Female);
        }
    }
    else    // validate
    {
        PQString writtenGender, errMsg;
        GENDER gender = globals->globalDr->getGender();
        if (gender == Male)
            writtenGender = PQString("Male");
        else
            writtenGender = PQString("Female");

        if (((unisex >= 0.9) && (gender == Female)) || ((unisex <= 0.1) && (gender == Male)))
        {
            errMsg << "Need to verify gender manually, words say " << writtenGender;
            errMsg << " but names imply opposite for: " << globals->globalDr->getURL();
            globals->logMsg(ErrorRecord, errMsg);
        }
    }
}
