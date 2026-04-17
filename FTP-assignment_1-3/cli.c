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

///////////////////////////////////////////////////////////////////////////////////////
// convert_command
// ===================================================================
// Input: int argc -> number of input arguments
//        char *argv[] -> user command arguments
//        char *buffer -> buffer to store FTP command
// Output: void
// Purpose: convert user command into corresponding FTP protocol command
///////////////////////////////////////////////////////////////////////////////////////
void convert_command(int argc, char *argv[], char *buffer)
{
    // ===================== argument validation =====================
    if (argc < 2) {
        write(2, "Error: No command\n", 18);
        exit(1);
    }

    ///////////////////////////////////////////////////////////////////////
    // ls → NLST
    ///////////////////////////////////////////////////////////////////////
    if (strcmp(argv[1], "ls") == 0) {

        strcpy(buffer, "NLST");

        // append options and path if exist
        if (argc >= 3) {
            for (int i = 2; i < argc; i++) {
                strcat(buffer, " ");
                strcat(buffer, argv[i]);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // dir → LIST
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "dir") == 0) {

        strcpy(buffer, "LIST");

        // append argument (directory path)
        if (argc >= 3) {
            strcat(buffer, " ");
            strcat(buffer, argv[2]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // pwd → PWD
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "pwd") == 0) {
        strcpy(buffer, "PWD");
    }

    ///////////////////////////////////////////////////////////////////////
    // cd → CWD / CDUP
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "cd") == 0) {

        if (argc < 3) {
            write(2, "Error: No directory name\n", 25);
            exit(1);
        }

        // cd ..
        if (strcmp(argv[2], "..") == 0) {
            strcpy(buffer, "CDUP");
        }
        // cd <dir>
        else {
            strcpy(buffer, "CWD ");
            strcat(buffer, argv[2]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // mkdir → MKD
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "mkdir") == 0) {

        if (argc < 3) {
            write(2, "Error: No directory name\n", 25);
            exit(1);
        }

        strcpy(buffer, "MKD");

        // support multiple directory names
        for (int i = 2; i < argc; i++) {
            strcat(buffer, " ");
            strcat(buffer, argv[i]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // delete → DELE
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "delete") == 0) {

        if (argc < 3) {
            write(2, "Error: No file name\n", 20);
            exit(1);
        }

        strcpy(buffer, "DELE");

        // support multiple file names
        for (int i = 2; i < argc; i++) {
            strcat(buffer, " ");
            strcat(buffer, argv[i]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // rmdir → RMD
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "rmdir") == 0) {

        if (argc < 3) {
            write(2, "Error: No directory name\n", 25);
            exit(1);
        }

        strcpy(buffer, "RMD");

        // support multiple directory names
        for (int i = 2; i < argc; i++) {
            strcat(buffer, " ");
            strcat(buffer, argv[i]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // rename → RNFR / RNTO
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "rename") == 0) {

        if (argc < 4) {
            write(2, "Error: usage rename <old> <new>\n", 32);
            exit(1);
        }

        // RNFR old RNTO new
        strcpy(buffer, "RNFR ");
        strcat(buffer, argv[2]);
        strcat(buffer, " RNTO ");
        strcat(buffer, argv[3]);
    }

    ///////////////////////////////////////////////////////////////////////
    // get → RETR
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
    // put → STOR
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
    // quit → QUIT
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "quit") == 0) {
        strcpy(buffer, "QUIT");
    }

    ///////////////////////////////////////////////////////////////////////
    // unknown command
    ///////////////////////////////////////////////////////////////////////
    else {
        write(2, "Error: Unknown command\n", 23);
        exit(1);
    }
}

///////////////////////////////////////////////////////////////////////////////////////
// main
// ===================================================================
// Input: int argc, char *argv[]
// Output: int
// Purpose: convert command and send to server (stdout)
///////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    char buffer[BUF_SIZE] = {0}; // buffer for FTP command

    // -------- convert command --------
    convert_command(argc, argv, buffer);

    // -------- send command --------
    write(STDOUT_FILENO, buffer, strlen(buffer));

    return 0;
}