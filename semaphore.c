#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "mmu.h"
#include "proc.h"

#define NSEMS 32	   // 32 entries for semophore table
#define MAX_WAITERS 64 // maximum no of waiters

extern struct
{
	struct spinlock lock;
	struct proc proc[NPROC];
} ptable;

struct semaphore
{
	int value;
	int active; // if it is used or not
	int waiters[MAX_WAITERS];
	struct spinlock lock;
} sem[NSEMS];

// function to initiallize the semophore table (at booting time)
// You DO NOT need call this seminit function!!
// proc.c kill 함수 참조
void seminit(void)
{
	int i, j;

	for (i = 0; i < NSEMS; ++i)
	{
		initlock(&sem[i].lock, "semaphore");
		sem[i].active = 0;
		sem[i].value = 0;

		for (j = 0; j < MAX_WAITERS; j++)
			sem[i].waiters[j] = -1; // HUFS
	}
}

int sem_create(int max)
{
	int i;

	// find an entry is NOT active (not used)
	for (i = 0; i < NSEMS; i++)
	{
		acquire(&sem[i].lock);
		if (sem[i].active == 0)
		{
			sem[i].value = max;
			sem[i].active = 1; // mark it as used (will be)
			release(&sem[i].lock);
			return i;
		}
		release(&sem[i].lock);
	}

	return -1;
}

int sem_destroy(int num)
{
	// check if the entry is valid
	if (num < 0 || num > NSEMS)
		return -1;

	acquire(&sem[num].lock);
	// check if the entry is actived
	if (sem[num].active != 1)
	{
		release(&sem[num].lock);
		return -1;
	}
	sem[num].active = 0;
	release(&sem[num].lock);

	return 0;
}

static int enqueue_waiter(struct semaphore *s, int pid)
{
	for (int i = 0; i < MAX_WAITERS; i++)
	{
		if (s->waiters[i] == -1)
		{
			s->waiters[i] = pid;
			return 0;
		}
	}
	return -1;
}

static int dequeue_waiter(struct semaphore *s)
{
	for (int i = 0; i < MAX_WAITERS; i++)
	{
		if (s->waiters[i] != -1)
		{
			int pid = s->waiters[i];
			s->waiters[i] = -1;
			return pid;
		}
	}
	return -1;
}

int sem_wait(int sem_id)
{
	if (sem_id < 0 || sem_id >= NSEMS)
		return -1;

	acquire(&sem[sem_id].lock);
	if (--sem[sem_id].value < 0)
	{
		enqueue_waiter(&sem[sem_id], proc->pid);
		block(&sem[sem_id].lock);
	}
	else
	{
		release(&sem[sem_id].lock);
	}

	return 0;
}

int sem_signal(int sem_id)
{
	if (sem_id < 0 || sem_id >= NSEMS)
		return -1;

	acquire(&sem[sem_id].lock);
	if (sem[sem_id].value++ < 0)
	{
		int pid = dequeue_waiter(&sem[sem_id]);
		if (pid != -1)
		{
			wakeup_pid(pid, &sem[sem_id].lock);
		}
	}
	else
	{
		release(&sem[sem_id].lock);
	}

	return 0;
}