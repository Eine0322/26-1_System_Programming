CC = gcc
CFLAGS = -Wall

all: cli srv

cli: cli.c
	$(CC) $(CFLAGS) -o cli cli.c

srv: srv.c
	$(CC) $(CFLAGS) -o srv srv.c

clean:
	rm -f cli srv