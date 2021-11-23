#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cFunctions.h"
#include <omp.h>
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

void calcScoreAlgorithmParallel(char** seq2Arr,char* seq1,int* weights,Score* topScore,int workerArrSize)
{
    float maxScore;
    
    #pragma omp parallel for private(maxScore)
    for (int seq2index = 0; seq2index < workerArrSize; seq2index++)
    {
        //printf("my thread id %d, current index i %d, my string %s\n", omp_get_thread_num(), i, work_arr[i]);
        for (int k = 1; k <= strlen(seq2Arr[seq2index]); k++)
        {
            for (int n = 0; n < strlen(seq1) - strlen(seq2Arr[seq2index]); n++)
            {
                for (int i = 0; i <  strlen(seq2Arr[seq2index]) ; i++)
                {
                if(i<k) 
                    topScore[seq2index].scoreWeight+= weights[SYMBOLS_FIT_MAT[seq1[i+n] - 'A'][seq2Arr[seq2index][i] - 'A']];
                else
                    topScore[seq2index].scoreWeight+= weights[SYMBOLS_FIT_MAT[seq1[i+n+1] - 'A'][seq2Arr[seq2index][i] - 'A']];                              
                }

                if (n == 0 && k == 1)
                    maxScore = topScore[seq2index].scoreWeight;
                
                if (maxScore <= topScore[seq2index].scoreWeight)
                {
                    topScore[seq2index].n = n;
                    topScore[seq2index].k = k;
                    maxScore = topScore[seq2index].scoreWeight;
                }  

                topScore[seq2index].scoreWeight = 0;             
            }      
        }
        topScore[seq2index].scoreWeight = maxScore;      
    }  
}

void calcScoreAlgorithmSerial(char* seq1,char* seq2,int* weights,Score* topScore)
{
    float maxScore;
    
    for (int k = 1; k <= strlen(seq2); k++)
    {
        for (int n = 0; n < strlen(seq1) - strlen(seq2); n++)
        {
            for (int i = 0; i <  strlen(seq2) ; i++)
            {
               if(i<k) 
                   topScore->scoreWeight+= weights[SYMBOLS_FIT_MAT[seq1[i+n] - 'A'][seq2[i] - 'A']];
               else
                   topScore->scoreWeight+= weights[SYMBOLS_FIT_MAT[seq1[i+n+1] - 'A'][seq2[i] - 'A']];                              
            }

            if (n == 0 && k == 1)
                maxScore = topScore->scoreWeight;
            
            if (maxScore <= topScore->scoreWeight)
            {
                topScore->n = n;
                topScore->k = k;
                maxScore = topScore->scoreWeight;
            }  

            topScore->scoreWeight = 0;             
        }      
    }
    topScore->scoreWeight = maxScore;
}

void printRes(Score* topScore,int workerArrSize)
{
    for(int i = 0; i < workerArrSize; i++)
        printf("N:%d K:%d Score:%0.2f\n", topScore[i].n, topScore[i].k,topScore[i].scoreWeight);
}

void freeMemory(char** seq2Arr, char* seq1, Score* topScores, int size)
{
    for (int i = 0; i < size; i++)
    {
        free(seq2Arr[i]);
    }

    free(seq2Arr);
    free(seq1);
    free(topScores);

}


