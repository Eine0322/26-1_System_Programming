///////////////////////////////////////////////////////////////////////////////
// File Name : cli.c
// Date : 2026/04/10
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyun-ji
// Student ID : 2024402055
// --------------------------------------------------------------------
// Title : System Programming Assignment #1-3 (FTP client)
// Description : This program implements a simple FTP client that converts user commands
//               into corresponding FTP protocol commands. It handles various commands
///////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SIZE 1024

///////////////////////////////////////////////////////////////////////////////////////
// convert_command
// ===================================================================
// Input: int argc -> number of input arguments
//        char *argv[] -> array of input arguments (user command)
//        char *buffer -> buffer to store the converted FTP command
// Output: void -> the converted command is stored in the buffer
// Purpose: To convert user commands into corresponding FTP protocol commands
///////////////////////////////////////////////////////////////////////////////////////
void convert_command(int argc, char *argv[], char *buffer)
{
    // Check if any command is provided
    if (argc < 2) {
        write(2, "Error: No command\n", 18);
        exit(1);
    }

    // ls
    if (strcmp(argv[1], "ls") == 0) {
        strcpy(buffer, "NLST");
        if (argc >= 3) {
            strcat(buffer, " ");
            strcat(buffer, argv[2]);
        }
    }

    // dir
    else if (strcmp(argv[1], "dir") == 0) {
        strcpy(buffer, "LIST");
        if (argc >= 3) {
            strcat(buffer, " ");
            strcat(buffer, argv[2]);
        }
    }

    // pwd
    else if (strcmp(argv[1], "pwd") == 0) {
        strcpy(buffer, "PWD");
    }

    // cd
    else if (strcmp(argv[1], "cd") == 0) {
        if (argc < 3) { // check if directory name is provided
            write(2, "Error: No directory name\n", 25);
            exit(1);
        }

        if (strcmp(argv[2], "..") == 0) { // handle cd .. case
            strcpy(buffer, "CDUP");
        } else { // handle cd <dir> case
            strcpy(buffer, "CWD ");
            strcat(buffer, argv[2]);
        }
    }

    // mkdir
    else if (strcmp(argv[1], "mkdir") == 0) {
        if (argc < 3) { // check if directory name is provided
            write(2, "Error: No directory name\n", 25);
            exit(1);
        }

        strcpy(buffer, "MKD");
        // concatenate all directory names if multiple are provided
        for (int i = 2; i < argc; i++) {
            strcat(buffer, " ");
            strcat(buffer, argv[i]);
        }
    }

    // delete
    else if (strcmp(argv[1], "delete") == 0) {
        if (argc < 3) { // check if file name is provided
            write(2, "Error: No file name\n", 20);
            exit(1);
        }

        strcpy(buffer, "DELE");
        // concatenate all file names if multiple are provided
        for (int i = 2; i < argc; i++) {
            strcat(buffer, " ");
            strcat(buffer, argv[i]);
        }
    }

    // rmdir
    else if (strcmp(argv[1], "rmdir") == 0) {
        if (argc < 3) { // check if directory name is provided
            write(2, "Error: No directory name\n", 25);
            exit(1);
        }

        strcpy(buffer, "RMD");
        // concatenate all directory names if multiple are provided
        for (int i = 2; i < argc; i++) {
            strcat(buffer, " ");
            strcat(buffer, argv[i]);
        }
    }

    // rename
    else if (strcmp(argv[1], "rename") == 0) {
        if (argc < 4) { // check if old and new file names are provided
            write(2, "Error: usage rename <old> <new>\n", 32);
            exit(1);
        }

        // RNFR old RNTO new
        strcpy(buffer, "RNFR ");
        strcat(buffer, argv[2]);
        strcat(buffer, " RNTO ");
        strcat(buffer, argv[3]);
    }

    // get
    else if (strcmp(argv[1], "get") == 0) {
        if (argc < 3) { // check if file name is provided
            write(2, "Error: No file name\n", 20);
            exit(1);
        }

        strcpy(buffer, "RETR ");
        strcat(buffer, argv[2]);
    }

    // put
    else if (strcmp(argv[1], "put") == 0) {
        if (argc < 3) { // check if file name is provided
            write(2, "Error: No file name\n", 20);
            exit(1);
        }

        strcpy(buffer, "STOR ");
        strcat(buffer, argv[2]);
    }

    // quit
    else if (strcmp(argv[1], "quit") == 0) {
        strcpy(buffer, "QUIT");
    }

    else {
        write(2, "Error: Unknown command\n", 23);
        exit(1);
    }
}

///////////////////////////////////////////////////////////////////////////////////////
// main
// ===================================================================
// Input: int argc -> number of input arguments
//        char *argv[] -> array of input arguments (user command)
// Output: int -> returns 0 when program terminates normally
// Purpose: To process user commands and communicate with the FTP server
///////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    char buffer[BUF_SIZE] = {0}; // buffer to store the converted FTP command

    convert_command(argc, argv, buffer); // convert user command to FTP command

    write(STDOUT_FILENO, buffer, strlen(buffer)); // print the converted command to standard output

    return 0;
}