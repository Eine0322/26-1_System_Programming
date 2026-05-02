###############################################################################
# File Name : Makefile
# Date : 2026/05/02
# OS : Ubuntu 20.04.6 LTS 64bits
# Author : Your Name
# Student ID : Your ID
# -----------------------------------------------------------------------------
# Title : System Programming Assignment #2-1 (FTP Client & Server)
# Description :
#   - Compile cli.c and srv.c into executable files (cli, srv)
#   - Provide clean target to remove binaries
###############################################################################

############################## Compiler Setting ###############################
CC = gcc                # C compiler
CFLAGS = -Wall -g      # Warning flags and debug option

############################## Target Setting #################################
TARGET = cli srv       # Output executable files

############################## Default Target #################################
# "make" will execute this rule first
all: $(TARGET)

############################## Client Build ###################################
# Build cli from cli.c
cli: cli.c
	$(CC) $(CFLAGS) -o cli cli.c   # Compile client

############################## Server Build ###################################
# Build srv from srv.c
srv: srv.c
	$(CC) $(CFLAGS) -o srv srv.c   # Compile server

############################## Clean ##########################################
# Remove generated executable files
clean:
	rm -f cli srv