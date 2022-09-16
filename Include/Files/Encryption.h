// Encryption.h   Version 1.0
//						  1.1 - Make items private

/************************************************************/
/*                   ENCRYPTION KEY                         */
/************************************************************/
#define FIELD_BREAK 124	// This is the "|" character
#define CRLF 10         // This is the newline "\n" character

const unsigned int encrCharSet[95] =
	{ 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	  48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
	  65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
	  97, 98, 99,100,101,102,103,104,105,106,
	  75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
	 107,108,109,110,111,112,113,114,115,116,
	  85, 86, 87, 88, 89, 90, 43, 44, 45, 46, 
	 117,118,119,120,121,122, 47,123,124,125,
	  58, 59, 60, 61, 62, 63, 64, 91, 92, 93,
	  94, 95, 96,126, 32 };

// Character equivalents
//	! " # $ % & ' ( ) * 
//  0 1 2 3 4 5 6 7 8 9
//	A B C D E F G H I J
//  a b c d e f g h i j
//	K L M N O P Q R S T
//  k l m n o p q r s t
//  U V W X Y Z + , - .
//  u v w x y z / { | }
//  : ; < = > ? @ [ \ ]
//  ^ _ ` ~

const unsigned int charPositions[95] = 
{ 91, 92, 30, 31, 32, 33, 34, 35, 36, 37,
  38, 39, 50, 51, 52, 53, 54, 55, 56, 57,
  58, 59, 70, 71, 72, 73, 74, 75, 77, 78,
  79, 93, 94,  0,  1,  2,  3,  4,  5,  6,
   7,  8,  9, 66, 67, 68, 69, 76, 10, 11,
  12, 13, 14, 15, 16, 17, 18, 19, 80, 81,
  82, 83, 84, 85, 86, 20, 21, 22, 23, 24,
  25, 26, 27, 28, 29, 40, 41, 42, 43, 44,
  45, 46, 47, 48, 49, 60, 61, 62, 63, 64,
  65, 87, 88, 89, 90 };
     
class ENCRYPTION
{
public:
	ENCRYPTION();
	~ENCRYPTION();

	bool IsEncrypted;

	// Methods
	int decode(const int);
	int encode(int);
	void setKey(const vector<unsigned int>  &newKey);

private:
	vector<unsigned int> key;
	unsigned int index;
	unsigned int lastPos;

	void setIndex(const unsigned int);
	void advIndex();
};

ENCRYPTION::ENCRYPTION()
{
	key = 0;
	IsEncrypted = false;
	index = 0;
	lastPos = 0;
}

ENCRYPTION::~ENCRYPTION()
{
}

int ENCRYPTION::decode(const int ch)
{
	if(!IsEncrypted)
		return ch;

	// Is Encypted
	unsigned int result;
	unsigned int priorLastPos = lastPos;
	if (ch >= 95)
		lastPos = charPositions[ch - 95];
	else
		lastPos = charPositions[ch];
	unsigned int priorSum = lastPos;
	while (priorSum  < (priorLastPos + key[index] + 32))
		priorSum += 95;
	result = priorSum - (priorLastPos + key[index]);
	advIndex();

	if (result == 126)  // Substitute ~ with CRLF
		return CRLF;
	else
		return result;
}

int ENCRYPTION::encode(int ch)
{
	if(!IsEncrypted)
		return ch;
	// Is Encrypted
	if (ch == CRLF)   // Substitute CRLF with ~
		ch = 126;
	lastPos = (lastPos + ch + key[index]) % 95;
	advIndex();

	return encrCharSet[lastPos];
}

void ENCRYPTION::setIndex(const unsigned int i)
{
	index = i%5;
}

void ENCRYPTION::advIndex()
{
	index = (index+1) % 5;
}

void ENCRYPTION::setKey(const vector<unsigned int> &newKey)
{
	if (newKey.getSize() != 5)
		return;
	key = newKey;
	return;
}
