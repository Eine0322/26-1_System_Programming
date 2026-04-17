///////////////////////////////////////////////////////////////////////////////
// File Name : cli.c
// Date : 2026/04/10
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyun-ji
// Student ID : 2024402055
// --------------------------------------------------------------------
// Title : System Programming Assignment #1-3 (FTP client)
// Description : This program implements a simple FTP client that converts
//               user commands into FTP protocol commands
///////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SIZE 1024

void convert_command(int argc, char *argv[], char *buffer)
{
    if (argc < 2) {
        write(2, "Error: No command\n", 18);
        exit(1);
    }

    ///////////////////////////////////////////////////////////////////////
    // ls
    ///////////////////////////////////////////////////////////////////////
    if (strcmp(argv[1], "ls") == 0) {

        strcpy(buffer, "NLST");

        for (int i = 2; i < argc; i++) {
            strcat(buffer, " ");
            strcat(buffer, argv[i]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // dir
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "dir") == 0) {

        strcpy(buffer, "LIST");

        if (argc >= 3) {
            strcat(buffer, " ");
            strcat(buffer, argv[2]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // pwd
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "pwd") == 0) {

        if (argc > 2) {
            write(2, "Error: pwd does not take argument\n", 35);
            exit(1);
        }

        strcpy(buffer, "PWD");
    }

    ///////////////////////////////////////////////////////////////////////
    // cd
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "cd") == 0) {

        if (argc < 3) {
            write(2, "Error: No directory name\n", 25);
            exit(1);
        }

        if (argv[2][0] == '-') {
            write(2, "Error: invalid option\n", 22);
            exit(1);
        }

        if (strcmp(argv[2], "..") == 0) {
            strcpy(buffer, "CDUP");
        }
        else {
            strcpy(buffer, "CWD ");
            strcat(buffer, argv[2]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // mkdir
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "mkdir") == 0) {

        if (argc < 3) {
            write(2, "Error: No directory name\n", 25);
            exit(1);
        }

        strcpy(buffer, "MKD");

        for (int i = 2; i < argc; i++) {

            if (argv[i][0] == '-') {
                write(2, "Error: invalid option\n", 22);
                exit(1);
            }

            strcat(buffer, " ");
            strcat(buffer, argv[i]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // delete
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "delete") == 0) {

        if (argc < 3) {
            write(2, "Error: No file name\n", 20);
            exit(1);
        }

        strcpy(buffer, "DELE");

        for (int i = 2; i < argc; i++) {

            if (argv[i][0] == '-') {
                write(2, "Error: invalid option\n", 22);
                exit(1);
            }

            strcat(buffer, " ");
            strcat(buffer, argv[i]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // rmdir
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "rmdir") == 0) {

        if (argc < 3) {
            write(2, "Error: No directory name\n", 25);
            exit(1);
        }

        strcpy(buffer, "RMD");

        for (int i = 2; i < argc; i++) {

            if (argv[i][0] == '-') {
                write(2, "Error: invalid option\n", 22);
                exit(1);
            }

            strcat(buffer, " ");
            strcat(buffer, argv[i]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // rename
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "rename") == 0) {

        if (argc != 4) {
            write(2, "Error: usage rename <old> <new>\n", 32);
            exit(1);
        }

        strcpy(buffer, "RNFR ");
        strcat(buffer, argv[2]);
        strcat(buffer, " RNTO ");
        strcat(buffer, argv[3]);
    }

    ///////////////////////////////////////////////////////////////////////
    // get
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "get") == 0) {

        if (argc < 3) {
            write(2, "Error: No file name\n", 20);
            exit(1);
        }

        strcpy(buffer, "RETR ");
        strcat(buffer, argv[2]);
    }

    ///////////////////////////////////////////////////////////////////////
    // put
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "put") == 0) {

        if (argc < 3) {
            write(2, "Error: No file name\n", 20);
            exit(1);
        }

        strcpy(buffer, "STOR ");
        strcat(buffer, argv[2]);
    }

    ///////////////////////////////////////////////////////////////////////
    // quit
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "quit") == 0) {

        if (argc > 2) {
            write(2, "Error: quit does not take argument\n", 35);
            exit(1);
        }

        strcpy(buffer, "QUIT");
    }

    ///////////////////////////////////////////////////////////////////////
    // unknown
    ///////////////////////////////////////////////////////////////////////
    else {
        write(2, "Error: Unknown command\n", 23);
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    char buffer[BUF_SIZE] = {0};

    convert_command(argc, argv, buffer);

    write(STDOUT_FILENO, buffer, strlen(buffer));

    return 0;
}