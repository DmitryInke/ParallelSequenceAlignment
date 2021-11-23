build:
	mpicxx -fopenmp -c main.c -o main.o
	mpicxx -fopenmp -c cFunctions.c -o cFunctions.o
	mpicxx -fopenmp -c mpiHelper.c -o mpiHelper.o
	mpicxx -fopenmp -o exec main.o cFunctions.o mpiHelper.o
	
clean:
	rm -f *.o ./exec

run:
	mpiexec -np 3 ./exec < input.txt

runOn2:
	mpiexec -np 2 -machinefile  mf  -map-by  node  ./exec

