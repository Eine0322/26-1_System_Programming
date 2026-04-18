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

#include <unistd.h>     // System calls: read, write
#include <string.h>     // String handling functions
#include <stdlib.h>     // General utilities (exit)

#define BUF_SIZE 1024   // Buffer size definition

///////////////////////////////////////////////////////////////////////////////
// convert_command
// ============================================================================
// Input: int argc -> number of arguments
//        char* argv[] -> argument list from command line
//        char* buffer -> output buffer for FTP command
// Output: none
// Purpose: Convert user CLI command into corresponding FTP protocol command
///////////////////////////////////////////////////////////////////////////////
void convert_command(int argc, char *argv[], char *buffer)
{
    if (argc < 2) { // No command provided
        write(2, "Error: No command\n", 18);
        exit(1);
    }

    ///////////////////////////////////////////////////////////////////////
    // ls : convert to NLST
    ///////////////////////////////////////////////////////////////////////
    if (strcmp(argv[1], "ls") == 0) {

        strcpy(buffer, "NLST"); // Base FTP command

        for (int i = 2; i < argc; i++) { // Append options/arguments
            strcat(buffer, " ");
            strcat(buffer, argv[i]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // dir : convert to LIST
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "dir") == 0) {

        strcpy(buffer, "LIST"); // LIST command

        if (argc >= 3) { // Optional path argument
            strcat(buffer, " ");
            strcat(buffer, argv[2]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // pwd : convert to PWD
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "pwd") == 0) {

        if (argc > 2) { // pwd should not have arguments
            write(2, "Error: pwd does not take argument\n", 35);
            exit(1);
        }

        strcpy(buffer, "PWD"); // Set command
    }

    ///////////////////////////////////////////////////////////////////////
    // cd : convert to CWD or CDUP
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "cd") == 0) {

        if (argc < 3) { // Missing directory argument
            write(2, "Error: No directory name\n", 25);
            exit(1);
        }

        if (argv[2][0] == '-') { // Invalid option check
            write(2, "Error: invalid option\n", 22);
            exit(1);
        }

        if (strcmp(argv[2], "..") == 0) { // Parent directory
            strcpy(buffer, "CDUP");
        }
        else {
            strcpy(buffer, "CWD "); // Change directory command
            strcat(buffer, argv[2]);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // mkdir : convert to MKD
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "mkdir") == 0) {

        if (argc < 3) { // No directory name
            write(2, "Error: No directory name\n", 25);
            exit(1);
        }

        strcpy(buffer, "MKD"); // Base command

        for (int i = 2; i < argc; i++) {

            if (argv[i][0] == '-') { // Option not allowed
                write(2, "Error: invalid option\n", 22);
                exit(1);
            }

            strcat(buffer, " ");
            strcat(buffer, argv[i]); // Append directory names
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // delete : convert to DELE
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "delete") == 0) {

        if (argc < 3) { // No file name
            write(2, "Error: No file name\n", 20);
            exit(1);
        }

        strcpy(buffer, "DELE"); // Base command

        for (int i = 2; i < argc; i++) {

            if (argv[i][0] == '-') { // Invalid option
                write(2, "Error: invalid option\n", 22);
                exit(1);
            }

            strcat(buffer, " ");
            strcat(buffer, argv[i]); // Append file names
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // rmdir : convert to RMD
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "rmdir") == 0) {

        if (argc < 3) { // No directory name
            write(2, "Error: No directory name\n", 25);
            exit(1);
        }

        strcpy(buffer, "RMD"); // Base command

        for (int i = 2; i < argc; i++) {

            if (argv[i][0] == '-') { // Invalid option
                write(2, "Error: invalid option\n", 22);
                exit(1);
            }

            strcat(buffer, " ");
            strcat(buffer, argv[i]); // Append directory names
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // rename : convert to RNFR + RNTO
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "rename") == 0) {

        if (argc != 4) { // Must have exactly 2 arguments
            write(2, "Error: usage rename <old> <new>\n", 32);
            exit(1);
        }

        strcpy(buffer, "RNFR "); // Old file name
        strcat(buffer, argv[2]);
        strcat(buffer, " RNTO "); // New file name
        strcat(buffer, argv[3]);
    }

    ///////////////////////////////////////////////////////////////////////
    // get : convert to RETR
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "get") == 0) {

        if (argc < 3) { // No file name
            write(2, "Error: No file name\n", 20);
            exit(1);
        }

        strcpy(buffer, "RETR "); // Download command
        strcat(buffer, argv[2]);
    }

    ///////////////////////////////////////////////////////////////////////
    // put : convert to STOR
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "put") == 0) {

        if (argc < 3) { // No file name
            write(2, "Error: No file name\n", 20);
            exit(1);
        }

        strcpy(buffer, "STOR "); // Upload command
        strcat(buffer, argv[2]);
    }

    ///////////////////////////////////////////////////////////////////////
    // quit : convert to QUIT
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "quit") == 0) {

        if (argc > 2) { // quit should not have arguments
            write(2, "Error: quit does not take argument\n", 35);
            exit(1);
        }

        strcpy(buffer, "QUIT"); // Exit command
    }

    ///////////////////////////////////////////////////////////////////////
    // unknown command
    ///////////////////////////////////////////////////////////////////////
    else {
        write(2, "Error: Unknown command\n", 23);
        exit(1);
    }
}

///////////////////////////////////////////////////////////////////////////////
// main
// ============================================================================
// Input: int argc -> argument count
//        char* argv[] -> argument list
// Output: int (exit status)
// Purpose: Convert user input command and print FTP command to stdout
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    char buffer[BUF_SIZE] = {0}; // Initialize buffer

    convert_command(argc, argv, buffer); // Convert command

    write(STDOUT_FILENO, buffer, strlen(buffer)); // Output result

    return 0; // Program end
}