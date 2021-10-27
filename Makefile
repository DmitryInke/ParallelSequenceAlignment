build:
	mpicxx -fopenmp -c main.c
	mpicxx -fopenmp -c helperFunctions.c
	mpicxx -fopenmp -c cFunctions.c
	nvcc -I./inc -c cudaFunctions.cu
	mpicxx -fopenmp -o finalProject main.o helperFunctions.o cFunctions.o cudaFunctions.o /usr/local/cuda-11.0/lib64/libcudart_static.a -ldl -lrt

clean:
	rm -f *.o ./finalProject

run:
	mpiexec -n 2 ./finalProject

runOn2:
	mpiexec -np 2 -machinefile  mf  -map-by  node  ./finalProject
