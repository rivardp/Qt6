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
    bool SetStyle, SetLanguage, ObitTitle, ObitGender, ObitDates, ObitText, ReadStructured, ReadUnstructured, TitleInfo, LookupFields;
    bool StructuredInfoFullyCredible, ReadMessages, ReadPublishDate;
    int runTypes;
    PQString errMsg;
    godReference = false;

    // General strategy is to start with low hanging fruit and get more involved if necessary. Steps are as follows:
    // 1.  Read in title to pick off a name that can be relied on to identify deceased
    // 2.  Read the unstructured obit to determine language and gender if possible
    // 3.  Read in any predefined fields
    // 4.  Read in any structured data
    // 5.  Read in any information contained in headers or titles
    // 6.  Finish with read of unstructured obit


    // Determine which functions must be run - basic package is 2852
    //    1 SetStyle
    //    2 SetLanguage
    //    4 ObitTitle
    //    8 ObitGender
    //   16 ObitDates
    //   32 ObitText
    //   64 LookupFields
    //  128 ReadMessages
    //  256 ReadStructured
    //  512 StructuredInfoFullyCredible
    // 1024 TitleInfo
    // 2048 ReadUnstructured
    // 4096 ReadPublishDate

    SetStyle = false;
    SetLanguage = false;
    ObitTitle = false;
    ObitGender = false;
    ObitDates = false;
    ObitText = false;
    LookupFields = false;
    ReadMessages = false;
    ReadStructured = false;
    StructuredInfoFullyCredible = false;
    TitleInfo = false;
    ReadUnstructured = false;
    ReadPublishDate = false;

    runTypes = readRunTypes(globals, globals->globalDr->getProvider(), globals->globalDr->getProviderKey());
    if ((runTypes & 1) == 1)
        SetStyle = true;
    if ((runTypes & 2) == 2)
        SetLanguage = true;
    if ((runTypes & 4) == 4)
        ObitTitle = true;
    if ((runTypes & 8) == 8)
        ObitGender = true;
    if ((runTypes & 16) == 16)
        ObitDates = true;
    if ((runTypes & 32) == 32)
        ObitText = true;
    if ((runTypes & 64) == 64)
        LookupFields = true;
    if ((runTypes & 128) == 128)
        ReadMessages = true;
    if ((runTypes & 256) == 256)
        ReadStructured = true;
    if ((runTypes & 512) == 512)
        StructuredInfoFullyCredible = true;
    if ((runTypes & 1024) == 1024)
        TitleInfo = true;
    if ((runTypes & 2048) == 2048)
        ReadUnstructured = true;
    if ((runTypes & 4096) == 4096)
        ReadPublishDate = true;

    if (globals->globalDr->getID() == QString("9301126"))
    {
        int dummyBreak = 0;
    }


    // Execute based on settings
    if (SetStyle)
        readStyle();
    if (SetLanguage)
        readLanguage();
    if (ObitTitle)
        readInObitTitle();
    if (ObitGender)
        readGender();
    if (ObitDates)
        readInObitDates();
    if (ObitText)
    {
        readInObitText();
        runStdProcessing();
        cleanAndExtract();
        // Outputs
        // uc               => Full processed text
        // cleadedUpUC      => uc with unnecessary words dropped
        // justInitialNames => The start of the uc comprised only of names

        uc.splitIntoSentences(mcn.listOfFirstWords);
        cleanedUpUC.splitIntoSentences();
        determinePotentialFirstName();
        addressMissingGender();
        readServiceDate();
    }
    if (ReadPublishDate)
        readPublishDate();
    if (ReadMessages)
        readMessages();
    if (ReadStructured)
        readStructuredContent();
    if (TitleInfo)
        readTitleInfo();
    if ((globals->globalDr->getGender() != Male) && !globals->globalDr->getNeeEtAlEncountered()
                                                 && (globals->globalDr->getMaidenNames().size() == 0))
        readParentsLastName();
    if (ReadUnstructured)
         readUnstructuredContent();
    if (1)
    {
        globals->globalDr->setMinMaxDOB();
        globals->globalDr->runDateValidations();
        runNameValidations();
        runGenderValidation();
        runRecordValidation();
    }
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
        if (moveTo("\"locale\":"))
        {
            langText = readNextBetween(QUOTES);
            langText = langText.left(2);

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

    case FosterMcGarvey:
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

void readObit::readGender()
{
    OQString field;
    GENDER gender = genderUnknown;
    beg();

    switch(globals->globalDr->getProvider())
    {
    case FuneralOne:
        if (moveTo("itemprop=\"gender\""))
        {
            field = readNextBetween(BRACKETS);
            if (field == OQString("Male"))
                gender = Male;
            if (field == OQString("Female"))
                gender = Female;
        }
        break;

    default:
        break;
    }

    globals->globalDr->setGender(gender);

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
        if (consecutiveMovesTo(500, "ObitTextHtml", ">"))
            uc = getUntil("<!--  -->");
        break;

    case Passages:
        if (moveTo("class=\"obituary_body\""))
        {
            uc = getUntil("As published in");
            uc.dropRight(15);
        }
        break;

    case LifeMoments:
        break;

    case Batesville:
        switch(style)
        {
        case 0:
            if (moveTo("obit_name"))
            {
                if (consecutiveMovesTo(100, "s:272", "liqpLiquid -->", "<p>"))
                    uc = getUntil("<style>", 5000, false);
            }

            // Do backup reads if necessary (jpeg or no content)
            if (uc.getLength() == 0)
            {
                beg();
                if (consecutiveMovesTo(100, "s:130", "liqpLiquid -->", "<p>"))
                    uc = getUntil("<style>", 5000, false);

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

        case 2:
            if (moveTo("class=\"obitBody\">"))
                uc = getUntil("</div>");
            break;

        case 3:
            if (moveTo("class=\"obit-content\">"))
                uc = getUntil("</div>");
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
                if (consecutiveMovesTo(500, "short-bio text", ">"))
                    uc = getUntil("</div>");
            }
        }
        break;

    case FosterMcGarvey:
        if (moveTo("<h1>Obituary</h1>"))
            uc = getUntil("</div>");
        else
        {
            beg();
            if (consecutiveMovesTo(500, ">Obituary<", "class=\"Text\">"))
                uc = getUntil("</td>");
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

            // Verify if a backup read should be attempted
            checkContent = uc.simplify();
            checkContent.removeHTMLtags();
            if (checkContent.getLength() < 10)
            {
                beg();
                if (consecutiveMovesTo(50, target, ">"))
                {
                    moveTo("</div>");
                    uc = getUntil("</div>");
                }

                // Deal with some really old formats (pre 2010)
                checkContent = uc.simplify();
                checkContent.removeHTMLtags();
                if (checkContent.getLength() < 10)
                {
                    beg();
                    if (consecutiveMovesTo(100, "id=\"obituary\"", "<div>"))
                        uc = getUntil("</div>");
                }
            }
        }

        break;

    case Frazer:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(100, "id=\"descr\"", "itemprop=\"description\">"))
                uc = getUntil("</p>");
            break;

        case 1:
        case 2:
            if (consecutiveMovesTo(500, "MaterialData.user =", "fullDescription:", "\'"))
            {
                uc = getUntil("showDescription:");
                uc.dropRight(uc.getLength() - static_cast<unsigned int>(uc.findPosition(PQString("\',"), -1)));
            }
            break;

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
        if (consecutiveMovesTo(300, "\"obituary-text\"", "Obituary of", "</h1>"))
        {
            unsigned int position = getPosition();
            if (consecutiveMovesTo(75, "<p>", "B I O G R A P H Y", "</p>"))
                uc = getUntil("</div>");
            else
            {
                beg();
                forward(position);
                uc = getUntil("obituary-button-row");
            }
        }
        break;

    case WordPress:

        if (moveTo("article id=\"post-"))
        {
            if (consecutiveMovesTo(1500, "meta", "class=\"entry-content\">"))
                uc = getUntil("<!-- .entry-content -->");
        }
        break;

    case FrontRunner:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(1000, "bom-obituary-wrapper", "<div", ">"))
                uc = getUntil("</div>");
            else
            {
                beg();
                if (consecutiveMovesTo(200, "og:title", "og:description", "content=\""))
                    uc = getUntil("\">");
            }

            break;

        case 1:
            if (consecutiveMovesTo(2000, "class=\"obituary-title\"", "<div>"))
                uc = getUntil("</div>");
            break;

        case 2:
        case 3:
            if (moveTo(">Obituary<"))
            {
                if (moveTo("class=\"read-more-content\">"))
                    uc = getUntil("</div>");
            }
        }
        break;
    }

    case FuneralOne:
        if (moveTo("class=\"header-obituary\""))
            uc = getUntil("<a href=");
        break;

    case WebStorm:
        switch (style)
        {
        case 0:
            if (consecutiveMovesTo(500, "class=\"fancy\"", "</div>"))
                uc = getUntil("<div id");
            break;

        case 1:
            if (consecutiveMovesTo(200, ">In Loving Memory Of<", ">", ">", ">"))
                uc = getUntil("<div id");
            break;
        }
        break;

    case GasLamp:
        if (moveTo("class=\"obit_text\">"))
            uc = getUntil("<!-- /.obit-border -->");
        break;

    case ClickTributes:
        if (consecutiveMovesTo(500, "class=\"ct-tab-content\"", "Print Obituary"))
            uc = getUntil("<!--/ct-tab-obituary-->");
        break;

    case ConnellyMcKinley:
        if (consecutiveMovesTo(4000, "original-source", "</div>"))
            uc = getUntil("Share on Google Plus<");
        break;

    case Arbor:
        switch(style)
        {
        case 0:
            if (moveTo("div itemprop=\"description\""))
                uc = getUntil("</div>");
            break;

        case 1:
            if(moveTo("<div class=\"rte\">"))
                uc = getUntil("</section>");
            break;
        }
        break;

    case SiteWyze:
        if (consecutiveMovesTo(2000, "post-content", "Date:"))
        {
            if(moveToEarliestOf("<p", "<div style"))
            {
                moveTo(">");
                uc = getUntil("announcement-sidebar\'>");
            }
        }
        break;

    case ThinkingThru:
    {
        QString targetText;
        if (globals->globalDr->getDOB().isValid())
            targetText = globals->globalDr->getDOB().toString("MMMM dd, yyyy");
        else
        {
            if (globals->globalDr->getDOD().isValid())
                targetText = globals->globalDr->getDOD().toString("MMMM dd, yyyy");
            else
                targetText = QString("<!-- .et_pb_text -->");
        }

        if (moveTo(targetText))
        {
            moveTo(">Full Obituary<");
            backward(1);
            uc = getUntil("</div>");
        }
        break;
    }

    case Codesign:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(100, "class=\"obt_info\"", ">"))
                uc = getUntil("Print Obituary<");
            break;

        case 1:
            if (moveTo("<!-- #TRX_REVIEWS_PLACEHOLDER# -->"))
                uc = getUntil("</section>");
            break;
        }
    }
        break;

    case Shape5:
    {
        switch (style)
        {
        case 0:
            if (consecutiveMovesTo(100000, "<![endif]-->", "class=\"MsoNormal\"", ">"))
                uc = getUntil("</div>");
            break;

        case 1:
            if (consecutiveMovesTo(50, "articleBody", ">"))
                uc = getUntil("</div>");
            break;
        }

        break;
    }

    case TributeArchive:
    {
        if (moveTo("details-copy\">"))
        {
            unsigned int startingPosition, endingPosition, distance;
            startingPosition = getPosition();
            moveTo("<br>");
            endingPosition = getPosition();
            distance = endingPosition - startingPosition;
            backward(distance);
            if (distance < 50)
            {
                // Addresses cases where name appears first, otherwise start reading immediately
                //conditionalMoveTo("<br>", "</div>", 0);
            }
            uc = getUntil("details-published\">");
        }

        break;
    }

    case YAS:
    {
        if (moveTo("class=\"post-content\">"))
           uc = getUntil("</div>");
    }
        break;

    case WFFH:
    {
        if (consecutiveMovesTo(500, "lifestorycontent", "editable-text", ">"))
           uc = getUntil("</div>");
    }
        break;

    case FHW:
    {
        if (consecutiveMovesTo(1000, "<!-- obit body tab -->", "id=\"obit_text\"", ">"))
           uc = getUntil("<!-- video stream link -->");
    }
        break;

    case Specialty:
    {
        if (consecutiveMovesTo(10, "class=\"contentPanel\"", ">"))
            uc = getUntil("<a id=\"guestbooks\"");
    }
        break;

    /*****************/
    /*   One Offs    */
    /*****************/

    case MikePurdy:
        if (moveTo("class=\"storycontent\""))
            uc = getUntil("<!-- You can start editing here. -->");
        break;

    case BowRiver:
        if (consecutiveMovesTo(100, "id=\"obitbody\"", ">"))
            uc = getUntil("Send Condolence");
        break;

    case Serenity:
        if (consecutiveMovesTo(50, "page-article-story", ">"))
            uc = getUntil("page-article-footer\">");
        break;

    case McInnis:
        switch (style)
        {
        case 0:
            if (moveTo("class=\"entry-content\">"))
            {
                uc = getUntil("title=\"Print This Obituary", 4000);
                uc += OQString(">");
            }
            break;

        case 1:
            if (consecutiveMovesTo(300, "obituary-text", "</h1>"))
                uc = getUntil("</div>");
            break;
        }
        break;

    case Sturgeon:
        switch (style)
        {
        case 0:
            if (consecutiveMovesTo(10000, "Details", "Published", "</div>"))
                uc = getUntil("<li class=\"previous\">", 5000);
            break;

        case 1:
            if (consecutiveMovesTo(600000, "Details", "<img src=", "</span>"))
                uc = getUntil("<li class=\"previous\">", 5000);
            break;
        }

        break;

    case CornerStone:
        if (moveTo("class=\"editor\""))
            uc = getUntil("</section>");
        uc.replace(QString("\\\\\\"), QString(""));
        break;

    case Pierson:
        switch(style)
        {
        case 0:
            if (moveTo("itemprop=\"description\""))
                uc = getUntil("</div>");
            break;

        case 1:
            if (moveTo("fullDescription:"))
            {
                uc = getUntil("showDescription:");
                uc.dropRight(1);
                uc.cleanUpEnds();
                uc.removeBookEnds(QUOTES);
            }
            break;
        }
        break;

    case Trinity:
        if (moveTo(">Print Obituary"))
            uc = getUntil(">Print Obituary");
        uc.dropRight(14);
        break;

    case CelebrateLife:
        if (consecutiveMovesTo(2000, "class=\"main-wrap\"", "Obituary of", "class=\"paragraph\"", ">"))
            uc = getUntil("</table>");
        break;

    case Funks:
        if (consecutiveMovesTo(2000, "content-main", "class=\"entry\">"))
            uc = getUntil("<!-- You can start editing here. -->");
        break;

    case WowFactor:
        if (consecutiveMovesTo(3000, "id=\"main-content\"", "</h3>", "et_pb_text_inner", ">"))
            uc = getUntil("</div>");
        break;

    case Dalmeny:
        if (consecutiveMovesTo(2000, "class=\"content\"", "</p>"))
            uc = getUntil("<!--StartFragment-->");
        break;

    case Hansons:
        if (consecutiveMovesTo(100, "obituaries_text", ">"))
            uc = getUntil("</div>");
        break;

    case Martens:
        if (moveTo("<!--BLOG_SUMMARY_END-->"))
            uc = getUntil("<div class=\"blog-social");
        break;

    case Shine:
        if (consecutiveMovesTo(100, ">Obituary<", ">"))
            uc = getUntil("</div>");
        break;

    default:
        PQString errMsg;
        errMsg << "Reading in of obit text not coded for: " << globals->globalDr->getProvider();
        globals->logMsg(ErrorRunTime, errMsg);

    }
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

    case LifeMoments:
        break;

    case SIDS:
        break;

    case Batesville:
        switch(style)
        {
        case 3:
        case 2:
            if (consecutiveMovesTo(100, "<title>", "Obituary | "))
                titleStream = getUntil(" |");
            break;

        default:
            if (moveTo("<title>"))
                titleStream = getUntil("</title>");
            break;
        }
        break;

    case BrowseAll:
    case Frazer:
    case Alternatives:
    case FuneralTech:
    case WordPress:
    case FrontRunner:
    case Serenity:
    case Sturgeon:
    case Shape5:
    case Dalmeny:
    case Shine:
    case FHW:
    case Specialty:
        if (moveTo("<title>"))
            titleStream = getUntil("</title>");
        break;

    case DignityMemorial:
    case Martens:
        if (consecutiveMovesTo(20, "og:title", "content="))
            titleStream = readNextBetween(DOUBLE_QUOTES);
        break;

    case FosterMcGarvey:
        if (moveTo("\"obit_name\">"))
        {
            titleStream = getUntil("</td>");
            titleStream.removeHTMLtags(false);
            titleStream.simplify();
        }
        break;

    case CFS:
        if (consecutiveMovesTo(100, "<title>", "Obituary for "))
            titleStream = getUntil("|");
        break;

    case FuneralOne:
        if (moveTo("itemprop=\"givenName\""))
            titleStream = readNextBetween(BRACKETS);
        break;

    case WebStorm:
        switch (style)
        {
        case 0:
            if (moveTo("class=\"fancy\""))
                titleStream = readNextBetween(BRACKETS);
            break;

        case 1:
            if (moveTo("<title>"))
                titleStream = getUntil("</title>");
            break;
        }
        break;

    case GasLamp:
        if (consecutiveMovesTo(100, "<title>", "Online Tribute for "))
            titleStream = getUntil("|");
        break;

    case ClickTributes:
        if (consecutiveMovesTo(100, "pagetitle", ">Obituary - "))
            titleStream = getUntil("<");
        break;

    case ConnellyMcKinley:
    case Arbor:
    case TributeArchive:
    case Funks:
    case WowFactor:
        if (moveTo("<title>"))
            titleStream = getUntil("|");
        break;

    case SiteWyze:
    {
        if (consecutiveMovesTo(2000, "post-content", "<h2"))
            titleStream = readNextBetween(BRACKETS);
        break;
    }

    case ThinkingThru:
    {
        QString targetText;
        if (globals->globalDr->getDOB().isValid())
            targetText = globals->globalDr->getDOB().toString("MMMM dd, yyyy");
        else
        {
            if (globals->globalDr->getDOD().isValid())
                targetText = globals->globalDr->getDOD().toString("MMMM dd, yyyy");
            else
                targetText = QString("<!-- .et_pb_text -->");
        }

        if (moveTo(targetText))
        {
            if(moveBackwardTo("<strong"))
                titleStream = readNextBetween(BRACKETS);
        }
        break;
    }

    case Passages:
    case Codesign:
    case Hansons:
        if (moveTo("<title>"))
            titleStream = getUntil(" - ");
        break;

    case YAS:
        if (consecutiveMovesTo(100, "og:title", "content=\""))
            titleStream = getUntil(" - ");
        break;

    case WFFH:
        if (consecutiveMovesTo(500, "persondetails-area", "personName", "personFirst"))
            titleStream = readNextBetween(BRACKETS);
        break;


    /*****************/
    /*   One Offs    */
    /*****************/

    case MikePurdy:
        if (consecutiveMovesTo(200, "<title>", "&raquo;", "&raquo;"))
        {
            titleStream = getUntil("</title>");
            titleStream.simplify();
        }
        break;

    case BowRiver:
        if (consecutiveMovesTo(100, "<title>", ":"))
        {
            titleStream = getUntil("</title>");
            titleStream.simplify();
        }
        break;

    case McInnis:
        switch(style)
        {
        case 0:
            if (moveTo("<title>"))
            {
                titleStream = getUntil(" - McInnis");
                titleStream.simplify();
            }
            break;

        case 1:
            if (consecutiveMovesTo(100, "<title>", "Obituary of "))
                titleStream = getUntil("|");
            break;
        }
        break;

    case CornerStone:
        if (consecutiveMovesTo(100, "<title>", "Obituary of "))
            titleStream = getUntil("<");
        break;

    case Pierson:
        if (consecutiveMovesTo(100, "og:image:alt", "content="))
            titleStream = readNextBetween(QUOTES);
        break;

    case Trinity:
        if (moveTo("<title>"))
            titleStream = getUntil(" - ");
        break;

    case CelebrateLife:
        if (moveTo("<title>"))
            titleStream = getUntil(" - CELEBRATE");
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
        while (!titleStream.isEOS() && ((titleText.getLength() < 2) || titleText.isRecognized()))
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

    case FosterMcGarvey:
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
                globals->globalDr->wi.dateFlag = 2;
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
                globals->globalDr->wi.dateFlag = 3;
            }
        }
    }
}

