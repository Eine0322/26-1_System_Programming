///////////////////////////////////////////////////////////////////////////////
// File Name : cli.c
// Date : 2026/04/18
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

///////////////////////////////////////////////////////////////////////////////
// convert_command                                                           //
// ========================================================================= //
// Input: int    -> Number of command line arguments                         //
//        char** -> Array of command line argument strings                   //
//        char* -> Buffer to store the converted FTP command                //
// Output: void                                                              //
// Purpose: Converts user-friendly shell commands into FTP protocol commands //
///////////////////////////////////////////////////////////////////////////////
void convert_command(int argc, char *argv[], char *buffer)
{
    ////////////////////////////// Check Argument /////////////////////////////
    if (argc < 2) {
        write(2, "Error: No command\n", 18); // Write error to stderr
        exit(1);
    }
    /////////////////////////// End of Check Argument /////////////////////////

    ///////////////////////////////////////////////////////////////////////
    // ls
    ///////////////////////////////////////////////////////////////////////
    if (strcmp(argv[1], "ls") == 0) {

        strcpy(buffer, "NLST"); // Convert 'ls' to 'NLST'

        //////////////////////////// Append Options ///////////////////////////
        for (int i = 2; i < argc; i++) {
            strcat(buffer, " ");
            strcat(buffer, argv[i]); // Append options or path
        }
        ///////////////////////// End of Append Options ///////////////////////
    }

    ///////////////////////////////////////////////////////////////////////
    // dir
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "dir") == 0) {

        strcpy(buffer, "LIST"); // Convert 'dir' to 'LIST'

        if (argc >= 3) {
            strcat(buffer, " ");
            strcat(buffer, argv[2]); // Append target directory if exists
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

        strcpy(buffer, "PWD"); // Convert 'pwd' to 'PWD'
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

        //////////////////////////// Path Conversion //////////////////////////
        if (strcmp(argv[2], "..") == 0) {
            strcpy(buffer, "CDUP"); // Use CDUP for parent directory
        }
        else {
            strcpy(buffer, "CWD "); // Use CWD for specific directory
            strcat(buffer, argv[2]);
        }
        ///////////////////////// End of Path Conversion //////////////////////
    }

    ///////////////////////////////////////////////////////////////////////
    // mkdir
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "mkdir") == 0) {

        if (argc < 3) {
            write(2, "Error: No directory name\n", 25);
            exit(1);
        }

        strcpy(buffer, "MKD"); // Convert 'mkdir' to 'MKD'

        //////////////////////////// Handle Arguments /////////////////////////
        for (int i = 2; i < argc; i++) {

            if (argv[i][0] == '-') {
                write(2, "Error: invalid option\n", 22);
                exit(1);
            }

            strcat(buffer, " ");
            strcat(buffer, argv[i]); // Add directory names to buffer
        }
        ///////////////////////// End of Handle Arguments /////////////////////
    }

    ///////////////////////////////////////////////////////////////////////
    // delete
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "delete") == 0) {

        if (argc < 3) {
            write(2, "Error: No file name\n", 20);
            exit(1);
        }

        strcpy(buffer, "DELE"); // Convert 'delete' to 'DELE'

        for (int i = 2; i < argc; i++) {

            if (argv[i][0] == '-') {
                write(2, "Error: invalid option\n", 22);
                exit(1);
            }

            strcat(buffer, " ");
            strcat(buffer, argv[i]); // Add filenames to delete
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

        strcpy(buffer, "RMD"); // Convert 'rmdir' to 'RMD'

        for (int i = 2; i < argc; i++) {

            if (argv[i][0] == '-') {
                write(2, "Error: invalid option\n", 22);
                exit(1);
            }

            strcat(buffer, " ");
            strcat(buffer, argv[i]); // Add directory names to remove
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

        //////////////////////////// Format Rename ////////////////////////////
        strcpy(buffer, "RNFR "); // Rename From
        strcat(buffer, argv[2]);
        strcat(buffer, " RNTO "); // Rename To
        strcat(buffer, argv[3]);
        ///////////////////////// End of Format Rename ////////////////////////
    }

    ///////////////////////////////////////////////////////////////////////
    // get
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(argv[1], "get") == 0) {

        if (argc < 3) {
            write(2, "Error: No file name\n", 20);
            exit(1);
        }

        strcpy(buffer, "RETR "); // Retrieve file from server
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

        strcpy(buffer, "STOR "); // Store file to server
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

        strcpy(buffer, "QUIT"); // Terminate session
    }

    ///////////////////////////////////////////////////////////////////////
    // unknown
    ///////////////////////////////////////////////////////////////////////
    else {
        write(2, "Error: Unknown command\n", 23);
        exit(1);
    }
}

///////////////////////////////////////////////////////////////////////////////
// main                                                                      //
// ========================================================================= //
// Input: int    -> Argument count                                           //
//        char** -> Argument vector                                          //
// Output: int   -> Returns 0 on success                                     //
// Purpose: Entry point for the client to translate and output FTP command   //
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    char buffer[BUF_SIZE] = {0}; // Initialize buffer with zeros

    /////////////////////////// Command Process ///////////////////////////////
    convert_command(argc, argv, buffer); // Process user input

    write(STDOUT_FILENO, buffer, strlen(buffer)); // Output the protocol command
    //////////////////////// End of Command Process ///////////////////////////

    return 0;
}