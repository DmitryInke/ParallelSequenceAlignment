#include "helperFunctions.h"

MPI_Datatype weightMPIType()
{
	weight weights;
    MPI_Datatype weightMPIType;
    MPI_Datatype type[4] = { MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT };
    int blocklen[4] = { 1, 1, 1, 1 };
    MPI_Aint disp[4];

    disp[0] = (char *) &weights.w1 -	(char *) &weights;
    disp[1] = (char *) &weights.w2 -	(char *) &weights;
    disp[2] = (char *) &weights.w3 -	(char *) &weights;
    disp[3] = (char *) &weights.w4 -	(char *) &weights;
	
    MPI_Type_create_struct(4, blocklen, disp, type, &weightMPIType);
    MPI_Type_commit(&weightMPIType);
    return weightMPIType;
}

MPI_Datatype scoreMPIType()
{
	score maxScore;
    MPI_Datatype ScoreMPIType;
    MPI_Datatype type[3] = { MPI_INT, MPI_INT, MPI_FLOAT };
    int blocklen[3] = { 1, 1, 1 };
    MPI_Aint disp[3];

    disp[0] = (char *) &maxScore.n -	(char *) &maxScore;
    disp[1] = (char *) &maxScore.k -	(char *) &maxScore;
    disp[2] = (char *) &maxScore.scoreWeight -	(char *) &maxScore;

    MPI_Type_create_struct(3, blocklen, disp, type, &ScoreMPIType);
    MPI_Type_commit(&ScoreMPIType);
    return ScoreMPIType;
}

int readFromFile(char **mainSeq, char ***sequences, weight *weights, int *numOfSequences)
{
	char charBuffer[BUFFER_SIZE] = {0};
	FILE *file = fopen(FILE_NAME, "r");
	if (!file)
    {
		fclose(file);
		return 0;
	}
	
	// Read the weights from the file
	fscanf(file, "%f%f%f%f", &weights->w1, &weights->w2, &weights->w3,&weights->w4);
	
	// Read the main sequence from the file
	fscanf(file,"%s",charBuffer);
	*mainSeq = (char*)malloc(strlen(charBuffer) + 1);
	strcpy(*mainSeq, charBuffer);
	
	// Read the number of sequences for memory allocation.
	fscanf(file, "%d", numOfSequences);
	
	// Allocate memory for sequence array
	*sequences = (char**) calloc(*numOfSequences,sizeof(char*));
	
	// Read all sequences from the files
	for (int i = 0; i < *numOfSequences; i++) 
    {
		fscanf(file,"%s",charBuffer);
		(*sequences)[i] = (char*) calloc(strlen(charBuffer),sizeof(char));
		strcpy((*sequences)[i], charBuffer);
	}
	fclose(file);
	return 1;
}


int writeToFile(score* maxScore)
{
	FILE *file = fopen(OUTPUT_FILE, "a");
	if (!file){
		fclose(file);
		return 0;
	}
	
	// Writing the score after getting it from the slave
	fprintf(file,"The best score is: offset = %d, hyphen Index = %d, score = %f\n",maxScore->n,maxScore->k,maxScore->scoreWeight);
	fclose(file);
	return 1;
}

// The Master sends to each process the weight and the main sequence to work with
void sendMainSequence(weight* weight,char* mainSequence,int numProc,int tag)
{
	int sequenceLength = strlen(mainSequence);
	for(int i = 1 ; i < numProc; i++)
    {
		MPI_Send(weight,1,weightMPIType(),i,tag,MPI_COMM_WORLD);
		MPI_Send(&sequenceLength,1,MPI_INT,i,tag,MPI_COMM_WORLD);
		MPI_Send(mainSequence,sequenceLength,MPI_CHAR,i,tag,MPI_COMM_WORLD);
	}
}

// The division gives each process number of string which he need to check for score
void sendNumberOfSequence(char** sequences,int numOfSequences, int numProc, int tag)
{
	int seqSize;
	for(int i = 1,seqNum = 0; i < numProc; i++)
    { 
		int tasks = (numOfSequences / (numProc-1)) + ((i <= (numOfSequences % (numProc-1)))?1:0); 
		MPI_Send(&tasks,1,MPI_INT,i,tag,MPI_COMM_WORLD);
		for(int j = 0; j < tasks; j++,seqNum++)
        {
			seqSize = strlen(sequences[seqNum]);
			MPI_Send(&seqSize,1,MPI_INT,i,tag,MPI_COMM_WORLD);
			MPI_Send(sequences[seqNum], strlen(sequences[seqNum]),MPI_CHAR,i,tag,MPI_COMM_WORLD);
		}
	}
}

// The Master gets all scores for the slaves and write them to the output file
void recieveScores(int numProc,int numOfSequences,int tag,score* maxScore,MPI_Status* status)
{
	for (int i = 1; i < numProc; i++)
    {
		int tasks = (numOfSequences / (numProc-1)) +((i <= (numOfSequences % (numProc-1)))?1:0); 
		for(int j = 0; j < tasks; j++)
        {
			MPI_Recv(maxScore,1,scoreMPIType(),i,tag,MPI_COMM_WORLD,status);
			writeToFile(maxScore);
		}
	}
}

// The Master sends each process the main strings and weights to calc socres
char* recieveMainSequence(weight* weight, int source, int tag, MPI_Status* status)
{
	int sequenceLength;
	char* sequence;
	MPI_Recv(weight,1, weightMPIType(), source, tag,MPI_COMM_WORLD, status);
	MPI_Recv(&sequenceLength,1,MPI_INT,source,tag,MPI_COMM_WORLD,status);
	sequence = (char*)calloc(sequenceLength,sizeof(char));
	MPI_Recv(sequence, sequenceLength, MPI_CHAR, source, tag,MPI_COMM_WORLD, status);
	return sequence;
}

// Each processor receives the number of sequences according to the division made by the Master
int recieveNumberOfSequence(int source, int tag, MPI_Status *status) 
{
	int tasks;
	MPI_Recv(&tasks, 1, MPI_INT, source, tag, MPI_COMM_WORLD, status);
	return tasks;
}

// Master -> process
char* recieveSequence(int source, int tag, MPI_Status* status)
{
	char* sequence;
	int seqSize;
	MPI_Recv(&seqSize,1,MPI_INT,source,tag,MPI_COMM_WORLD,status);
	sequence = (char*)calloc(seqSize,sizeof(char));
	MPI_Recv(sequence, seqSize, MPI_CHAR, source, tag,MPI_COMM_WORLD, status);
	return sequence;
}

// Each process after calculating the socre sends it to the Master
void sendScore(score* maxScore,int dest,int tag)
{
	MPI_Send(maxScore,1,scoreMPIType(),dest,tag,MPI_COMM_WORLD);
}