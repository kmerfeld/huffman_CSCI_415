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

// Used to store characters and their frequencies
const int asciiLength = 256;
int letterCount[asciiLength];
string codes[asciiLength];

// Used to tell how much memory compression saves
long long originalSize = 0, compressedSize = 0;

// Threading variables
int numThreads;
pthread_mutex_t mutex;
pthread_cond_t conditional;

// Structure used to make a tree
struct node
{
	long long value;
	char letter;
	struct node *left, *right;
};

// Compare class used to compare node values. Used for the priority queue
class compare
{
public:
	bool operator()(const node *left, const node *right) const
	{
		return left->value > right->value;
	}
};

// Priority queue used to make the tree
typedef std::priority_queue<node*, vector<node*>, compare> nodeTree;

// Builds the huffman tree by finding the two smallest nodes and summing their values 
// to create a parent node for the two cild nodes
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

// Traverse the tree to create the codes. Add 0 for left and 1 for right
void createCode(node *tree)
{
	static string bitCode = "";
	
	// Add a 1 to the code for nodes on the right
	if(tree->right != NULL)
	{
		bitCode += "1";
		createCode(tree->right);
		bitCode = bitCode.substr(0, bitCode.size() - 1);
	}
	
	// Add a 0 to the code for nodes on the left
	if(tree->left != NULL)
	{
		bitCode += "0";
		createCode(tree->left);
		bitCode = bitCode.substr(0, bitCode.size() - 1);
	}
	
	// Assign the code for the leaf's letter once we reach a leaf node
	if(!tree->left && !tree->right)
	{
		codes[tree->letter] = bitCode;
	}
	return;
}

// Gets the frequency of each character in the file
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

// Compresses the file in serial or parallel
void* compressFile (void *threadId)
{
	// Thread id 
	long id = (long) threadId;
	
	// Used to get the size of a partition and where to start and end the partition
	long long partition = originalSize / numThreads;
	long long start = id * partition;
	long long end;
	
	// Character and file reading variables
	char input;
	ofstream outf;
	bitChar bChar;
	string bits = "";
	string inFile = "original.txt";
	char outFile[16];
	sprintf(outFile, "compressed-%i.mpc", (int) id);
	
	// If its the last thread, the end of the file is always the end of the partition
	if(numThreads - 1 == id)
	{
		end = originalSize;
	}
	else // otherwise find the correct partition
	{
		end = (id + 1) * partition;
	}
	
	// Read in the original file
	ifstream inf(inFile.c_str());
	inf.seekg(start);
	inf >> noskipws;
	while(inf >> input && start <= end)
	{
		start++;
		bits += codes[input];
	}
	
	inf.close();

	// Prepare to write to the compressed file
	bChar.setBits(bits);
	outf.open(outFile);
	outf << noskipws;

	// Write to the compressed file and increase the bit count
	// Need mutual exclusion here to not overwrite compressedSize
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
	
	// If the user decides to compress the file
	if( compress == 1)
	{
		//initialize threading variables
		printf("How many threads would you like to use? >");
		scanf("%i", &numThreads);
		pthread_t threads[numThreads];
		
		// Set up input and output files
		inFile = "original.txt";
		outFile = "compressed.mpc";
		
		outf.open(outFile.c_str());
		
		// Get lettter frequencies
		countLetters(inFile, originalSize);
		
		// Make sure to add an EOT character
		if(letterCount[3] == 0)
		{
			letterCount[3] = 1;
		}
		
		// Output the frequency of each character at the top of the file to use when decoding the file
		for(i = 0; i < asciiLength; i++)
		{
			outf << letterCount[i] << " ";
		}
		outf << endl;
		outf << '#';
		
		// Make a node for each symbol in the file
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
		
		// Build the tree and make the code table
		printf("Building tree...\n");
		buildTree(tree);
		printf("Creating codes...\n");
		createCode(tree.top());
		
		printf("Starting compression timer...\n");
		GET_TIME(startTime);
		
		// Launch the threads to compress the file
		for(thread = 0; thread < numThreads; thread++)
		{
			pthread_create(&threads[(int)thread], NULL, compressFile, (void*) thread);
		}
		
		printf("Compressing...\n");
		
		// Wait for threads to finish
		for(thread = 0; thread < numThreads; thread++)
		{
			pthread_join(threads[(int)thread], NULL);
			pthread_detach(threads[thread]);
		}
		
		// Combine the individual partitions
		printf("Combining partitions...\n");
		system("./combine.sh");
		
		// Print an EOT character at the end of the file
		outf << codes[3];
		
		printf("Stopping compression timer...\n");
		GET_TIME(endTime);
		elapsedTime = endTime - startTime;
		
		// Get the amount of memory saved  and how much time compression took
		memorySaved = 100 - ((float)compressedSize / ((float)originalSize * 8.0) * 100.0);
		printf("File successfully compressed!\n");
		printf("Saved %.2f%% of space\n", memorySaved);
		printf("Compression took %f seconds with %d threads\n", elapsedTime, numThreads);
	}
	else // Otherwise decompress the file
	{
		inFile = "compressed.mpc";
		outFile = "decompressed.txt";
		inf.open(inFile.c_str());
		outf.open(outFile.c_str());
		
		// Create a node for each character
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
		
		// Build a tree and codes
		printf("building tree...\n");
		buildTree(tree);
		printf("creating codes...\n");
		createCode(tree.top());
		
		// Read the header of the file (frequencies of the characters)
		while(inChar != '#')
		{
			inf >> inChar;
		}
		
		// Convert the bits in the file to character 1's and 0's
		inf >> noskipws;
		while(inf >> inChar)
		{
			bits += bChar.getBits(inChar);
		}
		
		inf.close();
		
		// Convert the character 1's and 0's into actual characters after finding a matching code
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