import java.util.LinkedList;
import java.util.Queue;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.util.concurrent.locks.ReadWriteLock;

class MTI {
    int size;
    LinkedList<String> thequeue;
    Object not_empty_queue;
    Object not_full_queue;
    ReadWriteLock lock;
    MTI(int size) {
        this.size = size;
        thequeue = new LinkedList<String> ();
        lock = new ReentrantReadWriteLock();
        not_empty_queue = new Object();
        not_full_queue = new Object();
    }

    public void produce (String msg) throws InterruptedException {
        synchronized(not_full_queue) {
            while (thequeue.size () >= size)
                not_full_queue.wait();
        }
        lock.writeLock().lock();
        thequeue.add(msg);
        lock.writeLock().unlock();
        synchronized(not_empty_queue) {
            not_empty_queue.notify();
        }
    }

    public String consume () throws InterruptedException {
        String res;
        synchronized(not_empty_queue) {
            while (thequeue.size () == 0)
                not_empty_queue.wait();
        }
        lock.readLock().lock();
        res = thequeue.getFirst();
        thequeue.removeFirst();
        lock.readLock().unlock();
        synchronized(not_full_queue) {
            not_full_queue.notify();
        }
        return res;
    }
}

class ProducerConsumerConstants {
    public static final int TOTAL_MSGS = 100;
    public static final int TOTAL_PRODS = 10;
}

class Consumer extends ProducerConsumerConstants implements Runnable {
    MTI mti;
    Consumer(MTI mti) {
        super();
        this.mti = mti;
    }
    @Override
    public void run () {
        for (int i = 0; i < TOTAL_MSGS; i++) {
            try {
                String msg = mti.consume();
                System.out.println("consuming " + msg);
            } catch (InterruptedException e) {
                System.out.println (e);
            }
        }
    }
}

class Producer extends ProducerConsumerConstants implements Runnable {
    MTI mti;
    int myid;
    Producer(MTI mti, int myid) {
        super();
        this.mti = mti;
        this.myid = myid;
    }
    @Override
    public void run () {
        for (int i = 0; i < TOTAL_MSGS/TOTAL_PRODS; i++) {
            String msg = new String ("MSG: " + myid + " ^^ " + i);
            try {
                System.out.println("producing" + msg);
                mti.produce(msg);
            } catch (InterruptedException e) {
                System.out.println (e);
            }
        }
    }
}



class ProducerConsumer {
    public static void main (String[] args) {
        MTI mti = new MTI(10);
        Consumer c = new Consumer(mti);
        Thread consumerT = new Thread(c);
        consumerT.start();
        final int PRODS =  ProducerConsumerConstants.TOTAL_PRODS;
        Thread [] producers = new Thread[PRODS];
        int i = 0;
        while (i < PRODS) {
            producers[i] = new Thread(new Producer(mti, i+1));
            producers[i].start();
            i++;
        }
        try {
            consumerT.join();
            for (Thread t : producers) {
                t.join();
            }
        }
        catch (InterruptedException e) {
            System.out.println (e);
        }
    }
}
