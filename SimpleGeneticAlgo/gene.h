#ifndef _GENE_H
#define _GENE_H

#include "constants.h"
#include <string.h>
#include <stdlib.h>

/*
	Wraps a data type as a gene for use within the 
	algorithm.
	@param inner_data_type The datatype to be wrapped
*/
template<typename inner_data_type>
class Gene
{
	inner_data_type m_data;

public:
	
	/*
		Constructor
	*/
	Gene()
	{
	}

	/*
		Copy constructor
	*/
	Gene(const inner_data_type& src)
	{
		memcpy(&m_data, &src, sizeof(inner_data_type));
	}

	/*
		The size, in bytes, of the underlying data
	*/
	size_t dataSize()
	{
		static size_t sz = sizeof(inner_data_type);
		return sz;
	}

	size_t dataSizeInBits()
	{
		static size_t sz = dataSize() << 3;
		return sz;
	}


	void print()
	{
		printGene(*this);
	}

	/*
		Easy access to underlying data
	*/
	inner_data_type& operator()()
	{
		return m_data;
	}

	Gene<inner_data_type>& operator=(const Gene<inner_data_type>& other)
	{
		if (this != &other) 
			memcpy(&m_data, &other.m_data, sizeof(inner_data_type));
		return *this;
	}

	//
	// GENE OPERATIONS
	//

	/*
		Gets the bit at the given offset
		Take note that there is no bound checking.
	*/
	BIT getBit(size_t bitOffset)
	{
		size_t whichByte, whichBit;
		computeOffsets(bitOffset, &whichBit, &whichByte);

		UCHAR mask = 1 << whichBit;
		UCHAR* ptr = (UCHAR*)&m_data;
		return ((ptr[whichByte] & mask) >> whichBit);
	}

	/*
		Sets the bit at the specified offset
		Take note that there is no bound checking.
	*/
	void setBit(size_t bitOffset, BIT newBit)
	{
		size_t whichByte, whichBit;
		computeOffsets(bitOffset, &whichBit, &whichByte);

		UCHAR mask = 1 << whichBit;
		UCHAR* ptr = (UCHAR*)&m_data;

		if (newBit)
			ptr[whichByte] |= mask;
		else
			ptr[whichByte] &= ~mask;
	}

	/*
		Flips the bit at the specified offset
	*/
	void flipBit(size_t bitOffset)
	{
		size_t whichByte, whichBit;
		computeOffsets(bitOffset, &whichBit, &whichByte);

		UCHAR mask = 1 << whichBit;
		UCHAR* ptr = (UCHAR*)&m_data;

		ptr[whichByte] ^= mask;
	}

	/*
		Performs a crossover with a similar gene at the 
		specified bit offset
	*/
	void cross(size_t bitOffset, 
		Gene<inner_data_type>& partner)
	{
		size_t whichByte, whichBit;
		computeOffsets(bitOffset, &whichBit, &whichByte);

		UCHAR* pThis = (UCHAR*)&m_data;
		UCHAR* pThat = (UCHAR*)&partner.m_data;

		whichBit = 7 - whichBit;

		// swap the bits within the same byte offset 
		UCHAR maskL = 0xFF >> whichBit;
		UCHAR maskM = ~maskL;

		UCHAR resultByteForThis = (pThis[whichByte] & maskM) |
			(pThat[whichByte] & maskL);
		UCHAR resultByteForThat = (pThat[whichByte] & maskM) |
			(pThis[whichByte] & maskL);

		pThis[whichByte] = resultByteForThis;
		pThat[whichByte] = resultByteForThat;

		// swap the bytes after the current byte
		UCHAR temp;
		for (size_t i = whichByte + 1; i < dataSize(); i++) {
			temp = pThis[i];
			pThis[i] = pThat[i];
			pThat[i] = temp;
		}
	}


	/*
		Returns the fitness value of this gene. This references a method 
		with the signature:
		
		double evaluateFitness(Gene<inner_data_type>& gene)

		The function has to return a positive real value, whose magnitude 
		indicates the fitness of the gene. Negative or zero means invalid 
		gene.
	*/
	double fitness()
	{
		return evaluateFitness(*this);
	}

	/*
		Create functions with the following signature and pre-set 
		inner_data_type parameter to generate a fitness calculator
	*/
	friend double evaluateFitness(Gene<inner_data_type>& gene);

	/*
		Prints out the data of this gene
	*/
	friend void printGene(Gene<inner_data_type>& gene);

private:
	/*
		Helper method - calculates the byte offset and
		remainder bit offset. 
		@param whichBit Bit 0 is MSB, BIT 7 is LSB
	*/
	inline void computeOffsets(size_t bitOffset, 
		size_t* whichBit, size_t* whichByte)
	{
		*whichByte = bitOffset >> 3;
		*whichBit = 7 - (bitOffset & 0x7);
	}
};

template<class inner_data_type>
bool compare(Gene<inner_data_type>& i, Gene<inner_data_type>& j)
{
	return i.fitness() < j.fitness();
}

#endif