CC = gcc -c
CFLAGS = -wall -g

all: sender receiver

receiver: Receiver.o
	gcc -Wall -g Receiver.o -o Receiver

sender: Sender.o
	gcc -Wall -g Sender.o -o Sender

Receiver.o: Receiver.c
	$(cc) $(CFLAGS) Receiver.c

Sender.o: Sender.c
	$(cc) $(CFLAGS) Sender.c

clean:
	rm -f *.o Sender Receiver