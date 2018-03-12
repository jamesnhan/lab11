#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "timer.h"

const int MAX_THREADS = 1024;

long long int num_in_circle;
long long int num_tosses;
int thread_count;
pthread_mutex_t mutex;

void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
void* Thread_monty(void* _);

int main(int argc, char* argv[])
{
	long thread;
	pthread_t* thread_handles;
	double start, finish, elapsed;
	double pi_estimate;
	
	Get_args(argc, argv);

	thread_handles = (pthread_t*)malloc(thread_count*sizeof(pthread_t));
	srand(time(NULL));
	
	pthread_mutex_init(&mutex, NULL);

	GET_TIME(start);
	num_in_circle = 0;
	for (thread = 0; thread < thread_count; ++thread)
	{
		pthread_create(&thread_handles[thread], NULL,
			Thread_monty, (void*)thread);
	}
	for (thread = 0; thread < thread_count; ++thread)
	{
		pthread_join(thread_handles[thread], NULL);
	}
	GET_TIME(finish);
	elapsed = finish - start;

	pi_estimate = 4 * num_in_circle / ((double)num_tosses);
	printf("With num_tosses = %lld tosses\n", num_tosses);
	printf("	Multi-threaded estimate of pi = %.15f\n", pi_estimate);
	printf("	Elapsed time = %e seconds\n", elapsed);
	printf("	Math library estimate of pi   = %.15f\n",
		4.0 * atan(1.0));

	pthread_mutex_destroy(&mutex);
	free(thread_handles);
	return 0;
}

void* Thread_monty(void* _)
{
	double dist, x, y;
	long long int toss;
	long long int num_in_circle_local;
	long long int num_tosses_local = num_tosses / thread_count;

	num_in_circle_local = 0;
	for (toss = 0; toss < num_tosses_local; ++toss)
	{
		x = (double)rand() / RAND_MAX * 2.0 - 1.0;
		y = (double)rand() / RAND_MAX * 2.0 - 1.0;
		dist = x * x + y * y;
		if (dist <= 1)
		{
			++num_in_circle_local;
		}
	}

	pthread_mutex_lock(&mutex);
	num_in_circle += num_in_circle_local;
	pthread_mutex_unlock(&mutex);
	return NULL;
}

void Get_args(int argc, char* argv[])
{
	if (argc != 3) Usage(argv[0]);
	thread_count = strtol(argv[1], NULL, 10);
	if (thread_count <= 0 || thread_count > MAX_THREADS) Usage(argv[0]);
	num_tosses = strtoll(argv[2], NULL, 10);
	if (num_tosses <= 0) Usage(argv[0]);
}
void Usage(char* prog_name)
{
	fprintf(stderr, "usage: %s <number of threads> <n>\n", prog_name);
	fprintf(stderr, "	n is the number of tosses and should be >= 1\n");
	fprintf(stderr, "	n should be evenly divisible by the number of threads\n");
	exit(0);
}
