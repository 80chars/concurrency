# to build, enter make all
# to run, enter ./consumer ./producer
# Linux only


all: producer consumer

obj = producer.o consumer.o

$(obj): pcwshmem.h

$(obj): %.o: %.c
	gcc -c $< -o $@

producer: producer.o
	gcc -o producer -g -lrt -lpthread producer.o
consumer: consumer.o
	gcc -o consumer -g -lrt -lpthread consumer.o 

clean:
	rm consumer producer $(obj)
