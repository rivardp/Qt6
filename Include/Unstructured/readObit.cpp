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
    bool SetStyle, SetLanguage, ObitTitle, ObitAddress, ObitGender, ObitDates, ObitText, ReadStructured, ReadUnstructured, TitleInfo;
    bool ReadMessages, ReadPublishDate, UseFirstDateAsDOD, UndoBadSentence, DontInsertPeriods;
    int runTypes, InsertPeriods;
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
    //   64 ObitAddress
    //  128 ReadMessages
    //  256 ReadStructured
    //  512 StructuredInfoFullyCredible
    // 1024 TitleInfo
    // 2048 ReadUnstructured
    // 4096 ReadPublishDate
    // 8192 UseFirstDateAsDOD
    //16384 UndoBadSentence
    //32768 DontInsertPeriods

    SetStyle = false;
    SetLanguage = false;
    ObitTitle = false;
    ObitGender = false;
    ObitDates = false;
    ObitText = false;
    ObitAddress = false;
    ReadMessages = false;
    ReadStructured = false;
    //StructuredInfoFullyCredible = false;
    TitleInfo = false;
    ReadUnstructured = false;
    ReadPublishDate = false;
    UseFirstDateAsDOD = false;
    UndoBadSentence = false;
    DontInsertPeriods = false;
    InsertPeriods = 2;     // insertPeriods == 2 inserts at all break tags while == 1 excludes <br> and some dates


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
        ObitAddress = true;
    if ((runTypes & 128) == 128)
        ReadMessages = true;
    if ((runTypes & 256) == 256)
        ReadStructured = true;
/*    if ((runTypes & 512) == 512)
        StructuredInfoFullyCredible = true;*/
    if ((runTypes & 1024) == 1024)
        TitleInfo = true;
    if ((runTypes & 2048) == 2048)
        ReadUnstructured = true;
    if ((runTypes & 4096) == 4096)
        ReadPublishDate = true;
    if ((runTypes & 8192) == 8192)
        UseFirstDateAsDOD = true;
    if ((runTypes & 16384) == 16384)
        UndoBadSentence = true;
    if ((runTypes & 32768) == 32768)
        DontInsertPeriods = true;

    if (DontInsertPeriods)
        InsertPeriods--;

    // Execute based on settings
    globals->websiteLocationWords = globals->globalObit->getLocationWords(globals->globalDr->getProvider(), globals->globalDr->getProviderKey());
    replaceProblematicChars();
    if (SetStyle)
        readStyle();
    if (SetLanguage)
        readLanguage();
    if (ObitTitle)
        readInObitTitle();
    if (!globals->globalDr->getPostalCodeInfo().isValid())
        readInObitAddress();
    if (ObitAddress)
        readInCustomAddress();
    if (ObitGender)
        readGender();
    if (ObitDates)
        readInObitDates();
    if (ReadPublishDate)
        readPublishDate();
    if (ReadMessages)
        readMessages();
    if (ObitText)
    {
        readInObitText();
        processObitText(UndoBadSentence, InsertPeriods);
    }
    if (ReadStructured)
        readAndPrepareStructuredContent();
    if (ObitText)
    {
        addressMissingGender();
        determinePotentialFirstName();
        readServiceDate();
    }
    if (ReadStructured)
        processStructuredNames();
    if (TitleInfo)
        readTitleInfo();
    if ((globals->globalDr->getGender() != Male) && !globals->globalDr->getNeeEtAlEncountered()
                                                 && (globals->globalDr->getMaidenNames().size() == 0))
        readParentsLastName();
    if (ReadUnstructured)
         readUnstructuredContent(UseFirstDateAsDOD);
    if (1)
    {
        readImageNameInfo();
        globals->globalDr->setMinMaxDOB();
        fixProviderSpecificIssues();
        globals->globalDr->runDateValidations();
        fixNameIssues();
        globals->globalDr->wi.resetNonDateValidations();
        finalNameCleanup();
        treatmentNameCleanup();
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

    case Arbor:
    case Vortex:
    case ToraPro:
    case Descary:
        language = multiple_unknown;
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
    int providerKey = globals->globalDr->getProviderKey();
    beg();

    switch(globals->globalDr->getProvider())
    {
    default:
        break;

    case FuneralOne:
        // Field is not entered correctly on a reliable basis, so some providers eliminated
        bool excluded = ((providerKey == 38134) || (providerKey == 40845) || (providerKey == 736));
        if (!excluded)
        {
            if (moveTo("itemprop=\"gender\""))
            {
                field = readNextBetween(BRACKETS);
                if (field == OQString("Male"))
                    gender = Male;
                if (field == OQString("Female"))
                    gender = Female;
            }
        }
        break;
    }

    globals->globalDr->setGender(gender);
}

void readObit::readInObitText()
{
    // Check if robots are directed to ignore
    beg();
    QString robotDirections;
    if (consecutiveMovesTo(35, "<meta", "robots", "content="))
    {
        robotDirections = readQuotedMetaContent().getString();
        if (robotDirections.contains("noindex"))
            globals->globalDr->wi.futureUse += 1;
        if (robotDirections.contains("nofollow"))
            globals->globalDr->wi.futureUse += 2;
        if (robotDirections.contains("none"))
            globals->globalDr->wi.futureUse += 4;
    }

    // Basic read specified for each provider, with standard processing for all coded at end
    beg();
    unstructuredContent checkContent;

    QString target;

    switch(globals->globalDr->getProvider())
    {
    case Legacy:
        switch (style)
        {
        case 0:
            if (consecutiveMovesTo(500, "ObitTextHtml", ">"))
                uc = getUntil("<!--  -->");
            break;

        case 1:
            if (moveTo("\"articleBody\": "))
            {
                uc = getUntil("\"creator\":");
                uc.cleanUpEnds();
                uc.removeEnding(COMMA);
                uc.removeBookEnds(QUOTES);
            }
            break;

        case 2:
            if (consecutiveMovesTo(40, "class=\"details-copy", ">"))
                uc = getUntil("<div class=\"details-published\">");
            break;

        case 3:
            uc = globals->globalDr->getObitSnippet();
            break;

        }

        break;

    case Passages:
        if (moveTo("class=\"obituary_body\""))
        {
            if (conditionalMoveTo("textsize", "</p>", 0))
                moveTo("<p>");
            else
                moveTo(">");
            uc = getUntil("As published in");
        }
        break;

    case LifeMoments:
        break;

    case CooperativeFuneraire:
        if (moveTo("class=\"tj_field tj_description\">"))
            uc = getUntil("<div class=");
        else
        {
            beg();
            moveTo("class=\"obituary-years\"");
            if (conditionalMoveTo("<p", "<div class=", 1))
                moveTo(">");
            else
                consecutiveMovesTo(1000, "<div class=", "</div");
            uc = getUntil("<div class=\"obituary-form__wrapper");
//            if (consecutiveMovesTo(1000, "class=\"obituary-title\"", "<p", ">"))
//                uc = getUntil("<div class=");
        }
        break;

    case DomainFuneraire:
        if (consecutiveMovesTo(50, "mt-10 text-sm wysiwyg", ">"))
        {
            uc = getUntil("</div>");
            uc.replace("magnus poirier inc.", "", Qt::CaseInsensitive);
        }
        break;

    case GreatWest:
    {
        switch(globals->globalDr->getProviderKey())
        {
        case 18:
            if (consecutiveMovesTo(150, "id=\"details-body\"", ">"))
                uc = getUntil("<div");
            break;

        default:
            QRegularExpression target;
            target.setPattern("(Sibling|sibling)");
            itsString.replace(target, QString(" 99999 \\1"));

            if (consecutiveMovesTo(150, "id=\"details-body\"", "itemprop=\"articleBody\">"))
                uc = getUntil("<div id=\"details-widgets\">");
            break;
        }
    }
        break;

    case EchoVita:
        if (consecutiveMovesTo(200, "id=\"obituary\"", "</h1>"))
        {
            uc = getUntil("</div>");

            // Remove (Ashern, Manitoba)
            QRegularExpression target;
            target.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            target.setPattern("\\s+\\(\\w+,\\s+Manitoba\\)");
            QString tempString = uc.getString();
            tempString.replace(target, "");
            uc = tempString;
        }
        break;

    case SaltWire:
        if (consecutiveMovesTo(1000, "class=\"sw-page__title\"", "<div class=\"row\">"))
        {
            uc = getUntil("<h2>More Obituaries</h2>");
            /*uc.replace("Albert Bridge", "", Qt::CaseInsensitive);
            uc.replace("Alder Point", "", Qt::CaseInsensitive);
            uc.replace("Balls Creek", "", Qt::CaseInsensitive);
            uc.replace("Barachois Brook", "", Qt::CaseInsensitive);
            uc.replace("Bear Reserve", "", Qt::CaseInsensitive);
            uc.replace("Bible Hill", "", Qt::CaseInsensitive);
            uc.replace("Big Bras d’Or", "", Qt::CaseInsensitive);
            uc.replace("Birch Grove", "", Qt::CaseInsensitive);
            uc.replace("Blackett's Lake", "", Qt::CaseInsensitive);
            uc.replace("Bush Island", "", Qt::CaseInsensitive);
            uc.replace("Cariboo Marsh", "", Qt::CaseInsensitive);
            uc.replace("Caribou Marsh", "", Qt::CaseInsensitive);
            uc.replace("Chance Harbour", "", Qt::CaseInsensitive);
            uc.replace("Corner Brook", "", Qt::CaseInsensitive);
            uc.replace("Dalem Lake", "", Qt::CaseInsensitive);
            uc.replace("Dutch Brook", "", Qt::CaseInsensitive);
            uc.replace("Egan Street", "", Qt::CaseInsensitive);
            uc.replace("Enfield", "", Qt::CaseInsensitive);
            uc.replace("Florence", "", Qt::CaseInsensitive);
            uc.replace("Fox Harbour", "", Qt::CaseInsensitive);
            uc.replace("Gabarus Lake", "", Qt::CaseInsensitive);
            uc.replace("Gardiner Mines", "", Qt::CaseInsensitive);
            uc.replace("George's River", "", Qt::CaseInsensitive);
            uc.replace("Glace Bay", "", Qt::CaseInsensitive);
            uc.replace("Halifax", "", Qt::CaseInsensitive);
            uc.replace("Hillside Road", "", Qt::CaseInsensitive);
            uc.replace("Howie Center", "", Qt::CaseInsensitive);
            uc.replace("Howie Centre", "", Qt::CaseInsensitive);
            uc.replace("Ingonish Beach", "", Qt::CaseInsensitive);
            uc.replace("Keltic Drive", "", Qt::CaseInsensitive);
            uc.replace("Kentville", "", Qt::CaseInsensitive);
            uc.replace("Leitches Creek", "", Qt::CaseInsensitive);
            uc.replace("Little Bras d’Or", "", Qt::CaseInsensitive);
            uc.replace("Little Pond", "", Qt::CaseInsensitive);
            uc.replace("Louisbourg", "", Qt::CaseInsensitive);
            uc.replace("Main-A-Dieu", "", Qt::CaseInsensitive);
            uc.replace("Malay Falls", "", Qt::CaseInsensitive);
            uc.replace("Marble Mountain", "", Qt::CaseInsensitive);
            uc.replace("Marion Bridge", "", Qt::CaseInsensitive);
            uc.replace("Mill Creek", "", Qt::CaseInsensitive);
            uc.replace("Millville", "", Qt::CaseInsensitive);
            uc.replace("Mira Road", "", Qt::CaseInsensitive);
            uc.replace("Moncton", "", Qt::CaseInsensitive);
            uc.replace("Myers Point", "", Qt::CaseInsensitive);
            uc.replace("New Waterford", "", Qt::CaseInsensitive);
            uc.replace("North Sydney", "", Qt::CaseInsensitive);
            uc.replace("Pictou", "", Qt::CaseInsensitive);
            uc.replace("Pipers Cove", "", Qt::CaseInsensitive);
            uc.replace("Point Edward", "", Qt::CaseInsensitive);
            uc.replace("Port Arthur", "", Qt::CaseInsensitive);
            uc.replace("Port Hawkesbury", "", Qt::CaseInsensitive);
            uc.replace("Port Morien", "", Qt::CaseInsensitive);
            uc.replace("Porters Lake", "", Qt::CaseInsensitive);
            uc.replace("Prime Brook", "", Qt::CaseInsensitive);
            uc.replace("Red Deer", "", Qt::CaseInsensitive);
            uc.replace("Reserve Mines", "", Qt::CaseInsensitive);
            uc.replace("River Denys", "", Qt::CaseInsensitive);
            uc.replace("River Ryan", "", Qt::CaseInsensitive);
            uc.replace("Scotch Lake", "", Qt::CaseInsensitive);
            uc.replace("Sheet Harbour", "", Qt::CaseInsensitive);
            uc.replace("Smelt Brook", "", Qt::CaseInsensitive);
            uc.replace("South Bar", "", Qt::CaseInsensitive);
            uc.replace("South Haven", "", Qt::CaseInsensitive);
            uc.replace("South Sydney", "", Qt::CaseInsensitive);
            uc.replace("Spurgeon", "", Qt::CaseInsensitive);
            uc.replace("St. Andrew Maria", "", Qt::CaseInsensitive);
            uc.replace("St. Dolores", "", Qt::CaseInsensitive);
            uc.replace("Sydney Forks", "", Qt::CaseInsensitive);
            uc.replace("Sydney Mines", "", Qt::CaseInsensitive);
            uc.replace("Sydney River", "", Qt::CaseInsensitive);
            uc.replace("Sydney/", "", Qt::CaseInsensitive);
            uc.replace("West Arichat", "", Qt::CaseInsensitive);
            uc.replace("Westmount", "", Qt::CaseInsensitive);
            uc.replace("Weymouth", "", Qt::CaseInsensitive);
            uc.replace("White Point", "", Qt::CaseInsensitive);
            uc.replace("Whitney Pier", "", Qt::CaseInsensitive);*/
        }
        break;

    case BlackPress:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(100, "class=\"entry-content", ">"))
                uc = getUntilEarliestOf("<!-- AI CONTENT END 2 -->", "Share<");
            break;

        case 1:
            if (consecutiveMovesTo(125, "id=\"details-body\"", ">"))
                uc = getUntil("</div>");
            break;
        }

        break;

    case Aberdeen:
        if (consecutiveMovesTo(30, "class=\"obit-content\"", ">"))
            uc = getUntil("<div");
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
                        uc = readQuotedMetaContent();
                }
            }
            break;

        case 1:
        case 4:
        case 6:
            if (consecutiveMovesTo(350, "article class=", "obituary", "section id=\"obit-text\"", ">"))
                uc = getUntil("</section>", 5000, false);

            // Do backup read if necessary (jpeg or no content)
            if (uc.getLength() == 0)
            {
                beg();
                if (consecutiveMovesTo(20, "og:description", "content="))
                    uc = readQuotedMetaContent();
            }
            break;

        case 2:
        case 5:
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
            // Want to avoid reading same info as in readStructured, which is also going to >OBITUARY<

            if (consecutiveMovesTo(3000, ">OBITUARY<", "class=\"text-center mt-30\"", ">"))
                uc = getUntil("</div>");
            else
            {
                beg();
                if (moveTo(">OBITUARY<"))
                {
                    //conditionalMoveTo("class=\"fixed-container mt-30", "short-bio text", 0);
                    moveToEarliestOf("<a", "<h2", "short-bio text");
                    backward(1);
                    QString temp = get().getString();
                    if (temp == QString("a"))
                        moveTo("short-bio text");
                    if (temp == QString("2"))
                    {
                        moveTo(">");
                        uc = getUntil("</div>");
                        moveTo("short-bio text");
                    }
                    moveTo(">");
                    uc = uc.getString() + getUntil("</div>").getString();
                }
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
        if ((globals->globalDr->getProviderKey() > 90000) || (globals->globalDr->getProviderKey() == 824))
        {
            // Old style obit
            target = QString("id=\"obituary\"");
            if (consecutiveMovesTo(400, target, ">"))
                uc = getUntil("<script");
        }
        else
        {
            // New style obit
            if (style == 2)
            {
                if (consecutiveMovesTo(200, "id=\"obt-p2\"", "</p>"))
                {
                    uc = getUntil("<p id=\"ob-more\"");
                    runStdProcessing(uc, false);
                }

                if (uc.getLength() < 10)
                {
                    beg();
                    if (consecutiveMovesTo(200, "id=\"obt-p2\"", "<p>"))
                    {
                        uc = getUntil("<p id=\"ob-more\"");
                        runStdProcessing(uc, false);
                    }

                    if (uc.getLength() < 10)
                    {
                        beg();
                        if (consecutiveMovesTo(200, "id=\"obt-p1\"", "<p", ">"))
                            uc = getUntilEarliestOf("<p id=\"ob-more\"", "<p>Tribute Wall</p>");
                    }
                }
            }
            else
            {
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
        switch(style)
        {
        case 0:
        case 1:
            if (consecutiveMovesTo(300, "\"obituary-text", "Obituary of", "</h1>"))
            {
                if (conditionalMoveTo("<p", "<div class=\"obituary-button-row\"", 0))
                    backward(2);
                unsigned int position = getPosition();
                if (consecutiveMovesTo(75, "<p>", "B I O G R A P H Y", "</p>"))
                    uc = getUntil("</div>");
                else
                {
                    beg();
                    forward(position);
                    QString target;
                    QString target1("<span class=\"tribute-store-msg tree-store-message\">");
                    QString target2("obituary-button-row");
                    if (conditionalMoveTo(target1, target2, 0))
                        target = target1;
                    else
                        target = target2;

                    beg();
                    forward(position);
                    uc = getUntil(target);
                }
            }
            break;

        case 2:
            break;

        case 3:
            if (consecutiveMovesTo(750, "application/ld+json", "birthDate", "description", ":"))
            {
                uc = getUntil("}");
                uc.replaceHTMLentities();
                uc.cleanUpEnds();
                uc.removeBookEnds(QUOTES);
            }
            break;

        case 4:
            if (consecutiveMovesTo(50, "og:description", "content="))
                uc = readQuotedMetaContent();
            break;

        case 5:
            if (consecutiveMovesTo(1000, "tribute-book-obit", "View Tribute Book"))
            {
                consecutiveMovesTo(100, "<p", ">");
                uc = getUntil("</div>");
            }
            break;
        }

        if (uc.getLength() == 0)
        {
            beg();
            if (consecutiveMovesTo(300, "\"obituary-text", "</h1>"))
            {
                if (conditionalMoveTo("<p", "<div class=\"obituary-button-row\"", 0))
                    backward(2);
                uc = getUntil("</div>");
            }
        }

        if (uc.getLength() == 0)
        {
            beg();
            if (consecutiveMovesTo(50, "og:description", "content="))
                uc = readQuotedMetaContent();
        }
        break;

    case WordPress:
        if (moveTo("article id=\"post-"))
        {
            if (moveTo("<p>"))
                uc = getUntil("<footer");
        }
        break;

    case Burke:
    case FrontRunner:
    {
        if (globals->globalDr->getProviderKey() == 25)
        {
            if (consecutiveMovesTo(50, "og:description", "content="))
                uc = readQuotedMetaContent();
            return;
        }

        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(1000, "bom-obituary-wrapper", "<div", ">"))
                uc = getUntil("</div>");
            else
            {
                beg();
                if (consecutiveMovesTo(200, "og:title", "og:description", "content="))
                    uc = readQuotedMetaContent();
            }
            break;

        case 1:
            if (consecutiveMovesTo(2000, "class=\"obituary-title\"", "<div>"))
                uc = getUntil("</div>");
            break;

        case 2:
        case 3:
            if (moveTo("class=\"read-more-content\">"))
                uc = getUntil("</div>");

            // Extra coding to fix Kennedy Funeral Home ugliness
            uc.cleanUpEnds();
            if (uc.left(5) == PQString("Watch"))
            {
                int index1, index2;
                index1 = uc.findPosition("http");
                if (index1 > 0)
                {
                    index2 = uc.findPosition(" ", 1, index1 + 1);
                    PQString tempString = uc.getString();
                    tempString.dropLeft(index2);
                    uc.setItsString(QString("Passed ") + tempString.getString());
                }
            }
            break;

        case 4:
            if (consecutiveMovesTo(50, "og:description", "content="))
                uc = readQuotedMetaContent();
            break;

        case 5:
            uc.clear();
            break;

        case 6:
            if (consecutiveMovesTo(750, "application/ld+json", "birthDate", "description", ":"))
            {
                uc = getUntil("}");
                uc.cleanUpEnds();
                uc.removeBookEnds(QUOTES);
            }
            break;

        case 7:
            if (moveToEarliestOf(">Obituary<", ">Obituary <"))
            {
                moveTo(">");
                int position = getPosition();
                if (moveTo(">elevated", 100))
                    moveTo(">");
                else
                {
                    beg();
                    forward(position);
                    moveTo(">");
                }
                uc = getUntil("</div>");
            }
            else
            {
                beg();
                if(consecutiveMovesTo(1000, "\"entry-title\"", ">elevated", "</p>"))
                    uc = getUntil("</div>");
                else
                {
                    beg();
                    if (moveTo("<!-- . et_post_meta_wrapper-->"))
                        uc = getUntil("</div>");
                }
            }
            break;
        }

        QRegularExpression target;
        target.setPattern("(>)elevated[^<]*(<)");
        QString tempString = uc.getString();
        tempString.replace(target, "\\1\\2");
        uc = tempString;
        uc.removeHTMLtags();
        uc.cleanUpEnds();
        if ((uc.getLength() == 0) || (uc.left(11) == OQString("Coming soon")))
        {
            beg();
            if (consecutiveMovesTo(30, "class=\"obitTitle\"", " for "))
                uc = getUntil("<");
            else
            {
                beg();
                if (consecutiveMovesTo(50, "og:description", "content="))
                    uc = readQuotedMetaContent();
            }
            uc.removeLeading("View ");
        }
        break;
    }
        break;

    case FuneralOne:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(100, "class=\"header-obituary\"", ">"))
                uc = getUntil("<a href=");
            break;

        case 1:
            if (consecutiveMovesTo(50, "class=\"obit-dates-wrapper", ">"))
                uc = getUntil("</div>");
            break;
        }
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

        case 2:
            if (consecutiveMovesTo(200, "<!-- start obit tab-->", "<p>"))
                uc = getUntil("<!--obit text-->");
            break;
        }
        break;

    case GasLamp:
        if (moveTo("class=\"obit_text\">"))
            uc = getUntil("<!-- /.obit-border -->");
        break;

    case ClickTributes:
        if (consecutiveMovesTo(500, "class=\"ct-tab-content\"", "Print Tribute"))
            uc = getUntil("<!--/ct-tab-obituary-->");
        else
        {
            beg();
            if (consecutiveMovesTo(500, "class=\"ct-tab-content\"", "Print Obituary"))
                uc = getUntil("<!--/ct-tab-obituary-->");
        }
        break;

    case ConnellyMcKinley:
        if (consecutiveMovesTo(4000, "original-source", "</div>"))
            uc = getUntil("<footer class=\"entry-footer\">");
        break;

    case Arbor:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(100, "div itemprop=\"description\"", ">"))
                uc = getUntil("</div>");
            break;

        case 1:
            if(moveTo("<div class=\"rte\">"))
                uc = getUntil("</section>");
            break;

        case 2:
            if (moveToEarliestOf(">In Celebration of<", ">À la mémoire de<"))
            {
                if (consecutiveMovesTo(300, "</h", ">"))
                    uc = getUntil("</div>");
            }
            break;

        case 3:
            if (consecutiveMovesTo(25, "class=\"rte\"", ">"))
                uc = getUntil("</div>");
        }
        break;

    case SiteWyze:
        if (consecutiveMovesTo(2000, "post-content", "Date:"))
        {
            if(moveToEarliestOf("<p", "<div style"))
            {
                moveTo(">");
                uc = getUntil("announcement-sidebar");
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
            //if (consecutiveMovesTo(100, "class=\"obt_info\"", ">"))
            //    uc = getUntil("Print Obituary<");
            if (consecutiveMovesTo(5000, "<!-- #masthead -->", "class=\"elementor-heading-title elementor-size-default\"", "</span>", "<div class=\"elementor-widget-container\">"))
                uc = getUntil("</div>");
            break;

        case 1:
            if (moveTo("<!-- #TRX_REVIEWS_PLACEHOLDER# -->"))
                uc = getUntil("</section>");
            break;

        case 2:
            if(moveTo("itemprop=\"datePublished\""))
            {
                if (moveTo("<div class=\"elementor-widget-container\">"))
                    uc = getUntil("</div>");
            }
            break;

        case 3:
            if(moveTo(">OBITUARY<"))
            {
                if (moveTo("<div class=\"elementor-widget-container\">"))
                    uc = getUntil("</div>");
            }
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
        if (moveTo("details-copy"))
        {
            unsigned int startingPosition, endingPosition, distance;
            moveTo(">");
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
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(1000, "<!-- obit body tab -->", "id=\"obit_text\"", ">"))
               uc = getUntil("<!-- video stream link -->");
            break;

        case 1:
            if (consecutiveMovesTo(5000, "obit-container", ">Obituary For ", ">" ))
                uc = getUntil("<a href=>");
            break;

        case 2:
            if (consecutiveMovesTo(200, "<!-- start obit tab-->", "</h", ">"))
                uc = getUntil("<!--obit text-->");
            break;
        }
        break;
    }
        break;

    case Crew:
    {
        if (consecutiveMovesTo(2500, ">We Remember<", "<p>"))
            uc = getUntil("</div>");
    }
        break;

    case Jelly:
    {
        if (consecutiveMovesTo(100, "class=\"post-content", ">"))
            uc = getUntil("<a href=");
    }
        break;

    case Nirvana:
    {
        if (consecutiveMovesTo(5000, "class=\"displaytribute\"",  "class=\"box\"", "class=\"right", ">"))
            uc = getUntil("</section>");
    }
        break;

    case Bergetti:
    {
        if (consecutiveMovesTo(5, "itemprop=\"articleBody\"", ">"))
            uc = getUntil("</section>");
    }
        break;

    case PacificByte:
    {
        if (moveTo("Print Tribute</a>"))
            uc = getUntil("<!--/ct-tab-obituary-->");
    }
        break;

    case ROI:
    {
        if (consecutiveMovesTo(1100, "id=\"main\"", "entry-content", ">"))
            uc = getUntil("<!-- .entry-content -->");
    }
        break;

    case Vernon:
    {
        if (consecutiveMovesTo(2000, "id=\"main-content\"", "entry-title", "</div>"))
            uc = getUntil("<!-- .et_pb_post_content -->");
    }
        break;

    case Gustin:
    {
        if (moveTo("class=\"wpfh-obituary-content\""))
        {
            if (conditionalMoveTo("</h", "<p"))
                moveTo(">");
            else
                moveTo("</p>");

            uc = getUntil("<!-- .entry-content -->");
        }
    }
        break;

    case Ashlean:
    {
        if (consecutiveMovesTo(2500, "class=\"site-content\"", "class=\"ae-element-post-date\"", "</span>"))
            uc = getUntil("<div id=\"comments\"");
    }
        break;

    case Halifax:
    {
        if (consecutiveMovesTo(1500, "id=\"main-content\"", "entry-content", "<img", ">"))
            uc = getUntil("<strong>O");
    }
        break;

    case Specialty:
    {
        // Website designer is consistently inconsistent across all clients
        switch (style)
        {
        default:
        case 0:
            if (consecutiveMovesTo(500, "class=\"contentPanel\"", "class=\"obituary\"", ">"))
            {
                if (conditionalMoveTo("<img", "</div>", 0, 250))
                {
                    moveTo("</div>");
                    if (conditionalMoveTo("<img", "</div>", 0, 250))
                        moveTo("</div>");
                }

                if (conditionalMoveTo("<o:OfficeDocumentSettings>", "</div>", 0))
                {
                    moveTo("/* Style Definitions */");
                    moveTo("<![endif]-->");
                }
                //if (conditionalMoveTo("<p>&nbsp;</p>", "</div>", 0, 100))
                //    conditionalMoveTo("<p>&nbsp;</p>", "</div>", 0, 100);
                //uc = getUntil("</div>");
                uc = getUntil("<a id=\"guestbooks\"");
            }

            if (uc.getLength() == 0)
            {
                beg();
                if (consecutiveMovesTo(20, "class=\"contentPanel\"", ">"))
                    uc = getUntil("<a id=\"guestbooks\"");
            }

            if (uc.getLength() == 0)
            {
                beg();
                if (moveTo("<div class=\"obituary\">"))
                    uc = getUntilEarliestOf("<div class=\"epitaph\">", "<div layoutcode=\"4096\"");
            }

            if (globals->globalDr->getProviderKey() == 41)
            {
                uc.removeHTMLtags();
                uc.replaceHTMLentities();
                uc.extractLocationIfFirst(*globals);
            }
            break;

        case 1:
            if (moveTo("<div class=\"obituary\">"))
                uc = getUntil("<div class=\"epitaph\">");
            break;
        }
    }
        break;

    case ReneBabin:
    {
        if (consecutiveMovesTo(400, "class=\"post_content\"", "</span>"))
            uc = getUntil("</section>");
    }
        break;

    case Smiths:
    {
        if (consecutiveMovesTo(250, "<body>", "</span>"))
        {
            uc = getUntil("Copyright 20");
            uc.removeLineFormatting();
        }
    }
        break;

    case Seabreeze:
    {
        // Part of group of obits
        target = QString("thankyou") + globals->globalDr->getID().getString();
        if (moveTo(target))
        {
            moveBackwardTo("data-role=\"collapsible\"");
            consecutiveMovesTo(25, "<span", ">");
            consecutiveMovesTo(25, "<span", ">");
            if (conditionalMoveTo("<img", "<br", 0))
                moveTo(">");
            uc = getUntil("Online Condolences");
        }
    }
        break;

    case RedSands:
    {
        if (consecutiveMovesTo(100, "class=\"entry-content", "text"))
        {
            moveTo(">");
            uc = getUntil("Send a condolence<");
        }
    }
        break;

    case AdGraphics:
    {
        // Part of group of obits
        target = QString("images/") + globals->globalDr->getID().getString() + QString(".");
        if (moveTo(target))
        {
            moveBackwardTo("<tr>", 7500);
            consecutiveMovesTo(150, "<td align", ">");
            moveToEarliestOf("</strong>", "</p>");
            uc = getUntil("</td>");
        }
    }
        break;

    case Websites:
    {
        if (moveTo("class=\"entry-content\""))
            uc = getUntil("<!-- .entry-content -->");
    }
        break;

    case MPG:
    {
        if (consecutiveMovesTo(1000, ">In memory of<", "<div", ">"))
            uc = getUntil("</div>");
    }
        break;

    case MediaHosts:
    {
        if (moveTo("<p class=\"obit-spacing\">"))
            uc = getUntil("<button");
    }
        break;

    case DragonFly:
    {
        if (consecutiveMovesTo(150, ">Death Notice Of", "</p>"))
            uc = getUntil("</div>");
    }
        break;

    case MyFavourite:
    {
        if (moveToEarliestOf("class=\"obit-intro\"", "class=\"obit-text\""))
            uc = getUntil("Details<");
    }
        break;

    case Coop:
    {
        if (consecutiveMovesTo(200, "class=\"obituary-years\"", "</div>"))
            uc = getUntil("<div class=\"obituary-form");
    }
        break;

    case EverTech:
    {
        if (consecutiveMovesTo(100, "<meta itemprop=\"description\"", "content="))
        {
            uc = getUntil(" data-rdm/>");
            uc.removeBookEnds(QUOTES);
        }
    }
        break;

    case MacLean:
    {
        // Part of group of obits
        target = QString("modal_") + globals->globalDr->getID().getString();
        if (moveTo(target))
        {
            consecutiveMovesTo(100, ">MAKE DONATION<", "</div>");
            uc = getUntil("<!-- Reveal Modals end -->");
        }
    }
        break;

    case MCG:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(1500, "id=\"cwp_funerals_details\"", "class=\"comments\">"))
                uc = getUntil("</div>");
            break;

        case 1:
            if (moveTo("class=\"overview_feature_indent\">"))
                uc = getUntil("</div>");
            break;
        }
    }
        break;

    case UNI:
    {
        if (moveTo("<>"))
        {
            uc = readNextBetween(BRACKETS);
        }
    }
        break;

    case WebSolutions:
    {
        if (consecutiveMovesTo(1500, "class=\"card-body\"", "class=\"card-text", ">"))
            uc = getUntil("<form id=\"condolenceForm\"");
    }
        break;

    case Citrus:
    {
        if (consecutiveMovesTo(100000, "jet-listing-dynamic-field__content", "jet-listing-dynamic-field__content", "jet-listing-dynamic-field__content"))
        {
            moveTo(">");
            uc = getUntil("</div></div></div>");
        }
    }
        break;

    case TNours:
    {
        if (consecutiveMovesTo(50, "class=\"hr hr_top\"", "class=\"text-justify\">"))
            uc = getUntil("</div>");
    }
        break;

    case SandFire:
    {
        if (moveTo("<div class=\"post-content\">"))
            uc = getUntil("<section class=\"post-comments\">");
    }
        break;

    case Carve:
    {
        if (consecutiveMovesTo(50, ">Obituary<", ">"))
            uc = getUntil("<a data-title=\"view-condolences\"");
    }
        break;

    case BrandAgent:
    {
        if (consecutiveMovesTo(50, "message-view-body-content", ">"))
        {
            uc = getUntil("</div>");
            runStdProcessing(uc, false);

            if (uc.getLength() < 10)
                uc = getUntil("</div>");
        }
    }
        break;

    case EPmedia:
    {
        if (consecutiveMovesTo(150, "class=\"sc_tabs_content even\"", "class=\"wpb_text_column wpb_content_element", ">"))
            uc = getUntil("</div>");
    }
        break;

    case Linkhouse:
    {
        if (consecutiveMovesTo(10, "<header>", "<h"))
        {
            if (moveTo("<span", 100))
                moveTo("</span>");
            uc = getUntilEarliestOf("Condolences<", "</section>");
        }
    }
        break;

    case LinkWeb:
    {
        if (moveTo("<article class=\"uk-article\">"))
            uc = getUntil("<div id =");
    }
        break;

    case JoshPascoe:
    {
        if (moveTo("class=\"detailsText\">"))
            uc = getUntil("<script");
    }
        break;

    case ChadSimpson:
    {
        if (consecutiveMovesTo(200, "class=\"flex-1", "class=\"text-center", ">"))
        {
            if (conditionalMoveTo("<p class", "<div class", 0))
            {
                moveTo(">");
                uc = getUntil("</p>");
            }
        }
    }
        break;

    case MarketingImages:
    {
        if (consecutiveMovesTo(50, "class=\"details\"", "<h"))
        {
            moveTo("</strong>-->");
            uc = getUntil("<p>Online");
        }
    }
        break;

    case ESite:
    {
        switch(globals->globalDr->getProviderKey())
        {
        case 1:
            if (moveTo("<div class=\"obituaries\">"))
                uc = getUntil("</div>");
            break;

        case 2:
            if (moveTo("<div class=\"obituaries\">"))
            {
                moveTo("<p>");
                uc = getUntil("</div>");
            }
            break;
        }
    }
        break;

    case SquareSpace:
    {
        if (consecutiveMovesTo(200, "</div></div><div class=\"sqs-block html-block sqs-block-html\"", ">"))
            uc = getUntil("</div>");
    }
        break;

    case Eggs:
    {
        if (moveTo("<div class=\"entry\">"))
            uc = getUntil("<div id=");
    }
        break;

    case MFH:
    {
        if (consecutiveMovesTo(500, "class=\"obituary-text", ">In Loving Memory<", "<p style="))
        {
            moveTo(">");
            uc = getUntilEarliestOf("<p class=\"postmetadata\">", "<div class=\"obituary-button-row\">");
        }
    }
        break;

    case ONeil:
    {
        if (consecutiveMovesTo(10, "id=\"obitText\"", ">"))
            uc = getUntil("</div>");
    }
        break;

    case Back2Front:
    {
        if (consecutiveMovesTo(250, "id=\"app.tbl.funeral_services.message-span\"", ">"))
            uc = getUntil("</span>");
    }
        break;

    case InView:
    {
        if (consecutiveMovesTo(1000, ">Tranquility<", "class=\"entry-content\"", ">"))
            uc = getUntil("</div><footer");
        else
        {
            beg();
            if (consecutiveMovesTo(100, ">Obituaries<", "class=\"trail-end\"", ">"))
                uc = getUntil("</div><footer");
        }
    }
        break;

    case CreativeOne:
    {
        if (moveTo("<>"))
        {
            uc = readNextBetween(BRACKETS);
        }
    }
        break;

    case RKD:
    {
        moveTo("Send a Private Message Of Condolence");
        if (consecutiveMovesTo(100, "Date of Death:", "</div>"))
            uc = getUntil("<section class=");
        else
        {
            beg();
            moveTo(">Download or Print<");
            if (moveTo("<div class=\"elementor-widget-container\">"))
                uc = getUntil("</section>");
        }
    }
        break;

    case SDP:
    {
        if (consecutiveMovesTo(200, "<div class=\"fltlft content50\">", "</h3>"))
            uc = getUntil("<div class=\"PlusDetails temoignages\">");
    }
        break;

    case Globalia:
    {
        if (consecutiveMovesTo(50, "class=\"single-content\"", ">"))
            uc = getUntil("<div class=");
    }
        break;

    case Vortex:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(750, "class=\"colPrincipal_col2 colImagePasseport2\"", "class=\"both\"", "</div>"))
                uc = getUntil("</article>");
            else
            {
                beg();
                if (consecutiveMovesTo(1000, "class=\"colPrincipal_col2 colImagePasseport2\"", "class=\"mobileOnly\"", ">"))
                    uc = getUntil("</article>");
                else
                {
                    beg();
                    if (consecutiveMovesTo(200, "class=\"tlt-ficheDeces\"", "<h2", ">"))
                        uc = getUntil("</article>");
                    else
                    {
                        beg();
                        if (consecutiveMovesTo(50, "class=\"ctn-texte-deces texte\"", ">"))
                            uc = getUntil("</div>");
                    }
                }
            }
            break;

        case 1:
            if (moveTo("<div class=\"incTinyMce\">"))
                uc = getUntil("</div>");
            break;

        case 2:
            if (moveTo("<div class=\"obituary-content-left\">"))
                uc = getUntil("<div class=\"obituary-comments\">");
            break;
        }
    }
        break;

    case Elegant:
    {
        if (consecutiveMovesTo(50, "<section class=\"post_content\"", ">"))
            uc = getUntil("</section>");
    }
        break;

    case YellowPages:
    {
        switch(style)
        {
        case 0:
            target = QString("id=\"title") + globals->globalDr->getID().getString();
            if (consecutiveMovesTo(1000, target, "<div class=\"justify textNotice\">"))
                uc = getUntil("</div>");
            break;

        case 1:
            if (consecutiveMovesTo(100, "td class=\"deces\"", ">"))
                uc = getUntil("</td>");
            break;
        }

    }
        break;

    case Shooga:
    {
        if (consecutiveMovesTo(500, "class=\"blog-title\"", "</div>", "<p", ">"))
        {
            if (conditionalMoveTo("</em>", "</p>", 0))
                moveTo("</p>");
            uc = getUntil("<ul class=\"blog-footer clearfix\">");
        }
    }
        break;

    case NBL:
    {
        if (moveTo("<div class=\"texte-deces\">"))
            uc = getUntil("</div>");
    }
        break;

    case WPBakery:
    {
        if (moveTo("<div id=\"obtext\">"))
            uc = getUntil("</div>");
    }
        break;

    case Imago:
    {
        if (consecutiveMovesTo(2000, ">Partager/imprimer<", "title=\"Print\"", "</div></div>"))
            uc = getUntil("<div class=\"addtoany_share_save_container addtoany_content");
    }
        break;

    case TroisJoueur:
    {
        if (moveTo("<>"))
        {
            uc = readNextBetween(BRACKETS);
        }
    }
        break;

    case Ubeo:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(250, "class=\"margB_3 for_638\"", "</h3>"))
                uc = getUntil("<!--Messages de condoléances-->");
            break;

        case 1:
            if (moveTo("class=\"avis_main_text\""))
                uc = getUntil("</div>");
            break;
        }
    }
        break;

    case Acolyte:
    {
        if (moveTo("<>"))
        {
            uc = readNextBetween(BRACKETS);
        }
    }
        break;

    case Morin:
    {
        if (consecutiveMovesTo(50, "class=\"post_content\"", ">"))
            uc = getUntilEarliestOf("</section>", "<p>Direction funéraire");
    }
        break;

    case Taiga:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(50, "class=\"obituary-details-bottom\"", ">"))
                uc = getUntil("<div class=\"share_buttons\">");
            break;

        case 1:
            if (consecutiveMovesTo(100, "<!-- AddThis Button END -->", "class=\"col-lg-8\"", ">"))
            {
                int position = getPosition();
                if (!moveTo("<p>", 15))
                {
                    beg();
                    forward(position);
                }
                uc = getUntil("<p>");
            }
            break;
        }

    }
        break;

    case Zonart:
    {
        if (moveTo("<>"))
        {
            uc = readNextBetween(BRACKETS);
        }
    }
        break;

    case PubliWeb:
    {
        switch(style)
        {
        case 0:
            if (moveTo("<div class=\"inner\">"))
                uc = getUntil("</div>");
            break;

        case 1:
            if (consecutiveMovesTo(4000, "<!--Avis (1 par page) -->", "<div class=\"blanc18pt\"><b>", "</div>", "</div>"))
                uc = getUntil("</div>");
            break;
        }
    }
        break;

    case DirectImpact:
    {
        if (consecutiveMovesTo(50, "obituary-content-text", ">"))
        {
            uc = getUntil("</div>");
            unstructuredContent tempUC(uc);
            tempUC.removeHTMLtags();
            tempUC.cleanUpEnds();
            if (tempUC.getLength() < 10)
            {
                if (consecutiveMovesTo(50, "obituary-content-text", ">"))
                    uc = getUntil("</div>");
            }
        }
    }
        break;

    case SoleWeb:
    {
        if (consecutiveMovesTo(100, "class=\"vsel-image-info\"", "</p>"))
            uc = getUntil("<a href=\"http://maisonmallet.com\">");
    }
        break;

    case Voyou:
    {
        if (style < 2)
        {
            if (consecutiveMovesTo(300, "class=\"avis-deces-title\"", "</div>"))
                uc = getUntil("<!--<div class=\"residence\">");
        }
        else
        {
            if (consecutiveMovesTo(50, "class=\"vy_deces_description", ">"))
                uc = getUntil("</div>");
        }
    }
        break;

    case Scrypta:
    {
        if (globals->globalDr->getProviderKey() == 1)
        {
            if (moveTo("class=\"TGO06__content--description\""))
            {
                conditionalMoveTo(">Informations<", "</div>", 1);
                moveTo(">");
                uc = getUntil("</main>");
            }
        }
        else
        {
            if (moveTo("class=\"obi_right\">"))
                uc = getUntil("<span class=\"obi_accueil\">");
        }
    }
        break;

    case Jaimonsite:
    {
        if (consecutiveMovesTo(50, "og:description", "content="))
            uc = readQuotedMetaContent();
    }
        break;

    case Saguenay:
    {
        if (consecutiveMovesTo(25, "description", "content="))
            uc = readQuotedMetaContent();
    }
        break;

    case Lithium:
    {
        if (consecutiveMovesTo(250, "class=\"details\"", "<p>"))
            uc = getUntil("</div>");
    }
        break;

    case Cameleon:
    {
        if (consecutiveMovesTo(50, "class=\"pagePart", "data-id=\"avis\">"))
            uc = getUntil("</div>");
    }
        break;

    case LogiAction:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(1000, "class=\"content\"", "</p>", "</p>"))
                uc = getUntil("<div class=");
            break;

        case 1:
            if (consecutiveMovesTo(1000, "class=\"client\"", "<p", ">"))
                uc = getUntil("<div class=");
            break;
        }

    }
        break;

    case BLsolutions:
    {
        if (consecutiveMovesTo(100, "class=\"post_content\"", "itemprop=\"articleBody\"", ">"))
            uc = getUntil("</section>");
    }
        break;

    case ToraPro:
    {
        if (moveTo("class=\"obit\">"))
            uc = getUntil("<!-- .entry-content -->");
    }
        break;

    case Axanti:
    {
        if (consecutiveMovesTo(100, "class=\"obituary-content entry-content", ">"))
            uc = getUntil("<div class=");
    }
        break;

    case ADN:
    {
        if (consecutiveMovesTo(100, "class=\"obituary-viewer__content", ">"))
            uc = getUntil("<!-- Fin des informations -->");
    }
        break;

    case B367:
    {
        if (moveTo("class=\"h5-detail-deces\">"))
            uc = getUntil("</div>");
    }
        break;

    case Tomahawk:
    {
        if (consecutiveMovesTo(2000, ">Partager cette avis<", "</div>"))
        {
            moveToEarliestOf("<p", "<h");
            moveTo(">");
            uc = getUntil("<div class");
        }
    }
        break;

    case LeSaint:
    {
        if (moveTo("<>"))
        {
            uc = readNextBetween(BRACKETS);
        }
    }
        break;

    case Caza:
    {
        if (moveTo("class=\"content typography\">"))
            uc = getUntil("<div id=");
    }
        break;

    case Tegara:
    {
        if (consecutiveMovesTo(100, "class=\"obituary\"", ">"))
            uc = getUntil("<div class=\"share");
    }
        break;

    case NMedia:
    {
        if (consecutiveMovesTo(1000, "InventoryProductBroker_CF_da94c4f1-d139-4d6b-4b92-f2ccf25eb8e2", ">"))
            uc = getUntil("</div>");
    }
        break;

    case Webs:
    {
        if (consecutiveMovesTo(100, "<div class=\"webs-text", ">"))
            uc = getUntil("</div>");
    }
        break;

    case Descary:
    {
        if (consecutiveMovesTo(200, "class=\"post_content\"", ">"))
        {
            moveTo("</p>", 25);
            uc = getUntil("RÉSIDENCE FUNÉRAIRE");
        }
    }
        break;

    case Tonik:
    {
        if (consecutiveMovesTo(1000, "class=\"content detail\"", "class=\"date text-center\"", "</div>"))
        {
            consecutiveMovesTo(300, "<div", ">");
            uc = getUntil("</div>");
        }
    }
        break;

    case Kaleidos:
    {
        if (consecutiveMovesTo(1000, "class=\"description-fiche tab-content\"", "class=\"border\"", "</div>"))
            uc = getUntil("</div>");
    }
        break;

    case Gemini:
    {
        if (consecutiveMovesTo(200, "class=\"omc-post-heading-standard\"", "<p>"))
            uc = getUntil("<div class=\"sharedaddy");
    }
        break;

    case Alias:
    {
        if (consecutiveMovesTo(500, "class=\"printonly\"><em>", "<div>"))
            uc = getUntil("</div>");
    }
        break;

    case Cible:
    {
        if (consecutiveMovesTo(100, "class=\"detail_date\"", "</div>"))
            uc = getUntil("<div class=\"detail_retour_avis\">");
    }
        break;

    case Web2u:
    {
        if (moveTo("<>"))
        {
            uc = readNextBetween(BRACKETS);
        }
    }
        break;

    case District4Web:
    {
        switch(style)
        {
        case 0:
            //if (consecutiveMovesTo(200, "class=\"fusion-text fusion-text-1\"", "<p>"))
            if (consecutiveMovesTo(5000, ">Envoyer un témoignage de sympathie<", "<p>"))
                uc = getUntil("<div class=\"fusion-clearfix\"");
            break;

        case 1:
            if (moveTo("class=\"obituary-content"))
            {
                moveToEarliestOf("<p>", "</div>");
                uc = getUntil("</div>");
            }
            break;
        }
    }
        break;

    case Cake:
    {
        if (consecutiveMovesTo(100, "class=\"rufina\"", ">"))
            uc = getUntil("</div>");
    }
        break;

    case J27:
    {
        if (consecutiveMovesTo(100, "itemprop=\"articleBody\"", "<p>"))
            uc = getUntil("</div>");
    }
        break;

    case NetRevolution:
    {
        if (consecutiveMovesTo(200, "elementor-widget-theme-post-content", ">"))
            uc = getUntil("</div>");
    }
        break;

    case ImageXpert:
    {
        if (moveTo("class=\"group-right\">"))
            uc = getUntil("<!-- Feed icons ");
    }
        break;

    case Reactif:
    {
        if (consecutiveMovesTo(50, "class=\"deces-text\"", ">"))
            uc = getUntil("</article>");
    }
        break;

    case Boite:
    {
        if (consecutiveMovesTo(50, "class=\"wpfh-obituary-content\"", ">"))
            uc = getUntil("</div>");
    }
        break;

    case Orage:
    {
        if (consecutiveMovesTo(400, "class=\"col-txt fcomplete\"", "<p>"))
            uc = getUntil("<ul class=");
    }
        break;

    case Kerozen:
    {
        switch(style)
        {
        case 0:
            if (moveTo("class=\"col-12 text--description my-5\">"))
                uc = getUntil("</div>");
            break;

        case 1:
            if (consecutiveMovesTo(20, "id=\"necron\"", ">"))
                uc = getUntil("<br>");
            break;
        }

    }
        break;

    case InoVision:
    {
        if (consecutiveMovesTo(2000, "id=\"article-content\"", "class=\"text-container\">"))
            uc = getUntil("</section>");
    }
        break;

    case FRM:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(100, "class=\"entrytext\"", ">"))
            {
                uc = getUntil("</div>");
                uc.removeHTMLtags(false);
                uc.simplify();
                if (uc.getLength() < 75)
                    uc = getUntil("</div>");
            }
            break;

        case 1:
            if (consecutiveMovesTo(50, "class=\"entry-title\"", "itemprop=\"name\""))
            {
                int position = getPosition();
                switch(globals->globalDr->getProviderKey())
                {
                case 13:
                    target = QString(">Click ");
                    break;

                case 14:
                    target = QString("play.webvideocore.net");
                    break;

                default:
                    target = QString("blahblahblah");
                    break;
                }

                if (moveTo(target, 2000))
                    moveTo(">");
                else
                {
                    beg();
                    forward(position);
                    conditionalMoveTo("class=\"has-text-align-center\"", "<p>", 0);
                    moveToEarliestOf("</div>", "</p>");
                }
                uc = getUntil("<a href=");
            }
            break;

        case 2:
            if (moveTo("class=\"entry-title\""))
            {
                moveToEarliestOf("</div>", "<p>");
                uc = getUntil("<a href=");
            }
            break;

        case 3:
            if (moveTo("class=\"entry-content\""))
            {
                moveToEarliestOf("</div>", "<p>");
                uc = getUntil("<a href=");
            }
            break;

        case 4:
            if (consecutiveMovesTo(1000, "class=\"obituary-content", "<p>"))
                uc = getUntil("</div>");
            break;
        }
    }
        break;

    case PassageCoop:
    {
        if (consecutiveMovesTo(25, "Print:", ">"))
            uc = getUntil("<!-- .entry-content -->");
        else
        {
            beg();
            if (consecutiveMovesTo(25, "Imprimer", ">"))
                uc = getUntil("<!-- .entry-content -->");
        }

        uc.replaceHTMLentities();
        QString tempString = uc.getString();
        tempString.replace("</strong> – ", " -</strong>");
        QRegularExpression target("<strong>\\S+\\s?NB\\.? -</strong>");
        tempString.replace(target, "");
        uc.setItsString(tempString);
    }
        break;

    case JBCote:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(1000, "class=\"obituaries-announcement-detail\"", "<h"))
            {
                moveToEarliestOf("</h", "<p");
                moveTo(">");
                uc = getUntil("<div id=\"buttons\"");
                uc.replace("<br />", " ");
            }
            break;

        case 1:
            if (consecutiveMovesTo(20, "class=\"descr-avis\"", ">"))
                uc = getUntil("</table>");
            break;
        }

    }
        break;

    case BlackCreek:
    {
        if (moveTo("class=\"et_pb_text_inner\">"))
            uc = getUntil("<!-- .et_pb_text -->");
    }
        break;

    case CityMax:
    {
        target = OQString(globals->globalDr->getID()).convertFromID();
        if (moveTo(target))
        {
            consecutiveMovesTo(30, "<p", ">");
            uc = getUntil("<hr />");
        }
    }
        break;

    case SYGIF:
    {
        if (consecutiveMovesTo(1500, "h2 class=\"title\"", "class=\"paragraphe1\"", ">"))
            uc = getUntil("<hr />");
    }
        break;

    case PortNeuf:
    {
        moveTo("class=\"the_content_wrapper\"");
        int position = getPosition();
        if (moveTo("<p><strong", 30))
        {
            moveTo("</p");
            position = getPosition();

            if (moveTo("<p><strong", 130))
                moveTo("</p");
            else
            {
                beg();
                forward(position);
            }
        }
        else
        {
            beg();
            forward(position);
        }

        moveTo(">");
        uc = getUntilEarliestOf("<p>Pour renseignements :</p>", "<div class=\"section section-post-footer\">");
    }
        break;

    case Canadian:
    {
        if(consecutiveMovesTo(100, "itemprop=\"articleBody\"", ">"))
            uc = getUntil("<script>");
    }
        break;

    case BallaMedia:
    {
        if (consecutiveMovesTo(1000, "<h4>In Celebration of</h4>", "<div", ">"))
            uc = getUntil("</div>");
        else
        {
            beg();
            if (consecutiveMovesTo(20, "og:description", "content="))
                uc = readQuotedMetaContent();
        }
    }
        break;

    case Jac:
    {
        if (consecutiveMovesTo(250, "src=\"images/ob.png\"", "<TD", ">"))
            uc = getUntil("</TD>");
    }
        break;

    case Ministry:
    {
        QString target;
        int start, end;
        target = OQString(globals->globalDr->getID()).convertFromID();
        if (moveTo(target))
        {
            moveBackwardTo("<br />");
            start = getPosition();
            consecutiveMovesTo(20, "<br />", "<br />");
            end = getPosition();
            backward(end - start);
            uc = getNext(end - start);
        }

    }
        break;

    case Multinet:
    {
        simplify();
        if (moveTo("<!-- Slick Carousel-->"))
        {
            moveBackwardTo("</ul>", 25000);
            uc = getUntil("<!-- Slick Carousel-->");
        }
    }
        break;

    case PropulC:
    {
        if (consecutiveMovesTo(50, "class='_content-avis'", ">"))
            uc = getUntil("</section>");
    }
        break;

    case Nexion:
    {
        if (moveTo("id=\"avis-deces-texte\""))
            uc = getUntil("</div>");
    }
        break;

    case LCProduction:
    {
        if (consecutiveMovesTo(100, "class=\"wpfh-obituary-content\"", ">"))
            uc = getUntil("</div>");
    }
        break;

    case Absolu:
    {
        if (consecutiveMovesTo(50, "class=\"introduction\"", ">"))
            uc = getUntil("</div>");
    }
        break;

    case SuiteB:
    {
        if (consecutiveMovesTo(250, "lass=\"content\"", "<p>"))
            uc = getUntil("</div>");
    }
        break;

    case Map:
    {
        if (consecutiveMovesTo(25, "class='apercu'", ">"))
            uc = getUntil("<div class='events'>");
    }
        break;

    case iClic:
    {
        if (moveTo("<div id=\"tinfo\">"))
            uc = getUntil("<div id=\"tsympathie\">");
    }
        break;

    case Bouille:
    {
    }
        break;

    case Pub30:
    {
    }
        break;

    case Theories14:
    {
        if (consecutiveMovesTo(100, "class=\"date\"", "</div>"))
            uc = getUntil("<div class=\"condolences-form\">");
    }
        break;

    case Techlogical:
    {
        if (consecutiveMovesTo(100, "<h2>Obituary of ", ">"))
            uc = getUntil("<h2>Service Information</h2>");
    }
        break;

    case GyOrgy:
    {
        if (moveTo("style=\"font-size:24px\""))
        {
            if (moveTo("<span class=\"wixGuard\">"))
                uc = getUntil("<span class=\"wixGuard\">");
        }
        else
        {
            beg();
            if (moveTo("</h2>"))
            {
                if (moveTo("<span class=\"wixGuard\">"))
                    uc = getUntil("<span class=\"wixGuard\">");
            }
        }

        if (uc.getLength() == 0)
        {
            beg();
            if (moveTo("style=\"font-size:20px;\""))
            {
                moveTo(">");
                uc = getUntil("<footer");
            }
        }
    }
        break;

    case GemWebb:
    {
        if (consecutiveMovesTo(3500, "id=\"fl-main-content\"", "<p>"))
            uc = getUntil("</div>");
    }
        break;

    case RedChair:
    {
        if (consecutiveMovesTo(1500, "class=\"site-main\"", "</section>"))
            uc = getUntilEarliestOf("</section>", "Visitation Information<");
    }
        break;

    case ExtremeSurf:
    {
        if (consecutiveMovesTo(250, "id=\"tab11\"", "<p>"))
            uc = getUntil("</div>");
    }
        break;

    case Cahoots:
    {
        if (consecutiveMovesTo(25, "class=\"blogContent d-nones\"", ">"))
        {
            if (conditionalMoveTo("<img src=", "<!-- /snippets/social-sharing.liquid -->", 0))
                moveTo(">");
            uc = getUntil("<!-- /snippets/social-sharing.liquid -->");
        }
    }
        break;

    case Tride:
    {
        if  (consecutiveMovesTo(50, "class=\"obituarie-details\"", ">"))
            uc = getUntil("</div>");
    }
        break;

    case Jensii:
    {
        if (consecutiveMovesTo(100, ">Obituary<", ">"))
            uc = getUntil("Condolences</h3>");
    }
        break;

    case InterWeb:
    {
        if (consecutiveMovesTo(50, "class=\"wpfh-obituary-content\"", ">"))
            uc = getUntil("</div>");
    }
        break;

    case Brown:
    {
        if (consecutiveMovesTo(100, "class=\"d-avis-detail-title\"", "<div>"))
            uc = getUntil("</div>");
    }
        break;

    case Tukio:
    {
        if (moveTo("id=\"obit_section\""))
        {
            moveTo(">");
            uc = getUntil("</div>");
        }
        else
        {
            beg();
            if (consecutiveMovesTo(50, "obituary-text-main", ">"))
                uc = getUntil("</div>");
        }
    }
        break;

    case Etincelle:
    {
        if (consecutiveMovesTo(300, "class=\"nom\"", "class=\"date\"", "</div>"))
            uc = getUntil("<form");
    }
        break;

    /*****************/
    /*   One Offs    */
    /*****************/

    case MikePurdy:
        if (consecutiveMovesTo(100, "class=\"storycontent\"", ">"))
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
    {
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

        QRegularExpression target;
        QString tempString = uc.getString();
        target.setPatternOptions(QRegularExpression::CaseInsensitiveOption | QRegularExpression::InvertedGreedinessOption | QRegularExpression::UseUnicodePropertiesOption);
        target.setPattern(">(January|February|March|April|May|June|July|August|September|October|November|December)( \\d\\d?, \\d{4}) - (.*)(</span)");
        tempString.replace(target,">\\1\\2\\4");
        uc.setItsString(tempString);

        break;
    }

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
        if (consecutiveMovesTo(100, "class=\"editor\"", ">"))
            uc = getUntil("</section>");
        uc.replace(QString("\\\\\\"), QString(""));
        break;

    case Pierson:
    case SRS:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(100, "itemprop=\"description\"", ">"))
                uc = getUntil("</div>");
            break;

        case 1:
        case 2:
            if (moveTo("fullDescription:"))
            {
                uc = getUntil("showDescription:");
                uc.unQuoteHTML();
                uc.dropRight(1);
                uc.cleanUpEnds();
                uc.removeEnding(PUNCTUATION);
                uc.removeBookEnds(QUOTES);
            }
            break;
        }
        break;

    case Trinity:
        if (moveTo("class=\"obituary-content\">"))
            uc = getUntil("<div class=\"sidebar-widget-area\">");
        break;

    case CelebrateLife:
        if (consecutiveMovesTo(2000, "class=\"main-wrap\"", "Obituary of", "class=\"paragraph\"", ">"))
            uc = getUntil("</table>");
        break;

    case Funks:
        if (consecutiveMovesTo(2000, "content-main", "class=\"entry\">"))
            uc = getUntil("<!-- You can start editing here. -->");
        else
        {
            beg();
            if (consecutiveMovesTo(2000, "<body>", ">Archives<", "<div", ">"))
                uc = getUntil("Print this:");
        }
        break;

    case WowFactor:
        if (consecutiveMovesTo(3000, "id=\"main-content\"", "</h3>", "et_pb_text_inner", ">"))
            uc = getUntil("</div>");
        break;

    case Dalmeny:
        if (consecutiveMovesTo(3000, "class=\"content\"", "<p>"))
            uc = getUntil("<div class");
        break;

    case Hansons:
        if (consecutiveMovesTo(100, "obituaries_text", ">"))
            uc = getUntil("</div>");
        break;

    case Martens:
        if (moveTo("class=\"wpfh-obituary-content\">"))
            uc = getUntil("<!-- .et_builder_inner_content -->");
        break;

    case Shine:
        if (consecutiveMovesTo(25, "class=\"page\"", "title=\"Page 2\""))
            uc = getUntil("</div>");
        break;

    case Simply:
        switch(style)
        {
        case 0:
            // Nothing available
            uc.clear();
            break;

        case 1:
            if (moveTo("<!--/div .post-meta-->"))
                uc = getUntil("<!--/div .post-excerpt-->");
            break;
        }
        break;

    case McCall:
        if (consecutiveMovesTo(3500, "class=\"head-title\"", "<p", ">"))
            uc = getUntil("<a href=");
        break;

    case Care:
        if (consecutiveMovesTo(100, "class=\"wpfh-obituary-content\"", ">"))
            uc = getUntil("</div>");
        break;

    case Ancient:
        if (consecutiveMovesTo(100, "class=\"blog-content\"", ">"))
            uc = getUntil("<div class=\"blog-social");
        break;

    case Amherst:
        if (consecutiveMovesTo(150, ">Obituary for ", "<br>"))
            uc = getUntil("<h2>Condolence");
        break;

    case Heritage:
        if (consecutiveMovesTo(50, "class=\"entry-title eltdf-post-title\"", ">"))
            uc = getUntil("</div");
        break;

    case Koru:
        if (consecutiveMovesTo(100, "class=\"obituaries-block\"", ">"))
            uc = getUntil("<div class=\"at-below-post addthis_tool\"");
        break;

    case Kowalchuk:
        if (moveTo("class=\"entry-content\""))
            uc = getUntil("<!-- .entry-content -->");
        break;

    case Loehmer:
        if (consecutiveMovesTo(100, "class=\"article-tag Obituary\"", "</h", ">"))
        {
            if (moveTo("class=\"dod\"", 25))
                moveTo("</p>");
            uc = getUntil("<h3>Comments</h3>");
        }
        break;

    case Doyle:
        if (consecutiveMovesTo(100, "<div itemprop=\"articleBody\">", ""))
            uc = getUntil("<scrip");
        break;

    case Ethical:
        //if (consecutiveMovesTo(500, "<span style=\"font-family:playfair display,serif", "<span style=\"font-family:playfair display,serif", ">"))
        if (consecutiveMovesTo(500, "<span style=\"font-family:playfair display,serif", "</span>"))
            uc = getUntil("ETHICAL DEATH CARE");
        else
        {
            beg();
            QString target("style=\"text-shadow:#ffffff -1px -1px 0px, #ffffff -1px 1px 0px, #ffffff 1px 1px 0px, #ffffff 1px -1px 0px\"");
            if (consecutiveMovesTo(500, target, target, ">"))
                uc = getUntil("ETHICAL DEATH CARE");
        }

        if (uc.getLength() == 0)
        {
            beg();
            if (moveTo("<span style=\"font-family:playfair display,serif\""))
            {
                moveToEarliestOf("</p", "<span style=\"font-family:playfair display,serif\"");
                moveTo(">");
                uc = getUntil("ETHICAL DEATH CARE");
            }
        }
        if (uc.getLength() == 0)
        {
            beg();
            if (moveTo("<span style=\"font-family:playfair display,serif\""))
            {
                moveToEarliestOf("</p", "<span style=\"font-family:playfair display,serif\"");
                moveTo(">");
                uc = getUntil("ETHICAL DEATH CARE");
            }
        }
        break;

    case Direct:
        if (moveTo("class=\"et_pb_text_inner\""))
            uc = getUntil("</div>");
        break;

    case SMC:
        if (moveTo("class=\"bio w-richtext\">"))
            uc = getUntil("Condolences:<");
        break;

    case Belvedere:
        if (consecutiveMovesTo(100, "itemprop=\"description\"", "content="))
            uc = readQuotedMetaContent();
        break;

    case Davidson:
        if (moveTo("class=\"comments\">"))
            uc = getUntil("</div>");
        break;

    case Carnell:
        if (moveTo("<h2>Obituary</h2>"))
            uc = getUntil("</div>");
        break;

    case JOsmond:
        if (consecutiveMovesTo(30, "class=\"post-content\"", ">"))
            uc = getUntil("<div id=\"comments\"");
        break;

    case TivaHost:
        if (moveTo("<div class=\"nxs-postrows\">"))
            uc = getUntil("<div class=\"nxs-clear\">");
        break;

    case KMF:
        if (moveTo("class=\"body entry-content\">"))
            uc = getUntil("<!--POST FOOTER-->");
        break;

    case AMG:
        if (consecutiveMovesTo(1000, "class=\"title\"", "class=\"col-sm-5\""))
            uc = getUntil("</div>");
        break;

    case Orillia:
        if (consecutiveMovesTo(200, ">Subscription", "<p", ">"))
            uc = getUntil("<footer");
        break;

    case OSM:
        if (consecutiveMovesTo(400, "class=\"news_main_story_title\"", "<p", ">"))
        {
            uc = getUntil("<div class=\"news_clear_floats\">");
            uc.replace("</strong>", " </strong>");
            uc.replace("</span>", " </span>");
            uc.replace("</b>", " </b>");
            uc.replace("<br />", " ");
            uc.replace("</p>", " ");
        }
        break;

    case Alcock:
        if (moveTo("</span></span>"))
        {
            forward(5);
            moveTo("</span></span>");
            uc = getUntil("</div>");
        }
        break;

    case Abstract:
        if (consecutiveMovesTo(150, "class=\"right\"", "<h", "</h", ">"))
            uc = getUntil("</div>");
        break;

    case Beechwood:
        if (consecutiveMovesTo(100, "og:description", "content="))
            uc = readQuotedMetaContent();
        break;

    case Benjamins:
        if (consecutiveMovesTo(200, ">Notice", "ContentPlaceHolder1_lblNotice", ">"))
            uc = getUntil("</div>");
        else
        {
            beg();
            if (consecutiveMovesTo(100, "Notice:", ">"))
                uc = getUntil("</div>");
        }
        break;

    case Berthiaume:
        if (consecutiveMovesTo(50, ">AVIS DE D&#201;C&#200;S<", "<div>"))
            uc = getUntil("</div>");
        break;

    case Blenheim:
        if (moveTo("class=\"obituary-info cf\">"))
            uc = getUntil("</div>");
        break;

    case Brenneman:
        if (consecutiveMovesTo(1000, "id=\"viewer-c65df\"", "class=\"_2PHJq public-DraftStyleDefault-ltr\"", "<span"))
            uc = readNextBetween(BRACKETS);
        break;

    case Cardinal:
        if (consecutiveMovesTo(50, ">Obituary<", ">"))
            uc = getUntil("<div class=\"obit-details-section\">");
        break;

    case Carson:
        if (moveTo("<div class=\"obit\">"))
            uc = getUntil("</div>");
        break;

    case Turner:
        if (consecutiveMovesTo(1000, "id=\"main-content\"", "entry-content"))
        {
            moveToEarliestOf("<img", "<p");
            moveTo(">");
            uc = getUntil("<!-- .entry-content -->");
        }
        break;

    case Eagleson:
        if (consecutiveMovesTo(50, "<h", ">Obituary<", ">"))
            uc = getUntil("</div>");
        break;

    case FirstMemorial:
        if (consecutiveMovesTo(150, "class=\"content-outer\"", "</ul>"))
            uc = getUntil("</section>");
        break;

    case Haine:
        target = QString(">") + OQString(globals->globalDr->getID()).convertFromID() + QString("<");
        if (consecutiveMovesTo(200, target, "<div class=\"plain\">"))
            uc = getUntil("</div>");
        break;

    case RHB:
        if (moveTo("id=\"main-content\">"))
            uc = getUntil("</div>");
        break;

    case Rhody:
        if (consecutiveMovesTo(100, ">Rhody Family Funeral Home<", "<!-- .et_pb_text -->"))
            uc = getUntil("<!-- .et_pb_post_content -->");
        break;

    case Simpler:
        if (consecutiveMovesTo(2000, "<!-- Obit Container right col -->", "<br />"))
            uc = getUntil("<strong>Continue to");
        break;

    case Steadman:
        if (consecutiveMovesTo(500, "class=\"inner-page-ttl\"", "class=\"row\"", ">"))
            uc = getUntil("<p>Visitation");
        break;

    case Steeles:
        if (consecutiveMovesTo(1000, ">Contact Us<", "class=\"post-content\"", ">"))
            uc = getUntil("</span>");
        break;

    case Bridge:
        if (moveTo("itemprop=\"articleBody\">"))
            uc = getUntil("<div class=\"fastsocialshare_container\"");
        break;

    case McCormack:
        if (consecutiveMovesTo(15, "</svg>", "</div>"))
            uc = getUntil("</svg>");
        /*if (consecutiveMovesTo(30, ">Obituary Announcement<", ">"))
        {
            moveTo("</p>");
            uc = getUntil("#comp-");

            QRegularExpression target;
            QString tempString = uc.getString();
            target.setPattern("(, [1-2][0|9][0-9][0-9])-([A-Z][a-z])");
            tempString.replace(target, "\\1 - \\2");
            uc = tempString;
        }*/
        break;

    case Brunet:
        if (style == 1)
        {
            if (consecutiveMovesTo(100, ">Obituary<", ">"))
                uc = getUntil("</div>");
        }
        break;

    case TurnerFamily:
        if (moveTo("class=\"entry-content\">"))
            uc = getUntilEarliestOf("<!-- .entry-content -->", "<section");
        break;

    case VanHeck:
        if (consecutiveMovesTo(50, "class=\"entry-content\"", ">"))
            uc = getUntilEarliestOf("<!-- .entry-content -->", "<section");
        break;

    case TBK:
        if (consecutiveMovesTo(100, "id=\"field_tab_obituary\"", ">"))
            uc = getUntil("</div>");
        break;

    case Whelan:
        target = OQString(globals->globalDr->getID().left(15)).convertFromID();
        if (moveTo(target))
        {
            if (conditionalMoveTo("<p style=\"margin-bottom: 2em;\">", "<p style=\"margin-bottom: 1em;\">", 0))
                uc = getUntil("</p>");
        }
        break;

    case Aeterna:
        if (moveTo("class=\"death__notice-section\""))
            uc = getUntil("</div>");
        break;

    case Actuel:
        if (consecutiveMovesTo(20, "class=\"single-content\"", ">"))
            uc = getUntil("<div class=\"single-expo");
        break;

    case Dupuis:
        if (moveTo("<div class=\"description\"><div class=\"message\">"))
            uc = getUntil("</span></span>");
        break;

    case HGDivision:
        if (consecutiveMovesTo(50, "class=\"obituarie-content\"", ">"))
            uc = getUntil("</div>");
        break;

    case Jacques:
        if (consecutiveMovesTo(50, "class=\"item-title\"", "</h2>"))
            uc = getUntil("</div>");
        break;

    case Joliette:
        if (moveTo("class=\"post-content\">"))
            uc = getUntil("</div>");
        break;

    case Rajotte:
        target = QString("php/form_share.php?id=") + globals->globalDr->getID().getString();
        if (moveTo(target))
        {
            moveBackwardTo("<div class=\"td\"><p>", 5000);
            uc = getUntil("<table");
        }
        break;

    case BM:
        if (consecutiveMovesTo(100, "class=\"necrologie_fiche_right\"", "<h", ">"))
        {
            if (conditionalMoveTo("<pre", "<h"))
                moveTo("</pre>");
            uc = getUntil("<div");
        }
        break;

    case Jodoin:
        if (consecutiveMovesTo(100, "class=\"detailsdeces-fiche\"", "<p>"))
            uc = getUntil("</div>");
        break;

    case Fournier:
        if (moveTo("<div class=\"infos-avis\">"))
            uc = getUntil("</div>");
        break;

    case Desnoyers:
        if (moveTo("class=\"post-content\">"))
            uc = getUntil("</div>");
        break;

    case Desrosiers:
        if (moveTo("<div class=\"entry-content\">"))
            uc = getUntil("<!-- .entry-content -->");
        break;

    case MontPetit:
        if (moveTo("<div class=\"ds-1col node node-avis-de-deces view-mode-full clearfix \">"))
            uc = getUntil("<a class=\"a2a_button_facebook\"");
        break;

    case Parent:
        if (consecutiveMovesTo(100, "class=\"bloc-nec\"", ">"))
            uc = getUntil("</div>");
        break;

    case RichardPhilibert:
        if (moveTo("class=\"entry-body\">"))
            uc = getUntil("<!--/ .entry-body-->");
        break;

    case Kane:
        if (consecutiveMovesTo(150, ">In Memory Of<", "<div", ">"))
            uc = getUntil("</div>");
        break;

    case Gaudet:
        if (consecutiveMovesTo(5000, "<div id=\"texte\">", "href=\"avis-de-deces", "<p>"))
            uc = getUntil("<div class=\"avis\"");
        break;

    case NouvelleVie:
        if (consecutiveMovesTo(600, ">Imprimer<", "class=\"dates\"", "</span>"))
            uc = getUntil("<span class=\"sous_titre");
        break;

    case Santerre:
        if (consecutiveMovesTo(100, "class=\"post_content\"", ">"))
            uc = getUntil("<div class=\"at-below-post addthis_tool\"");
        break;

    case Shields:
        if (consecutiveMovesTo(100, ">AVIS DE D", "<div>"))
            uc = getUntil("</div>");
        break;

    case Gamache:
        if (consecutiveMovesTo(750, "<h4>Service</h4>", "</p>"))
            uc = getUntil("<div class=\"column mcb-column one column_column");
        break;

    case Landry:
        if (consecutiveMovesTo(350, "<article class=", "</header>"))
            uc = getUntil("<li class=\"dropdown\">");
        break;

    case StLouis:
        if (moveTo("class=m-obituary-single-details>"))
            uc = getUntil("</div>");
        break;

    case McGerrigle:
        if (consecutiveMovesTo(100, "class=\"entry-content\"", "articleBody", ">"))
            uc = getUntil("<!-- .entry-content -->");
        break;

    case Paperman:
        if (moveTo("<aside id='sidebar'>"))
            uc = getUntil("</section>");
        break;

    case Poissant:
        if (consecutiveMovesTo(200, "deces-liste-nom", "<p", ">"))
            uc = getUntil("</div>");
        break;

    case Legare:
        if (consecutiveMovesTo(50, "class=\"bodyDefunt\"", "</p>"))
            uc = getUntil("</div>");
        break;

    case Longpre:
        if (consecutiveMovesTo(300, "<!-- .et_pb_text -->", "class=\"et_pb_text_inner\"", ">"))
            uc = getUntil("<!-- .et_pb_text -->");
        else
        {
            beg();
            if (consecutiveMovesTo(400, "class=\"et_pb_module", "class=\"et_pb_text_inner\"", ">"))
                uc = getUntil("<div class=\"et_pb_section");
        }
        break;

    case Lanaudiere:
        if (consecutiveMovesTo(100, "class=\"wpb_text_column wpb_content_element", "class=\"wpb_wrapper\"", ">"))
            uc = getUntil("<div class=");
        else
        {
            beg();
            if (consecutiveMovesTo(1000, "class=\"post_content\"", "<h", ">"))
                uc = getUntil("<div class=");
            else
            {
                beg();
                consecutiveMovesTo(1000, "class=\"post_content\"", "<p", ">");
                    uc = getUntil("<div class=");
            }
        }
        break;

    case Theriault:
        if (consecutiveMovesTo(100, "", ""))
            uc = getUntil("</div>");
        break;

    case Voluntas:
        if (consecutiveMovesTo(100, "class=\"entry-content\"", ">"))
        {
            moveTo("</p>", 50);
            uc = getUntil("<section id=\"comments\"");
        }
        break;

    case Wilbrod:
        if (moveTo("<section>"))
            uc = getUntil("<!-- <h2>");
        break;

    case Hodges:
        moveTo("<!--POST BODY-->");
        conditionalMoveTo("</figure>", "<!--POST FOOTER-->", 0);
        if (consecutiveMovesTo(500, "sqs-block html-block sqs-block-html", "class=\"sqs-block-content\"", ">"))
            uc = getUntil("<div class=");
        break;

    case Bergeron:
        if (consecutiveMovesTo(500, "Imprimer l'avis de décès", "class=\"colonne-droite\">"))
            uc = getUntil("<div id=\"sympathies\">");
        break;

    case Passage:
        if (consecutiveMovesTo(400, "<article>", "<p>"))
            uc = getUntil("</div>");
        break;

    case Granit:
        if (consecutiveMovesTo(2500, "class=\"titreAvis\"", "</p></div>"))
            uc = getUntil("<p class=\"Titre_Scrypt_Avis\"");
        break;

    case Affordable:
        if (consecutiveMovesTo(2000, "<div class=\"site-inner\">", "<p", ">"))
        {
            conditionalMoveTo("Condolences</a>", "<p", 0);
            uc = getUntil("<a href='#respond'>");
        }
        break;

    case LFC:
        if (consecutiveMovesTo(100, "class=\"about-inner-content single_annoucement_content\"", ">"))
        {
            if(conditionalMoveTo("Date de Décès :", "</div>", 0))
                moveTo(">");
            uc = getUntil("</div>");
            uc.replace(" -", "");
        }
        break;

    case LifeTransitions:
        if (consecutiveMovesTo(100, "class=\"read-more-content\"", ">"))
            uc = getUntil("</div>");
        break;

    case Davis:
    {
        if (consecutiveMovesTo(100, ">ANNOUNCEMENT<", ">"))
            uc = getUntil("</div>");
    }
        break;

    case MacLark:
    {
        if (moveTo("class=\"tab-content\">"))
            uc = getUntil("</div>");
    }
        break;

    case Fallis:
    {
        if (moveTo("class=\"pure-u-1 pure-u-md-2-3 pure-form\""))
            uc = getUntil("<h2>");
    }
        break;

    case Timiskaming:
    {
        if (moveTo("class=\"obituary-image-main\">"))
            uc = getUntil("<div class=");
    }
        break;

    case Garrett:
    {
        if (consecutiveMovesTo(100, "class=\"nxs-default-p nxs-applylinkvarcolor nxs-padding-bottom0 nxs-align-left", ">"))
            uc = getUntil("</blockquote>");
    }
        break;

    case Smith:
    {
        if (consecutiveMovesTo(150, ">Back to List<", "</strong>", "<br />", "<br />"))
            uc = getUntil("<a href=");
    }
        break;

    case Picard:
    {
        target = OQString(globals->globalDr->getID()).convertFromID();
        if (moveTo(target))
        {
            moveTo(">");
            uc = getUntil("</div>");
        }
    }
        break;

    case Richelieu:
    {
        if (consecutiveMovesTo(250, "class=\"info font-ASAP\"", "<h4", ">"))
            uc = getUntil("<!-- testing -->");
    }
        break;

    case Roy:
    {
        if (consecutiveMovesTo(1200, "class=\"post-title\"", "</figure>"))
            uc = getUntil("<div class=");
    }
        break;

    case CharleVoix:
    {
        if (consecutiveMovesTo(500, "id=\"middle\"", "<p", ">"))
            uc = getUntil("</a>");
    }
        break;

    case Aurora:
    {
        if (moveTo("class=card-body>"))
            uc = getUntil("</div>");
    }
        break;

    case Montcalm:
    {
        if (consecutiveMovesTo(100, "class=\"post_title entry-title\"", ">"))
            uc = getUntil("<div class=");
    }
        break;

    case Trahan:
    {
    }
        break;

    case Laurent:
    {
        if (consecutiveMovesTo(500, "class=\"details\"", "<hr/>"))
            uc = getUntil("</div>");
    }
        break;

    case Eternel:
    {
        if (moveTo("id=\"tinfo\""))
            uc = getUntil("<div id=");
    }
        break;

    case Ruel:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(100, "</td>", "<td class=\"wsite-multicol-col\"", ">"))
                uc = getUntil("</td>");
            break;

        case 1:
            target = OQString(globals->globalDr->getID()).convertFromID();
            if (moveTo(target))
            {
                if (consecutiveMovesTo(100, "</td>", "<td class=\"wsite-multicol-col\"", ">"))
                    uc = getUntil("</td>");
            }
            break;
        }
    }
        break;

    case Hamel:
    {
        if (moveTo("<div id=\"memorial\">"))
            uc = getUntil("</div>");
    }
        break;

    case CremAlt:
    case Forest:
    case TriCity:
    {
        if (moveTo("class=\"blog-content\">"))
            uc = getUntil("<div class=\"blog-social");
    }
        break;

    case London:
    {
        if (consecutiveMovesTo(100, "id=\"listing-content\"", "<p>"))
            uc = getUntil("</div>");
    }
        break;

    case Dryden:
    {
        if (moveTo("class=\"post-content\">"))
            uc = getUntil("</div>");
    }
        break;

    case Lampman:
    {
        if (moveTo("class='entry-content'>"))
            uc = getUntil("<div id='comments'");
    }
        break;

    case ecoPassages:
    {
        if (moveTo("<div class=\"col sqs-col-8 span-8\">"))
            uc = getUntil("Online Condolences<");
    }
        break;

    case Peaceful:
    {
        if (moveTo("class=\"obituary-description\">"))
            uc = getUntil("</div>");
    }
        break;

    case Ranger:
    {
        if (moveTo("class=\"post_content\">"))
            uc = getUntil("<div class=");
    }
        break;

    case People:
    {
        moveTo("class=\"entry-content\"");
        conditionalMoveTo(globals->globalDr->getTitle().getString(), "<section id=", 0);
        uc = getUntil("<section id=");
    }
        break;

    case Whitcroft:
    {
        if (consecutiveMovesTo(300, "id=\"text3\"", "<p", ">"))
            uc = getUntil("</div>");
    }
        break;

    case LegacyCardstrom:
    {
        if (consecutiveMovesTo(1750, "<div id=\"content\">", "<div class=\"paragraph", ">"))
            uc = getUntil("<div id=\"footer\">");
        else
        {
            beg();
            if (consecutiveMovesTo(100, "class=\"wsite-button-inner\">", "</span>"))
                uc = getUntil("<div id=\"footer\">");
        }
    }
        break;

    case Wiebe:
    {
        if (consecutiveMovesTo(2000, "<div itemprop=\"articleBody\">", "<hr />", 0))
            uc = getUntil("</script>");

        /*QRegularExpression rxTarget;
        rxTarget.setPattern("\\d{4} (-|~) \\d{4}");
        int index = itsString.indexOf(rxTarget);
        if (index != -1)
        {
            forward(index);
            consecutiveMovesTo(50, "</", ">");
            uc = getUntil("<script");
        }
        else
        {
            if (moveTo("style=\"text-align: center;"))
            {
                if (conditionalMoveTo("<img", "</span>", 0))
                    moveTo("style=\"text-align: center;");
                if (conditionalMoveTo("<iframe", "</span>"))
                    moveTo("style=\"text-align: center;");
                while (moveTo("style=\"text-align: center;", 175)){
                    moveTo("</p>");}
                uc = getUntil("<script");
            }
        }*/
    }
        break;

    case Arimathea:
    {
        if (moveTo("<footer class="))
        {
            moveBackwardTo("</figure>", 15000);
            moveTo("<div class=\"sqs-block-content\">");
            uc = getUntil("</div>");
        }
    }
        break;

    case GFournier:
    {
        if (consecutiveMovesTo(1000, "class=\"banner-avis sxs\"", "<div class=\"wrap\">"))
            uc = getUntil("</div>");
    }
        break;

    case CharleVoix2:
    {
    }
        break;

    case Harmonia:
    {
        if (consecutiveMovesTo(200, "<div class=\"right\">", "<div>"))
            uc = getUntil("</section");
    }
        break;

    case Omega:
    {
        target = OQString(globals->globalDr->getID()).convertFromID();
        if (moveTo(target))
        {
            moveTo("</p>");
            moveTo("</p>");
            uc = getUntil("* * *");
        }
    }
        break;

    case HeritageWP:
    {
        if (consecutiveMovesTo(500, "class=\"entry-title\"", "</div>"))
            uc = getUntil("</div>");
    }
        break;

    case Ouellet:
    {
        if (consecutiveMovesTo(300, ">Envoyer un message<", "<div class=\"txt", ">"))
        {
            uc = getUntil("</div>");
            uc.removeHTMLtags();
        }

        if (uc.getLength() < 100)
        {
            beg();
            if (consecutiveMovesTo(300, ">Retour<", "<div class=\"txt", ">"))
            {
                if (conditionalMoveTo("font-size:26px", "</div>", 0))
                    consecutiveMovesTo(200, "<div class=\"txt", ">");
                uc = getUntil("</div>");
                uc.removeHTMLtags();
            }
        }

        if (uc.getLength() < 100)
        {
            beg();
            if(consecutiveMovesTo(750, "itemprop=\"name\"", ">R&#233;sidence Fun&#233;raire Ouellet<", "<div class=\"txt", ">"))
                uc = getUntil("</div>");
        }
    }
        break;

    case HommageNB:
    {
        if (moveTo("<!-- obituaries block -->"))
            uc = getUntil("</div>");
    }
        break;

    case Drake:
    {
        if (consecutiveMovesTo(750, "class=\"entry-title\"", "<p>"))
            uc = getUntil("</div>");
    }
        break;

    case CityLine:
    {
        if (consecutiveMovesTo(100, ">Write a Tribute<", "</div>"))
            uc = getUntil("</div>");
    }
        break;

    case Komitas:
    {
        if (consecutiveMovesTo(50, "class=\"life-legacy-content\"", ">"))
            uc = getUntil("</div>");
    }
        break;

    case Driftwood:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(75, "entry-content clearfix single-post-content", ">"))
                uc = getUntil("</div>");
            break;

        case 1:
            moveTo("class=\"tdb-title-text\"");
            if (consecutiveMovesTo(75, "tdb-block-inner td-fix-index", ">"))
                uc = getUntil("</div>");
            break;
        }
    }
        break;

    case MLBW:
    {
        if (consecutiveMovesTo(150, "461d092", "<div", ">"))
            uc = getUntil("</div>");
    }
        break;

    case Sproing:
    {
        if (consecutiveMovesTo(1750, "class=\"row\"", "is-layout-flow wp-block-column", "</div>"))
            uc = getUntil("</div>");
    }
        break;

    case WebCemeteries:
        uc = globals->globalDr->getObitSnippet();
        break;

    case Legacy3000:
        uc = globals->globalDr->getObitSnippet();
        break;

    default:
        PQString errMsg;
        errMsg << "Reading in of obit text not coded for: " << globals->globalDr->getProvider();
        globals->logMsg(ErrorRunTime, errMsg);

    }

    // Final attempt at a backup read
    if (uc.getLength() == 0)
    {
        beg();
        if (consecutiveMovesTo(75, "og:description", "content="))
        {
            uc = readQuotedMetaContent();
            uc.removeLeading("View ");
        }
    }
}

void readObit::processStructuredNames()
{
    unstructuredContent uc = globals->globalObit->getStructuredNamesProcessed();
    bool ignoreBookendedLetters = true;

    if (uc.contains(",", ignoreBookendedLetters))
        uc.readLastNameFirst(nameStatsList);
    else
        uc.readFirstNameFirst(nameStatsList);
}

void readObit::processObitText(bool UndoBadSentence, int insertPeriods)
{
    runStdProcessing(uc, insertPeriods);
    if (UndoBadSentence)
        uc.reverseUncapitalizedSentenceStarts();
    cleanAndExtract();
    // Outputs
    // uc               => Full processed text
    // justInitialNames => The start of the uc comprised only of names

    QStringList stopWords = getStopWords();
    uc.splitIntoSentences(mcn.listOfFirstWords, stopWords);

    // Setup cleaned content
    unstructuredContent sentence, tempUC;
    OQString newSentence;
    bool firstSentence;
    unsigned int numSentences = 0;
    LANGUAGE lang = uc.getLanguage();

    ucCleaned.setContentLanguage(lang);
    ucFillerRemoved.setContentLanguage(lang);
    ucFillerRemovedAndTruncated.setContentLanguage(lang);

    uc.beg();
    while (!uc.isEOS())
    {
        sentence = uc.getSentence();
        if (sentence.getLength() > 0)
        {
            numSentences++;

            sentence.clean(lang);
            newSentence = sentence.getString() + QString(". ");
            ucCleaned.addSentence(newSentence);

            sentence.removeDates(lang);
            sentence.removeSpouseForFiller(lang);
            sentence.removeBornToFiller(lang);
            sentence.removeAtTheHospitalFiller(lang);
            newSentence = sentence.getString() + QString(". ");
            ucFillerRemoved.addSentence(newSentence);

            if (numSentences <= 5)
            {
                if (numSentences == 1)
                    firstSentence = true;
                else
                    firstSentence = false;

                sentence = OQString(" ") + sentence;
                sentence.truncateAfter(":");
                sentence.truncateAfter(OQString::getParentReferences(lang), firstSentence);
                sentence.truncateAfter(OQString::getSiblingReferences(lang), firstSentence);
                sentence.truncateAfter(OQString::getChildReferences(lang), firstSentence);
                sentence.truncateAfter(OQString::getRelativeReferences(lang), firstSentence);
                sentence.truncateAfter(OQString::getRelationshipWords(lang), firstSentence);
                sentence.truncateAfter(OQString::getMiscWords(lang), firstSentence);
                sentence.removeSpousalReference(lang, firstSentence);
                sentence.removeEnding(PUNCTUATION);
                tempUC.compressCompoundNames(sentence, lang);
                if ((sentence.getLength() == 0) || (sentence.getString() == QString(" ")))
                    sentence = OQString("VoidedOutSentence");
                newSentence = sentence.getString() + QString(". ");
                newSentence.removeLeading(QString(" "));
                ucFillerRemovedAndTruncated.addSentence(newSentence);
            }
        }
    }
}

void readObit::readInObitTitle()
{
    // Language is unknown at this stage
    OQStream titleStream, cleanStream;
    OQString titleText;
    QString targetText;
    databaseSearches dbSearch;
    unstructuredContent tempUC;

    beg();

    switch(globals->globalDr->getProvider())
    {
    case Legacy:
    case Tukio:
        if (moveTo("<title>"))
        {
            titleStream = getUntilEarliestOf(" Obituary", "</title>");
            titleStream.removeEnding("|");
            titleStream.removeEnding(" ");
        }
        break;

    case LifeMoments:
        break;

    case SIDS:
        break;

    case Batesville:
        switch(style)
        {
        case 6:
        case 5:
        case 4:
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
    case WordPress:
    case Serenity:
    case Sturgeon:
    case Shape5:
    case Dalmeny:
    case Shine:
    case Jelly:
    case ReneBabin:
    case MPG:
    case SquareSpace:
    case Eggs:
    case ONeil:
    case Simply:
    case Hodges:
    case Elegant:
    case SoleWeb:
    case Jaimonsite:
    case ToraPro:
    case Axanti:
    case Tomahawk:
    case Gemini:
    case District4Web:
    case Cake:
    case Reactif:
    case Kowalchuk:
    case Doyle:
    case JOsmond:
    case KMF:
    case Landry:
    case Lanaudiere:
    case Voluntas:
    case BallaMedia:
    case Nexion:
    case Absolu:
    case Aurora:
    case Montcalm:
    case Eternel:
    case Ouellet:
    case HommageNB:
    case DragonFly:
    case NMedia:
    case Brenneman:
        if (moveTo("<title>"))
            titleStream = getUntil("</title>");
        break;

    case EchoVita:
    case SaltWire:
    case Tegara:
    case Arbor:
    case TributeArchive:
    case Nirvana:
    case Funks:
    case WowFactor:
    case WebSolutions:
    case OSM:
    case Beechwood:
    case Cardinal:
    case Turner:
    case Rhody:
    case Steadman:
    case Bridge:
    case TurnerFamily:
    case Lithium:
    case Trinity:
    case LogiAction:
    case BLsolutions:
    case ImageXpert:
    case McCall:
    case Heritage:
    case Ethical:
    case SMC:
    case Dupuis:
    case HGDivision:
    case MontPetit:
    case RichardPhilibert:
    case Affordable:
    case PropulC:
    case iClic:
    case GyOrgy:
    case Davis:
    case Fallis:
    case Timiskaming:
    case Ranger:
    case People:
    case Voyou:
    case CityLine:
    case FirstMemorial:
    case Etincelle:
    case Driftwood:
    case Drake:
        if (moveTo("<title>"))
            titleStream = getUntilEarliestOf("|", "<");
        break;

    case Passages:
    case CooperativeFuneraire:
    case GreatWest:
    case BlackPress:
    case Village:
    case Aberdeen:
    case ConnellyMcKinley:
    case Codesign:
    case Hansons:
    case Crew:
    case Bergetti:
    case ROI:
    case Vernon:
    case Ashlean:
    case Halifax:
    case Websites:
    case Coop:
    case Citrus:
    case MLBW:
    case SandFire:
    case Linkhouse:
    case ChadSimpson:
    case InView:
    case Imago:
    case Steeles:
    case Globalia:
    case NBL:
    case Morin:
    case Scrypta:
    case Saguenay:
    case B367:
    case Descary:
    case Alias:
    case NetRevolution:
    case Kerozen:
    case InoVision:
    case Koru:
    case Direct:
    case Carnell:
    case Aeterna:
    case Actuel:
    case Joliette:
    case Desrosiers:
    case StLouis:
    case McGerrigle:
    case Paperman:
    case Longpre:
    case Wilbrod:
    case BlackCreek:
    case SYGIF:
    case LFC:
    case SuiteB:
    case Map:
    case Theories14:
    case Techlogical:
    case GemWebb:
    case RedChair:
    case Cahoots:
    case RKD:
    case Garrett:
    case Roy:
    case CremAlt:
    case Dryden:
    case Lampman:
    case LifeTransitions:
    case ecoPassages:
    case Peaceful:
    case LegacyCardstrom:
    case Arimathea:
    case Harmonia:
    case HeritageWP:
    case Komitas:
    case TBK:
    case PassageCoop:
    case Brunet:
    case Kane:
    case Carve:
    case Sproing:
    case Richelieu:
        if (moveTo("<title>"))
            titleStream = getUntilEarliestOf(" - ", "<");
        break;

    case DomainFuneraire:
        if (consecutiveMovesTo(100, "<title>", "Avis", " - "))
            titleStream = getUntilEarliestOf(" |", "<");
        else
        {
            beg();
            if (moveTo("<title>"))
                titleStream = getUntil(" - ");
        }
        break;

    case FuneralTech:
        switch(style)
        {
        case 0:
        case 1:
        case 2:
        case 5:
            moveTo("<title>");
            conditionalMoveTo("Obituary of ", "</title>");
                titleStream = getUntilEarliestOf("|", "<");
            break;

        case 3:
            if (moveTo("<title>"))
                titleStream = getUntilEarliestOf("</title>", " - ");
            break;

        case 4:
            if (consecutiveMovesTo(100, "class=\"deceased-info\"", "<h"))
                titleStream = readNextBetween(BRACKETS);
            break;
        }

        break;

    case Burke:
    case FrontRunner:
        if (globals->globalDr->getProviderKey() == 25)
        {
            if (consecutiveMovesTo(50, "og:title", "content="))
            {
                conditionalMoveTo(" of ", ">", 0);
                titleStream = getUntil(" |");
            }
        }
        else
        {
            if (moveTo("<title>"))
            {
                conditionalMoveTo(" for ", "</title>", 0);
                conditionalMoveTo(" of ", "</title>", 0);
                titleStream = getUntilEarliestOf("</title>", " | ");

                if (titleStream == OQString("Book of Memories"))
                {
                    if (consecutiveMovesTo(50, "og:title", "content="))
                    {
                        titleStream = readQuotedMetaContent();
                        // Includes style = 3
                    }
                    else
                    {
                        beg();
                        if (consecutiveMovesTo(200, "id=\"bom-tunnel-deceased\">", "<span>"))
                            titleStream = getWord();
                    }
                }
            }            
        }
        break;

    case DignityMemorial:
        if (consecutiveMovesTo(100, "og:title", "content="))
        {
            titleStream = readQuotedMetaContent();
            if (titleStream.left(5) == PQString("Find "))
            {
                // Most likely means a deleted obituary, so pull name from URL
                QString tempString = globals->globalDr->getURL();
                int indexA = tempString.lastIndexOf("/");
                int indexB = tempString.lastIndexOf("-");
                if ((indexA >=0) && (indexB >= 0))
                {
                    indexA++;
                    tempString.replace("-", " ");
                    titleStream = tempString.mid(indexA, indexB - indexA);
                }
            }
        }
        break;

    case Cible:
    case YAS:
    case Ancient:
    case Loehmer:
    case NouvelleVie:
    case Granit:
    case Forest:
    case TriCity:
        if (consecutiveMovesTo(100, "og:title", "content="))
            titleStream = readQuotedMetaContent();
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
        if (consecutiveMovesTo(100, "<title>", " for "))
            titleStream = getUntil("|");
        break;

    case FuneralOne:
        switch (style)
        {
        case 0:
            if (moveTo("itemprop=\"givenName\""))
                titleStream = readNextBetween(BRACKETS);
            break;

        case 1:
            if (moveTo("<title>"))
                titleStream = getUntil("</title>");
            break;
        }
        break;

    case WebStorm:
        switch (style)
        {
        case 0:
            if (moveTo("class=\"fancy\""))
                titleStream = readNextBetween(BRACKETS);
            break;

        case 1:
        case 2:
            if (moveTo("<title>"))
                titleStream = getUntil("</title>");
            break;
        }
        break;

    case FHW:
        switch (style)
        {
        case 0:
        case 2:
            if (moveTo("<title>"))
                titleStream = getUntil("</title>");
            break;

        case 1:
            if (consecutiveMovesTo(100, "property=\"og:title\"", "content="))
                titleStream = readQuotedMetaContent();
            break;
        }
        break;

    case GasLamp:
    case PacificByte:
        if (consecutiveMovesTo(100, "<title>", "Online Tribute for "))
            titleStream = getUntil("|");
        break;

    case ClickTributes:
        if (consecutiveMovesTo(100, "pagetitle", ">Obituary - "))
            titleStream = getUntil("<");
        else
        {
            beg();
            if (consecutiveMovesTo(100, "<title>", " for "))
                titleStream = getUntilEarliestOf(" - ", "<");
        }
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

    case WFFH:
        if (consecutiveMovesTo(500, "persondetails-area", "personName", "personFirst"))
            titleStream = readNextBetween(BRACKETS);
        break;

    case Gustin:
    case Orillia:
        if (consecutiveMovesTo(10, "<title>", "Remembering "))
            titleStream = getUntil(" | ");
        break;

    case Specialty:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(50, "class=\"contentPanel\"", "<h"))
                titleStream = readNextBetween(BRACKETS);

            if (titleStream.getLength() == 0)
            {
                beg();
                if (moveTo("<title>"))
                    titleStream = getUntil("</title>");
            }

            break;

        case 1:
            if (moveTo("<title>"))
                titleStream = getUntil("</title>");
            break;
        }

        break;

    case Smiths:
        if (moveTo("<title>Obituary "))
        {
            titleStream = getUntil("</title>");
            titleStream.removeBookEnds(PARENTHESES);
        }
        break;

    case Seabreeze:
        // Part of group of obits
        targetText = QString("thankyou") + globals->globalDr->getID().getString();
        if (moveTo(targetText))
        {
            moveBackwardTo("data-role=\"collapsible\"");
            consecutiveMovesTo(25, "<span", ">");
            backward(1);
            titleStream = readNextBetween(BRACKETS);
        }
        break;

    case RedSands:
    case BrandAgent:
    case EPmedia:
    case Eagleson:
    case VanHeck:
    case PortNeuf:
    {
        if (moveTo("<title>"))
            titleStream = getUntilEarliestOf(" &#8211; ", "</title>", 100);
    }
        break;

    case AdGraphics:
    {
        // Part of group of obits
        targetText = QString("images/") + globals->globalDr->getID().getString() + QString(".");
        if (moveTo(targetText))
        {
            moveBackwardTo("<tr>", 7500);
            consecutiveMovesTo(150, "<td align", ">");
            titleStream = getUntilEarliestOf("</strong>", "</p>");
            titleStream.removeHTMLtags();
            titleStream.cleanUpEnds();
        }
    }
        break;

    case MediaHosts:
    {
        if (moveTo("<title>"))
            titleStream = getUntil(" Obituary");
    }
        break;

    case MyFavourite:
    {
        if (moveTo("<title>"))
            titleStream = getUntil(" Obituary");
    }
        break;

    case EverTech:
    {
        if (moveTo("<title>Memorial - "))
            titleStream = getUntil("</title>");
    }
        break;

    case MacLean:
    {
        targetText = QString("modal_") + globals->globalDr->getID().getString();
        if (moveTo(targetText))
        {
            moveBackwardTo("<header>", 500);
            moveTo("<h");
            titleStream = readNextBetween(BRACKETS);
        }
    }
        break;

    case MCG:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(1000, "id=\"cwp_funerals_details\"", "class=\"details\"", "<h"))
                titleStream = readNextBetween(BRACKETS);
            break;

        case 1:
            if (moveTo("ctl00_cphBody_fvDetails_lblTitle"))
                titleStream = readNextBetween(BRACKETS);
            break;
        }
    }
        break;

    case TNours:
    {
        if (consecutiveMovesTo(100, "<title>", ":: "))
            titleStream = getUntil("</title>");
    }
        break;

    case LinkWeb:
    {
        if (consecutiveMovesTo(50, "class=\"uk-panel\"", "class=\"uk-panel-title\""))
            titleStream = readNextBetween(BRACKETS);
    }
        break;

    case JoshPascoe:
    {
        if (consecutiveMovesTo(25, "name=\"title\"", "content="))
            titleStream = readNextBetween(QUOTES);
    }
        break;

    case MarketingImages:
    {
        if (consecutiveMovesTo(50, "class=\"details\"", "<h"))
            titleStream = readNextBetween(BRACKETS);
    }
        break;

    case ESite:
    {
        switch(globals->globalDr->getProviderKey())
        {
        case 1:
            if (consecutiveMovesTo(100, "<title>", "Obituary for "))
                titleStream = getUntil("<");
            break;

        case 2:
            if (moveTo("<title>"))
                titleStream = getUntil("</title>");
            break;
        }
    }
        break;

    case MFH:
    {
        if (consecutiveMovesTo(100, "<title>", " of "))
            titleStream = getUntil(" |");
    }
        break;

    case Back2Front:
    {
        if (moveTo("class=\"app.tbl.first_name\""))
            titleStream = readNextBetween(BRACKETS);
    }
        break;

    case CreativeOne:
    {
        if (moveTo("<>"))
        {
            titleStream = getUntil("<>");
        }
    }
        break;

    case SDP:
    {
        if (consecutiveMovesTo(100, "<div class=\"fltlft content50\">", "<h3"))
            titleStream = readNextBetween(BRACKETS);
    }
        break;

    case Vortex:
    {
        switch(style)
        {
        case 0:
            if (moveTo("<title>Obituary "))
                titleStream = getUntil("<");
            break;

        case 1:
            if (moveTo(" - "))
                titleStream = getUntil("<");
            break;

        case 2:
            if (moveTo("<title>"))
                titleStream = getUntilEarliestOf(" - ", "<");
        }
    }
        break;

    case YellowPages:
    {
        switch(style)
        {
        case 0:
            targetText = QString("id=\"title") + globals->globalDr->getID().getString();
            if (consecutiveMovesTo(75, targetText, ">"))
            {
                backward(1);
                titleStream = readNextBetween(BRACKETS);
            }
            break;

        case 1:
            if (moveTo("<title>"))
                titleStream = getUntil("<");
            break;
        }
    }
        break;

    case Shooga:
    {
        if (consecutiveMovesTo(50, "<title>", " | "))
            titleStream = getUntil("<");
    }
        break;

    case WPBakery:
    {
        if (consecutiveMovesTo(100, "<title>", "Obituary for "))
            titleStream = getUntil("|");
        else
        {
            beg();
            if (moveTo("<title>"))
                titleStream = getUntil("<");
        }
    }
        break;

    case TroisJoueur:
    {
        if (moveTo("<>"))
        {
            titleStream = getUntil("<>");
        }
    }
        break;

    case Ubeo:
    {
        switch(style)
        {
        case 0:
            if (moveTo("property=\"og:title\" content=\""))
                titleStream = getUntil(" | ");
            break;

        case 1:
            if (moveTo("<title>"))
                titleStream = getUntil("</title>");
            break;
        }

    }
        break;

    case Acolyte:
    {
        if (moveTo("<>"))
        {
            titleStream = getUntil("<>");
        }
    }
        break;

    case Taiga:
    {
        if (consecutiveMovesTo(150, "property=\"og:title\"", "Avis de décès - "))
            titleStream = getUntil("\"");
    }
        break;

    case Zonart:
    {
        if (moveTo("<>"))
        {
            titleStream = getUntil("<>");
        }
    }
        break;

    case PubliWeb:
    {
        switch(style)
        {
        case 0:
            if (moveTo("<title>"))
                titleStream = getUntil(" | ");
            break;

        case 1:
            if (consecutiveMovesTo(50, "<meta name=\"description\"", "Avis de décès de "))
                titleStream = getUntil("\"");
            break;
        }
   }
        break;

    case DirectImpact:
    {
        if (consecutiveMovesTo(50, "<title", ">"))
            titleStream = getUntilEarliestOf(" - ", "<");
    }
        break;

    case Cameleon:
    {
        if (moveTo("<title>Avis de décès de "))
            titleStream = getUntil("</title>");
    }
        break;

    case ADN:
    {
        if (moveTo("<title>Avis de décès - "))
            titleStream = getUntil("</title>");
    }
        break;

    case Caza:
    case Webs:
    {
        if (consecutiveMovesTo(100, "<title>", " - "))
            titleStream = getUntil("<");
    }
        break;

    case Tonik:
    {
        if (moveTo("class=\"page-title\""))
            titleStream = readNextBetween(BRACKETS);
    }
        break;

    case Kaleidos:
    {
        if (consecutiveMovesTo(50, "<title>", "-- "))
            titleStream = getUntil("<");
    }
        break;

    case Web2u:
    {
        if (moveTo("<>"))
        {
            titleStream = getUntil("<>");
        }
    }
        break;

    case J27:
    {
        if (consecutiveMovesTo(100, "<title>", "Fils - "))
            titleStream = getUntil("<");
    }
        break;

     case Boite:
    {
        if (consecutiveMovesTo(50, "<title>", "souvenir "))
            titleStream = getUntil("<");
    }
        break;

    case Orage:
    {
        if (consecutiveMovesTo(100, "og:title", "Avis de décès - "))
            titleStream = getUntilEarliestOf("(", "\"");
    }
        break;

    case FRM:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(100, "property=\"og:title\"", "content="))
                titleStream = readQuotedMetaContent();
            break;

        case 1:
        case 2:
            if (moveTo("<title>"))
                titleStream = getUntil(" - ");
            break;

        case 3:
        case 4:
            if (moveTo("<title>"))
                titleStream = getUntil(" | ");
            break;
        }
    }
        break;

    case JBCote:
        switch(style)
        {
        case 0:
            if (moveTo("<title>"))
                titleStream = getUntilEarliestOf(" - ", "<");
            break;

        case 1:
            if (consecutiveMovesTo(400, "class=\"avis\"", "<td valign=\"top\">", "<b"))
                titleStream = readNextBetween(BRACKETS);
            break;
        }

        break;

    case CityMax:
    {
        targetText = OQString(globals->globalDr->getID()).convertFromID();
        if (moveTo(targetText))
        {
            moveBackwardTo("<h");
            titleStream = readNextBetween(BRACKETS);
        }
    }
        break;

    case Canadian:
    {
        if (moveTo("<title>"))
        {
            titleStream = getUntilEarliestOf("</title>", " - ");
            titleStream.fixBasicErrors(true);
            tempUC.compressCompoundNames(titleStream, globals->globalDr->getLanguage());
            titleStream.removeRepeatedLastName();
        }
    }
        break;

    case Jac:
    {
        if (consecutiveMovesTo(250, "src=\"images/name.png\"", "<TD", ">"))
            titleStream = getUntil("</TD>");
    }
        break;

    case Ministry:
    {
        QString target;
        target = OQString(globals->globalDr->getID()).convertFromID();
        if (moveTo(target))
        {
            moveBackwardTo("<br />");
            titleStream = getUntil("<br />");
            titleStream.removeHTMLtags();
            titleStream.replaceHTMLentities();
            titleStream.simplify();

            if (titleStream.getLength() == 0)
            {
                titleStream = getUntil("<br />");
                titleStream.removeHTMLtags();
                titleStream.replaceHTMLentities();
                titleStream.simplify();
            }
        }
    }
        break;

    case Multinet:
    {
        if (moveTo("class=\"h3\""))
            titleStream = readNextBetween(BRACKETS);
        //titleStream = OQString(globals->globalDr->getID()).convertFromID();
    }
        break;

    case LCProduction:
    {
        if (consecutiveMovesTo(50, "<title>", "  "))
            titleStream = getUntilEarliestOf(" | ", "</title>");
    }
        break;

    case ExtremeSurf:
    {
        if (consecutiveMovesTo(50, "<title>", " | "))
            titleStream = getUntil("</title>");
    }
        break;

    case Tride:
    {
        if (consecutiveMovesTo(50, "<title>", " for "))
            titleStream = getUntilEarliestOf(" - ", "</title>");
    }
        break;

    case Jensii:
    {
        if (consecutiveMovesTo(50, "<title>", "Obituary: "))
            titleStream = getUntil("</title>");
    }
        break;

    case InterWeb:
    {
        if (consecutiveMovesTo(50, "og:title", "Se souvenir "))
            titleStream = getUntil("\">");
    }
        break;

    case Brown:
    {
        if (consecutiveMovesTo(75, "<title>", " - "))
            titleStream = getUntil("<");
    }
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
    case SRS:
        switch(style)
        {
        case 0:
        case 1:
            if (consecutiveMovesTo(100, "og:image:alt", "content="))
                titleStream = readQuotedMetaContent();
            break;

        case 2:
            if (moveTo("<title>"))
                titleStream = getUntil(" Obituary");
            break;
        }
        break;

    case CelebrateLife:
        if (moveTo("<title>"))
            titleStream = getUntil(" - CELEBRATE");
        break;

    case Martens:
        if (moveTo("<title>Remembering "))
            titleStream = getUntil("<");
        break;

    case Care:
        if (consecutiveMovesTo(100, "og:title", "content=", "Remembering "))
            titleStream = getUntil("\"");
        break;

    case Amherst:
        if (moveTo("<title>"))
            titleStream = getUntil(" Memorial");
        break;

    case Belvedere:
        if (consecutiveMovesTo(75,"<title>", " - "))
            titleStream = getUntil("<");
        break;

    case Davidson:
        if (consecutiveMovesTo(100, "class=\"details\"", "<h"))
            titleStream = readNextBetween(BRACKETS);
        break;

    case TivaHost:
        if (consecutiveMovesTo(50, "<title>", "Obituary for "))
            titleStream = getUntil("<");
        break;

    case AMG:
        if (consecutiveMovesTo(100, "class=\"title\"", "<h"))
            titleStream = readNextBetween(BRACKETS);
        break;

    case Alcock:
        if (moveTo("</span></span>"))
        {
            moveBackwardTo("<span", 200);
            moveTo(">");
            backward(1);
            titleStream = readNextBetween(BRACKETS);
        }
        break;

    case Abstract:
        if (consecutiveMovesTo(30, "class=\"right\"", "<h"))
            titleStream = readNextBetween(BRACKETS);
        break;

    case Benjamins:
        if (moveTo("id=\"ContentPlaceHolder1_Label1\""))
            titleStream = readNextBetween(BRACKETS);
        break;

    case Berthiaume:
        if (consecutiveMovesTo(50, "<title>", "Fiche de "))
            titleStream = getUntil(" - ");
        break;

    case Blenheim:
        if (consecutiveMovesTo(50, "<title>", "Funeral Home "))
            titleStream = getUntil("</title>");
        break;

    case Carson:
        if (consecutiveMovesTo(50, "<title>", "Remembering "))
            titleStream = readNextBetween(BRACKETS);
        break;

    case Haine:
        titleStream = OQString(globals->globalDr->getID()).convertFromID();
        break;

    case RHB:
        if (consecutiveMovesTo(75, "<title>", " :: "))
            titleStream = getUntil("</title>");
        break;

    case Simpler:
        if (consecutiveMovesTo(500, "<!-- Obit Container right col -->", "<h"))
            titleStream = readNextBetween(BRACKETS);
        break;

    case McCormack:
        if (consecutiveMovesTo(100, "<title>", "Stewart Chapel - "))
            titleStream = getUntil("</title>");
        else
        {
            beg();
            if (moveTo("<title>"))
                titleStream = getUntil("</title>");
        }
        break;

    case Whelan:
        targetText = OQString(globals->globalDr->getID().left(15)).convertFromID();
        if (moveTo(targetText))
        {
            moveBackwardTo("<b");
            titleStream = readNextBetween(BRACKETS);
        }
        break;

    case Jacques:
        if (consecutiveMovesTo(100, "og:title", "Avis de décès: "))
            titleStream = getUntil("/");
        break;

    case Rajotte:
        targetText = QString("php/form_share.php?id=") + globals->globalDr->getID().getString();
        if (moveTo(targetText))
        {
            moveTo("Avis de décès - ");
            titleStream = getUntil(" - ");
        }
        break;

    case BM:
        if (consecutiveMovesTo(100, "class=\"necrologie_fiche_right\"", "<h"))
            titleStream = readNextBetween(BRACKETS);
        break;

    case Jodoin:
        if (consecutiveMovesTo(30, "<title>", "Avis de", "s "))
            titleStream = getUntilEarliestOf(" - ", "<");
        break;

    case Fournier:
        if (consecutiveMovesTo(30, "og:title", "Avis de", "s "))
            titleStream = getUntilEarliestOf(" | ", "<");
        break;

    case Desnoyers:
        if (consecutiveMovesTo(50, "<title>", "| "))
            titleStream = getUntilEarliestOf(" | ", "<");
        break;

    case Parent:
        if (consecutiveMovesTo(50, "og:title", "content=\""))
        {
            conditionalMoveTo("Avis de décès de ", ">", 0);
            titleStream = getUntil("\"");
        }
        break;

    case Gaudet:
        if (consecutiveMovesTo(100, "og:title", "Avis de", "s : "))
            titleStream = getUntil("\"");
        else
        {
            beg();
            consecutiveMovesTo(1500, "href=\"avis-de-deces-archives.php?p=", "<h1", "?");
            backward(1);
            titleStream = readNextBetween(BRACKETS);
        }
        break;

    case Santerre:
        if (consecutiveMovesTo(50, "<title>", "s : "))
            titleStream = getUntil("<");
        break;

    case Shields:
        if (consecutiveMovesTo(50, "<title>", "Fiche de "))
            titleStream = getUntilEarliestOf("<", " - ");
        break;

    case Gamache:
        if (consecutiveMovesTo(100, "<title>", " pour "))
            titleStream = getUntil("</title>");
        break;

    case Poissant:
        if (consecutiveMovesTo(100, "<title>", " - "))
            titleStream = getUntil("</title>");
        break;

    case Legare:
        if (consecutiveMovesTo(100, "id=\"txtDefunt\"", "<span", ">"))
            titleStream = getUntil("<");
        break;

    case Bergeron:
        if (consecutiveMovesTo(50, "<title>", "Salon", " - "))
            titleStream = getUntil("<");
        break;

    case Passage:
        if (moveTo("class=\"belwe\""))
            titleStream = readNextBetween(BRACKETS);
        break;

    case MacLark:
        if (consecutiveMovesTo(200, ">Book of Condolences<", "<li"))
            titleStream = readNextBetween(BRACKETS);
        break;

    case Smith:
        if (consecutiveMovesTo(100, ">Back to List<", "<strong"))
            titleStream = readNextBetween(BRACKETS);
        break;

    case Picard:
    {
        targetText = OQString(globals->globalDr->getID()).convertFromID();
        if (moveTo(targetText))
        {
            moveBackwardTo("<h3");
            titleStream = readNextBetween(BRACKETS);
            OQString temp = titleStream.left(1);
            if (temp.isNumeric() || (temp == OQString("à")))
            {
                int index = titleStream.findPosition("-");
                if (index >= 0)
                    titleStream.dropLeft(index + 1);
            }
            titleStream.cleanUpEnds();
        }
    }
        break;

    case CharleVoix:
        if (consecutiveMovesTo(250, "id=\"middle\"", "<br>"))
            titleStream = getUntil("<");
        break;

    case Laurent:
        if (consecutiveMovesTo(100, "<title>", "Avis de décès de "))
            titleStream = getUntilEarliestOf(" | ", "<");
        break;

    case Ruel:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(200, "class=\"wsite-menu-subitem-wrap wsite-nav-current\"", "class=\"wsite-menu-title\""))
                titleStream = readNextBetween(BRACKETS);
            break;

        case 1:
            targetText = OQString(globals->globalDr->getID()).convertFromID();
            if (moveTo(targetText))
                titleStream = targetText;
            break;
        }
        break;

    case Hamel:
        if (consecutiveMovesTo(250, "wpfh-single-header-right", "href", ">"))
            titleStream = getUntil("<");
        break;

    case London:
        if (moveTo("<title>"))
            titleStream = getUntilEarliestOf("-", "<");
        break;

    case Whitcroft:
        if (consecutiveMovesTo(300, "id=\"text1\"", "<I>"))
            titleStream = getUntil("<");
        else
        {
            beg();
            if (consecutiveMovesTo(300, "id=\"text1\"", "<i>"))
                titleStream = getUntil("<");
        }
        break;

    case Wiebe:
        moveTo("<title>");
        if (conditionalMoveTo("Ltd.", "</title>", 0))
            moveTo(" - ");
        titleStream = getUntil("<");
        break;

    case GFournier:
        if (consecutiveMovesTo(150, "<title", "Avis de", " de "))
            titleStream = getUntilEarliestOf(" | ", "<");
        break;

    case Omega:
        targetText = OQString(globals->globalDr->getID()).convertFromID();
        if (moveTo(targetText))
            titleStream = targetText;
        break;

    case ObitAssistant:
    {
        OQStream word;
        OQStream tempStream = globals->globalDr->getURL();
        int index = tempStream.findPosition("=");
        tempStream.dropLeft(index+1);

        while (!tempStream.isEOS()){
            titleStream += tempStream.getUntil("-").proper() + PQString(" ");
        }
        titleStream.cleanUpEnds();
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
        cleanStream = titleStream.convertFromID();
        cleanStream.simplify();
        cleanStream.replaceHTMLentities();
        cleanStream.standardizeQuotes();
        /*if (cleanStream.fixQuotes())
        {
            PQString errMsg;
            errMsg << "Potential issue with mismatched quotes for: " << globals->globalDr->getURL();
            globals->logMsg(ErrorRecord, errMsg);
        }*/
        cleanStream.fixBasicErrors();
        cleanStream.cleanUpEnds();
        globals->globalDr->setTitle(cleanStream.getString());

        titleText = cleanStream.getCleanWord();
        while (!cleanStream.isEOS() && ((titleText.getLength() < 2) || titleText.isRecognized()))
        {
            titleText = cleanStream.getCleanWord().proper();
        }

        if ((titleText.getLength() > 0) && !titleText.isRecognized())
        {
            globals->globalDr->setTitleKey(titleText);

            NAMESTATS nameStats;
            dbSearch.nameStatLookup(titleText.getString(), globals, nameStats);

            if (nameStats.isLikelyGivenName)
            {
                if (nameStats.malePct >= 0.75)
                    globals->globalDr->setWorkingGender(Male);
                else
                {
                    if (nameStats.malePct <= 0.25)
                        globals->globalDr->setWorkingGender(Female);
                }
            }
        }
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
    case FosterMcGarvey:
        if (consecutiveMovesTo(40, "\"Copyright\"", "<br>"))
        {
            datesText = getUntil("</td>");
            datesText.simplify();
            ucDates = datesText;
            dates = ucDates.readDates(globals->globalDr->getLanguage());
        }
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
                if (currentDOB.year() >= 2020)
                    globals->globalDr->wi.dateFlag = 10;
                else
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
                if (dates.potentialDOD < currentDOD)
                    globals->globalDr->wi.dateFlag = 13;
                else
                    globals->globalDr->wi.dateFlag = 3;
            }
        }
    }
}

void readObit::readLookupFields(const SEARCHPARAM &sp, const fieldNames &FieldNames)
{
    Q_UNUSED(sp)
    Q_UNUSED(FieldNames)    
}

void readObit::validateJustInitialNames()
{
    LANGUAGE lang = globals->globalDr->getLanguage();
    GENDER gender = globals->globalDr->getGender();

    databaseSearches dbSearch;
    NAMESTATS nameStats;
    NAMETYPE nameType;
    unstructuredContent firstTwoSentences, tempSentence, tempUC, UCsource;
    OQString checkWord, tempWord, nextWord;
    bool hasBookEnds, isAboriginal, hasComma, started, hadComma;
    bool potentialGenderMarker = false;
    QList<QString> nameList;
    int wordCount;

    OQString originalWord, word, lastWord, doubleWord, wordBefore1, wordBefore2;
    unsigned int numWords;
    bool unrecognizedWordEncountered, invalidated;
    bool restrictNamesToDR;

    if (globals->globalDr->getProvider() == SaltWire)
    {
        justInitialNamesUC.clear();
        return;
    }

    justInitialNamesUC.fixBasicErrors();
    justInitialNamesUC.removeIntroductions();

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

        tempUC.compressCompoundNames(justInitialNamesUC, lang);
        OQStream checkStream(justInitialNamesUC);

        while (noMatch && (i < cutOff))
        {
            checkWord = checkStream.getWord().getString();
            if (gender == genderUnknown)
            {
                if (checkWord.isTitle(lang))
                {
                    potentialGenderMarker = true;
                    if (checkWord.isMaleTitle(lang))
                        gender = Male;
                    else
                        gender = Female;
                }
            }
            //tempWord = checkWord;
            hasComma = checkWord.removeEnding(QString(","));
            checkWord.removeEnding(PUNCTUATION);
            hasBookEnds = checkWord.removeBookEnds();
            isAboriginal = !hasComma && checkWord.isAboriginalName(checkStream.peekAtWord());
            tempWord = checkWord;
            while (checkWord.isCompoundName() || (isAboriginal && !hasBookEnds))
            {
                checkWord = checkStream.getWord().getString();
                hasComma = (checkWord.right(1) == OQString(","));
                tempWord += OQString(" ");
                tempWord += checkWord;
                isAboriginal = !hasComma && isAboriginal && checkWord.isAboriginalName(checkStream.peekAtWord());
            }
            checkWord = tempWord;
            nameType = globals->globalDr->isAName(checkWord);
            if ((nameType >= 1) && (nameType <= 3))
            {
                noMatch = false;
                if (potentialGenderMarker)
                    globals->globalDr->setGender(gender);
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
            bool isNickName = dbSearch.nicknameLookup(firstName, globals);
            if (isNickName)
                matched = OQString(firstName).isNickNameRelativeToList(nameList);
            if (matched)
                noMatch = false;
        }

        if (noMatch || (cutOff < 2))
            justInitialNamesUC.clear();
        else
            justInitialNamesUC.pickOffNames();  // From cleanAndExtract
    }

    // Replace justInitialNamesUC if appropriate
    if (justInitialNamesUC.countWords() < 2)
        justInitialNamesUC.clear();
    if ((justInitialNamesUC.getLength() == 0) && (ucFillerRemovedAndTruncated.getLength() > 1))
    {
        restrictNamesToDR = false;
        if (globals->globalDr->getProvider() == Legacy3000)
            UCsource = ucCleaned;
        else
            UCsource = ucFillerRemovedAndTruncated;

        UCsource.beg();
        int i = 1;
        while (!UCsource.isEOS() && (i <= 2))
        {
            tempSentence = UCsource.getNextRealSentence(restrictNamesToDR, 3);
            if (globals->globalDr->getProvider() == Legacy3000)
            {
                tempSentence.truncateAfter(OQString::getParentReferences(lang), (i == 1));
                tempSentence.truncateAfter(OQString::getSiblingReferences(lang), (i == 1));
                tempSentence.truncateAfter(OQString::getChildReferences(lang), (i == 1));
                tempSentence.truncateAfter(OQString::getRelativeReferences(lang), (i == 1));
                tempSentence.truncateAfter(OQString::getRelationshipWords(lang), (i == 1));
            }

            firstTwoSentences.addSentence(tempSentence);
            i++;
        }

        // If unsucessful, start again from first unrecognized word
        started = false;
        firstTwoSentences.beg();
        while (!firstTwoSentences.isEOS() && !started)
        {
            lastWord.clear();
            unrecognizedWordEncountered = false;
            invalidated = false;
            numWords = 0;

            tempSentence = firstTwoSentences.getSentence(lang);

            while (!unrecognizedWordEncountered && !invalidated && !tempSentence.isEOS() && (numWords < 20))
            {
                originalWord = tempSentence.getWord(true);

                checkWord = originalWord;
                checkWord.removeBookEnds();
                checkWord.cleanUpEnds();
                checkWord.removeEnding(PUNCTUATION);
                unrecognizedWordEncountered = !checkWord.isRecognized() && !globals->websiteLocationWords.contains(checkWord.lower().getString());
                numWords++;

                doubleWord = lastWord + OQString(" ") + checkWord;
                if (!globals->websiteLocationWords.contains(checkWord.lower().getString()))
                    lastWord = checkWord;
                else
                    lastWord.clear();
                if (doubleWord.areRelationshipWords(gender, lang))
                        invalidated = true;
            }

            NAMETYPE nameType = globals->globalDr->isAName(checkWord);
            if (((nameType >= 1) && (nameType <= 3)) || globals->globalDr->isANameVersion(checkWord))
            {
                started = true;
                justInitialNamesUC.clear();
                justInitialNamesUC += originalWord;
                while (!tempSentence.isEOS())
                {
                    originalWord = tempSentence.getWord(true);
                    if (originalWord == OQString("-."))
                        originalWord.removeEnding(".");
                    if (originalWord.getLength() > 2)
                        originalWord.removeEnding(".");
                    if (originalWord != OQString(","))
                        justInitialNamesUC += OQString(" ");
                    justInitialNamesUC += originalWord;
                }
                justInitialNamesUC.pickOffNames();  // From first unrecognized word
            }
            else
            {
                bool tryNextWord = dbSearch.givenNameLookup(checkWord.lower().getString(), globals, gender);
                if (tryNextWord)
                {
                    originalWord = tempSentence.getWord(true);
                    originalWord.removeEnding(PUNCTUATION);
                    checkWord = originalWord;
                    checkWord.removeBookEnds();
                    if (checkWord.isWrittenMonth())
                    {
                        nextWord = tempSentence.peekAtWord(false, 1);
                        nextWord.removeEnding(PUNCTUATION);
                        if (nextWord.isNumeric() || nextWord.removeOrdinal())
                            checkWord.clear();
                    }
                    nameType = globals->globalDr->isAName(checkWord);
                    if ((nameType >= 1) && (nameType <= 3))
                    {
                        started = true;
                        justInitialNamesUC.clear();
                        justInitialNamesUC += lastWord + OQString(" ");
                        justInitialNamesUC += originalWord;
                        while (!tempSentence.isEOS())
                        {
                            originalWord = tempSentence.getWord(true);
                            originalWord.removeEnding(".");
                            if (originalWord != OQString(","))
                                justInitialNamesUC += OQString(" ");
                            justInitialNamesUC += originalWord;
                        }
                        justInitialNamesUC.pickOffNames();  // From first unrecognized word
                    }
                }
            }
        }
    }

    // If still unsucessful, determine start from first saved name
    if (justInitialNamesUC.countWords() < 2)
        justInitialNamesUC.clear();
    if ((justInitialNamesUC.getLength() == 0) && (firstTwoSentences.getLength() > 1))
    {
        started = false;
        firstTwoSentences.beg();

        while (!firstTwoSentences.isEOS() && (justInitialNamesUC.countWords() < 2))
        {
            tempSentence = firstTwoSentences.getSentence(lang);
            justInitialNamesUC.clear();
            originalWord.clear();
            wordBefore1.clear();
            wordBefore2.clear();
            invalidated = false;
            hadComma = false;
            wordCount = 0;

            while (!tempSentence.isEOS() && !invalidated)
            {
                wordBefore2 = wordBefore1;
                if ((hadComma) && !(globals->globalDr->isASavedName(word) || globals->globalDr->isASimilarName(word)))
                    wordBefore1.clear();
                else
                    wordBefore1 = originalWord;
                wordBefore1.removeEnding(PUNCTUATION);
                originalWord = tempSentence.getWord(true);
                wordCount++;
                word = originalWord;
                hadComma = word.removeEnding(",");
                word.removeEnding(PUNCTUATION);
                word.removePossessive();
                word.removeBookEnds(PARENTHESES | QUOTES);
                doubleWord = wordBefore1 + OQString(" ") + word;
                if (doubleWord.areRelationshipWords(gender, lang))
                {
                    if (!((wordCount < 7) && (wordBefore1.lower() == OQString("surrounded"))))
                        invalidated = true;
                }
                if (!started && !invalidated)
                {
                    started = globals->globalDr->isASavedName(word) || globals->globalDr->isANickName(word) || word.isTitle(lang, gender) ;
                    if (started)
                    {
                        bool abandon = false;
                        if (globals->globalDr->isALastName(word) && !(globals->globalDr->isASimilarName(wordBefore1) || globals->globalDr->isASimilarName(wordBefore2)))
                            abandon = true;
                        if (!abandon)
                        {
                            if (wordBefore2.isCapitalized() && dbSearch.givenNameLookup(wordBefore2.getString(), globals, gender) && !wordBefore1.isRecognized())
                                justInitialNamesUC += wordBefore2 + OQString(" ");
                            if ((justInitialNamesUC.getLength() > 0) || (wordBefore1.isCapitalized() && dbSearch.givenNameLookup(wordBefore1.getString(), globals, gender)))
                                justInitialNamesUC += wordBefore1 + OQString(" ");
                        }
                    }
                }
                if (started)
                    justInitialNamesUC += originalWord + OQString(" ");
            }
            if (!started && invalidated)
                justInitialNamesUC.clear();
            else
                justInitialNamesUC.pickOffNames();  // From first saved name
        }
    }

    // If still nothing, special coding for Great West
    if (((justInitialNamesUC.getLength() == 0) || (justInitialNamesUC.countWords() == 1)) && (globals->globalDr->getProvider() == GreatWest))
    {
        justInitialNamesUC.clear();
        justInitialNamesUC = globals->globalObit->getStructuredNamesProcessed();
    }

    // If still nothing, special coding for Legacy3000
    if (((justInitialNamesUC.getLength() == 0) || (justInitialNamesUC.countWords() <= 2)) && (globals->globalDr->getProvider() == Legacy3000))
    {
        if ((globals->globalDr->getFirstName().getLength() == 0) && ((justInitialNamesUC.countFirstNames() == 0) || (justInitialNamesUC.countWords() <= 1)))
        {
            // Only a last name is known, so pull first names from obitSnippet
            justInitialNamesUC.clear();
            started = false;
            firstTwoSentences.beg();
            while (!firstTwoSentences.isEOS() && !started)
            {
                lastWord.clear();
                unrecognizedWordEncountered = false;
                invalidated = false;
                numWords = 0;

                tempSentence = firstTwoSentences.getSentence(lang);

                while (!unrecognizedWordEncountered && !invalidated && !tempSentence.isEOS() && (numWords < 20))
                {
                    originalWord = tempSentence.getWord(true);

                    checkWord = originalWord;
                    checkWord.removeBookEnds();
                    checkWord.cleanUpEnds();
                    checkWord.removeEnding(PUNCTUATION);
                    numWords++;

                    if (checkWord.isCapitalized())
                    {
                        unrecognizedWordEncountered = !checkWord.isRecognized() && !globals->websiteLocationWords.contains(checkWord.lower().getString());

                        if (unrecognizedWordEncountered)
                        {
                            dbSearch.nameStatLookup(checkWord.getString(), globals, nameStats, gender);
                            if (nameStats.isLikelyGivenName)
                            {
                                globals->globalDr->setFirstName(checkWord);

                                started = true;
                                justInitialNamesUC.clear();
                                justInitialNamesUC += originalWord;
                                while (!tempSentence.isEOS())
                                {
                                    originalWord = tempSentence.getWord(true);
                                    if (originalWord == OQString("-."))
                                        originalWord.removeEnding(".");
                                    if (originalWord.getLength() > 2)
                                        originalWord.removeEnding(".");
                                    if (originalWord != OQString(","))
                                        justInitialNamesUC += OQString(" ");
                                    justInitialNamesUC += originalWord;
                                }
                                justInitialNamesUC.pickOffNames();  // From first unrecognized word
                            }
                            else
                                unrecognizedWordEncountered = false;
                        }
                    }
                }
            }
        }
    }

    // If still nothing, pull names from titleStream
    if ((justInitialNamesUC.getLength() == 0) || (justInitialNamesUC.countWords() == 1))
    {
        justInitialNamesUC.clear();

        OQStream word;
        bool keepGoing = true;
        bool started = false;
        OQStream tempStream(globals->globalDr->getTitle());
        tempStream.fixBasicErrors();
        tempUC.compressCompoundNames(tempStream, lang);
        tempStream.beg();

        while (!tempStream.isEOS() && keepGoing)
        {
            word = tempStream.getWord();
            if (word == globals->globalDr->getTitleKey())
                started = true;
            if ((word == OQString("|")) || (word == OQString("-")) || (word.lower() == OQString("obituary")))
                keepGoing = false;
            if (started && keepGoing)
            {
                justInitialNamesUC += word;
                justInitialNamesUC += OQString(" ");
            }
        }
        justInitialNamesUC.pickOffNames();  // From title stream
    }

    justInitialNamesUC.removeCelebrations();
    justInitialNamesUC.removeEnding(".");
    justInitialNamesUC.removeAllSuffixPrefix();
    justInitialNamesUC.fixBasicErrors();
    justInitialNamesUC.removeIntroductions();

    // Adjust for "Smith: John Harry"
    if (justInitialNamesUC.contains(":") && !justInitialNamesUC.contains(","))
        justInitialNamesUC.replace(":", ",");
}

QStringList readObit::getStopWords()
{
    return globals->websiteLocationWords;
}

QStringList readObit::getLocationWords(PROVIDER provider, unsigned int providerKey)
{
    QStringList locations;
    QStringList genericLocations = QString("north|south|east|west|new|nord|sud|est|ouest|nouveau|northern|southern|eastern|western|upper|lower").split("|");
    genericLocations += QString("arm|bay|branch|brook|cove|creek|falls|fort|grove|harbour|hill|house|lake|meadows|mountain|park|pier|point|port|ridge|river|shore|sound|valley").split("|");
    genericLocations += QString("care|chapel|home").split("|");
    genericLocations += provAbbreviations;
    genericLocations += provLong;

    switch(provider)
    {
    case 2:
    {
        switch(providerKey)
        {
        case 1:
            locations = QString("swan").split(QString("|"));
            break;

        default:
            locations = QString("manitoba").split(QString("|"));
            break;
        }
    }
        break;

    case 6:
    {
        QList<QString> tempLocations;
        switch(providerKey)
        {
        case 45:
            tempLocations.append(QString("dekebaowek"));
            tempLocations.append(QString("dekipawa"));
            tempLocations.append(QString("demême"));
            tempLocations.append(QString("detémiscaming"));
            tempLocations.append(QString("dethorne"));
            tempLocations.append(QString("dewinneway"));
            break;

        default:
            break;
        }

        while (tempLocations.size() > 0)
        {
            locations.append(tempLocations.takeFirst());
        }
    }
        break;

    case 11:
    {
        switch(providerKey)
        {
        case 10:
            locations = QString("barrie|street|st.vincent").split("|");
            break;

        case 11:
        case 15:
            locations = QString("guelph|loring|mines|sydney").split(QString("|"));
            break;

        default:
            break;
        }
    }
        break;

    case 14:
        locations =  QString("abercrombie|canard|gardiner|glace|halifax|hillside|howie|kentville|mines|morien|mountain|new|niagara|ottawa|reserve").split(QString("|"));
        locations += QString("sackville|stratford|summerside|sydney|truro|waterford|whitney|winsloe").split(QString("|"));
        break;

    case 15:
    {
        switch(providerKey)
        {
        case 2:
            locations = QString("campbell").split(QString("|"));
            break;

        case 5:
            locations = QString("maple").split(QString("|"));
            break;

        case 22:
            locations = QString("langley").split(QString("|"));
            break;

        case 27:
            locations = QString("langley").split(QString("|"));
            break;

        case 64:
            locations = QString("nelson|star").split(QString("|"));
            break;

        case 68:
        case 73:
            locations = QString("vernon|morningstar|salmon|arm").split(QString("|"));
            break;

        case 71:
            locations = QString("the|free|press").split(QString("|"));
            break;
        }
    }
        break;

    case 1000:
    {
        switch(providerKey)
        {
        case 10973:
            locations = QString("norris").split(QString("|"));
            break;

        default:
            break;
        }
    }
        break;

    case 1003:
    {
        switch(providerKey)
        {
        case 3758:
            locations = QString("halifax").split(QString("|"));
            break;

        case 3190:
            locations = QString("cape|breton|london").split(QString("|"));
            break;

        case 3198:
        case 3842:
            locations = QString("campbellton|charlo|dalhousie|decampbellton|decharlo|dedalhousie|dedundee|dundee").split(QString("|"));
            break;

        default:
            break;
        }
    }
        break;

    case 1005:
    {
        switch(providerKey)
        {
        case 824:
            locations = QString("aconi|adler|bevis|little|mill|mines|pond|sydney").split(QString("|"));
            break;

        case 6694:
            locations = QString("halifax|sydney").split(QString("|"));
            break;

        case 8286:
            locations = QString("cowley|high|millarville|nanton|okotoks|pincher").split(QString("|"));
            break;

        case 10307:
            locations = QString("ottawa").split(QString("|"));
            break;

        case 10338:
            locations = QString("perry|uxbridge").split(QString("|"));
            break;

        case 10776:
            locations = QString("manitoulin").split(QString("|"));
            break;

        case 10900:
            locations = QString("casselman|decasselman|deembrun|embrun").split(QString("|"));
            break;

        case 12429:
            locations = QString("cornwall|covehead|hampshire|stratford|vernon|winsloe").split(QString("|"));
            break;

        default:
            break;
        }
    }
        break;

    case 1006:
    {
        switch(providerKey)
        {
        case 60097:
            locations = QString("black|brocket|burmis|calgary|cowley|diamond|eden|foothills|high|mcleod|millarville|nanton|pincher|turner|vulcan").split(QString("|"));
            break;

        default:
            break;
        }
    }
        break;

    case 1008:
    {
        switch(providerKey)
        {
        case 18:
            locations = QString("three|brooks").split(QString("|"));
            break;

        case 82:
            locations = QString("dawson").split(QString("|"));
            break;

        case 84:
            locations = QString("radium").split(QString("|"));
            break;

        case 88:
            locations = QString("winnipeg").split(QString("|"));
            break;

        case 94:
            locations = QString("caribou|central|hardwood|kentville|pictou|plymouth|scotch|stellarton").split(QString("|"));
            break;

        case 95:
            locations = QString("pictou").split(QString("|"));
            break;

        case 96:
            locations = QString("dartmouth|halifax").split(QString("|"));
            break;

        case 104:
            locations = QString("blanc|corner|deer|drive|duck|grand|mainland|sablon|safe|st.david's|st.george's").split(QString("|"));
            break;

        case 112:
            locations = QString("brockville|charles").split(QString("|"));
            break;

        case 145:
            locations = QString("birch|house|loch|lomond").split(QString("|"));
            break;

        case 154:
            locations = QString("toronto").split(QString("|"));
            break;

        case 156:
            locations = QString("banwell").split(QString("|"));
            break;

        case 179:
            locations = QString("ottawa").split(QString("|"));
            break;

        case 194:
            locations = QString("ottawa").split(QString("|"));
            break;

        default:
            break;
        }
        break;
    } // end 1008
        break;

    case 1010:
    {
        switch(providerKey)
        {
        case 6:
            locations = QString("saskatoon").split(QString("|"));
            break;

        case 34:
            locations = QString("anson").split(QString("|"));
            break;

        case 54:
            locations = QString("celebration|centre|chapel|chatham|kingsville|wheatley").split(QString("|"));
            break;

        case 57:
            locations = QString("lock|down").split(QString("|"));
            break;

        case 63:
            locations = QString("aurora").split(QString("|"));
            break;

        case 132:
            locations = QString("sydney|mines").split(QString("|"));
            break;

        case 2836:
            locations = QString("thunder|bay|thunderbay").split(QString("|"));
            break;

        case 255100:
            locations = QString("toronto").split(QString("|"));
            break;

        default:
            break;
        }
    } // end 1010
        break;

    case 1011:
    {
        switch(providerKey)
        {
        case 38134:
            locations = QString("acton|ashby|ayer|bolton|grafton|littleton|newton|pepperell|townsend").split(QString("|"));
            break;

        default:
            break;
        }
    } // end 1011
        break;

    case 1021:
    {
        switch(providerKey)
        {
        case 93:
            locations = QString("sturgeon").split(QString("|"));
            break;

        default:
            break;
        }
    } // end 1021
        break;

    case 1033:
    {
       locations = QString("st.boniface").split(QString("|"));
    } // end 1033
        break;

    case 1034:
    {
       locations = QString("sydney|whitney").split(QString("|"));
    } // end 1034
        break;

    case 1035:
    {
        switch(providerKey)
        {
        case 1:
            locations = QString("brierly|johnstown|antigonish").split(QString("|"));
            break;

        case 34:
            locations = QString("grand|falls").split(QString("|"));
            break;

        default:
            break;
        }
    } // end 1035
        break;

    case 1040:
    {
       locations = QString("belleville").split(QString("|"));
    } // end 1040
        break;

    case 1045:
    {
       locations = QString("campbell").split(QString("|"));
    } // end 1045
        break;

    case 1117:
    {
       locations = QString("lamaison").split(QString("|"));
    } // end 1117
        break;

    case 2003:
    {
        locations = QString("ab|alberta").split(QString("|"));
        break;
    }   // end 2003
        break;

    case 2031:
    {
        locations = QString("cornwall|hampshire|rose|stratford|vernon|warren|winsloe").split(QString("|"));
        break;
    }   // end 2003
        break;

    default:
        break;
    }

    return locations + genericLocations;
}

void readObit::removeProblematicWordsFromJIN()
{
    LANGUAGE lang = globals->globalDr->getLanguage();
    PROVIDER provider = globals->globalDr->getProvider();
    unsigned int providerKey = globals->globalDr->getProviderKey();

    QStringList badWords;
    QString badProv;
    QString newString;

    bool replace = false;

    badWords = globals->websiteLocationWords;

    switch(provider)
    {
    case 1008:
    {
        switch(providerKey)
        {
        case 94:
            badProv = QString(", nl");
            break;

        case 104:
            badProv = QString(", nl");
            break;

        default:
            break;
        }
        break;
    } // end 1008

    default:
        break;
    }

    if ((badProv.length() > 0) && justInitialNamesUC.getString().contains(badProv, Qt::CaseInsensitive))
        replace = true;

    if (!replace && badWords.size() > 0)
    {
        OQStream tempStream(justInitialNamesUC.getString());
        OQString tempWord;
        newString.clear();

        while (!replace && !tempStream.isEOS())
        {
            tempWord = tempStream.getWord();
            if (tempWord.isSaint())
                tempWord += tempStream.getWord();
            replace = badWords.contains(tempWord.lower().getString());
            if (!replace)
            {
                if (newString.length() > 0)
                    newString += QString(" ");
                newString += tempWord.getString();
            }
        }
    }

    if (replace)
    {
        switch(provider)
        {
        case 1008:
        {
            switch(providerKey)
            {
            default:
                // Drop words after first match
                justInitialNamesUC.clear();
                justInitialNamesUC = newString;
                break;

            case 104:
                justInitialNamesUC.clear();
                justInitialNamesUC = uc.getSentence(lang, 1);
                OQString tempWords = uc.getSentence(lang, 2);
                if ((justInitialNamesUC.countWords() <= 3) && (tempWords.countWords(SPACE, ALPHANUMERIC) <= 2) && (tempWords.right(2).lower() != QString("nl")))
                {
                    justInitialNamesUC += OQString(" ");
                    justInitialNamesUC += tempWords;
                }
                break;
            }
            break;
        } // end 1008

        default:
            break;
        }
    }
}

void readObit::readUnstructuredContent(bool UseFirstDateAsDOD)
{
    LANGUAGE lang = globals->globalDr->getLanguage();
    GENDER workingGender = globals->globalDr->getGender();
    if (workingGender == genderUnknown)
        workingGender = globals->globalDr->getWorkingGender();
    QList<QString> listOfNames;
    databaseSearches dbSearch;

    // STEP 1 - Fill in names

    // Look for most common name used within the obituary
    globals->globalDr->setAlternates(mcn.readMostCommonName(globals, globals->structuredNamesProcessed->getString()));

    // Turn to initial unstructured text
    validateJustInitialNames();
    removeProblematicWordsFromJIN();

    if (workingGender == genderUnknown)
    {
        double unisex = 0.5;
        listOfNames = globals->globalDr->getGivenNameList(globals);
        if (listOfNames.count() > 0)
            unisex = dbSearch.genderLookup(listOfNames, globals);
        if (unisex >= 0.9)
            workingGender = Male;
        else
        {
            if (unisex <= 0.1)
                workingGender = Female;
        }
    }

    // Look for alternate first and last names
    if (justInitialNamesUC.getLength() > 0)
    {
        justInitialNamesUC.setContentLanguage(lang);
        globals->globalDr->setAlternates(justInitialNamesUC.readAlternates(PARENTHESES | QUOTES, false, workingGender));
    }
    globals->globalDr->setAlternates(ucFillerRemovedAndTruncated.readAlternates(PARENTHESES | QUOTES, true, workingGender));

    // Look for middle name(s), if any, as a single string and sets other names along the way if they are encountered
    globals->globalDr->setMiddleNames(justInitialNamesUC.processAllNames());

    // Where middlename is used as a first name, add it as an AKA as well
    PQString tempName;
    tempName = globals->globalDr->getMiddleNameUsedAsFirstName();
    if (tempName.getLength() > 0)
        globals->globalDr->setFirstNames(tempName);

    // Sort firstNames to have formal ones first
    globals->globalDr->sortFirstNames();

    // STEP 2 - Fill in gender
    addressUnknownGender();

    // STEP 3 - Fill in dates
    // First three passes get run whether dates exist or not as high level info occasionally wrong (obit itself more reliable)

    // First pass will see if pure complete dates are at front of string
    // Second pass will look at just years at front (eg. 1966 - 2016)
    // Third pass will review one sentence at a time for dates and key words (high credibility for first two sentences only)
    // Forth pass - Try to find DOB or YOB (different actions depending how far match occurs)
    // Fifth pass is a deeper dive (full content by sentence) looking for two consecutive dates, with potential messages generated
    // Sixth pass is to look for age at death (in first two sentences only)
    // Seventh pass is to look for the first word in the cleanedUpUC to be a number OR other short sentence with a number.
    // Eighth pass looks for any two dates in first three sentences, but only keeps if DODs available and match
    // 8.5 pass looks for age next birthday
    // Ninth pass includes final attempts to pull incomplete information together
    // Tenth pass looks at naked age at death
    // Eleventh pass looks at years married to impute maximum YOB
    // Use date published or funeral service date as proxy if still missing DOD
    // Finally, set min and max DOB if DOD is known and age at death is known

    fillInDatesFirstPass();
    fillInDatesSecondPass();
    fillInDatesThirdPass(uc);

    if (globals->globalDr->getYOB() == 0)
        fillInDatesFourthPass();
    if (missingDateInfo())
        fillInDatesFifthPass();
    if (globals->globalDr->missingDOB() || globals->globalDr->missingDOD() || (globals->globalDr->getDOB() == globals->globalDr->getDOD()))
        fillInDatesSixthPass();
    if (globals->globalDr->missingDOB() && !globals->globalDr->missingDOD())
        fillInDatesSeventhPass();
    if (globals->globalDr->missingDOB() && !globals->globalDr->missingDOD())
        fillInDatesEighthPass();
    if (globals->globalDr->missingDOB() && !globals->globalDr->missingDOD())
        fillInDates85Pass();  // New style using regex
    if (globals->globalDr->missingDOB() || globals->globalDr->missingDOD() || (globals->globalDr->getDOB() == globals->globalDr->getDOD()))
        fillInDatesNinthPass();
    if ((globals->globalDr->missingDOB() || globals->globalDr->missingDOD()) && (globals->globalDr->getAgeAtDeath() == 0))
        fillInDatesTenthPass();
    if (globals->globalDr->missingDOB() && (globals->globalDr->getYOD() > 0) && (globals->globalDr->getAgeAtDeath() == 0))
        fillInDatesEleventhPass();
    if (globals->globalDr->getYOB() == 0)
        fillInDatesTwelfthPass();
    if ((globals->globalDr->getYOD() == 0) && (globals->globalDr->getDeemedYOD() > 0))
        globals->globalDr->setYOD(globals->globalDr->getDeemedYOD());

    // Use a single as DOD if the only date found in the first sentence and within 60 days of run Date or publish date
    if (globals->globalDr->missingDOD())
    {
        if (useFirstSentenceSingleDate())
            fillInDatesThirdPass(ucCleaned);
    }

    // Make higher risk assumptions for specific groups where pattern of historical obits is problematic but where issues are reliably consistent
    if (globals->globalDr->missingDOD() && UseFirstDateAsDOD)
    {
        assignFirstDateToDOD();
        fillInDatesThirdPass(ucCleaned);
    }

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

    // STEP 4 - Read spouse's name
    ucCleaned.contentReadSpouseName(lang);
}

void readObit::readMessages()
{
    QDate commentDate, tempDate;
    QList<QDate> dateList;
    OQString cleanString;
    unstructuredContent sentence, tempUC;

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
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Burke:
    case FrontRunner:
        commentDate = globals->today;
        while (moveTo("class=\"tributePost-postDate\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
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
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Codesign:
        commentDate = globals->today;
        while (consecutiveMovesTo(25, "comment-date__time", ">"))
        {
            tempUC = getUntil(" at ");
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Crew:
        commentDate = globals->today;
        while (consecutiveMovesTo(200, "comment-meta commentmetadata", ">", ">"))
        {
            tempUC = getUntil(" at ");
            tempUC.cleanUpEnds();
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);

        if (!commentDate.isValid())
        {
            beg();
            if (consecutiveMovesTo(1000, ">We Remember<", "<img", "springfieldfuneralhome.com/wp-content/uploads/"))
            {
                int yyyy = getUntil("/").getString().toInt();
                forward(1);
                int mm = getUntil("/").getString().toInt();
                tempDate.setDate(yyyy, mm, 1);
                if (tempDate.isValid())
                    commentDate = tempDate;
            }
        }
        break;

    case Nirvana:
        commentDate = globals->today;
        while (consecutiveMovesTo(200, "class=\"date_tribute\"", "Date :", "</strong"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Bergetti:
        commentDate = globals->today;
        while (moveTo("class=\"comment_date_value\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case ROI:
        commentDate = globals->today;
        while (consecutiveMovesTo(50, "class=\"comment-meta\"", "Posted "))
        {
            tempUC = getUntil(" at ");
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Vernon:
        commentDate = globals->today;
        while (moveTo("class=\"comment_date\""))
        {
            moveTo("on ");
            tempUC = getUntil(" at ");
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Ashlean:
        commentDate = globals->today;
        while (consecutiveMovesTo(500, "class=\"ast-comment-time\"", "<time datetime", ">"))
        {
            tempUC = getUntil(" at ");
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Halifax:
    case Turner:
        commentDate = globals->today;
        while (moveTo("class=\"comment_date\""))
        {
            moveTo("on ");
            tempUC = getUntil(" at ");
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case ReneBabin:
        commentDate = globals->today;
        while (moveTo("class=\"comment_date_value\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Citrus:
        commentDate = globals->today;
        while (consecutiveMovesTo(300, "class=\"comment-submitted\"", "datetime=\""))
        {
            tempUC = getUntil("T");
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case SandFire:
        commentDate = globals->today;
        while (consecutiveMovesTo(750, "class=\"post-comment-content\"", "class=\"meta-data\">"))
        {
            tempUC = getUntil(" at ");
            tempUC.simplify();
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Carve:
        commentDate = globals->today;
        while (moveTo("class=\"datestamp\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.dropRight(8);
            tempUC.cleanUpEnds();
            tempUC.removeEnding(",");
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case ChadSimpson:
        commentDate = globals->today;
        while (consecutiveMovesTo(100, "class=\"text-xs block pb-3\"", " - "))
        {
            tempUC = getUntil("<");
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case MarketingImages:
        commentDate = globals->today;
        moveTo(">Condolences<");
        while (consecutiveMovesTo(30, "<li class=\"odd\">", "<p"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Eggs:
        commentDate = globals->today;
        while (consecutiveMovesTo(200, "id=\"div-comment-", "title=\"\">"))
        {
            tempUC = getUntil(" at ");
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case InView:
        commentDate = globals->today;
        while (moveTo("<time  >"))
        {
            tempUC = getUntil(" at ");
            tempUC.simplify();
            tempDate = tempUC.readDateField(doMDY);
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Elegant:
        commentDate = globals->today;
        while (moveTo("class=\"comment_date_value\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.simplify();
            tempDate = tempUC.readDateField(doDMY);
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Ubeo:
        commentDate = globals->today;

        switch(style)
        {
        case 0:
            while (moveTo("<span class=\"gold\">le "))
            {
                dateList.clear();
                tempUC = getUntil("<");
                tempUC.simplify();
                if (tempUC.pullOutDates(globals->globalDr->getLanguage(), dateList, 1, cleanString, true))
                {
                    if (dateList[0].isValid() && (tempDate < commentDate))
                        commentDate = dateList[0];
                }
            }
            break;

        case 1:
            break;
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
            if (tempDate.isValid() && (tempDate < commentDate))
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

    case Beechwood:
        commentDate = globals->today;
        while (consecutiveMovesTo(100, "<p>Submitted by", "</span>", ", "))
        {
            tempUC = getUntil(" -");
            tempDate = tempUC.readDateField(doMDY);
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Cardinal:
        commentDate = globals->today;
        while (moveTo("class=\"comment_date\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case FirstMemorial:
        commentDate = globals->today;
        while (consecutiveMovesTo(200, "class=\"comment-author meta\"", "</strong>"))
        {
            tempUC = getUntil(" at ");
            tempUC.simplify();
            tempDate = tempUC.readDateField(doMDY);
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case RHB:
        commentDate = globals->today;
        while (moveTo("class=\"contentDate\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.simplify();
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Rhody:
        commentDate = globals->today;
        while (consecutiveMovesTo(20, "wpd-comment-date", "title="))
        {
            tempUC = readNextBetween(QUOTES);
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Bridge:
        commentDate = globals->today;
        while (moveTo("class=\"comment-date\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempDate = tempUC.readDateField(doYMD);
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Affordable:
        commentDate = globals->today;
        while (consecutiveMovesTo(250, "class=\"comment-time-link\"", ">"))
        {
            tempUC = getUntil(" at ");
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case LifeTransitions:
        commentDate = globals->today;
        while (moveTo("class=\"tributePost-postDate\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Davis:
    {
        OQString unit;
        int days, num;

        commentDate = globals->today;
        end();
        moveBackwardTo("class=\"sttime\">", 100000);
        num = getWord().asNumber();
        if ((num > 0) && (num <= 31))
        {
            unit = getWord();
            unit.removeEnding("s");
            if (unit == QString("month"))
                days = 30 * num;
            else
            {
                if (unit == QString("week"))
                    days = 7 * num;
                else
                {
                    if (unit == QString("day"))
                        days = num;
                    else
                        days = 0;
                }
            }
            commentDate = commentDate.addDays(-days);
        }
    }
        break;

    case London:
        commentDate = globals->today;
        while (consecutiveMovesTo(750, "class=\"comment-author vcard\"", "datetime=\""))
        {
            tempUC = getUntil("T");
            tempDate = tempUC.readDateField(doYMD);
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Lampman:
        commentDate = globals->today;
        while (consecutiveMovesTo(750, "class=\"comment-metadata\"", "datetime=\""))
        {
            tempUC = getUntil("T");
            tempDate = tempUC.readDateField(doYMD);
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Ranger:
        commentDate = globals->today;
        while (moveTo("class=\"commentDate\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case Komitas:
        commentDate = globals->today;
        while (consecutiveMovesTo(150, "class=\"komi-header\"", "<span"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    case MLBW:
        commentDate = globals->today;
        while (consecutiveMovesTo(1000, "id=\"plume-temoignage\"", ">Date "))
        {
            tempUC = getUntil("<");
            tempDate = tempUC.readDateField();
            if (tempDate.isValid() && (tempDate < commentDate))
                commentDate = tempDate;
        }
        if (commentDate == globals->today)
            commentDate.setDate(0,0,0);
        break;

    }

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
        switch (style)
        {
        case 0:
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

        case 1:
            if (moveTo("\"datePublished\": \""))
            {
                tempUC = getUntil("T");
                pubDate = tempUC.readDateField(doYMD);
            }
            break;

        case 2:
            if (consecutiveMovesTo(25, "publishedDate", ":"))
            {
                tempUC = readNextBetween(QUOTES);
                pubDate = tempUC.readDateField(doYMD);
            }

            break;
        }

        break;

    case Passages:
        if (consecutiveMovesTo(100, ">As published in", " on "))
        {
            tempUC = getUntil("<");
            if (tempUC.getLength() > 0)
            {
                tempUC.simplify();
                tempUC.replaceHTMLentities();
                pubDate = tempUC.readDateField();
            }
        }
        break;

    case CFS:
    case WPBakery:
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

    case FuneralOne:
        if (consecutiveMovesTo(100, "class=\"mt-0\"", "Posted", "datetime=\""))
        {
            tempUC = getUntil("T");
            pubDate = tempUC.readDateField(doYMD);
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

    case RedSands:
        if (consecutiveMovesTo(100,"itemprop=\"datePublished\"", "content="))
        {
            tempUC = readNextBetween(QUOTES);
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

    case Eggs:
        if (consecutiveMovesTo(100, "article:published_time", "content=\""))
        {
            tempUC = getUntil("T");
            tempUC.removeHTMLtags();
            tempUC.simplify();
            pubDate = tempUC.readDateField();
        }
        break;

    case ONeil:
        if (moveTo("'ObitPublishDate':"))
        {
            tempUC = readNextBetween(QUOTES);
            pubDate = tempUC.readDateField(doMDY);
        }
        break;

    case InView:
        if (consecutiveMovesTo(100, "article:published_time", "content=\""))
        {
            tempUC = getUntil("T");
            pubDate = tempUC.readDateField(doYMD);
        }
        break;

    case RKD:
        if (moveTo("datePublished\":\""))
        {
            tempUC = getUntil("T");
            pubDate = tempUC.readDateField(doYMD);
        }
        break;

    case FRM:
        switch(style)
        {
        case 0:
        case 1:
        case 3:
            if (consecutiveMovesTo(100, "article:published_time", "content=\""))
            {
                tempUC = getUntil("T");
                pubDate = tempUC.readDateField(doYMD);
            }
            break;

        case 2:
        case 4:
            if (moveTo("datePublished\":\""))
            {
                tempUC = getUntil("T");
                pubDate = tempUC.readDateField(doYMD);
            }
            break;
        }

        break;

    }

    if (pubDate.isValid())
        globals->globalDr->setPublishDate(pubDate);

}

void readObit::readServiceDate()
{
    int numSentences = uc.getNumSentences();
    int numChecked, numDates, maxCheck, forceYear;
    unstructuredContent sentence;
    bool dateFound = false;
    LANGUAGE lang;
    QList<QDate> dateList;
    OQString cleanString;

    QDate cutOffDate = globals->globalDr->getDOD();
    if (!cutOffDate.isValid() && (globals->globalDr->getYOD() > 0))
        cutOffDate = QDate(globals->globalDr->getYOD(), 1, 1);
    if (cutOffDate.isValid())
        forceYear = cutOffDate.year();
    else
    {
        cutOffDate = QDate(2000, 1, 1);
        forceYear = 0;
    }

    if (numSentences != -1)
    {
        numChecked = 1;
        lang = globals->globalDr->getLanguage();
        if (numSentences >= 9)
            maxCheck = static_cast<int>(numSentences / 2);
        else
            maxCheck = numSentences;
        uc.beg();
        while ((numChecked <= maxCheck) && (numSentences > 0) && !dateFound)
        {
            sentence = ucCleaned.getSentence(lang, numSentences);
            numDates = sentence.pullOutDates(lang, dateList, 2, cleanString, false, true, forceYear);
            if ((numDates == 1) || ((numDates == 2) && (dateList[0] == dateList[1])))
            {
                dateFound = true;
                if (dateList[0] >= cutOffDate)
                    globals->globalDr->setDOS(dateList[0]);
            }
            else
            {
                if (numDates == 2)
                {
                    int diff = dateList[1].toJulianDay() - dateList[0].toJulianDay();
                    if ((diff <= 7) && (diff >= -7))
                    {
                        dateFound = true;
                        if (diff > 0)
                        {
                            if (cutOffDate <= dateList[0])
                                globals->globalDr->setDOS(dateList[0]);
                        }
                        else
                        {
                            if (cutOffDate <= dateList[1])
                                globals->globalDr->setDOS(dateList[1]);
                        }
                    }
                    else
                        numSentences = 0;
                }
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
    uc.replaceHTMLentities();
    uc.removeHTMLtags();
    uc.unQuoteHTML();
    uc.simplify();
    uc.fixBasicErrors();
    uc.cleanUpEnds();
    uc.removeBookEnds(PARENTHESES);
    uc.fixDateFormats();
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
    unstructuredContent sentence;
    LANGUAGE lang = globals->globalDr->getLanguage();

    ucCleaned.beg();
    int i = 0;
    while ((i == 0) || (!sentence.isEOS() && (sentence.isJustNames() || (sentence.getLength() == 0))))
    {
        i++;
        sentence = ucCleaned.getSentence(lang, i);
        sentence.removeBookEnds(PARENTHESES);
    }

    bool limitWords = true;
    dates = sentence.readDates(globals->globalDr->getLanguage(), limitWords);
    if (dates.potentialDOB.isValid() || dates.potentialDOD.isValid())
        dates.fullyCredible = true;
    else
    {
        // Try one more sentence in limited circumstances
        if (sentence.countWords() <= 6)
        {
            i++;
            sentence = ucCleaned.getSentence(lang, i);
            sentence.removeBookEnds(PARENTHESES);
            dates = sentence.readDates(globals->globalDr->getLanguage(), limitWords);
            if (dates.potentialDOB.isValid() || dates.potentialDOD.isValid())
                dates.fullyCredible = true;
        }
    }

    processNewDateInfo(dates, 1);
}

void readObit::fillInDatesSecondPass()
{
    // Looks for two complete years at beginning of content (YYYY - YYYY)
    DATES dates;
    unstructuredContent sentence;
    LANGUAGE lang = globals->globalDr->getLanguage();

    ucCleaned.beg();
    int i = 0;
    while ((i == 0) || (!sentence.isEOS() && (sentence.isJustNames() || (sentence.getLength() == 0))))
    {
        i++;
        sentence = ucCleaned.getSentence(lang, i);
        sentence.removeBookEnds(PARENTHESES);
    }

    dates = sentence.readYears();
    if (dates.potentialYOB > 0 || dates.potentialYOD > 0)
    {
        dates.fullyCredible = true;
        if (globals->globalDr->getDOB().isValid() && (globals->globalDr->getDOB().year() != dates.potentialYOB))
            dates.fullyCredible = false;
        if (globals->globalDr->getDOD().isValid() && (globals->globalDr->getDOD().year() != dates.potentialYOD))
            dates.fullyCredible = false;
    }
    else
    {
        // Try one more sentence in limited circumstances
        if (sentence.countWords() <= 6)
        {
            i++;
            sentence = ucCleaned.getSentence(lang, i);
            sentence.removeBookEnds(PARENTHESES);
            dates = sentence.readYears();
            if (dates.potentialYOB > 0 || dates.potentialYOD > 0)
                dates.fullyCredible = true;
        }
    }

    processNewDateInfo(dates, 2);

    if ((globals->globalDr->getYOB() == 0) || (globals->globalDr->getYOD() == 0))
    {
        QRegularExpression target;
        target.setPattern("\\. \\(?(\\d{4})\\s?-\\s?(202\\d)\\)?\\.");
        QRegularExpressionMatch match = target.match(uc.getString());

        if (match.hasMatch())
        {
            int potentialYOB = match.captured(1).toInt();
            int potentialYOD = match.captured(2).toInt();

            if ((potentialYOB >= 1900) && (potentialYOB <= globals->today.year()))
                globals->globalDr->setYOB(potentialYOB);
            if ((potentialYOD >= 2020) && (potentialYOD <= globals->today.year()))
                globals->globalDr->setYOD(potentialYOD);
        }
    }
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

    dates = ucCleaned.contentReadBornYear(globals->globalDr->getLanguage());
    processNewDateInfo(dates, 4);
}

void readObit::fillInDatesFifthPass()
{
    // Look for two consecutive dates in any one sentence
    DATES dates;

    dates = ucCleaned.contentPullBackToBackDates(globals->globalDr->getLanguage());
    processNewDateInfo(dates, 5);
}

void readObit::fillInDatesSixthPass()
{
    // Look for age at death (in first two sentences only)
    // If a valid age at death (over 15) is located and a discrepancy in DODs is found (title/header vs text), DOD in text is used and dataRecord is updated
    unsigned int maxSentences = 2;
    ucCleaned.contentReadAgeAtDeath(maxSentences);
}

void readObit::fillInDatesSeventhPass()
{
    unstructuredContent sentence;
    PQString word;
    bool keepGoing = true;
    bool ageFound = false;
    bool restrictNamesToDR = true;

    // Look for the first "new word" in the first unrecognized sentence to be a number

    if (globals->globalDr->missingDOB() && !globals->globalDr->missingDOD() && (globals->globalDr->getAgeAtDeath() == 0))
    {
        ucCleaned.beg();
        sentence = ucCleaned.getNextRealSentence(restrictNamesToDR);
        /*while (!ucCleaned.isEOS() && ((sentence.getLength() == 0) || sentence.isJustDates() || sentence.isJustSavedNames() || sentence.startsWithClick(true)))
        {
            sentence = ucCleaned.getSentence();
        }*/

        sentence.beg();
        while (keepGoing && !sentence.isEOS())
        {
            keepGoing = false;
            word = sentence.getWord();
            word.removeBookEnds(PARENTHESES);
            word.removeEnding(PUNCTUATION);

            if (word.isHyphenated() || (word.right(1) == PQString("-")))
                word = word.postHyphen();

            if (globals->globalDr->isASavedName(word))
                keepGoing = true;

            if (word.lower() == PQString("age"))
                keepGoing = true;
        }

        if (word.isNumeric() && !word.isHyphenated())
        {
            // Eliminate possibility of a french date
            bool frenchDate = false;
            LANGUAGE lang = globals->globalDr->getLanguage();
            if ((lang == french) || (lang == multiple))
            {
                OQString nextWord = sentence.peekAtWord();
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
                    ageFound = true;
                }
            }
        }
    }

    if (!ageFound && (globals->globalDr->getAgeAtDeath() == 0))
        fillInDatesSeventhPassFollowUp();
}

void readObit::fillInDatesSeventhPassFollowUp()
{
    // Approach is to eliminate basic common words and if only number remains it is age at death

    unstructuredContent sentence;
    QString Qsentence;
    OQString word, peek1, peek2, peek3;
    int sentenceCount;
    GENDER gender;
    bool missingGender, potentialMatch, potentialMismatch;
    bool keepGoing = true;
    bool restrictNamesToDR = true;
    bool precedingFlag;

    QRegularExpression targetS;
    QRegularExpressionMatch match;

    gender = globals->globalDr->getGender();
    if (gender == genderUnknown)
        missingGender = true;
    else
        missingGender = false;

    if (globals->globalDr->getAgeAtDeath() == 0)
    {
        ucCleaned.beg();
        sentenceCount = 0;

        while (!ucCleaned.isEOS() && (sentenceCount < 5) && keepGoing)
        {
            sentence = ucCleaned.getNextRealSentence(restrictNamesToDR, 4);
            /*sentence = ucCleaned.getSentence();
            while (!ucCleaned.isEOS() && ((sentence.getLength() == 0) || sentence.isJustDates() || sentence.isJustSavedNames() || sentence.startsWithClick(true))){
                sentence = ucCleaned.getSentence();}*/

            if (sentence.getLength() > 0)
            {
                sentenceCount++;
                precedingFlag = false;

                // Look for preceding age indications
                sentence.beg();
                while (!sentence.isEOS() && !precedingFlag)
                {
                    word = sentence.getWord();
                    peek1 = sentence.peekAtWord(false, 1);
                    peek2 = sentence.peekAtWord(false, 2);
                    peek3 = sentence.peekAtWord(false, 3);
                    precedingFlag = word.followedByPrecedingIndicators(peek1, peek2, peek3);
                }

                if (precedingFlag)
                    sentence.removeStrings(precedingIndicators);

                // Check for pronouns
                Qsentence = sentence.getString();
                potentialMatch = false;
                potentialMismatch = false;

                targetS.setPattern("\\bHe\\b");
                match = targetS.match(Qsentence);
                if (match.hasMatch())
                {
                    Qsentence.replace(targetS, "");
                    if (gender == Female)
                        potentialMismatch = true;
                    if (gender == genderUnknown)
                        potentialMatch = true;
                }

                targetS.setPattern("\\bIl\\b");
                match = targetS.match(Qsentence);
                if (match.hasMatch())
                {
                    Qsentence.replace(targetS, "");
                    if (gender == Female)
                        potentialMismatch = true;
                    if (gender == genderUnknown)
                        potentialMatch = true;
                }

                targetS.setPattern("\\bShe\\b");
                match = targetS.match(Qsentence);
                if (match.hasMatch())
                {
                    Qsentence.replace(targetS, "");
                    if (gender == Male)
                        potentialMismatch = true;
                    if (gender == genderUnknown)
                        potentialMatch = true;
                }

                targetS.setPattern("\\bElle\\b");
                match = targetS.match(Qsentence);
                if (match.hasMatch())
                {
                    Qsentence.replace(targetS, "");
                    if (gender == Male)
                        potentialMismatch = true;
                    if (gender == genderUnknown)
                        potentialMatch = true;
                }

                // Basic words
                sentence = OQString(Qsentence);
                QStringList englishWords = QString("was|years|year|old").split("|");
                QStringList frenchWords = QString("était|ans|an").split("|");
                sentence.removeStrings(englishWords);
                sentence.removeStrings(frenchWords);

                sentence.replace(" ", "");
                sentence.removeOrdinal();
                if (sentence.isNumeric())
                {
                    unsigned int potentialAge = sentence.asNumber();
                    if (precedingFlag)
                        potentialAge--;
                    if ((potentialAge > 0) && (potentialAge < 125))
                    {
                        globals->globalDr->setAgeAtDeath(potentialAge);
                        globals->globalDr->setMinMaxDOB();

                        if (missingGender && potentialMatch)
                            globals->globalDr->setGender(gender);
                        if (!missingGender && potentialMismatch)
                        {
                            globals->globalDr->setGender(gender);
                            if (gender == Male)
                                globals->globalDr->wi.genderFlag = 21;
                            else
                                globals->globalDr->wi.genderFlag = 22;
                        }

                        keepGoing = false;
                    }
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
    dates = ucCleaned.sentencePullOutDates(globals->globalDr->getLanguage(), maxSentences);
    processNewDateInfo(dates, 8);
}

void readObit::fillInDates85Pass()
{
    // Look for age at next birthday (in first four sentences only)
    // DOB is also updated if valid age at death found
    unsigned int maxSentences = 4;
    ucCleaned.contentReadAgeNextBirthday(maxSentences);
}

void readObit::fillInDatesNinthPass()
{
    unstructuredContent sentence;

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

        bool restrictNamesToDR = false;

        success = query.prepare("SELECT fhFirstObit FROM death_audits.funeralhomedata WHERE providerID = :fhProviderID AND providerKey = :fhProviderKey AND "
                                "(fhRunStatus = 1 OR fhRunstatus = 2 OR fhRunStatus = 100 OR fhRunStatus = 101)");
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
            ucCleaned.beg();
            sentence = ucCleaned.getNextRealSentence(restrictNamesToDR, 1);

            /*sentence = ucCleaned.getSentence(globals->globalDr->getLanguage());
            while (sentence.isJustDates() || sentence.isJustNames() || sentence.startsWithClick(true))
            {
                sentence = ucCleaned.getSentence();
            }*/

            sentence.sentenceReadAgeAtDeath(true);
        }
    }
}

void readObit::fillInDatesTenthPass()
{
    if (globals->globalDr->getAgeAtDeath() != 0)
        return;

    bool restrictNamesToDR = false;
    unstructuredContent sentence;

    ucCleaned.beg();
    sentence = ucCleaned.getNextRealSentence(restrictNamesToDR, 1);

    /*sentence = ucCleaned.getSentence(globals->globalDr->getLanguage());
    while (!ucCleaned.isEOS() && (sentence.hasBookEnds(PARENTHESES | QUOTES) || sentence.hasAllWordsCapitalized() || sentence.isJustDates() || sentence.isJustNames() || sentence.startsWithClick(true)))
    {
        sentence = ucCleaned.getSentence();
    }*/

    sentence.sentenceReadNakedAgeAtDeath();

    // Try on second sentence
    if ((globals->globalDr->getAgeAtDeath() == 0) && !ucCleaned.isEOS())
    {
        sentence = ucCleaned.getSentence();
        sentence.sentenceReadNakedAgeAtDeath();
    }
}

void readObit::fillInDatesEleventhPass()
{
    bool keepLooking = !((globals->globalDr->getAgeAtDeath() != 0) || globals->globalDr->getDOB().isValid());
    bool restrictNamesToDR = false;

    unstructuredContent sentence;

    ucCleaned.beg();
    sentence = ucCleaned.getNextRealSentence(restrictNamesToDR, 5);

    /*sentence = ucCleaned.getSentence(globals->globalDr->getLanguage());
    while (keepLooking && !ucCleaned.isEOS() && (sentence.hasAllWordsCapitalized() || sentence.isJustDates() || sentence.isJustNames() || sentence.startsWithClick(true)))
    {
        sentence = ucCleaned.getSentence(globals->globalDr->getLanguage());
    }*/

    while (keepLooking)
    {
        keepLooking = sentence.sentenceReadYearsMarried(globals->globalDr->getLanguage());
        if (!ucCleaned.isEOS())
            sentence = ucCleaned.getSentence(globals->globalDr->getLanguage());
        else
            keepLooking = false;
    }
}

void readObit::fillInDatesTwelfthPass()
{
    if (globals->globalDr->getYOB() != 0)
        return;

    unstructuredContent sentence;
    unsigned int YOB = 0;

    ucCleaned.beg();
    sentence = ucCleaned.getSentence(globals->globalDr->getLanguage());
    while (!ucCleaned.isEOS() && (sentence.hasBookEnds(PARENTHESES | QUOTES) || sentence.hasAllWordsCapitalized() || sentence.isJustNames() || sentence.startsWithClick(true)))
    {
        sentence = ucCleaned.getSentence();
    }

    YOB = sentence.sentenceReadUnbalancedYOB();

    // Try on second sentence
    if ((YOB == 0) && !ucCleaned.isEOS())
    {
        sentence = ucCleaned.getSentence();
        YOB = sentence.sentenceReadUnbalancedYOB();
    }

    if ((YOB > 1900) && (YOB < static_cast<unsigned int>(globals->today.year())))
        globals->globalDr->setYOB(YOB);
}

void readObit::assignFirstDateToDOD()
{
    unstructuredContent sentence;
    QList<QDate> dateList;
    OQString cleanString;
    unsigned int YOD = globals->globalDr->getYOD();
    if (YOD == 0)
        YOD = globals->today.year();
    int i = 0;
    bool success = false;
    ucCleaned.beg();

    while (!success && !ucCleaned.isEOS() && (i < 2))
    {
        while (!ucCleaned.isEOS() && ((sentence.getLength() == 0) || sentence.isJustSavedNames() || sentence.startsWithClick(true)))
        {
            sentence = ucCleaned.getSentence();
        }
        i++;
        dateList.clear();

        sentence.pullOutDates(globals->globalDr->getLanguage(), dateList, 2, cleanString, false, false, YOD);
        if (dateList.size() == 1)
        {
            if ((YOD < 2021) && dateList[0].isValid() && (dateList[0].year() == static_cast<int>(YOD)))
                success = true;
            else
            {
                if ((globals->today.toJulianDay() - dateList[0].toJulianDay()) < 30)
                    success = true;
            }
        }
        else
        {
            if (dateList.size() != 0)
                i = 2;
        }
    }

    if (success)
        globals->globalDr->setDOD(dateList[0]);
}

bool readObit::useFirstSentenceSingleDate()
{
    bool implemented = false;

    ucCleaned.beg();
    unstructuredContent sentence;
    OQString cleanString;
    int numDates, daysEarlier;
    QList<QDate> dateList;
    QDate referenceDate;

    while (!ucCleaned.isEOS() && (((sentence.getLength() == 0) || sentence.isJustSavedNames() || sentence.startsWithClick(true)) || sentence.streamIsJustDates()))
    {
        sentence = ucCleaned.getSentence();
    }

    numDates = sentence.pullOutDates(globals->globalDr->getLanguage(), dateList, 3, cleanString, false);
    if (numDates == 1)
    {
        referenceDate = globals->today;
        if (globals->globalDr->getPublishDate().isValid())
            referenceDate = globals->globalDr->getPublishDate();
        daysEarlier = referenceDate.toJulianDay() - dateList[0].toJulianDay();
        if ((daysEarlier >= 0) && (daysEarlier <= 60))
        {
            globals->globalDr->setDOD(dateList[0]);
            implemented = true;
        }
    }

    return implemented;
}

QDate readObit::getFirstSentenceSingleDate()
{
    ucCleaned.beg();
    unstructuredContent sentence;
    OQString cleanString;
    int numDates;
    QList<QDate> dateList;
    QDate resultDate;

    while (!ucCleaned.isEOS() && (((sentence.getLength() == 0) || sentence.isJustSavedNames() || sentence.startsWithClick(true)) || sentence.streamIsJustDates()))
    {
        sentence = ucCleaned.getSentence();
    }

    numDates = sentence.pullOutDates(globals->globalDr->getLanguage(), dateList, 3, cleanString, false);
    if (numDates == 1)
        resultDate = dateList[0];

    return resultDate;
}

void readObit::readAndPrepareStructuredContent()
{
    beg();
    bool DOBfound, DODfound, primarySourceProcessed;
    unsigned int position;
    databaseSearches dbSearch;
    QDate qdate;
    QString target;
    OQString cleanString, word, fullWord;
    QString space(" ");
    QList<QString> purgeList;
    PQString name;
    unstructuredContent tempUC, tempTempUC;
    tempUC.setGlobalVars(*this->globals);
    tempTempUC.setGlobalVars(*this->globals);
    QRegularExpression rxTarget;
    QRegularExpressionMatch match;

    DOBfound = false;
    DODfound = false;

    switch(globals->globalDr->getProvider())
    {
    case Legacy:

        switch(style)
        {
        case 0:
            // DOB
            if (conditionalMoveTo("\"dob\"", "lago", 0))
            {
                tempUC = readNextBetween(QUOTES);
                tempUC.processDateField(dfDOB, doMDY);
            }

            // DOD
            if (conditionalMoveTo("\"dod\"", "lago", 0))
            {
                tempUC = readNextBetween(QUOTES);
                tempUC.processDateField(dfDOD, doMDY);
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
                tempUC.prepareStructuredNames();

                // In some cases, can find (YYYY - YYYY)
                if (moveTo(" (", 20))
                {
                    backward(1);
                    tempUC = readNextBetween(PARENTHESES);
                    tempUC.processStructuredYears();
                }
           }
            break;

        case 1:
            if (moveTo("\"birthDate\": ", 20000))
            {
                tempUC = readNextBetween(QUOTES);
                tempUC.processDateField(dfDOB);
            }

            if (moveTo("\"deathDate\": ", 100))
            {
                tempUC = readNextBetween(QUOTES);
                tempUC.processDateField(dfDOD);
            }

            if (moveTo("\"givenName\": ", 500))
                tempUC = readNextBetween(QUOTES);

            if (moveBackwardTo("\"additionalName\": ", 1000))
            {
                tempTempUC = readNextBetween(QUOTES);
                if (tempTempUC.getLength() > 0)
                    tempUC += PQString(" ") + tempTempUC;
            }

            if (moveTo("\"familyName\": ", 1000))
            {
                tempTempUC = readNextBetween(QUOTES);
                if (tempTempUC.getLength() > 0)
                    tempUC += PQString(" ") + tempTempUC;
            }

            tempUC.prepareStructuredNames();

            if (moveTo("class=\"year-born-year-died\""))
            {
                DATES dates;
                tempUC = readNextBetween(BRACKETS);
                if (tempUC.getLength() == 11)
                    tempUC.processStructuredYears();
                else
                {
                    dates = tempUC.readNumericDates(doYMD);
                    if (dates.hasDateInfo())
                        processNewDateInfo(dates, 1);
                }
            }

            beg();
            // Second source of structured names
            if (moveTo("\"keywords\": ["))
            {
                tempUC = readNextBetween(QUOTES);
                tempUC.replace(QString("Obituary"), QString(""), Qt::CaseInsensitive);
                tempUC.processAllNames();
            }

            // set flag to potential ignore memorial records
            if (moveTo("\"PaymentCode\": "))
            {
                tempUC = readNextBetween(QUOTES);
                if (tempUC == OQString("memoriam"))
                    globals->globalDr->wi.memorialFlag = 1;
            }
            break;

        case 2:
            if (moveTo("<div class=\"obit-name\">"))
            {
                tempUC = getUntil("</div>");
                tempUC.removeHTMLtags();
                tempUC.cleanUpEnds();
                tempUC.prepareStructuredNames();

                if (moveTo("<div class=\"obit-dates\">"))
                {
                    tempUC = getUntil("</div>");
                    fillInDatesStructured(tempUC);
                }

            }
            break;

        }

        break;

    case Passages:
        if (moveTo("h1 class=\"details\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
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

    case CooperativeFuneraire:
        if (moveTo("class=\"obituary-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.cleanUpEnds();
            tempUC.removeEnding(",");
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"obituary-years\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }

            if (moveTo("class=\"tj_field tj_date_du_deces\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }
        }
        break;

    case DomainFuneraire:
        if (moveTo("<h1 class=\"text"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<span", 50))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case GreatWest:
    {
        switch (globals->globalDr->getProviderKey())
        {
        case 18:
            if (moveTo("title text-noleading mb-2"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
            break;

        default:
            OQStream tempStream(globals->uc->getString());
            if (tempStream.moveTo("Date of death:"))
            {
                tempUC = tempStream.getUntil(".");
                tempUC.cleanUpEnds();
                if (tempUC.getLength() == 4)
                    tempUC.processYearField(dfDOD);
                else
                    tempUC.processDateField(dfDOD);
            }
            tempStream.beg();
            if (tempStream.moveTo("Date of birth:"))
            {
                tempUC = tempStream.getUntil(".");
                tempUC.cleanUpEnds();
                if (tempUC.getLength() == 4)
                    tempUC.processYearField(dfDOB);
                else
                    tempUC.processDateField(dfDOB);
            }
            tempStream.beg();
            if (tempStream.moveTo("Age: ", 150))
            {
                tempUC = tempStream.getUntil(" ");
                tempUC.processAgeAtDeath();
            }

            this->itsString.replace(QString("</strong>:"), QString(":</strong>"), Qt::CaseSensitive);
            if (consecutiveMovesTo(200, ">Obituaries<", "data-title="))
            {
                tempUC = getUntil(" data-tag");
                tempUC.removeBookEnds(QUOTES);

                // Fix problem of extra wording at front
                int index = tempUC.getString().lastIndexOf(":");
                if (index != -1)
                {
                    bool dropBeg = false;
                    QString beginning = tempUC.left(index).lower().getString();
                    QStringList badWords = QString("announced|announcement|burial|celebration|funeral|mass|memorial|memoriam|service|visitation").split("|");
                    while (!dropBeg && !badWords.isEmpty())
                    {
                        dropBeg = beginning.contains(badWords.takeFirst(), Qt::CaseInsensitive);
                    }
                    if (dropBeg)
                        tempUC.dropLeft(index + 1);
                }

                // Fix double quote problem
                index = tempUC.getString().indexOf("&quot;&quot;");
                if (index != -1)
                {
                    QString newString;
                    QString tempString = tempUC.getString();
                    int index2 = tempString.indexOf(" ", index);
                    if (index2 == -1)
                        index2 = tempString.length() + 1;
                    newString  = tempUC.left(index).getString();
                    newString += QString("\"");
                    newString += tempString.mid(index + 12, index2 - index - 12);
                    newString += QString("\"");
                    if (index2 < tempString.length())
                        newString += tempString.right(tempString.length() - index2);

                    tempUC = newString;
                }

                tempUC.prepareStructuredNames();

                bool eitherFound = conditionalMoveTo("class=\"mt-0\"", "class=\"cp-obit-age\">", 0, 500);
                if (eitherFound)
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processDateField(dfDOD);

                    if (moveTo("class=\"cp-obit-age\">", 100))
                    {
                        tempUC = getUntilEarliestOf(" ", "<", 100);
                        tempUC.processAgeAtDeath();
                    }
                }
                else
                    backward(500);

                if (moveTo("class=\"cp-obit-age\">", 500))
                {
                    tempUC = getUntilEarliestOf(" ", "<", 100);
                    tempUC.processAgeAtDeath();
                }
                else
                {
                    if (globals->globalDr->missingDOB() || globals->globalDr->missingDOD())
                    {
                        beg();
                        if (moveTo("<strong>Date of death:</strong>"))
                        {
                            tempUC = getUntil("<");
                            tempUC.processDateField(dfDOD);

                            if (moveTo("<strong>Date of birth:</strong>"))
                            {
                                tempUC = getUntil("<");
                                tempUC.processDateField(dfDOB);
                            }

                            if (moveTo("<strong>Age:</strong>"))
                            {
                                tempUC = getUntil("<");
                                tempUC.processAgeAtDeath();
                            }
                        }
                        else
                        {
                            beg();
                            if (consecutiveMovesTo(250, ">Date of death", ": "))
                            {
                                tempUC = getUntil("<");
                                tempUC.processDateField(dfDOD);

                                if (consecutiveMovesTo(250, ">Date of birth", ": "))
                                {
                                    tempUC = getUntil("<");
                                    tempUC.processYearField(dfDOB);
                                }

                                if (consecutiveMovesTo(250, ">Age", ": "))
                                {
                                    tempUC = getUntil("<");
                                    tempUC.processAgeAtDeath();
                                }
                            }
                        }
                    }
                }
            }
            break;
        }

    }
        break;

    case EchoVita:
        if (moveTo("mobile-title"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(100, "fa-calendar-day", "</i>"))
            {
                tempUC = getUntil("<span class=");
                fillInDatesStructured(tempUC);
            }

            tempUC = readNextBetween(BRACKETS);
            tempUC.cleanUpEnds();
            tempUC.removeBookEnds(PARENTHESES);

            int index = tempUC.findPosition(" ");
            if (index > 0)
            {
                int age = tempUC.left(index).asNumber();
                if ((age > 0) && (age < 125))
                    globals->globalDr->setAgeAtDeath(age);
            }

            // Potential backup read of YOB - YOD in title
            if (!globals->globalDr->getDOB().isValid())
            {
                tempUC = globals->globalDr->getTitle();
                tempUC.cleanUpEnds();
                index = tempUC.findPosition("(", -1);
                if (index > 0)
                {
                    tempTempUC = tempUC.right(tempUC.getLength() - index);
                    tempTempUC.removeBookEnds(PARENTHESES);
                    if (tempTempUC.getLength() == 11)
                        tempTempUC.processStructuredYears();
                }
            }
        }
        break;

    case SaltWire:
        if (moveTo("class=\"sw-page__title\">"))
        {
            tempUC = getUntilEarliestOf("<", "&mdash;");
            if (tempUC.contains("Halifax"))
            {
                beg();
                if (consecutiveMovesTo(100, "class=\"sw-page__title\">", "&mdash;"))
                    tempUC = getUntil("<");
            }
            tempUC.SaltWireCleanUp();
            tempUC.prepareStructuredNames();
        }
        break;

    case BlackPress:
        switch(style)
        {
        case 0:
            if (moveTo("class=\"entry-title\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (consecutiveMovesTo(50, "class=\"entry-content", "<p><b>"))
                {
                    tempUC = getUntil("</b>");
                    fillInDatesStructured(tempUC);
                }
            }
            else
            {
                beg();
                if (consecutiveMovesTo(100, "class=\"post-title\"", ">"))
                {
                    tempUC = getUntil("<");
                    tempUC.prepareStructuredNames();

                    if (consecutiveMovesTo(200, "class=\"post-published", "datetime=", ">On "))
                    {
                        tempUC = getUntil("</time>");
                        tempUC.processDateField(dfDOD);
                    }
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(100, "class=\"title text", "\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("<p", 25))
                {
                    tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);
                }
            }
            break;
        }

        break;

    case Aberdeen:
        if (consecutiveMovesTo(50, "class=\"obit-name\"", "obit-h1"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"obit-dates\"", 100))
            {
                if (conditionalMoveTo("obit-born", "obit-content", 0))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processDateField(dfDOB);
                }

                if (conditionalMoveTo("obit-died", "obit-content", 0))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processDateField(dfDOD);
                }
                else
                {
                    tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);
                }
            }

            beg();
            if (consecutiveMovesTo(30, "datePublished", ":\""))
            {
                tempUC = getUntil("T");
                tempUC.processDateField(dfDOP);
            }
        }
        break;

    case Batesville:
        switch (style)
        {
        case 0:
            if (moveTo("obit_name"))
            {
                // Read names (First Name first expected)
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                // Read DOB and DOD in form (mmm dd, yyyy - mmm dd, yyyy)
                if (moveTo("lifespan"))
                {
                    tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);

                    // If still no go, assume that if field contains only a single date, then it is DOD
                    if ((globals->globalDr->getYOB() == 0) && (globals->globalDr->getYOD() == 0))
                    {
                        if (tempUC.getLength() <= 21)
                            tempUC.processDateField(dfDOD);
                    }
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(150, "\"text-container\"", "<h1>"))
            {
                // Read names (First Name first expected)
                tempUC = getUntil("</h1>", 100);
                tempUC.prepareStructuredNames();

                // Read DOB and DOD in form (mmm dd, yyyy - mmm dd, yyyy)
                if (moveTo("<h5>"))
                {
                    tempUC = getUntil("</h5>", 200);
                    fillInDatesStructured(tempUC);
                }
            }
            break;

        case 2:
            // Read full name
            if (consecutiveMovesTo(100, "keywords", "Obituary, "))
            {
                tempUC = getUntil(",");
                tempUC.prepareStructuredNames();
            }

            // No dates to be read as they would be in the JSON data if known
            break;

        case 3:
            // Read full name
            if (moveTo("class=\"obitHD\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
            break;

        case 4:
            if (consecutiveMovesTo(400, "class\"obithead\"", "class=\"title\"","<h1"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("<h", 25))
                {
                    tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);
                }
            }
            break;

        case 5:
            if (consecutiveMovesTo(50, "class=\"obit-name-text\"", "<h"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("<p", 25))
                {
                    tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);
                }
            }
            break;

        case 6:
            if (consecutiveMovesTo(300, "class=\"obitbar-name\"", "<h"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("<small", 25))
                {
                    tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);
                }
            }
            else
            {
                beg();
                if (consecutiveMovesTo(300, ">View Obituaries<", "<h", "swatch-"))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.prepareStructuredNames();

                    if (consecutiveMovesTo(50, "swatch-", "small"))
                    {
                        tempUC = readNextBetween(BRACKETS);
                        fillInDatesStructured(tempUC);
                    }
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
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

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
            tempUC.prepareStructuredNames();
        }
        else
        {
            // Most likely means a deleted obituary, so pull name from URL
            QString tempString = globals->globalDr->getURL();
            int indexA = tempString.lastIndexOf("/");
            int indexB = tempString.lastIndexOf("-");
            if ((indexA >=0) && (indexB >= 0))
            {
                indexA++;
                tempString.replace("-", " ");
                tempUC = tempString.mid(indexA, indexB - indexA);
                tempUC.prepareStructuredNames();
            }
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

        // Look for funeral date
        beg();
        if (consecutiveMovesTo(100, "class=\"service-date\"", "class=\"date\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.processDateField(dfDOS);
        }

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

            tempUC.prepareStructuredNames();
        }

        break;

    case CFS:
        if ((globals->globalDr->getProviderKey() > 90000) || (globals->globalDr->getProviderKey() == 824))
        {
            // Old style of obits
            if (consecutiveMovesTo(500, "class=\"obituary\"", "style=\"font-size:120%;\""))
            {
                // Read name first
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

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

            if (style == 2)
            {
                if (moveTo("<h1 class=\"hidden-xs\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.prepareStructuredNames();

                    if (consecutiveMovesTo(200, "class=\"dates hidden-xs\"", "<p"))
                    {
                        if (conditionalMoveTo("class=\"dod\"", "</p>", 0))
                        {
                            tempUC = readNextBetween(BRACKETS);
                            tempUC.processDateField(dfDOD);
                        }
                        else
                        {
                            tempUC = readNextBetween(BRACKETS);
                            tempUC.processStructuredYears();
                        }
                    }
                }
            }
            else
            {
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
                    tempUC.prepareStructuredNames();
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
                            tempUC.processYearField(dfDOB);
                        else
                        {
                            DOBfound = true;
                        }
                    }

                    backward(getPosition() - position);

                    if (moveTo("class=\"dod\""))
                    {
                        tempTempUC = readNextBetween(BRACKETS);
                        if (tempTempUC.getLength() == 4)
                            tempTempUC.processYearField(dfDOD);
                        else
                        {
                            DODfound = true;
                        }
                    }

                    if (DOBfound)
                    {
                        if (DODfound)
                        {
                            tempUC = tempUC + OQString(" - ") + tempTempUC;
                            fillInDatesStructured(tempUC);
                        }
                        else
                            tempUC.processDateField(dfDOB);
                    }
                    else
                    {
                        if (DODfound)
                            tempTempUC.processDateField(dfDOD);
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
                tempUC.prepareStructuredNames();

                // Read dates if provided
                position = getPosition();
                if (moveTo("itemprop=\"birthDate\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processDateField(dfDOB);
                }

                backward(getPosition() - position);

                if (moveTo("itemprop=\"deathDate\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processDateField(dfDOD);
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
                tempUC.prepareStructuredNames();
            }

            // Read dates if provided
            position = getPosition();
            if (moveTo("itemprop=\"birthDate\":"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOB);
            }

            backward(getPosition() - position);

            if (moveTo("itemprop=\"deathDate\":"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }

        break;

    case 2:
        // Read Name
        if (consecutiveMovesTo(200, "MaterialData.user =", "fullName:", "\'"))
        {
            tempUC = getUntil("avatar:");
            tempUC.dropRight(tempUC.getLength() - static_cast<unsigned int>(tempUC.findPosition(PQString("\',"), -1)));
            tempUC.purge(purgeList);
            tempUC.prepareStructuredNames();
        }

        // Read dates if provided
        position = getPosition();
        if (moveTo("\"birthDate\":"))
        {
            tempUC = readNextBetween(QUOTES);
            tempUC.processDateField(dfDOB);
        }

        backward(getPosition() - position);

        if (moveTo("\"deathDate\":"))
        {
            tempUC = readNextBetween(QUOTES);
            tempUC.processDateField(dfDOD);
        }
        break;

        }
        break;

    case Alternatives:
        if (moveTo("class=\"page-title obit-title\""))
        {
            // Get name
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            // Read DOD only
            if (moveTo("class=\"obit-dod\""))
            {
                tempUC = readNextBetween(BRACKETS);
                // On occasion field will contain both DOB and DOD
                DATES dates = tempUC.readDOBandDOD();
                if (dates.hasDateInfo())
                    processNewDateInfo(dates, 1);
                else
                    tempUC.processDateField(dfDOD);
            }
        }

        break;

    case FuneralTech:
        switch (style)
        {
        case 0:
            tempUC = globals->globalDr->getTitle();
            //tempUC.dropLeft(12);    // "Obituary of "
            tempUC.simplify();

            // Everything after comma for V.J. McGillivray is location info
            if (globals->globalDr->getProviderKey() == 76)
            {
                int index = tempUC.getString().indexOf(QString(","));
                if (index >= 0)
                    tempUC = tempUC.left(index);
            }
            tempUC.prepareStructuredNames();

            beg();
            if (consecutiveMovesTo(100, "In loving memory of", "<h3"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                if (tempUC.getLength() > 11)
                    fillInDatesStructured(tempUC);
                else
                    tempUC.processStructuredYears();
            }
            break;

        case 1:
        {
            if (consecutiveMovesTo(200, "class=\"deceased-info\"", "<h2"))
            {
                // Read names (First Name first expected)
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();

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
                    tempStream = globals->globalObit->uc;
                    while ((wordCount < 30) && !tempStream.isEOS() && keepGoing)
                    {
                        word = tempStream.getWord();
                        wordCount++;
                        if (word.lower() == PQString("on"))
                            indexOn = wordCount;

                        if ((indexOn > 0) && ((wordCount - indexOn) == 4))
                        {
                            OQString nextWord = tempStream.peekAtWord();
                            if (nextWord.isCapitalized() && dbSearch.givenNameLookup(nextWord.getString(), globals))
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
                                tempUC.prepareStructuredNames();
                                replaced = true;
                            }
                        }
                    }
                }

                if (!originalProblematic || !replaced)
                    tempUC.prepareStructuredNames();

                // Read DOB and DOD in form (mmm dd, yyyy - mmm dd, yyyy)
                if (moveTo("<h3"))
                {
                    tempUC = readNextBetween(PARENTHESES);
                    fillInDatesStructured(tempUC);

                    if (moveTo("class=\"in-memory-years\""))
                    {
                        tempUC = readNextBetween(BRACKETS);
                        tempUC.processStructuredYears();
                    }
                }
                else
                {
                    beg();
                    if (consecutiveMovesTo(100, "class=\"obituary-text\"", "Obituary of "))
                        tempUC = getUntil("<");
                }
            }
        }
            break;

        case 2:
            if (consecutiveMovesTo(2000, "class=\"flipbook-container\"", "class=\"bottom-text\"", "<div"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
            break;

        case 3:
            if (consecutiveMovesTo(75, "class=\"top-full-name\"", "<h"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("<div class=\"top-dates\">"))
                {
                    tempUC = getUntil("</div>");
                    tempUC.removeHTMLtags();
                    tempUC.simplify();
                    fillInDatesStructured(tempUC);
                }
            }
            break;

        case 4:
        case 5:
            if (consecutiveMovesTo(100, "class=\"deceased-info\"", "<h"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("<h3", 50))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processStructuredYears();
                }
            }
            break;
        }

        break;

    case WordPress:
        // Structured content written as "Smith, John Stanley  1968 - 2015"
        if (moveTo("class=\"entry-title\""))
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
            tempUC.prepareStructuredNames();

            // Read YOB and YOD in form (yyyy - yyyy)
            tempUC = unstructuredContent(endString);
            fillInDatesStructured(tempUC);
        }
        break;

    case Burke:
    case FrontRunner:
        // Cooperative in Quebec WAS completely different - now coded as style = 0
        if ((globals->globalDr->getProviderKey() == 25) && ((2+2)==3))
        {
            OQString tempString, firstName, middleNames, lastName, nickName, maidenName;

            moveTo("var firstName =");
            firstName = readNextBetween(QUOTES);

            moveTo("var lastName =");
            lastName = readNextBetween(QUOTES);

            moveTo("var middleName =");
            middleNames = readNextBetween(QUOTES);

            moveTo("var nickName =");
            nickName = readNextBetween(QUOTES);
            if (nickName.getLength() > 0)
                nickName = OQString("\"") + nickName + OQString("\"");

            moveTo("var dateOfDeath =");
            tempString = readNextBetween(QUOTES);
            if (tempString.getLength() == 4)
                globals->globalDr->setYOD(static_cast<unsigned int>(tempString.asNumber()));

            moveTo("var dateOfBirth =");
            tempString = readNextBetween(QUOTES);
            if (tempString.getLength() == 4)
                globals->globalDr->setYOB(static_cast<unsigned int>(tempString.asNumber()));

            moveTo("var maidenName =");
            maidenName = readNextBetween(QUOTES);
            if (maidenName.getLength() > 0)
                maidenName = OQString("(") + maidenName + OQString(")");

            tempString = firstName;
            if (middleNames.getLength() > 0)
                tempString += OQString(" ") + middleNames;
            if (nickName.getLength() > 0)
                tempString += OQString(" ") + nickName;
            if (lastName.getLength() > 0)
                tempString += OQString(" ") + lastName;
            if (maidenName.getLength() > 0)
                tempString += OQString(" ") + maidenName;

            tempUC = tempString;
            tempUC.prepareStructuredNames();
        }
        else
        {
            tempTempUC.clear();
            if (consecutiveMovesTo(20, "class=\"obitTitle\"", " for "))
            {
                tempTempUC = getUntil("<");
                tempTempUC.extraNameProcessing();
            }
            beg();

            switch(style)
            {
            case 0:
                if (consecutiveMovesTo(500, "bom-in-memory-name", ">"))
                {
                    // Read name
                    tempUC = getUntil("</div>");
                    tempUC.extraNameProcessing();
                    tempUC.enhanceWith(tempTempUC);
                    tempUC.prepareStructuredNames();

                    // Read YOB and YOD (YYYY - YYYY)
                    if (moveTo("bom-in-memory-date\">"))
                    {
                        tempUC = getUntil("</div>");
                        fillInDatesStructured(tempUC);
                    }
                }
                break;

            case 1:
                if (consecutiveMovesTo(500, "class=\"bom-header-info\"", "a href", "php\">"))
                {
                    // Read name
                    tempUC = getUntil("</a>");
                    tempUC.extraNameProcessing();
                    tempUC.enhanceWith(tempTempUC);
                    tempUC.prepareStructuredNames();

                    // Read DOB and DOD
                    if (moveTo("<p>"))
                    {
                        tempUC = getUntil("</p>");
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
                    tempUC.extraNameProcessing();
                    tempUC.enhanceWith(tempTempUC);
                    tempUC.prepareStructuredNames();

                    // Read DOB and DOD
                    if (moveTo("<!-- obit date -->"))
                    {
                        position = getPosition();

                        if (moveTo("class=\"dateofBirth\""))
                        {
                            tempUC = readNextBetween(BRACKETS);
                            if (tempUC.getLength() == 4)
                                tempUC.processYearField(dfDOB);
                            else
                                tempUC.processDateField(dfDOB);
                        }

                        backward(getPosition() - position);

                        if (moveTo("class=\"dateofDeath\""))
                        {
                            tempUC = readNextBetween(BRACKETS);
                            if (tempUC.getLength() == 4)
                                tempUC.processYearField(dfDOD);
                            else
                                tempUC.processDateField(dfDOD);
                        }
                    }
                }
                break;

            case 4:
                if (moveTo("class=\"deceased-info\""))
                {
                    consecutiveMovesTo(10, "<h", ">");
                    tempUC = getUntil("<");
                    tempUC.prepareStructuredNames();

                    if (conditionalMoveTo("<h", "</div>", 0))
                    {
                        moveTo(">");
                        tempUC = getUntil("<");
                        tempUC.processDateField(dfDOD);
                    }
                }
                break;

            case 5:
                if (consecutiveMovesTo(200, "id=\"bom-tunnel-deceased\"", "<span>"))
                {
                    tempUC = getUntil("</span>");
                    tempUC.removeHTMLtags();
                    tempUC.prepareStructuredNames();

                    if (consecutiveMovesTo(200, "id=\"bom-tunnel-dod\"", "<span>"))
                    {
                        tempUC = getUntil("</span>");
                        fillInDatesStructured(tempUC);
                    }
                }
                break;

            case 6:
                if (consecutiveMovesTo(75, "class=\"top-full-name\"", "<h"))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.prepareStructuredNames();

                    if (moveTo("<div class=\"top-dates\">"))
                    {
                        tempUC = getUntil("</div>");
                        tempUC.removeHTMLtags();
                        tempUC.simplify();
                        fillInDatesStructured(tempUC);
                    }
                }
                break;

            case 7:
                if (moveTo("\"entry-title\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.prepareStructuredNames();

                    if (moveTo("<p>"))
                    {
                        if (conditionalMoveTo(">elevated", "</p>", 0))
                        {
                            moveTo(">");
                            tempUC = getUntil("</p>");
                            tempUC.removeHTMLtags();
                            tempUC.cleanUpEnds();
                            fillInDatesStructured(tempUC);
                        }
                    }
                    else
                    {
                        beg();
                        if (consecutiveMovesTo(500, "\"entry-title\"", "<span"))
                        {
                            tempUC = readNextBetween(BRACKETS);
                            fillInDatesStructured(tempUC);
                        }
                    }

                    if (moveToEarliestOf(", ON ", ", ON, ", 1000))
                    {
                        QString pc = getNext(7).getString();
                        globals->globalDr->setPostalCode(pc);
                    }
                    else
                    {
                        QRegularExpression target;
                        QRegularExpressionMatch match;
                        target.setPattern("[L-M][0-9][A-Z] [0-9][A-Z][0-9]");
                        match = target.match(this->itsString);
                        if (match.hasMatch())
                        {
                            QString pc = match.captured(0);
                            globals->globalDr->setPostalCode(pc);
                        }
                    }
                }
                break;
            }

            // Supplementary read to fill in missing information (service.php vs. obituary.php info)
            if (!globals->globalDr->getDOB().isValid() || !globals->globalDr->getDOD().isValid())
            {
                beg();
                if (moveTo("\"birthDate\":"))
                {
                    tempUC = readNextBetween(QUOTES);
                    tempUC.processDateField(dfDOB);

                    if (moveTo("\"deathDate\":"))
                    {
                        tempUC = readNextBetween(QUOTES);
                        tempUC.processDateField(dfDOD);
                    }
                }

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
                                    tempUC.processDateField(dfDOB);
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
                                    tempUC.processDateField(dfDOD);
                                }
                            }
                        }
                    }
                }

                if (!globals->globalDr->getDOB().isValid() || !globals->globalDr->getDOD().isValid())
                {
                    beg();
                    if (moveTo("top-tribute-list"))
                    {
                        QString ID = globals->globalDr->getID().getString();
                        if (moveTo(ID))
                        {
                            moveBackwardTo("top-tribute-list-record-con", 1500);
                            if (conditionalMoveTo(">B: ", ID, 0))
                            {
                                tempUC = getUntil("<");
                                tempUC.processDateField(dfDOB);
                            }
                            if (conditionalMoveTo(">D: ", ID, 0))
                            {
                                tempUC = getUntil("<");
                                tempUC.processDateField(dfDOD);
                            }
                        }
                    }
                }
            }

            // Another final effort on DOD
            if (!globals->globalDr->getDOD().isValid())
            {
                beg();
                if (moveTo("<meta property=\"og:title\"  content="))
                {
                    tempUC = readQuotedMetaContent();
                    QList<QDate> dateList;
                    OQString cleanString;
                    int numDates = tempUC.pullOutDates(language_unknown, dateList, 2, cleanString, false);
                    if (numDates == 1)
                        globals->globalDr->setDOD(dateList[0]);
                }
            }
        }

        break;

    case FuneralOne:
        switch(style)
        {
        case 0:
            if (moveTo("tributeDisplayName:"))
            {
                moveTo("\"");
                tempUC = getUntil("\",");
                tempUC = tempUC.getString().replace("\\\"", "\"");
                tempUC.prepareStructuredNames();
            }
            else
                beg();

            if (consecutiveMovesTo(200, "<dt>Name</dt>", "itemprop=\"name\""))
            {
                if (moveTo("itemprop") && conditionalMoveTo("birthDate", "itemprop", 0))
                {
                    moveTo("datetime=");
                    tempUC = readNextBetween(QUOTES);
                    tempUC.processDateField(dfDOB, doYMD);
                }

                if (moveTo("itemprop") && conditionalMoveTo("deathDate", "itemprop", 0))
                {
                    moveTo("datetime=");
                    tempUC = readNextBetween(QUOTES);
                    tempUC.processDateField(dfDOD, doYMD);
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
                    tempUC.compressCompoundNames(word, language_unknown);
                    name = word.getString();
                    globals->globalDr->setFamilyName(name);
                }

                consecutiveMovesTo(100, "id=\"hidBirth\"", "value=");
                if (!globals->globalDr->getDOB().isValid())
                {
                    tempUC = readNextBetween(QUOTES);
                    tempUC.processDateField(dfDOB, doMDY);
                }

                consecutiveMovesTo(100, "id=\"hidDeath\"", "value=");
                if (!globals->globalDr->getDOD().isValid())
                {
                    tempUC = readNextBetween(QUOTES);
                    tempUC.processDateField(dfDOD, doMDY);
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(50, "<div class=\"obit-heading-wrapper\">", "<h"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("class=\"obit-dates-wrapper\">"))
                {
                    tempUC = getUntil("</div></div>");
                    tempUC.removeHTMLtags();
                    fillInDatesStructured(tempUC);
                }
            }
            break;
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
                tempUC.prepareStructuredNames();
            }

            // Read DOB - DOD
            if (moveTo("<p>"))
            {
                tempUC = getUntil("</p>");
                fillInDatesStructured(tempUC);
            }
            break;

        case 1:
            // Read name
            if (consecutiveMovesTo(100, "class=\"obit_name\"", "\"name\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }

            // Read DOB - DOD
            if (moveTo("class=\"Small_Text\""))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
            break;

        case 2:
            if (moveTo("obit_name_and_date"))
            {
                moveTo("<h");
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                moveTo("<h");
                tempUC = readNextBetween(BRACKETS);
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

            tempUC.prepareStructuredNames();

            // Read YOB and YOD (YYYY - YYYY)
            if (moveTo("class=\"years\""))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case ClickTributes:
        // Read name
        if (moveTo("class=\"ct-name\">"))
        {
            tempUC = getUntil("</h2>");
            tempUC.prepareStructuredNames();
        }

        // Read DOB - DOD
        if (!globals->globalDr->getDOB().isValid() || !globals->globalDr->getDOD().isValid())
        {
            if (moveTo("class=\"ct-dates\">"))
            {
                tempUC = getUntil("<");
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case ConnellyMcKinley:
        // Read name
        if (consecutiveMovesTo(100, "entry-title", "itemprop=\"name\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Arbor:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(200, "id=\"personInfoContainer\"", "id=\"displayName\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }

            if (conditionalMoveTo("birthDate", "siteCredits", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOB);
            }

            if (conditionalMoveTo("deathDate", "siteCredits", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
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
                    tempUC.prepareStructuredNames();
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
                    tempUC.prepareStructuredNames();
            }
            break;

        case 2:
            if (moveToEarliestOf(">In Celebration of<", ">À la mémoire de<"))
            {
                if (moveTo("<strong", 50))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.prepareStructuredNames();

                    if (conditionalMoveTo("itemprop=\"birthDate\"", "</h"))
                    {
                        tempUC = readNextBetween(BRACKETS);
                        tempUC.processDateField(dfDOB);
                    }

                    if (conditionalMoveTo("itemprop=\"deathDate\"", "</h"))
                    {
                        tempUC = readNextBetween(BRACKETS);
                        tempUC.processDateField(dfDOD);
                    }
                }
            }
            break;

        case 3:
            if (moveTo("class=\"hero__name\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("class=\"hero__lifetime\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);
                }
            }
        }
        break;

   case SiteWyze:
        // Only name available (same was used for titlestream)
        if (consecutiveMovesTo(2000, "post-content", "<h2"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
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
                tempUC.prepareStructuredNames();
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
            if (consecutiveMovesTo(4000, "<!-- #masthead -->", "<h1", ">"))
            {
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }

            // Read DOD
            if (moveTo("elementor-post-info__item--type-date", 2000))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }

            break;

        case 1:
            // Only name at this point
            if (moveTo("post_title entry-title"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
            break;

        case 2:
        case 3:
            if(moveTo("itemprop=\"datePublished\""))
            {
                if (moveBackwardTo("elementor-heading-title elementor-size-default"))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.prepareStructuredNames();
                }

                if (consecutiveMovesTo(200, "<span", ">"))
                {
                    tempUC = getUntil("<");
                    tempUC.processDateField(dfDOD);
                }
            }
            break;

        }

        break;
    }

    case Shape5:
    {
        if (consecutiveMovesTo(200, "page-header", "headline", ">"))
            tempUC = getUntil("<");
        tempUC.prepareStructuredNames();
        break;
    }

    case TributeArchive:
    {
        if (moveTo("subtitle-story hidden-xs"))
        {
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

            tempUC.prepareStructuredNames();

            // Some obits will have dates BUT they will always be english, even if text is french
            if (moveTo("date-story hidden-xs", 250))
            {
                DATES dates;
                bool reliable = true;
                moveTo(">");
                tempUC = getUntil(("</p>"));
                tempUC.simplify();
                tempUC.removeHTMLtags();
                if (tempUC.getLength() > 0)
                {
                    tempUC.setContentLanguage(english);
                    fillInDatesStructured(tempUC, reliable);
                }
            }
        }
        else
        {
            beg();
            if (moveTo("class=\"obit-name\""))
            {
                //tempUC = readNextBetween(BRACKETS);
                moveTo(">");
                tempUC = getUntil("</div>");
                tempUC.removeHTMLtags();
                tempUC.prepareStructuredNames();

                if (moveTo("class=\"obit-dates\""))
                {
                    moveTo(">");
                    tempUC = getUntil("</div>");
                    tempUC.removeHTMLtags();
                    //tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);
                }
            }
        }

        break;
    }

    case YAS:
    {
        if (moveTo("class=\"post-header\">"))
        {
           tempUC = getUntil("</header>");
           tempUC.prepareStructuredNames();
        }
        break;
    }

    case WFFH:
    {
        // Name, DOB and DOD typcially availabile
        if (consecutiveMovesTo(500, "persondetails-area", "personName", "personFirst", ">"))
        {
            tempUC = getUntil("</div>");
            tempUC.prepareStructuredNames();
        }

        // Dates
        if (conditionalMoveTo("personBirth", "personContact", 0))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.processDateField(dfDOB);
        }

        if (conditionalMoveTo("personDeath", "personContact", 0))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.processDateField(dfDOD);
        }
    }
        break;

    case FHW:
    {
        switch(style)
        {
        case 0:
        case 2:
            // Name, DOB and DOD typcially availabile
            if (consecutiveMovesTo(150, "class=\"obit_name_and_date", "<h2"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }

            // Dates
            // Available but not necessary as already read in
            if (conditionalMoveTo("<h4", "</div>", 2))
            {
                tempUC = readNextBetween(BRACKETS);
                if (tempUC.left(7) == OQString("Passed "))
                {
                    tempUC.dropLeft(7);
                    tempUC.processDateField(dfDOD, doMDY);
                }
                else
                    fillInDatesStructured(tempUC);
            }
            break;

        case 1:
            if (consecutiveMovesTo(5000, "obit-container", ">Obituary For "))
            {
                tempUC = getUntil("<");
                tempUC.prepareStructuredNames();
            }
            break;
        }
    }
        break;

    case Crew:
    {
        if (consecutiveMovesTo(2500, ">We Remember<", "<h4"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }

        if (moveTo("<h5", 100))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.replace("January 1, 1970 &#8212; January 1, 1970", "", Qt::CaseInsensitive);
            tempUC.replace("January 1, 1970 &#8212;", "", Qt::CaseInsensitive);
            tempUC.replace("&#8212; January 1, 1970", "", Qt::CaseInsensitive);
            tempUC.replace("January 10, 2021 &#8212;", "", Qt::CaseInsensitive);
            fillInDatesStructured(tempUC);
        }
    }
        break;

    case Jelly:
    {
        if (consecutiveMovesTo(50, "class=\"text-center  custom  font-family-h1", "\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }

        if (consecutiveMovesTo(50, "class=\"text-center  custom  font-family-text", "\""))
        {
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);
        }
    }
        break;

    case Nirvana:
    {
        if (consecutiveMovesTo(100, ">Home <", "<li"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }

        // Dates
        if (consecutiveMovesTo(50, "Birth:", "strong"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.processDateField(dfDOB);
        }

        if (consecutiveMovesTo(50, "Died:", "strong"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.processDateField(dfDOD);
        }

        if (consecutiveMovesTo(100, "Service Date:", "strong", ", "))
        {
            tempUC = getUntil("<");
            tempUC.processDateField(dfDOS);
        }
    }
        break;

    case Bergetti:
    {
        if (moveTo("post_title entry-title"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }

        // Dates
        if (consecutiveMovesTo(100, "itemprop=\"datePublished\"", "content="))
        {
            tempUC = readNextBetween(QUOTES);
            tempUC.processDateField(dfDOP);
        }
    }
        break;

    case PacificByte:
    {
        if (moveTo("class=\"ct-name\""))
        {
            moveTo("firstname");
            tempUC = readNextBetween(BRACKETS);
            tempUC += PQString(" ");
            backward(100);
            moveTo("lastname");
            tempUC += readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("class=\"ct-dates\"", "</div>", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.simplify();
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case ROI:
    {
        if (consecutiveMovesTo(1000, "id=\"main\"", "entry-title", "href", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("<span", "entry-content", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.replaceHTMLentities();
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case Vernon:
    {
        if (consecutiveMovesTo(2000, "id=\"main-content\"", "entry-title"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case Gustin:
    {
        if (consecutiveMovesTo(300, "class=\"wpfh_main_obit\"", "href", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        else
        {
            beg();
            if (consecutiveMovesTo(300, "id=\"wpfh_main_obit\"", "href", ">"))
            {
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
        }

        if (conditionalMoveTo("class=\"wpfh_obit_date\"", "</div>", 0))
        {
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);
        }
    }
        break;

    case Ashlean:
    {
        if (consecutiveMovesTo(1500, "class=\"site-content\"", "elementor-heading-title", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            consecutiveMovesTo(200, "class=\"ae-element-post-date\"", "ae-element-post-date", "date");
            tempUC = readNextBetween(BRACKETS);
            tempUC.processDateField(dfDOP);
        }
    }
        break;

    case Halifax:
    {
        if (consecutiveMovesTo(500, "id=\"main-content\"", "entry-title"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }

        // Dates
        if (conditionalMoveTo("class=\"published\"", "</div>"))
        {
            tempUC = readNextBetween(QUOTES);
            tempUC.processDateField(dfDOP);
        }
    }
        break;

    case Specialty:
    {
        switch (style)
        {
        default:
        case 0:
            switch(globals->globalDr->getProviderKey())
            {
            case 1:
            case 3:
                if (moveTo("class=\"name\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.prepareStructuredNames();

                    if (moveTo("class=\"dates\""))
                    {
                        tempUC = readNextBetween(BRACKETS);
                        tempUC.processStructuredYears();
                    }
                }
                break;

            default:
                if (consecutiveMovesTo(50, "class=\"contentPanel\"", "<h"))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.prepareStructuredNames();

                    if (moveTo("<h2", 50))
                    {
                        tempUC = readNextBetween(BRACKETS);
                        tempUC.processStructuredYears();
                    }
                }
                else
                {
                    tempUC = globals->globalDr->getTitle();
                    tempUC.prepareStructuredNames();
                }
                break;
            }
            break;

        case 1:
            if (moveTo("class=\"name\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("class=\"dates\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processStructuredYears();
                }
            }
            break;
        }
   }
        break;

    case ReneBabin:
    {
        if (consecutiveMovesTo(200, "class=\"post_title entry-title\"", "</span"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("class=\"post_info_date\"");
            tempUC = readNextBetween(BRACKETS);
            tempUC.processDateField(dfDOP);
        }
    }
        break;

    case Smiths:
    {
        // Only name available
        if (consecutiveMovesTo(250, "<body>", "<span", ">"))
        {
            tempUC = getUntil("<");
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case Seabreeze:
    {
        // Part of group of obits
        target = QString("thankyou") + globals->globalDr->getID().getString();
        if (moveTo(target))
        {
            moveBackwardTo("data-role=\"collapsible\"");
            consecutiveMovesTo(25, "<span", ">");
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("<span", "<img", 0, 100))
            {
                moveTo(">");
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }
        }
    }
        break;

    case RedSands:
    {
        if (consecutiveMovesTo(100, "class=\"entry-title\"", "headline", "href"))
        {
            moveTo(">");
            tempUC = getUntilEarliestOf(" &#8211; ", "</a>", 100);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case AdGraphics:
    {
        // Part of group of obits
        target = QString("images/") + globals->globalDr->getID().getString() + QString(".");
        if (moveTo(target))
        {
            moveBackwardTo("<tr>", 7500);
            consecutiveMovesTo(150, "<td align", ">");
            tempUC = getUntilEarliestOf("</strong>", "</p>");
            tempUC.removeHTMLtags();
            tempUC.cleanUpEnds();
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case Websites:
    {
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case MPG:
    {
        if (consecutiveMovesTo(100, ">In memory of<", "<strong"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }

        if (conditionalMoveTo("itemprop=\"birthDate\"", "<div", 0))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.processDateField(dfDOB);
        }

        if (conditionalMoveTo("itemprop=\"deathDate\"", "<div", 0))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.processDateField(dfDOD);
        }
    }
        break;

    case MediaHosts:
    {
        if (consecutiveMovesTo(400, "class=\"container container-text\"", "<h"))
        {
            moveTo(">");
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case DragonFly:
    {
        if (consecutiveMovesTo(100, "id=\"Obituarytitle\"", "</div>", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<p", 50))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }
        }
    }
        break;

    case MyFavourite:
    {
        if (consecutiveMovesTo(200, "class=\"entry-title\"", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(250, "<!--", "<!--", ", "))
            {
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOD);
            }
        }
    }
        break;

    case Coop:
    {
        if (moveTo("class=\"obituary-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }

        if (moveTo("class=\"obituary-years\""))
        {
            DATES dates;
            tempUC = readNextBetween(BRACKETS);
            tempUC.processStructuredYears();
        }
    }
        break;

    case EverTech:
    {
        if (moveTo("<title>Memorial - "))
            tempUC = getUntil("</title>");
        tempUC.prepareStructuredNames();
    }
        break;

    case MacLean:
    {
        // Part of group of obits
        target = QString("modal_") + globals->globalDr->getID().getString();
        if (moveTo(target))
        {
            moveBackwardTo("<header>", 500);
            moveTo("<h");
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("class=\"lifetime\"", "Service Date:", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }

            if (consecutiveMovesTo(100, "Service Date:", ">"))
            {
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOS);
            }
        }
    }
        break;

    case MCG:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(1000, "id=\"cwp_funerals_details\"", "class=\"details\"", "<h"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (consecutiveMovesTo(100, "Passing Date:", ", "))
                {
                    tempUC = getUntil("<");
                    tempUC.processDateField(dfDOD);
                }

                if (consecutiveMovesTo(100, "Funeral Date:", ", "))
                {
                    tempUC = getUntil("<");
                    tempUC.processDateField(dfDOS);
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(175, ">Name:<", "class=\"detail_info_funerals\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }

            if (consecutiveMovesTo(175, ">Passing Date:<", "class=\"detail_info_funerals\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }
            break;
        }
    }
        break;

    case UNI:
    {
        if (moveTo("<>"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case WebSolutions:
    {
        if (moveTo("class=\"card-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("class=\"float-right\"", "class=\"list-group-item\"", 1))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }

            if (conditionalMoveTo("Date of Death:", "class=\"list-group-item\"", 1))
            {
                moveTo("</b");
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }
        }
    }
        break;

    case Citrus:
    {
        if (moveTo("jet-listing-dynamic-field__content"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("jet-listing-dynamic-field__content");
            tempUC = readNextBetween(BRACKETS);
            tempUC.processStructuredYears();
        }
    }
        break;

    case TNours:
    {
        if (consecutiveMovesTo(100, "class=\"avis_page", "class=\"text-center\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("class=\"button button-date\"");
            tempUC = getUntil("</div>");
            tempUC.replace("<br>", " ", Qt::CaseSensitive);
            fillInDatesStructured(tempUC);
        }
    }
        break;

    case SandFire:
    {
        if (consecutiveMovesTo(200,"class=\"container\"", "page-banner-text", "<small"))
        {
            moveBackwardTo("<h");
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("small>"))
            {
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case Carve:
    {
        if (consecutiveMovesTo(50, "meta name=\"title\"", "content="))
        {
            tempUC = readQuotedMetaContent();
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case BrandAgent:
    {
        if (consecutiveMovesTo(300, "class=\"ptb_post_title ptb_entry_title\"", "href", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(200, ">Death<", "datetime="))
            {
                tempUC = readNextBetween(QUOTES);
                fillInDatesStructured(tempUC);
            }

            if (consecutiveMovesTo(200, ">Funeral<", "datetime="))
            {
                tempUC = readNextBetween(QUOTES);
                tempUC.processDateField(dfDOS);
            }
        }
    }
        break;

    case EPmedia:
    {
        if (moveTo("class=\"post_title entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(100, "</noscript>", "</p>", "<p", ">"))
            {
                tempUC = getUntil("<");
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case Linkhouse:
    {
        if (consecutiveMovesTo(20, "<header>", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"post-date\""))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case LinkWeb:
    {
        if (consecutiveMovesTo(50, "class=\"uk-panel\"", "class=\"uk-panel-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case JoshPascoe:
    {
        if (moveTo("class=\"contentheading\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case ChadSimpson:
    {
        if (consecutiveMovesTo(200, "class=\"flex-1", "class=\"text-center", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("<span class", "<div class", 0))
            {
                moveTo(">");
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case MarketingImages:
    {
        if (consecutiveMovesTo(50, "class=\"details\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case ESite:
    {
        switch(globals->globalDr->getProviderKey())
        {
        case 1:
            if (consecutiveMovesTo(500, "<div id=\"container\"", "<h"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
            break;

        case 2:
            if (consecutiveMovesTo(2500, "id=\"main-content\"", "<h1", ">"))
            {
                tempUC = getUntil("<");
                tempUC.prepareStructuredNames();

                if (consecutiveMovesTo(150, "<h3", ">"))
                {
                    tempUC = getUntil("<");
                    tempUC.processDateField(dfDOD);
                }
            }
            break;
        }
    }
        break;

    case SquareSpace:
    {
        if (consecutiveMovesTo(100, "class=\"BlogItem-title\"", "field=\"title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case Eggs:
    {
        //
        if (moveTo("class=\"posttitle\""))
        {
            moveTo("<h", 100);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case MFH:
    {
        if (consecutiveMovesTo(50, "class=\"deceased-info\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<h", 50))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case ONeil:
    {
        if (consecutiveMovesTo(100, "id=\"obitHeader\"", "<h"))
        {
            QString datesRemoved, justDates;
            tempUC = readNextBetween(BRACKETS);
            tempUC.splitComponents(datesRemoved, justDates);

            tempUC = datesRemoved;
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case Back2Front:
    {
        if (moveTo("class=\"app.tbl.first_name\""))
        {
            tempUC = readNextBetween(BRACKETS);
            backward(200);
            moveTo("class=\"app.tbl.last_name\"");
            tempUC = tempUC + PQString(" ") + readNextBetween(BRACKETS);

            tempUC.prepareStructuredNames();
        }
    }
        break;

    case InView:
    {
        if (consecutiveMovesTo(100, ">Obituaries<", "class=\"trail-end\""))
            tempUC = readNextBetween(BRACKETS);
        else
            tempUC = globals->globalDr->getTitle();

        tempUC.prepareStructuredNames();
    }
        break;

    case CreativeOne:
    {
        if (moveTo("<>"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case RKD:
    {
        if (!moveTo("Send a Private Message Of Condolence"))
        {
            beg();
            moveTo("Send a private message of condolence");
        }

        if (moveToEarliestOf("Date of Death:", "Date of Passing:"))
        {
            moveTo("</strong");
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);

            if (consecutiveMovesTo(50, "name=\"referer_title\"", "value="))
            {
                tempUC = readNextBetween(QUOTES);
                tempUC.prepareStructuredNames();

            }
        }
        else
        {
            beg();
            if (moveTo("h1 class=\"elementor-heading-title elementor-size-default"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveToEarliestOf("Date of Death:", "Date of Passing:"))
                {
                    moveTo("</strong");
                    tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);
                }
            }
            else
            {

            }
        }
    }
        break;

    case SDP:
    {       
        if (consecutiveMovesTo(100, "<div class=\"fltlft content50\">", "<h3"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            int position = getPosition();

            if (moveTo("br /", 5))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD, doYMD);
            }
            else
            {
                beg();
                forward(position);
            }

            if (moveTo("<div style=", 150))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
            else
            {
                beg();
                forward(position);
                moveTo("<p align=\"center\"");
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case Globalia:
    {
        if (moveTo("class=\"single-nom\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"single-date\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case Vortex:
    {
        switch(style)
        {
        case 0:
            if (moveTo("class=\"h2 disparu-titre\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("class=\"disparu-date\"", 100))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processStructuredYears();
                }
            }
            /*if (moveTo("class=\"tlt-ficheDeces\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("<h2", 100))
                {
                    DATES dates;
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processStructuredYears();
                }
            }*/
            break;

        case 1:
            if (consecutiveMovesTo(250, "class=\"divDroite\"", "class=\"h3\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("class=\"h3 dateNaissance\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processStructuredYears();
                }
                break;
            }
            break;

        case 2:
            if (moveTo("class=\"entry-title\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("class=\"single-header-year\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processStructuredYears();
                }
                break;
            }
        }
    }
        break;

    case Elegant:
    {
        if (moveTo("class=\"post_title entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempTempUC = tempUC.right(9);
            if ((tempTempUC.getString().at(4) == QString("-")) && tempUC.getString().at(tempUC.getLength() - 10) == QString(" "))
                tempUC.dropRight(10);
            tempUC.prepareStructuredNames();

            tempTempUC.processStructuredYears();
        }
    }
        break;

    case YellowPages:
    {
        switch(style)
        {
        case 0:
            target = QString("id=\"title") + globals->globalDr->getID().getString();
            if (consecutiveMovesTo(75, target, ">"))
            {
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("<span", 100))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processStructuredYears();
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(100, "class=\"page-header\"", "itemprop=\"headline\"", ">"))
            {
                tempUC = getUntil("<");
                tempUC.cleanUpEnds();
                QString tempString = tempUC.getString();

                rxTarget.setPattern("(\\d{4})\\s+?(-|–)\\s+?(\\d{4})");
                match = rxTarget.match(tempString);
                if (match.hasMatch())
                {
                    tempTempUC = OQString(match.captured(1)) + OQString("-") + OQString(match.captured(3));
                    tempTempUC.processStructuredYears();
                    tempString.replace(rxTarget, "");
                    tempUC = tempString;
                    tempUC.cleanUpEnds();
                }

                tempUC.removeLeading("Remerciements");
                tempUC.cleanUpEnds();
                tempUC.removeEnding("-");
                tempUC.removeEnding(QChar(8212));
                tempUC.removeLeading("- ");
                tempUC.prepareStructuredNames();
            }
            break;
        }
    }
        break;

    case Shooga:
    {
        if (moveTo("class=\"blog-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("<p", "</div>", 0))
            {
                conditionalMoveTo("<br", "</div>", 0);
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case NBL:
    {
        if (consecutiveMovesTo(100, "class=\"unit-75 conteneur-avis\"", "class=\"titre-deces\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<p><strong", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();

                if (moveTo("Décédé(e) le ", 100))
                {
                    tempUC = getUntil(",");
                    tempUC.processDateField(dfDOD);
                }
            }
        }
    }
        break;

    case WPBakery:
    {
        if (consecutiveMovesTo(50, "class=\"entry-title", "fusion-post-title"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(30, "fusion-text fusion-text-1", "<p"))
            {
                tempUC = readNextBetween(BRACKETS);
                int index = tempUC.findPosition("(");
                if (index >= 0)
                {
                    tempTempUC = tempUC.right(tempUC.getLength() - index);
                    tempTempUC.removeBookEnds(PARENTHESES);
                    PQString age = tempTempUC.getWord();
                    if (age.isNumeric())
                        globals->globalDr->setAgeAtDeath(static_cast<unsigned int>(age.asNumber()));
                    tempUC.dropRight(tempUC.getLength() - index);
                }

                tempUC.readYears();
            }
        }

    }
        break;

    case Imago:
    {
        if (moveTo("class=\"avis-titre\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }

        if (moveTo("class=\"deces-date\">"))
        {
            tempUC = getUntilEarliestOf(" (", "<");
            tempUC.processStructuredYears();

            tempUC = getUntilEarliestOf(" )", "<");
            tempUC.replace("Age ", "", Qt::CaseInsensitive);
            tempUC.replace(" ans", "", Qt::CaseInsensitive);
            tempUC.cleanUpEnds();
            tempUC.processAgeAtDeath();
        }
    }
        break;

    case TroisJoueur:
    {
        if (moveTo("<>"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case Ubeo:
    {
        switch(style)
        {
        case 0:
            if (moveTo("class=\"margB_3 for_638\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("class=\"deces_annees for_638\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.removeBookEnds(PARENTHESES);
                    tempUC.processStructuredYears();
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(50, "class=\"avis_main_infos\"", "<h"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (consecutiveMovesTo(5, "Né", "le "))
                {
                    QList<QDate> dateList;
                    tempUC = getUntil("<", 50, true);
                    if (tempUC.pullOutDates(globals->globalDr->getLanguage(), dateList, 1, cleanString, true))
                         globals->globalDr->setDOB(dateList.takeFirst());
                }

                if (consecutiveMovesTo(10, "Décédé", "le "))
                {
                    QList<QDate> dateList;
                    tempUC = getUntil("<", 50, true);
                    if (tempUC.pullOutDates(globals->globalDr->getLanguage(), dateList, 1, cleanString, true))
                         globals->globalDr->setDOD(dateList.takeFirst());
                }

                if (conditionalMoveTo("à l'âge de ", "</div>"))
                {
                    tempUC = getUntil(" ");
                    tempUC.processAgeAtDeath();
                }
            }
            break;
        }
    }
        break;

    case Acolyte:
    {
        if (moveTo("<>"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case Morin:
    {
        if (consecutiveMovesTo(100, "Retour aux avis de décès", "<h1", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(25, "<h2", ">"))
            {
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case Taiga:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(50, "class=\"obituary-details-right\"", "<h"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
            break;

        case 1:
            if (consecutiveMovesTo(400, "<!-- Fiche de l'avis -->", "<h2", ">"))
            {
                tempUC = getUntil("<");
                tempUC.prepareStructuredNames();

                if (moveTo("Décédé(e) le  "))
                {
                    tempUC = getUntil("<");
                    tempUC.processDateField(dfDOD);
                }
            }
            break;
        }
    }
        break;

    case Zonart:
    {
        if (moveTo("<>"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case PubliWeb:
    {
        switch (style)
        {
        case 0:
            if (moveTo("class=\"date\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);

                if (moveTo("class=\"f-title\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.prepareStructuredNames();
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(4000, "<!--Avis (1 par page) -->", "class=\"blanc18pt\"><b"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
            break;
        }
    }
        break;

    case DirectImpact:
    {
        if (consecutiveMovesTo(500, "class=\"w-100 text-center obituary-header\"", "<span"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.replace(QString("."), QString("/"));
            tempUC.processDateField(dfDOB, doDMY);

            conditionalMoveTo("</span", "<h", 0);
            tempUC = readNextBetween(BRACKETS);
            tempUC.replace(QString("."), QString("/"));
            tempUC.processDateField(dfDOD, doDMY);

            moveTo("<h", 50);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case SoleWeb:
    {
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(50, "class=\"vsel-image-info\"", "<p"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case Voyou:
    {
        switch (style)
        {
        case 0:
            if (consecutiveMovesTo(100, "class=\"avis-deces-title\"", "<h"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveToEarliestOf("<span class=\"date\"", "class=\"body\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processStructuredYears();
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(100, "class=\"avis-deces-title\"", "<h"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("<span class=\"date\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processStructuredYears();
                }
            }
            break;

        case 2:
            if (moveTo("class=\"vy_deces_details_nom uk-h3"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("class=\"vy_deces_details_date", 200))
                {
                    tempUC = readNextBetween(BRACKETS);
                    int index1, index2;
                    PQString age;
                    index1 = tempUC.findPosition(" de ");
                    if (index1 > 0)
                    {
                        index2 = tempUC.findPosition(" ", 1, index1 + 1);
                        age = tempUC.middle(index1 + 4, index2 - (index1 + 4));
                        globals->globalDr->setAgeAtDeath(age.asNumber());
                        tempUC.dropRight(tempUC.getLength() - index1);
                    }
                    index1 = tempUC.findPosition(" le ");
                    if (index1 > 0)
                        tempUC.dropLeft(index1 + 3);
                    tempUC.processDateField(dfDOD);
                }
            }
        }
    }
        break;

    case Scrypta:
    {
        if (globals->globalDr->getProviderKey() == 1)
        {
            if (consecutiveMovesTo(100, "class=\"TGO06__header--title", ">"))
            {
                tempUC = getUntil("</div>");
                tempUC.prepareStructuredNames();

                if (consecutiveMovesTo(100, "class=\"TGO06__header--date", ">"))
                {
                    tempUC = getUntil("</div>");
                    tempUC.processStructuredYears();
                }
            }
        }
        else
        {
            if (consecutiveMovesTo(1000, "class=\"obituaries-details\"", "<span"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if(moveTo("<br", 20))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processStructuredYears();
                }
            }
        }
    }
        break;

    case Jaimonsite:
    {
        if (consecutiveMovesTo(50, "og:title", "content="))
        {
            tempUC = readQuotedMetaContent();
            QString tempString = tempUC.getString();
            int index = tempString.indexOf(" | ");
            if (index > 0)
            {
                tempString.chop(tempUC.getLength() - index);
                index = tempString.lastIndexOf("-");
                if (index == (tempString.length() - 5))
                {
                    tempTempUC = tempString.right(9);
                    tempTempUC.processStructuredYears();
                    tempString.chop(10);  //?
                    tempUC = tempString;
                }
            }
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case Saguenay:
    {
        if (consecutiveMovesTo(2000, "class=\"content-page\"", "<h2"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<h5", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                if ((tempUC.getLength() == 11) && (tempUC.middle(5,1) == PQString("-")))
                    tempUC.processStructuredYears();
                else
                    tempUC.processDateField(dfDOD);
            }
        }
    }
        break;

    case Lithium:
    {
        if (consecutiveMovesTo(50, "class=\"details\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<span", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }
        }
    }
        break;

    case Cameleon:
    {
        if (consecutiveMovesTo(750, "<!-- HEADER -->", "class=\"date\"", "<p"))
        {
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);

            if (consecutiveMovesTo(50, "class=\"nom\"", "<p"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
        }
    }
        break;

    case LogiAction:
    {
        switch (style)
        {
        case 0:
            if (consecutiveMovesTo(1000, "class=\"content\"", "<h", ">"))
            {
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if ((globals->globalDr->getYOB() == 0) || (globals->globalDr->getYOD() == 0))
                {
                    if (moveTo("<span", 25))
                    {
                        tempUC = readNextBetween(BRACKETS);
                        tempUC.processStructuredYears();
                    }
                }

                if (conditionalMoveTo(">Décédé", "</strong>", 0))
                {
                    moveTo(": ", 25);
                    tempUC = getUntil(" à l'âge de ");
                    tempUC.processDateField(dfDOD);

                    tempUC = getWord();
                    tempUC.processAgeAtDeath();

                    if (consecutiveMovesTo(100, ">Né", " le", ">"))
                    {
                        tempUC = readNextBetween(BRACKETS);
                        tempUC.processDateField(dfDOB);
                    }
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(500, "<h1", "href=", ">"))
            {
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if ((globals->globalDr->getYOB() == 0) || (globals->globalDr->getYOD() == 0))
                {
                    if (moveTo("<span", 25))
                    {
                        tempUC = readNextBetween(BRACKETS);
                        tempUC.processStructuredYears();
                    }
                }

                if (conditionalMoveTo(">Décédé", "</strong>", 0))
                {
                    moveTo(": ", 25);
                    tempUC = getUntil(" à l'âge de ");
                    tempUC.processDateField(dfDOD);

                    tempUC = getWord();
                    tempUC.processAgeAtDeath();

                    if (consecutiveMovesTo(100, ">Né", " le", ">"))
                    {
                        tempUC = readNextBetween(BRACKETS);
                        tempUC.processDateField(dfDOB);
                    }
                }
            }
            break;
        }
     }
        break;

    case BLsolutions:
    {
        if (moveTo("class=\"post_title entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempTempUC = tempUC.right(9);
            if (tempTempUC.getString().indexOf("-") == 4)
            {
                tempUC.dropRight(10);
                tempTempUC.processStructuredYears();
            }
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"post_info_date\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOP);
            }
        }
    }
        break;

    case ToraPro:
    {
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("id=\"dob-dod\""))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case Axanti:
    {
        if (consecutiveMovesTo(100, "class=\"entry-title-wrap\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"entry-date\""))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case ADN:
    {
        if (consecutiveMovesTo(100, "class=\"obituary-viewer__container", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"obtiuary-viewer__date\"", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case B367:
    {
        if (consecutiveMovesTo(100, "class=\"page-banner-text\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            forward(4);
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);
        }
    }
        break;

    case Tomahawk:
    {
        if (moveTo("class=\"avis-titre\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("class=\"deces-date\"");
            tempUC = readNextBetween(BRACKETS);
            int index = tempUC.getString().indexOf("(");
            if (index > 0)
            {
                tempTempUC = tempUC.right(tempUC.getLength() - index);
                tempUC.dropRight(tempUC.getLength() - index + 1);
                tempUC.processStructuredYears();

                if (tempTempUC.removeBookEnds(PARENTHESES))
                {
                    tempUC = tempTempUC.getWord();
                    tempUC.processAgeAtDeath();
                }
            }
        }

        if (consecutiveMovesTo(200, "class=\"posted-on\"", "datetime=\""))
        {
            tempUC = getUntil("T");
            tempUC.processDateField(dfDOP, doYMD);
        }
    }
        break;

    case Caza:
    {
        if (consecutiveMovesTo(500, "id=\"avisdeces\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(500, "class=\"row deces\"", "<h", ">"))
            {
                tempUC = getUntil(" |");
                tempUC.processStructuredYears();

                if (consecutiveMovesTo(25, "décédé", " le "))
                {
                    tempUC = getUntil("<");
                    tempUC += QString(" ");
                    tempUC += QString::number(globals->globalDr->getYOD());
                    tempUC.processDateField(dfDOD);
                }
            }
        }
    }
        break;

    case Tegara:
    {
        if (moveTo("class=\"name-avis\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("class=\"infos-carte\"");
            tempUC = readNextBetween(QUOTES);
            tempUC.processDateField(dfDOD);

            moveTo("class=\"ann-e\"");
            tempUC = readNextBetween(BRACKETS);
            tempUC.processStructuredYears();
        }
    }
        break;

    case NMedia:
    {
        if (consecutiveMovesTo(1000, "InventoryProductBroker_CF_407b7048-899b-2a2e-e230-7ae2d781f3f9", ">"))
        {
            tempUC = getUntil("<") + OQString(", ");
            if (consecutiveMovesTo(1000, "InventoryProductBroker_CF_e165c184-157a-1bae-454b-208ff2d84047", ">"))
                tempUC += getUntil("<");
            if (consecutiveMovesTo(1000, "InventoryProductBroker_CF_e1e5eebc-a3dc-9c0f-845f-6dadc41c353e", ">"))
                tempUC += OQString(" (") + getUntil("<") + OQString(")");
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(1000, "InventoryProductBroker_CF_f2fec77b-53a8-484f-1526-0b7638c241a8", ">"))
            {
                tempUC = getUntil("<");
                tempUC.processYearField(dfDOB);
            }

            if (consecutiveMovesTo(1000, "InventoryProductBroker_CF_17d538cf-a248-d961-d652-4a1db6d995a4", ">"))
            {
                tempUC = getUntil("<");
                tempUC.processYearField(dfDOD);
            }
        }
    }
        break;

    case Webs:
    {
        if (consecutiveMovesTo(5000, "<div class=\"w-text", "</div>"))
        {
            int position1, position2;

            if (moveBackwardTo("- ", 100))
            {
                moveBackwardTo("\"");
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }

            backward(10);
            position1 = getPosition() - 10;

            moveBackwardTo("</span>");
            moveBackwardTo("wz-bold");
            moveTo(">");
            OQString singleChar = peekAtNext(1);
            if (!(singleChar.isAlpha() && singleChar.isAllCaps()))
            {
                moveTo("<span");
                moveTo(">");
            }
            backward(1);
            position2 = getPosition();

            if (position2 < position1)
                tempUC = readNextBetween(BRACKETS);
            else
            {
                beg();
                if (consecutiveMovesTo(200, "class=\"w-text", "wz-bold"))
                    tempUC = readNextBetween(BRACKETS);
            }
            if (tempUC.getLength() > 0)
                tempUC.prepareStructuredNames();
        }
    }
        break;

    case Descary:
    {
        if (moveTo("class=\"post_title entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("itemprop=\"datePublished\"", "class=\"post_content\"", 0))
            {
                moveTo("content=");
                tempUC = readNextBetween(QUOTES);
                tempUC.processDateField(dfDOP, doYMD);
            }

            if (consecutiveMovesTo(100, "class=\"post_content\"", "<p"))
            {
                conditionalMoveTo("<em", "<p", 0);
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case Tonik:
    {
        if (moveTo("class=\"page-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("class=\"date text-center\"");
            tempUC = readNextBetween(BRACKETS);
            tempUC.processDateField(dfDOD);
        }
    }
        break;

    case Kaleidos:
    {
        if (consecutiveMovesTo(100, "class=\"description-fiche tab-content\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"date-avis\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case Gemini:
    {
        if (moveTo("class=\"omc-post-heading-standard\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempTempUC = tempUC.right(11);
            if (tempTempUC.removeBookEnds(PARENTHESES))
            {
                tempUC.dropRight(12);
                tempTempUC.processStructuredYears();
            }

            tempUC.prepareStructuredNames();
        }
    }
        break;

    case Alias:
    {
        if (moveTo("class=\"printonly\"><em"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case Cible:
    {
        if (moveTo("class=\"detail_nom\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"detail_date\""))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case Web2u:
    {
        if (moveTo("<>"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case District4Web:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(150, "class=\"project-content\"", "class=\"entry-title rich-snippet-hidden\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("class=\"updated rich-snippet-hidden\">"))
                {
                    tempUC = getUntil("T");
                    tempUC.processDateField(dfDOP, doYMD);

                    if (consecutiveMovesTo(200, "class=\"fusion-text fusion-text-1\"", "<h5", ">"))
                    {
                        backward(1);
                        tempUC = readNextBetween(BRACKETS);
                        fillInDatesStructured(tempUC);
                    }
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(100, "class=\"entry-title\"", "itemprop=\"name\"", ">"))
            {
                tempUC = getUntilEarliestOf(" of ", "<");
                tempUC.prepareStructuredNames();
            }
            break;
        }

    }
        break;

    case Cake:
    {
        if (moveTo("class=\"h1 title margin-none\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case J27:
    {
        if (moveTo("itemprop=\"headline\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveToEarliestOf("Naissance:", "Birth:"))
            {
                moveTo(">");
                tempUC = getUntilEarliestOf("Décès:", "Death:");
                runStdProcessing(tempUC, false);
                tempUC.processDateField(dfDOB);

                moveTo(">");
                tempUC = getUntil("</h4>");
                runStdProcessing(tempUC, false);
                tempUC.processDateField(dfDOD);
            }
            else
            {
                beg();
                consecutiveMovesTo(100, "class=\"article-content\"", "itemprop=\"articleBody\"", "<h4>");
                tempUC = getUntil("</h");
                runStdProcessing(tempUC, false);
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case NetRevolution:
    {
        if (moveTo("class=\"print-page\""))
        {
            moveTo("class=\"elementor-heading-title elementor-size-default\"");
            tempUC = readNextBetween(BRACKETS);
            int index1, index2;
            QString tempString = tempUC.getString();
            index1 = tempString.indexOf(",");
            index2 = tempString.lastIndexOf(",");
            if (index1 != index2)
                tempUC = tempString.left(index1) + tempString.right(tempString.length() - index1 - 1);
            tempUC.prepareStructuredNames();

            moveTo("class=\"elementor-heading-title elementor-size-default\"");
            tempUC = readNextBetween(BRACKETS);
            tempUC.processStructuredYears();

            moveTo("class=\"jet-listing-dynamic-field__content\"");
            tempUC = readNextBetween(BRACKETS);
            tempUC.processDateField(dfDOD);
        }
    }
        break;

    case ImageXpert:
    {
        if (moveTo("id=\"page-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case Reactif:
    {
        if (moveTo("class=\"no-margin\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempTempUC = tempUC.right(9);
            if (tempTempUC[4] == QChar('-'))
            {
                tempUC.dropRight(10);
                tempTempUC.processStructuredYears();
            }
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case Boite:
    {
        if (consecutiveMovesTo(500, "class=\"wpfh-print-output-box\"", "href="))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"wpfh_obit_date\""))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case Orage:
    {
        if (consecutiveMovesTo(50, "class=\"col-txt fcomplete\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<span", 25))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case Kerozen:
    {
        switch(style)
        {
        case 0:
            if (moveTo("class=\"title text-center\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("class='text-center'><strong>", 50))
                {
                    tempUC = getUntil(" ");
                    tempUC.processAgeAtDeath();
                }

                if (moveTo("class=\"title--grey-small text-center\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processStructuredYears();
                }
            }
            break;

        case 1:
            if (consecutiveMovesTo(300, "class=\"deces\"", "<h2", ">"))
            {
                tempUC = getUntil("<");
                tempUC.prepareStructuredNames();

                if (moveTo("<h3"))
                {
                    tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);
                }

                if (consecutiveMovesTo(50, "<h5>", "à l'âge de  "))
                {
                    tempUC = getWord();
                    tempUC.processAgeAtDeath();
                }
            }
            break;
        }
    }
        break;

    case InoVision:
    {
        if (consecutiveMovesTo(150, "id=\"article-title\"", "<strong" ))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("<p", "</div>", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case FRM:
    {
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(250, ">In Memory of <", "title=\"Permanent Link:", ">"))
            {
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                tempUC.removeTrailingLocation();
                tempUC.prepareStructuredNames();
            }
            break;

        case 1:
            if (consecutiveMovesTo(50, "class=\"entry-title\"", "itemprop=\"name\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.removeTrailingLocation();
                tempUC.prepareStructuredNames();

                if (conditionalMoveTo("class=\"has-text-align-center\"", "<p>", 0))
                {
                    tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);
                }
            }
            break;

        case 2:
        case 3:
        case 4:
            if (moveTo("class=\"entry-title\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
            break;
        }
    }
        break;

    case PassageCoop:
    {
        if (consecutiveMovesTo(200, "Loving Memory Of<", "<b"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("<span");
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);
            //tempUC.processDateField(dfDOD);
        }
    }
        break;

    case JBCote:
    {        
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(1000, "class=\"obituaries-announcement-detail\"", "<h", ">"))
            {
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                moveTo("<h");
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
            break;

        case 1:
            if (consecutiveMovesTo(400, "class=\"avis\"", "<td valign=\"top\">", "<b"))
            {
                tempUC = readNextBetween(BRACKETS);
                if (conditionalMoveTo("class=\"nom\"", "<p>", 0))
                    tempUC += PQString(" ") + readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (moveTo("<p", 200))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processStructuredYears();
                }
            }
            break;
        }
    }
        break;

    case BlackCreek:
    {
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case CityMax:
    {
        target = OQString(globals->globalDr->getID()).convertFromID();
        if (moveTo(target))
        {
            moveBackwardTo("<h");
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case SYGIF:
    {
        if (moveTo("class=\"title\" id=\"page-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            consecutiveMovesTo(100, "class=\"title\"", "<li>");
            if (conditionalMoveTo("content=\"", "</li>"))
            {
                tempUC = getUntil("T");
                tempUC.processDateField(dfDOB, doYMD);
            }

            if (conditionalMoveTo("content=\"", "</li>"))
            {
                tempUC = getUntil("T");
                tempUC.processDateField(dfDOD, doYMD);
            }

            moveTo("</ul>");
            moveBackwardTo("<li>", 200);
            if (moveTo("ge :", 25))
            {
                tempUC = getWord();
                tempUC.processAgeAtDeath();
            }
        }
    }
        break;

    case PortNeuf:
    {
        if (consecutiveMovesTo(100, "class=\"entry-title\"", "itemprop=\"headline\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }

        moveTo("class=\"the_content_wrapper\"");
        if (moveTo("<p><strong", 30))
        {
            if (moveTo("<p><strong", 130))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case Canadian:
    {
        if (moveTo("itemprop=\"headline\""))
        {
            unstructuredContent ucTemp;
            tempUC = readNextBetween(BRACKETS);
            tempUC.fixBasicErrors(true);
            ucTemp.compressCompoundNames(tempUC, globals->globalDr->getLanguage());
            tempUC.removeRepeatedLastName();
        }
        else
            tempUC = globals->globalDr->getTitle();
        tempUC.prepareStructuredNames();
    }
        break;

    case BallaMedia:
    {
        if (consecutiveMovesTo(50, "<h4>In Celebration of</h4>", "<strong"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("itemprop=\"birthDate\"", "</h2>", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOB);
            }

            if (conditionalMoveTo("itemprop=\"deathDate\"", "</h2>", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }
        }
        else
        {
            beg();
            if (consecutiveMovesTo(100, "og:title", "content=\""))
            {
                tempUC = getUntilEarliestOf(" | ", "<");
                tempUC.prepareStructuredNames();
            }
        }
    }
        break;

    case Jac:
    {
        if (consecutiveMovesTo(250, "src=\"images/name.png\"", "<TD", ">"))
        {
            tempUC = getUntil("</TD>");
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(250, "src=\"images/dob.png\"", "<TD", ">"))
            {
                tempUC = getUntil("</TD>");
                tempUC.processDateField(dfDOB);
            }

            if (consecutiveMovesTo(250, "src=\"images/dod.png\"", "<TD", ">"))
            {
                tempUC = getUntil("</TD>");
                tempUC.processDateField(dfDOD);
            }
        }
    }
        break;

    case Ministry:
    {
        tempUC = globals->globalDr->getTitle();
        tempUC.prepareStructuredNames();
    }
        break;

    case Multinet:
    {
        if (moveTo("class=\"h3\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<p", 20))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }

        /*QString target;
        target = OQString(globals->globalDr->getID()).convertFromID();
        if (moveTo(target))
        {
            tempUC = globals->globalDr->getTitle();
            tempUC.prepareStructuredNames();

            moveTo("<br");
            tempUC = readNextBetween(BRACKETS);
            tempUC.processStructuredYears();
        }*/
    }
        break;

    case PropulC:
    {
        if (moveTo("class='_name'"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class='_years'", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case Nexion:
    {
        if (moveTo("id=\"avis-deces-nom\">"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("id=\"avis-deces-dates\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case LCProduction:
    {
        if (consecutiveMovesTo(150, "class=\"wpfh-obit-alternate-title\"", "<h1>"))
        {
            if (moveTo("<a", 10))
                moveTo(">");
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"wpfh-obit-alternate-dates\"", 200))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case Absolu:
    {
        if (consecutiveMovesTo(400, "class=\"avis-deces-informations\"", "class=\"titre\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"annee\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case SuiteB:
    {
        if (consecutiveMovesTo(100, "class=\"content\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"date\"", 25))
            {
                tempUC = readNextBetween(BRACKETS);
                if ((tempUC.getLength() == 9) && (tempUC[4] == "-"))
                    tempUC.processStructuredYears();
                else
                    tempUC.processDateField(dfDOD, doYMD);
            }
        }
    }
        break;

    case Map:
    {
        if (moveTo("class='name'"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("lass='yspan'", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case iClic:
    {
        if (consecutiveMovesTo(750, "id=\"avisdeces-fiche\"", "<h1"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.removeLeading("Madame ");
            tempUC.prepareStructuredNames();

            if (moveTo("<span", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                int index = tempUC.findPosition("(");
                if (index == -1)
                    tempUC.processStructuredYears();
                else
                {
                    tempTempUC = tempUC.right(tempUC.getLength() - index);
                    tempTempUC.cleanUpEnds();
                    tempTempUC.removeBookEnds(PARENTHESES);
                    tempTempUC.removeEnding(" ans");
                    tempTempUC.processAgeAtDeath();

                    tempUC.dropRight(tempUC.getLength() - index + 1);
                    tempUC.processStructuredYears();
                }
            }
        }
    }
        break;

    case Bouille:
    {
    }
        break;

    case Pub30:
    {
    }
        break;

    case Theories14:
    {
        if (moveTo("class=\"pageTitle\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"date\"", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case Techlogical:
    {
        if (consecutiveMovesTo(4000, "<div class=\"post-content\">", "<h2"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<h2", 50))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
    }
        break;

    case GyOrgy:
    {
        if (moveTo("style=\"font-size:24px\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(200, "<p class", "<span", ">"))
            {
                tempUC = getUntil("<");
                fillInDatesStructured(tempUC);
            }
        }
        else
        {
            beg();
            if (moveTo("</h2>"))
            {
                moveBackwardTo("\">", 200);
                tempUC = getUntil("<");
                tempUC.prepareStructuredNames();

                if (moveTo("</h2>"))
                {
                    moveTo("</h2>");
                    moveBackwardTo("\">", 200);
                    tempUC = getUntil("<");
                    fillInDatesStructured(tempUC);
                }
            }
            else
            {
                beg();
                if (moveTo("style=\"font-size:20px;\""))
                {
                    moveTo(">");
                    tempUC = getUntil("</p>");
                    tempUC.prepareStructuredNames();
                }
            }
        }
    }
        break;

    case GemWebb:
    {
        if (moveTo("class=\"fl-heading-text\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case RedChair:
    {
        if (consecutiveMovesTo(1000, "class=\"site-main\"", "class=\"elementor-heading-title"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case ExtremeSurf:
    {
        if (consecutiveMovesTo(50, "id=\"tab11\"", "<strong>"))
        {
            tempUC = getUntilEarliestOf("<", " of ");
            int index = tempUC.findPosition(" - ");
            if (index != -1)
            {
                tempTempUC = tempUC.right(tempUC.getLength() - index - 3);
                tempTempUC.cleanUpEnds();
                if(tempTempUC.removeLeading("Age "))
                    tempTempUC.processAgeAtDeath();
                tempUC.dropRight(tempUC.getLength() - index);
                tempUC.prepareStructuredNames();
            }
            else
            {
                tempUC.prepareStructuredNames();
                if (conditionalMoveTo(" - ", "/strong>"))
                {
                    tempUC = getUntil("<");
                    if(tempUC.removeLeading("Age "))
                        tempUC.processAgeAtDeath();
                }
            }

            if (moveTo("<br", 10))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case Cahoots:
    {
        if (moveTo("class=\"article__title"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(50, "class=\"article__date\"", "datetime=\""))
            {
                tempUC = getUntil("T");
                tempUC.processDateField(dfDOD, doYMD);
            }
        }
    }
        break;

    case Tride:
    {
        if (moveTo("class=\"obituaries-sub-heading\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(50, "class=\"obituaries-heading-content\"", ">"))
            {
                tempUC = getUntil("<");
                tempUC.removeEnding(" -");
                fillInDatesStructured(tempUC);
            }

            if (moveTo(" Age ", 100))
            {
                tempUC = getUntil("<");
                tempUC.processAgeAtDeath();
            }
        }
    }
        break;

    case Jensii:
    {
        if (consecutiveMovesTo(125, "<div class=\"x_\">", "<span"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("<span", "<div class=\"x_\">", 0))
            {
                tempUC = getUntil("</p>");
                tempUC.removeHTMLtags();
                fillInDatesStructured(tempUC);
            }
        }
    }
        break;

    case InterWeb:
    {
        if (moveTo("class=\"wpfh_obit_date\""))
        {
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);

            moveBackwardTo("<h");
            consecutiveMovesTo(150, "href", ">");
            tempUC = getUntil("<");
            tempUC.prepareStructuredNames();
        }
    }
        break;

    case Brown:
    {
        if (moveTo("class=\"d-avis-detail-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("Date de naissance", "Date de décès", 0))
            {
                moveTo(": ", 10);
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOB);
            }

            if (moveTo("Date de décès", 300))
            {
                moveTo(": ", 10);
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOD);
            }
        }
    }
        break;

    case Tukio:
    {
        if (moveTo("<!-- Fancy Name -->"))
        {
            moveTo("<h");
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("<h");
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);
        }
        else
        {
            beg();
            if (consecutiveMovesTo(200, "class=\"decedent_info\"", "itemprop=\"name\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (conditionalMoveTo("itemprop=\"birthDate\"", "itemprop=\"deathDate\"", 0))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processDateField(dfDOB);
                }

                if (conditionalMoveTo("itemprop=\"deathDate\"", "<div>, 0"))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processDateField(dfDOD);
                }
            }
            else
            {
                beg();
                if (consecutiveMovesTo(200, "class=", "obituary-text", "<h", ">"))
                {
                    tempUC = getUntil("<");
                    tempUC.prepareStructuredNames();

                    if (moveTo("<h2", 100))
                    {
                        tempUC = readNextBetween(BRACKETS);
                        fillInDatesStructured(tempUC);
                    }
                }
            }
        }
    }
        break;

    case Etincelle:
    {
        if (consecutiveMovesTo(100, "class=\"nom\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            int index = tempUC.findPosition(", MESSE");
            if (index > 0)
                tempUC = tempUC.left(index);
            if (tempUC.left(20) == PQString("MESSE ANNIVERSAIRE, "))
                tempUC.dropLeft(20);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("<p", "</div>", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                if (tempUC.left(4) == OQString("0000"))
                {
                    tempUC.dropLeft(5);
                    tempUC.processYearField(dfDOD);
                }
                else
                    tempUC.processStructuredYears();
            }

            if (consecutiveMovesTo(50, "class=\"date\"", "<p"))
            {
                conditionalMoveTo("<br", "</p>", 0);
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOP);
            }
        }
    }
        break;

    case ObitAssistant:
    {
        tempUC = globals->globalDr->getTitle();
        tempUC.prepareStructuredNames();
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
            tempUC.prepareStructuredNames();
        }
        break;

    case BowRiver:
        if (consecutiveMovesTo(100, "id=\"content_body\"", "h1"))
        {
            // Read names
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            // Read DOB and DOD
            if (moveTo("h2", 50))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Serenity:
        if (consecutiveMovesTo(100, "title page-title", "itemprop=\"headline\""))
        {
            // Read names
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

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
                tempUC.prepareStructuredNames();

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
                tempUC.prepareStructuredNames();

                // Look for possible date info
                if (moveTo("<h3>", 100))
                {
                    tempUC = getUntil("</h3>");
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
            tempUC.prepareStructuredNames();

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
            tempUC.prepareStructuredNames();

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
    case SRS:
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
            tempUC.prepareStructuredNames();

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
                tempUC.processDateField(dfDOD);
            }

            break;

        case 1:
            // Read Name
            if (consecutiveMovesTo(100, "ObituaryFullName", "\'"))
                tempUC = getUntil("\',");
            tempUC.prepareStructuredNames();

            // Read dates
            if (conditionalMoveTo("birthDate\":", "deathDate", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOB);
            }

            if (moveTo("deathDate\":"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }

            break;

        case 2:
            // Read Name
            if (consecutiveMovesTo(20, "givenName", ":"))
                word = readNextBetween(QUOTES);
            if (consecutiveMovesTo(20, "additionalName", ":"))
            {
                word += space;
                word += readNextBetween(QUOTES);
            }
            moveBackwardTo("givenName");
            if (consecutiveMovesTo(20, "familyName", ":"))
            {
                word += space;
                word += readNextBetween(QUOTES);
            }
            tempUC = word;
            tempUC.prepareStructuredNames();

            // Read dates
            if (conditionalMoveTo("birthDate", "deathDate", 0))
            {
                moveTo(":");
                tempUC = readNextBetween(QUOTES);
                tempUC.processDateField(dfDOB);
            }

            if (moveTo("deathDate"))
            {
                moveTo(":");
                tempUC = readNextBetween(QUOTES);
                tempUC.processDateField(dfDOD);
            }

            break;
        }

        break;

    case Trinity:
        // Read name
        if (moveTo("class=\"obituary-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if(conditionalMoveTo("class=\"birth-date\"", "class=\"obituary-date\"", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOB);

                moveTo("class=\"death-date\"");
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }

            if (moveTo("class=\"obituary-date\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOP);
            }
        }
        break;

    case CelebrateLife:
        // Read name
        if (consecutiveMovesTo(3000, "class=\"main-wrap\"", "Obituary of"))
            tempUC = getUntil("<");
        tempUC.simplify();
        tempUC.removeLeading(SPACE);
        tempUC.prepareStructuredNames();
        break;

    case Funks:
        // Only name available
        if (consecutiveMovesTo(50, "og:title", "content="))
        {
            tempUC = readQuotedMetaContent();
            tempUC.prepareStructuredNames();
        }
        break;

    case WowFactor:
        // Only name available
        if (consecutiveMovesTo(3000, "id=\"main-content\"", "<h3", ">"))
        {
            tempUC = getUntil("</h3>");
            tempUC.prepareStructuredNames();
        }
        break;

    case Dalmeny:
        if (moveTo("class=\"content\""))
        {
            if (consecutiveMovesTo(150, "<p class", "<span", ">"))
            {
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();


                if (consecutiveMovesTo(150, "<p class", "<span", ">"))
                {
                    backward(1);
                    tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);
                }
            }
            else
            {
                beg();
                if (moveTo("class=\"post_title entry-title\""))
                {
                    tempUC = readNextBetween(BRACKETS);
                    if (consecutiveMovesTo(1000, tempUC.getString(), "<p class=", ">"))
                    {
                        conditionalMoveTo("<strong", "</p>", 0);
                        tempUC = readNextBetween(BRACKETS);
                        fillInDatesStructured(tempUC);
                    }
                }
            }
        }
        break;

    case Hansons:
        // Only name available
        if (moveTo("post post-obituary current-item"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Martens:
        // Name, DOB and DOD now available
        if (moveTo("class=\"wpfh_obit_date\""))
        {
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);

            moveBackwardTo("href=");
            moveTo(">");
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Shine:
        // Name, DOB and DOD typically available
        if (moveTo("class=\"vc_custom_heading\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }

        // Dates
        if (moveTo("class=\"vc_custom_heading\"", 1000))
        {
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);
        }
        break;

    case Simply:
        // Name and some dates available - Format changed in 2020

        if (consecutiveMovesTo(200, "<!--/nav .navigation-->", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }

        // Pub Date
        if (moveTo("Obituary Added: "))
        {
            tempUC = getUntil("<");
            tempUC.processDateField(dfDOP);
        }

        switch(style)
        {
        case 0:
            // YOB
            if (conditionalMoveTo(">Born", ">Passed Away"), 2)
            {
                moveTo("<p");
                tempUC = readNextBetween(BRACKETS);
                tempTempUC = tempUC.peekAtWord(false, 2);
                tempTempUC.processYearField(dfDOB);
            }

            // DOD
            if (consecutiveMovesTo(100, ">Passed Away", "<p"))
            {
                tempUC = readNextBetween(BRACKETS);
                int index = tempUC.findPosition(PQString(" "), 1, 0, 3);
                if (index != -1)
                    tempUC.dropRight(tempUC.getLength() - index);
                tempUC.processDateField(dfDOD);
            }
            break;

        case 1:
            // Nothing - All in uc
            break;
        }

        break;

    case McCall:
        if (consecutiveMovesTo(100, "class=\"head-title\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<h", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Care:
        if (moveTo("class=\"wpfh_obit_date\""))
        {
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);

            moveBackwardTo("</a");
            backward(5);
            moveBackwardTo(">");
            tempUC = getUntil("<");
            tempUC.prepareStructuredNames();
        }
        break;

    case Ancient:
        if (consecutiveMovesTo(100, "class=\"blog-title-link blog-link\"", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Amherst:
        if (consecutiveMovesTo(350, "id=\"mainContent\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(15, "</h", "<p",0))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Heritage:
        if (consecutiveMovesTo(75, "class=\"eltdf-page-title entry-title\"", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(50, "class=\"eltdf-page-subtitle\"", ">"))
            {
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Koru:
        if (moveTo("class=\"page-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            tempUC = globals->globalDr->getURL();
            tempUC.removeEnding("/");
            tempTempUC = tempUC.right(9);
            if (tempTempUC.getString().at(4) == QString("-"))
                tempTempUC.processStructuredYears();
        }
        break;

    case Kowalchuk:
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Loehmer:
        if (consecutiveMovesTo(100, "class=\"content\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(50, "class=\"dod\"", "Passed on ", 0))
            {
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOD);
            }
        }
        break;

    case Doyle:
        if (consecutiveMovesTo(50, "class=\"page-header\"", "<h", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Ethical:
        replaceHTMLentities();
        if (consecutiveMovesTo(75, "<span style=\"font-family:playfair display,serif", ">"))
        {
            tempUC = getUntil("</p>");
            tempUC.removeHTMLtags();
            //tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo(" - "), 4000)
            {
                moveBackwardTo("\"");
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        else
        {
            beg();
            if (moveTo("style=\"text-shadow:#ffffff -1px -1px 0px, #ffffff -1px 1px 0px, #ffffff 1px 1px 0px, #ffffff 1px -1px 0px\">"))
            {
                //tempUC = readNextBetween(BRACKETS);
                tempUC = getUntil("</p>");
                tempUC.removeHTMLtags();
                tempUC.prepareStructuredNames();

                if (moveTo(" - "), 4000)
                {
                    moveBackwardTo("\"");
                    tempUC = readNextBetween(BRACKETS);
                    fillInDatesStructured(tempUC);
                }
            }
            else
            {
                beg();

            }
        }
        break;

    case Direct:
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"et_pb_text_inner\""))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case SMC:
        if (moveTo("class=\"name-of-person\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"life-span\""))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Belvedere:
        {
            tempUC = globals->globalDr->getTitle();
            tempUC.prepareStructuredNames();
        }
        break;

    case Davidson:
        if (consecutiveMovesTo(100, "class=\"details\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(50, "Passing Date:", ", "))
            {
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOD);

                if (consecutiveMovesTo(50, "Funeral Date:", ", "))
                {
                    tempUC = getUntil("<");
                    tempUC.processDateField(dfDOS);
                }
            }
        }
        break;

    case Carnell:
        if (consecutiveMovesTo(1000, "class=\"container obituary-container\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<h", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case JOsmond:
        if (consecutiveMovesTo(100, "class=\"page-banner-text\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("mall", 10))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case TivaHost:
        if (moveTo("class=\"nxs-title nxs-align-left"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"nxs-blog-meta nxs-applylinkvarcolor\"", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }
        }
        break;

    case KMF:
        if (consecutiveMovesTo(250, "class=\"entry-title\"", "href=", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case AMG:
        if (consecutiveMovesTo(50, "class=\"title\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("ContentPlaceHolder1_lblDateOfDeath");
            moveTo("Died: ", 5);
            tempUC = getUntil("<");
            tempUC.processDateField(dfDOD);
        }
        break;

    case Orillia:
        if (consecutiveMovesTo(200, "id=\"wpfh_main_obit\"", "title=", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("class=\"wpfh_obit_date\"");
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);
        }
        break;

    case OSM:
        if (consecutiveMovesTo(100, "class=\"news_full_story_posted_text\"", "</span>"))
        {
            // Pub date
            tempUC = getUntil("<");
            tempUC.processDateField(dfDOP);

            // Name
            moveTo("class=\"news_main_story_title\"");
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Alcock:
        if (moveTo("</span></span>"))
        {
            moveBackwardTo("<span", 200);
            moveTo(">");
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            forward(5);
            if (moveTo("</span></span>"))
            {
                moveBackwardTo("<span", 200);
                moveTo(">");
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Abstract:
        if (consecutiveMovesTo(30, "class=\"right\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Beechwood:
        if (consecutiveMovesTo(200, "class=\"service-name-date\"", "<span"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("block-birthdate", "block-deathdate", 1))
            {
                if (conditionalMoveTo("datetime=\"", "block-deathdate", 0))
                {
                    tempUC = getUntil("T");
                    tempUC.processDateField(dfDOB, doYMD);
                }
                moveTo("</section>");
                forward(1);
            }

            if (conditionalMoveTo("block-deathdate", "</section>", 1))
            {
                if (conditionalMoveTo("datetime=\"", "</section>", 0))
                {
                    tempUC = getUntil("T");
                    tempUC.processDateField(dfDOD, doYMD);
                }
            }

            if (consecutiveMovesTo(750, "class=\"service-details\"", "WHEN", "datetime=\""))
            {
                tempUC = getUntil("T");
                tempUC.processDateField(dfDOS, doYMD);
            }
        }

        break;

    case Benjamins:
        if (moveTo("id=\"ContentPlaceHolder1_lblName\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            consecutiveMovesTo(250, ">Death Date", "ContentPlaceHolder1_lblDeathDate", ", ");
            tempUC = getUntil("<");
            tempUC.processDateField(dfDOD);

            consecutiveMovesTo(250, ">Funeral Date", "ContentPlaceHolder1_lblFuneralDate", ", ");
            tempUC = getUntil("<");
            tempUC.processDateField(dfDOS);
        }
        break;

    case Berthiaume:
        if (moveTo("<div class=\"topflag\">"))
        {
            tempUC = getUntil("<span>");
            tempUC.prepareStructuredNames();

            if (moveTo(" ", 20))
            {
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOD);
            }
        }
        break;

    case Blenheim:
        if (consecutiveMovesTo(100, "class=\"entry-title single-title\"", ">"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Brenneman:
        if (moveTo("class=\"blog-post-title-font blog-post-title-color\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(500, "type=\"first\"", "class=\"_2PHJq public-DraftStyleDefault-ltr\"", "<span"))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Cardinal:
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"entry-birth-death-date\""))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Carson:
        if (consecutiveMovesTo(100, "class=\"hero-unit\"", "<h", ">"))
        {
            conditionalMoveTo("Remembering ", "<");
            tempUC = getUntil("<");
            tempUC.prepareStructuredNames();
        }
        break;

    case Turner:
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"published\""))
            {
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOP);
            }
        }
        break;

    case Eagleson:
        if (moveTo("class=\"entry-title clearfix banner-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case FirstMemorial:
        if (moveTo("class=\"year\""))
        {
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);

            moveTo("<li><h");
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Haine:
        tempUC = OQString(globals->globalDr->getID()).convertFromID();
        tempUC.prepareStructuredNames();
        break;

    case RHB:
        if (consecutiveMovesTo(100, "id=\"obit-details\"", "<h", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Rhody:
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Simpler:
        if (moveTo("columns birth-death-date"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.simplify();
            fillInDatesStructured(tempUC);

            moveTo("<p");
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Steadman:
        if (consecutiveMovesTo(30, "class=\"inner-page-ttl\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            consecutiveMovesTo(30, "class=\"single_ttl_btm\"", "<h");
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);
        }
        break;

    case Steeles:
        if (consecutiveMovesTo(600, ">Contact Us<", "class=\"item-title section-title\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(100, ">Date of Death:<", ", "))
            {
                tempUC = getUntil("<");
                qdate = tempUC.readDateField();
                if (qdate.isValid())
                    globals->globalDr->setDOD(qdate);
            }
        }
        break;

    case Bridge:
        if (moveTo("itemprop=\"headline\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("datetime=\"", 1000))
            {
                tempUC = getUntil("T");
                qdate = tempUC.readDateField(doYMD);
                if (qdate.isValid())
                    globals->globalDr->setPublishDate(qdate);
            }
        }
        break;

    case McCormack:
        if (consecutiveMovesTo(125, "name=\"twitter:description\"", "remembrance book for the late "))
        {
            tempUC = getUntil("\"/>");
            tempUC.prepareStructuredNames();
        }

        if (consecutiveMovesTo(500, ">Born<", "<span", "</span>"))
        {
            backward(10);
            moveBackwardTo(">");
            tempUC = getUntil("<");
            qdate = tempUC.readDateField();
            if (qdate.isValid())
                globals->globalDr->setDOB(qdate);
        }
        else
            beg();

        if (consecutiveMovesTo(500, ">Died<", "<span", "</span>"))
        {
            backward(10);
            moveBackwardTo(">");
            tempUC = getUntil("<");
            qdate = tempUC.readDateField();
            if (qdate.isValid())
                globals->globalDr->setDOD(qdate);
        }

        /*if (consecutiveMovesTo(25, ">Obituary Announcement<", ">"))
        {
            tempUC = getUntil("</p>");
            tempUC.prepareStructuredNames();
        }

        end();
        if (moveBackwardTo("</h4>", 1000000))
        {
            moveBackwardTo("<span");
            moveTo(">");
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            qdate = tempUC.readDateField();
            if (qdate.isValid())
            {
                globals->globalDr->setDOD(qdate);
                backward(10);
                if (moveBackwardTo("</h4>"))
                {
                    moveBackwardTo("<span");
                    moveTo(">");
                    backward(1);
                    tempUC = readNextBetween(BRACKETS);
                    qdate = tempUC.readDateField();
                    if (qdate.isValid())
                        globals->globalDr->setDOB(qdate);
                    else
                    {
                        backward(10);
                        if (moveBackwardTo("</h4>"))
                        {
                            moveBackwardTo("<span");
                            moveTo(">");
                            backward(1);
                            tempUC = readNextBetween(BRACKETS);
                            qdate = tempUC.readDateField();
                            if (qdate.isValid())
                                globals->globalDr->setDOB(qdate);
                        }
                    }
                }
            }
        }*/
        break;

    case Brunet:
        if (moveTo("class=\"page__title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }

        beg();
        if (style == 0)
        {
            if (conditionalMoveTo("Date of birth", "Date of death", 0))
            {
                moveTo(":");
                if (tempUC.left(9) == PQString("</strong>"))
                    forward(9);
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOB);
            }

            if (consecutiveMovesTo(50, "Date of death", ":"))
            {
                if (tempUC.left(9) == PQString("</strong>"))
                    forward(9);
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOD);
            }

            beg();
            if (conditionalMoveTo("de naissance", "de décès", 0))
            {
                moveTo(":");
                if (tempUC.left(9) == PQString("</strong>"))
                    forward(9);
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOB);
            }

            if (consecutiveMovesTo(50, "de décès", ":"))
            {
                if (tempUC.left(9) == PQString("</strong>"))
                    forward(9);
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOD);
            }            
        }
        else
        {
            if (moveTo("class=\"page__title\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();

                if (conditionalMoveTo("Birth date", "</span>", 0))
                {
                    moveTo("</span");
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processDateField(dfDOB);
                }

                if (conditionalMoveTo("Date of death", "</span>", 0))
                {
                    moveTo("</span");
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processDateField(dfDOD);
                }
            }
        }

        break;

    case TurnerFamily:
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case VanHeck:
        if (consecutiveMovesTo(100, "class=\"entry-title\"", "itemprop=\"headline\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case TBK:
        if (consecutiveMovesTo(750, "id=\"main\" class=\"main\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<h", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Whelan:
        target = OQString(globals->globalDr->getID().left(15)).convertFromID();
        if (moveTo(target))
        {
            moveBackwardTo("style=\"margin-bottom: 1em;\"");
            moveTo("<b");
            tempUC = readNextBetween(BRACKETS);
            tempUC.fixBasicErrors();
            tempUC.prepareStructuredNames();

            moveTo("<b>", 100);
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.fixBasicErrors();
            tempUC.removeBookEnds();
            fillInDatesStructured(tempUC);
        }
        break;

    case Aeterna:
        if (moveTo("class=\"title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"subtitle\"", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case Actuel:
        if (moveTo("class=\"single-nom\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"single-date\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case Dupuis:
        if (consecutiveMovesTo(50, "class=\"avis-header\"", "<spa"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempTempUC = tempUC.right(11);
            tempTempUC.removeBookEnds(PARENTHESES);
            if (tempTempUC[4] == QChar('-'))
            {
                tempUC.dropRight(12);
                tempTempUC.processStructuredYears();
            }
            tempUC.prepareStructuredNames();
        }
        break;

    case HGDivision:
        if (moveTo("class=\"obituaries-sub-heading\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"obituaries-heading-content\""))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);

                if (moveTo("<span> - ", 50))
                {
                    tempUC = getWord();
                    tempUC.processAgeAtDeath();
                }
            }
        }
        break;

    case Jacques:
        if (consecutiveMovesTo(100, "class=\"item-title\"", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(10, "<p>", "le "))
            {
                tempUC = getUntil(",");
                tempUC.processDateField(dfDOD);

                if (moveTo(QString("à l’âge de "), 15))
                {
                    tempUC = getUntil(" ");
                    tempUC.processAgeAtDeath();
                }
            }
        }
        break;

    case Joliette:
        if (moveTo("class=\"entry-title fusion-post-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(100, "class=\"post-content\"", "<br"))
            {
                moveBackwardTo("<p>");
                tempUC = getUntil("<br />");
                tempUC.removeHTMLtags();
                if (tempUC.getString()[4] == QString("-"))
                    tempUC.processStructuredYears();
                else
                {
                    tempUC = getUntilEarliestOf("<br />", "</p>");
                    tempUC.cleanUpEnds();
                    tempUC.removeHTMLtags();
                    if (tempUC.getString()[4] == QString("-"))
                        tempUC.processStructuredYears();
                }
            }
        }
        break;

    case Rajotte:
        target = QString("php/form_share.php?id=") + globals->globalDr->getID().getString();
        if (moveTo(target))
        {
            moveTo("Avis de décès - ");
            tempUC = getUntil(" - ");
            tempUC.prepareStructuredNames();

            tempUC = getUntil("\"");
            tempUC.processStructuredYears();
        }
        break;

    case BM:
        if (consecutiveMovesTo(100, "class=\"necrologie_fiche_right\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<pre", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case Jodoin:
        if (moveTo("class=\"dates-range\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.processStructuredYears();

            moveTo("class=\"detailsdeces-fiche\"");
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Fournier:
        if (consecutiveMovesTo(100, "type=\"application/ld+json\"", "\"name\":"))
        {
            tempUC = readNextBetween(QUOTES);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("\"birthDate\":", "deathDate", 0))
            {
                tempUC = readNextBetween(QUOTES);
                tempUC.processDateField(dfDOB, doYMD);
            }

            if (conditionalMoveTo("\"deathDate\":", "gender", 0))
            {
                tempUC = readNextBetween(QUOTES);
                tempUC.processDateField(dfDOD, doYMD);
            }

            if (moveTo("\"gender\":"))
            {
                tempUC = readNextBetween(QUOTES);
                if (tempUC == PQString("Male"))
                    globals->globalDr->setGender(Male);
                else
                {
                    if (tempUC == PQString("Female"))
                        globals->globalDr->setGender(Female);
                }
            }
        }
        break;

    case Desnoyers:
        if (moveTo("class=\"entry-title fusion-post-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            QString tempString = tempUC.getString();
            int index1 = tempString.indexOf(",");
            int index2 = tempString.lastIndexOf(",");
            if ((index1 >= 0) && (index2 >= 0) && (index1 != index2))
            {
                if (tempString.mid(index1 + 2, 3) == QString("NÉE"))
                    tempUC = tempString.left(index1) + QString(" (") + tempString.mid(index1 + 2, index2 - index1 - 2) +  QString("),") + tempString.right(tempString.length() - index2 - 1);
            }
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(100, "class=\"entry-date\"", "</i"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }
        }
        break;

    case Desrosiers:
        if (moveTo("class=\"entry-title-main\""))
        {
            tempUC = readNextBetween(BRACKETS).getString();
            tempUC.cleanUpEnds();
            QString tempString = tempUC.getString();
            int index = tempString.indexOf(",");
            if (index == -1)
            {
                index = tempString.lastIndexOf(" ");
                tempUC = tempString.left(index) + QString(",") + tempString.right(tempString.length() - index);
            }
            else
                tempUC = tempString;
            tempUC.prepareStructuredNames();
        }
        break;

    case MontPetit:
        if (consecutiveMovesTo(250, "<div class=\"ds-1col node node-avis-de-deces view-mode-full clearfix", "property=\"dc:title\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(200, "<div class=\"field field-name-dates-nessance-et-deces", "class=\"field-item even\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case Parent:
        if (consecutiveMovesTo(100, "<h1", "class=\"def_title\"", ">"))
        {
            tempUC = getUntil("</h1>");
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"date_obs\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.replace("/", "-");
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case RichardPhilibert:
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Kane:
        if (moveTo(">In Memory Of<"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Gaudet:
        if (consecutiveMovesTo(5000, "<div id=\"texte\">", "href=\"avis-de-deces", "<h1", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(100, "<h2", ">"))
            {
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case NouvelleVie:
        if (consecutiveMovesTo(300, ">Imprimer<", "<h", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("class=\"dates\"");
            tempUC = readNextBetween(BRACKETS);
            tempUC.processStructuredYears();
        }
        break;

    case Santerre:
        if (moveTo("class=\"post_title entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            beg();
            if (consecutiveMovesTo(50, "<title>", "s : "))
            {
                tempTempUC = getUntil("<");
                int index = tempTempUC.getString().lastIndexOf(", ");
                tempUC = tempTempUC.right(tempTempUC.getLength() - index - 2);
                if (tempUC.getLength() == 23)
                {
                    tempTempUC = tempUC.left(10);
                    tempTempUC.processDateField(dfDOB, doYMD);
                }
                tempTempUC = tempUC.right(10);
                tempTempUC.processDateField(dfDOD, doYMD);
            }
        }
        break;

    case Shields:
        if (consecutiveMovesTo(200, "class=\"topflag\"", "<img", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            consecutiveMovesTo(50, "<span>", " ");
            tempUC = getUntil("<");
            tempUC.processDateField(dfDOD);
        }
        break;

    case Gamache:
        if (consecutiveMovesTo(100, "class=\"column mcb-column three-fourth column_column", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<span>, ", 20))
            {
                tempUC = getUntil("<");
                tempUC.processStructuredYears();
            }
        }
        break;

    case Landry:
        if (consecutiveMovesTo(300, "<article class=", "<header>", "<small"))
        {
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);

            moveTo("/small");
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case StLouis:
        if (consecutiveMovesTo(100, "class=m-obituary-single-intro>", "class=text-uppercase>"))
        {
            tempUC = getUntil("</h");
            tempUC.prepareStructuredNames();

            consecutiveMovesTo(25, "class=birth-death", "<p");
            tempUC = readNextBetween(BRACKETS);
            tempUC.processStructuredYears();

            consecutiveMovesTo(50, "Décès", "datetime=");
            tempUC = getUntil(">");
            tempUC.processDateField(dfDOD);
        }
        break;

    case McGerrigle:
        if (consecutiveMovesTo(100, "class=\"entry-title\"", "headline"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempTempUC = tempUC.right(9);
            if (tempTempUC.getString()[4] == QString("-"))
            {
                tempUC.dropRight(11);
                tempTempUC.processStructuredYears();
            }
            else
            {
                int index = tempUC.getString().indexOf(",");
                if (index > 0)
                {
                    tempTempUC = tempUC.middle(index + 2, tempUC.getLength() - index - 2);
                    if (tempTempUC.isHyphenated())
                    {
                        tempUC.dropRight(tempUC.getLength() - index);
                        fillInDatesStructured(tempTempUC);
                    }
                }
            }
            tempUC.prepareStructuredNames();
        }
        break;

    case Paperman:
        if (consecutiveMovesTo(100, "<div id='content'>", "<header>", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(100, ">Funeral Service:<", "<td>", ", "))
            {
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOS);
            }
        }
        break;

    case Poissant:
        if (consecutiveMovesTo(100, "deces-liste-nom", "href", ">"))
        {
            tempUC = getUntil("<");
            tempUC.prepareStructuredNames();

            if (moveTo("<h3", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case Legare:
        if (consecutiveMovesTo(100, "id=\"txtDefunt\"", "<span", ">"))
        {
            tempUC = getUntil("<");
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(25, "class=\"bodyDefunt\"", "<p"))
            {
                tempUC = getUntil("<");
                tempUC.processStructuredYears();
            }
        }
        break;

    case Longpre:
        if (moveTo("class=\"et_pb_module_header\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"et_pb_member_position\"", 25))
            {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processStructuredYears();
            }

            if (consecutiveMovesTo(100, "<p>", "</p>", "<strong"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }
        }
        break;

    case Lanaudiere:
        if (consecutiveMovesTo(750, "class=\"post_content\"", "<h", ">"))
        {
            tempUC = getUntil("</h");
            tempUC.removeHTMLtags();
            tempUC.cleanUpEnds();
            tempTempUC = tempUC.right(11);
            if (tempTempUC.removeBookEnds(PARENTHESES))
            {
                tempUC.dropRight(12);
                tempTempUC.processStructuredYears();
            }
            tempUC.prepareStructuredNames();
        }
        else
        {
            beg();
            if (consecutiveMovesTo(750, "class=\"post_content\"", "<p", ">"))
            {
                conditionalMoveTo("<em>", "</", 0);
                backward(1);
                tempUC = readNextBetween(BRACKETS);
                tempTempUC = tempUC.right(11);
                if (tempTempUC.removeBookEnds(PARENTHESES))
                {
                    tempUC.dropRight(12);
                    tempTempUC.processStructuredYears();
                }
                tempUC.prepareStructuredNames();
            }
        }
        break;

    case Theriault:
        if (consecutiveMovesTo(100, "", ""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Voluntas:
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"passing\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }

            if (moveTo("class=\"entry-content\">"))
            {
                tempUC = getUntil("</p>");
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Wilbrod:
        if (consecutiveMovesTo(100, "class=\"obituaries-announcement-detail\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<h", 25))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case Hodges:
        if (consecutiveMovesTo(200, "class=\"entry-title\"", "href=", ">"))
        {
            backward(1);
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Bergeron:
        if (consecutiveMovesTo(1000, "class=\"fiche\"", "<h", "<span"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("</h"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case Passage:
        if (moveTo("class=\"belwe\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<h", 20))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD, doMDY);
            }
        }
        break;

    case Granit:
        if (moveTo("class=\"titreAvis\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"annee_nais_deces_avis\"", 200))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case Affordable:
        if (consecutiveMovesTo(75, "class=\"entry-title\"", "itemprop=\"name\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case LFC:
        if (consecutiveMovesTo(100, "class=\"about-inner-content single_annoucement_content\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("<p", "</div>", 0))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();

                if (conditionalMoveTo("Date de Décès : ", "</div>", 0))
                {
                    tempUC = getUntil("<");
                    tempUC.setContentLanguage(french);
                    tempUC.processDateField(dfDOD);
                }
            }
        }
        break;

    case LifeTransitions:
        if (consecutiveMovesTo(100, "<!-- obit name -->", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<!-- obit date -->"))
            {
                if (conditionalMoveTo("dateofBirth", "</div>"))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processDateField(dfDOB);
                }

                if (conditionalMoveTo("dateofDeath", "</div>"))
                {
                    tempUC = readNextBetween(BRACKETS);
                    tempUC.processDateField(dfDOD);
                }
            }
        }
        break;

    case Davis:
        if (consecutiveMovesTo(100, "id=\"headerwrap\"", "<h1"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case MacLark:
        if (moveTo("class=\"h3 text-blue\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"text-muted\"", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Fallis:
        if (consecutiveMovesTo(75, "class=\"pure-g container\"", "class=\"pure-u-1\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Timiskaming:
        if (moveTo("class=\"obituary-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"obituary-years\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case Garrett:
        if (moveTo("class=\"nxs-title nxs-align-left"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Smith:
        if (consecutiveMovesTo(100, ">Back to List<", "<strong"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<br /", 15))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }
        }
        break;

    case Picard:
        tempUC = globals->globalDr->getTitle();
        tempTempUC = tempUC.right(11);
        if (tempTempUC.removeBookEnds(PARENTHESES))
        {
            tempUC.dropRight(12);
            tempTempUC.processStructuredYears();
        }

        tempUC.prepareStructuredNames();

        break;

    case Richelieu:
        if (consecutiveMovesTo(75, "class=\"info font-ASAP\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("/h", 25))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case Roy:
        if (moveTo("class=\"post-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case CharleVoix:
        if (consecutiveMovesTo(250, "id=\"middle\"", "<br>"))
        {
            moveBackwardTo("\">");
            tempUC = getUntil("<") + PQString(", ");
            moveTo("br", 5);
            tempUC += readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("<br");
            tempUC = readNextBetween(BRACKETS);
            tempUC.processStructuredYears();
        }
        break;

    case Aurora:
        if (consecutiveMovesTo(175, "card-title", "display-", "aurora_latename", ">"))
        {
            tempUC = getUntil("</div>") + PQString(", ");
            moveTo(">");
            tempUC += getUntil("<");
            tempUC.prepareStructuredNames();

            if (moveTo("p-city-date", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                int index = tempUC.findPosition("(");
                if (index >= 0)
                {
                    tempTempUC = tempUC.right(tempUC.getLength() - index);
                    tempUC.dropRight(tempUC.getLength() - index);
                    tempTempUC.removeBookEnds(PARENTHESES);
                    index = tempTempUC.findPosition(" ");
                    tempTempUC.dropRight(tempTempUC.getLength() - index);
                    tempTempUC.processAgeAtDeath();
                }
                tempUC.cleanUpEnds();
                if (tempUC.getLength() <= 10)
                    tempUC.processStructuredYears();
                else
                    fillInDatesStructured(tempUC);
            }
        }
        break;

    case Montcalm:
        if (moveTo("class=\"post_title entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempTempUC = tempUC.right(11);
            if (tempTempUC.removeBookEnds(PARENTHESES))
            {
                tempUC.dropRight(11);
                tempTempUC.processStructuredYears();
            }
            tempUC.prepareStructuredNames();
        }
        break;

    case Trahan:
        if (moveTo(""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Laurent:
        if (consecutiveMovesTo(100, "class=\"details\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("Décédé(e) le ", "<hr/>"))
            {
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOD);
            }
        }
        break;

    case Eternel:
        if (consecutiveMovesTo(750, "id=\"avisdeces-fiche\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<span", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                int index = tempUC.findPosition("(");
                if (index >= 0)
                {
                    tempTempUC = tempUC.right(tempUC.getLength() - index);
                    tempUC.dropRight(tempUC.getLength() - index);
                    tempTempUC.removeBookEnds(PARENTHESES);
                    index = tempTempUC.findPosition(" ");
                    tempTempUC.dropRight(tempTempUC.getLength() - index);
                    tempTempUC.processAgeAtDeath();
                }
                tempUC.cleanUpEnds();
                if (tempUC.getLength() <= 10)
                    tempUC.processStructuredYears();
                else
                    fillInDatesStructured(tempUC);
            }
        }
        break;

    case Ruel:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(200, "class=\"wsite-menu-subitem-wrap wsite-nav-current\"", "class=\"wsite-menu-title\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
            break;

        case 1:
            target = OQString(globals->globalDr->getID()).convertFromID();
            if (moveTo(target))
            {
                tempUC = target;
                tempUC.prepareStructuredNames();
            }
            break;
        }
        break;

    case Hamel:
        if (consecutiveMovesTo(250, "wpfh-single-header-right", "href", ">"))
        {
            tempUC = getUntil("<");
            tempUC.prepareStructuredNames();

            if (moveTo("wpfh-obit-alternate-dates"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case CremAlt:
    case Forest:
    case TriCity:
        if (consecutiveMovesTo(175, "class=\"blog-title-link blog-link\"", ">"))
        {
            tempUC = getUntil("<");
            tempUC.prepareStructuredNames();

            if (moveTo("class=\"date-text\"", 150))
            {                
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD, doMDY);
            }
        }
        break;

    case London:
        if (consecutiveMovesTo(50, "id=\"text_area\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case Dryden:
        if (consecutiveMovesTo(100, "class=\"page-banner-text\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<small"))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Lampman:
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case ecoPassages:
        if (consecutiveMovesTo(125, "class=\"entry-title", ">"))
        {
            tempUC = getUntil("<");
            tempUC.prepareStructuredNames();
        }
        break;

    case Peaceful:
        if (consecutiveMovesTo(50, "class=\"obituary-name\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("<h", 50))
            {
                tempUC = readNextBetween(BRACKETS);
                fillInDatesStructured(tempUC);
            }
        }
        break;

    case Ranger:
        if (moveTo("class=\"title-header\""))
        {
            conditionalMoveTo("<strong", "</h");
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case People:
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(50, "class=\"entry-content\"", "Date of Death: "))
            {
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOD);
            }
        }
        break;

    case Whitcroft:
        if (consecutiveMovesTo(300, "id=\"text1\"", "<I>"))
        {
            tempUC = getUntil("<");
            tempUC.prepareStructuredNames();

            if (consecutiveMovesTo(300, "id=\"text2\"", "<I>"))
            {
                tempUC = getUntil("<");
                tempUC.processStructuredYears();
            }
        }
        else
        {
            beg();
            if (consecutiveMovesTo(300, "id=\"text1\"", "<i>"))
            {
                tempUC = getUntil("<");
                tempUC.prepareStructuredNames();

                if (consecutiveMovesTo(300, "id=\"text2\"", "<I>"))
                {
                    tempUC = getUntil("<");
                    tempUC.processStructuredYears();
                }
            }
        }
        break;

    case LegacyCardstrom:
        if (consecutiveMovesTo(50, "og:title", "content="))
        {
            tempUC = readQuotedMetaContent();
            tempUC.prepareStructuredNames();
        }
        break;

    case Wiebe:
    {
        if (moveTo("<h2 itemprop=\"headline\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        /*rxTarget.setPattern("\\d{4} (-|~) \\d{4}");
        int index = itsString.indexOf(rxTarget);
        if (index != -1)
        {
            forward(index);
            moveBackwardTo("style=\"text-align: center;");
            backward(30);
            moveBackwardTo("style=\"text-align: center;");
            conditionalMoveTo("<span", "</", 0);
            moveTo(">");
            if (conditionalMoveTo("<img", "</", 0))
                moveTo(">");
            if (conditionalMoveTo("<img", "</", 0))
                moveTo(">");
            if (conditionalMoveTo("<frame", "</", 0))
                moveTo(">");
            tempUC = getUntil("<");
            tempUC.prepareStructuredNames();

            if (moveTo("style=\"text-align: center;", 100))
            {
                conditionalMoveTo("<span", "</");
                moveTo(">");
                tempUC = getUntil("<");
                tempUC.processStructuredYears();
            }
        }
        else
        {
            if (moveTo("style=\"text-align: center;"))
            {
                if (conditionalMoveTo("<img", "</span>"))
                    moveTo("style=\"text-align: center;");
                if (conditionalMoveTo("<iframe", "</span>"))
                    moveTo("style=\"text-align: center;");
                conditionalMoveTo("<span", "</", 0);
                moveTo(">");
                tempUC = getUntil("<");
                tempUC.prepareStructuredNames();

                if (moveTo("style=\"text-align: center;", 100))
                {
                    conditionalMoveTo("<span", "</");
                    moveTo(">");
                    tempUC = getUntil("<");
                    tempUC.processStructuredYears();
                }
            }
        }*/
    }
        break;

    case Arimathea:
        if (moveTo("class=\"entry-dateline-link\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.processDateField(dfDOD);

            consecutiveMovesTo(200, "href=", "rel=");
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case GFournier:
        if (moveTo("type=\"application/ld+json\""))
        {
            moveTo("\"name\": \"");
            tempUC = getUntil("\",");
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("\"birthDate\": \"", "</script>", 0))
            {
                tempUC = getUntil("\",");
                tempUC.processDateField(dfDOB);
            }

            if (conditionalMoveTo("\"deathDate\": \"", "</script>", 0))
            {
                tempUC = getUntil("\",");
                tempUC.processDateField(dfDOD);
            }

            if (conditionalMoveTo("\"gender\": \"", "</script>", 0))
            {
                tempUC = getUntil("\",");
                tempUC.processGender();
            }
        }
        break;

    case Harmonia:
        if (consecutiveMovesTo(35, "<div class=\"right\">", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            if (moveTo("span", 10))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }
        }
        break;

    case Omega:
        target = OQString(globals->globalDr->getID()).convertFromID();
        if (moveTo(target))
        {
            tempUC = target;
            tempUC.prepareStructuredNames();

            consecutiveMovesTo(100, "<p", ">");
            tempUC = getUntil("<");
            tempUC.processStructuredYears();
        }
        break;

    case HeritageWP:
        if (consecutiveMovesTo(50, "class=\"entry-title\"", "In Memory of "))
        {
            tempUC = getUntil("<");
            tempUC.prepareStructuredNames();

            if (conditionalMoveTo("class=\"h3\"", "</div>"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processStructuredYears();
            }

            if (conditionalMoveTo("class=\"h4\"", "</div>"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD, doDMY);
            }
        }
        break;

    case Ouellet:
        if (consecutiveMovesTo(30, "font-size:26px;", ">"))
        {
            tempUC = getUntil("&nbsp;");
            tempUC.prepareStructuredNames();

            conditionalMoveTo("&nbsp;", "<", 0);
            tempUC = getUntil("<");
            tempUC.processStructuredYears();
        }
        break;

    case HommageNB:
        if (moveTo("class=\"page-title\""))
        {
            int indexA, indexB;

            tempUC = readNextBetween(BRACKETS);
            indexA = tempUC.findPosition("(19");
            if (indexA == -1)
                indexA = tempUC.findPosition("(20");
            if (indexA >= 0)
            {
                indexB = tempUC.findPosition(")", indexA);
                tempTempUC = tempUC.middle(indexA, indexB - indexA);
                tempTempUC.removeBookEnds(PARENTHESES);
                tempTempUC.processStructuredYears();
                tempUC = tempUC.left(indexA);
            }
            else
            {
                // Drop ( - 2022)
                indexA = tempUC.findPosition("( -");
                tempUC = tempUC.left(indexA);
            }

            tempUC.prepareStructuredNames();

            if (moveTo("class=\"obituary-details\""))
            {
                if (conditionalMoveTo(">Date de d", ">Salon fun",0))
                {
                    moveTo(" : ");
                    tempUC = getUntil("<");
                    tempUC.processDateField(dfDOD);
                }
            }
        }
        break;

    case Drake:
        if (consecutiveMovesTo(100, "article:published_time", "content=\""))
        {
            tempUC = getUntil("T");
            tempUC.processDateField(dfDOP);
        }

        beg();
        if (moveTo("class=\"entry-title\""))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();
        }
        break;

    case CityLine:
        if (consecutiveMovesTo(350, "class='tributecontent-tributepost", "class='posts_head'", "<h1"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("class='tributepostdata'");
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);
        }
        break;

    case Komitas:
        if (consecutiveMovesTo(100, "class=\"obituary-ttl-info\"", "<h"))
        {
            tempUC = readNextBetween(BRACKETS);
            tempUC.prepareStructuredNames();

            moveTo("<span", 100);
            tempUC = readNextBetween(BRACKETS);
            fillInDatesStructured(tempUC);
        }
        break;

    case Driftwood:
        switch(style)
        {
        case 0:
            if (consecutiveMovesTo(100, "class=\"post-title\"", "headline"))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
            break;

        case 1:
            if (moveTo("class=\"tdb-title-text\""))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
            break;
        }
        break;

    case MLBW:
        if (consecutiveMovesTo(250, "fb37a48", "<h2"))
        {
            tempUC = readNextBetween(BRACKETS);
            if (consecutiveMovesTo(250, "ea90534", "<h2"))
            {
                tempUC += readNextBetween(BRACKETS);
                tempUC.prepareStructuredNames();
            }
            else
                beg();

            if (conditionalMoveTo(">Né(e) le ", ">décédé(e) le ", 0))
            {
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOB);
            }

            if (moveTo(">décédé(e) le "))
            {
                tempUC = getUntil("<");
                tempUC.processDateField(dfDOD);
            }
        }
        break;

    case Sproing:
        if (consecutiveMovesTo(300, "et-main-area", "h2-heading"))
        {
            conditionalMoveTo("strong", "</", 0);
            moveTo(">");
            tempUC = getUntil("<");
            tempUC.prepareStructuredNames();

            if (moveTo("dod fs-5", 100))
            {
                tempUC = readNextBetween(BRACKETS);
                tempUC.processDateField(dfDOD);
            }
        }
        break;

    case WebCemeteries:
    {
        tempUC = globals->globalDr->getRawFullName();
        tempUC.prepareStructuredNames();
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
        {
            tempUC = getUntil(" Obituary");
            tempUC.removeEnding("|");
        }
        break;

    case Passages:
        // Initial name search from title and header sections to bookend unstructured searches
        // Format is always LASTNAME FIRSTNAME -

        if (moveTo("<title>"))
        {
            PQString firstName, lastName;
            OQString name;
            unsigned int numNamesRemaining;

            tempUC = getUntil(" - ", 75, true);
            if (tempUC.getLength() == 0)
            {
                // Backup read
                // TODO
            }
            tempUC.cleanUpEnds();
            tempUC.beg();
            numNamesRemaining = tempUC.countWords();
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
    case Legacy:
    {
        if (getSource().getLength() == 0)
            style = 3;
        else
        {
            if (consecutiveMovesTo(25, "publishedDate", ": "))
            {
                style = 2;
            }
            else
            {
                beg();
                if (moveTo("\"articleSection\": \"Obituaries\"", 10000))
                    style = 1;
                else
                    style = 0;
            }
        }
    }
        break;

    case BlackPress:
        if (moveTo("class=\"title"))
            style = 1;
        else
            style = 0;
        break;

    case Batesville:
    {
        beg();
        if (moveTo("id=\"obitbar\""))
            style = 6;
        else
        {
            beg();
            if (moveTo("class=\"obithead"))
                style = 4;
            else
            {
                beg();
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
                        else
                        {
                            beg();
                            if (moveTo("class=\"obit-name-text\""))
                                style = 5;
                        }
                    }
                }
            }
        }
    }
        break;

    case FuneralTech:
        if (moveTo("api.secure.tributecenteronline.com"))
            style = 3;
        else
        {
            beg();
            if (moveTo("ogden.funeraltechweb.com"))
                style = 5;
            else
            {
                beg();
                if (consecutiveMovesTo(100, "<title", "Obituary / Nécrologie"))
                {
                    style = 4;
                }
                else
                {
                    beg();
                    if (moveTo("class=\"deceased-info\""))
                        style = 1;
                    else
                    {
                        beg();
                        if (moveTo("class=\"flipbook-container\""))
                            style = 2;
                        else
                        {
                            beg();
                            if (moveTo("class=\"obituary-text\""))
                                style = 2;
                        }
                    }
                }
            }
        }
        break;

    case CFS:
    {
        if (moveTo("initV321Page()"))
            style = 2;
        else
        {
            beg();
            if (moveTo("initV31Obit()"))
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

    case Burke:
    case FrontRunner:
    {
        if (globals->globalDr->getProviderKey() == 255100)
            style = 7;
        else
        {
            if (moveTo("ObituariesList"))
                style = 6;
            else
            {
                beg();
                if (moveTo("funeraltechweb.com"))
                    style = 4;
                else
                {
                    beg();
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
                            {
                                beg();
                                if (moveTo("bom-tunnel-deceased"))
                                    style = 5;
                                else
                                    style = 2;
                            }
                        }
                    }
                }
            }
        }
    }
        break;

    case FuneralOne:
        if (globals->globalDr->getProviderKey() == 449)
        {
            if (moveTo("class=\"header-obituary\""))
                style = 0;
            else
                style = 1;
        }
        else
            style = 0;
        break;

    case WebStorm:
    {
        if (moveTo("Website Credits"))
            style = 2;
        else
        {
            beg();
            moveTo("<title>");
            tempString = getUntil(" ");
            if (tempString == OQString("Obituary"))
                style = 1;
            else
                style = 0;
        }
    }
        break;

    case Arbor:
    {
        if (moveTo("hero__lifetime"))
            style = 3;
        else
        {
            if ((globals->globalDr->getProviderKey() == 99) || globals->globalDr->getProviderKey() == 100)
                style = 2;
            else
            {
                beg();
                if (moveTo("SKYPE_TOOLBAR", 3000))
                    style = 1;
                else
                    style = 0;
            }
        }
    }
        break;

    case Codesign:
    {
        if (moveTo("<!-- #TRX_REVIEWS_PLACEHOLDER# -->"))
            style = 1;
        else
        {
            beg();
            if (moveTo("Digital Monk Marketing"))
                style = 2;
            else
            {
                beg();
                if (moveTo("digitalmonkmarketing.com"))
                    style = 3;
                else
                    style = 0;
            }
        }
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

    case FHW:
    {
        if (moveTo("Author: Elegant Themes"))
            style = 2;
        else
        {
            beg();
            if (moveTo("<!-- This is Squarespace. -->"))
                style = 1;
            else
                style = 0;
        }
    }
        break;

    case Specialty:
    {
        if (moveTo("class=\"contentPanel\""))
            style = 0;
        else
            style = 1;
    }
        break;

    case MCG:
    {
        if (globals->globalDr->getProviderKey() == 3)
            style = 1;
        else
            style = 0;
    }
        break;

    case Vortex:
    {
        switch(globals->globalDr->getProviderKey())
        {
        case 1:
            style = 0;
            break;

        case 2:
            style = 1;
            break;

        case 3:
            style = 2;
            break;
        }
    }
        break;

    case YellowPages:
    {
        style = globals->globalDr->getProviderKey() - 1;
    }
        break;

    case Ubeo:
    {
        if (globals->globalDr->getProviderKey() == 1)
            style = 0;
        else
            style = 1;
    }
        break;

    case Taiga:
    {
        if (globals->globalDr->getProviderKey() == 1)
            style = 0;
        else
            style = 1;
    }
        break;

    case PubliWeb:
    {
        if (globals->globalDr->getProviderKey() == 1)
            style = 0;
        else
            style = 1;
    }
        break;

    case Voyou:
    {
        if (globals->globalDr->getProviderKey() == 2)
            style = 1;
        else
        {
            if (globals->globalDr->getProviderKey() == 3)
                style = 2;
            else
                style = 0;
        }
    }
        break;

    case LogiAction:
    {
        if (globals->globalDr->getProviderKey() == 2)
            style = 1;
        else
            style = 0;
    }
        break;

    case District4Web:
    {
        if (globals->globalDr->getProviderKey() == 2)
            style = 1;
        else
            style = 0;
    }
        break;

    case Kerozen:
    {
        if (globals->globalDr->getProviderKey() == 2)
            style = 1;
        else
            style = 0;
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
    case SRS:
    {
        if (globals->globalDr->getProvider() == SRS)
            style = 2;
        else
        {
            if (moveTo("smartphones"))
                style = 1;
            else
                style = 0;
        }
    }
        break;

    case FRM:
    {
        switch(globals->globalDr->getProviderKey())
        {
        case 1:
        case 2:
        case 5:
        case 11:
        case 13:
        case 14:
            style = 1;
            break;

        case 3:
            style = 2;
            break;

        case 4:
        case 10:
        case 12:
            style = 3;
            break;

        case 100:
            style = 4;
            break;

        default:
            style = 0;
            break;
        }
    }
        break;

    case JBCote:
    {
        if (globals->globalDr->getProviderKey() == 3)
            style = 1;
        else
            style = 0;
    }
        break;

    case Simply:
    {
        if (moveTo(">Passed Away"))
            style = 0;
        else
            style = 1;
    }
        break;

    case Ruel:
    {
        if (moveTo("<title>Salon"))
            style = 0;
        else
            style = 1;
    }
        break;

    case Brunet:
    {
        if (moveTo("Yoast"))
            style = 1;
        else
            style = 0;
    }
        break;

    case Driftwood:
    {
        if (moveTo("tdb-block-inner td-fix-index"))
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
        uc.determineLanguageAndGender(justInitialNamesUC);
        globals->globalDr->setLanguage(uc.getLanguage());
        globals->globalDr->setGender(uc.getGender());
    }
}

void readObit::replaceProblematicChars()
{
    itsString.replace(QChar(160),QChar(32));    // non-breaking space
    itsString.replace(QChar(8203), "");         // zero-width space
    itsString.replace(QChar(8211), QChar(45));  // hyphen
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
    return static_cast<unsigned int>(uc.countFrequency(word, caseSensitivity));
}

unsigned int readObit::countFrequencyFirst(QString word, Qt::CaseSensitivity caseSensitivity) const
{
    return static_cast<unsigned int>(uc.countFrequencyFirst(word, caseSensitivity));
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

unstructuredContent* readObit::getUCaddress()
{
    return &uc;
}

unstructuredContent* readObit::getJustInitialNamesAddress()
{
    return &justInitialNamesUC;
}

unstructuredContent* readObit::getStructuredNamesAddress()
{
    return &structuredNamesProcessed;
}

mostCommonName* readObit::getMCNaddress()
{
    return &mcn;
}

void readObit::readParentsLastName()
{
    if ((uc.getLength() == 0) || (globals->globalDr->getNeeEtAlEncountered() == true))
        return;

    bool found = false;
    QList<QString> targetPhrases;
    QList<QString> parentFirstNames;
    QList<QString> brotherLastNames;
    QString targetText, tempText, sentence, allContent;
    OQString newSentence;
    unstructuredContent content, tempContent, sentenceWithMaiden;
    int i, index, startPosition, wordCount, wordCountAfterAnd, sentenceNum, nextSentenceNum;
    int totalParentNames, numMaleParentFirstNames, numFemaleParentFirstNames;
    double firstUnisex, secondUnisex, unisex;
    OQString word, tempWord, name, savedName, lastChar, nextWord, nextNextWord, priorWord, maleLastName, femaleLastName, brotherLastName;
    QString space(" ");
    QString period(".");
    QString semicolon(";");
    QString comma(",");
    bool keepGoing, wordIsAnd, andEncountered, compoundName, nextCompoundName, endOfNames, nextWordIsSurname, nextWordIsAnd, wordIsInitial, getAnotherWord, wasCapitalized;
    bool step1complete, step2complete, hadComma, valid, exception, nextWordIsInitial, justFatherName;
    bool isSurname, isGivenName, isAboriginal, isLikelySurname;
    bool savedNameIsLikelySurname = false;
    bool brotherNameIsLikelySurname = false;
    bool highConfidence = false;
    bool maleFirstNameEncountered = false;
    bool femaleFirstNameEncountered = false;
    bool motherMaidenNameIncluded = false;
    wordCount = 0;
    firstUnisex = secondUnisex = 0.5;
    GENDER currentGender;
    LANGUAGE lang = globals->globalDr->getLanguage();
    PQString tempName;
    NAMESTATS savedNameStats, nextWordNameStats;
    databaseSearches dbSearch;
    unstructuredContent tempUC;

    allContent = ucFillerRemoved.getString();
    allContent = allContent.simplified();

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
            content = ucFillerRemoved.getSentence(lang, ucFillerRemoved.getSentenceNum(index));
            sentence = content.getString();
            index = sentence.indexOf(targetText, 0, Qt::CaseInsensitive);
            sentence.remove(0, index);
            sentence.replace(QString("the late "), QString(""));
            sentence.replace(QString("de feu "), QString(""));
            content = sentence;
            content.removeContentWithin(PARENTHESES | QUOTES);

            // Start actual processing
            word = content.getWord(); // should be "brother[s:]
            if (!(word.removeEnding(period) || (word.right(3).lower() == OQString("law"))))
            {
                // Step 1 - Retrieve what should be first name of brother, excluding "Hank,"
                while (!step1complete && keepGoing && !content.isEOS())
                {
                    word = content.getWord();
                    hadComma = word.removeEnding(comma);
                    if (word.removeEnding(period) || !word.isCapitalized() || !dbSearch.givenNameLookup(word.getString(), globals, Male) || globals->globalDr->isASavedName(word))
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
                    valid = word.isCapitalized() || word.isAnd();
                    tempWord = word;
                    while (word.isUncapitalizedName() && !content.isEOS())
                    {
                        if (word.isSaint())
                            tempWord += period;
                        word = content.getWord();
                        word.removeEnding(PUNCTUATION);
                        tempWord += word;
                        valid = true;
                    }
                    word = tempWord.proper();
                    tempUC.compressCompoundNames(word, language_unknown);
                    if (!valid)
                        keepGoing = false;
                    isGivenName = isSurname = false;

                    while (!step2complete && keepGoing)
                    {
                        nextWord = content.peekAtWord(true, 1);
                        nextWord.removeEnding(PUNCTUATION);
                        nextWord.removePossessive();
                        getAnotherWord = word.isAnd() && !content.isEOS();
                        if (!getAnotherWord)
                        {
                            NAMESTATS nameStats;
                            dbSearch.nameStatLookup(word.getString(), globals, nameStats, Male);
                            isGivenName = nameStats.isGivenName;
                            isSurname = nameStats.isSurname;
                            brotherNameIsLikelySurname = nameStats.isLikelySurname;
                            if (isGivenName && !brotherNameIsLikelySurname && !content.isEOS())
                                getAnotherWord = true;
                            else
                            {
                                // Assume brand new name in limited circumstances
                                if (valid && !isGivenName && !isSurname && !priorWord.isAnd())
                                    step2complete = true;
                            }
                        }

                        if (getAnotherWord && nextWord.isCapitalized() && !content.isEOS() &&
                                (dbSearch.givenNameLookup(nextWord.getString(), globals) || (dbSearch.surnameLookup(nextWord.getString(), globals) > 0) || globals->globalDr->isASavedName(nextWord.getString())))
                        {
                            priorWord = word;
                            word = content.getWord();
                            word.removeEnding(PUNCTUATION);
                        }
                        else
                        {
                            if (isSurname && word.isCapitalized() && !priorWord.isAnd())
                                step2complete = true;
                            else
                                keepGoing = false;
                        }
                    }
                }

                if (step2complete)
                {
                    QList<QString> badWords = QString("sister|parent|outlaw").split("|");
                    QString first6 = word.left(6).lower().getString();

                    if (!badWords.contains(first6))
                    {
                        brotherLastNames.append(word.getString());
                        found = true;
                    }
                }
            }
        }
    }

    if (brotherLastNames.size() > 0)
        brotherLastName = brotherLastNames.at(0);

    // Now look at parent names
    targetPhrases.clear();
    targetPhrases = OQString::getParentReferences(lang, Female);
    found = false;
    nextSentenceNum = 0;
    index = -1;

    while (!found && targetPhrases.size() > 0)
    {
        startPosition = ucFillerRemoved.getSentenceStartPosition(nextSentenceNum);
        if ((index == -1) || (startPosition == -1))
        {
            startPosition = 0;
            targetText = targetPhrases.takeFirst();
        }

        index = allContent.indexOf(targetText, startPosition, Qt::CaseInsensitive);
        if (index >= 0)
        {
            keepGoing = true;
            andEncountered = false;
            endOfNames = false;
            nextWordIsSurname = false;
            femaleFirstNameEncountered = false;
            maleFirstNameEncountered = false;
            motherMaidenNameIncluded = false;
            wordIsInitial = false;
            wasCapitalized = false;
            name.clear();
            savedName.clear();
            parentFirstNames.clear();
            currentGender = genderUnknown;
            wordCount = 0;
            wordCountAfterAnd = 0;
            firstUnisex = 0.5;
            secondUnisex = 0.5;
            totalParentNames = 0;
            numMaleParentFirstNames = 0;
            numFemaleParentFirstNames = 0;
            if (targetText == QString("her father"))
                justFatherName = true;
            else
                justFatherName = false;

            // Prepare content to be processed
            if (targetText.left(1) == space)
            {
                targetText.remove(0, 1);
                index++;
            }
            sentenceNum = ucFillerRemoved.getSentenceNum(index);
            nextSentenceNum = sentenceNum + 1;
            content = ucFillerRemoved.getSentence(lang, sentenceNum);
            sentence = content.getString();
            index = sentence.indexOf(targetText, 0, Qt::CaseInsensitive);
            sentence.remove(0, index + targetText.size());
            sentence.replace(QString("the late "), QString(""));
            sentence.replace(QString("de feu "), QString(""));
            content = sentence;
            content.removeLeading(PUNCTUATION);
            sentenceWithMaiden = content;
            content.removeContentWithin(PARENTHESES | QUOTES);
            content.cleanUpEnds();
            sentenceWithMaiden.cleanUpEnds();

            // Begin processing
            while (keepGoing && !found && !highConfidence && !content.isEOS())
            {
                // Initialize variables
                if (!wordIsInitial)     // refers to prior word
                    priorWord = word;
                compoundName = false;
                nextCompoundName = false;

                // Get word
                word = content.getWord(true);
                while ((parentFirstNames.size() == 0) && word.isPrefix() && !content.isEOS())
                {
                    word = content.getWord(true);
                }

                word.removePossessive();
                wasCapitalized = word.isCapitalized();

                i = 2;
                exception = false;
                tempWord = word;
                while (!exception && tempWord.isUncapitalizedName() && !content.isEOS())
                {
                    exception = (word == OQString("E")) || (word == OQString("E."));

                    if (!exception)
                    {
                        tempWord = content.getWord(true);
                        word += tempWord;
                        i++;
                        wasCapitalized = true;
                    }
                }
                if (i > 2)
                    compoundName = true;

                if (word.isSaint())
                {
                    if (word.right(1) != period)
                        word += OQString(".");
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
                exception = false;
                while (!exception && tempWord.isUncapitalizedName() && !content.isEOS())
                {
                    exception = (tempWord == OQString("E")) || (tempWord == OQString("E."));

                    if (!exception)
                    {
                        tempWord = content.peekAtWord(true, i);
                        nextWord += tempWord;
                        i++;
                    }
                }
                if (i > 2)
                    nextCompoundName = true;
                nextWord.removeEnding(PUNCTUATION);

                if (globals->globalDr->isALastName(nextWord))
                    return;

                nextWordIsInitial = (nextWord.getLength() == 1) && !nextWord.isAnd();
                while (nextWordIsInitial)
                {
                    nextWord = content.getWord(true);
                    nextWord = content.peekAtWord(true, 1);
                    nextWord.removeEnding(PUNCTUATION);
                    nextWordIsInitial = (nextWord.getLength() == 1) && !nextWord.isAnd();
                }
                nextWordIsAnd = nextWord.isAnd();
                nextWord.removePossessive();

                dbSearch.nameStatLookup(nextWord.getString(), globals, nextWordNameStats, genderUnknown);
                nextWordIsSurname = nextCompoundName || (nextWord.isCapitalized() && (nextWord.getLength() >= 2) &&
                                                         (nextWordNameStats.isSurname || (!nextWordNameStats.isGivenName && !nextWord.isRecognized(lang))));

                // Analyze word
                lastChar = word.right(1);
                endOfNames = word.removeEnding(PUNCTUATION);
                isAboriginal = word.isAboriginalName(nextWord);
                compoundName = compoundName || isAboriginal;
                wordIsInitial = word.isInitial();

                // Assess if end of names attained
                endOfNames = (endOfNames && !wordIsInitial) || content.isEOS();
                if (!endOfNames && !nextWordIsSurname)
                {
                    if (andEncountered)
                        endOfNames = nextWord.isRecognized(lang);
                    else
                        endOfNames = !nextWordIsAnd && nextWord.isRecognized(lang);
                }
                if (endOfNames || !(nextWordIsSurname || nextWordIsAnd || nextWordIsInitial))
                    nextWord.clear();

                if (compoundName)
                    word = word.proper();

                if (word.isAnd())
                {
                    wordIsAnd = true;
                    andEncountered = true;
                    currentGender = genderUnknown;
                    parentFirstNames.clear();
                    if (wordCount == 0)
                    {
                        keepGoing = false;
                        endOfNames = true;
                        wordIsAnd = false;
                    }
                }
                else
                    wordIsAnd = false;

                // Determine if this is the last word to be considered
                if (((lastChar == period) && !word.isSaint() && !wordIsInitial) || (lastChar == semicolon) || endOfNames || (!compoundName && !wasCapitalized))
                    keepGoing = false;

                if (!word.hasBookEnds(PARENTHESES))
                {
                    NAMESTATS nameStats, nextNameStats;

                    if (wasCapitalized && !wordIsAnd && !wordIsInitial && !word.isSuffix())
                    {
                        // Stand alone analysis based on gender lookups
                        nameStats.clear();
                        dbSearch.nameStatLookup(word.getString(), globals, nameStats, genderUnknown);
                        isGivenName = nameStats.isGivenName;
                        isSurname = nameStats.isSurname;
                        isLikelySurname = nameStats.isLikelySurname;
                        totalParentNames++;

                        if ((isGivenName && !isLikelySurname && !endOfNames) || nextWordIsSurname)
                        {
                            if (isGivenName && !isLikelySurname)
                                parentFirstNames.append(word.getString());

                            nextNameStats.clear();
                            nextNameStats = nextWordNameStats;

                            if (nextNameStats.isGivenName && !(nextNameStats.isSurname || nextCompoundName))
                                parentFirstNames.append(nextWord.getString());

                            unisex = dbSearch.genderLookup(parentFirstNames, globals);

                            if (unisex >= 0.9)
                            {
                                maleFirstNameEncountered = true;
                                numMaleParentFirstNames++;
                                currentGender = Male;
                                if ((nextNameStats.isLikelySurname || nextWordIsSurname) && !endOfNames && !nextWordIsAnd)
                                    maleLastName = nextWord;
                                else
                                {
                                    if (endOfNames && femaleFirstNameEncountered && (femaleLastName.getLength() > 0))
                                        maleLastName = word;
                                }

                                if ((maleLastName.getLength() > 0) && femaleFirstNameEncountered && (maleLastName == femaleLastName))
                                {
                                    highConfidence = true;
                                    found = true;
                                    name = maleLastName;
                                }
                            }
                            else
                            {
                                if (unisex <= 0.1)
                                {
                                    femaleFirstNameEncountered = true;
                                    numFemaleParentFirstNames++;
                                    currentGender = Female;
                                    if ((nextNameStats.isLikelySurname || nextWordIsSurname) && !endOfNames)
                                        femaleLastName = nextWord;
                                    else
                                    {
                                        if (endOfNames && maleFirstNameEncountered && (maleLastName.getLength() > 0))
                                            femaleLastName = word;
                                    }

                                    if ((femaleLastName.getLength() > 0) && maleFirstNameEncountered && (maleLastName == femaleLastName))
                                    {
                                        highConfidence = true;
                                        found = true;
                                        name = femaleLastName;
                                    }

                                    if (numFemaleParentFirstNames == 1)
                                    {
                                        sentenceWithMaiden.beg();
                                        bool keepGoing = true;
                                        while (!sentenceWithMaiden.isEOS() && keepGoing)
                                        {
                                            tempWord = sentenceWithMaiden.getWord(true);
                                            if (word == tempWord)
                                            {
                                                keepGoing = false;
                                                if (!sentenceWithMaiden.isEOS())
                                                {
                                                    tempWord = sentenceWithMaiden.getWord(true);
                                                    if (tempWord.hasBookEnds(PARENTHESES))
                                                        motherMaidenNameIncluded = true;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (andEncountered)
                                    {
                                        if ((firstUnisex > 0.5) && (nameStats.femaleCount > 0))
                                            numFemaleParentFirstNames++;

                                        if ((firstUnisex < 0.5) && (nameStats.maleCount > 0))
                                            numMaleParentFirstNames++;
                                    }
                                    else
                                    {
                                        if (nameStats.malePct >= 0.5)
                                            numMaleParentFirstNames++;
                                        else
                                            numFemaleParentFirstNames++;
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (endOfNames && nameStats.isLikelySurname)
                                name = word;

                            if (!endOfNames && ((nameStats.maleCount > 0) || (nameStats.femaleCount > 0)))
                            {
                                if (andEncountered)
                                {
                                    if ((firstUnisex > 0.5) && (nameStats.femaleCount > 0))
                                        numFemaleParentFirstNames++;

                                    if ((firstUnisex < 0.5) && (nameStats.maleCount > 0))
                                        numMaleParentFirstNames++;
                                }
                                else
                                {
                                    if (nameStats.malePct >= 0.5)
                                        numMaleParentFirstNames++;
                                    else
                                        numFemaleParentFirstNames++;
                                }
                            }
                        }

                        // Resume normal process
                        if ((wordCount >= 2) && (wordCountAfterAnd >= 1) && !nextWordIsSurname && !highConfidence && keepGoing)
                        {
                            if ((firstUnisex >= 0.9) && !savedNameStats.isLikelyGivenName)
                                name += savedName;
                            else
                                name += word;
                            found = true;
                        }

                        wordCount++;
                        if (andEncountered)
                            wordCountAfterAnd++;

                    }
                    else
                    {
                        if ((wordCount >= 2) && (compoundName || word.isSaint()))
                        {
                            name += word;
                            wordCount++;
                            if (andEncountered)
                                wordCountAfterAnd++;
                        }
                        else
                        {
                            if (wordIsAnd || wordIsInitial)
                                keepGoing = true;
                            else
                            {
                                if (wasCapitalized && nextWordIsSurname)
                                    keepGoing = true;
                                else
                                    keepGoing = false;
                            }
                        }
                    }

                    // Retain stats and run some checks
                    if (!wordIsAnd && !wordIsInitial && wasCapitalized)
                    {
                        if (wordCount == 1)
                        {
                            QList<QString> firstName;
                            firstName.append(word.getString());
                            firstUnisex = dbSearch.genderLookup(firstName, globals);
                        }

                        if (andEncountered && (wordCountAfterAnd == 1))
                        {
                            QList<QString> secondName;
                            secondName.append(word.getString());
                            secondUnisex = dbSearch.genderLookup(secondName, globals);
                        }

                        if ((wordCount >= 2) && !andEncountered)
                        {
                            if (globals->globalDr->isALastName(word))
                                return;

                            bool keepName = isSurname || (word == brotherLastName);
                            if (!keepName)
                            {
                                if (!isGivenName && (content.isEOS() || nextWordIsAnd) && !word.isRecognized(lang))
                                    keepName = true;
                            }

                            if (keepName)
                            {
                                savedName = word;
                                savedNameStats = nameStats;
                                if (content.isEOS() && justFatherName)
                                {
                                    found = true;
                                    name = word;
                                }
                            }
                        }

                        if ((wordCountAfterAnd >= 2) && globals->globalDr->isALastName(word))
                            return;

                        if ((content.isEOS() || endOfNames) && (wordCountAfterAnd >= 2))
                        {
                            if (nameStats.isSurname || compoundName || (word == brotherLastName) || ((maleLastName.getLength() > 0) && (femaleLastName.getLength() > 0)))
                            {
                                found = true;
                                if ((maleLastName.getLength() > 0) && justInitialNamesUC.getString().contains(maleLastName.getString(), Qt::CaseInsensitive))
                                    name = maleLastName;
                                else
                                {
                                    if ((femaleLastName.getLength() > 0) && justInitialNamesUC.getString().contains(femaleLastName.getString(), Qt::CaseInsensitive))
                                        name = femaleLastName;
                                    else
                                    {
                                        if ((word == brotherLastName) || ((currentGender == Female) && motherMaidenNameIncluded))
                                            name = word;
                                        else
                                        {
                                            if (((totalParentNames % 2) == 1) && (numMaleParentFirstNames == numFemaleParentFirstNames) && (nameStats.isSurname || !nameStats.isGivenName))
                                                name = word;
                                            else
                                            {
                                                if ((maleLastName.getLength() > 0) && ((femaleLastName.getLength() == 0) || ((femaleLastName.getLength() > 0) && (maleLastName != femaleLastName))))
                                                   name = maleLastName;
                                                else
                                                    name = word;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                // Check if perhaps a new last name
                                int numFreq, numFreqFirst;
                                numFreq = uc.countFrequency(word.getString(), Qt::CaseInsensitive);
                                numFreqFirst = uc.countFrequencyFirst(word.getString(), Qt::CaseInsensitive);
                                if ((numFreq >= 2) && (numFreqFirst == 0))
                                {
                                    found = true;
                                    name = word;
                                }
                                else
                                {
                                    if (((maleFirstNameEncountered && femaleFirstNameEncountered) || ((numMaleParentFirstNames > 0) && (numFemaleParentFirstNames > 0))) && !nameStats.isGivenName)
                                    {
                                        found = true;
                                        name = word;
                                        if (globals->globalDr->isALastName(word))
                                            return;
                                    }
                                }
                            }
                        }
                    }
                }
            }   // end while running through text excerpt
        }
    }   // end while looping through potential parent references

    // Compare results of brothers to parents
    if (!found && (brotherLastName.getLength() > 0))
    {
        if (brotherNameIsLikelySurname || (name.getLength() == 0))
        {
            name = brotherLastName;
            highConfidence = true;
        }
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
        if (maleFirstNameEncountered && (maleLastName.getLength() == 0) && femaleFirstNameEncountered && (femaleLastName.getLength() == 0) && name.getLength() > 0)
            highConfidence = true;
        else
        {
            if (maleFirstNameEncountered && (maleLastName.getLength() == 0) && femaleFirstNameEncountered && (femaleLastName.getLength() > 0))
            {
                name = femaleLastName;
                highConfidence = true;
            }
            else
            {
                int freq;
                if (maleFirstNameEncountered && (maleLastName.getLength() > 0))
                {
                    freq = uc.countFrequency(maleLastName.getString(), Qt::CaseInsensitive);
                    if (freq >= 2)
                        highConfidence = true;
                }
                else
                {
                    if (femaleFirstNameEncountered && (femaleLastName.getLength() > 0))
                    {
                        freq = uc.countFrequency(femaleLastName.getString(), Qt::CaseInsensitive);
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
            if ((firstUnisex > 0.5) && (secondUnisex < 0.5) && (wordCount >= 4) && !priorWord.hasBookEnds(PARENTHESES) && savedNameIsLikelySurname)
                tempName = savedName;
            else
                tempName = name;
        }

        if ((tempName.left(6).lower() != PQString("sister")) && (tempName != PQString("Teresa")))
        {
            if (globals->globalDr->isALastName(tempName))
                highConfidence = true;

            globals->globalDr->setParentsLastName(tempName);
            globals->globalDr->removeFromMiddleNames(tempName);
            if (globals->globalDr->getGender() != Male)
                globals->globalDr->setFamilyName(tempName);
        }
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

void readObit::clear()
{
    filename.clear();
    uc.clear();
    ucCleaned.clear();
    ucFillerRemoved.clear();
    ucFillerRemovedAndTruncated.clear();
    justInitialNamesUC.clear();
    structuredNamesProcessed.clear();
    mcn.clear();
    nameStatsList.clear();

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
    // Fourth pass - Try to find DOB or YOB (different actions depending how far match occurs)
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

    QDate origDOD = globals->globalDr->getDOD();
    QDate origDOB = globals->globalDr->getDOB();
    QDate origMinDOB = globals->globalDr->getMinDOB();
    if (!origMinDOB.isValid())
        origMinDOB.setDate(1875,1,1);
    QDate origMaxDOB = globals->globalDr->getMaxDOB();
    if (!origMaxDOB.isValid())
        origMaxDOB = globals->today;
    int origYOD = static_cast<int>(globals->globalDr->getYOD());
    int origYOB = static_cast<int>(globals->globalDr->getYOB());
    unsigned int origAgeAtDeath = globals->globalDr->getAgeAtDeath();

    newDODinfo = dates.potentialDOD.isValid();
    newYODinfo = !newDODinfo && (dates.potentialYOD > 0);
    newDOBinfo = dates.potentialDOB.isValid();
    newYOBinfo = !newDOBinfo && (dates.potentialYOB > 0);
    forceDODoverride = false;
    forceDOBoverride = false;
    forceYODoverride = false;
    forceYOBoverride = false;
    YOBindirectIssueExists = false;

    // Attempt to flush out typo on new DOB, which is really the DOD
    if (newDOBinfo && (origYOB == 0) && origDOD.isValid())
    {
        if ((dates.potentialDOB.month() == origDOD.month()) && (dates.potentialDOB.day() == origDOD.day()))
        {
            dates.potentialDOB = QDate();
            newDOBinfo = false;
        }
    }

    // Look for obvious error in YOD being entered as YOB
    if (newDOBinfo && newDODinfo && (dates.potentialDOB.year() == dates.potentialDOD.year()) && (dates.potentialDOD.year() < (globals->today.year() - 5)))
    {
        QDate firstDate = getFirstSentenceSingleDate();
        if (firstDate.isValid() && (firstDate.month() == dates.potentialDOD.month()) && (firstDate.day() == dates.potentialDOD.day()))
            dates.potentialDOD = firstDate;
    }

    excludeInfo = ((pass == 8) && (dates.potentialDOD != origDOD)) || !dates.hasDateInfo();
    nonFixableDOBerror = (newDOBinfo && !newDODinfo && origDOD.isValid() && (dates.potentialDOB > origDOD)) ||
                         (newYOBinfo && !newYODinfo && (((origDOD.isValid() && (dates.potentialYOB > origDOD.year())) ||
                                                         ((origYOD > 0) && (dates.potentialYOB > origYOD)))));
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

    YODissueExists = newYODinfo && (origYOD > 0) && (origYOD != dates.potentialYOD);
    YOBissueExists = newYOBinfo && (origYOB > 0) && (origYOB != dates.potentialYOB);
    DODissueExists = newDODinfo && ((origDOD.isValid() && (dates.potentialDOD != origDOD)) || (!origDOD.isValid() && YODissueExists));
    DOBissueExists = newDOBinfo && ((origDOB.isValid() && (dates.potentialDOB != origDOB)) || (!origDOB.isValid() && YOBissueExists) ||
                                    (dates.potentialDOB < origMinDOB) || (dates.potentialDOB > origMaxDOB));

    // Assess if indirect YOB issue exists where ageAtDeath set
    if (newYOBinfo && (origYOB == 0) && (globals->globalDr->getAgeAtDeath() > 0))
    {
        int origMinYOB = origMinDOB.year();
        int origMaxYOB = origMaxDOB.year();
        bool consistent = ((dates.potentialYOB >= origMinYOB) && (dates.potentialYOB <= origMaxYOB));
        if (!consistent)
        {
            if (globals->globalDr->getAgeNextReference() && ((dates.potentialYOB + 1) == origMinYOB))
            {
                globals->globalDr->setMinDOB(origMinDOB.addYears(-1));
                globals->globalDr->setMaxDOB(origMaxDOB.addYears(-1));
                globals->globalDr->setAgeAtDeath(origAgeAtDeath + 1, true, true);
                globals->globalDr->setYOB(dates.potentialYOB);
                globals->globalDr->setAgeNextReference(false);
            }
            else
                YOBindirectIssueExists = true;
        }
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

        implementSolution = ((newDOBinfo && !origDOB.isValid() && (dates.potentialDOB <= origDOD)) || (pass == 3) || (pass == 9)) && (dates.potentialDOB != dates.potentialDOD);
        if (implementSolution)
        {
            daysDifferent = origDOD.daysTo(dates.potentialDOD);
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

        altDates = uc.contentKeyWordsAndDates(firstNameList, lang, maxSentences);
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
        bool useDefaultSolution = false;

        if (DOBissueExists)
        {
            // Implement fixes where reasonable assumptions can be made
            if (origDOB.isValid())
            {
                // This means the new DOB doesn't match the existing DOB - rely on credibility settings
                if (dates.fullyCredible && !globals->globalDr->getDOBcredibility())
                    fullyCredible = true;
                else
                    fullyCredible = false;

                globals->globalDr->setDOB(dates.potentialDOB, forceDOBoverride, fullyCredible);
                if (pass >= 1)
                    globals->globalDr->setDatesLocked(true);
            }
            else
            {
                // This means new DOB doesn't align with existing YOB or minDOB, maxDOB
                // If margin of error is two years or less (allows for incorrect YOB by one year), attempt to fix, otherwise rely on credibility setting

                bool highRangeError = false;
                bool lowRangeError = false;
                bool forceOverride = false;

                if ((dates.potentialDOB > origMaxDOB) && (elapse(origMaxDOB, dates.potentialDOB) < 2.0))
                    highRangeError = true;
                if ((dates.potentialDOB < origMinDOB) && (elapse(dates.potentialDOB, origMinDOB) < 2.0))
                    lowRangeError = true;

                if (origYOB > 0)
                {
                    if (dates.potentialDOB.year() == origYOB)
                    {
                        // Assume improperly reported age at death
                        forceOverride = true;
                        fullyCredible = false;
                        globals->globalDr->setDOB(dates.potentialDOB, forceOverride, fullyCredible);
                        // Remaining minDOB, maxDOB, YOB and ageAtDeath all get reset as well automatically
                        globals->globalDr->setAgeNextReference(false);
                        globals->globalDr->setDatesLocked(true);
                    }
                    else
                    {
                        // New DOB is just as likely to be wrong as YOB
                        if (highRangeError || lowRangeError)
                        {
                            forceOverride = true;
                            fullyCredible = true;
                            globals->globalDr->clearDOB();
                            globals->globalDr->setYOB(0, forceOverride, fullyCredible);
                            if (highRangeError)
                                globals->globalDr->setMaxDOB(dates.potentialDOB);
                            if (lowRangeError)
                                globals->globalDr->setMinDOB(dates.potentialDOB);
                            globals->globalDr->setAgeAtDeath(0, fullyCredible, forceOverride);
                            globals->globalDr->setAgeNextReference(false);
                            if (pass >= 1)
                                globals->globalDr->setDatesLocked(true);
                        }
                        else
                        {
                            if (((origYOD > 0) && (dates.potentialDOB.year() == origYOD)) || (dates.potentialDOB.year() == globals->today.year()))
                                useDefaultSolution = false;
                            else
                                useDefaultSolution = true;
                        }
                    }
                }
                else
                {
                    // New DOB is just as likely to be wrong as minDOB or maxDOB
                    if (highRangeError || lowRangeError)
                    {
                        forceOverride = true;
                        fullyCredible = true;
                        globals->globalDr->clearDOB();
                        if (highRangeError)
                            globals->globalDr->setMaxDOB(dates.potentialDOB);
                        if (lowRangeError)
                            globals->globalDr->setMinDOB(dates.potentialDOB);
                        globals->globalDr->setAgeAtDeath(0, fullyCredible, forceOverride);
                        globals->globalDr->setAgeNextReference(false);
                        if (pass >= 1)
                            globals->globalDr->setDatesLocked(true);
                     }
                    else
                        useDefaultSolution = true;
                }

                newYOBinfo = false;
            }
        }
        else
            useDefaultSolution = true;

        if (useDefaultSolution)
        {
            fullyCredible = dates.fullyCredible;
            if (fullyCredible)
                forceDOBoverride = true;
            globals->globalDr->setDOB(dates.potentialDOB, forceDOBoverride, fullyCredible);
        }
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
        if ((pass >= 1) && YODissueExists)
            globals->globalDr->setDatesLocked(true);
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
        if ((pass >= 1) && YOBissueExists)
            globals->globalDr->setDatesLocked(true);
    }

    return newDODinfo || newDOBinfo || newYODinfo || newYOBinfo;
}

bool readObit::missingDateInfo() const
{
    return (globals->globalDr->missingDOB() || globals->globalDr->missingDOD());
}

bool readObit::isLocation(OQString word)
{
    if (word.getLength() == 0)
        return false;

    QStringList words = word.getString().split(" ");
    QString currentWord;

    bool allMatched = true;
    while (allMatched && (words.size() > 0))
    {
        currentWord = words.takeFirst();
        allMatched = globals->websiteLocationWords.contains(currentWord, Qt::CaseInsensitive);
    }

    return allMatched;
}

void readObit::treatmentNameCleanup()
{
    if (globals->globalDr->wi.confirmTreatmentName.length() == 0)
        return;

    unstructuredContent tempUC;
    tempUC.compressCompoundNames(globals->globalDr->wi.confirmTreatmentName, globals->globalDr->getLanguage());
    OQStream tempStream(tempUC.getString());

    OQString word;
    bool partialMatch = false;
    bool allNames = true;
    bool allLastNames = true;
    bool allSaved = true;
    NAMESTATS nameStats;
    databaseSearches dbSearch;

    QList<QString> nameList;

    while (!tempStream.isEOS())
    {
        word = tempStream.getWord();
        word.removeEnding("COMMA", Qt::CaseSensitive);
        if (!word.isNeeEtAl())
        {
            if (globals->globalDr->isASavedName(word))
            {
                nameList.append(word.getString());
                partialMatch = true;
                if (!globals->globalDr->isALastName(word))
                    allLastNames = false;
            }
            else
            {
                allSaved = false;
                dbSearch.nameStatLookup(word.getString(), globals, nameStats, globals->globalDr->getGender());
                if (nameStats.isSurname || nameStats.isGivenName)
                    nameList.append(word.getString());
                if (!nameStats.isSurname)
                    allLastNames = false;
                else
                {
                    if (!nameStats.isGivenName)
                        allNames = false;
                }
            }
        }
    }

    if (allSaved)
        globals->globalDr->wi.confirmTreatmentName.clear();
    else
    {
        if (allLastNames)
        {
            NAMEINFO nameInfo;
            nameInfo.type = ntLast;
            OQString name;
            while (nameList.size() > 0)
            {
                name = nameList.takeFirst();
                nameInfo.name = name.proper();
                nameInfo.numWords = nameInfo.name.countWords();
                globals->globalDr->setAlternates(nameInfo);
            }
            globals->globalDr->wi.confirmTreatmentName.clear();
        }
        else
        {
            if (partialMatch && allNames)
            {
                QList<NAMESTATS> nameStatsList;
                unstructuredContent names;
                OQString name;
                while (nameList.size() > 0)
                {
                    name = nameList.takeFirst();
                    names +=  name + OQString(" ");
                    dbSearch.nameStatLookup(name.getString(), globals, nameStats, globals->globalDr->getGender());
                }

                names.readFirstNameFirst(nameStatsList);
                globals->globalDr->wi.confirmTreatmentName.clear();
            }
        }
    }
}

void readObit::finalNameCleanup()
{
    // Look for potential middlenames recorded as last names
    if ((globals->globalDr->getMaidenNames().length() == 0) && (globals->globalDr->getLastNameAlt2().getLength() > 0))
    {
        NAMESTATS nameStats;
        databaseSearches dbSearch;
        int startingLastNames = 3;
        int newMiddleNames = 0;
        int i;
        int pass;
        int nameCount;
        QList<int> movePositions;
        QList<QString> lastNames;
        QList<QString> middleNames;
        QString name;
        bool moveOver;
        double ratio;

        lastNames.append(globals->globalDr->getLastName().getString());
        lastNames.append(globals->globalDr->getLastNameAlt1().getString());
        lastNames.append(globals->globalDr->getLastNameAlt2().getString());

        if (globals->globalDr->getLastNameAlt3().getLength() > 0)
        {
            startingLastNames = 4;
            lastNames.append(globals->globalDr->getLastNameAlt3().getString());
        }

        pass = 1;
        while (pass <= 3)
        {
            i = 0;

            while (i < startingLastNames)
            {
                dbSearch.nameStatLookup(lastNames.at(i), globals, nameStats, globals->globalDr->getGender());
                if (pass == 1)
                    moveOver = nameStats.isLikelyGivenName;
                else
                {
                    if (globals->globalDr->getGender() == Male)
                        nameCount = nameStats.maleCount;
                    else
                    {
                        if (globals->globalDr->getGender() == Female)
                            nameCount = nameStats.femaleCount;
                        else
                            nameCount = nameStats.maleCount + nameStats.femaleCount;
                    }
                    ratio = static_cast<double>(nameCount) / static_cast<double>(nameCount + nameStats.surnameCount);
                    if (pass == 2)
                        moveOver = nameStats.isGivenName && (ratio > 0.5);
                    else
                        moveOver = nameStats.isGivenName && (ratio > 0.15);
                }
                if (moveOver)
                {
                    newMiddleNames++;
                    movePositions.append(i);
                }
                i++;
            }

            if (newMiddleNames == 0)
                pass++;
            else
            {
                pass = 4;
                if (newMiddleNames < startingLastNames)
                {
                    globals->globalDr->clearLastNames();

                    while(movePositions.size() > 0)
                    {
                        i = movePositions.takeLast();
                        middleNames.append(lastNames.takeAt(i));
                    }

                    globals->globalDr->setMiddleNames(PQString(QStringList(middleNames).join(" ")));

                    while (lastNames.size() > 0)
                        globals->globalDr->setFamilyName(PQString(lastNames.takeFirst()));
                }
            }
        }
    }

    // Look for potential first names recorded as last names
    if ((globals->globalDr->getFirstName().getLength() > 0) && (globals->globalDr->getLastNameAlt1().getLength() > 0))
    {
        OQString firstName, lastName;
        QList<QString> firstNameList, lastNameList;
        int i, j, position;
        bool nothingMoved;
        PQString errMsg;

        firstNameList.append(globals->globalDr->getFirstName().getString());
        if (globals->globalDr->getFirstNameAKA1().getLength() > 0)
            firstNameList.append(globals->globalDr->getFirstNameAKA1().getString());
        if (globals->globalDr->getFirstNameAKA2().getLength() > 0)
            firstNameList.append(globals->globalDr->getFirstNameAKA2().getString());

        lastNameList.append(globals->globalDr->getLastName().getString());
        lastNameList.append(globals->globalDr->getLastNameAlt1().getString());
        if (globals->globalDr->getLastNameAlt2().getLength() > 0)
            lastNameList.append(globals->globalDr->getLastNameAlt2().getString());
        if (globals->globalDr->getLastNameAlt3().getLength() > 0)
            lastNameList.append(globals->globalDr->getLastNameAlt3().getString());

        nothingMoved = true;
        for (i = 0; i < firstNameList.size(); i++)
        {
            firstName = firstNameList.at(i);
            for (j = 0; j < lastNameList.size(); j++)
            {
                lastName = lastNameList.at(j);
                if (nothingMoved && (firstName.isFormalVersionOf(lastName.getString(), errMsg) || lastName.isFormalVersionOf(firstName.getString(), errMsg)))
                {
                    nothingMoved = false;
                    position = j;
                    globals->globalDr->setFirstNames(lastName);
                }
            }
        }

        if (!nothingMoved)
        {
            globals->globalDr->clearLastNames();
            for (i = 0; i < lastNameList.size(); i++)
            {
                if (i != position)
                    globals->globalDr->setFamilyName(PQString(lastNameList.at(i)));
            }
            globals->globalDr->sortFirstNames();
        }
    }

    // Fix last name Senior for female
    if ((globals->globalDr->getSuffix() == PQString("Senior")) && (globals->globalDr->getGender() == Female) && (globals->globalDr->getLastNameAlt1().getLength() == 0))
    {
        QString name = globals->globalDr->getLastName().getString();
        NAMESTATS nameStats;
        databaseSearches dbSearch;
        dbSearch.nameStatLookup(name, globals, nameStats, Female);
        if (nameStats.isLikelyGivenName)
        {
            globals->globalDr->clearLastNames();
            globals->globalDr->setMiddleNames(name);
            globals->globalDr->setFamilyName("Senior");
        }
    }

    // Move a last name over if no first names
    if ((globals->globalDr->getFirstName().getLength() == 0) && (globals->globalDr->getLastNameAlt1().getLength() > 0))
    {
        QString name;
        QList<QString> nameList;
        NAMESTATS nameStats;
        databaseSearches dbSearch;
        double givenNamePct = 0;
        double ratio;
        int nameCount, position;

        nameList.append(globals->globalDr->getLastName().getString());
        nameList.append(globals->globalDr->getLastNameAlt1().getString());
        if (globals->globalDr->getLastNameAlt2().getLength() > 0)
            nameList.append(globals->globalDr->getLastNameAlt2().getString());
        if (globals->globalDr->getLastNameAlt3().getLength() > 0)
            nameList.append(globals->globalDr->getLastNameAlt3().getString());

        position = 0;
        for (int i = 0; i < nameList.size(); i++)
        {
            dbSearch.nameStatLookup(nameList.at(i), globals, nameStats, globals->globalDr->getGender());
            if (globals->globalDr->getGender() == Male)
                nameCount = nameStats.maleCount;
            else
            {
                if (globals->globalDr->getGender() == Female)
                    nameCount = nameStats.femaleCount;
                else
                    nameCount = nameStats.maleCount + nameStats.femaleCount;
            }
            ratio = static_cast<double>(nameCount) / static_cast<double>(nameCount + nameStats.surnameCount);
            if (ratio > givenNamePct)
            {
                givenNamePct = ratio;
                position = i;
            }
        }

        if (position > 0)
        {
            globals->globalDr->clearLastNames();
            globals->globalDr->setFirstName(PQString(nameList.at(position)));
            for (int i = 0; i < nameList.size(); i++)
            {
                if (i != position)
                    globals->globalDr->setFamilyName(PQString(nameList.at(i)));
            }
        }
    }

    // Review middles names of length == 2 to determine if
}

int readObit::runNameValidations()
{
    NAMESTATS nameStats;
    databaseSearches dbSearch;
    OQString name, firstChar;
    unsigned int count, size;
    int warningScore = 0;

    GENDER gender = globals->globalDr->getGender();
    QList<OQString> nameList;
    QList<QString> firstThreeLetters;
    OQStream tempStream;

    QList<QString> recognizedExclusions = QString("bishop|deacon|french|good|husband|major|wall|way").split("|");
    QList<QString> badWords = QString("of|and").split("|");

    globals->globalDr->reorderLee();

    name = globals->globalDr->getLastName();
    if (name.getLength() > 0)
    {
        nameList.append(name);
        name = globals->globalDr->getLastNameAlt1();
        if (name.getLength() > 0)
        {
            nameList.append(name);
            firstThreeLetters.append(name.left(3).getString());
            name = globals->globalDr->getLastNameAlt2();
            if (name.getLength() > 0)
            {
                nameList.append(name);
                firstThreeLetters.append(name.left(3).getString());
                name = globals->globalDr->getLastNameAlt3();
                if (name.getLength() > 0)
                {
                    firstThreeLetters.append(name.left(3).getString());
                    nameList.append(name);
                }
            }
        }
    }
    else
        warningScore += 15;


    // Checks on last names
    int i = 0;
    while (i < nameList.size())
    {
        name = nameList.at(i);
        size = name.getLength();

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
            dbSearch.nameStatLookup(name.getString(), globals, nameStats, genderUnknown);

        // Add points for errors
        if ((name.findPosition(PQString("(")) != -1) || (name.findPosition(PQString(")")) != -1))
            warningScore += 10;

        // Add points for "formerly of.." error
        if (name.isProblematicFirstName())
        {
            warningScore += 10;
            if (badWords.contains(name.lower().getString()))
                warningScore += 10;
        }

        // Add points for Males with more than one last name
        if ((gender == Male) && (nameList.size() > 1) && !globals->globalDr->getMaleHyphenated())
            warningScore += 10;

        // Add points if it is also a saved given name
        if (globals->globalDr->isAFirstName(name) || globals->globalDr->isAMiddleName(name))
            warningScore += 10;

        // Add points if it is equal to "Nee"
        if (name.isNeeEtAl())
            warningScore += 15;

        // Add points if surnames include sister
        if ((name.left(6) == PQString("sister")) || (name.left(5) == PQString("soeur")))
            warningScore += 20;

        // Add points if it is an initial
        if (size == 1)
            warningScore += 10;

        // Add points for unexpected abbreviations
        QList goodLastNames = QString("Cyr|Fry|Ng|Dyck").split("|");
        if ((size <= 4) && !name.containsVowel() && !goodLastNames.contains(name.getString()))
            warningScore += 10;

        // Add points for characters that must be errors
        if (name.getString().contains("&"))
            warningScore += 10;

        // Add points if word is recognized
        if (name.isRecognized() && !recognizedExclusions.contains(name.lower().getString()))
            warningScore += 15;

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
        {
            if (name.isFoundIn(problematicFemaleMiddleNames, 1))
            {
                if (!globals->globalDr->wi.nameWarningException)
                    warningScore += 5;
            }
            else
                warningScore += 5;
        }

        // Check for consumed duplicates
        for (j = 0; j < nameList.size(); j++)
        {
            if ((j != i) && nameList[i].getString().contains(nameList[j].getString(), Qt::CaseInsensitive))
                warningScore += 10;
        }

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

        // Add points if it is a location
        if (name.isLocation() || name.isFoundIn(routes, 1))
            warningScore += 10;

        if (count > 0)
            warningScore += 1;

        if (count > 100)
            warningScore += 1;

        i++;
    }

    // Checks on first names
    // Intentionally just checking first name for now
    i = 0;
    while (i < 3)
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
            // All validations except first few only apply to primary name

            // Add points for errors
            if ((name.findPosition(PQString("(")) != -1) || (name.findPosition(PQString(")")) != -1))
                warningScore += 10;

            firstChar = name.left(1);
            if (!name.isAlpha())
                warningScore += 10;

            // Add points for "formerly of.." error
            if (name.isProblematicFirstName())
            {
                warningScore += 10;
                if (badWords.contains(name.lower().getString()))
                    warningScore += 10;
            }

            if (name.isWrittenMonth(globals->globalDr->getLanguage()))
            {
                warningScore += 10;

                if ((gender == Female) && ((name == "April") || (name == "June") || (name == "Avril") || (name == "May")))
                    warningScore -= 5;
            }

            if (firstThreeLetters.contains(name.left(3).getString()))
                warningScore += 10;

            if (i == 0)
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
                    dbSearch.nameStatLookup(name.getString(), globals, nameStats, gender);

                if (nameStats.surnameCount > 0)
                    warningScore += 1;

                if (nameStats.isLikelySurname)
                    warningScore += 5;

                //if (name.isHyphenated() && (globals->globalDr->getLanguage() == english))
                //    warningScore += 10;

                if (name.isRecognized() && (name.lower() != OQString("ella")))
                    warningScore += 15;

                if (name.isPronoun() && (name.lower() != OQString("ella")))
                    warningScore += 15;

                if ((i == 0) && (name.getLength() < 2) && (globals->globalDr->getFirstNameAKA1().getLength() == 0))
                {
                    tempStream = globals->globalDr->getMiddleNames();
                    if (tempStream.getLength() > 2)
                        globals->globalDr->setFirstNames(tempStream.getWord());
                    else
                        warningScore += 10;
                }

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

                // Add points if it is a location
                if (name.isLocation() || name.isFoundIn(routes, 1))
                    warningScore += 10;

                if (count < 10)
                    warningScore += 1;

                if (count == 0)
                    warningScore += 3;
            }

            i++;
        }
        else
        {
            if (i == 0)
                warningScore += 15;
            i = 3;
        }
    }

    // Check on middlenames
    name = globals->globalDr->getMiddleNames();
    if (name.getLength() > 0)
    {
        // Add points for errors
        if ((name.findPosition(PQString("(")) != -1) || (name.findPosition(PQString(")")) != -1))
            warningScore += 10;

        if ((name.countWords() >= 3) || (name.getLength() > 40))
            warningScore += 8;
    }

    globals->globalDr->wi.nameFlagGeneral += warningScore;
    return warningScore;
}

void readObit::fixProviderSpecificIssues()
{
    QList<QString> tempList;

    // Provider specific issues
    switch (globals->globalDr->getProvider())
    {

    case 2003:
        if (globals->globalDr->getFirstNameAKA2() == PQString("Alberta"))
            globals->globalDr->setFirstName("", 3);
        if (globals->globalDr->getFirstNameAKA1() == PQString("Alberta"))
            globals->globalDr->setFirstName(globals->globalDr->getFirstNameAKA2(), 2);
        globals->globalDr->getMiddleNameList(tempList);
        if (tempList.contains("Ab"))
        {
            globals->globalDr->clearMiddleNames();
            for (int i = 0; i < tempList.size(); i++)
            {
                if (tempList.at(i) != "Ab")
                    globals->globalDr->setMiddleNames(PQString(tempList.at(i)));
            }
        }
        break;

    case 3001:
        if (!globals->globalDr->getDOD().isValid() && (globals->globalDr->getYOD() == 0) && globals->globalDr->getDOB().isValid() && (globals->globalDr->getYOB() >= 2000))
        {
            QDate DOD = globals->globalDr->getDOB();
            globals->globalDr->clearDates();
            globals->globalDr->setDOD(DOD);
        }
        break;

    default:
        break;
    }
}

int readObit::runRecordValidation()
{
    bool valid = true;
    bool finished = false;
    int check = 0;

    while (valid && !finished)
    {
        switch(check)
        {
        case 0:
            valid = !globals->globalDr->removeExtraneousCommas();
            break;

        case 1:
            if (globals->globalDr->getLastName().getLength() < 2)
                valid = false;
            break;

        case 2:
            if (globals->globalDr->getFirstName().getLength() == 0)
                valid = false;
            break;

        case 3:
            //if (globals->globalDr->getGender() == genderUnknown)
            //    valid = false;
            if (globals->globalDr->getYOD() == 0)
                valid = false;
            break;

        case 4:
            //if(!globals->globalDr->getDOD().isValid() || (globals->today < globals->globalDr->getDOD()) || (globals->globalDr->getDOD().daysTo(globals->today) > 30))
            if((globals->today < globals->globalDr->getDOD()) || (globals->globalDr->getDOD().daysTo(globals->today) > 180))
                valid = false;
            break;

        case 5:
            if (globals->globalDr->getLanguage() == language_unknown)
                valid = false;
            break;

        case 6:
            if (((globals->globalDr->wi.dateFlag >= 10) && (globals->globalDr->wi.dateFlag <= 20)) || (globals->globalDr->wi.genderFlag >= 10) ||
                (globals->globalDr->wi.nameFlagGeneral >= 10) || (globals->globalDr->wi.doubleMemorialFlag >= 10) || (globals->globalDr->wi.nameReversalFlag == 20))
                valid = false;
            break;

        case 7:
            if ((globals->globalDr->wi.confirmTreatmentName.length() > 0))
                valid = false;
            break;

        case 8:
            if ((globals->globalDr->getGender() == Male) && (globals->globalDr->getLastNameAlt1().getLength() > 0) && !globals->globalDr->getMaleHyphenated())
                valid = false;
            break;

        case 9:
            if ((globals->globalDr->getAgeAtDeath() < 18) && (globals->globalDr->getDOB().isValid()))
                valid = false;
            break;

        case 10:
            switch(globals->globalDr->getProvider())
            {
            case 16:
                if ((globals->globalDr->getProviderKey() == 4297) && !globals->globalDr->getDOD().isValid())
                    valid = false;
                break;

            case 1040:
                if (!globals->globalDr->getDOD().isValid())
                {
                    globals->globalDr->wi.dateFlag = 11;
                    valid = false;
                }
                break;

            default:
                break;
            }

        }
        check++;
        if (check > 10)
            finished = true;
    }

    if (valid)
    {
        globals->globalDr->wi.validated = 1;
        return 1;
    }
    else
    {
        if (globals->globalDr->permanentErrorFlag)
            globals->globalDr->wi.nameFlagGeneral = 99;
        return 0;
    }
}

int readObit::runGenderValidation()
{
    databaseSearches dbSearch;
    double unisex = dbSearch.genderLookup(globals->globalDr, globals);
    PQString writtenGender, errMsg;
    GENDER gender = globals->globalDr->getGender();

    switch (gender)
    {
    case Male:
        writtenGender = PQString("Male");
        break;

    case Female:
        writtenGender = PQString("Female");
        break;

    case genderUnknown:
        if (unisex >= 0.9)
        {
            globals->globalDr->setGender(Male);
            writtenGender = PQString("Male");
        }

        if (unisex <= 0.1)
        {
            globals->globalDr->setGender(Female);
            writtenGender = PQString("Female");
        }
    }

    if (((unisex >= 0.975) && (gender == Female)) || ((unisex <= 0.025) && (gender == Male)))
    {
        globals->globalDr->wi.genderFlag = static_cast<int>(gender) + 10;
        return 0;
    }
    else
    {
        if (((unisex >= 0.875) && (gender == Female)) || ((unisex <= 0.125) && (gender == Male)))
        {
            globals->globalDr->wi.genderFlag = static_cast<int>(gender);
            return 0;
        }
        else
            return 1;
    }
}

void readObit::fixNameIssues()
{
    // Attempts to fix name issues and warnings
    if ((globals->globalDr->wi.nameReversalFlag >= 10) && (globals->globalDr->wi.nameReversalFlag < 20))
        runAlternateNameProcessing1();

    if (globals->globalDr->getLastName().getLength() == 0)
    {
        bool revised = false;
        revised = reorderNames();

        if (revised && false)
        {
            globals->globalDr->wi.resetNonDateValidations();
            runNameValidations();
            runGenderValidation();
            runRecordValidation();
        }
    }

    if ((globals->globalDr->getLastName().getLength() > 0) && (globals->globalDr->getFirstName().getLength() == 0) && (globals->globalDr->getMiddleNames().getLength() > 0))
    {
        QString firstName, middleNames;
        PQString errMsg;
        unstructuredContent origMiddleNames(globals->globalDr->getMiddleNames());

        firstName = origMiddleNames.getWord().getString();
        if (OQString(firstName).isAGivenName(errMsg))
        {
            globals->globalDr->setFirstNames(firstName);
            globals->globalDr->clearMiddleNames();
            while(!origMiddleNames.isEOS()){
                globals->globalDr->setMiddleNames(origMiddleNames.getWord());}
        }
    }

    if ((globals->globalDr->getGender() == Male) && !globals->globalDr->getMaleHyphenated() && (globals->globalDr->getMiddleNames().getLength() == 0) && (globals->globalDr->getLastNameAlt1().getLength() > 0) && (globals->globalDr->getLastNameAlt2().getLength() == 0))
    {
        NAMESTATS nameStat1, nameStat2;
        databaseSearches dbSearch;

        PQString lastName1 = globals->globalDr->getLastName();
        PQString lastName2 = globals->globalDr->getLastNameAlt1();

        dbSearch.nameStatLookup(lastName1.getString(), globals, nameStat1, Male);
        dbSearch.nameStatLookup(lastName2.getString(), globals, nameStat2, Male);

        if (nameStat1.isLikelySurname && nameStat2.isLikelyGivenName)
        {
            globals->globalDr->setMiddleNames(lastName2);
            globals->globalDr->removeFromLastNames(lastName2);
        }

        if (nameStat2.isLikelySurname && nameStat1.isLikelyGivenName)
        {
            globals->globalDr->setMiddleNames(lastName1);
            globals->globalDr->removeFromLastNames(lastName1);
        }
    }

    if ((globals->globalDr->getMiddleNames().getLength() == 0) && (globals->globalDr->getLastNameAlt3().getLength() > 0))
    {
        databaseSearches dbSearch;
        QList<NAMESTATS> nameStats;
        QList<OQString> lastNames;
        NAMESTATS nameStat;

        unsigned int minSurnameCount = 999999;
        unsigned int maxGivenCount = 0;
        unsigned int count;
        int iMin = 0;
        int iMax = 0;

        lastNames.append(globals->globalDr->getLastName());
        lastNames.append(globals->globalDr->getLastNameAlt1());
        lastNames.append(globals->globalDr->getLastNameAlt2());
        lastNames.append(globals->globalDr->getLastNameAlt3());

        for (int i = 0; i < 4; i++)
        {
            nameStats.append(nameStat);
            dbSearch.nameStatLookup(lastNames.at(i).getString(), globals, nameStats[i]);
            if (nameStats[i].surnameCount <= minSurnameCount)
            {
                minSurnameCount = nameStats[i].surnameCount;
                iMin = i;
            }

            switch(globals->globalDr->getGender())
            {
            case Male:
                count = nameStats[i].maleCount;
                break;

            case Female:
                count = nameStats[i].femaleCount;
                break;

            default:
                count = nameStats[i].femaleCount + nameStats[i].maleCount;
                break;
            }

            if (count >= maxGivenCount)
            {
                maxGivenCount = count;
                iMax = i;
            }
        }

        if (minSurnameCount == 0)
        {
            globals->globalDr->clearLastNames();
            lastNames.removeAt(iMin);
            globals->globalDr->setFamilyNames(lastNames);
        }
        else
        {
            if (maxGivenCount > 10)
            {
                globals->globalDr->setMiddleNames(lastNames.at(iMax));

                globals->globalDr->clearLastNames();
                lastNames.removeAt(iMax);
                globals->globalDr->setFamilyNames(lastNames);
            }
        }
    }
}

void readObit::determinePotentialFirstName()
{
    // Strategy is to identify the first given name used in a real sentence (not in first two words)

    QString word, firstNameBackup;
    OQString Oword, nextWord;
    unstructuredContent sentence;
    bool isGivenName = false;
    databaseSearches dbSearch;
    NAMESTATS nameStats;
    GENDER gender = globals->globalDr->getGender();
    PQString errMsg;
    int wordCount = 0;

    ucFillerRemovedAndTruncated.beg();
    sentence = ucFillerRemovedAndTruncated.getNextRealSentence();

    if (sentence.getLength() > 0)
    {
        sentence.beg();
        Oword = sentence.getWord();
        nextWord = sentence.peekAtWord();
        wordCount++;

        dbSearch.nameStatLookup(Oword.getString(), globals, nameStats, gender);
        if (nameStats.isLikelyGivenName && !Oword.isFoundIn(problematicFirstNames))
            firstNameBackup = Oword.getString();

        while (!sentence.isEOS() && (nextWord.isAGivenName(errMsg) || nextWord.isALastName(errMsg)))
        {
           Oword = sentence.getWord();
           nextWord = sentence.peekAtWord();
           wordCount++;
        }

        while (!sentence.isEOS() && !isGivenName)
        {
            Oword = sentence.getWord();
            word = Oword.getString();
            nextWord = sentence.peekAtWord();
            wordCount++;
            isGivenName = Oword.isCapitalized() && !Oword.isProblematicFirstName() && !nextWord.isHyphen() &&
                            (dbSearch.givenNameLookup(word, globals, gender) || (word == globals->globalDr->getUsedFirstNameFromStructured()) || (word == globals->globalDr->getUsedFirstNameFromUnstructured()));
        }

        if (isGivenName)
            globals->globalDr->setFirstGivenNameUsedInSentence(word);
        else
        {
            if ((wordCount > 6) && (firstNameBackup.length() > 0))
                globals->globalDr->setFirstGivenNameUsedInSentence(firstNameBackup);
        }
    }
}

void readObit::runStdProcessing(unstructuredContent &uc, int insertPeriods)
{
    uc.unQuoteHTML();
    uc.replaceHTMLentities();
    uc.fixOneLargeQuoteBlock();
    uc.insertBreaks();
    uc.simplify(true);
    uc.removeLinks();
    uc.removeHTMLtags(insertPeriods);
    uc.removeBlankSentences();
    uc.conditionalBreaks();
    uc.standardizeQuotes();
    uc.fixBasicErrors();
    uc.simplify(false);
    uc.removeBlankSentences();
    uc.cleanUpEnds();
    uc.removeLeadingJunk();
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
            genderByUnionReference();
            break;

        case 2:
            genderByNameLookup();
            break;

        case 3:
            genderByAltName();
            break;

        case 4:
            globals->globalDr->setGender(globals->globalDr->getWorkingGender());
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
    unsigned int numChecked = 0;
    unsigned int maleCount = ucCleaned.getNumMaleWords();
    unsigned int femaleCount = ucCleaned.getNumFemaleWords();
    unsigned int startingCount = maleCount + femaleCount;
    unsigned int maxSentences = 3;  // information pulled from fourth or later sentence could be anything, so stop reading after three sentences
    bool restrictNamesToDR = false;
    if (hasGodReference())
        maleCount = 0;
    unstructuredContent sentence;
    ucCleaned.beg();

    while (!ucCleaned.isEOS() && (numChecked < maxSentences))
    {
        sentence = ucCleaned.getNextRealSentence(restrictNamesToDR, 3);
        sentence = OQString(" ") + sentence;
        sentence.genderRelationalReferences(maleCount, femaleCount);
        numChecked++;

        /*sentence = ucCleaned.getSentence();
        if (!(sentence.hasBookEnds(PARENTHESES) || sentence.isJustDates() || sentence.isJustNames() || sentence.startsWithClick()))
        {
            sentence = OQString(" ") + sentence;
            sentence.genderRelationalReferences(maleCount, femaleCount);
            numChecked++;
        }*/
    }

    if ((maleCount + femaleCount) > startingCount)
    {
        if ((maleCount > 0) && (maleCount > femaleCount))
        {
            if ((maleCount - femaleCount) >= 2)
                globals->globalDr->setGender(Male);
            else
            {
                if (globals->globalDr->getWorkingGender() != Female)
                    globals->globalDr->setWorkingGender(Male);
            }
        }

        if ((femaleCount > 0) && (femaleCount > maleCount))
        {
            if ((femaleCount - maleCount) >= 2)
                globals->globalDr->setGender(Female);
            else
            {
                if (globals->globalDr->getWorkingGender() != Male)
                    globals->globalDr->setWorkingGender(Female);
            }
        }
    }
}

void readObit::genderByTitle()
{
    // Only works after ReadStructured has executed

    bool unknownGender = (globals->globalDr->getGender() == genderUnknown);
    LANGUAGE lang = globals->globalDr->getLanguage();
    unstructuredContent sentence;
    bool restrictNamesToDR = false;

    if (unknownGender)
    {
        ucCleaned.beg();

        OQString word;
        GENDER potentialGender = genderUnknown;

        sentence = ucCleaned.getNextRealSentence(restrictNamesToDR, 3);
        /*sentence = ucCleaned.getSentence();
        while (sentence.isJustDates() || sentence.isJustNames() || sentence.startsWithClick(true))
            sentence = ucCleaned.getSentence();*/

        sentence.beg();
        word = sentence.getWord();
        if (word.isTitle(lang))
        {
            if (word.isMaleTitle(lang))
                potentialGender = Male;
            if (word.isFemaleTitle(lang))
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
    int countDiff = getNumMaleWords() - getNumFemaleWords();
    if (globals->globalObit->hasGodReference())
        countDiff = 0;
    if (countDiff > 3)
        countDiff = 3;
    if (countDiff < -3)
        countDiff = -3;

    double maleThreshold, femaleThreshold;
    databaseSearches dbSearch;

    switch (countDiff)
    {
    case 3:
        maleThreshold = 0.750;
        femaleThreshold = 0.025;
        break;

    case 2:
        maleThreshold = 0.800;
        femaleThreshold = 0.050;
        break;

    case 1:
        maleThreshold = 0.850;
        femaleThreshold = 0.075;
        break;

    case 0:
        maleThreshold = 0.900;
        femaleThreshold = 0.100;
        break;

    case -1:
        maleThreshold = 0.925;
        femaleThreshold = 0.150;
        break;

    case -2:
        maleThreshold = 0.950;
        femaleThreshold = 0.200;
        break;

    case -3:
        maleThreshold = 0.975;
        femaleThreshold = 0.250;
        break;
    }

    double unisex = dbSearch.genderLookup(globals->globalDr, globals);

    if (globals->globalDr->getGender() == genderUnknown)
    {
        if (unisex >= maleThreshold)
            globals->globalDr->setGender(Male);
        else
        {
            if (unisex <= femaleThreshold)
                globals->globalDr->setGender(Female);
        }
    }
}

void readObit::saveStructuredNamesProcessed(QString namesProcessed)
{
    structuredNamesProcessed = namesProcessed;
}

unstructuredContent readObit::getStructuredNamesProcessed() const
{
    return structuredNamesProcessed;
}

bool readObit::runAlternateNameProcessing1()
{
    QList<NAMESTATS> nameStatList;
    dataRecord origRecord = *globals->globalDr;
    int origNameCount = origRecord.countNames();
    int JINnameCount = justInitialNamesUC.countWords();
    bool reset = false;

    if (origNameCount == JINnameCount)
    {
        globals->globalDr->clearNames();
        globals->globalDr->wi.nameReversalFlag = 0;
        globals->globalDr->wi.nameFlagGeneral = 0;

        if (justInitialNamesUC.contains(","))
            justInitialNamesUC.readLastNameFirst(nameStatList);
        else
            justInitialNamesUC.readFirstNameFirst(nameStatList);

        reset = true;
        runNameValidations();
        if (!((globals->globalDr->wi.nameReversalFlag == 0) && (globals->globalDr->wi.nameFlagGeneral <= origRecord.wi.nameFlagGeneral)))
        {
            globals->globalDr->copyNames(origRecord);
            runAlternateNameProcessing1b();
        }
        else
        {
            // Go with the new ording and re-run other validations
            runGenderValidation();
            runRecordValidation();
        }
    }
    else
    {
        unstructuredContent revisedNames = getStructuredNamesProcessed();
        if (revisedNames.contains(","))
        {
            globals->globalDr->clearNames();
            globals->globalDr->wi.nameReversalFlag = 0;
            globals->globalDr->wi.nameFlagGeneral = 0;

            revisedNames.replace(",", "");
            revisedNames.readFirstNameFirst(nameStatList);

            reset = true;
            runNameValidations();
            if (!((globals->globalDr->wi.nameReversalFlag == 0) && (globals->globalDr->wi.nameFlagGeneral <= origRecord.wi.nameFlagGeneral)))
                globals->globalDr->copyNames(origRecord);
            else
            {
                // Go with the new ording and re-run other validations
                runGenderValidation();
                runRecordValidation();
            }
        }
        else
            reset = runAlternateNameProcessing1b();
    }

    return reset;
}

bool readObit::runAlternateNameProcessing1b()
{
    QList<NAMESTATS> nameStatList;
    dataRecord origRecord = *globals->globalDr;
    bool reset = false;

    unstructuredContent revisedNames = getStructuredNamesProcessed();
    if (revisedNames.contains(","))
        return reset;

    NAMESTATS nameStats1, nameStats2;
    databaseSearches dbSearch;

    QString firstName = revisedNames.getWord().getString();
    QString secondName = revisedNames.getWord().getString();

    dbSearch.nameStatLookup(firstName, globals, nameStats1, globals->globalDr->getGender());
    dbSearch.nameStatLookup(secondName, globals, nameStats2, globals->globalDr->getGender());

    if (nameStats1.isLikelySurname && nameStats2.isLikelyGivenName)
    {
        globals->globalDr->clearNames();
        globals->globalDr->wi.nameReversalFlag = 0;
        globals->globalDr->wi.nameFlagGeneral = 0;

        revisedNames = firstName + QString(", ") + revisedNames.right(revisedNames.getLength() - firstName.length() - 1).getString();
        revisedNames.readLastNameFirst(nameStatList);

        reset = true;
        globals->globalDr->wi.resetNonDateValidations();
        runNameValidations();
        if (!((globals->globalDr->wi.nameReversalFlag == 0) && (globals->globalDr->wi.nameFlagGeneral <= origRecord.wi.nameFlagGeneral)))
            globals->globalDr->copyNames(origRecord);
        else
        {
            // Go with the new ording and re-run other validations
            runGenderValidation();
            runRecordValidation();
        }
    }
    return reset;
}

bool readObit::reorderNames()
{
    // Run if last name is blank
    bool reset = false;
    bool allFirstNames = true;
    int nameCount = 0;
    int i = 0;

    if (globals->globalDr->getLastName().getLength() > 0)
        return reset;

    unstructuredContent revisedNames = getStructuredNamesProcessed();
    if (revisedNames.contains(","))
        return reset;

    NAMESTATS nameStats;
    databaseSearches dbSearch;
    QString name;

    while (!revisedNames.isEOS())
    {
        name = revisedNames.getWord().getString();
        nameCount++;

        dbSearch.nameStatLookup(name, globals, nameStats, globals->globalDr->getGender());
        allFirstNames = allFirstNames && nameStats.isGivenName;
    }

    if (allFirstNames && (nameCount >= 2))
    {
        revisedNames.beg();
        globals->globalDr->clearNames();
        reset = true;

        while (!revisedNames.isEOS())
        {
            name = revisedNames.getWord().getString();
            i++;

            if (i == 1)
                globals->globalDr->setFirstNames(name);
            else
            {
                if (i == nameCount)
                    globals->globalDr->setFamilyName(name);
                else
                    globals->globalDr->setMiddleNames(name);
            }
        }
    }

    return reset;
}

void readObit::readInObitAddress()
{
    databaseSearches dbSearch;

    QString pc = dbSearch.lookupPostalCode(globals, globals->globalDr->getProvider(), globals->globalDr->getProviderKey());
    if ((pc.length() >= 6) && (pc.length() <= 7))
        globals->globalDr->setPostalCode(pc);
}

void readObit::readInCustomAddress()
{
    QString pc;
    QString location;
    databaseSearches dbSearch;

    QString searchArea;
    QRegularExpression pcTarget;
    QRegularExpressionMatch match;
    pcTarget.setPattern("([A-Z][0-9][A-Z])\\s?([0-9][A-Z][0-9])");

    PROVIDER providerID = globals->globalDr->getProvider();
    unsigned int providerKey = globals->globalDr->getProviderKey();
    beg();

    switch(providerID)
    {
    case 6:
        // Location is read in UpdateDeceased
        break;

    case 15:
        if (consecutiveMovesTo(20, "postal_code", ":"))
            pc = readNextBetween(QUOTES).getString();
        break;

    case 1000:
        if (moveTo("<footer>"))
        {
            searchArea = getUntil("</footer>").getString();
            match = pcTarget.match(searchArea);

            if (match.hasMatch())
                pc = match.captured(1) + QString(" ") + match.captured(2);
        }
        break;

    case 1010:
        if ((providerKey == 44) && !globals->globalDr->getPostalCodeInfo().isValid())
        {
            if (moveTo("Milverton"))
                pc = QString("N0K 1M0");
            else
                pc = QString("N3A 1J5");
        }
        break;

    case 1080:
        if (providerKey == 1)
        {
            if (consecutiveMovesTo(50, "class=\"texte-adresse\"", "href="))
            {
                location = readNextBetween(QUOTES).getString();
                pc = dbSearch.pcLookup(globals, providerID, providerKey, location);
            }
        }
        break;

    case 1086:
        if (moveTo("class=\"deces-lieu\""))
        {
            location = readNextBetween(BRACKETS).getString();
            pc = pc = dbSearch.pcLookupPlaces(globals, providerID, providerKey, location).getPostalCode();
        }
        break;

    case 1093:
        if (consecutiveMovesTo(250, "class=\"location\"", "<p>"))
        {
            searchArea = getUntil("</p>").getString();
            match = pcTarget.match(searchArea);

            if (match.hasMatch())
                pc = match.captured(1) + QString(" ") + match.captured(2);
        }
        break;

    case 1096:
        if (providerKey == 3)
        {
            if (moveTo("class=\"vy_deces_details_ville"))
            {
                location = readNextBetween(BRACKETS).getString();
                //globals->globalDr->wi.checkInclName = location;
                pc = dbSearch.pcLookupPlaces(globals, providerID, providerKey, location).getPostalCode();
            }
        }
        break;

    case 1126:
        if (providerKey == 1)
        {
            QString word;
            OQString OWord;
            QStringList leadInWords = QString("À|De|From").split("|");

            if (consecutiveMovesTo(50, "class=\"deces-text\"", "<p>"))
            {
                uc = getUntil("</p>");
                word = uc.getWord().getString();
                if (leadInWords.contains(word))
                {
                    OWord = uc.getWord();
                    OWord.removeEnding(COMMA);
                    location = OWord.getString();
                }
                else
                {
                    beg();
                    if (moveTo("Hemmingford"))
                        location = QString("Hemmingford");
                    else
                    {
                        beg();
                        if (moveTo("Saint-Michel"))
                            location = QString("Saint-Michel");
                        else
                        {
                            beg();
                            if (moveTo("Saint-Édouard"))
                                location = QString("Saint-Édouard");
                            else
                            {
                                beg();
                                if (moveTo("Sherrington"))
                                    location = QString("Sherrington");
                                else
                                {
                                    beg();
                                    if (moveTo("Napierville"))
                                        location = QString("Napierville");
                                }
                            }
                        }
                    }
                }
                pc = dbSearch.pcLookupPlaces(globals, providerID, providerKey, location).getPostalCode();
            }
        }
        break;

    case 1132:
        if (moveTo("<!-- .entry-header -->"))
        {
            location = getUntil("<!-- .entry-content -->").getString();
            if (location.contains("Passage Funeral Co-operative"))
                pc = QString("E4R 1T3");
            else
            {
                if (location.contains("Chartersville Funeral Home"))
                    pc = QString("E1A 1G1");
                else
                {
                    if (location.contains("Passage Funeral Chapel"))
                        pc = QString("E1C 2P1");
                }
            }
        }

        break;

    case 1144:
        if (consecutiveMovesTo(1000, "id=\"headingTwo\"", "class=\"panel-body\""))
        {
            location = readNextBetween(BRACKETS).getString();
            pc = dbSearch.pcLookup(globals, providerID, providerKey, location);
        }
        break;

    case 1167:
        if (consecutiveMovesTo(50, "class=\"date\"", "<p>"))
        {
            location = getUntilEarliestOf("<br>", "</p>").getString();
            pc = dbSearch.pcLookup(globals, providerID, providerKey, location);
        }
        break;

    case 2147:
        if (moveTo("https://www.hommagenb.com/Location/"))
        {
            location = getUntil("/").getString();
            if (location != "Autre")
                pc = dbSearch.pcLookup(globals, providerID, providerKey, location);
        }
        break;

    case 2152:
        if (consecutiveMovesTo(1000, "8e1bd6b", "</span>", "</span>"))
        {
            backward(14);
            pc = getUntil("<").getString();
        }
        break;

    case 2153:
        if (moveTo(", BC V"))
        {
            backward(1);
            pc = getUntil("<").getString();
        }
        break;

    default:
        break;
    }

    if (pc.length() == 6)
        pc = pc.left(3) + QString(" ") + pc.right(3);

    if (pc.length() == 7)
        globals->globalDr->setPostalCode(pc);
}

void readObit::readImageNameInfo()
{
    QString tempString;
    int yod, mod, dod;
    QDate potentialDate;
    bool validDate = false;

    switch(globals->globalDr->getProvider())
    {
    case DignityMemorial:
        if (!globals->globalDr->getDOD().isValid())
        {
            beg();
            if (consecutiveMovesTo(150, "meta property=\"og:image\"", globals->globalDr->getID().getString()))
            {
                forward(1);
                tempString = getUntil("_").getString();
                if (tempString.length() == 8)
                {
                    yod = tempString.left(4).toInt();
                    mod = tempString.mid(4,2).toInt();
                    dod = tempString.right(2).toInt();
                    potentialDate = QDate(yod,mod,dod);
                    if (potentialDate.isValid())
                    {
                        validDate = true;
                        if (globals->globalDr->getDOB().isValid())
                        {
                            if (potentialDate < globals->globalDr->getDOB())
                                validDate = false;
                        }
                        else
                        {
                            if (globals->globalDr->getYOB() > 0)
                            {
                                if (potentialDate.year() < static_cast<int>(globals->globalDr->getYOB()))
                                    validDate = false;
                            }
                        }

                        if (globals->globalDr->getYOD() > 0)
                        {
                            if (potentialDate.year() != static_cast<int>(globals->globalDr->getYOD()))
                                validDate = false;
                        }

                        if (validDate)
                            globals->globalDr->setDOD(potentialDate);
                    }
                }
            }
        }
        break;

    default:
        break;
    }
}
