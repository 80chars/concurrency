#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <semaphore.h>
#include "pcwshmem.h"

sem_t *p_sem_lock = NULL;
sem_t *p_sem_not_empty = NULL;
sem_t *p_sem_not_full = NULL;
static int prod_id = 0;
static const unsigned int number_of_procs = 10;

// copy message pmsg into the queue
void write_msg(PCQUEUE* pq, MESSAGE_T *pmsg) {
    assert(pq->qsize < PCQSIZE);
    memcpy(&pq->buffer[pq->head], pmsg, sizeof(MESSAGE_T));
    pq->head++;
    if (pq->head >= PCQSIZE)
        pq->head = 0;
    pq->qsize++;
}

// send pmsg using pq
void send_msg (PCQUEUE *pq, char *pmsg) {
    // wait until it is possible to write into the queue
    int rc = sem_wait(p_sem_not_full);
    if (rc != 0) {
        perror("sem_wait");
        exit(1);
    }
    
    sem_wait(p_sem_lock);
    MESSAGE_T mymsg;
    memset (&mymsg, 0, sizeof(mymsg));
    memcpy (&mymsg, pmsg, strlen(pmsg));
    printf ("PRODUCER: sending <%s>\n", pmsg);
    write_msg(pq, &mymsg);
    sem_post(p_sem_lock);
    // singnal the consumer(s) that data are available.
    sem_post(p_sem_not_empty);

}

int main () {
    int shfd = shm_open (QNAME, O_RDWR, 0);
    if (shfd < 0) {
        perror("shm_open");
        exit(1);
    }
    PCQUEUE *pq = (PCQUEUE*) mmap(NULL, sizeof(PCQUEUE),
        PROT_READ|PROT_WRITE,
        MAP_SHARED, shfd, 0);
    
    // two arguments in sem_open means "do not create the semaphore"
    p_sem_lock = sem_open(p_queue_lock_name, 0);
    p_sem_not_full = sem_open(p_not_full_name, 0);
    p_sem_not_empty = sem_open(p_not_empty_name, 0);
    pid_t pid = 0;
    while (prod_id++ < number_of_procs)
    {
        pid = fork();
        if (pid == 0)
            break;
    }
    if (pid == 0)
        for (int i = 0; i < NUMBER_OF_MESSAGES/number_of_procs; i++)
        {
            char buff[MAX_LEN];
            sprintf (buff, "message from %d, id %d\n", prod_id, i);
            printf ("producer %d is sending %d\n", prod_id, i);
            send_msg(pq, buff);
        }
    else
        wait(NULL);
    munmap(pq, PCQSIZE);
    if (pid == 0)
        printf ("PRODUCER process number %d exit\n", prod_id);
    return 0;
}
