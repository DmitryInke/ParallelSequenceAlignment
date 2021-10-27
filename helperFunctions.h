#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>
#include "myStructs.h"

MPI_Datatype weightMPIType();
MPI_Datatype scoreMPIType();

int readFromFile(char **mainSeq,char ***sequences,weight *weights,int *numOfSequences);
int writeToFile(score maxScore);
void sendMainSequence(weight* weight,char* mainSequence,int numProc,int tag);
void sendNumberOfSequence(char** sequences,int numOfSequences, int numProc, int tag);
void recieveScores(int numProc,int numOfSequences,int tag,score* maxScore,MPI_Status* status);
char* recieveMainSequence(weight* weight, int source, int tag, MPI_Status* status);
int recieveNumberOfSequence(int source, int tag, MPI_Status *status);
char* recieveSequence(int source, int tag, MPI_Status* status);
void sendScore(score* maxScore,int dest,int tag);
