#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

#define MAX_NUM_THREADS		65	// since the pid is in the range of 1 and 64

struct _thread_info {
	void *stack;
} thread_info[MAX_NUM_THREADS]; 

int thread_num = 0;


int hufs_thread_create(void *func(), void *args)
{
	void *stack; 
	int pid;

	stack = malloc(4096);
	if (stack==0) return -1;

	pid = clone(func, args, stack); 
	if (pid==-1) {
		free(stack);
		return -1;
	}

	thread_info[pid].stack = stack; 

	return pid; 
}

int hufs_thread_join(int pid)
{
	void *stack = thread_info[pid].stack;

	if (stack==0) return -1;

	join(&thread_info[pid].stack);	
	free(stack);

	return pid;
}
