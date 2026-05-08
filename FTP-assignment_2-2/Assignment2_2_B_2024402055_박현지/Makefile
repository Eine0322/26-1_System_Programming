CC = gcc
CFLAGS = -Wall -g

TARGET = cli srv

all: $(TARGET)

cli: cli.c
	$(CC) $(CFLAGS) -o cli cli.c

srv: srv.c
	$(CC) $(CFLAGS) -o srv srv.c

clean:
	rm -f $(TARGET)