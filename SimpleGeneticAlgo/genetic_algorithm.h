#ifndef _GENETIC_ALGO_H
#define _GENETIC_ALGO_H

#include <vector>
#include <algorithm>
#include "gene.h"

template<typename inner_data_type>
using PopulationList = std::vector<Gene<inner_data_type>>;

template<typename inner_data_type>
class GeneticAlgorithm
{
	typedef PopulationList<inner_data_type> PopnList;
	typedef Gene<inner_data_type> GeneType;
	PopnList *m_pPopulation;
	
	std::vector<double> *m_pCumuFitness;

	GeneType m_overallBestGene;
	double m_dOverallBestFitness;

public:
	double m_dMutationRate;
	int m_iPopulationSize;
	double m_dKillInvalidGeneProbability;
	int m_iIterationCount;
	bool m_bVerbose;

	/*
		Constructor for the algorithm
	*/
	GeneticAlgorithm() :
		m_dMutationRate(0.1),
		m_iPopulationSize(1000),
		m_pPopulation(0),
		m_pCumuFitness(0),
		m_dKillInvalidGeneProbability(0.5),
		m_iIterationCount(0),
		m_bVerbose(true),
		m_dOverallBestFitness(0)
	{
		srand(clock());
	}

	/*
		A single iteration. Creates the initial pool if needed.
	*/
	void step()
	{
		int i;
		double fitness;
		double cumuFitness = 0;
		double bestFitness = 0;
		GeneType bestGene;

		// re-populate
		if (m_pPopulation)
		{
			PopnList* pNewPop = new PopnList;
			int geneId1, geneId2;
			UINT bitOffset;
			std::vector<double> *pCumuFitness = new std::vector<double>;

			while(pNewPop->size() < m_iPopulationSize) {
				// choose 2 genes by roulette selection
				chooseTwoGenes(&geneId1, &geneId2);

				// create gene copies
				GeneType gene1((*m_pPopulation)[geneId1]);
				GeneType gene2((*m_pPopulation)[geneId2]);

				// crossover at a random bit
				bitOffset = randomInt() % gene1.dataSizeInBits();
				gene1.cross(bitOffset, gene2);

				// mutate
				mutateGene(gene1);
				mutateGene(gene2);

				fitness = gene1.fitness();
				if (fitness > 0 || !shouldKillGene()) {
					pNewPop->push_back(gene1);
					cumuFitness += fitness;
					pCumuFitness->push_back(cumuFitness);
				}
				if (fitness > bestFitness) {
					bestFitness = fitness;
					bestGene = gene1;
				}
				if (fitness > m_dOverallBestFitness) {
					m_overallBestGene = gene1;
					m_dOverallBestFitness = fitness;
				}

				fitness = gene2.fitness();
				if (fitness > 0 || !shouldKillGene()) {
					pNewPop->push_back(gene2);
					cumuFitness += fitness;
					pCumuFitness->push_back(cumuFitness);
				}
				if (fitness > bestFitness) {
					bestFitness = fitness;
					bestGene = gene2;
				}
				if (fitness > m_dOverallBestFitness) {
					m_overallBestGene = gene2;
					m_dOverallBestFitness = fitness;
				}
			}

			// delete old population
			delete m_pPopulation;
			m_pPopulation = pNewPop;

			delete m_pCumuFitness;
			m_pCumuFitness = pCumuFitness;

		}
		else
		{
			m_pPopulation = new PopnList;
			m_pCumuFitness = new std::vector<double>;

			for (i = 0; i < m_iPopulationSize; i++) {
				GeneType gene; // create a gene
				randomizeGene(gene); // randomize it by some custom rule

				// mutate gene
				mutateGene(gene);


				fitness = gene.fitness();
				if (fitness <= 0 && shouldKillGene()) {// invalid gene
					i--; continue; // redo
				}

				cumuFitness += fitness;
				m_pPopulation->push_back(gene); // append to population
				m_pCumuFitness->push_back(cumuFitness);

				if (fitness > bestFitness) {
					bestFitness = fitness;
					bestGene = gene;
				}
				if (fitness > m_dOverallBestFitness) {
					m_overallBestGene = gene;
					m_dOverallBestFitness = fitness;
				}
			}			
		}

		m_iIterationCount++;

		if (m_bVerbose)
		{
			printf("Iteration %d - Best fitness: %.3e\n", m_iIterationCount, bestFitness);
			bestGene.print();
			printf("\n");
		}
	}

	/*
		Resets this algo, clearing all population
	*/
	void reset()
	{
		if (m_pPopulation) {
			delete m_pPopulation;
			m_pPopulation = 0;
		}

		m_dOverallBestFitness = 0;
		m_iIterationCount = 0;
	}

	/*
		Returns the population list
	*/
	PopnList& population()
	{
		return *m_pPopulation;
	}

	GeneType& getBestGene()
	{
		return m_overallBestGene;
	}

	double getBestFitness()
	{
		return m_dOverallBestFitness;
	}

	friend void randomizeGene(Gene<inner_data_type>& theGene);


private:
	/*
		Helper method - returns a random double from 0 up to specified number.
	*/
	double randomDouble(double max)
	{
		UINT s = static_cast<UINT>(rand()) & 0xFFFF;
		double ret = s / 65535.0 * max;

		return ret;
	}

	/*
		Helper method - returns a random unsigned integer
	*/
	UINT randomInt()
	{
		return static_cast<UINT>(rand());
	}


	/*
		Returns 2 distinct indices to genes in population by roulette 
		selection
	*/
	void chooseTwoGenes(int* id1, int* id2)
	{
		double largestCumuFitness = m_pCumuFitness->back();
		*id1 = getFirstGeneWithCumulativeFitness(
			randomDouble(largestCumuFitness));

		do
			*id2 = getFirstGeneWithCumulativeFitness(
				randomDouble(largestCumuFitness));
		while (*id2 == *id1);
	}

	int getFirstGeneWithCumulativeFitness(double cumuFitness)
	{
		for (int i = 0; i < m_pCumuFitness->size(); i++) {
			if ((*m_pCumuFitness)[i] >= cumuFitness)
				return i;
		}
		return (*m_pCumuFitness).size() - 1;
	}

	/*
		Mutates the given gene
	*/
	void mutateGene(GeneType& gene)
	{
		if (randomDouble(1) < m_dMutationRate) 
			gene.flipBit(randomInt() % gene.dataSizeInBits());
	}


	/*
		Helper method - determines by chance if a gene should be killed
	*/
	bool shouldKillGene()
	{
		if (m_dKillInvalidGeneProbability == 1) return true;
		else {
			double x = randomDouble(1);
			return (x < m_dKillInvalidGeneProbability);
		}
	}

};

#endif