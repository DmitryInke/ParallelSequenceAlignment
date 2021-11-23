#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <mpi.h>
#include "cFunctions.h"
#include "mpiHelper.h"

int main(int argc, char *argv[]) 
{
	char** seq2Arr; // sequence 2 string
	int seq2ArrSize; // Length of sequence 2 string for each process
	int workerArrSize; // Length of worker array size
	char* seq1; // sequence 1 string
	int numOfSequences; // number of sequences from file
	int weights[SYMBOLS_NUM]; // array of weights
	Score* topScores; // The best score for every sequence 2 string
	double time; // calculate serial and parallel times
	int myRank, numProc;


	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
	MPI_Comm_size(MPI_COMM_WORLD,&numProc);

	MPI_Datatype scoreType;
	createScoreType(&scoreType);

	if(myRank == MASTER)
	{
		if(!readFromFile(&seq2Arr,&seq1,&numOfSequences,weights,&topScores))
			MPI_Abort(MPI_COMM_WORLD,1);

		calcSeq2Size(&seq2ArrSize,numProc,myRank,numOfSequences);

		masterSendDataToWorkers(seq1,numOfSequences,weights,numProc,seq2Arr,seq2ArrSize,workerArrSize);
		
	}
	else
	{
		if(!workerReciveDataFromMaster(&seq1,&numOfSequences,weights,&workerArrSize,&seq2Arr,numProc,myRank,&topScores))
			MPI_Abort(MPI_COMM_WORLD,1);

		printf("proc #%d, num_of_sequences: %d\n", myRank, numOfSequences);
		printf("proc #%d, size seq: %zu, str:%s\n", myRank, strlen(seq1), seq1);
		for (int i = 0; i < SYMBOLS_NUM; i++)
			printf("proc #%d, weight %d: %d\n", myRank, i ,weights[i]);
		printf("proc #%d, size %d\n", myRank, workerArrSize);
		for (int i = 0; i < workerArrSize; i++)
			printf("proc #%d, size seq: %zu, str:%s, n:%d, k:%d, score:%f\n", myRank, strlen(seq2Arr[i]), seq2Arr[i],topScores[i].n,topScores[i].k,topScores[i].scoreWeight);
			
	}

	

	MPI_Finalize();
	return 0;
	
}
