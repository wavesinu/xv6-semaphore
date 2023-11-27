#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define N 10000 // number of items to be produced and consumed
#define BUFFER_SIZE 1

int buffer[BUFFER_SIZE];
int fill_ptr = 0;
int use_ptr = 0;
int count = 0;

int pro_counter[2] = {0, 0};
int con_counter[2] = {0, 0};

int empty, full, mutex;

void put(int value)
{
    buffer[fill_ptr] = value;
    fill_ptr = (fill_ptr + 1) % BUFFER_SIZE;
    count++;
}

int get()
{
    int tmp = buffer[use_ptr];
    use_ptr = (use_ptr + 1) % BUFFER_SIZE;
    count--;
    return tmp;
}

void *producer(void *arg)
{
    int i;
    int id = *(int *)arg;
    for (i = 0; i < N; i++)
    {
        sem_wait(empty);
        sem_wait(mutex);
        put(i);
        pro_counter[id]++;
        sem_signal(mutex);
        sem_signal(full);
    }
    exit();
}

void *consumer(void *arg)
{
    int i;
    int id = *(int *)arg;
    for (i = 0; i < N; i++)
    {
        sem_wait(full);
        sem_wait(mutex);
        get();
        con_counter[id]++;
        sem_signal(mutex);
        sem_signal(empty);
    }
    exit();
}

int main()
{
    int i;
    int ids[2];

    empty = sem_create(BUFFER_SIZE);
    full = sem_create(0);
    mutex = sem_create(1);

    for (i = 0; i < 2; i++)
    {
        hufs_thread_create(producer, (void *)ids[i]);
        hufs_thread_create(consumer, (void *)ids[i]);
    }

    for (i = 0; i < 4; i++)
    {
        hufs_thread_join(i + 1);
    }

    printf(1, "producer (1): %d produced\n", pro_counter[0]);
    printf(1, "producer (2): %d produced\n", pro_counter[1]);
    printf(1, "consumer (1): %d consumed\n", con_counter[0]);
    printf(1, "consumer (2): %d consumed\n", con_counter[1]);

    exit();
}