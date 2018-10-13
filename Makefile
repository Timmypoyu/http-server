CC = gcc
CXX = g++

CFLAGS = -g -Wall
CXXFLAGS = -g -Wall
LDGLAGS = -g

http-server: http-server.c 

.PHONY: clean
clean: 
	rm -f *.o *~ a.out core http-server 

.PHONY: all
all: clean http-server

