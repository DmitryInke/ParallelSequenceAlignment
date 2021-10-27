#include "cFunctions.h"

// Calculates score for each mutant by the weights specified in the file
float calcWeight(const char* str, const weight* weights)
{
	int numberOfDollars = 0,numberOfPercents = 0,numberOfHashes = 0,numberOfSpaces = 0;
	for(int i = 0 ; i < strlen(str) ; i++)
    {
		switch(str[i]){
		case '$':
			numberOfDollars++;
			break;
		case '%':
			numberOfPercents++;
			break;
		case '#':
			numberOfHashes++;
			break;
		case ' ':
			numberOfSpaces++;
			break;
		default:
			break;
		}
	}
	return ((weights->w1) * numberOfDollars) - ((weights->w2) * numberOfPercents) - ((weights->w3) * numberOfHashes) - ((weights->w4) * numberOfSpaces);
}

// Gets the max of all scores
score maxWeight(const score *scoreList,const int size)
{
	score maxScore = scoreList[0];
	for(int i = 1; i < size; i++)
    {
		if(maxScore.scoreWeight < scoreList[i].scoreWeight){
			maxScore.n = scoreList[i].n;
			maxScore.k = scoreList[i].k;
			maxScore.scoreWeight = scoreList[i].scoreWeight;
		}
	}
	return maxScore;
}

//activating OpenMP and Cuda
void calcScore(weight *weight, const char *mainSequence,const char *secSequence, score *maxScore) 
{
	int sizeOfOffset = strlen(mainSequence) - strlen(secSequence);
	int sizeOfArray = sizeOfOffset*strlen(secSequence);
	score* arrayScore = (score*)malloc(sizeof(score)*(sizeOfOffset*strlen(secSequence)));
	// using OpenMP for every offset 
	
	#pragma omp parallel for
	for (int offset = 0; offset < sizeOfOffset; offset++) 
    {
		for (int hyphenIdx = 1; hyphenIdx <= strlen(secSequence); hyphenIdx++) 
        {
			score *currentScore = (score*)malloc(sizeof(score));
			//cuda to create mutant by offset and hyphen location
			char *mutant = computeOnGPU(strlen(secSequence) + 1 + 1,hyphenIdx,offset,mainSequence,secSequence,omp_get_thread_num());
			float scoreOfMutant = calcWeight(mutant,weight);
			currentScore->n = offset;
			currentScore->k = hyphenIdx;
			currentScore->scoreWeight = scoreOfMutant;
			memcpy(&arrayScore[offset * strlen(secSequence) + hyphenIdx - 1], currentScore, sizeof(score));
			free(mutant);
			free(currentScore);
		}
	}

	*maxScore = maxWeight(arrayScore,sizeOfArray);
	free(arrayScore);
}