void readObit::readLookupFields(const SEARCHPARAM &sp, const fieldNames &FieldNames)
{
    Q_UNUSED(sp)
    Q_UNUSED(FieldNames)

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
        OQString checkWord, tempWord, nextWord;
        bool hasBookEnds, isAboriginal;
        bool potentialGenderMarker = false;
        GENDER potentialGender = globals->globalDr->getGender();
        QList<QString> nameList;

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
            checkWord.removeEnding(PUNCTUATION);
            hasBookEnds = checkWord.removeBookEnds();
            tempWord = checkWord;
            isAboriginal = false;
            if (checkWord.isAboriginalName())
            {
                nextWord = checkStream.peekAtWord();
                if (nextWord.isAboriginalName())
                    isAboriginal = true;
            }

            while (checkWord.isCompoundName() || (isAboriginal && !hasBookEnds))
            {
                checkWord = checkStream.getWord().getString();
                tempWord += OQString(" ");
                tempWord += checkWord;
                isAboriginal = isAboriginal && checkWord.isAboriginalName();
            }
            checkWord = tempWord;
            nameType = globals->globalDr->isAName(checkWord);
            if ((nameType >= 1) && (nameType <= 3))
            {
                noMatch = false;
                if (potentialGenderMarker)
                    globals->globalDr->setGender(potentialGender);
            }
            else
                nameList.append(checkWord.getString());
            i++;
        }

        // If not matched, check if structured name was a nickname
        if (noMatch)
        {
            bool matched = false;
            QString firstName = globals->globalDr->getFirstName().lower().getString();
            bool isNickName = nicknameLookup(firstName, globals);
            if (isNickName)
                matched = nickNameInList(firstName, nameList, globals);
            if (matched)
                noMatch = false;
        }

        if (noMatch || (cutOff < 2))
            justInitialNamesUC.clear();
    }

    // Replace justInitialNamesUC if appropriate
    if ((justInitialNamesUC.getLength() == 0) && (uc.getLength() > 1))
    {
        unstructuredContent sentence;
        OQString originalWord, checkWord, lastWord, doubleWord;
        unsigned int position = 0;
        unsigned int wordLength = 0;
        unsigned int numWords = 0;
        bool notRecognized = true;
        bool invalidated = false;

        uc.beg();
        while ((sentence.getLength() == 0) || sentence.isJustDates())
        {
            sentence = uc.getSentence();
        }

        while (notRecognized && !invalidated && !sentence.isEOS() && (numWords < 15))
        {
            originalWord = sentence.getWord(true);
            checkWord = originalWord;
            wordLength = checkWord.getLength();
            checkWord.removeBookEnds();
            checkWord.cleanUpEnds();
            checkWord.removeEnding(",");
            checkWord.removeEnding(":");
            notRecognized = checkWord.isRecognized();
            position = sentence.getPosition();
            numWords++;

            doubleWord = lastWord + OQString(" ") + checkWord;
            lastWord = checkWord;
            if (doubleWord.areRelationshipWords(globals->globalDr->getGender(), globals->globalDr->getLanguage()))
                    invalidated = true;
        }

        NAMETYPE nameType = globals->globalDr->isAName(checkWord);
        if ((nameType >= 1) && (nameType <= 3))
        {
            justInitialNamesUC.clear();
            justInitialNamesUC += originalWord;
            while (!sentence.isEOS())
            {
                originalWord = sentence.getWord(true);
                if (originalWord != OQString(","))
                    justInitialNamesUC += OQString(" ");
                justInitialNamesUC += originalWord;
            }
            justInitialNamesUC.pickOffNames();
        }
        else
        {
            bool tryNextWord = givenNameLookup(checkWord.lower().getString(), globals, globals->globalDr->getGender());
            if (tryNextWord)
            {
                originalWord = sentence.getWord(true);
                checkWord = originalWord;
                checkWord.removeBookEnds();
                nameType = globals->globalDr->isAName(checkWord);
                if ((nameType >= 1) && (nameType <= 3))
                {
                    justInitialNamesUC.clear();
                    justInitialNamesUC += originalWord;
                    while (!sentence.isEOS())
                    {
                        originalWord = sentence.getWord(true);
                        if (originalWord != OQString(","))
                            justInitialNamesUC += OQString(" ");
                        justInitialNamesUC += originalWord;
                    }
                    justInitialNamesUC.pickOffNames();
                }
            }
        }
    }

    // If still unsucessful, try eliminating common beginning to obituaries prior to full name
    if ((justInitialNamesUC.getLength() == 0) && (uc.getLength() > 1))
    {
        OQString word, lastWord, doubleWord;
        LANGUAGE lang = globals->globalDr->getLanguage();

        OQStream tempStream(uc.left(300));
        tempStream.beg();
        unstructuredContent sentence = tempStream.getSentence(lang);
        if (sentence.isJustDates())
            sentence = tempStream.getSentence(lang);

        bool started = false;
        bool invalidated = false;
        while (!sentence.isEOS() && !invalidated)
        {
            word = sentence.getWord(true);
            doubleWord = lastWord + OQString(" ") + word;
            lastWord = word;
            if (doubleWord.areRelationshipWords(globals->globalDr->getGender(), globals->globalDr->getLanguage()))
                    invalidated = true;
            if (!started && !invalidated)
                started = globals->globalDr->isASavedName(word) || word.isTitle();
            if (started)
            {
                justInitialNamesUC += OQString(" ");
                justInitialNamesUC += word;
            }
        }
        if (invalidated)
            justInitialNamesUC.clear();
        else
            justInitialNamesUC.pickOffNames();
    }

    // If still nothing, pull names from titleStream
    if (justInitialNamesUC.getLength() == 0)
    {
        OQStream word;
        bool keepGoing = true;
        bool started = false;
        OQStream tempStream(globals->globalDr->getTitle());
        while (!tempStream.isEOS() && keepGoing)
        {
            word = tempStream.getWord();
            if (word == globals->globalDr->getTitleKey())
                started = true;
            if ((word == OQString("|")) || (word == OQString("-")))
                keepGoing = false;
            if (started && keepGoing)
            {
                justInitialNamesUC += word;
                justInitialNamesUC += OQString(" ");
            }
        }
        justInitialNamesUC.pickOffNames();
    }

    justInitialNamesUC.removeEnding(".");

    // Adjust for "Smith: John Harry"
    if (justInitialNamesUC.contains(":") && !justInitialNamesUC.contains(","))
        justInitialNamesUC.replace(":", ",");
}

void readObit::readUnstructuredContent()
{
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

    // Look for middle name(s), if any, as a single string and sets other names along the way if they are encountered
    globals->globalDr->setMiddleNames(justInitialNamesUC.processAllNames());

    // Look for most common name used within the obituary
    globals->globalDr->setAlternates(mcn.readMostCommonName(globals));

    // Where middlename is used as a first name, add it as an AKA as well
    PQString tempName;
    tempName = globals->globalDr->getMiddleNameUsedAsFirstName();
    if (tempName.getLength() > 0)
        globals->globalDr->setFirstNames(tempName);

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
        // Ninth pass includes final attempts to pull incomplete information together
        // Use date published or funeral service date as proxy if still missing DOD
        // Finally, set min and max DOB if DOD is known and age at death is known

        fillInDatesFirstPass();
        if (missingDateInfo())
            fillInDatesSecondPass();
        if (missingDateInfo())
            fillInDatesThirdPass(uc);
        if (globals->globalDr->getYOB() == 0)
            fillInDatesFourthPass();
        if (missingDateInfo())
            fillInDatesFifthPass();
        if (globals->globalDr->missingDOB() || globals->globalDr->missingDOD())
            fillInDatesSixthPass();
        if (globals->globalDr->missingDOB() && !globals->globalDr->missingDOD())
            fillInDatesSeventhPass();
        if (globals->globalDr->missingDOB() && !globals->globalDr->missingDOD())
            fillInDatesEighthPass();
        if (globals->globalDr->missingDOB() || globals->globalDr->missingDOD())
            fillInDatesNinthPass();
        if ((globals->globalDr->getYOD() == 0) && (globals->globalDr->getDeemedYOD() > 0))
            globals->globalDr->setYOD(globals->globalDr->getDeemedYOD());

    }	// end getDates

    // Fill in missing info if possible
    if (globals->globalDr->getAgeAtDeath() == 0)
    {
        if (globals->globalDr->getDOB().isValid() && globals->globalDr->getDOD().isValid())
            globals->globalDr->setAgeAtDeath(static_cast<unsigned int>(elapse(globals->globalDr->getDOB(), globals->globalDr->getDOD())));
        globals->globalDr->setMinMaxDOB();
    }

    if ((globals->globalDr->getAgeAtDeath() > 0) && (globals->globalDr->getYOD() > 0) && (globals->globalDr->getYOB() == 0) && !globals->globalDr->getDOD().isValid())
    {
        globals->globalDr->setMinDOB(QDate(static_cast<int>(globals->globalDr->getYOD() - globals->globalDr->getAgeAtDeath() - 1), 1, 2));
        globals->globalDr->setMinDOB(QDate(static_cast<int>(globals->globalDr->getYOD() - globals->globalDr->getAgeAtDeath()), 12, 31));
    }
}

