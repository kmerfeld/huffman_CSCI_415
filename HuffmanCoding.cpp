#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>


struct node
{
	long long value;
	char letter;
	struct node *left, *right;
};

typedef struct node Node;

long long alphabetFrequencies [27] = {8, 2, 3, 4, 12, 2, 2, 6, 7, 2, 0, 4, 2, 6, 7, 2, 1, 5, 5, 7, 3, 1, 2, 0, 2, 0, 5};   
long long codeTable[27], codeTable2[27]; 
long long original = 0, compressed = 0;

int fileSize;
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
		while(subTrees[i]-> == -1)
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
	
	return smaller
}

void buildTree (Node **tree)
{
	Node *temp;
	Node *array[27];
	long long i, subTrees = 27L;
	long long smallOne, smallTwo;
	
	for (i = 0; i < 27; i++)
	{
		array[i] = malloc(sizeof(Node));
		array[i]->value = alphabetFrequencies[i];
		array[i]->letter = i;
		array[i]->left = NULL;
		array[i]->right = NULL
	}
	
	while(subTrees > 1)
	{
		smallOne = findSmallest(array, -1);
		smallTwo = findSmallest(array, smallOne);
		temp = array[smallOne];
		array[smallOne] = malloc(sizeof(Node));
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