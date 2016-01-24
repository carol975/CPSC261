CC = gcc
CFLAGS = -g -std=c11 -Og

main: main.o implicit.o
	$(CC) $(CFLAGS) -o $@ main.o implicit.o -lm

clean:
	/bin/rm main main.o implicit.o

main.o: implicit.h
implicit.o: implicit.h
