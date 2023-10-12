CC = gcc
CFLAGS = -c -Wall

all: User_Management_Program

User_Management_Program: User_Management_Program.o
	$(CC) User_Management_Program.o -o User_Management_Program

User_Management_Program.o: User_Management_Program.c
	$(CC) $(CFLAGS) User_Management_Program.c

clean:
	rm -rf *.o User_Management_Program