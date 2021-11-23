#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cFunctions.h"
#include <ctype.h>


int readFromFile(char*** seq2arr,char** seq1,int* numOfSequences,int* weights,Score** topScore)
{
    char tempString[SEQ1_MAX_LENGTH];

    for(int i=0;i<SYMBOLS_NUM;i++)
    {
        scanf("%d",&weights[i]);
        if(i!= 0)
            weights[i] *=-1;
    }
    scanf("%s",tempString);
    *seq1 = (char*)malloc(strlen(tempString)*sizeof(char)+1);
    if(!*seq1)
    {
        fprintf(stderr,"Allocation error\n");
        return 0;
    }
    strcpy(*seq1,tempString);
    makeUpperStr(*seq1);

    scanf("%d",numOfSequences);

    *seq2arr = (char**)malloc(sizeof(char*)* (*numOfSequences));
    if(!*seq2arr)
    {
        fprintf(stderr,"Allocation error\n");
        return 0;
    }

    for(int i = 0; i < *numOfSequences; i++)
    {
        scanf("%s",tempString);
        (*seq2arr)[i] = (char*)malloc(strlen(tempString)*sizeof(char)+1);
        if(!(*seq2arr)[i])
        {
            fprintf(stderr,"Allocation error\n");
            return 0;
        }
        strcpy((*seq2arr)[i],tempString);
        makeUpperStr((*seq2arr)[i]);
    }

    if(!initScore(topScore, *numOfSequences))
        return 0;

    return 1;
}

void makeUpperStr(char* str)
{
    for(int i=0;i<strlen(str);i++)
        str[i] = toupper(str[i]);
}

int initScore(Score** topScore,int size)
{
    *topScore = (Score*)malloc(sizeof(Score)*size);
    if(!*topScore)
    {
        fprintf(stderr,"Allocation error\n");
        return 0;
    }

    for(int i = 0; i < size; i++)
    {
        (*topScore)[i].n = 0;
        (*topScore)[i].k = 0; 
        (*topScore)[i].scoreWeight = 0.0f;
    }
    return 1;
}

void calcSeq2Size(int* seq2ArrSize, int numProc,int myRank,int numOfSequences)
{
    if(numOfSequences >= numProc)
        *seq2ArrSize = numOfSequences/numProc;
    else if(myRank < numOfSequences)
        *seq2ArrSize = 1;
    else
        *seq2ArrSize = 0;

}