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

void put(int value, int id)
{
        sem_wait(empty);
        sem_wait(mutex);

        buffer[fill_ptr] = value;
        fill_ptr = (fill_ptr + 1) % BUFFER_SIZE;
        count++;
        pro_counter[id]++;

        sem_signal(mutex);
        sem_signal(full);
}

int get(int id)
{
        sem_wait(full);
        sem_wait(mutex);

        int tmp = -1;

        tmp = buffer[use_ptr];
        use_ptr = (use_ptr + 1) % BUFFER_SIZE;
        count--;
        con_counter[id]++;

        sem_signal(mutex);
        sem_signal(empty);
        return tmp;
}

void *producer(void *arg)
{
    int i;
    int id = *(int *)arg;
    for (i = 0; i < N; i++)
    {
        put(i, id % 2);
    }
        return 0;
}

void *consumer(void *arg)
{
    int i;
    int id = *(int *)arg;
    for (i = 0; i < N; i++)
    {
        get(id % 2);
    }
        return 0;
}

int main()
{
    int i;
    int ids[4] = {0, 1, 2, 3};

    empty = sem_create(BUFFER_SIZE);
    full = sem_create(0);
    mutex = sem_create(1);


        // Check if semaphore creation is successful
    if (empty < 0 || full < 0 || mutex < 0) {
        printf(1, "Error creating semaphores\n");
        exit();
    }

    // Create producer and consumer threads
    for (int i = 0; i < 2; i++) {
        if (hufs_thread_create(producer, &ids[i]) < 0) {
            printf(1, "Error creating producer thread\n");
            exit();
        }
        if (hufs_thread_create(consumer, &ids[i+2]) < 0) {
            printf(1, "Error creating consumer thread\n");
            exit();
        }
    }

    for (i = 0; i < 4; i++)
    {
        hufs_thread_join(ids[i]);
    }

    printf(1, "producer (1): %d produced\n", pro_counter[0]);
    printf(1, "producer (2): %d produced\n", pro_counter[1]);
    printf(1, "consumer (1): %d consumed\n", con_counter[0]);
    printf(1, "consumer (2): %d consumed\n", con_counter[1]);

    exit();
}