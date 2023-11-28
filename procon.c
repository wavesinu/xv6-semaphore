#include "types.h"
#include "stat.h"
#include "user.h"

#define BUFFER_SIZE 1
#define TOTAL_DATA 10000

int buffer[BUFFER_SIZE];
int count = 0;
int produced = 0, consumed = 0;

int pro_counter[2] = {0, 0};
int con_counter[2] = {0, 0};

int empty, full, mutex;

void producer(void *arg) {
    int index = *(int*)arg;
    while (1) {
        sem_wait(empty);
        sem_wait(mutex);

        if (produced >= TOTAL_DATA) {
            sem_signal(mutex);
            sem_signal(full);
            break;
        }

        buffer[count] = produced;
        count++;
        produced++;
        pro_counter[index]++;

        sem_signal(mutex);
        sem_signal(full);
    }
    exit();
}

void consumer(void *arg) {
    int index = *(int*)arg;
    while (1) {
        sem_wait(full);
        sem_wait(mutex);

        if (consumed >= TOTAL_DATA) {
            sem_signal(mutex);
            sem_signal(empty);
            break;
        }

        count--;
        consumed++;
        con_counter[index]++;

        sem_signal(mutex);
        sem_signal(empty);
    }
    exit();
}

int main() {
    empty = sem_create(BUFFER_SIZE);
    full = sem_create(0);
    mutex = sem_create(1);

    int arg1 = 0, arg2 = 1;

	int pid1 = hufs_thread_create((void*)producer, (void*)&arg1);
    int pid3 = hufs_thread_create((void*)consumer, (void*)&arg1);
    int pid2 = hufs_thread_create((void*)producer, (void*)&arg2);
    int pid4 = hufs_thread_create((void*)consumer, (void*)&arg2);

    hufs_thread_join(pid1);
    hufs_thread_join(pid3);
    hufs_thread_join(pid2);
    hufs_thread_join(pid4);

    printf(1, "producer (1): %d produced\n", pro_counter[0]);
    printf(1, "producer (2): %d produced\n", pro_counter[1]);
    printf(1, "consumer (1): %d consumed\n", con_counter[0]);
    printf(1, "consumer (2): %d consumed\n", con_counter[1]);

    sem_destroy(empty);
    sem_destroy(full);
	sem_destroy(mutex);

	exit();
}