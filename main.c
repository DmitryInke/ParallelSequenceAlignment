#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <mpi.h>
#include "myStructs.h"
#include "cFunctions.h"
#include "helperFunctions.h"

int main(int argc, char *argv[]) {
	int my_rank; // rank of process 
	int proc; // number of processes 
	int tag = 0; // tag for messages 
	MPI_Status status; // return status for receive 

	char **sequences;
	char *mainSeq, *checkSeq;
	score score = {0};
	weight weights = {0};
	int numOfSequence;
	
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	MPI_Comm_size(MPI_COMM_WORLD, &proc);
	
	//check if we have two processes running
	if(proc < 2){
		printf("You need to run 2 processes\n");
		MPI_Abort(MPI_COMM_WORLD,1);
	}
	// The master sends the slaves everything he has read from the file and waits for an answer from them
	if (my_rank == MASTER) {
		if (!readFromFile(&mainSeq, &sequences, &weights, &numOfSequence))
			MPI_Abort(MPI_COMM_WORLD, 1);
		else {
			sendMainSequence(&weights, mainSeq, proc, tag);
			sendNumberOfSequence(sequences, numOfSequence, proc, tag);
			recieveScores(proc,numOfSequence,tag,&score,&status);
		}
	} else {
		mainSeq = recieveMainSequence(&weights, MASTER, tag, &status);
		numOfSequence = recieveNumberOfSequence(MASTER, tag, &status);

		for (int i = 0; i < numOfSequence; i++) 
		{
			checkSeq = recieveSequence(0, tag, &status);
			calcScore(&weights, mainSeq, checkSeq, &score);
			printf("The best score is: offset = %d, hyphen Index = %d, score = %f\n",score.n,score.k,score.scoreWeight);
			sendScore(&score,MASTER,tag);
		}
	}
	
	MPI_Finalize();
	return 0;
}
