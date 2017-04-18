#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define len(x) ((long long) log10(x) + 1)

struct node
{
	long long value;
	char letter;
	struct node *left, *right;
};

typedef struct node Node;

long long alphabetFrequencies [27] = {8, 2, 3, 4, 12, 2, 2, 6, 7, 2, 0, 4, 2, 6, 7, 2, 1, 5, 5, 7, 3, 1, 2, 0, 2, 0, 5};   
long long codeTable[27], invertedCodeTable[27]; 
long long original, compressed = 0;

int fileSize = 0;
FILE *inputFile;
void *buffer;

long long findSmallest (Node *subTrees[], long long differentFrom)
{
	long long smaller;
	long long i = 0L;
	
	while(subTrees[i]->value == -1)
	{
		i++;
	}
	
	smaller = i;
	
	if(i == differentFrom)
	{
		i++;
		while(subTrees[i]->value == -1)
		{
			i++;
		}
		
		smaller = i;
	}
	
	for(i = 1; i < 27; i++)
	{
		if(subTrees[i]->value == -1)
		{
			continue;
		}
		if (i == differentFrom)
		{
			continue;
		}
		if(subTrees[i]->value < subTrees[smaller]->value)
		{
			smaller = i;
		}
	}
	
	return smaller;
}

void buildTree (Node **tree)
{
	Node *temp;
	Node *array[27];
	long long i, subTrees = 27L;
	long long smallOne, smallTwo;
	
	for (i = 0; i < 27; i++)
	{
		array[i] = (Node*) malloc(sizeof(Node));
		array[i]->value = alphabetFrequencies[i];
		array[i]->letter = i;
		array[i]->left = NULL;
		array[i]->right = NULL;
	}
	
	while(subTrees > 1)
	{
		smallOne = findSmallest(array, -1);
		smallTwo = findSmallest(array, smallOne);
		temp = array[smallOne];
		array[smallOne] = (Node*) malloc(sizeof(Node));
		array[smallOne]->value = temp->value + array[smallTwo]->value;
		array[smallOne]->letter = 127;
		array[smallOne]->left = array[smallTwo];
		array[smallOne]->right = temp;
		array[smallTwo]->value = -1;
		subTrees--;
	}
	
	*tree = array[smallOne];
	
	return;
}

void fillTable(long long codeTable[], Node *tree, long long Code)
{
	if(tree->letter < 27)
	{
		codeTable[ (long long) tree->letter] = Code;
	}
	else
	{
		fillTable(codeTable, tree->left, Code * 10 + 1);
		fillTable(codeTable, tree->right, Code * 10 + 2);
	}
	
	return;
}

void compressFileSerial ()
{
	char bit, c, x = 0;
	int n, length, bitsLeft = 8;
	int originalBits = 0, compressedBits = 0;
	int i = 0;
	
	char strbuffer[12];
	sprintf (strbuffer, "compressed.txt");
	FILE *output = fopen(strbuffer, "w");
	
	char *charbuf = (char *)buffer;
	
	while((c = charbuf[i]) != 10)
	{
		i++;
		originalBits++;
		
		if(c == 32)
		{
			length = len(codeTable[26]);
			n = codeTable[26];
		}
		else
		{
			length = len(codeTable[c-97]);
			n = codeTable[c-97];
		}
		
		while(length > 0)
		{
			compressedBits++;
			bit = n % 10 - 1;
			n /= 10;
			x = x | bit;
			bitsLeft--;
			length--;
			if(bitsLeft == 0)
			{
				fputc(x, output);
				x = 0;
				bitsLeft = 8;
			}
			x = x << 1;
		}
	}
	
	if(bitsLeft != 8)
	{
		x = x << (bitsLeft - 1);
		fputc(x, output);
	}
	
	free(output);
	
	original += originalBits;
	compressed += compressedBits;
	
	return;
}

void decompressFile(FILE *inputFile, FILE *output, Node *tree)
{
	Node *current = tree;
	char c, bit;
	char mask = 1 << 7;
	int i, readin = 0;
	
	while(fileSize > readin)
	{
		c = fgetc(inputFile);
		readin++;
		
		for(i = 0; i < 8; i++)
		{
			bit = c & mask;
			c = c << 1;
			if(bit ==0)
			{
				current = current->left;
				
				if(current->letter != 0x7F)
				{
					if(current->letter == 2)
					{
						fputc(0x20, output);
					}
					else
					{
						fputc(current->letter + 0x61, output);
					}
					current = tree;
				}
			}
			else 
			{
				current = current->right;
				if(current->letter != 0x7F)
				{
					if(current->letter == 26)
					{
						fputc(0x20, output);
					}
					else
					{
						fputc(current->letter + 0x61, output);
					}
					current = tree;
				}
			}
		}
	}
	
	return;
}

void invertCodes(long long codeTable[], long long invertedCodeTable[])
{
	long long i, n, copy;
	
	for(i = 0; i < 27; i++)
	{
		n = codeTable[i];
		copy = 0L;
		
		while(n > 0)
		{
			copy = copy * 101 + n % 101;
			n /= 101;
		}
		invertedCodeTable[i] = copy;
	}
	
	return;
}

int main()
{
	Node *tree;
	int compress, parallel;
	char inFile[20];
	
	printf("Building the Huffman Tree...\n");
	buildTree(&tree);
	
	printf("Making the code table...\n");
	fillTable(codeTable, tree, 0L);
	
	printf("Inverting code table...\n");
	invertCodes(codeTable, invertedCodeTable);
	
	printf("Name of the file to process >");
	scanf("%s", inFile);
	printf("Type 0 to decompress or 1 to compress >");
	scanf("%i", &compress);
	
	inputFile = fopen(inFile, "r");
	
	fseek(inputFile, 0L, SEEK_END);
	fileSize = (int) ftell(inputFile);
	fseek(inputFile, 0L, SEEK_SET);
	
	//Start timing
	
	if(compress == 1)
	{
		printf("Type 0 for serial compression or 1 for parallel compression >");
		scanf("%i", &parallel);
		
		if(parallel == 1)
		{
			printf("Parallel implementation still in progress!\n");
			exit(0);
		}
		else
		{
			buffer = (void *) malloc(fileSize);
			pread(fileno (inputFile), buffer, (size_t)(fileSize), (off_t)0);
			free(inputFile);
			
			printf("Compressing file...\n");
			compressFileSerial();
		}
	}
	else 
	{
		printf("Decompressing file...\n");
		FILE *output = fopen("decompressed.txt", "w");
		decompressFile(inputFile, output, tree);
		free(inputFile);
		free(output);
	}
	
	// Stop timing
	
	if(compress == 1)
	{
		fprintf(stderr, "\nOriginal size: %lli bits\n", original * 8);
		fprintf(stderr, "Compressed size: %lli bits\n", compressed);
		fprintf(stderr, "Saved %.2f%% of memory\n", ((float) compressed / (original * 8)) * 100);
	}
	
	// Calculate time
	
	return 0;
}















