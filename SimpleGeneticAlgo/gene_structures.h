#ifndef _GENE_STRUCTURES_H
#define _GENE_STRUCTURES_H

#include <math.h>
#include <stdio.h>
#include "gene.h"

typedef struct {
	int x;
	int y;
} Point2D_Circle;

/*
	Evaluates the fitness of a Point2D_Circle structure. 
	The point is most fit if it is closest to a circle of 
	radius 100 centered at origin.
*/
double evaluateFitness(Gene<Point2D_Circle>& gene)
{
	double x = gene().x;
	double y = gene().y;
	double dist = sqrt(x*x+y*y) - 100;
	dist *= dist;

	return exp(-dist);
}

/*
	Generates a random gene for Point2D_Circle
*/
void randomizeGene(Gene<Point2D_Circle>& theGene)
{
	theGene().x = rand() % 1000;
	theGene().y = rand() % 1000;
}

void printGene(Gene<Point2D_Circle>& gene)
{
	printf("X: %d, Y: %d", gene().x, gene().y);
}
#endif