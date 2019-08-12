// producer consumer problem with shared memeory and semaphores
// to build: make all
// to run ./consumer ./producer
// 


#include <assert.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include "pcwshmem.h"

sem_t *p_sem_lock = NULL;
sem_t *p_sem_not_empty = NULL;
sem_t *p_sem_not_full = NULL;


MESSAGE_T* get_msg(PCQUEUE* pq) {
    if (pq->qsize > 0)
        return &pq->buffer[pq->tail];
    else
        return NULL;
}
void dispose_msg(PCQUEUE* pq) {
    assert (pq->qsize > 0);
    pq->tail++;
    if (pq->tail >= PCQSIZE)
        pq->tail = 0;
    pq->qsize--;
}

void receive (PCQUEUE *pq) {
    sem_wait(p_sem_not_empty);
    sem_wait(p_sem_lock);
    printf ("CONSUMER received message: <%s>\n", (char *) get_msg(pq));
    dispose_msg(pq);
    sem_post(p_sem_lock);
    sem_post(p_sem_not_full);
}

int main (int argc, char *argv[]) {
    if (argc < 2) {
        printf ("Usage: %s producer_binary_name\n", argv[0]);
        exit(1);
    }
    int shfd = shm_open (QNAME, O_CREAT|O_RDWR, 0600);
    if (shfd < 0) {
        perror("shm_open");
        exit(1);
    }
    if (ftruncate(shfd, sizeof(PCQUEUE)) == -1) {
        perror("ftruncate");
        exit(1);
    }
    // NULL means kernel shall choose an address for the mapping
    PCQUEUE *pq = (PCQUEUE*) mmap(NULL, sizeof(PCQUEUE), PROT_READ|PROT_WRITE,
        MAP_SHARED, shfd, 0);
    if (MAP_FAILED == pq) {
        perror("mmap");
        close(shfd);
        exit(1);
    }
    pq->head = 0;
    pq->tail = 0;
    pq->qsize = 0;
    // if old semaphores exist in an unknown state, remove them
    sem_unlink(p_queue_lock_name);
    sem_unlink(p_not_full_name);
    sem_unlink(p_not_empty_name);
    p_sem_lock = sem_open(p_queue_lock_name, O_CREAT, 0600, 1);
    p_sem_not_full = sem_open(p_not_full_name, O_CREAT, 0600, PCQSIZE);
    p_sem_not_empty = sem_open(p_not_empty_name, O_CREAT, 0600, 0);

    int pid = fork();
    if (pid == 0) {
        execl(argv[1], NULL);
        perror ("execl");
        exit (1);
    }
    // one receiver receives NUMBER_OF_MESSAGES
    for (int i = 0; i < NUMBER_OF_MESSAGES; i++)
        receive(pq);
    munmap(pq, PCQSIZE);
    printf ("CONSUMER EXITS\n");
    return 0;
}
