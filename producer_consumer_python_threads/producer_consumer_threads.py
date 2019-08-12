# Classic producer-consumer problem solution using condition variables.
# one producer and ten consumers access a queue.
# Producers generate a total of 100 messages each,
# and the consumer consumes all of them.
# Random sleep time is used to make execution less deterministic.

import threading
import collections
from random import randint
from time import sleep

MAX_QUEUE_SIZE=50
NUMBER_OF_PRODUCERS=10
ITEMS_TO_CONSUME=1000

# Multi-thread Interface
class MTI:
    def __init__ (self, size):
        self.size = size
        # signal consumer
        self.cv_queue_is_not_empty = threading.Condition()
        # signal producer
        self.cv_queue_is_not_full = threading.Condition()
        # lock the queue (not really required)
        self.rwlock = threading.Lock()
        # note deque operations are thread safe
        self.thequeue = collections.deque([""], size)
        self.thequeue.clear()

    def consume(self):
        self.cv_queue_is_not_empty.acquire()
        while len(self.thequeue) == 0:
            self.cv_queue_is_not_empty.wait()
        # deque.pop is thread is thread safe, but let's assume it is not.
        # "To make pop thread safe" rwlock is used
        self.rwlock.acquire()
        res=self.thequeue.pop()
        self.rwlock.release()
        self.cv_queue_is_not_empty.release()
        self.cv_queue_is_not_full.acquire()
        self.cv_queue_is_not_full.notify()
        self.cv_queue_is_not_full.release()
        return res
        
    def produce(self, what, idd):
        # only one producer at a time can enter
        self.cv_queue_is_not_full.acquire()
        # wait for free space in the queue
        while self.size <= len(self.thequeue):
            self.cv_queue_is_not_full.wait()
        #print("PRODUCER release lock" + idd + "number of els " + str(self.count) + " qlen " + str(len(self.thequeue)))
        # deque.append is thread safe, but let's assume it is not...
        # acquire exclusive access to the queue;
        # privent the consumer from accessing the queue.
        self.rwlock.acquire()
        self.thequeue.append(what)
        # The item has been produced, allow the consumer to access
        self.rwlock.release()
        # allow the other producers to enter
        self.cv_queue_is_not_full.release()
        # notify the consumer
        self.cv_queue_is_not_empty.acquire()
        self.cv_queue_is_not_empty.notify()
        self.cv_queue_is_not_empty.release()


def producer (pr_id, mti):
    ctr=1
    while ctr<=ITEMS_TO_CONSUME/NUMBER_OF_PRODUCERS:
         mti.produce("abc" + str((pr_id + 1) * 100000 + ctr), str(pr_id))
         ctr=ctr+1
    return

def consumer (mti):
    ctr=0
    all_msg=[]
    while ctr<ITEMS_TO_CONSUME:
        msg=mti.consume()
        all_msg.append(msg)
        sleep(randint(0,100)/10000)
        ctr+=1
    print(sorted(all_msg))
    return

if __name__ == "__main__":
    mti = MTI(MAX_QUEUE_SIZE)
    consumer_thread = threading.Thread(target=consumer, args=(mti,))
    producers = []
    i = 0
    while i < NUMBER_OF_PRODUCERS:
        producers.append(threading.Thread(target=producer, args=(i, mti)))
        i+=1

    consumer_thread.start()
    for prod in producers:
        prod.start();
    consumer_thread.join();
    for prod in producers:
        prod.join();
