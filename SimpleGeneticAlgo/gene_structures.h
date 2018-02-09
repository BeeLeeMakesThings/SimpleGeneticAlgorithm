#ifndef _GENE_STRUCTURES_H
#define _GENE_STRUCTURES_H

#include <math.h>
#include <stdio.h>
#include "gene.h"

//
// Find a point closest to a circle of radius 100 centered at origin
// 

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

	return exp(-(dist*dist)/10000);
}

/*
	Generates a random gene for Point2D_Circle
*/
void randomizeGene(Gene<Point2D_Circle>& theGene)
{
	theGene().x = randomInt(1000) - 500;
	theGene().y = randomInt(1000) - 500;
}

void printGene(Gene<Point2D_Circle>& gene)
{
	printf("X: %d, Y: %d", gene().x, gene().y);
}

//
// Schedule 2 machines to finish a N tasks
// 
#define TOTAL_TIME_QUANTUM 40
#define TASK_DURATIONS { 4,5,6,7,5,8 }
#define TASK_COUNT 6
#define PENALTY_PREEMPTION 50.0
#define PENALTY_REDUNDANCY 10.0
#define PENALTY_DURATION 20.0

typedef struct {
	UCHAR machine_time1[TOTAL_TIME_QUANTUM];
	UCHAR machine_time2[TOTAL_TIME_QUANTUM];
} TaskScheduling;

typedef struct {
	bool done;
	int preemptCount;
	int totalDuration;
} CompletionStatus;

CompletionStatus totalJobDuration(TaskScheduling& schedules, int jobs[]);

/*
	Evaluates the fitness of a TaskScheduling structure.
*/
double evaluateFitness(Gene<TaskScheduling>& gene)
{
	// Rules:
	// 1. There are 4 jobs
	// 2. A job can only run on one machine at a time
	// 3. Each job takes a certain number of time quantums
	//    to complete
	//
	//
	// Penalties:
	// 1. Time to complete all jobs
	// 2. Extra time given to completed jobs before 
	//    last job is done (redundant work)
	// 3. Frequent pre-emption of jobs 
	//
	// Invalid:
	// 1. Some jobs are not completed
	// 2. A job exists concurrently on two machines

	int JOB_DURATION[] = TASK_DURATIONS;
	CompletionStatus status = totalJobDuration(gene(), JOB_DURATION);
	if (!status.done || status.totalDuration==0) return 0; // not done, invalid

	double penalty = 50.0*status.totalDuration; // duration to complete all jobs
	for (int i = 0; i < TASK_COUNT; i++) {
		if (JOB_DURATION[i] < 0)
			penalty += -JOB_DURATION[i]* 10.0; // redundant work
	}
	penalty += 20.0*status.preemptCount; // number of pre-emptions
	return 10*exp(-penalty/200.0);
}

CompletionStatus totalJobDuration(TaskScheduling& schedules, int jobs[])
{
	CompletionStatus status;
	status.done = false;
	status.preemptCount = 0;
	status.totalDuration = 0;

	UCHAR* p1 = schedules.machine_time1;
	UCHAR* p2 = schedules.machine_time2;

	int i;
	UINT doneFlags = 0xFFFFFFFF; // each bit represents the status of each job
	doneFlags >>= TASK_COUNT;
	doneFlags <<= TASK_COUNT;

	int lastJob1, lastJob2;

	for (i = 0; i < TOTAL_TIME_QUANTUM && doneFlags != 0xFFFFFFFF; i++) {
		int job1 = p1[i] % (TASK_COUNT + 1); // 0 = no job, 1-N = job indices
		int job2 = p2[i] % (TASK_COUNT + 1);

		if (job1 == job2 && job1 != 0) return status; // invalid config

		if (i > 0) { // count the preemptions
			if (job1 != lastJob1) status.preemptCount++;
			if (job2 != lastJob2) status.preemptCount++;
		}

		if (job1 > 0) {
			jobs[job1 - 1]--;
			if (jobs[job1 - 1] <= 0)
				doneFlags |= 1 << (job1 - 1);
		}
		if (job2 > 0) {
			jobs[job2 - 1]--;
			if (jobs[job2 - 1] <= 0)
				doneFlags |= 1 << (job2 - 1);
		}

		lastJob1 = job1;
		lastJob2 = job2;
	}
	status.done = (doneFlags == 0xFFFFFFFF);
	status.totalDuration = i;
	return status;
}

void randomizeGene(Gene<TaskScheduling>& theGene)
{
	for (int i = 0; i < TOTAL_TIME_QUANTUM; i++) {
		theGene().machine_time1[i] = randomInt(255) & 0xFF;
		theGene().machine_time2[i] = randomInt(255) & 0xFF;
	}
}

void printGene(Gene<TaskScheduling>& gene)
{
	int JOB_DURATION[] = TASK_DURATIONS;
	CompletionStatus status = totalJobDuration(gene(), JOB_DURATION);

	printf("Machine1: ");
	for (int i = 0; i < status.totalDuration; i++)
		printf("%d ", gene().machine_time1[i] % (TASK_COUNT + 1));
	printf("\nMachine2: ");
	for (int i = 0; i <  status.totalDuration; i++)
		printf("%d ", gene().machine_time2[i] % (TASK_COUNT + 1));
}

#endif