CC = gcc
CFLAGS = -c -Wall -I/usr/include/libxml2
LIBS = -lcurl -lxml2

all: resolver

resolver: resolver.o
	$(CC) $(LDFLAGS) resolver.o -o resolver $(LIBS) $(LIBXML2_LIB)

resolver.o: resolver.c
	$(CC) $(CFLAGS) $(LIBXML2_INCLUDE) resolver.c

clean:
	rm -rf *.o resolver *~
