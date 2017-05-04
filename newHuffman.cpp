#include <queue>
#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <pthread.h>
#include "bitChar.h"
#include "timer.h"
using namespace std;

const int asciiLength = 256;
int letterCount[asciiLength];
string codes[asciiLength];
long long originalSize = 0, compressedSize = 0;
int numThreads;
pthread_mutex_t mutex;
pthread_cond_t conditional;

struct node
{
	long long value;
	char letter;
	struct node *left, *right;
};

class compare
{
public:
	bool operator()(const node *left, const node *right) const
	{
		return left->value > right->value;
	}
};

typedef std::priority_queue<node*, vector<node*>, compare> nodeTree;

void buildTree(nodeTree &tree)
{
	while(tree.size() > 1)
	{
		node *current = new node;
		current->left = tree.top();
		tree.pop();
		current->right = tree.top();
		tree.pop();
		current->value = current->left->value + current->right->value;
		current->letter = -1;
		tree.push(current);
	}
	return;
}

// Travers the tree to create the codes. Add 0 for left and 1 for right
void createCode(node *tree)
{
	static string bitCode = "";
	
	if(tree->right != NULL)
	{
		bitCode += "1";
		createCode(tree->right);
		bitCode = bitCode.substr(0, bitCode.size() - 1);
	}
	
	if(tree->left != NULL)
	{
		bitCode += "0";
		createCode(tree->left);
		bitCode = bitCode.substr(0, bitCode.size() - 1);
	}
	
	if(!tree->left && !tree->right)
	{
		codes[tree->letter] = bitCode;
	}
	return;
}

void countLetters(string file, long long &count)
{
	char letter;
	ifstream inFile(file.c_str());
	
	inFile >> noskipws;
	
	for(int i = 0; i < asciiLength; i++)
	{
		letterCount[i] = 0;
	}
	while(inFile >> letter)
	{
		if(letter >= 0 && letter < asciiLength)
		{
			++letterCount[letter];
			++count;
		}
	}
	inFile.close();
	return;
}

void* compressFile (void *threadId)
{
	long id = (long) threadId;
	long long partition = originalSize / numThreads;
	long long start = id * partition;
	long long end;
	char input;
	ofstream outf;
	bitChar bChar;
	string bits = "";
	string inFile = "original.txt";

	char outFile[16];
	sprintf(outFile, "compressed-%i.mpc", (int) id);
	
	if(numThreads - 1 == id)
	{
		end = originalSize;
	}
	else
	{
		end = (id + 1) * partition;
	}
	
	ifstream inf(inFile.c_str());
	inf.seekg(start);
	inf >> noskipws;
	while(inf >> input && start <= end)
	{
		start++;
		bits += codes[input];
	}
	
	inf.close();

	bChar.setBits(bits);
	outf.open(outFile);
	outf << noskipws;

	pthread_mutex_lock(&mutex);
	compressedSize += bChar.insertBits(outf);
	pthread_mutex_unlock(&mutex);

	pthread_exit(NULL);
}

int main()
{
	int compress;
	long long i;
	unsigned char inChar;
	string inFile = "", outFile = "", bits = "", bitsSub = "";
	ofstream outf;
	ifstream inf;
	nodeTree tree;
	bitChar bChar;
	float memorySaved = 0;
	node *temp;
	double startTime, endTime, elapsedTime;
	
	long thread;
	pthread_mutex_init (&mutex, NULL);
    pthread_cond_init (&conditional, NULL);
	

	// Decide whether to compress or decompress it
	printf("Type 0 to decompress or 1 to compress >");
	scanf("%i", &compress);
	
	if( compress == 1)
	{
		printf("How many threads would you like to use? >");
		scanf("%i", &numThreads);
		pthread_t threads[numThreads];
		
		inFile = "original.txt";
		outFile = "compressed.mpc";
		
		outf.open(outFile.c_str());
		
		countLetters(inFile, originalSize);
		
		if(letterCount[3] == 0)
		{
			letterCount[3] = 1;
		}
		
		for(i = 0; i < asciiLength; i++)
		{
			outf << letterCount[i] << " ";
		}
		outf << endl;
		outf << '#';
		
		for (i = 0; i < asciiLength; i++)
		{
			if(letterCount[i] > 0)
			{
				temp = (node*) malloc(sizeof(node));
				temp->value = letterCount[i];
				temp->letter = i;
				temp->left = NULL;
				temp->right = NULL;
				
				tree.push(temp);
			}
		}
		
		printf("Building tree...\n");
		buildTree(tree);
		printf("Creating codes...\n");
		createCode(tree.top());
		
		printf("Starting compression timer...\n");
		GET_TIME(startTime);
		for(thread = 0; thread < numThreads; thread++)
		{
			pthread_create(&threads[(int)thread], NULL, compressFile, (void*) thread);
		}
		
		printf("Compressing...\n");
		
		for(thread = 0; thread < numThreads; thread++)
		{
			pthread_join(threads[(int)thread], NULL);
			pthread_detach(threads[thread]);
		}
		printf("Combining partitions...\n");
		system("./combine.sh");
		
		outf << codes[3];
		
		printf("Stopping compression timer...\n");
		GET_TIME(endTime);
		elapsedTime = endTime - startTime;
		
		memorySaved = 100 - ((float)compressedSize / ((float)originalSize * 8.0) * 100.0);
		printf("File successfully compressed!\n");
		printf("Saved %.2f%% of space\n", memorySaved);
		printf("Compression took %f seconds with %d threads\n", elapsedTime, numThreads);
	}
	else
	{
		inFile = "compressed.mpc";
		outFile = "decompressed.txt";
		inf.open(inFile.c_str());
		outf.open(outFile.c_str());
		
		for(i = 0; i < asciiLength; i++)
		{
			inf >> letterCount[i];
			if(letterCount[i] > 0)
			{
				temp = (node*) malloc(sizeof(node));
				temp->value = letterCount[i];
				temp->letter = i;
				temp->left = NULL;
				temp->right = NULL;
				
				tree.push(temp);
			}
		}
		
		printf("building tree...\n");
		buildTree(tree);
		printf("creating codes...\n");
		createCode(tree.top());
		
		while(inChar != '#')
		{
			inf >> inChar;
		}
		
		inf >> noskipws;
		
		while(inf >> inChar)
		{
			bits += bChar.getBits(inChar);
		}
		
		inf.close();
		
		for(i = 0; i < bits.length(); i++)
		{
			bitsSub += bits[i];
			for(int j = 0; j < asciiLength; j++)
			{
				if(bitsSub == codes[j])
				{
					if(j == 3)
					{
						outf << "\n";
						i = bits.length();
						break;
					}
					outf << (char)j;
					bitsSub = "";
					break;
				}
			}
		}
	}
	outf.close();
	return 0;
}