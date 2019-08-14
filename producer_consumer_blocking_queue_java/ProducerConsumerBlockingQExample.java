import java.util.Random;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

class PCConstants {
    final public static int NUMBER_OF_PRODUCERS = 10;
    final public static int NUMBER_OF_MESSAGES = 9000;
}

class Producer extends PCConstants implements Runnable {
    BlockingQueue<String> queue = null;
    int prod_id;

    public Producer(BlockingQueue<String> queue, int prod_id) {
        this.queue = queue;
        this.prod_id = prod_id;
    }

    public void run() {
        Random rand = new Random(); 
        try {
            for (int i = 0; i < NUMBER_OF_MESSAGES/NUMBER_OF_PRODUCERS; i++) {
                String msg = new String ("msg " + prod_id + " ^^ " + i);
                queue.put(msg);
                Thread.sleep(rand.nextInt(3));
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}

class Consumer extends PCConstants implements Runnable {
    BlockingQueue<String> queue = null;
    int prod_id;

    public Consumer(BlockingQueue<String> queue) {
        this.queue = queue;
    }

    public void run() {
        try {
            for (int i = 0; i < NUMBER_OF_MESSAGES; i++) {
                String msg = queue.take();
                System.out.println("Consumer: " + msg);
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}



class ProducerConsumerBlockingQExample {
    public static void main (String [] s) {
        final int PCQUEUESIZE = 1000;
        BlockingQueue<String> queue = new ArrayBlockingQueue<String> (PCQUEUESIZE);
        Consumer c = new Consumer(queue);
        Thread consumer = new Thread(c);
        Thread [] producers = new Thread [PCConstants.NUMBER_OF_PRODUCERS];
        for (int i = 0; i < PCConstants.NUMBER_OF_PRODUCERS; i++){
            producers[i] = new Thread(new Producer(queue, i));
            producers[i].start();
        }
        try {
            consumer.start();
            consumer.join();
            for (Thread t : producers) {
                t.join();
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

    }

}
