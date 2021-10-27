#pragma once

#define BUFFER_SIZE 3000
#define MASTER 0
#define FILE_NAME "input.txt"
#define OUTPUT_FILE "output.txt"

#define CONSERVATIVE 9
#define SECOND_CONSERVATIVE 11

struct Weight {
	float w1;
	float w2;
	float w3;
	float w4;
}typedef weight;

struct Score {
	int n; // offset
	int k; // hyphen index mutant
	float scoreWeight; // weight
}typedef score;