void readObit::readMessages()
{
    QDate commentDate, tempDate;
    QList<QDate> dateList;
    OQString cleanString;
    unstructuredContent sentence, tempUC;
    //int tempYOD;
    //unsigned int numDates;

    // Looking specifically for obits without valid DOD which have DOD reference at beginning that excludes year, which is instead pulled from comment date
    if (globals->globalDr->getDOD().isValid() || (globals->globalDr->getYOD() > 0))
        return;

    beg();

    // Pull comment date
    switch(globals->globalDr->getProvider())
    {
    default:
        break;

    case DignityMemorial:
        commentDate = globals->today;
        while (moveTo("class=\"memory-date\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempDate = tempUC.readDateField();
            if (tempDate < commentDate)
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case FrontRunner:
        commentDate = globals->today;
        while (moveTo("class=\"tributePost-postDate\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempDate = tempUC.readDateField();
            if (tempDate < commentDate)
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case ConnellyMcKinley:
        commentDate = globals->today;
        while (consecutiveMovesTo(200, "comment-time-link", ">"))
        {
            tempUC = getUntil(" at");
            tempDate = tempUC.readDateField();
            if (tempDate < commentDate)
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case McInnis:
        if (consecutiveMovesTo(200, "comment-time-link", ">"))
        {
            tempUC = getUntil(" at");
            commentDate = tempUC.readDateField();
        }
        break;

    case WowFactor:
        commentDate = globals->today;
        while (consecutiveMovesTo(100, "comment_date", "on"))
        {
            tempUC = getUntil(" at");
            tempDate = tempUC.readDateField();
            if (tempDate < commentDate)
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Dalmeny:
        if (moveTo("comment_date_value"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.removeHTMLtags();
            tempUC.simplify();
            commentDate = uc.readDateField();
        }
        break;

    case Hansons:
        if (consecutiveMovesTo(100, "fa-calendar-alt", "</i"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.removeHTMLtags();
            tempUC.simplify();
            commentDate = uc.readDateField();
        }
        break;

    case Martens:
        if (consecutiveMovesTo(100, "blog-date", "date-text"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.removeHTMLtags();
            tempUC.simplify();
            commentDate = tempUC.readDateField(doMDY);
        }
        break;

    }

    /*// Resume std processing
    if (commentDate.isValid())
    {
        tempYOD = commentDate.year();
        cleanedUpUC.beg();
        sentence = cleanedUpUC.getSentence();
        numDates = sentence.pullOutDates(globals->globalDr->getLanguage(), dateList, 2, cleanString, false);
        if (numDates == 0)
        {
            sentence = cleanedUpUC.getSentence();
            numDates = sentence.pullOutDates(globals->globalDr->getLanguage(), dateList, 2, cleanString, false);
        }
        if (numDates > 9900)
        {
            // Where year was pulled from comment, assume sentence reads "... passed away on May 23rd at the age of 65"
            unstructuredContent tempContent(cleanString);
            cleanString.clear();
            QDate tempDOD;
            tempDOD = tempContent.fillInDate(globals->globalDr->getLanguage(), cleanString, tempYOD);
            if (tempDOD.isValid() && (tempDOD.month() > commentDate.month()))
            {
                tempYOD--;
                tempDOD = tempContent.fillInDate(globals->globalDr->getLanguage(), cleanString, tempYOD);
            }
            if (tempDOD.isValid())
            {
                globals->globalDr->setDOD(tempDOD);
                if (globals->globalDr->getAgeAtDeath() == 0)
                    sentence.sentenceReadAgeAtDeath();
                if (globals->globalDr->getDOB().isValid())
                    globals->globalDr->setAgeAtDeath(static_cast<unsigned int>(elapse(globals->globalDr->getDOB(), globals->globalDr->getDOD())));
            }
        }

        if (!globals->globalDr->getDOD().isValid())
        {
            if (commentDate.month() >= 2)
                globals->globalDr->setYOD(static_cast<unsigned int>(commentDate.year()));
        }
    }*/

    if (commentDate.isValid())
        globals->globalDr->setCommentDate(commentDate);
}

void readObit::readPublishDate()
{
    if (globals->globalDr->getPublishDate().isValid() && (globals->globalDr->getPublishDate() > QDate(1900,1,1)))
        return;

    beg();
    QDate pubDate;
    unstructuredContent tempUC;

    // Pull publish date
    switch(globals->globalDr->getProvider())
    {
    default:
        break;

    case Legacy:
        if (moveTo(">Published in "))
        {
            if (conditionalMoveTo("to ", "<", 0))
                tempUC = getUntil("<");
            else
            {
                if (conditionalMoveTo("on ", "<", 0))
                        tempUC = getUntil("<");
            }
            if (tempUC.getLength() > 0)
            {
                tempUC.simplify();
                tempUC.replaceHTMLentities();
                pubDate = tempUC.readDateField();
            }
        }
        break;

    case CFS:
        if (moveTo("datePublished\":\""))
        {
            tempUC = getUntil("T");
            pubDate = tempUC.readDateField(doYMD);
        }
        break;

    case WordPress:
        if(consecutiveMovesTo(500, "date published", ">"))
        {
            backward();
            tempUC = readNextBetween(BRACKETS);
            pubDate = tempUC.readDateField();
        }
        break;

    case TributeArchive:
        if (consecutiveMovesTo(200,"class=\"details-published\"", "Published on", "</span>"))
        {
            tempUC = getUntil("</div>");
            tempUC.simplify();
            pubDate = tempUC.readDateField();
        }
        break;

    case Sturgeon:
        if (moveTo("Published:"))
        {
            tempUC = getUntil("</div>");
            tempUC.simplify();
            pubDate = tempUC.readDateField(doDMY);
        }
        break;

    case Trinity:
        if (consecutiveMovesTo(100, "article:published_time", "content=\""))
        {
            tempUC = getUntil("T");
            tempUC.removeHTMLtags();
            tempUC.simplify();
            pubDate = tempUC.readDateField();
        }
        break;

    }

    if (pubDate.isValid())
        globals->globalDr->setPublishDate(pubDate);

}

void readObit::readServiceDate()
{
    int numSentences = uc.getNumSentences();
    int numChecked, numDates;
    unstructuredContent sentence;
    bool dateFound = false;
    LANGUAGE lang;
    QList<QDate> dateList;
    OQString cleanString;

    if (numSentences != -1)
    {
        numChecked = 1;
        lang = globals->globalDr->getLanguage();
        uc.beg();
        while ((numChecked <= 4) && (numSentences > 0) && !dateFound)
        {
            sentence = uc.getSentence(lang, numSentences);
            numDates = sentence.pullOutDates(lang, dateList, 2, cleanString, false, true);
            if ((numDates == 1) || ((numDates == 2) && (dateList[0] == dateList[1])))
            {
                dateFound = true;
                if (dateList[0].year() > 2000)
                    globals->globalDr->setDOS(dateList[0]);
            }
            else
            {
                if (numDates > 1)
                    numSentences = 0;
            }
            numChecked++;
            numSentences--;
        }
    }
}

bool readObit::fillInDatesStructured(unstructuredContent &uc, bool reliable)
{
    // This function is only designed to be called with a clean short stream of complete dates
    DATES dates;
    bool newInfoFound = false;
    uc.beg();

    dates = uc.readDOBandDOD(reliable);
    newInfoFound = processNewDateInfo(dates, 0);

    if (!newInfoFound && (globals->globalDr->missingDOB() || globals->globalDr->missingDOD()))
    {
        dates = uc.readYears(reliable);
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
    dates = uc.readDates(globals->globalDr->getLanguage(), limitWords);
    processNewDateInfo(dates, 1);
}

void readObit::fillInDatesSecondPass()
{
    // Looks for two complete years at beginning of content (YYYY - YYYY)
    DATES dates;

    dates = uc.readYears();
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

    dates = uc.contentReadBornYear(globals->globalDr->getLanguage());
    processNewDateInfo(dates, 4);
}

void readObit::fillInDatesFifthPass()
{
    // Look for two consecutive dates in any one sentence
    DATES dates;

    dates = uc.contentPullBackToBackDates(globals->globalDr->getLanguage());
    processNewDateInfo(dates, 5);
}


void readObit::fillInDatesSixthPass()
{
    // Look for age at death (in first two sentences only)
    // If a valid age at death (over 15) is located and a discrepancy in DODs is found (title/header vs text), DOD in text is used and dataRecord is updated
    unsigned int maxSentences = 2;
    uc.contentReadAgeAtDeath(maxSentences);
}

void readObit::fillInDatesSeventhPass()
{
    // Look for the first word in the cleanedUpUC to be a number
    uc.beg();
    if (globals->globalDr->missingDOB() && !globals->globalDr->missingDOD() && (globals->globalDr->getAgeAtDeath() == 0))
    {
        PQString word = uc.getWord();
        word.removeBookEnds(PARENTHESES);
        word.removeEnding(PUNCTUATION);
        if (word.isNumeric() && !word.isHyphenated())
        {
            // Eliminate possibility of a french date
            bool frenchDate = false;
            LANGUAGE lang = globals->globalDr->getLanguage();
            if (lang == french)
            {
                OQString nextWord = uc.peekAtWord();
                if (nextWord.isWrittenMonth(lang))
                    frenchDate = true;
            }

            if (!frenchDate)
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
}

void readObit::fillInDatesEighthPass()
{
    // Looks for key words and dates in sentences
    DATES dates;

    unsigned int maxSentences = 3;
    dates = uc.sentencePullOutDates(globals->globalDr->getLanguage(), maxSentences);
    processNewDateInfo(dates, 8);
}

void readObit::fillInDatesNinthPass()
{
    // Attempt to use incomplete or partial data
    unsigned int singleYear = globals->globalDr->getSingleYear();
    if (singleYear > 0)
    {
        QSqlQuery query;
        QSqlError error;

        QDate fhFirstObit, thresholdDate;
        unsigned int thresholdYear;
        PQString errorMessage;
        bool success;

        unsigned int providerID = globals->globalDr->getProvider();
        unsigned int providerKey = globals->globalDr->getProviderKey();

        success = query.prepare("SELECT fhFirstObit FROM funeralhomedata WHERE providerID = :fhProviderID AND providerKey = :fhProviderKey AND "
                                "(fhRunStatus = 1 OR fhRunstatus = 2 OR fhRunStatus = 100)");
        query.bindValue(":fhProviderID", QVariant(providerID));
        query.bindValue(":fhProviderKey", QVariant(providerKey));
        success = query.exec();

        if (!success || (query.size() != 1))
        {
            error = query.lastError();

            errorMessage << QString("SQL problem retrieving first obit date for:");
            errorMessage << providerID << PQString(" ") << providerKey;
            globals->logMsg(ErrorSQL, errorMessage, static_cast<int>(error.type()));
        }
        else
        {
            while (query.next())
            {
                fhFirstObit = query.value(0).toDate();
                if (!fhFirstObit.isValid())
                    fhFirstObit = globals->today;
            }
        }

        QDate EighteenYearOldDOB(globals->today.year()-18, globals->today.month(), globals->today.day());

        if (fhFirstObit < EighteenYearOldDOB)
            thresholdDate = EighteenYearOldDOB;
        else
            thresholdDate = fhFirstObit;
        thresholdYear = static_cast<unsigned int>(thresholdDate.year());

        if (singleYear >= thresholdYear)
        {
            // Assume we have correct DOD, update record and rerun limited readAgeAtDeath
            globals->globalDr->setYOD(singleYear);
            uc.beg();
            unstructuredContent sentence = uc.getSentence(globals->globalDr->getLanguage());
            sentence.sentenceReadAgeAtDeath(true);
        }
    }
}

void readObit::readStructuredContent()
{
    beg();
    bool fixHyphens, DOBfound, DODfound, primarySourceProcessed;
    unsigned int position, number;
    QDate qdate;
    QString target;
    OQString cleanString, word, fullWord;
    QString space(" ");
    QList<QString> purgeList;
    PQString name;
    unstructuredContent tempUC;

    DOBfound = false;
    DODfound = false;

    switch(globals->globalDr->getProvider())
    {
    case Legacy:

        // DOB
        if (conditionalMoveTo("\"dob\"", "lago", 0))
        {
            tempUC = readNextBetween(QUOTES);
            qdate = tempUC.readDateField(doMDY);
            if (qdate.isValid())
                globals->globalDr->setDOB(qdate, true, true);
        }

        // DOD
        if (conditionalMoveTo("\"dod\"", "lago", 0))
        {
            tempUC = readNextBetween(QUOTES);
            qdate = tempUC.readDateField(doMDY);
            if (qdate.isValid())
                globals->globalDr->setDOD(qdate, true, true);
        }

        // Read Name
        if (consecutiveMovesTo(200, "ObituaryHeader", "givenName"))
        {
            word = readNextBetween(BRACKETS);
            if (conditionalMoveTo("additionalName", "familyName", 0))
            {
                word += space;
                word += readNextBetween(BRACKETS);
            }
            if (moveTo("familyName"))
            {
                word += space;
                word += readNextBetween(BRACKETS);
            }
            tempUC = word;
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);

            // In some cases, can find (YYYY - YYYY)
            if (moveTo(" (", 20))
            {
                DATES dates;
                backward(1);
                tempUC = readNextBetween(PARENTHESES);
                dates = tempUC.readYears();

                if (dates.hasDateInfo())
                    processNewDateInfo(dates, 2);
            }
        }

        break;

    case Passages:
        if (moveTo("h1 class=\"details\""))
        {
            tempUC = readNextBetween(BRACKETS);
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);
        }

        if (conditionalMoveTo("Born:", "obituary_body", 0))
        {
            QList<QDate> dateList;
            tempUC = getUntil("<", 50, true);
            if (tempUC.pullOutDates(globals->globalDr->getLanguage(), dateList, 1, cleanString, true))
                 globals->globalDr->setDOB(dateList.takeFirst());
        }
        else
            beg();

        if (moveTo("Date of Passing:"))
        {
            QList<QDate> dateList;
            tempUC = getUntil("<", 50, true);
            if (tempUC.pullOutDates(globals->globalDr->getLanguage(), dateList, 1, cleanString, true))
                globals->globalDr->setDOD(dateList.takeFirst());
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
                tempUC = readNextBetween(BRACKETS);
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);

                // Read DOB and DOD in form (mmm dd, yyyy - mmm dd, yyyy)
                if (moveTo("lifespan"))
                {
                    tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);

                    // If still no go, assume that if field contains only a single date, then it is DOD
                    if ((globals->globalDr->getYOB() == 0) && (globals->globalDr->getYOD() == 0))
                    {
                        if (tempUC.getLength() <= 21)
                        {
                            QDate potentialDate = tempUC.readDateField();
                            if (potentialDate.isValid())
                                globals->globalDr->setDOD(potentialDate);
                        }
                    }
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(150, "\"text-container\"", "<h1>"))
            {
                // Read names (First Name first expected)
                tempUC = getUntil("</h1>", 100);
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);

                // Read DOB and DOD in form (mmm dd, yyyy - mmm dd, yyyy)
                if (moveTo("<h5>"))
                {
                    tempUC = getUntil("</h5>", 200);
                    tempUC.removeHTMLtags();
                    tempUC.unQuoteHTML();
                    fillInDatesStructured(tempUC);
                }
            }
            break;

        case 2:
            // Read full name
            if (consecutiveMovesTo(100, "keywords", "Obituary, "))
            {
                tempUC = getUntil(",");
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);
            }

            // No dates to be read as they would be in the JSON data if known
            break;

        case 3:
            // Read full name
            if (moveTo("class=\"obitHD\""))
            {
                tempUC = readNextBetween(BRACKETS);
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);
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
            tempUC = readNextBetween(BRACKETS);
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);

            // Read DOB and DOD in form (mmm dd, yyyy - mmm dd, yyyy)
            if (moveTo("obitlink\">"))
            {
                tempUC = readNextBetween(PARENTHESES);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case DignityMemorial:
        if (consecutiveMovesTo(100, ">OBITUARY<", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);
        }

        // Look for date information if necessary
        if (globals->globalDr->missingDOB() || globals->globalDr->missingDOD())
        {
            beg();
            moveTo("obituary-post-date");
            QString dateFormat("M/d/yyyy");
            QDate tempDate;
            loadValue(QString("obituary-dob"), tempDate, dateFormat);
            if (tempDate.isValid())
            {
                globals->globalDr->setDOB(tempDate);
                tempDate.setDate(0,0,0);
            }
            loadValue(QString("obituary-dod"), tempDate, dateFormat);
            if (tempDate.isValid())
                globals->globalDr->setDOD(tempDate);
        }

       /*     // If still missing, take shot at YYYY - YYYY
            if (globals->globalDr->missingDOB() || globals->globalDr->missingDOD())
            {
                //beg();
            }
        }*/
        /*if (consecutiveMovesTo(500, "class=\"w-content", ">OBITUARY<", "itemprop=\"name\""))
        {
            // Read names (First Name first expected)
            tempUC = readNextBetween(BRACKETS);
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);

            // Read DOB and DOD in form (mmm dd, yyyy - mmm dd, yyyy), split into two separate fields
            // Error messages will be logged if new dates are inconsistent with prior information
            if (moveTo("itemprop=\"birthDate\""))
            {

                tempUC = readNextBetween(PARENTHESES);
                qdate = tempUC.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOB(qdate);
            }
            else
            {
                // Reset read position to read in DOD
                beg();
                consecutiveMovesTo(500, "class=\"w-content", ">OBITUARY<", "itemprop=\"name\"");
            }

            if (moveTo("itemprop=\"deathDate\""))
            {

                tempUC = readNextBetween(PARENTHESES);
                qdate = tempUC.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOD(qdate);
            }

            // If still no valid DOD, potential date included in a header
            if (!globals->globalDr->getDOD().isValid())
            {
                beg();
                if(consecutiveMovesTo(500, "class=\"w-content", ">OBITUARY<", "class=\"header-subtitle\""))
                {
                    QList<QDate> dateList;
                    uc = readNextBetween(BRACKETS);
                    if(tempUC.pullOutDates(globals->globalDr->getLanguage(), dateList, 1, cleanString, true))
                        qdate = dateList.takeFirst();
                    if (qdate > QDate(2009,12,31))
                         globals->globalDr->setDOD(qdate);
                }
            }
        }*/
        break;

    case FosterMcGarvey:
        if (moveTo("\"obit_name\">"))
        {
            tempUC = getUntil("</td>");
            tempUC.removeHTMLtags(false);
            tempUC.simplify();

            // Read names (First Name first expected)
            // For this site, maiden names are separated by a comma  (e.g., "Charlotte Adams	(Maiden: Fitzpatrick, Harr)")
            QString newString;
            QString tempString = tempUC.getString();
            int index = tempString.indexOf("Maiden:", 0, Qt::CaseSensitive);
            if (index >= 0)
            {
                int newIndex = tempString.indexOf(", ", index + 7, Qt::CaseSensitive);
                if (newIndex >= 0)
                {
                    newString = tempString.left(newIndex);
                    newString += QString("/");
                    newString += tempString.right(tempString.length() - newIndex - 2);
                    tempUC = newString;
                }
            }

            // This site also has problematic double quotes
            newString = tempUC.getString();
            while (newString.indexOf("''") >= 0)
                newString.replace(QString("''"), QString("'"), Qt::CaseInsensitive);
            if (newString[0] == QChar(' '))  // get rid of any leading space
                newString.remove(0, 1);
            tempUC = newString;

            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);
        }
        break;

    case CFS:

        if (globals->globalDr->getProviderKey() > 90000)
        {
            // Old style of obits
            if (consecutiveMovesTo(500, "class=\"obituary\"", "style=\"font-size:120%;\""))
            {
                // Read name first
                tempUC = readNextBetween(BRACKETS);
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);

                // Move on to DOB and DOD
                if (consecutiveMovesTo(25, "Born:", ","))
                {
                    QList<QDate> dateList;
                    tempUC = getUntil("<", 50, true);
                    if (tempUC.pullOutDates(globals->globalDr->getLanguage(), dateList, 1, cleanString, true))
                         globals->globalDr->setDOB(dateList.takeFirst());
                }
                else
                    beg();

                if (consecutiveMovesTo(25, "Died:", ","))
                {
                    QList<QDate> dateList;
                    tempUC = getUntil("<", 50, true);
                    if (tempUC.pullOutDates(globals->globalDr->getLanguage(), dateList, 1, cleanString, true))
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
                target = QString("\"obitnameV3\"");
                break;

            }

            if (moveTo(target))
            {
                tempUC = readNextBetween(BRACKETS);
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);
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
                    tempUC = readNextBetween(BRACKETS);
                    if (tempUC.getLength() == 4)
                    {
                        number = tempUC.getString().toUInt();
                        if ((number > 1875) && (number <= static_cast<unsigned int>(globals->today.year())))
                            globals->globalDr->setYOB(number);
                    }
                    else
                    {
                        qdate = tempUC.readDateField();
                        if (qdate.isValid())
                        {
                            globals->globalDr->setDOB(qdate);
                            DOBfound = true;
                        }
                    }
                }

                backward(getPosition() - position);

                if (moveTo("class=\"dod\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    if (tempUC.getLength() == 4)
                    {
                        number = tempUC.getString().toUInt();
                        if ((number > 2000) && (number <= static_cast<unsigned int>(globals->today.year())))
                            globals->globalDr->setYOD(number);
                    }
                    else
                    {
                        qdate = tempUC.readDateField();
                        if (qdate.isValid())
                        {
                            globals->globalDr->setDOD(qdate);
                            DODfound = true;
                        }
                    }
                }

                backward(getPosition() - position);

                if (!(DOBfound && DODfound) && moveTo("class=\"ob-age\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.cleanUpEnds();
                    tempUC.removeBookEnds();
                    PQString age = tempUC.getWord();  // Indirect way of dropping "age" at front
                    age = tempUC.getWord();
                    if (age.isNumeric())
                        globals->globalDr->setAgeAtDeath(static_cast<unsigned int>(age.asNumber()));
                }
            }
        }

        break;

    case Frazer:
        purgeList = QString("Cowley|High River|Millarville|Okotoks|Pincher Creek").split("|");

        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(500, "class=\"obituariesDetailsPage\"", "itemprop=\"name\""))
            {
                // Get name, dropping any reference to location at end
                tempUC = readNextBetween(BRACKETS);
                PQString singleChar = tempUC.right(1);
                if ((singleChar.getCharType() & PARENTHESES) == PARENTHESES)
                {
                    int index = tempUC.findPosition(PQString("("), -1);
                    tempUC.dropRight(tempUC.getLength() - static_cast<unsigned int>(index) + 1);
                    if (tempUC.getString().right(1) == space)
                        tempUC.dropRight(1);
                }
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);

                // Read dates if provided
                position = getPosition();
                if (moveTo("itemprop=\"birthDate\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    qdate = tempUC.readDateField();
                    if (qdate.isValid())
                        globals->globalDr->setDOB(qdate);
                }

                backward(getPosition() - position);

                if (moveTo("itemprop=\"deathDate\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    qdate = tempUC.readDateField();
                    if (qdate.isValid())
                        globals->globalDr->setDOD(qdate);
                }
            }
            break;

        case 1:
            // Read Name
            if (consecutiveMovesTo(200, "MaterialData.user =", "fullName:", "\'"))
            {
                tempUC = getUntil("avatar:");
                tempUC.dropRight(tempUC.getLength() - static_cast<unsigned int>(tempUC.findPosition(PQString("\',"), -1)));
                tempUC.purge(purgeList);
                fixHyphens = true;
                tempUC.processStructuredNames(nameStatsList, fixHyphens);
            }

            // Read dates if provided
            position = getPosition();
            if (moveTo("itemprop=\"birthDate\":"))
            {
                tempUC = readNextBetween(BRACKETS);
                qdate = tempUC.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOB(qdate);
            }

            backward(getPosition() - position);

            if (moveTo("itemprop=\"deathDate\":"))
            {
                tempUC = readNextBetween(BRACKETS);
                qdate = tempUC.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOD(qdate);
            }

        break;

    case 2:
        // Read Name
        if (consecutiveMovesTo(200, "MaterialData.user =", "fullName:", "\'"))
        {
            tempUC = getUntil("avatar:");
            tempUC.dropRight(tempUC.getLength() - static_cast<unsigned int>(tempUC.findPosition(PQString("\',"), -1)));
            tempUC.purge(purgeList);
            fixHyphens = true;
            tempUC.processStructuredNames(nameStatsList, fixHyphens);
        }

        // Read dates if provided
        position = getPosition();
        if (moveTo("\"birthDate\":"))
        {
            tempUC = readNextBetween(QUOTES);
            qdate = tempUC.readDateField();
            if (qdate.isValid())
                globals->globalDr->setDOB(qdate);
        }

        backward(getPosition() - position);

        if (moveTo("\"deathDate\":"))
        {
            tempUC = readNextBetween(QUOTES);
            qdate = tempUC.readDateField();
            if (qdate.isValid())
                globals->globalDr->setDOD(qdate);
        }
        break;

        }
        break;

    case Alternatives:
        if (moveTo("class=\"page-title obit-title\""))
        {
            // Get name
            tempUC = readNextBetween(BRACKETS);
            fixHyphens = true;
            tempUC.processStructuredNames(nameStatsList, fixHyphens);

            // Read DOD only
            if (moveTo("class=\"obit-dod\""))
            {
                tempUC = readNextBetween(BRACKETS);
                // On occasion field will contain both DOB and DOD
                DATES dates = tempUC.readDOBandDOD();
                if (dates.hasDateInfo())
                    processNewDateInfo(dates, 1);
                else
                {
                    qdate = tempUC.readDateField();
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
            tempUC = readNextBetween(BRACKETS);
            tempUC.simplify();
            fixHyphens = true;      // Must be confident content contains only names

            // Everything after comma for V.J. McGillivray is location info
            if (globals->globalDr->getProviderKey() == 76)
            {
                int index = tempUC.getString().indexOf(QString(","));
                if (index >= 0)
                    tempUC = tempUC.left(index);
            }

            // Special coding to address garbage in field for Northern Lights
            bool originalProblematic = false;
            bool replaced = false;

            if (globals->globalDr->getProviderKey() == 6)
            {
                OQStream tempStream;
                PQString newStructuredName;

                if (tempUC.getString().contains(QString(",")))
                    originalProblematic = true;

                bool inQuotes;
                unsigned int position = tempUC.getPosition();
                while (!tempUC.isEOS())
                {
                   // Process some potentially useful maiden name information
                   word = tempUC.getWord(true);
                   inQuotes = word.removeBookEnds(QUOTES);
                   if (inQuotes)
                   {
                       tempStream = word;
                       word = tempStream.getWord();
                       if (word.isNeeEtAl() && !tempStream.isEOS())
                       {
                           PQString maidenName;
                           word = tempStream.getWord();
                           while (word.isCompoundName() && !tempStream.isEOS())
                           {
                               maidenName += word;
                               maidenName += space;
                               word = tempStream.getWord();
                           }
                           maidenName += word;
                           maidenName.removeEnding(SPACE);
                           globals->globalDr->setMaidenNames(maidenName);
                       }
                   }
                }
                tempUC.beg();
                tempUC.forward(position);

                // Determine if more reliable information exists in unstructured data
                int wordCount = 0;
                int indexOn = 0;
                bool keepGoing = true;
                bool startReadingName = false;
                tempStream = globals->globalObit->blockedContent.firstBlock;
                while ((wordCount < 30) && !tempStream.isEOS() && keepGoing)
                {
                    word = tempStream.getWord();
                    wordCount++;
                    if (word.lower() == PQString("on"))
                        indexOn = wordCount;

                    if ((indexOn > 0) && ((wordCount - indexOn) == 4))
                    {
                        OQString nextWord = tempStream.peekAtWord();
                        if (nextWord.isCapitalized() && givenNameLookup(nextWord.getString(), globals))
                            startReadingName = true;
                    }

                    if (word.isTitle())
                    {
                        startReadingName = true;
                        if (word.isMaleTitle())
                            globals->globalDr->setGender(Male);
                        else
                        {
                            if (word.isFemaleTitle())
                                globals->globalDr->setGender(Female);
                        }
                    }

                    if (startReadingName)
                    {
                        while (keepGoing && !tempStream.isEOS())
                        {
                            word = tempStream.getWord(true);
                            if (word.isCapitalized() || word.isCompoundName() || word.hasBookEnds(QUOTES | PARENTHESES))
                            {
                                newStructuredName += word;
                                newStructuredName += space;
                            }
                            else
                                keepGoing = false;
                        }

                        newStructuredName.removeEnding(SPACE);
                        if (newStructuredName.countWords() >= 2)
                        {
                            unstructuredContent tempUC(newStructuredName);
                            tempUC.processStructuredNames(nameStatsList, fixHyphens);
                            replaced = true;
                        }
                    }
                }
            }

            if (!originalProblematic || !replaced)
                tempUC.processStructuredNames(nameStatsList, fixHyphens);

            // Read DOB and DOD in form (mmm dd, yyyy - mmm dd, yyyy)
            if (moveTo("<h3"))
            {
                tempUC = readNextBetween(PARENTHESES);
                tempUC.simplify();
                fillInDatesStructured(tempUC);
            }
        }
        else
        {
            tempUC = globals->globalDr->getTitle();
            tempUC.dropLeft(12);    // "Obituary of "
            tempUC.simplify();
            fixHyphens = true;      // Must be confident content contains only names

            // Everything after comma for V.J. McGillivray is location info
            if (globals->globalDr->getProviderKey() == 76)
            {
                int index = tempUC.getString().indexOf(QString(","));
                if (index >= 0)
                    tempUC = tempUC.left(index);
            }
            tempUC.processStructuredNames(nameStatsList, fixHyphens);

            beg();
            if (consecutiveMovesTo(100, "In loving memory of", "<h3"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                tempUC.readYears();
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case WordPress:
        // Structured content written as "Smith, John Stanley  1968 - 2015"
        if (moveTo("entry-title hidden"))
        {
            OQString begString, endString;
            tempUC = readNextBetween(BRACKETS);
            bool numEncountered = false;

            while (!tempUC.isEOS())
            {
                word = tempUC.getWord(true);
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

            tempUC = unstructuredContent(begString);
            fixHyphens = false;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);

            // Read YOB and YOD in form (yyyy - yyyy)
            tempUC = unstructuredContent(endString);
            fillInDatesStructured(tempUC);
        }
        break;

    case FrontRunner:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(500, "bom-in-memory-name", ">"))
            {
                // Read name
                tempUC = getUntil("</div>");
                tempUC.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);

                // Read YOB and YOD (YYYY - YYYY)
                if (moveTo("bom-in-memory-date"))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.simplify();
                    fillInDatesStructured(tempUC);
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(500, "class=\"bom-header-info\"", "a href", "php\">"))
            {
                // Read name
                tempUC = getUntil("</a>");
                tempUC.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);

                // Read DOB and DOD
                if (moveTo("<p>"))
                {
                    tempUC = getUntil("</p>");
                    tempUC.simplify();
                    fillInDatesStructured(tempUC);
                }
            }
            break;

        case 2:
        case 3:
            if (moveTo("<!-- obit name -->"))
            {
                moveTo("<h1");
                // Read names (First Name first expected)
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);

                // Read DOB and DOD
                if (moveTo("<!-- obit date -->"))
                {
                    position = getPosition();

                    if (moveTo("class=\"dateofBirth\""))
                    {
                        tempUC = readNextBetween(BRACKETS);
                        qdate = tempUC.readDateField();
                        if (qdate.isValid())
                            globals->globalDr->setDOB(qdate);
                    }

                    backward(getPosition() - position);

                    if (moveTo("class=\"dateofDeath\""))
                    {
                        tempUC = readNextBetween(BRACKETS);
                        qdate = tempUC.readDateField();
                        if (qdate.isValid())
                            globals->globalDr->setDOD(qdate);
                    }
                }
            }
            break;
        }

        // Supplementary read to fill in missing information (service.php vs. obituary.php info)
        if (!globals->globalDr->getDOB().isValid() || !globals->globalDr->getDOD().isValid())
        {
            beg();
            if (moveTo("id=\"bom-content-wrapper"))
            {
                if(!globals->globalDr->getDOB().isValid())
                {
                    if (conditionalMoveTo(">Date of Birth<", "<!--Show above vistions-->", 0))
                    {
                        if (consecutiveMovesTo(200, "infoContent", ","))
                        {
                            tempUC = getUntil("<");
                            qdate = tempUC.readDateField();
                            if (qdate.isValid())
                                globals->globalDr->setDOB(qdate);
                        }
                    }
                }

                if(!globals->globalDr->getDOD().isValid())
                {
                    if (conditionalMoveTo(">Date of Death<", "<!--Show above vistions-->", 0))
                    {
                        if (consecutiveMovesTo(200, "infoContent", ","))
                        {
                            tempUC = getUntil("<");
                            qdate = tempUC.readDateField();
                            if (qdate.isValid())
                                globals->globalDr->setDOD(qdate);
                        }
                    }
                }
            }

        }


        break;

    case FuneralOne:
        if (moveTo("tributeDisplayName:"))
        {
            tempUC = readNextBetween(QUOTES);
            fixHyphens = true;
            tempUC.processStructuredNames(nameStatsList, fixHyphens);
        }
        else
            beg();

        if (consecutiveMovesTo(200, "<dt>Name</dt>", "itemprop=\"name\""))
        {            
            if (moveTo("itemprop") && conditionalMoveTo("birthDate", "itemprop", 0))
            {
                moveTo("datetime=");
                tempUC = readNextBetween(QUOTES);
                qdate = tempUC.readDateField(doYMD);
                if (qdate.isValid())
                    globals->globalDr->setDOB(qdate, true, true);
            }

            if (moveTo("itemprop") && conditionalMoveTo("deathDate", "itemprop", 0))
            {
                moveTo("datetime=");
                tempUC = readNextBetween(QUOTES);
                qdate = tempUC.readDateField(doYMD);
                if (qdate.isValid())
                    globals->globalDr->setDOD(qdate, true, false);
            }

            if (moveTo("itemprop") && conditionalMoveTo("givenName", "itemprop", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                while (!tempUC.isEOS())
                {
                    name.clear();
                    word = tempUC.getWord(true);
                    word.removeEnding(PUNCTUATION);
                    word.removeBookEnds();
                    name = word.getString();
                    globals->globalDr->setFirstNames(name);
                }
            }

            if (moveTo("itemprop") && conditionalMoveTo("familyName", "itemprop", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                name.clear();
                word = tempUC.getWord(true);
                fullWord = word;
                while (word.isCompoundName())
                {
                    word = tempUC.getWord(true);
                    fullWord += space;
                    fullWord += word;
                }
                word = fullWord;
                word.removeEnding(PUNCTUATION);
                name = word.getString();
                globals->globalDr->setFamilyName(name);
            }

            consecutiveMovesTo(100, "id=\"hidBirth\"", "value=");
            if (!globals->globalDr->getDOB().isValid())
            {
                tempUC = readNextBetween(QUOTES);
                qdate = tempUC.readDateField(doMDY);
                if (qdate.isValid())
                    globals->globalDr->setDOB(qdate, true, false);
            }

            consecutiveMovesTo(100, "id=\"hidDeath\"", "value=");
            if (!globals->globalDr->getDOD().isValid())
            {
                tempUC = readNextBetween(QUOTES);
                qdate = tempUC.readDateField(doMDY);
                if (qdate.isValid())
                    globals->globalDr->setDOD(qdate, true, false);
            }
        }
        break;

    case WebStorm:
        switch(style)
        {
        case 0:
            // Read name
            if (moveTo("class=\"fancy\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);
            }

            // Read DOB - DOD
            if (moveTo("<p>"))
            {
                tempUC = getUntil("</p>");
                tempUC.removeHTMLtags();
                tempUC.simplify();
                fillInDatesStructured(tempUC);
            }
            break;

        case 1:
            // Read name
            if (consecutiveMovesTo(100, "class=\"obit_name\"", "\"name\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);
            }

            // Read DOB - DOD
            if (moveTo("class=\"Small_Text\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                fillInDatesStructured(tempUC);
            }
            break;
        }
        break;

    case GasLamp:
        if (consecutiveMovesTo(1000, "ct-mediaSection obit-single", "<h2"))
        {
            // Read name
            tempUC = readNextBetween(BRACKETS);
            tempUC.simplify();

            // Special coding required to deal with all the unnecessary hyphens inserted between the names
            QString tempString, newString, name;
            tempString = tempUC.getString();
            tempString.replace(QString("-"), QString(" "), Qt::CaseInsensitive);
            tempUC = tempString;

            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);

            // Read YOB and YOD (YYYY - YYYY)
            if (moveTo("class=\"years\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case ClickTributes:
        // Read name
        if (moveTo("class=\"ct-name\">"))
        {
            tempUC = getUntil("</h2>");
            tempUC.simplify();
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);
        }

        // Read DOB - DOD
        // Nothing coded as will be same info as original list
        break;

    case ConnellyMcKinley:
        // Read name
        if (consecutiveMovesTo(100, "entry-title", "itemprop=\"name\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.simplify();
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);
        }
        break;

    case Arbor:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(200, "id=\"personInfoContainer\"", "id=\"displayName\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);
            }

            if (conditionalMoveTo("birthDate", "siteCredits", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                qdate = tempUC.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOB(qdate, true, true);
            }

            if (conditionalMoveTo("deathDate", "siteCredits", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                qdate = tempUC.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOD(qdate, true, true);
            }

            break;

        case 1:
            tempUC.clear();
            primarySourceProcessed = false;

            if (consecutiveMovesTo(20, ">Obituary Overview<", "<h2"))
            {
                // Could just be text such as "In loving memory..."
                OQStream tempStream = readNextBetween(BRACKETS);
                bool started = false;
                bool keepGoing = true;
                OQString word;

                while (!tempStream.isEOS() && keepGoing)
                {
                    word = tempStream.getWord(true);
                    while (!started && word.isTitle())
                    {
                        if (globals->globalDr->getGender() == genderUnknown)
                        {
                            if (word.isMaleTitle())
                                globals->globalDr->setGender(Male);
                            else
                            {
                                if (word.isFemaleTitle())
                                    globals->globalDr->setGender(Female);
                            }
                        }
                        word = tempStream.getWord(true);
                    }

                    if (!started)
                    {
                        if (word.lower() == globals->globalDr->getTitleKey().lower())
                            started = true;
                        else
                            keepGoing = false;
                    }

                    if (started)
                        uc += word + OQString(" ");
                }
                tempUC.removeEnding(SPACE);

                if ((tempUC.getLength() > 0) && (tempUC.countWords() >= 2))
                {
                    fixHyphens = true;      // Must be confident content contains only names
                    tempUC.processStructuredNames(nameStatsList, fixHyphens);
                    primarySourceProcessed = true;

                    // Process secondary source now
                    tempUC.clear();
                    beg();
                    if (moveTo("hero__name"))
                        tempUC = readNextBetween(BRACKETS);

                    if (tempUC.getLength() > 0)
                    {
                        tempUC.standardizeQuotes();
                        globals->globalDr->setAlternates(tempUC.readAlternates(PARENTHESES | QUOTES, true));
                        globals->globalDr->setMiddleNames(tempUC.processAllNames());
                    }
                }
            }

            if (!primarySourceProcessed)
            {
                // Process secondary source as primary now
                tempUC.clear();
                beg();
                if (moveTo("hero__name"))
                    tempUC = readNextBetween(BRACKETS);

                if (tempUC.getLength() > 0)
                {
                    tempUC.simplify();
                    fixHyphens = true;      // Must be confident content contains only names
                    tempUC.processStructuredNames(nameStatsList, fixHyphens);
                }
            }
            break;
        }
        break;

   case SiteWyze:
        // Only name available (same was used for titlestream)
        if (consecutiveMovesTo(2000, "post-content", "<h2"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.simplify();
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);
        }
        break;

    case ThinkingThru:
    {
        QString targetText;
        if (globals->globalDr->getDOB().isValid())
            targetText = globals->globalDr->getDOB().toString("MMMM dd, yyyy");
        else
        {
            if (globals->globalDr->getDOD().isValid())
                targetText = globals->globalDr->getDOD().toString("MMMM dd, yyyy");
            else
                targetText = QString("<!-- .et_pb_text -->");
        }

        if (moveTo(targetText))
        {
            if(moveBackwardTo("<strong"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);
            }
        }
        break;
    }

    case Codesign:
    {
        switch(style)
        {
        case 0:
            // DOB and DOD in semi-structured section at top

            // Read name
            if (consecutiveMovesTo(100, "class=\"obituaries_content\"", "<h1"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);
            }

            // Read DOB and DOD
            if (moveTo("class=\"clearfix obt_meta\">"))
            {
                //DATES dates;
                //bool limitWords = false;

                tempUC = getUntil("</div>");
                tempUC.simplify();

                OQStream tempStream(tempUC.getString());
                if (tempStream.conditionalMoveTo("Born", "Departed", 0))
                {
                    if (tempStream.conditionalMoveTo("on ", "Departed", 0))
                    {
                        tempUC = tempStream.getUntil("<");
                        qdate = tempUC.readDateField(doYMD);
                        if (qdate.isValid())
                            globals->globalDr->setDOB(qdate, true, false);
                    }
                }

                if (tempStream.moveTo("Departed on "))
                {
                    tempUC = tempStream.getUntil("<");
                    qdate = tempUC.readDateField(doYMD);
                    if (qdate.isValid())
                        globals->globalDr->setDOD(qdate, true, false);
                }

                //dates = tempUC.readDates(globals->globalDr->getLanguage(), limitWords);
                //processNewDateInfo(dates, 1);
            }
            break;

        case 1:
            // Only name at this point
            if (moveTo("post_title entry-title"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);
            }
            break;
        }

        break;
    }

    case Shape5:
    {
        if (consecutiveMovesTo(200, "page-header", "headline", ">"))
            tempUC = getUntil("<");
        tempUC.simplify();
        fixHyphens = true;      // Must be confident content contains only names
        tempUC.processStructuredNames(nameStatsList, fixHyphens);
        break;
    }

    case TributeArchive:
    {
        if (moveTo("subtitle-story hidden-xs"))
            tempUC = readNextBetween(BRACKETS);
        tempUC.simplify();

        // Deal with issue of dates embedded within name
        if (globals->globalDr->getProviderKey() <= 2)
        {
            QList<QDate> dateList;
            OQString cleanString;
            tempUC.pullOutDates(language_unknown, dateList, 2, cleanString, true);
            tempUC = cleanString;
        }

        fixHyphens = true;      // Must be confident content contains only names
        tempUC.processStructuredNames(nameStatsList, fixHyphens);

        // Some obits will have dates BUT they will always be english, even if text is french
        if (moveTo("date-story hidden-xs", 250))
        {
            DATES dates;
            bool reliable = false;
            moveTo(">");
            tempUC = getUntil(("</p>"));
            tempUC.simplify();
            tempUC.removeHTMLtags();
            if (tempUC.getLength() > 0)
            {
                tempUC.setContentLanguage(english);
                fillInDatesStructured(uc, reliable);
            }
        }

        break;
    }

    case YAS:
    {
        if (moveTo("class=\"post-header\">"))
        {
           tempUC = getUntil("</header>");
           tempUC.removeHTMLtags();
           tempUC.simplify();
           fixHyphens = true;      // Must be confident content contains only names
           tempUC.processStructuredNames(nameStatsList, fixHyphens);
        }
        break;
    }

    case WFFH:
    {
        // Name, DOB and DOD typcially availabile
        if (consecutiveMovesTo(500, "persondetails-area", "personName", "personFirst", ">"))
        {
            tempUC = getUntil("</div>");
            tempUC.removeHTMLtags();
            tempUC.simplify();
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);
        }

        // Dates
        if (conditionalMoveTo("personBirth", "personContact", 0))
        {
            tempUC = readNextBetween(BRACKETS);
            qdate = tempUC.readDateField();
            if (qdate.isValid())
                globals->globalDr->setDOB(qdate, true, true);
        }

        if (conditionalMoveTo("personDeath", "personContact", 0))
        {
            tempUC = readNextBetween(BRACKETS);
            qdate = tempUC.readDateField();
            if (qdate.isValid())
                globals->globalDr->setDOD(qdate, true, true);
        }
    }
        break;

    case FHW:
    {
        // Name, DOB and DOD typcially availabile
        if (consecutiveMovesTo(100, "class=\"obit_name_and_date\"", "<h2"))
        {
            tempUC = readNextBetween(BRACKETS);
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);
        }

        // Dates
        // Available but not necessary as already read in
        /*if (conditionalMoveTo("<h4", "</div>", 2))
        {
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);
        }*/
    }
        break;

    case Specialty:
    {
        // Only name available from title text
        tempUC = globals->globalDr->getTitle();
        fixHyphens = true;      // Must be confident content contains only names
        tempUC.processStructuredNames(nameStatsList, fixHyphens);
    }
        break;

    /***************/
    /*  One Offs   */
    /***************/

    case MikePurdy:
        if (consecutiveMovesTo(500, "class=\"storytitle\"", "Permanent Link to"))
        {
            // Read names
            tempUC = readNextBetween(BRACKETS);
            tempUC.simplify();
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);
        }
        break;

    case BowRiver:
        if (consecutiveMovesTo(100, "id=\"content_body\"", "h1"))
        {
            // Read names
            tempUC = readNextBetween(BRACKETS);
            tempUC.simplify();
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);

            // Read DOB and DOD
            if (moveTo("h2", 50))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Serenity:
        if (consecutiveMovesTo(100, "title page-title", "itemprop=\"headline\""))
        {
            // Read names
            tempUC = readNextBetween(BRACKETS);
            tempUC.simplify();
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);

            // Read DOB and DOD
            // Nothing coded for now as it appears to be exact duplicate of original listing
        }
        break;

    case McInnis:
        switch(style)
        {
        case 0:
            if (moveTo("class=\"entry-title\""))
            {
                // Read names
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);

                // Read DOB and DOD
                // Hit and miss here both in terms of content and structure (eg. might get YYYY for DOB and mmm dd, yyyy for DOD)
                int shortDatesFound = 0;
                int longDatesFound = 0;
                bool firstLong = false;
                unsigned int yyyy, numDates, potentialYear;
                unstructuredContent firstDate, secondDate;
                DATES dates;
                QList<QDate> dateList;
                OQString nextChar, tempString;

                if (conditionalMoveTo("<strong>", "Print This Obituary", 0))
                {
                    firstDate = getUntil(QString(QChar(8211)), 20);
                    firstDate.cleanUpEnds();
                    if (firstDate.getLength() <= 4)
                        shortDatesFound++;
                    else
                    {
                        longDatesFound++;
                        firstLong = true;
                    }

                    if (conditionalMoveTo("<strong>", "Print This Obituary", 0))
                    {
                        secondDate = getUntil(QString(QChar(8211)), 20);
                        secondDate.cleanUpEnds();
                        if (secondDate.getLength() <= 4)
                            shortDatesFound++;
                        else
                            longDatesFound++;
                    }
                }
                else
                {
                    if (consecutiveMovesTo(300, "entry-content", "<p", ">"))
                    {
                        nextChar = peekAtNextRealChar();
                        while (nextChar == OQString("<"))
                        {
                            moveTo(">");
                            nextChar = peekAtNextRealChar();
                        }
                        tempUC = getUntil("<");
                        numDates = tempUC.pullOutDates(language_unknown, dateList, 2, cleanString, true);
                        if (numDates != 1)
                        {
                            // Check for short date before assuming it's a name or something else
                            potentialYear = 0;
                            tempUC.beg();
                            tempString = tempUC.getNext(4);
                            if (tempString.isNumeric())
                                potentialYear = static_cast<unsigned int>(tempString.asNumber());
                            if ((potentialYear < 1900) || (potentialYear > static_cast<unsigned int>(globals->today.year())))
                            {
                                moveTo(">");
                                nextChar = peekAtNextRealChar();
                                while (nextChar == OQString("<"))
                                {
                                    moveTo(">");
                                    nextChar = peekAtNextRealChar();
                                }
                                tempUC = getUntil("<");
                            }
                        }
                        moveTo(">");

                        tempUC.beg();
                        firstDate = tempUC.getUntil(QString(QChar(8211)), 20);
                        firstDate.cleanUpEnds();
                        if (firstDate.getLength() > 0)
                        {
                            if (firstDate.getLength() <= 4)
                                shortDatesFound++;
                            else
                            {
                                longDatesFound++;
                                firstLong = true;
                            }
                        }

                        tempUC = getUntil("</p>");
                        tempUC.simplify();
                        tempUC.removeHTMLtags();
                        secondDate = tempUC.getUntil(QString(QChar(8211)), 20);
                        secondDate.cleanUpEnds();
                        if (secondDate.getLength() > 0)
                        {
                            if (secondDate.getLength() <= 4)
                                shortDatesFound++;
                            else
                                longDatesFound++;
                        }
                    }
                }

                // Deal with year only dates
                switch(shortDatesFound)
                {
                case 0:
                    break;

                case 1:
                    if (!firstLong)
                    {
                        yyyy = firstDate.getString().toUInt();
                        if ((yyyy < 2000) || (longDatesFound == 1))
                            globals->globalDr->setYOB(yyyy);
                        else
                            globals->globalDr->setYOD(yyyy);
                    }
                    else
                    {
                        yyyy = secondDate.getString().toUInt();
                        globals->globalDr->setYOD(yyyy);
                    }
                    break;

                case 2:
                    tempUC = firstDate + OQString(" - ") + secondDate;
                    dates = tempUC.readYears();
                    processNewDateInfo(dates, 2);
                    break;
                }

                // Deal with full length dates
                switch(longDatesFound)
                {
                case 0:
                    tempUC.clear();
                    break;

                case 1:
                    if (firstLong)
                        tempUC = firstDate;
                    else
                        tempUC = secondDate;
                    break;

                case 2:
                    tempUC = firstDate + OQString(" - ") + secondDate;
                    break;
                }

                fillInDatesStructured(tempUC);
            }
            break;

        case 1:
            if (consecutiveMovesTo(100, "class=\"deceased-info\"", "<h2>"))
            {
                tempUC = getUntil("</h2>");
                tempUC.simplify();
                fixHyphens = true;      // Must be confident content contains only names
                tempUC.processStructuredNames(nameStatsList, fixHyphens);

                // Look for possible date info
                if (moveTo("<h3>", 100))
                {
                    tempUC = getUntil("</h3>");
                    tempUC.simplify();
                    fillInDatesStructured(tempUC);
                }
            }
            break;
        }
        break;

    case Sturgeon:
        if (consecutiveMovesTo(1000, "class=\"page-header\"", "a href=", ">"))
        {
            // Read names
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.simplify();
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);

            // Read DOB and DOD
            // Nothing else available
        }
        break;

    case CornerStone:
        // Read names
        if (consecutiveMovesTo(100, "name=\"parentTitle\"", "value="))
            tempUC = readNextBetween(DOUBLE_QUOTES);
        else
        {
            beg();
            if (consecutiveMovesTo(1000, "column four-fifths", "<h2"))
                tempUC = readNextBetween(BRACKETS);
        }
        if (tempUC.getLength() > 3)
        {
            tempUC.simplify();
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);
        }

        // Read DOB and DOD
        // Coding is duplicate of original listing, but look for "YYYY - YYYY", which would have been dropped
        if (!globals->globalDr->getDOB().isValid() || !globals->globalDr->getDOD().isValid())
        {
            beg();
            tempUC.clear();
            if (consecutiveMovesTo(1000, "column four-fifths", "<h2>", "<h4"))
            {
                word = peekAtNext(1);
                if (word == OQString(">"))
                    tempUC = readNextBetween(BRACKETS);
                else
                {
                    if (moveTo("class=\"nodash\"", 100))
                        tempUC = readNextBetween(BRACKETS);
                }
            }
            if (tempUC.getLength() >= 4)
                fillInDatesStructured(tempUC);
        }

        break;

    case Pierson:
        switch(style)
        {
        case 0:
            // Read Name
            if (moveTo("givenName"))
                word = readNextBetween(BRACKETS);
            if (conditionalMoveTo("additionalName", "familyName", 0))
            {
                word += space;
                word += readNextBetween(BRACKETS);
            }
            if (moveTo("familyName"))
            {
                word += space;
                word += readNextBetween(BRACKETS);
            }
            tempUC = word;
            tempUC.simplify();
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);

            // Read dates
            if (conditionalMoveTo("birthDate", "deathDate", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                qdate = tempUC.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOB(qdate);

            }

            if (moveTo("deathDate"))
            {
                tempUC = readNextBetween(BRACKETS);
                qdate = tempUC.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOD(qdate);
            }

            break;

        case 1:
            // Read Name
            if (consecutiveMovesTo(100, "ObituaryFullName", "\'"))
                tempUC = getUntil("\',");
            tempUC.simplify();
            fixHyphens = true;      // Must be confident content contains only names
            tempUC.processStructuredNames(nameStatsList, fixHyphens);

            // Read dates
            if (conditionalMoveTo("birthDate\":", "deathDate", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                qdate = tempUC.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOB(qdate);

            }

            if (moveTo("deathDate\":"))
            {
                tempUC = readNextBetween(BRACKETS);
                qdate = tempUC.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOD(qdate);
            }

            break;
        }
        break;

    case Trinity:
        // Read name
        if (moveTo("entry-title"))
            tempUC = readNextBetween(BRACKETS);

        tempUC.simplify();
        fixHyphens = true;      // Must be confident content contains only names
        tempUC.processStructuredNames(nameStatsList, fixHyphens);
        break;

    case CelebrateLife:
        // Read name
        if (consecutiveMovesTo(3000, "class=\"main-wrap\"", "Obituary of"))
            tempUC = getUntil("<");
        tempUC.simplify();
        tempUC.removeLeading(SPACE);
        fixHyphens = true;      // Must be confident content contains only names
        tempUC.processStructuredNames(nameStatsList, fixHyphens);
        break;

    case Funks:
        // Only name available
        if (consecutiveMovesTo(50, "og:title", "content="))
            tempUC = readNextBetween(QUOTES);
        tempUC.simplify();
        fixHyphens = true;      // Must be confident content contains only names
        tempUC.processStructuredNames(nameStatsList, fixHyphens);
        break;

    case WowFactor:
        // Only name available
        if (consecutiveMovesTo(3000, "id=\"main-content\"", "<h3", ">"))
            tempUC = getUntil("</h3>");
        tempUC.removeHTMLtags();
        tempUC.simplify();
        tempUC.cleanUpEnds();
        fixHyphens = true;      // Must be confident content contains only names
        tempUC.processStructuredNames(nameStatsList, fixHyphens);
        break;

    case Dalmeny:
        // Only name available
        if (consecutiveMovesTo(2000, "class=\"content\"", "<p", ">"))
            tempUC = getUntil("</p>");
        tempUC.removeHTMLtags();
        tempUC.simplify();
        tempUC.cleanUpEnds();
        fixHyphens = true;      // Must be confident content contains only names
        tempUC.processStructuredNames(nameStatsList, fixHyphens);
        break;

    case Hansons:
        // Only name available
        if (moveTo("post post-obituary current-item"))
            tempUC = readNextBetween(BRACKETS);
        tempUC.simplify();
        fixHyphens = true;      // Must be confident content contains only names
        tempUC.processStructuredNames(nameStatsList, fixHyphens);
        break;

    case Martens:
        // Only name available
        if (moveTo("blog-title-link blog-link"))
            tempUC = readNextBetween(BRACKETS);
        tempUC.simplify();
        fixHyphens = true;      // Must be confident content contains only names
        tempUC.processStructuredNames(nameStatsList, fixHyphens);
        break;

    case Shine:
        // Name, DOB and DOD typically available
        if (consecutiveMovesTo(1000, "section id=\"content\"", "p7-1", ">"))
            tempUC = getUntil("</h3>");
        tempUC.simplify();
        fixHyphens = true;      // Must be confident content contains only names
        tempUC.processStructuredNames(nameStatsList, fixHyphens);

        // Dates
        if (conditionalMoveTo("p3-1", "Service Details", 0))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.simplify();
            fillInDatesStructured(tempUC);
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
    unstructuredContent tempUC;

    switch (globals->globalDr->getProvider())
    {
    case Legacy:
        if (moveTo("<title>"))
            tempUC = getUntil(" Obituary");
        break;

    case Passages:
        // Initial name search from title and header sections to bookend unstructured searches
        // Format is always LASTNAME FIRSTNAME -

        if (moveTo("<title>"))
        {
            PQString firstName, lastName;
            OQString name;
            unsigned int numNamesRemaining, numWordsFirstName, numWordsLastName;

            tempUC = getUntil(" - ", 75, true);
            if (tempUC.getLength() == 0)
            {
                // Backup read
                // TODO
            }
            tempUC.cleanUpEnds();
            tempUC.beg();
            numNamesRemaining = tempUC.countWords();
            numWordsFirstName = 1;
            numWordsLastName = numNamesRemaining - numWordsFirstName;
            while (numNamesRemaining > 0)
            {
                name = tempUC.getWord();
                if (numNamesRemaining > 1)
                    lastName += name.proper();
                else
                    firstName = name.proper();
                if (numNamesRemaining > 2)
                    lastName += PQString(" ");
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
            tempUC = getUntil(" - ", 75, true);
            if (tempUC.getLength() == 0)
            {
                // Backup read
                beg();
                if (moveTo("<title>"))
                        tempUC = getUntil(" - ", 50, true);
            }
            tempUC.cleanUpEnds();
            tempUC.beg();
            if (tempUC.countWords() > 1)
            {
                if (tempUC.contains(","))
                    tempUC.readLastNameFirst(nameStatsList);
                else
                    tempUC.readFirstNameFirst(nameStatsList);
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
    {
        if (moveTo("class=\"obit-content\""))
        {
            if (moveTo("class=\"obitHD\""))
                style = 3;
            else
                style = 0;
        }
        else
        {
            beg();
            if (moveTo("meaningfulfunerals.net", 1000))
                style = 2;
            else
            {
                beg();
                if (moveTo("text-container"))
                    style = 1;
            }
        }
    }
        break;

    case CFS:
    {
        if(moveTo("initV31Obit()"))
            style = 1;
        else
        {
            beg();
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
        }
    }
        break;

    case Frazer:
    {
        if (moveTo("site-header main-header"))
            style = 2;
        else
        {
            beg();
            if (moveTo("MaterialData.user ="))
                style = 1;
            else
                style = 0;
        }
    }
        break;

    case FrontRunner:
    {
        if (moveTo("bom-in-memory-name"))
            style = 0;
        else
        {
            beg();
            if (moveTo("class=\"bom-header-info\""))
                style = 1;
            else
            {
                beg();
                if (moveTo("<!-- bom content area -->"))
                    style = 3;
                else
                    style = 2;
            }
        }

        /*unsigned int ID = static_cast<unsigned int>(globals->globalDr->getID().asNumber());
        style = 0;

        if (ID > 3612282)
            style++;

        if (ID > 3667415)
            style++;*/

    }
        break;

    case WebStorm:
    {
        moveTo("<title>");
        tempString = getUntil(" ");
        if (tempString == OQString("Obituary"))
            style = 1;
        else
            style = 0;
    }
        break;

    case Arbor:
    {
        if (moveTo("SKYPE_TOOLBAR", 3000))
            style = 1;
        else
            style = 0;
    }
        break;

    case Codesign:
    {
        if (moveTo("<!-- #TRX_REVIEWS_PLACEHOLDER# -->"))
            style = 1;
        else
            style = 0;
    }
        break;

    case Sturgeon:
    {
        if (consecutiveMovesTo(500, "Details:", "Published:"))
            style = 0;
        else
            style = 1;
    }
        break;

    case Shape5:
    {
        if (moveTo("<w:WordDocument>"))
            style = 0;
        else
            style = 1;
    }
        break;

    case McInnis:
    {
        if (moveTo("apple-mobile-web-app-status-bar-style"))
            style = 1;
        else
            style = 0;
    }
        break;

    case Pierson:
    {
        if (moveTo("smartphones"))
            style = 1;
        else
            style = 0;
    }
        break;

    default:
        // Do nothing

        break;

    }
}

void readObit::cleanAndExtract()
{
    if (uc.getLength() > 0)
    {
        beg();
        uc.determineLanguageAndGender(cleanedUpUC, justInitialNamesUC);
        globals->globalDr->setLanguage(uc.getLanguage());
        globals->globalDr->setGender(uc.getGender());
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
    return static_cast<unsigned int>(cleanedUpUC.countFrequency(word, caseSensitivity));
}

unsigned int readObit::countFrequencyFirst(QString word, Qt::CaseSensitivity caseSensitivity) const
{
    return static_cast<unsigned int>(cleanedUpUC.countFrequencyFirst(word, caseSensitivity));
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

void readObit::setHyphenatedNameFlag(bool flag)
{
    hyphenatedLastName = flag;
}

bool readObit::hasGodReference() const
{
    return godReference;
}

bool readObit::hasHyphenatedName() const
{
    return hyphenatedLastName;
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
    if (uc.getLength() == 0)
        return;

    bool found = false;
    QList<QString> targetPhrases;
    QList<QString> parentNames;
    QList<QString> brotherLastNames;
    QString allContent = uc.getString();
    QString targetText, tempText, sentence;
    unstructuredContent content, tempContent;
    int i, index, wordCount, wordCountAfterAnd;
    double firstUnisex, secondUnisex, unisex;
    OQString word, tempWord, name, savedName, lastChar, nextWord, priorWord, maleLastName, femaleLastName;
    QString space(" ");
    QString period(".");
    QString semicolon(";");
    QString comma(",");
    bool keepGoing, wordIsAnd, andEncountered, compoundName, endOfNames, nextWordIsSurname, isInitial, getAnotherWord;
    bool step1complete, step2complete, hadComma, valid;
    bool isSurname, isGivenName, isAboriginal;
    bool highConfidence = false;
    bool maleNameEncountered = false;
    bool femaleNameEncountered = false;
    wordCount = 0;
    firstUnisex = secondUnisex = 0.5;
    GENDER currentGender;
    LANGUAGE lang = globals->globalDr->getLanguage();
    PQString tempName;

    // Approach is to find location of key word in all content, then search sentence by sentence from that point forward

    // Before looking for parents, create a list of brother last names
    // Coded to read for:
    //  Hank Smith
    //  Hank and Harry Smith
    //  Hank (Judy) Smith
    //  Hank, Troy and Bill Smith

    targetPhrases = OQString::getBrotherReferences(lang);
    while (!found && targetPhrases.size() > 0)
    {
        targetText = targetPhrases.takeFirst();
        index = allContent.indexOf(targetText, 0, Qt::CaseInsensitive);
        if (index >= 0)
        {
            keepGoing = true;
            step1complete = false;
            step2complete = false;

            // Prepare content to be processed
            content = uc.getSentence(lang, uc.getSentenceNum(index));
            sentence = content.getString();
            index = sentence.indexOf(targetText, 0, Qt::CaseInsensitive);
            sentence.remove(0, index);
            sentence.replace(QString("the late "), QString(""));
            sentence.replace(QString("de feu "), QString(""));
            content = sentence;
            content.removeParenthesesContent();

            // Start actual processing
            word = content.getWord(); // should be "brother[s:]
            if (!(word.removeEnding(period) || (word.right(3).lower() == OQString("law"))))
            {
                // Step 1 - Retrieve what should be first name of brother, excluding "Hank,"
                while (!step1complete && keepGoing && !content.isEOS())
                {
                    word = content.getWord();
                    hadComma = word.removeEnding(comma);
                    if (word.removeEnding(period) || !word.isCapitalized() || !givenNameLookup(word.getString(), globals, Male) || globals->globalDr->isASavedName(word))
                        keepGoing = false;
                    else
                    {
                        if (!hadComma)
                            step1complete = true;
                    }
                }

                // Step 2 - Retrieve next valid first or last name
                if (step1complete && keepGoing)
                {
                    word = content.getWord();
                    word.removeEnding(PUNCTUATION);
                    while (word.isCapitalized() && (word.getLength() == 1) && !content.isEOS())
                    {
                        // Deal with initials
                        word = content.getWord();
                        word.removeEnding(PUNCTUATION);
                    }
                    valid = word.isCapitalized();
                    tempWord = word;
                    while (word.isUncapitalizedName() && !content.isEOS())
                    {
                        if (word.isSaint())
                            tempWord += period;
                        word = content.getWord();
                        tempWord += word;
                        valid = true;
                    }
                    word = tempWord.proper();
                    word.compressCompoundNames();
                    if (!valid)
                        keepGoing = false;
                    isGivenName = isSurname = false;

                    while (!step2complete && keepGoing)
                    {
                        getAnotherWord = (word == OQString("and")) && !content.isEOS();
                        if (!getAnotherWord)
                        {
                            NAMESTATS nameStats;
                            nameStatLookup(word.getString(), globals, nameStats, Male);
                            isGivenName = nameStats.isGivenName;
                            isSurname = nameStats.isSurname;
                            if (isGivenName && !nameStats.isLikelySurname && !content.isEOS())
                            {
                                nextWord = content.peekAtWord();
                                getAnotherWord = true;
                            }
                            else
                            {
                                // Assume brand new name in limited circumstances
                                if (valid && !isGivenName && !isSurname)
                                    step2complete = true;
                            }
                        }

                        if (getAnotherWord && nextWord.isCapitalized() && (surnameLookup(nextWord.getString(), globals) > 0) && !content.isEOS())
                            word = content.getWord();
                        else
                        {
                            if (isSurname && word.isCapitalized())
                                step2complete = true;
                            else
                                keepGoing = false;
                        }
                    }
                }

                if (step2complete)
                {
                    brotherLastNames.append(word.getString());
                    found = true;
                }
            }
        }
    }

    // Now look at parent names
    targetPhrases.clear();
    targetPhrases = OQString::getParentReferences(lang);
    found = false;
    beg();
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

            // Prepare content to be processed
            content = uc.getSentence(lang, uc.getSentenceNum(index));
            sentence = content.getString();
            index = sentence.indexOf(targetText, 0, Qt::CaseInsensitive);
            sentence.remove(0, index + targetText.size());
            sentence.replace(QString("the late "), QString(""));
            sentence.replace(QString("de feu "), QString(""));
            content = sentence;
            content.removeLeading(PUNCTUATION);
            content.removeParenthesesContent();
            content.cleanUpEnds();

            // Beging processing
            while (keepGoing && !found && !highConfidence && !content.isEOS())
            {
                priorWord = word;

                // Get word
                word = content.getWord(true);
                if (word.isSaint())
                {
                    if (word.right(1) != period)
                        word += OQString(".");
                    word += content.getWord(true);
                }
                while (word.isUncapitalizedName() && !content.isEOS())
                {
                    word += content.getWord(true);
                }

                // Get next word
                tempWord = content.peekAtWord(true);
                if (tempWord.isSaint())
                {
                    if (tempWord.right(1) != period)
                        tempWord += OQString(".");
                    tempWord += content.peekAtWord(true, 2);
                    nextWord = tempWord;
                }
                i = 2;
                nextWord = tempWord;
                while (tempWord.isUncapitalizedName() && !content.isEOS())
                {
                    tempWord = content.peekAtWord(true, i);
                    nextWord += tempWord;
                    i++;
                }
                nextWord.removeEnding(PUNCTUATION);

                /*if (nextWord.removeBookEnds(PARENTHESES))
                {
                    nextWord = content.peekAtWord(true, 2);
                    nextWord.removeEnding(PUNCTUATION);
                }*/

               // Analyze word
                isAboriginal = false;
                if (word.isAboriginalName() && nextWord.isAboriginalName())
                    isAboriginal = true;
                compoundName = word.isCompoundName() || isAboriginal;
                lastChar = word.right(1);
                endOfNames = word.removeEnding(PUNCTUATION);
                isInitial = (word.getLength() == 1);
                endOfNames = (endOfNames && !isInitial) || content.isEOS();
                nextWordIsSurname = !endOfNames && (surnameLookup(nextWord.getString(), globals) > 0);

                if ((word.lower() == OQString("and")) || (word.lower() == OQString("et")) || (word.lower() == OQString("y")) || (word == OQString("&")))
                {
                    wordIsAnd = true;
                    andEncountered = true;
                    currentGender = genderUnknown;
                    parentNames.clear();
                }
                else
                    wordIsAnd = false;

                // Determine if this is the last word to be considered
                if (((lastChar == period) && !word.isSaint() && !isInitial) || (lastChar == semicolon) || endOfNames)
                    keepGoing = false;

                if (!word.hasBookEnds(PARENTHESES))
                {

                    if (word.isCapitalized() && !wordIsAnd)
                    {
                        // Stand alone analysis based on gender lookups
                        isGivenName = givenNameLookup(word.getString(), globals, genderUnknown);
                        isSurname = (surnameLookup(word.getString(), globals) > 0);

                        if (isGivenName && !isSurname)
                        {
                            parentNames.append(word.getString());

                            isGivenName = givenNameLookup(nextWord.getString(), globals, genderUnknown);
                            isSurname = (surnameLookup(nextWord.getString(), globals) > 0);

                            if (isGivenName && !isSurname)
                                parentNames.append(nextWord.getString());

                            unisex = genderLookup(parentNames, globals);

                            if (unisex >= 0.9)
                            {
                                maleNameEncountered = true;
                                currentGender = Male;
                                if (!isGivenName && isSurname && !endOfNames)
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
                                    if (!isGivenName && isSurname && !endOfNames)
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

    // Compare results of brothers to parents
    if (!found && (brotherLastNames.size() > 0))
    {
        name = brotherLastNames.takeFirst();
        highConfidence = true;
    }

    bool matched = false;
    while (found && !matched && (brotherLastNames.size() > 0))
    {
        tempWord = brotherLastNames.takeFirst();
        if (tempWord.lower() == name.lower())
        {
            matched = true;
            highConfidence = true;
        }
        else
        {
            if (tempWord.lower() == savedName.lower())
            {
                matched = true;
                highConfidence = true;
                name = savedName;
            }
        }
    }

    // Make due with whatever we have at this point
    if (!highConfidence)
    {
        if (maleNameEncountered && (maleLastName.getLength() == 0) && femaleNameEncountered && (femaleLastName.getLength() == 0) && name.getLength() > 0)
            highConfidence = true;
        else
        {
            if (maleNameEncountered && (maleLastName.getLength() == 0) && femaleNameEncountered && (femaleLastName.getLength() > 0))
            {
                name = femaleLastName;
                highConfidence = true;
            }
            else
            {
                int freq;
                if (maleNameEncountered && (maleLastName.getLength() > 0))
                {
                    freq = cleanedUpUC.countFrequency(maleLastName.getString(), Qt::CaseInsensitive);
                    if (freq >= 2)
                        highConfidence = true;
                }
                else
                {
                    if (femaleNameEncountered && (femaleLastName.getLength() > 0))
                    {
                        freq = cleanedUpUC.countFrequency(femaleLastName.getString(), Qt::CaseInsensitive);
                        if (freq >= 2)
                            highConfidence = true;
                    }
                }
            }
        }
    }    

    if (found || highConfidence)
    {
        if (highConfidence)
            tempName = name;
        else
        {
            if ((firstUnisex > 0.5) && (secondUnisex < 0.5) && (wordCount >= 4) && !priorWord.hasBookEnds(PARENTHESES))
                tempName = savedName;
            else
                tempName = name;
        }

        if (globals->globalDr->isALastName(tempName))
            highConfidence = true;

        globals->globalDr->setParentsLastName(tempName);
        globals->globalDr->removeFromMiddleNames(tempName);
        if (globals->globalDr->getGender() != Male)
            globals->globalDr->setFamilyName(tempName);
    }

    if (found && (wordCount >= 4) && !highConfidence)
    {
        PQString errMsg;
        errMsg << "Verify inclusion of (" << tempName << ") as parents name for: " << globals->globalDr->getURL();
        //globals->logMsg(ErrorRecord, errMsg);
        globals->globalDr->wi.checkParentsName = tempName.getString();
    }

}

void readObit::processMaidenNames()
{
    QList<PQString> nameList = globals->globalDr->getMaidenNames();
    if (nameList.size() == 0)
        return;

    QString name, drName;
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
        drName = name;
        globals->globalDr->setFamilyName(drName);
    }
}

void readObit::sortFirstNames()
{
    OQString temp, firstName, alt1Name, alt2Name;
    PQString warningMessage;
    bool switchNames;
    PQString name;

    alt1Name = globals->globalDr->getFirstNameAKA1().lower();
    if (alt1Name.getLength() == 0)
        return;

    firstName = globals->globalDr->getFirstName().lower();
    switchNames = firstName.isInformalVersionOf(alt1Name.getString(), warningMessage);

    if (switchNames)
    {
        name = alt1Name;
        globals->globalDr->setFirstName(name, 1);

        name = firstName;
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
            name = alt2Name;
            globals->globalDr->setFirstName(name, 1);

            name = firstName;
            globals->globalDr->setFirstName(name, 2);

            name = alt1Name;
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
    hyphenatedLastName = false;
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
    if (!dates.hasDateInfo())
        return false;

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
        //PQString errMsg;
        //errMsg << "Encountered issues in reading dates for: " << globals->globalDr->getURL();
        //globals->logMsg(ErrorRecord, errMsg);
        globals->globalDr->wi.dateFlag = 1;
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

int readObit::runNameValidations()
{
    NAMESTATS nameStats;
    PQString name;
    unsigned int count, size;
    int warningScore = 0;
    GENDER gender = globals->globalDr->getGender();


    // Checks on last names
    int i = 0;
    while (i < 4)
    {
        switch (i)
        {
        case 0:
            name = globals->globalDr->getLastName();
            break;
        case 1:
            name = globals->globalDr->getLastNameAlt1();
            break;
        case 2:
            name = globals->globalDr->getLastNameAlt2();
            break;
        case 3:
            name = globals->globalDr->getLastNameAlt3();
            break;
        }

        size = name.getLength();

        if (size > 0)
        {
            // Match up with appropriate nameStatsList item
            bool matched = false;
            int j = 0;
            while (!matched && (j < nameStatsList.size()))
            {
                if (name.lower() == nameStatsList[j].name)
                {
                    nameStats = nameStatsList[j];
                    matched = true;
                }
                j++;
            }

            if (!matched)
                nameStatLookup(name.getString(), globals, nameStats, gender);

            // Add points for errors
            if ((name.findPosition(PQString("(")) != -1) || (name.findPosition(PQString(")")) != -1))
                warningScore += 10;

            // Add points if it is also a saved given name
            if (globals->globalDr->isAFirstName(name) || globals->globalDr->isAMiddleName(name))
                warningScore += 10;

            // Add points if it is equal to "Nee"
            if (name.lower() == PQString("nee") || name.lower() == PQString("née"))
                warningScore += 15;

            // Add points if it is an initial
            if (size == 1)
                warningScore += 10;

            // Add points if ending in possessive
            if ((name.right(2) == PQString("'s")) || (name.right(2) == PQString("s'")))
                warningScore += 10;

            // Add points if it is not a common surname
            switch(nameStats.surnameCount)
            {
            case 0:
                warningScore += 2;
                break;

            case 1:
            case 2:
                warningScore += 1;
                break;

            default:
                warningScore += 0;
            }

            if (nameStats.isLikelyGivenName)
                warningScore += 5;

            // Add points if it is also a given name
            switch (gender)
            {
            case Male:
                count = nameStats.maleCount;
                break;

            case Female:
                count = nameStats.femaleCount;
                break;

            default:
                count = nameStats.maleCount + nameStats.femaleCount;
            }

            if (count > 0)
                warningScore += 1;

            if (count > 100)
                warningScore += 1;

            i++;
        }
        else
            i = 4;
    }

    // Checks on first names
    // Intentionally just checking first name for now
    i = 0;
    while (i < 1)
    {
        switch (i)
        {
        case 0:
            name = globals->globalDr->getFirstName();
            break;
        case 1:
            name = globals->globalDr->getFirstNameAKA1();
            break;
        case 2:
            name = globals->globalDr->getFirstNameAKA2();
            break;
        }

        if (name.getLength() > 0)
        {
            // Match up with appropriate nameStatsList item
            bool matched = false;
            int j = 0;
            while (!matched && (j < nameStatsList.size()))
            {
                if (name.lower() == nameStatsList[j].name)
                {
                    nameStats = nameStatsList[j];
                    matched = true;
                }
                j++;
            }

            if (!matched)
                nameStatLookup(name.getString(), globals, nameStats, gender);

            // Add points for errors
            if ((name.findPosition(PQString("(")) != -1) || (name.findPosition(PQString(")")) != -1))
                warningScore += 10;

            if (nameStats.surnameCount > 0)
                warningScore += 1;

            if (nameStats.isLikelySurname)
                warningScore += 5;

            // Add points if it is a rare given name
            switch (gender)
            {
            case Male:
                count = nameStats.maleCount;
                break;

            case Female:
                count = nameStats.femaleCount;
                break;

            default:
                count = nameStats.maleCount + nameStats.femaleCount;
            }

            if (count < 10)
                warningScore += 1;

            if (count == 0)
                warningScore += 3;

            i++;
        }
        else
            i = 3;
    }

    globals->globalDr->wi.nameFlagGeneral = warningScore;
    return warningScore;
}

int readObit::runRecordValidation()
{
    bool valid = true;
    bool finished = false;
    int check = 1;

    while (valid && !finished)
    {
        switch(check)
        {
        case 1:
            if (globals->globalDr->getLastName().getLength() < 2)
                valid = false;
            break;

        case 2:
            if (globals->globalDr->getFirstName().getLength() == 0)
                valid = false;
            break;

        case 3:
            if (globals->globalDr->getGender() == genderUnknown)
                valid = false;
            break;

        case 4:
            if(!globals->globalDr->getDOD().isValid() || (globals->today < globals->globalDr->getDOD()) || (globals->globalDr->getDOD().daysTo(globals->today) > 30))
                valid = false;
            break;

        case 5:
            if (globals->globalDr->getLanguage() == language_unknown)
                valid = false;
            break;

        case 6:
            if ((globals->globalDr->wi.ageFlag > 0) || (globals->globalDr->wi.dateFlag > 0) || (globals->globalDr->wi.genderFlag > 0) || (globals->globalDr->wi.nameReversalFlag > 0) ||
                (globals->globalDr->wi.nameFlagGeneral > 8) || (globals->globalDr->wi.doubleMemorialFlag > 0) || (globals->globalDr->wi.outstandingErrMsg > 0))
                valid = false;
            break;

        case 7:
            if ((globals->globalDr->wi.checkParentsName.length() > 0) || (globals->globalDr->wi.checkInclName.length() > 0) || (globals->globalDr->wi.checkExclName.length() > 0) ||
                (globals->globalDr->wi.confirmTreatmentName.length() > 0) || (globals->globalDr->wi.confirmMiddleName.length() > 0))
                valid = false;
            break;

        case 8:
            if ((globals->globalDr->getGender() == Male) && (globals->globalDr->getLastNameAlt1().getLength() > 0))
                valid = false;
            break;

        }
        check++;
        if (check > 8)
            finished = true;
    }

    if (valid)
    {
        globals->globalDr->wi.validated = 1;
        return 1;
    }
    else
        return 0;
}

int readObit::runGenderValidation()
{
    double unisex = genderLookup(globals->globalDr, globals);
    PQString writtenGender, errMsg;
    GENDER gender = globals->globalDr->getGender();

    if (gender == Male)
        writtenGender = PQString("Male");
    else
        writtenGender = PQString("Female");

    if (((unisex >= 0.9) && (gender == Female)) || ((unisex <= 0.1) && (gender == Male)))
    {
        globals->globalDr->wi.genderFlag = static_cast<int>(gender);
        return 0;
    }
    else
        return 1;
}

void readObit::determinePotentialFirstName()
{
    // First step is to check if unstructured content starts with "LASTNAME, FIRSTNAME", otherwise second step may erroneously return the lastname as a potential first name if
    // the real first name does not appear in the obit at least twice

    OQString firstWord, secondWord, thirdWord;
    bool firstWordHadComma, secondWordHadComma, secondWordInParentheses, secondWordWasHyphen;
    NAMESTATS firstWordStats, secondWordStats, thirdWordStats;

    unstructuredContent tempUC = uc;

    firstWord = tempUC.getWord();
    firstWordHadComma = firstWord.removeEnding(QString(","));
    if (!firstWordHadComma)
        firstWordHadComma = firstWord.removeEnding(QString(";"));
    if (!firstWordHadComma)
        firstWordHadComma = firstWord.removeEnding(QString(":"));
    secondWord = tempUC.getWord();
    if (secondWord.isHyphen())
    {
        secondWordWasHyphen = true;
        secondWord = tempUC.getWord();
    }
    else
        secondWordWasHyphen = false;
    secondWordHadComma = secondWord.removeEnding(QString(","));
    secondWordInParentheses = secondWord.removeBookEnds(PARENTHESES);

    // Deal with rate  "Smith (Johnson), Michelle"
    if (secondWordHadComma && secondWordInParentheses)
    {
        thirdWord = tempUC.getWord();
        thirdWord.removeEnding(PUNCTUATION);
        if ((thirdWord.lower() != firstWord.lower() && (thirdWord.lower() != secondWord.lower())))
        {
            nameStatLookup(thirdWord.getString(), globals, thirdWordStats, globals->globalDr->getGender());
            if (thirdWordStats.isLikelyGivenName)
            {
                globals->globalDr->setPotentialFirstName(thirdWord.getString());
                return;
            }
        }
    }

    nameStatLookup(firstWord.getString(), globals, firstWordStats, globals->globalDr->getGender());
    nameStatLookup(secondWord.getString(), globals, secondWordStats, globals->globalDr->getGender());

    if ((firstWordHadComma || secondWordWasHyphen) && (firstWordStats.isLikelySurname || secondWordStats.isLikelyGivenName || (!secondWordStats.isSurname && secondWordStats.isGivenName)))
        globals->globalDr->setPotentialFirstName(secondWord.getString());
    else
        globals->globalDr->setPotentialFirstName(mcn.readPotentialFirstName(globals));
}

void readObit::runStdProcessing()
{
    uc.simplify(true);
    uc.replaceHTMLentities();
    uc.removeHTMLtags(true);
    uc.removeBlankSentences();
    uc.fixQuotes();
    uc.fixBasicErrors();
    uc.cleanUpEnds();
}

void readObit::addressUnknownGender()
{
    // These are designed to run after ReadStructured's execution

    int strategy = 1;

    while ((globals->globalDr->getGender() == genderUnknown) && (strategy < 5))
    {
        switch (strategy)
        {
        case 1:
            genderByTitle();
            break;

        case 2:
            genderByAltName();
            break;

        case 3:
            genderByUnionReference();
            break;

        case 4:
            genderByNameLookup();
            break;
        }

        strategy++;
    }
}

void readObit::addressMissingGender()
{
    // These are designed to run prior to ReadStructured's execution

    int strategy = 1;

    while ((globals->globalDr->getGender() == genderUnknown) && (strategy < 4))
    {
        switch (strategy)
        {
        case 1:
            genderBySentencesStartingWithPronouns();
            break;

        case 2:
            genderByRelationshipWords();
            break;

        case 3:
            genderByTitle();
            break;
        }

        strategy++;
    }
}

void readObit::genderBySentencesStartingWithPronouns()
{
    // Check if any sentences start with a gender pronoun
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
}

void readObit::genderByRelationshipWords()
{
    // Check first three sentences for "relationship words"
    bool unknownGender = (globals->globalDr->getGender() == genderUnknown);
    unsigned int numChecked = 0;
    unsigned int maxSentences = 3;  // information pulled from fourth or later sentence could be anything, so stop reading after three sentences
    unstructuredContent sentence;
    uc.beg();

    while (unknownGender && !uc.isEOS() && (numChecked < maxSentences))
    {
        sentence = uc.getSentence();
        if (!(sentence.isJustDates() || sentence.isJustNames()))
        {
            unknownGender = sentence.genderRelationalReferences();
            numChecked++;
        }
    }
}

void readObit::genderByTitle()
{
    // Only works after ReadStructured has executed

    bool unknownGender = (globals->globalDr->getGender() == genderUnknown);
    unstructuredContent sentence;

    if (unknownGender)
    {
        uc.beg();

        OQString word;
        GENDER potentialGender = genderUnknown;

        sentence = uc.getSentence();
        while (sentence.isJustDates() || sentence.isJustNames())
            sentence = uc.getSentence();

        sentence.beg();
        word = sentence.getWord();
        if (word.isTitle())
        {
            if (word.isMaleTitle())
                potentialGender = Male;
            if (word.isFemaleTitle())
                potentialGender = Female;

            if (potentialGender != genderUnknown)
            {
                word = sentence.getWord();
                if (globals->globalDr->isASavedName(word))
                    globals->globalDr->setGender(potentialGender);
            }
        }
    }
}

void readObit::genderByAltName()
{
    // If altName exists and last name is not hyphenated, assume female except for "Singh"
    if (globals->globalDr->getGender() == genderUnknown)
    {
        if ((globals->globalDr->getLastNameAlt1().getLength() > 0) && !hasHyphenatedName())
        {
            if (!globals->globalDr->isALastName(PQString("Singh")))
                globals->globalDr->setGender(Female);
        }
    }
}

void readObit::genderByUnionReference()
{
    // Look for obits published by Unions for "Brother" or "Sister"
    if (globals->globalDr->getGender() == genderUnknown)
    {
        int index1, index2;
        bool isUnion = false;
        QString target = uc.getString();
        PQString potentialName;
        GENDER potentialGender = genderUnknown;

        isUnion = target.contains(QString("Local"), Qt::CaseSensitive) || target.contains(QString("Lodge"), Qt::CaseSensitive);
        if (isUnion)
        {
            index1 = target.indexOf(QString("Brother "), Qt::CaseSensitive);
            if (index1 >= 0)
                potentialGender = Male;
            else
            {
                index1 = target.indexOf(QString("Sister "), Qt::CaseSensitive);
                if (index1 >= 0)
                    potentialGender = Female;
                else
                {
                    index1 = target.indexOf(QString("Brother, "), Qt::CaseSensitive);
                    if (index1 >= 0)
                        potentialGender = Male;
                    else
                    {
                        index1 = target.indexOf(QString("Sister, "), Qt::CaseSensitive);
                        if (index1 >= 0)
                            potentialGender = Female;
                    }
                }
            }

            if (potentialGender != genderUnknown)
            {
                index2 = target.indexOf(QString(" "), index1);
                index1 = target.indexOf(QString(" "), index2 + 1);
                potentialName = target.mid(index2 + 1, index1 - index2 - 1);
                if (globals->globalDr->isAName(potentialName))
                    globals->globalDr->setGender(potentialGender);
            }
        }
    }
}

void readObit::genderByNameLookup()
{
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
}
