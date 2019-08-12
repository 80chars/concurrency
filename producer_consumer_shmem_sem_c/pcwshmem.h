#ifndef PCWSHMEM
#define PCWSHMEM
#define PCQSIZE 3
#define MAX_LEN 50
#define NUMBER_OF_MESSAGES 1000
const static char *QNAME = "/CQUEUE";
const static char *p_queue_lock_name = "pcqueue_lock";
const static char *p_not_empty_name = "pcqueue_not_empty";
const static char *p_not_full_name = "pcqueue_not_full";


typedef struct{char chrs [MAX_LEN];} MESSAGE_T;

typedef struct {
    unsigned int qsize;
    unsigned int head;
    unsigned int tail;
    MESSAGE_T buffer[PCQSIZE];
} PCQUEUE;

#endif
