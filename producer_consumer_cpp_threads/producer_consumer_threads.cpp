// classic producer-consumer problem solution using C++11 condition variables.
// The producer-consumer queue size is limited.
// One producer and ten consumers access a queue.
// Each producer generates a total of10 messages,
// and the consumer consumes all of them.
// Random sleep time is used to make execution less deterministic.
// Build instructions:
// g++ -o producer_consumer_threads -std=c++11 producer_consumer_threads.cpp

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <thread>

using namespace std;

static const uint32_t MAX_QUEUE_SIZE = 5;
static const uint32_t MAX_MESSAGE_SIZE = 100;
static const uint32_t NUMBER_OF_PRODUCERS = 10;

static uint32_t items_consumed = 0;
static uint32_t items_produced = 0;

typedef struct {char buffer[MAX_MESSAGE_SIZE];} QUEUE_EL_T;

queue<QUEUE_EL_T> pcqueue;
mutex rwlock;
condition_variable cv_queue_not_full;
condition_variable cv_queue_not_empty;
mutex mtx_queue_not_full;
mutex mtx_queue_not_empty;
// correctness verification
static uint32_t number_of_elements = 0;

// Let each message has this header.
// Nothing else shall be sent/received but the header.
struct MSG {
    int pid;
    int mid;
};

void producer (int pr_id)
{
    random_device rd;
    mt19937 mt(rd()); // random engige
    uniform_int_distribution<int> dist(0, 10); // distribution
    for (int i = 0; i < 10; i++) {
        // until the queue is full wait for the consumer to free queue elements
        unique_lock<mutex> lock_queue_not_full(mtx_queue_not_full);
        while (pcqueue.size() >= MAX_QUEUE_SIZE) {
            cv_queue_not_full.wait(lock_queue_not_full);
            // sleep 0 - 10 ms
            this_thread::sleep_for (chrono::milliseconds(dist(mt)));
        }
        // get the lock for the queue
        // and produce an item
        rwlock.lock();
        QUEUE_EL_T el;
        MSG *pmsg = (MSG *) &el;
        pmsg->pid = pr_id;
        pmsg->mid = i;
        pcqueue.push(el);
        items_produced++;
        number_of_elements++;
        // check the invariant
        assert(number_of_elements <= MAX_QUEUE_SIZE);
        // Queue updates are done. Other threads can access the queue. Release the lock.
        rwlock.unlock();
        // In case the consumer was waiting on empty queue,
        // notify the consumer because the queue is not empty anymore
        cv_queue_not_empty.notify_all();
    }
}

void consumer ()
{
    random_device rd;
    mt19937 mt(rd());
    uniform_int_distribution<int> dist(0, 10);
    for (int i = 0; i < 100; i++) {
        unique_lock<mutex> lock_queue_not_empty(mtx_queue_not_empty);
        // wait for producers to produce
        while (pcqueue.size() == 0) {
            cv_queue_not_empty.wait(lock_queue_not_empty);
            this_thread::sleep_for (chrono::milliseconds(dist(mt)));
        }
        // read what was produced
        rwlock.lock();
        // read just one "message". It is possible to read all of them, of course.
        QUEUE_EL_T el = pcqueue.front();
        MSG *pmsg = (MSG *) &el;
        cout << "consumed: prod_id " << pmsg->pid << " mid " << pmsg->mid << " queue size " << pcqueue.size() << endl;
        pcqueue.pop();
        items_consumed++;
        number_of_elements--;
        assert(number_of_elements <= MAX_QUEUE_SIZE);
        rwlock.unlock();
        // notify producers if they were waiting on the full queue
        cv_queue_not_full.notify_all();
    }
    cout << "CONSUMER HAS RETIRED" << endl;   
}

main ()
{
    thread consumer_thread(consumer);
    thread *producers[NUMBER_OF_PRODUCERS];
    for (int i = 0; i < NUMBER_OF_PRODUCERS; i++) {
        producers[i] = new thread (producer, i);
    }
    consumer_thread.join();
    for (auto prod : producers)
        prod->join();
    assert(items_consumed == items_produced);
}
