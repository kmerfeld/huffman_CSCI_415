#include <sstream>
#include <fstream>
#include <string>
#include <stdlib.h>
using namespace std;

class bitChar
{
public:
	unsigned char *ch;
	int bitShiftCount;
	string BITS;
	
	bitChar();
	void setBits(string bitsToSet);
	long long insertBits(ofstream &outFile);
	string getBits(unsigned char character);
	void writeBits(ofstream &outFile);
	void freeCh();
};

bitChar::bitChar()
{
	bitShiftCount = 0;
	ch = (unsigned char*)calloc(1, sizeof(char));
}

void bitChar::setBits(string characters)
{
	BITS = characters;
	return;
}

long long bitChar::insertBits(ofstream &outFile)
{
	long long total = 0;
	while(BITS.length())
	{
		if(BITS[0] == '1')
		{
			*ch |= 1;
		}
		*ch <<= 1;
		++bitShiftCount;
		++total;
		
		BITS.erase(0, 1);
		
		if(bitShiftCount == 7)
		{
			writeBits(outFile);
			bitShiftCount = 0;
			free(ch);
			ch = (unsigned char*)calloc(1, sizeof(char));
		}
	}
	if(bitShiftCount > 0)
	{
		*ch <<= (8 - bitShiftCount);
		writeBits(outFile);
		free(ch);
		ch = (unsigned char*)calloc(1, sizeof(char));
	}
	
	return total;
}

string bitChar::getBits(unsigned char character)
{
	stringstream _itoa;
	
	int _size = sizeof(unsigned char) * 8;
	
	for(unsigned _s = 0; _s < _size - 1; ++_s)
	{
		_itoa << ((character >> (_size - 1 - _s)) & 1);
	}
	
	return _itoa.str();
}

void bitChar::writeBits(ofstream &outFile)
{
	outFile << *ch;
	return;
}

void bitChar::freeCh()
{
	if(ch)
	{
		free(ch);
	}
	return;
}