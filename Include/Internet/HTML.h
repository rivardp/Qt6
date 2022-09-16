//	HTML.h		Version 1.0		November 20, 2015

//  Used to create HTML output

#ifndef HTML_H
#define HTML_H

#include "PString.h"
#include "tables.h"

#include <fstream>

class POFOutput;

class POFHeader
{
public:
	POFHeader();
	~POFHeader();

	void setTopLeft(PString &tl);
	void setTopCenter(PString &tc);
	void setTopRight(PString &tr);
	void exportInfo(std::basic_fstream<TCHAR> &stream) const;
	
private:
	PString TopLeft;
	PString TopCenter;
	PString TopRight;

};

class POFOutput
{

	friend class POFHeader;

public:
	POFOutput(PString &POFfilename);
	~POFOutput();

	void startDoc();
	void endDoc();
	void addMeta();
	void addTable(Table &table);
	void addParagraph(PString &para, unsigned int crlf = 1);
	void addHeading(PString &heading, unsigned int level = 1);
	void setTitle(PString &screenTitle);
	void setPlanName(PString &planName);
	void startNewPage();
	std::basic_fstream<TCHAR>& getStream();
	PString getPlanName() const;

	void POFtoHTML();

	POFHeader pageHeader;

private:
//	unsigned int indent;
	PString title;
	PString filename;
	PString planname;
	std::basic_fstream<TCHAR> streamname;
//	int currentPage;
//	int totalPages;

};

POFOutput::POFOutput(PString &POFfilename)
{
	filename = POFfilename + PString(_T(".POF"));
	streamname.open(filename.getString(), std::ios_base::out);
	if (!streamname.is_open())
	{
		std::cerr << "Cannot create POF file " && POFfilename.getString() ;
		exit(-1);
	}
}

POFOutput::~POFOutput()
{
}

void POFOutput::startDoc()
{
	streamname << _T("<!DOCTYPE html>\n");
	streamname << _T("<html>\n<head>\n");
	streamname << _T("   <title>");
	streamname << title.getString();
	streamname << _T("</title>\n");
	streamname << _T("   <link rel = \"stylesheet\" type = \"text / css\" href = \"file:///C:/Users/Phil/Documents/Visual Studio 2013/StyleSheets/mystyle.css\"> \n");
	streamname << _T("</head>\n<body>\n");
}

void POFOutput::endDoc()
{
	streamname << _T("</body>\n</html>");
	streamname.close();
}

void POFOutput::addTable(Table &table)
{
	table.exportTable(streamname);
}

void POFOutput::addParagraph(PString &para, unsigned int crlf)
{
	streamname << _T("<p>");
	streamname << para.getString();
	streamname << _T("</p>");
	if (crlf)
		streamname << _T("\n");
}

void POFOutput::addHeading(PString &heading, unsigned int level)
{
	PString lvl = ConvertNumberToString(level, 0, NO_FORMAT);

	streamname << _T("<h") << lvl.getString() << _T(">");
	streamname << heading.getString();
	streamname << _T("</h") << lvl.getString() << _T(">\n");
}

void POFOutput::startNewPage()
{
	streamname << _T("</body>\n</html>\n");
	streamname << _T("! Insert Page Break here\n");
	this->startDoc();
}

void POFOutput::setTitle(PString &screenTitle)
{
	title = screenTitle;
	title.removeSpecialChar();
}

void POFOutput::setPlanName(PString &planName)
{
	planname = planName;
	planname.removeSpecialChar();
}

PString POFOutput::getPlanName() const
{
	return planname;
}

std::basic_fstream<TCHAR>& POFOutput::getStream() 
{
	return streamname;
}

void POFOutput::POFtoHTML()
{
	PString destination(filename);
	destination.dropRight(3);
	destination += PString(_T("html"));

	std::ifstream  src(filename.getString(), std::ios_base::in);
	std::ofstream  dst(destination.getString(), std::ios_base::out);

	dst << src.rdbuf();
}

POFHeader::POFHeader()
{
}

POFHeader::~POFHeader()
{
}

void POFHeader::setTopLeft(PString &tl)
{
	TopLeft = tl;
//	TopLeft.removeSpecialChar();
}

void POFHeader::setTopCenter(PString &tc)
{
	TopCenter = tc;
//	TopCenter.removeSpecialChar();
}

void POFHeader::setTopRight(PString &tr)
{
	TopRight = tr;
//	TopRight.removeSpecialChar();
}

void POFHeader::exportInfo(std::basic_fstream<TCHAR> &stream) const
{
	if (!stream)
		return;

	if (TopLeft.getLength() > 0)
	{
		stream << _T("<div class = \"topLeftHeader\"><span>");
		stream << TopLeft.getString();
		stream << _T("</span></div>\n");
	}

	if (TopCenter.getLength() > 0)
	{
		stream << _T("<div class = \"topCenterHeader\"><span>");
		stream << TopCenter.getString();
		stream << _T("</span></div>\n");
	}

	if (TopRight.getLength() > 0)
	{
		stream << _T("<div class = \"topRightHeader\"><span>");
		stream << TopRight.getString();
		stream << _T("</span></div>\n");
	}
}


#endif