#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

#define MAX_NUM_THREADS					64
//#define NUM_THREADS					3	
//#define TARGET_COUNT_PER_THREAD		400	
int NUM_THREADS;
int TARGET_COUNT_PER_THREAD;


uint g_counter;
int sem_id;

void *thread(void *arg)
{
	int i;
	uint counter;

	sleep(10);
	printf(1, "thread %d: started...\n", *(int*)arg);

	for (i=0; i<TARGET_COUNT_PER_THREAD; i++) {
//		sem_wait(sem_id);
		
		counter = g_counter;
		sleep(0);
		counter++;
		sleep(0);
		g_counter = counter;

//		sem_signal(sem_id);
	}

	exit();
}

int main(int argc, char **argv)
{
	int i;
	int sem_size;
	int final_counter;
	int final_target = NUM_THREADS*TARGET_COUNT_PER_THREAD;

	/*
	if (argc >= 2)
		sem_size = NUM_THREADS;
	else
		sem_size = 1;
		*/

	if (argc < 3) {
		printf(2, "Usage: thread_nosync num_threads count_per_thread");
		exit();
	}

	NUM_THREADS = atoi(argv[1]);
	TARGET_COUNT_PER_THREAD = atoi(argv[2]);

	final_target = NUM_THREADS*TARGET_COUNT_PER_THREAD;

	//sem_size = NUM_THREADS; 
	sem_size = 1;

	// Initialize semaphore to 1
	if ((sem_id = sem_create(sem_size)) < 0)
	{
		printf(1, "main: error initializing semaphore %d\n");
		exit();
	}

	// Initialize counter
	g_counter = 0;

	// Set up thread stuff

	// Args
	int *args[MAX_NUM_THREADS];

	// Allocate stacks and args and make sure we have them all
	// Bail if something fails
	for (i=0; i<NUM_THREADS; i++) {

		args[i] = (int*) malloc(4);
		if (!args[i]) {
			printf(1, "main: could not get memory (for arg) for thread %d, exiting...\n");
			exit();
		}

		*args[i] = i;
	}

	printf(1, "main: running with %d threads...\n", NUM_THREADS);

	// Start all children
	int pid[MAX_NUM_THREADS];

	for (i=0; i<NUM_THREADS; i++) {
		pid[i] = hufs_thread_create(thread, args[i]);
		if (pid[i]==-1) {
			printf(1, "main: failed to creat a %d-th thread with pid %d\n", i);
		}
		else printf(1, "main: created thread with pid %d\n", pid[i]);
	}
	
	// Wait for all children
	for (i=0; i<NUM_THREADS; i++) {
		printf(1, "before joining... \n");
		if (pid[i]!=-1) 
			printf(1, "main: thread %d joined...\n", hufs_thread_join(pid[i]));
	}

	// Check the result
	final_counter = g_counter;
	printf(1, "Final counter is %d, target is %d\n", final_counter, final_target);
	if (final_counter == final_target)
		printf(1, "TEST PASSED!\n");
	else
		printf(1, "TEST FAILED!\n");
	
	// Clean up semaphore
	sem_destroy(sem_id);

	// Exit
	exit();
}
