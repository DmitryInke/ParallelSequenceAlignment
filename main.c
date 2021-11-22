#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <mpi.h>
#include "myProto.h"

int main(int argc, char *argv[]) 
{
	Score* bestScores;
	Score** scoreArr;
	char** workArr;
	char* seq1;
	double time;
	int* workArrSizes, *scoreArrSizes;
	int numOfSeq, workArrSize;
	int myRank, numProc;
	int weights[SYMBOLS_NUM] = {0};
	int symbolCounter[SYMBOLS_NUM] = {0};

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
	MPI_Comm_size(MPI_COMM_WORLD,&numProc);
}
