#ifndef _GENETIC_ALGO_H
#define _GENETIC_ALGO_H

#include <random>
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

	std::uniform_real_distribution<double> m_random;
	std::default_random_engine m_generator;

public:
	double m_dMutationRate;
	double m_dCrossoverRate;
	int m_iPopulationSize;
	int m_iIterationCount;
	bool m_bVerbose;
	int m_iStatusPrintInterval;

	/*
		Constructor for the algorithm
	*/
	GeneticAlgorithm() :
		m_dMutationRate(0.4),
		m_iPopulationSize(1000),
		m_pPopulation(0),
		m_pCumuFitness(0),
		m_iIterationCount(0),
		m_bVerbose(true),
		m_iStatusPrintInterval(100),
		m_dOverallBestFitness(0),
		m_random(0,1),
		m_generator(clock())
	{
		GeneType dummy;
		m_dCrossoverRate = 3.0 / dummy.dataSizeInBits();
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
			std::vector<double> *pCumuFitness = new std::vector<double>;

			while((int)pNewPop->size() < m_iPopulationSize) {
				// choose 2 genes by roulette selection
				chooseTwoGenes(&geneId1, &geneId2);

				// create gene copies
				GeneType gene1((*m_pPopulation)[geneId1]);
				GeneType gene2((*m_pPopulation)[geneId2]);

				// crossover
				crossOver(gene1, gene2);

				// mutate
				mutateGene(gene1);
				mutateGene(gene2);

				fitness = gene1.fitness();
				if (fitness > 0) {
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
				if (fitness > 0) {
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

			// shuffle population (mingle mingle)
			std::random_shuffle(m_pPopulation->begin(), m_pPopulation->end());

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
				if (fitness <= 0) {// invalid gene
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
		if (m_bVerbose && 
			(m_iIterationCount % m_iStatusPrintInterval == 0 || m_iIterationCount==1))
		{
			printf("Iteration %d - Best fitness: %.6e\n", m_iIterationCount, bestFitness);
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
		for (UINT i = 0; i < m_pCumuFitness->size(); i++) {
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
		if (randomDouble(1) < m_dMutationRate) {
			int bitOffset = static_cast<int>(randomDouble(gene.dataSizeInBits()));
			gene.flipBit(bitOffset);
		}
	}

	/*
		Crosses two genes
	*/
	void crossOver(GeneType& gene1, GeneType& gene2)
	{
		for (UINT i = 0; i < gene1.dataSizeInBits(); i++) {
			if (randomDouble(1) < m_dCrossoverRate)
				gene1.cross(i, gene2);
		}
	}

};

#endif