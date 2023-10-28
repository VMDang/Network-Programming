CFLAGS = -c -Wall -lcrypto -lssl
CC = gcc
LIB = -lm

all: client server

debug: CFLAGS += -g
debug: client server

client: client.o
	${CC} client.o -o client -lcrypto -lssl

client.o: client.c
	$(CC) $(CFLAGS) client.c

server: server.o
	${CC} server.c  -o server -lcrypto -lssl

server.o: server.c
	${CC} $(CFLAGS) server.c

clean:
	rm -f *.o *~