###############################################################################
# Makefile for assignment 3, Parallel and Distributed Computing 2020.
###############################################################################

CC = mpicc
CFLAGS = -g -O3
LIBS = -lm

BIN = ssnew

all: $(BIN)

ssnew: shearSortNew.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $<

clean:
	$(RM) $(BIN)
