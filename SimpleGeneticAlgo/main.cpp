#include "gene_structures.h"
#include "genetic_algorithm.h"
#include <stdio.h>

/*
	Entry point
*/
int main(int argc, char** argv)
{
	GeneticAlgorithm<Point2D_Circle> algo;
	algo.m_iStatusPrintInterval = 100;
	algo.m_iPopulationSize = 500;

	for (int i = 0; i < 1000; i++)
		algo.step();

	printf("Best fitness: %.3e\n", algo.getBestFitness());
	algo.getBestGene().print();

	return 0;
}
