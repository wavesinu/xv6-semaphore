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

static int enqueue_waiter(struct semaphore *sem, int pid)
{
	for (int i = 0; i < MAX_WAITERS; i++)
	{
		if (sem->waiters[i] == 0)
		{ // 0은 비어있음을 나타냄
			sem->waiters[i] = pid;
			return 0; // 성공
		}
	}
	return -1; // 실패: 대기열이 가득 참
}

static int dequeue_waiter(struct semaphore *sem)
{
	for (int i = 0; i < MAX_WAITERS; i++)
	{
		if (sem->waiters[i] != 0)
		{
			int pid = sem->waiters[i];
			sem->waiters[i] = 0; // 대기열에서 제거
			return pid;
		}
	}
	return -1; // 실패: 대기열이 비어 있음
}

int sem_wait(int sem_id)
{
	acquire(&sem[sem_id].lock);

	while (sem[sem_id].value <= 0)
	{
		enqueue_waiter(&sem[sem_id], mypid());
		block(&sem[sem_id].lock);
	}

	sem[sem_id].value--;
	release(&sem[sem_id].lock);
}

int sem_signal(int sem_id)
{
	acquire(&sem[sem_id].lock);

	if (++sem[sem_id].value <= 0)
	{
		int pid = dequeue_waiter(&sem[sem_id]);
		if (pid >= 0)
		{
			wakeup_pid(pid, &ptable.lock);
		}
	}

	release(&sem[sem_id].lock);
}
