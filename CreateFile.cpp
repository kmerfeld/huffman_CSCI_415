#include <stdio.h>
#include <stdlib.h>

int main() 
{
	int i, j;
	float filesize = 0.0;
	char output[20];
	
	// Used for determining file length
	long bytesPerGb = 1073741824;
	long characters = 0;
	
	// Used to populate the file with a accurate distribution of letters
	char alphabet[] = "abcdefghijklmnopqrstuvwxyz ";
	int alphabetPercentage [27] = {8, 2, 3, 4, 12, 2, 2, 6, 7, 2, 0, 4, 2, 6, 7, 2, 1, 5, 5, 7, 3, 1, 2, 0, 2, 0, 5};
	int* alphabetFrequency = (int *) malloc(sizeof(int) * 100);
	int letterStart = 0;
	int letterEnd = 0;
	
	// Get filesize and name of the file to write to
	printf ("Enter the size of the file in Gb > ");
	scanf  ("%f", &filesize);

	// Get the number of characters to write to the file
	characters = filesize * bytesPerGb;
	
	// Open the file 
	FILE *file = fopen("original.txt", "w");
	
	// Check for problems opening the file
	if(file == NULL)
	{
		printf("Error opening file\n");
		exit(1);
	}
	
	// Fills alphabetFrequency with references to the indexes of letters of the alphabet
	//according to their percentages established in alphabetPercentage
	for(i = 0; i < 27; i++)
	{
		// Increment letterEnd to the percentage of the current letter
		letterEnd += alphabetPercentage[i];
		
		// Fill alphabetFrequency from the range letterStart-letterEnd with the 
		//index of the letter at index i
		for (j = letterStart; j <= letterEnd; j++)
		{
			alphabetFrequency[j] = i;			
		}
		
		// Increment letterStart to the index of the next letter
		letterStart = letterEnd + 1;
	}
	
	// Write random characters at their frequencies to the file
	for(i = 0; i < characters; i++) 
	{
		fprintf(file, "%c", alphabet[alphabetFrequency[rand() % 100]]);
	}
	
	fclose(file);
	
	printf("File Created Successfully\n");
}