#include "gene_structures.h"
#include "genetic_algorithm.h"
#include <stdio.h>
#include <time.h>
/*
	Entry point
*/
int main(int argc, char** argv)
{
	srand(clock());
	GeneticAlgorithm<TaskScheduling> algo;
	
	for (int i = 0; i < 1000; i++)
		algo.step();

	printf("Best fitness: %.3e\n", algo.getBestFitness());
	algo.getBestGene().print();

	return 0;
}


double evaluateFitness(Gene<unsigned int>& gene)
{
	return static_cast<double>(gene());
}
