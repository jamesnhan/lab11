#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include "timer.h"

const int MAX_THREADS = 1024;

double left, right, integral, base_len;
int thread_count, trap_count;
sem_t* semaphores;

void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
void* Thread_trap(void* rank);
double f(double x);

int main(int argc, char* argv[])
{
	long thread;
	pthread_t* thread_handles;
	double start, finish, elapsed;
	
	Get_args(argc, argv);

	thread_handles = (pthread_t*)malloc(thread_count*sizeof(pthread_t));
	srand(time(NULL));
	
	semaphores = malloc(thread_count*sizeof(sem_t));
	for (thread = 0; thread < thread_count; ++thread)
	{
		sem_init(&semaphores[thread], 0, 0);
	}

	GET_TIME(start);
	integral = 0.0;
	base_len = (right - left) / trap_count;
	for (thread = 0; thread < thread_count; ++thread)
	{
		pthread_create(&thread_handles[thread], NULL,
			Thread_trap, (void*)thread);
	}
	for (thread = 0; thread < thread_count; ++thread)
	{
		pthread_join(thread_handles[thread], NULL);
	}
	GET_TIME(finish);
	elapsed = finish - start;

	printf("With trap_count = %d intervals\n", trap_count);
	printf("	Multi-threaded estimate of the integral = %.15f\n", integral);
	printf("	Elapsed time = %e seconds\n", elapsed);

	for (thread = 0; thread < thread_count; ++thread)
	{
		sem_destroy(&semaphores[thread]);
	}

	free(semaphores);
	free(thread_handles);
	return 0;
}

void* Thread_trap(void* rank)
{
	long my_rank = (long) rank;
	double local_integral, local_left, local_right, x;
	int local_trap_count = trap_count / thread_count;
	int i;

	local_left = left + my_rank * local_trap_count * base_len;
	local_right = local_left + local_trap_count * base_len;

	local_integral = (f(local_left) + f(local_right)) / 2.0;
	for (i = 1; i <= local_trap_count - 1; ++i)
	{
		x = local_left + i * base_len;
		local_integral += f(x);
	}
	local_integral = local_integral * base_len;

	integral += local_integral;
	sem_post(&semaphores[(my_rank + 1) % thread_count]);
	sem_wait(&semaphores[my_rank]);
	return NULL;
}

double f(double x)
{
	return x * x;
}

void Get_args(int argc, char* argv[])
{
	if (argc != 5) Usage(argv[0]);
	left = atof(argv[1]);
	right = atof(argv[2]);
	trap_count = strtol(argv[3], NULL, 10);
	if (trap_count <= 0) Usage(argv[0]);
	thread_count = strtol(argv[4], NULL, 10);
	if (thread_count <= 0 || thread_count > MAX_THREADS) Usage(argv[0]);
}
void Usage(char* prog_name)
{
	fprintf(stderr, "usage: %s <left endpoint> <right endpoint> <trap count> <n>\n", prog_name);
	fprintf(stderr, "	trap count is the number of trapezoids and should be >= 1\n");
	fprintf(stderr, "	n is the number of threads and should be >= 1\n");
	exit(0);
}
