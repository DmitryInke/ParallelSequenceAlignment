#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <string.h>
#include "myStructs.h"

__device__ const char* conservative[]={"NDEQ","NEQK","STA","MILV","QHRK","NHQK","FYW","HY","MILF"};

__device__ const char* secondConservative[]={"SAG","ATV","CSA","SGND","STPA","STNK","NEQHRK","NDEQHK","SNDEQK","HFY","FVLIM"};

__device__ int checkChar(const char* s, char c)
{
	do{
		if(*s == c) return 1;
	}while (*s++);
	return 0;
}

__device__ int compare(const char* conservative[],const int size,char ch1, char ch2)
{
	for (int i =0; i< size;i++)
    {
		if(checkChar(conservative[i],ch1) && checkChar(conservative[i],ch2))
			return 1;
	}
	return 0;
}

__global__  void createMutant(char *arr,int numElements,int hyphenIdx,int offset,const char *mainSequence,const char *secSequence)
{
	int i = blockDim.x * blockIdx.x + threadIdx.x;
	if (i >= numElements-1)
		return;
	int g = 0;

	if(i > hyphenIdx){
		g = -1;	
	}
	if(i == hyphenIdx){
		arr[i] = '-';
	}		
	else if(mainSequence[i+offset] == secSequence[i + g])
		arr[i]= '$'; 
	else if(compare(conservative,CONSERVATIVE,mainSequence[i+offset],secSequence[i + g]))
		arr[i]= '%';
	else if(compare(secondConservative,SECOND_CONSERVATIVE,mainSequence[i+offset],secSequence[i + g]))
		arr[i]= '#';
	else
		arr[i] = ' ';
}

char* computeOnGPU(int numElements,int hypenIdx,int offset,const char *mainSequence,const char *secSequence,int tid) 
{
    // Error code to check return values for CUDA calls
    cudaError_t err = cudaSuccess;

	//Using cuda stream
	const int num_streams = 8;
	cudaStream_t stream[num_streams];
	cudaStreamCreate(&stream[tid]);

    // Allocate memory on GPU to copy the data from the host
    char *d_Mutant;
	size_t size = numElements * sizeof(char);
    err = cudaMalloc((void **)&d_Mutant, size);
    if (err != cudaSuccess) 
    {
        fprintf(stderr, "Failed to allocate device memory - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
	cudaMemset(d_Mutant,0,size);
	
	 // Allocate memory on GPU to copy the data from the host
    char *d_mainSequence;
	size_t size_mainSequence = (strlen(mainSequence) + 1) * sizeof(char);
    err = cudaMalloc((void **)&d_mainSequence, size_mainSequence);
    if (err != cudaSuccess) 
    {
        fprintf(stderr, "Failed to allocate device memory - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Copy data from host to the GPU memory
    err = cudaMemcpy(d_mainSequence, mainSequence, size_mainSequence, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) 
    {
        fprintf(stderr, "Failed to copy data from host to device - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
	
	 // Allocate memory on GPU to copy the data from the host
    char *d_secSequence;
	size_t size_secSequence = (strlen(secSequence) + 1) * sizeof(char);
    err = cudaMalloc((void **)&d_secSequence, size_secSequence);
    if (err != cudaSuccess) 
    {
        fprintf(stderr, "Failed to allocate device memory - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Copy data from host to the GPU memory
    err = cudaMemcpy(d_secSequence, secSequence, size_secSequence, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) 
    {
        fprintf(stderr, "Failed to copy data from host to device - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }


    int threadsPerBlock = 256;
    int blocksPerGrid =(numElements + threadsPerBlock - 1) / threadsPerBlock;
	createMutant<<<blocksPerGrid, threadsPerBlock, 0, stream[tid]>>>(d_Mutant,numElements,hypenIdx,offset,d_mainSequence,d_secSequence);

    err = cudaGetLastError();
    if (err != cudaSuccess) 
    {
        fprintf(stderr, "Failed to launch vectorAdd kernel -  %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

	char* result = (char*)malloc(numElements*sizeof(char));
    // Copy the  result from GPU to the host memory.
    err = cudaMemcpyAsync(result, d_Mutant, numElements, cudaMemcpyDeviceToHost, stream[tid]);

    if (err != cudaSuccess) 
    {
        fprintf(stderr, "Failed to copy result array from device to host -%s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Free allocated memory on GPU:
    if (cudaFree(d_Mutant) != cudaSuccess) 
    {
        fprintf(stderr, "Failed to free device data - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    if (cudaFree(d_mainSequence) != cudaSuccess) 
    {
        fprintf(stderr, "Failed to free device data - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    if (cudaFree(d_secSequence) != cudaSuccess) 
    {
        fprintf(stderr, "Failed to free device data - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

	cudaStreamDestroy(stream[tid]);
    return result;
}
