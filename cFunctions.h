#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "myStructs.h"
#include "helperFunctions.h"

float calcWeight(const char* str, const weight* weights);

score maxWeight(const score *scoreList,const int size);

void calcScore(weight *weight, const char *mainSequence,const char *secSequence, score *maxScore);

char* computeOnGPU(int numElements,int hypenIdx,int offset,const char *mainSequence,const char *secSequence,int tid);

