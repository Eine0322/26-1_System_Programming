###############################################################################
# File Name : Makefile
# Date : 2026/05/14
# OS : Ubuntu 20.04.6 LTS 64bits
# Author : Park Hyunji
# Student ID : 2024402055
# --------------------------------------------------------------------------- #
# Title : System Programming Assignment #2-3
# Description :
#   - Compile FTP client and server program
#   - Generate executable files cli and srv
###############################################################################

CC = gcc
CFLAGS = -Wall

all: cli srv

cli: cli.c
	$(CC) $(CFLAGS) -o cli cli.c

srv: srv.c
	$(CC) $(CFLAGS) -o srv srv.c

clean:
	rm -f cli srv