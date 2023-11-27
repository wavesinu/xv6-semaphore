#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "semaphore.h"

#define BUFFER_SIZE 1
#define DATA_COUNT 10000

int buffer[BUFFER_SIZE];
int in = 0, out = 0;

int empty, full, mutex;
int pro_counter[2] = {0, 0}, con_counter[2] = {0, 0};

void *producer(void *arg) {
    int id = (int)arg;
    for (;;) {
        sem_wait(empty);
        sem_wait(mutex);

        if (pro_counter[0] + pro_counter[1] >= DATA_COUNT) {
            sem_signal(mutex);
            sem_signal(empty);
            break;
        }

        buffer[in] = 1; // Produce data
        in = (in + 1) % BUFFER_SIZE;

        sem_signal(mutex);
        sem_signal(full);

                sem_wait(mutex);
                pro_counter[id]++;
                sem_signal(mutex);
    }
    return 0;
}

void *consumer(void *arg) {
    int id = (int)arg;
    for (;;) {
        sem_wait(full);
        sem_wait(mutex);

        if (con_counter[0] + con_counter[1] >= DATA_COUNT) {
            sem_signal(mutex);
            sem_signal(full);
            break;
        }

        out = (out + 1) % BUFFER_SIZE;

        sem_signal(mutex);
        sem_signal(empty);

                sem_wait(mutex);
                con_counter[id]++;
                sem_signal(mutex);
    }
    return 0;
}

int main(void) {
    empty = sem_create(BUFFER_SIZE);
    full = sem_create(0);
    mutex = sem_create(1);

    hufs_thread_create(producer, (void *)0);
    hufs_thread_create(producer, (void *)1);
    hufs_thread_create(consumer, (void *)0);
    hufs_thread_create(consumer, (void *)1);

    for (int i = 0; i < 4; i++)
        hufs_thread_join(i);

    printf(1, "producer (1): %d produced\n", pro_counter[0]);
    printf(1, "producer (2): %d produced\n", pro_counter[1]);
    printf(1, "consumer (1): %d consumed\n", con_counter[0]);
    printf(1, "consumer (2): %d consumed\n", con_counter[1]);

    exit();
}