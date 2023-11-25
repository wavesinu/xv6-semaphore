#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "semaphore.h"

#define BUFFER_SIZE 1
#define DATA_SIZE 10000

int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;
int pro_counter[2] = {0, 0};
int con_counter[2] = {0, 0};

int empty;
int full;
int mutex;

void *producer(void *param)
{
    int id = *((int *)param);
    for (int i = 0; i < DATA_SIZE; i++)
    {
        sem_wait(empty);
        sem_wait(mutex);

        buffer[in] = i;
        in = (in + 1) % BUFFER_SIZE;
        pro_counter[id]++;

        sem_signal(mutex);
        sem_signal(full);
    }
    exit();
}

void *consumer(void *param)
{
    int id = *((int *)param);
    for (int i = 0; i < DATA_SIZE; i++)
    {
        sem_wait(full);
        sem_wait(mutex);

        out = (out + 1) % BUFFER_SIZE;
        con_counter[id]++;

        sem_signal(mutex);
        sem_signal(empty);
    }
    exit();
}

int main()
{
    int pro[2], con[2];
    int pro_id[2] = {0, 1}, con_id[2] = {0, 1};

    empty = sem_create(BUFFER_SIZE);
    full = sem_create(0);
    mutex = sem_create(1);

    for (int i = 0; i < 2; i++)
    {
        pro[i] = hufs_thread_create(producer, &pro_id[i]);
        con[i] = hufs_thread_create(consumer, &con_id[i]);
    }

    for (int i = 0; i < 2; i++)
    {
        hufs_thread_join(pro[i]);
        hufs_thread_join(con[i]);
    }

    printf(1, "producer (1): %d produced\n", pro_counter[0]);
    printf(1, "producer (2): %d produced\n", pro_counter[1]);
    printf(1, "consumer (1): %d consumed\n", con_counter[0]);
    printf(1, "consumer (2): %d consumed\n", con_counter[1]);

    exit();
}