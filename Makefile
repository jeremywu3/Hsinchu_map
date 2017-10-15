#!/usr/bin/make -f

CC=gcc
LDLIBS=-lX11 -lmysqlclient
CFLAGS=-std=c99 -Wall -Wextra -pedantic -O2


main    : src/def.o src/UI.o src/query.o
	$(CC) 'src/def.o' 'src/UI.o' 'src/query.o' -o Hsinchu-Map $(LDLIBS)

def.o   : src/def.c
	$(CC) $(CFLAGS) -c 'src/def.c'

UI.o    : src/UI.c
	$(CC) $(CFLAGS) -c 'src/UI.c'

query.o : src/query.c
	$(CC) $(CFLAGS) -c 'src/query.c'

clean   :
	rm -f "src/*.o"
