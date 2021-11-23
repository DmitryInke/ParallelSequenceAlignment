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

		//printf("MASTER: %d\n", seq2ArrSize);
		masterSendDataToWorkers(seq1,numOfSequences,weights,numProc,seq2Arr,seq2ArrSize,workerArrSize);
		
		time = MPI_Wtime();

		// ### start cuda ###
		//calcScoreAlgorithmParallel(seq2Arr,seq1,weights,topScores,seq2ArrSize);
        // ### MAYBE openMP on score_array reduce MAX- find best scores and put them in best_scores
	
        int numJobs = seq2ArrSize;
        for (int wId = 1; wId < numProc; wId++)   
        {
			MPI_Recv(&workerArrSize,1,MPI_INT,wId,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(topScores + numJobs, workerArrSize, scoreType, wId, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			numJobs += workerArrSize;
        }   

		// handle remains
		for (; numJobs < numOfSequences; numJobs++)
			calcScoreAlgorithmSerial(seq1, seq2Arr[numJobs], weights,&topScores[numJobs]); 
                  
        printf("Time for parallel is %lf\n", MPI_Wtime()-time);
		printRes(topScores,numOfSequences);
               
      // ### need to zero best_scores probably before serial ###
        for (int i = 0; i < numOfSequences; i++)
        {
          topScores[i].n = 0; 
		  topScores[i].k = 0; 
		  topScores[i].scoreWeight = 0.0;
        }
     
        time = MPI_Wtime();     
        for(int i=0; i<numOfSequences; i++)
          calcScoreAlgorithmSerial(seq1, seq2Arr[i], weights,&topScores[i]); 
        printf("Time for serial is %lf\n", MPI_Wtime()-time);
		printRes(topScores,numOfSequences);

		freeMemory(seq2Arr, seq1, topScores, numOfSequences);
	}
	else
	{
		if(!workerReciveDataFromMaster(&seq1,&numOfSequences,weights,&workerArrSize,&seq2Arr,numProc,myRank,&topScores))
			MPI_Abort(MPI_COMM_WORLD,1);

		// printf("proc #%d, num_of_sequences: %d\n", myRank, numOfSequences);
		// printf("proc #%d, size seq: %zu, str:%s\n", myRank, strlen(seq1), seq1);
		// for (int i = 0; i < SYMBOLS_NUM; i++)
		// 	printf("proc #%d, weight %d: %d\n", myRank, i ,weights[i]);
		// printf("proc #%d, size %d\n", myRank, workerArrSize);
		// for (int i = 0; i < workerArrSize; i++)
		// 	printf("proc #%d, size seq: %zu, str:%s, n:%d, k:%d, score:%f\n", myRank, strlen(seq2Arr[i]), seq2Arr[i],topScores[i].n,topScores[i].k,topScores[i].scoreWeight);

		//printf("proc #%d, size %d\n", myRank, workerArrSize);

		calcScoreAlgorithmParallel(seq2Arr,seq1,weights,topScores,workerArrSize);
		//printRes(topScores,workerArrSize);
		MPI_Send(&workerArrSize,1,MPI_INT,MASTER,0,MPI_COMM_WORLD);
		MPI_Send(topScores,workerArrSize,scoreType,MASTER,0,MPI_COMM_WORLD);

		freeMemory(seq2Arr, seq1, topScores, workerArrSize);
	}

	MPI_Finalize();

	return 0;
	
}