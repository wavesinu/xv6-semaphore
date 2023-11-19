#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

//#define NUM_CHILDREN 10
//#define TARGET_COUNT_PER_CHILD 5000
int NUM_CHILDREN;
int TARGET_COUNT_PER_CHILD; 
#define COUNTER_FILE "counter"

int sem_id;

int counter_init(char *filename, int value)
{
	int fd;

	if ((fd = open(filename, O_CREATE | O_RDWR)) < 0) {
		printf(1, "counter_init: error initializing file: %s\n", filename);
		exit();
	}

	printf(fd, "%d\n", value);
	close(fd);

	return 0;
}

int counter_get(char *filename)
{
	int fd, n, value;
	char buffer[32];

	if ((fd = open(filename, O_CREATE | O_RDWR)) < 0) {
		printf(1, "counter_get: error opening file: %s\n", filename);
		exit();
	}

	n = read(fd, buffer, 31);
	buffer[n] = '\0';
	value = atoi(buffer);
	close(fd);

	return value;
}

int counter_set(char *filename, int value)
{
	int fd;

	if ((fd = open(filename, O_CREATE | O_RDWR)) < 0) {
		printf(1, "counter_set: error opening file: %s\n", filename);
		exit();
	}

	printf(fd, "%d\n", value);
	close(fd);

	return value;
}

void child(void)
{
	int i;
	int counter;

	printf(1, "Process started...\n");
	sleep(10);

	for (i=0; i<TARGET_COUNT_PER_CHILD; i++) {
		sem_wait(sem_id);
		
		counter = counter_get("counter");
		counter++;
		counter_set("counter", counter);

		sem_signal(sem_id);
	}

	exit();
}

int main(int argc, char **argv)
{
	int i;
	int final_counter;
	int final_target;

	if (argc < 3) {
		printf(2, "Usage: proc_sem num_procs count_per_proc \n");
		exit();
	}

	NUM_CHILDREN = atoi(argv[1]);
	TARGET_COUNT_PER_CHILD = atoi(argv[2]);

	final_target = NUM_CHILDREN*TARGET_COUNT_PER_CHILD;

	// Initialize semaphore to 1
	if ((sem_id = sem_create(1)) < 0)
	{
		printf(1, "main: error initializing semaphore \n");
		exit();
	}

	// Initialize counter
	counter_init(COUNTER_FILE, 0);

	printf(1, "Running with %d processes...\n", NUM_CHILDREN);

	// Start all children
	for (i=0; i<NUM_CHILDREN; i++) {
		int pid = fork();
		if (pid == 0)
			child();
	}
	
	// Wait for all children
	for (i=0; i<NUM_CHILDREN; i++) {
		wait();
	}

	// Check the result
	final_counter = counter_get(COUNTER_FILE);
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
