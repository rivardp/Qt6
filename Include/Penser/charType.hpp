// charType.h

// Determines the type of information contained in a PString



#ifndef CHARTYPE_H

#define CHARTYPE_H



#define CHARSET_ARRAY_SIZE 256



typedef struct 
{

	unsigned int value;		// The ANSI definition value (decimal)
	
	char *char_oct;			// The ANSI definition value (oct)

	char *char_hex;			// The ANSI definition value (hex)

	char *char_byte;		// The ANSI definition value (byte)

	char *character;		// The ANSI definition character

	char *HTML_code;		// The HTML code for the character

	char *HTML_name;		// The HTML name for the character

	char *description;		// The character description
	
	unsigned int lowerUpperMatch;	// The associated 'case' character, or same value if none exists

	unsigned int charType_defns;	// The bitwise characteristics defined below

} CHARSET_INFO;



// Definitions for character type function

#define UNDEFINED		0

#define ALPHA			1	// All alpha characters (hypens and spaces allowed)

#define UPPER			2	// All upper case alpha characters (hypens and spaces allowed)

#define LOWER			4	// All lower case alpha characters (hypens and spaces allowed)

#define PROPER			8	// First character is capitalized alpha, then lower case alpha (hypens and spaces allowed)

#define MIXED			16	// Combination of alpha characters not otherwise defined

#define ALPHANUMERIC		32	// Combination of alpha and numeric characters

#define NUMERICAL		64	// All numeric characters (includes numeric formats) - Can't use "NUMERIC" due to conflict with MySQL data type

#define SPACE			128	// Blank space

#define HYPHENATED		256	// Hyphen

#define NUMERIC_FORMAT		512	// Includes $, comma and decimal

#define COMMA			1024    // Comma - need to be consistent with NumToString.h

#define QUOTES			2048    // Single or double quote

#define DOUBLE_QUOTES   	4096    // Double quote

#define OPENING			8192	// Used with quotes and parentheses

#define CLOSING			16384	// Used with quotes and parentheses

#define PARENTHESES		32768	// Includes more than one "word" separated by a blank space

#define PERIOD			65536   // Period

#define PUNCTUATION     	131072  // Includes period, comma, colon, semi-colon, exclation mark and question mark

#define SLASH			262144  // Forward or back slash

#define OTHER			524288  // Character not otherwise defined




