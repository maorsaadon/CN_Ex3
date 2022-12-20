CC = gcc -c
CFLAGS = -Wall -g

all: sender receiver

receiver: Receiver.o
	gcc $(CFLAGS) Receiver.o -o Receiver

sender: Sender.o
	gcc $(CFLAGS) Sender.o -o Sender

Receiver.o: Receiver.c
	$(CC) $(CFLAGS) Receiver.c

Sender.o: Sender.c
	$(CC) $(CFLAGS) Sender.c

clean:
	rm -f *.o Sender Receiver