static const CHARSET_INFO ANSI[] =
{

	{ 0, "000", "00", "00000000", "NUL", "&#000;", NULL, "Null char", 0, UNDEFINED },

	{ 1, "001", "01", "00000001", "SOH", "&#001;", NULL, "Start of Heading", 1, OTHER },

	{ 2, "002", "02", "00000010", "STX", "&#002;", NULL, "Start of Text", 2, OTHER },

	{ 3, "003", "03", "00000011", "ETX", "&#003;", NULL, "End of Text", 3, OTHER },

	{ 4, "004", "04", "00000100", "EOT", "&#004;", NULL, "End of Transmission", 4, OTHER },

	{ 5, "005", "05", "00000101", "ENQ", "&#005;", NULL, "Enquiry", 5, OTHER },

	{ 6, "006", "06", "00000110", "ACK", "&#006;", NULL, "Acknowledgment", 6, OTHER },

	{ 7, "007", "07", "00000111", "BEL", "&#007;", NULL, "Bell", 7, OTHER },

	{ 8, "010", "08", "00001000", "BS", "&#008;", NULL, "Back Space", 8, OTHER },

	{ 9, "011", "09", "00001001", "HT", "&#009;", NULL, "Horizontal Tab", 9, OTHER },

	{ 10, "012", "0A", "00001010", "LF", "&#010;", NULL, "Line Feed", 10, OTHER },

	{ 11, "013", "0B", "00001011", "VT", "&#011;", NULL, "Vertical Tab", 11, OTHER },

	{ 12, "014", "0C", "00001100", "FF", "&#012;", NULL, "Form Feed", 12, OTHER },

	{ 13, "015", "0D", "00001101", "CR", "&#013;", NULL, "Carriage Return", 13, OTHER },

	{ 14, "016", "0E", "00001110", "SO", "&#014;", NULL, "Shift Out / X-On", 14, OTHER },

	{ 15, "017", "0F", "00001111", "SI", "&#015;", NULL, "Shift In / X-Off", 15, OTHER },

	{ 16, "020", "10", "00010000", "DLE", "&#016;", NULL, "Data Line Escape", 16, OTHER },

	{ 17, "021", "11", "00010001", "DC1", "&#017;", NULL, "Device Control 1 (oft. XON)", 17, OTHER },

	{ 18, "022", "12", "00010010", "DC2", "&#018;", NULL, "Device Control 2", 18, OTHER },

	{ 19, "023", "13", "00010011", "DC3", "&#019;", NULL, "Device Control 3 (oft. XOFF)", 19, OTHER },

	{ 20, "024", "14", "00010100", "DC4", "&#020;", NULL, "Device Control 4", 20, OTHER },

	{ 21, "025", "15", "00010101", "NAK", "&#021;", NULL, "Negative Acknowledgement", 21, OTHER },

	{ 22, "026", "16", "00010110", "SYN", "&#022;", NULL, "Synchronous Idle", 22, OTHER },

	{ 23, "027", "17", "00010111", "ETB", "&#023;", NULL, "End of Transmit Block", 23, OTHER },

	{ 24, "030", "18", "00011000", "CAN", "&#024;", NULL, "Cancel", 24, OTHER },

	{ 25, "031", "19", "00011001", "EM", "&#025;", NULL, "End of Medium", 25, OTHER },

	{ 26, "032", "1A", "00011010", "SUB", "&#026;", NULL, "Substitute", 26, OTHER },

	{ 27, "033", "1B", "00011011", "ESC", "&#027;", NULL, "Escape", 27, OTHER },

	{ 28, "034", "1C", "00011100", "FS", "&#028;", NULL, "File Separator", 28, OTHER },

	{ 29, "035", "1D", "00011101", "GS", "&#029;", NULL, "Group Separator", 29, OTHER },

	{ 30, "036", "1E", "00011110", "RS", "&#030;", NULL, "Record Separator", 30, OTHER },

	{ 31, "037", "1F", "00011111", "US", "&#031;", NULL, "Unit Separator", 31, OTHER },

	{ 32, "040", "20", "00100000", NULL, "&#32;", NULL, "Space", 32, ALPHA | SPACE },

	{ 33, "041", "21", "00100001", "!", "&#33;", NULL, "Exclamation mark", 33, PUNCTUATION | OTHER },

    	{ 34, "042", "22", "00100010", """", "&#34;", "&quot;", "Double quotes (or speech marks)", 34, QUOTES | DOUBLE_QUOTES | OPENING | CLOSING },

	{ 35, "043", "23", "00100011", "#", "&#35;", NULL, "Number", 35, OTHER },

	{ 36, "044", "24", "00100100", "$", "&#36;", NULL, "Dollar", 36, NUMERIC_FORMAT },

	{ 37, "045", "25", "00100101", "%", "&#37;", NULL, "Procenttecken", 37, NUMERIC_FORMAT },

	{ 38, "046", "26", "00100110", "&", "&#38;", "&amp;", "Ampersand", 38, OTHER },

	{ 39, "047", "27", "00100111", "'", "&#39;", NULL, "Single quote", 39, QUOTES | OPENING | CLOSING },

	{ 40, "050", "28", "00101000", "(", "&#40;", NULL, "Open parenthesis (or open bracket)", 41, NUMERIC_FORMAT | PARENTHESES | OPENING },

	{ 41, "051", "29", "00101001", ")", "&#41;", NULL, "Close parenthesis (or close bracket)", 40, NUMERIC_FORMAT | PARENTHESES | CLOSING },

	{ 42, "052", "2A", "00101010", "*", "&#42;", NULL, "Asterisk", 42, OTHER },

	{ 43, "053", "2B", "00101011", "+", "&#43;", NULL, "Plus", 45, OTHER },

	{ 44, "054", "2C", "00101100", ",", "&#44;", NULL, "Comma", 44, COMMA | NUMERIC_FORMAT | PUNCTUATION },

	{ 45, "055", "2D", "00101101", "-", "&#45;", NULL, "Hyphen", 43, NUMERIC_FORMAT | HYPHENATED },

	{ 46, "056", "2E", "00101110", ".", "&#46;", NULL, "Period, dot or full stop", 46, ALPHA | NUMERIC_FORMAT | PUNCTUATION },

	{ 47, "057", "2F", "00101111", "/", "&#47;", NULL, "Slash or divide", 47, SLASH | OTHER },

	{ 48, "060", "30", "00110000", "0", "&#48;", NULL, "Zero", 48, NUMERICAL | ALPHANUMERIC },

	{ 49, "061", "31", "00110001", "1", "&#49;", NULL, "One", 49, NUMERICAL | ALPHANUMERIC },

	{ 50, "062", "32", "00110010", "2", "&#50;", NULL, "Two", 50, NUMERICAL | ALPHANUMERIC },

	{ 51, "063", "33", "00110011", "3", "&#51;", NULL, "Three", 51, NUMERICAL | ALPHANUMERIC },

	{ 52, "064", "34", "00110100", "4", "&#52;", NULL, "Four", 52, NUMERICAL | ALPHANUMERIC },

	{ 53, "065", "35", "00110101", "5", "&#53;", NULL, "Five", 53, NUMERICAL | ALPHANUMERIC },

	{ 54, "066", "36", "00110110", "6", "&#54;", NULL, "Six", 54, NUMERICAL | ALPHANUMERIC },

	{ 55, "067", "37", "00110111", "7", "&#55;", NULL, "Seven", 55, NUMERICAL | ALPHANUMERIC },

	{ 56, "070", "38", "00111000", "8", "&#56;", NULL, "Eight", 56, NUMERICAL | ALPHANUMERIC },

	{ 57, "071", "39", "00111001", "9", "&#57;", NULL, "Nine", 57, NUMERICAL | ALPHANUMERIC },

	{ 58, "072", "3A", "00111010", ":", "&#58;", NULL, "Colon", 58, PUNCTUATION | OTHER },

	{ 59, "073", "3B", "00111011", ";", "&#59;", NULL, "Semicolon", 59, PUNCTUATION | OTHER },

	{ 60, "074", "3C", "00111100", "<", "&#60;", "&lt;", "Less than (or open angled bracket)", 62, PARENTHESES | OPENING },

	{ 61, "075", "3D", "00111101", "=", "&#61;", NULL, "Equals", 61, OTHER },

	{ 62, "076", "3E", "00111110", ">", "&#62;", "&gt;", "Greater than (or close angled bracket)", 60, PARENTHESES | CLOSING },

	{ 63, "077", "3F", "00111111", "?", "&#63;", NULL, "Question mark", 63, PUNCTUATION | OTHER },

	{ 64, "100", "40", "01000000", "@", "&#64;", NULL, "At symbol", 64, OTHER },

	{ 65, "101", "41", "01000001", "A", "&#65;", NULL, "Uppercase A", 97, ALPHA | UPPER | ALPHANUMERIC },

	{ 66, "102", "42", "01000010", "B", "&#66;", NULL, "Uppercase B", 98, ALPHA | UPPER | ALPHANUMERIC },

	{ 67, "103", "43", "01000011", "C", "&#67;", NULL, "Uppercase C", 99, ALPHA | UPPER | ALPHANUMERIC },

	{ 68, "104", "44", "01000100", "D", "&#68;", NULL, "Uppercase D", 100, ALPHA | UPPER | ALPHANUMERIC },

	{ 69, "105", "45", "01000101", "E", "&#69;", NULL, "Uppercase E", 101, ALPHA | UPPER | ALPHANUMERIC },

	{ 70, "106", "46", "01000110", "F", "&#70;", NULL, "Uppercase F", 102, ALPHA | UPPER | ALPHANUMERIC },

	{ 71, "107", "47", "01000111", "G", "&#71;", NULL, "Uppercase G", 103, ALPHA | UPPER | ALPHANUMERIC },

	{ 72, "110", "48", "01001000", "H", "&#72;", NULL, "Uppercase H", 104, ALPHA | UPPER | ALPHANUMERIC },

	{ 73, "111", "49", "01001001", "I", "&#73;", NULL, "Uppercase I", 105, ALPHA | UPPER | ALPHANUMERIC },

	{ 74, "112", "4A", "01001010", "J", "&#74;", NULL, "Uppercase J", 106, ALPHA | UPPER | ALPHANUMERIC },

	{ 75, "113", "4B", "01001011", "K", "&#75;", NULL, "Uppercase K", 107, ALPHA | UPPER | ALPHANUMERIC },

	{ 76, "114", "4C", "01001100", "L", "&#76;", NULL, "Uppercase L", 108, ALPHA | UPPER | ALPHANUMERIC },

	{ 77, "115", "4D", "01001101", "M", "&#77;", NULL, "Uppercase M", 109, ALPHA | UPPER | ALPHANUMERIC },

	{ 78, "116", "4E", "01001110", "N", "&#78;", NULL, "Uppercase N", 110, ALPHA | UPPER | ALPHANUMERIC },

	{ 79, "117", "4F", "01001111", "O", "&#79;", NULL, "Uppercase O", 111, ALPHA | UPPER | ALPHANUMERIC },

	{ 80, "120", "50", "01010000", "P", "&#80;", NULL, "Uppercase P", 112, ALPHA | UPPER | ALPHANUMERIC },

	{ 81, "121", "51", "01010001", "Q", "&#81;", NULL, "Uppercase Q", 113, ALPHA | UPPER | ALPHANUMERIC },

	{ 82, "122", "52", "01010010", "R", "&#82;", NULL, "Uppercase R", 114, ALPHA | UPPER | ALPHANUMERIC },

	{ 83, "123", "53", "01010011", "S", "&#83;", NULL, "Uppercase S", 115, ALPHA | UPPER | ALPHANUMERIC },

	{ 84, "124", "54", "01010100", "T", "&#84;", NULL, "Uppercase T", 116, ALPHA | UPPER | ALPHANUMERIC },

	{ 85, "125", "55", "01010101", "U", "&#85;", NULL, "Uppercase U", 117, ALPHA | UPPER | ALPHANUMERIC },

	{ 86, "126", "56", "01010110", "V", "&#86;", NULL, "Uppercase V", 118, ALPHA | UPPER | ALPHANUMERIC },

	{ 87, "127", "57", "01010111", "W", "&#87;", NULL, "Uppercase W", 119, ALPHA | UPPER | ALPHANUMERIC },

	{ 88, "130", "58", "01011000", "X", "&#88;", NULL, "Uppercase X", 120, ALPHA | UPPER | ALPHANUMERIC },

	{ 89, "131", "59", "01011001", "Y", "&#89;", NULL, "Uppercase Y", 121, ALPHA | UPPER | ALPHANUMERIC },

	{ 90, "132", "5A", "01011010", "Z", "&#90;", NULL, "Uppercase Z", 122, ALPHA | UPPER | ALPHANUMERIC },

	{ 91, "133", "5B", "01011011", "[", "&#91;", NULL, "Opening bracket", 93, PARENTHESES | OPENING },

	{ 92, "134", "5C", "01011100", "\\", "&#92;", NULL, "Backslash", 92, SLASH | OTHER },

	{ 93, "135", "5D", "01011101", "]", "&#93;", NULL, "Closing bracket", 91, PARENTHESES | CLOSING },

	{ 94, "136", "5E", "01011110", "^", "&#94;", NULL, "Caret - circumflex", 94, OTHER },

	{ 95, "137", "5F", "01011111", "_", "&#95;", NULL, "Underscore", 95, OTHER },

	{ 96, "140", "60", "01100000", "`", "&#96;", NULL, "Grave accent", 96, OTHER },

	{ 97, "141", "61", "01100001", "a", "&#97;", NULL, "Lowercase a", 65, ALPHA | LOWER | ALPHANUMERIC },

	{ 98, "142", "62", "01100010", "b", "&#98;", NULL, "Lowercase b", 66, ALPHA | LOWER | ALPHANUMERIC },

	{ 99, "143", "63", "01100011", "c", "&#99;", NULL, "Lowercase c", 67, ALPHA | LOWER | ALPHANUMERIC },

	{ 100, "144", "64", "01100100", "d", "&#100;", NULL, "Lowercase d", 68, ALPHA | LOWER | ALPHANUMERIC },

	{ 101, "145", "65", "01100101", "e", "&#101;", NULL, "Lowercase e", 69, ALPHA | LOWER | ALPHANUMERIC },

	{ 102, "146", "66", "01100110", "f", "&#102;", NULL, "Lowercase f", 70, ALPHA | LOWER | ALPHANUMERIC },

	{ 103, "147", "67", "01100111", "g", "&#103;", NULL, "Lowercase g", 71, ALPHA | LOWER | ALPHANUMERIC },

	{ 104, "150", "68", "01101000", "h", "&#104;", NULL, "Lowercase h", 72, ALPHA | LOWER | ALPHANUMERIC },

	{ 105, "151", "69", "01101001", "i", "&#105;", NULL, "Lowercase i", 73, ALPHA | LOWER | ALPHANUMERIC },

	{ 106, "152", "6A", "01101010", "j", "&#106;", NULL, "Lowercase j", 74, ALPHA | LOWER | ALPHANUMERIC },

	{ 107, "153", "6B", "01101011", "k", "&#107;", NULL, "Lowercase k", 75, ALPHA | LOWER | ALPHANUMERIC },

	{ 108, "154", "6C", "01101100", "l", "&#108;", NULL, "Lowercase l", 76, ALPHA | LOWER | ALPHANUMERIC },

	{ 109, "155", "6D", "01101101", "m", "&#109;", NULL, "Lowercase m", 77, ALPHA | LOWER | ALPHANUMERIC },

	{ 110, "156", "6E", "01101110", "n", "&#110;", NULL, "Lowercase n", 78, ALPHA | LOWER | ALPHANUMERIC },

	{ 111, "157", "6F", "01101111", "o", "&#111;", NULL, "Lowercase o", 79, ALPHA | LOWER | ALPHANUMERIC },

	{ 112, "160", "70", "01110000", "p", "&#112;", NULL, "Lowercase p", 80, ALPHA | LOWER | ALPHANUMERIC },

	{ 113, "161", "71", "01110001", "q", "&#113;", NULL, "Lowercase q", 81, ALPHA | LOWER | ALPHANUMERIC },

	{ 114, "162", "72", "01110010", "r", "&#114;", NULL, "Lowercase r", 82, ALPHA | LOWER | ALPHANUMERIC },

	{ 115, "163", "73", "01110011", "s", "&#115;", NULL, "Lowercase s", 83, ALPHA | LOWER | ALPHANUMERIC },

	{ 116, "164", "74", "01110100", "t", "&#116;", NULL, "Lowercase t", 84, ALPHA | LOWER | ALPHANUMERIC },

	{ 117, "165", "75", "01110101", "u", "&#117;", NULL, "Lowercase u", 85, ALPHA | LOWER | ALPHANUMERIC },

	{ 118, "166", "76", "01110110", "v", "&#118;", NULL, "Lowercase v", 86, ALPHA | LOWER | ALPHANUMERIC },

	{ 119, "167", "77", "01110111", "w", "&#119;", NULL, "Lowercase w", 87, ALPHA | LOWER | ALPHANUMERIC },

	{ 120, "170", "78", "01111000", "x", "&#120;", NULL, "Lowercase x", 88, ALPHA | LOWER | ALPHANUMERIC },

	{ 121, "171", "79", "01111001", "y", "&#121;", NULL, "Lowercase y", 89, ALPHA | LOWER | ALPHANUMERIC },

	{ 122, "172", "7A", "01111010", "z", "&#122;", NULL, "Lowercase z", 90, ALPHA | LOWER | ALPHANUMERIC },

	{ 123, "173", "7B", "01111011", "{", "&#123;", NULL, "Opening brace", 125, PARENTHESES | OPENING },

	{ 124, "174", "7C", "01111100", "|", "&#124;", NULL, "Vertical bar", 124, OTHER },

	{ 125, "175", "7D", "01111101", "}", "&#125;", NULL, "Closing brace", 123, PARENTHESES | CLOSING },

	{ 126, "176", "7E", "01111110", "~", "&#126;", NULL, "Equivalency sign - tilde", 126, OTHER },

	{ 127, "177", "7F", "01111111", NULL, "&#127;", NULL, "Delete", 127, OTHER },

	{ 128, "200", "80", "10000000", "€", "&#128;", "&euro;", "Euro sign", 128, OTHER },

	{ 129, "201", "81", "10000001", NULL, NULL, NULL, NULL, 129, OTHER },

	{ 130, "202", "82", "10000010", "‚", "&#130;", "&sbquo;", "Single low-9 quotation mark", 130, OTHER },

	{ 131, "203", "83", "10000011", "ƒ", "&#131;", "&fnof;", "Latin small letter f with hook", 131, OTHER },

	{ 132, "204", "84", "10000100", "„", "&#132;", "&bdquo;", "Double low-9 quotation mark", 132, OTHER },

	{ 133, "205", "85", "10000101", "…", "&#133;", "&hellip;", "Horizontal ellipsis", 133, OTHER },

	{ 134, "206", "86", "10000110", "†", "&#134;", "&dagger;", "Dagger", 134, OTHER },

	{ 135, "207", "87", "10000111", "‡", "&#135;", "&Dagger;", "Double dagger", 135, OTHER },

	{ 136, "210", "88", "10001000", "ˆ", "&#136;", "&circ;", "Modifier letter circumflex accent", 136, OTHER },

	{ 137, "211", "89", "10001001", "‰", "&#137;", "&permil;", "Per mille sign", 137, OTHER },

	{ 138, "212", "8A", "10001010", "Š", "&#138;", "&Scaron;", "Latin capital letter S with caron", 138, ALPHA | UPPER | ALPHANUMERIC },

	{ 139, "213", "8B", "10001011", "‹", "&#139;", "&lsaquo;", "Single left-pointing angle quotation", 155, QUOTES | OPENING },

	{ 140, "214", "8C", "10001100", "Œ", "&#140;", "&OElig;", "Latin capital ligature OE", 156, ALPHA | UPPER | ALPHANUMERIC },

	{ 141, "215", "8D", "10001101", NULL, NULL, NULL, NULL, 141, OTHER },

	{ 142, "216", "8E", "10001110", "Ž", "&#142;", NULL, "Latin captial letter Z with caron", 158, ALPHA | UPPER | ALPHANUMERIC },

	{ 143, "217", "8F", "10001111", NULL, NULL, NULL, NULL, 143, OTHER },

	{ 144, "220", "90", "10010000", NULL, NULL, NULL, NULL, 144, OTHER },

	{ 145, "221", "91", "10010001", "‘", "&#145;", "&lsquo;", "Left single quotation mark", 146, QUOTES | OPENING },

	{ 146, "222", "92", "10010010", "’", "&#146;", "&rsquo;", "Right single quotation mark", 145, QUOTES | CLOSING },

    	{ 147, "223", "93", "10010011", "“", "&#147;", "&ldquo;", "Left double quotation mark", 148, QUOTES | DOUBLE_QUOTES | OPENING },

    	{ 148, "224", "94", "10010100", "”", "&#148;", "&rdquo;", "Right double quotation mark", 147, QUOTES | DOUBLE_QUOTES | CLOSING },

	{ 149, "225", "95", "10010101", "•", "&#149;", "&bull;", "Bullet", 149, OTHER },

	{ 150, "226", "96", "10010110", "–", "&#150;", "&ndash;", "En dash", 150, NUMERIC_FORMAT | HYPHENATED },

	{ 151, "227", "97", "10010111", "—", "&#151;", "&mdash;", "Em dash", 151, OTHER },

	{ 152, "230", "98", "10011000", "˜", "&#152;", "&tilde;", "Small tilde", 152, OTHER },

	{ 153, "231", "99", "10011001", "™", "&#153;", "&trade;", "Trade mark sign", 153, OTHER },

	{ 154, "232", "9A", "10011010", "š", "&#154;", "&scaron;", "Latin small letter S with caron", 154, ALPHA | LOWER | ALPHANUMERIC },

	{ 155, "233", "9B", "10011011", "›", "&#155;", "&rsaquo;", "Single right-pointing angle quotation mark", 139, QUOTES | CLOSING },

	{ 156, "234", "9C", "10011100", "œ", "&#156;", "&oelig;", "Latin small ligature oe", 140, ALPHA | LOWER | ALPHANUMERIC },

	{ 157, "235", "9D", "10011101", NULL, NULL, NULL, NULL, 157, OTHER },

	{ 158, "236", "9E", "10011110", "ž", "&#158;", NULL, "Latin small letter z with caron", 142, ALPHA | LOWER | ALPHANUMERIC },

	{ 159, "237", "9F", "10011111", "Ÿ", "&#159;", "&Yuml;", "Latin capital letter Y with diaeresis", 255, ALPHA | UPPER | ALPHANUMERIC },

	{ 160, "240", "A0", "10100000", NULL, "&#160;", "&nbsp;", "Non-breaking space", 160, SPACE },

	{ 161, "241", "A1", "10100001", "¡", "&#161;", "&iexcl;", "Inverted exclamation mark", 161, OTHER },

	{ 162, "242", "A2", "10100010", "¢", "&#162;", "&cent;", "Cent sign", 162, NUMERIC_FORMAT },

	{ 163, "243", "A3", "10100011", "£", "&#163;", "&pound;", "Pound sign", 163, NUMERIC_FORMAT },

	{ 164, "244", "A4", "10100100", "¤", "&#164;", "&curren;", "Currency sign", 164, NUMERIC_FORMAT },

	{ 165, "245", "A5", "10100101", "¥", "&#165;", "&yen;", "Yen sign", 165, NUMERIC_FORMAT },

	{ 166, "246", "A6", "10100110", "¦", "&#166;", "&brvbar;", "Pipe, Broken vertical bar", 166, OTHER },

	{ 167, "247", "A7", "10100111", "§", "&#167;", "&sect;", "Section sign", 167, OTHER },

	{ 168, "250", "A8", "10101000", "¨", "&#168;", "&uml;", "Spacing diaeresis - umlaut", 168, OTHER },

	{ 169, "251", "A9", "10101001", "©", "&#169;", "&copy;", "Copyright sign", 169, OTHER },

	{ 170, "252", "AA", "10101010", "ª", "&#170;", "&ordf;", "Feminine ordinal indicator", 170, OTHER },

    	{ 171, "253", "AB", "10101011", "«", "&#171;", "&laquo;", "Left double angle quotes", 187, QUOTES | DOUBLE_QUOTES | OPENING},

	{ 172, "254", "AC", "10101100", "¬", "&#172;", "&not;", "Not sign", 172, OTHER },

	{ 173, "255", "AD", "10101101", "­", "&#173;", "&shy;", "Soft hyphen", 173, OTHER },

	{ 174, "256", "AE", "10101110", "®", "&#174;", "&reg;", "Registered trade mark sign", 174, OTHER },

	{ 175, "257", "AF", "10101111", "¯", "&#175;", "&macr;", "Spacing macron - overline", 175, OTHER },

	{ 176, "260", "B0", "10110000", "°", "&#176;", "&deg;", "Degree sign", 176, OTHER },

	{ 177, "261", "B1", "10110001", "±", "&#177;", "&plusmn;", "Plus-or-minus sign", 177, OTHER },

	{ 178, "262", "B2", "10110010", "²", "&#178;", "&sup2;", "Superscript two - squared", 178, OTHER },

	{ 179, "263", "B3", "10110011", "³", "&#179;", "&sup3;", "Superscript three - cubed", 179, OTHER },

	{ 180, "264", "B4", "10110100", "´", "&#180;", "&acute;", "Acute accent - spacing acute", 180, OTHER },

	{ 181, "265", "B5", "10110101", "µ", "&#181;", "&micro;", "Micro sign", 181, OTHER },

	{ 182, "266", "B6", "10110110", "¶", "&#182;", "&para;", "Pilcrow sign - paragraph sign", 182, OTHER },

	{ 183, "267", "B7", "10110111", "·", "&#183;", "&middot;", "Middle dot - Georgian comma", 183, OTHER },

	{ 184, "270", "B8", "10111000", "¸", "&#184;", "&cedil;", "Spacing cedilla", 184, OTHER },

	{ 185, "271", "B9", "10111001", "¹", "&#185;", "&sup1;", "Superscript one", 185, OTHER },

	{ 186, "272", "BA", "10111010", "º", "&#186;", "&ordm;", "Masculine ordinal indicator", 186, OTHER },

    	{ 187, "273", "BB", "10111011", "»", "&#187;", "&raquo;", "Right double angle quotes", 171, QUOTES | DOUBLE_QUOTES | CLOSING },

	{ 188, "274", "BC", "10111100", "¼", "&#188;", "&frac14;", "Fraction one quarter", 188, OTHER },

	{ 189, "275", "BD", "10111101", "½", "&#189;", "&frac12;", "Fraction one half", 189, OTHER },

	{ 190, "276", "BE", "10111110", "¾", "&#190;", "&frac34;", "Fraction three quarters", 190, OTHER },

	{ 191, "277", "BF", "10111111", "¿", "&#191;", "&iquest;", "Inverted question mark", 191, OTHER },

	{ 192, "300", "C0", "11000000", "À", "&#192;", "&Agrave;", "Latin capital letter A with grave", 224, ALPHA | UPPER | ALPHANUMERIC },

	{ 193, "301", "C1", "11000001", "Á", "&#193;", "&Aacute;", "Latin capital letter A with acute", 225, ALPHA | UPPER | ALPHANUMERIC },

	{ 194, "302", "C2", "11000010", "Â", "&#194;", "&Acirc;", "Latin capital letter A with circumflex", 226, ALPHA | UPPER | ALPHANUMERIC },

	{ 195, "303", "C3", "11000011", "Ã", "&#195;", "&Atilde;", "Latin capital letter A with tilde", 227, ALPHA | UPPER | ALPHANUMERIC },

	{ 196, "304", "C4", "11000100", "Ä", "&#196;", "&Auml;", "Latin capital letter A with diaeresis", 228, ALPHA | UPPER | ALPHANUMERIC },

	{ 197, "305", "C5", "11000101", "Å", "&#197;", "&Aring;", "Latin capital letter A with ring above", 229, ALPHA | UPPER | ALPHANUMERIC },

	{ 198, "306", "C6", "11000110", "Æ", "&#198;", "&AElig;", "Latin capital letter AE", 230, ALPHA | UPPER | ALPHANUMERIC },

	{ 199, "307", "C7", "11000111", "Ç", "&#199;", "&Ccedil;", "Latin capital letter C with cedilla", 231, ALPHA | UPPER | ALPHANUMERIC },

	{ 200, "310", "C8", "11001000", "È", "&#200;", "&Egrave;", "Latin capital letter E with grave", 232, ALPHA | UPPER | ALPHANUMERIC },

	{ 201, "311", "C9", "11001001", "É", "&#201;", "&Eacute;", "Latin capital letter E with acute", 233, ALPHA | UPPER | ALPHANUMERIC },

	{ 202, "312", "CA", "11001010", "Ê", "&#202;", "&Ecirc;", "Latin capital letter E with circumflex", 234, ALPHA | UPPER | ALPHANUMERIC },

	{ 203, "313", "CB", "11001011", "Ë", "&#203;", "&Euml;", "Latin capital letter E with diaeresis", 235, ALPHA | UPPER | ALPHANUMERIC },

	{ 204, "314", "CC", "11001100", "Ì", "&#204;", "&Igrave;", "Latin capital letter I with grave", 236, ALPHA | UPPER | ALPHANUMERIC },

	{ 205, "315", "CD", "11001101", "Í", "&#205;", "&Iacute;", "Latin capital letter I with acute", 237, ALPHA | UPPER | ALPHANUMERIC },

	{ 206, "316", "CE", "11001110", "Î", "&#206;", "&Icirc;", "Latin capital letter I with circumflex", 238, ALPHA | UPPER | ALPHANUMERIC },

	{ 207, "317", "CF", "11001111", "Ï", "&#207;", "&Iuml;", "Latin capital letter I with diaeresis", 239, ALPHA | UPPER | ALPHANUMERIC },

	{ 208, "320", "D0", "11010000", "Ð", "&#208;", "&ETH;", "Latin capital letter ETH", 240, ALPHA | UPPER | ALPHANUMERIC },

	{ 209, "321", "D1", "11010001", "Ñ", "&#209;", "&Ntilde;", "Latin capital letter N with tilde", 241, ALPHA | UPPER | ALPHANUMERIC },

	{ 210, "322", "D2", "11010010", "Ò", "&#210;", "&Ograve;", "Latin capital letter O with grave", 242, ALPHA | UPPER | ALPHANUMERIC },

	{ 211, "323", "D3", "11010011", "Ó", "&#211;", "&Oacute;", "Latin capital letter O with acute", 243, ALPHA | UPPER | ALPHANUMERIC },

	{ 212, "324", "D4", "11010100", "Ô", "&#212;", "&Ocirc;", "Latin capital letter O with circumflex", 244, ALPHA | UPPER | ALPHANUMERIC },

	{ 213, "325", "D5", "11010101", "Õ", "&#213;", "&Otilde;", "Latin capital letter O with tilde", 245, ALPHA | UPPER | ALPHANUMERIC },
	{ 214, "326", "D6", "11010110", "Ö", "&#214;", "&Ouml;", "Latin capital letter O with diaeresis", 246, ALPHA | UPPER | ALPHANUMERIC },

	{ 215, "327", "D7", "11010111", "×", "&#215;", "&times;", "Multiplication sign", 247, OTHER },

	{ 216, "330", "D8", "11011000", "Ø", "&#216;", "&Oslash;", "Latin capital letter O with slash", 248, ALPHA | UPPER | ALPHANUMERIC },

	{ 217, "331", "D9", "11011001", "Ù", "&#217;", "&Ugrave;", "Latin capital letter U with grave", 249, ALPHA | UPPER | ALPHANUMERIC },

	{ 218, "332", "DA", "11011010", "Ú", "&#218;", "&Uacute;", "Latin capital letter U with acute", 250, ALPHA | UPPER | ALPHANUMERIC },

	{ 219, "333", "DB", "11011011", "Û", "&#219;", "&Ucirc;", "Latin capital letter U with circumflex", 251, ALPHA | UPPER | ALPHANUMERIC },

	{ 220, "334", "DC", "11011100", "Ü", "&#220;", "&Uuml;", "Latin capital letter U with diaeresis", 252, ALPHA | UPPER | ALPHANUMERIC },

	{ 221, "335", "DD", "11011101", "Ý", "&#221;", "&Yacute;", "Latin capital letter Y with acute", 253, ALPHA | UPPER | ALPHANUMERIC },

	{ 222, "336", "DE", "11011110", "Þ", "&#222;", "&THORN;", "Latin capital letter THORN", 254, ALPHA | UPPER | ALPHANUMERIC },

	{ 223, "337", "DF", "11011111", "ß", "&#223;", "&szlig;", "Latin small letter sharp s - ess-zed", 223, ALPHA | LOWER | ALPHANUMERIC },

	{ 224, "340", "E0", "11100000", "à", "&#224;", "&agrave;", "Latin small letter a with grave", 192, ALPHA | LOWER | ALPHANUMERIC },

	{ 225, "341", "E1", "11100001", "á", "&#225;", "&aacute;", "Latin small letter a with acute", 193, ALPHA | LOWER | ALPHANUMERIC },

	{ 226, "342", "E2", "11100010", "â", "&#226;", "&acirc;", "Latin small letter a with circumflex", 194, ALPHA | LOWER | ALPHANUMERIC },

	{ 227, "343", "E3", "11100011", "ã", "&#227;", "&atilde;", "Latin small letter a with tilde", 195, ALPHA | LOWER | ALPHANUMERIC },

	{ 228, "344", "E4", "11100100", "ä", "&#228;", "&auml;", "Latin small letter a with diaeresis", 196, ALPHA | LOWER | ALPHANUMERIC },

	{ 229, "345", "E5", "11100101", "å", "&#229;", "&aring;", "Latin small letter a with ring above", 197, ALPHA | LOWER | ALPHANUMERIC },

	{ 230, "346", "E6", "11100110", "æ", "&#230;", "&aelig;", "Latin small letter ae", 198, ALPHA | LOWER | ALPHANUMERIC },

	{ 231, "347", "E7", "11100111", "ç", "&#231;", "&ccedil;", "Latin small letter c with cedilla", 199, ALPHA | LOWER | ALPHANUMERIC },

	{ 232, "350", "E8", "11101000", "è", "&#232;", "&egrave;", "Latin small letter e with grave", 200, ALPHA | LOWER | ALPHANUMERIC },

	{ 233, "351", "E9", "11101001", "é", "&#233;", "&eacute;", "Latin small letter e with acute", 201, ALPHA | LOWER | ALPHANUMERIC },

	{ 234, "352", "EA", "11101010", "ê", "&#234;", "&ecirc;", "Latin small letter e with circumflex", 202, ALPHA | LOWER | ALPHANUMERIC },

	{ 235, "353", "EB", "11101011", "ë", "&#235;", "&euml;", "Latin small letter e with diaeresis", 203, ALPHA | LOWER | ALPHANUMERIC },

	{ 236, "354", "EC", "11101100", "ì", "&#236;", "&igrave;", "Latin small letter i with grave", 204, ALPHA | LOWER | ALPHANUMERIC },

	{ 237, "355", "ED", "11101101", "í", "&#237;", "&iacute;", "Latin small letter i with acute", 205, ALPHA | LOWER | ALPHANUMERIC },

	{ 238, "356", "EE", "11101110", "î", "&#238;", "&icirc;", "Latin small letter i with circumflex", 206, ALPHA | LOWER | ALPHANUMERIC },

	{ 239, "357", "EF", "11101111", "ï", "&#239;", "&iuml;", "Latin small letter i with diaeresis", 207, ALPHA | LOWER | ALPHANUMERIC },

	{ 240, "360", "F0", "11110000", "ð", "&#240;", "&eth;", "Latin small letter eth", 208, ALPHA | LOWER | ALPHANUMERIC },

	{ 241, "361", "F1", "11110001", "ñ", "&#241;", "&ntilde;", "Latin small letter n with tilde", 209, ALPHA | LOWER | ALPHANUMERIC },

	{ 242, "362", "F2", "11110010", "ò", "&#242;", "&ograve;", "Latin small letter o with grave", 210, ALPHA | LOWER | ALPHANUMERIC },

	{ 243, "363", "F3", "11110011", "ó", "&#243;", "&oacute;", "Latin small letter o with acute", 211, ALPHA | LOWER | ALPHANUMERIC },

	{ 244, "364", "F4", "11110100", "ô", "&#244;", "&ocirc;", "Latin small letter o with circumflex", 212, ALPHA | LOWER | ALPHANUMERIC },

	{ 245, "365", "F5", "11110101", "õ", "&#245;", "&otilde;", "Latin small letter o with tilde", 213, ALPHA | LOWER | ALPHANUMERIC },

	{ 246, "366", "F6", "11110110", "ö", "&#246;", "&ouml;", "Latin small letter o with diaeresis", 214, ALPHA | LOWER | ALPHANUMERIC },

	{ 247, "367", "F7", "11110111", "÷", "&#247;", "&divide;", "Division sign", 215, OTHER },

	{ 248, "370", "F8", "11111000", "ø", "&#248;", "&oslash;", "Latin small letter o with slash", 216, ALPHA | LOWER | ALPHANUMERIC },

	{ 249, "371", "F9", "11111001", "ù", "&#249;", "&ugrave;", "Latin small letter u with grave", 217, ALPHA | LOWER | ALPHANUMERIC },

	{ 250, "372", "FA", "11111010", "ú", "&#250;", "&uacute;", "Latin small letter u with acute", 218, ALPHA | LOWER | ALPHANUMERIC },

	{ 251, "373", "FB", "11111011", "û", "&#251;", "&ucirc;", "Latin small letter u with circumflex", 219, ALPHA | LOWER | ALPHANUMERIC },

	{ 252, "374", "FC", "11111100", "ü", "&#252;", "&uuml;", "Latin small letter u with diaeresis", 220, ALPHA | LOWER | ALPHANUMERIC },

	{ 253, "375", "FD", "11111101", "ý", "&#253;", "&yacute;", "Latin small letter y with acute", 221, ALPHA | LOWER | ALPHANUMERIC },

	{ 254, "376", "FE", "11111110", "þ", "&#254;", "&thorn;", "Latin small letter thorn", 222, ALPHA | LOWER | ALPHANUMERIC },

	{ 255, "377", "FF", "11111111", "ÿ", "&#255;", "&yuml;", "Latin small letter y with diaeresis", 159, ALPHA | LOWER | ALPHANUMERIC }

};



#endif
 