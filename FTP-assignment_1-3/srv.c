///////////////////////////////////////////////////////////////////////////////
// File Name : srv.c
// Date : 2026/04/18
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyun-ji
// Student ID : 2024402055
// --------------------------------------------------------------------
// Title : System Programming Assignment #1-3 (FTP server)
// Description : This program implements a simple FTP server
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>      // Standard I/O functions
#include <unistd.h>     // System calls: read, write, close
#include <string.h>     // String handling: strcmp, strtok
#include <stdlib.h>     // General utilities
#include <dirent.h>     // Directory handling
#include <sys/stat.h>   // File status (stat)
#include <fcntl.h>      // File control options

#define BUF_SIZE 1024   // Buffer size definition

///////////////////////////////////////////////////////////////////////////////
// write_str
// ============================================================================
// Input: const char* str -> string to be printed
// Output: none
// Purpose: Write a string to standard output using system call
///////////////////////////////////////////////////////////////////////////////
void write_str(const char *str) {
    write(STDOUT_FILENO, str, strlen(str)); // Write string to stdout
}

///////////////////////////////////////////////////////////////////////////////
// execute_command
// ============================================================================
// Input: char* buffer -> command string input
// Output: none
// Purpose: Parse input command and execute corresponding FTP operation
///////////////////////////////////////////////////////////////////////////////
void execute_command(char *buffer)
{
    char *cmd;                  // Command token
    char *arg;                  // Argument token
    struct stat st;             // File status structure

    cmd = strtok(buffer, " ");  // Extract command
    if (cmd == NULL) return;    // Return if empty input
    arg = strtok(NULL, " ");    // Extract argument
    
    // Option validation (-a, -l only)
    if (arg != NULL){
        if (arg[0]=='-'){       // If option detected
            if (arg[1]!='a' && arg[1]!='l'){ // Invalid option check
                write_str("Error: invalid option\n");
                return;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // PWD : Print current working directory
    ///////////////////////////////////////////////////////////////////////
    if (strcmp(cmd, "PWD") == 0) {

        if (arg != NULL) { // PWD should not have arguments
            write_str("Error: argument is not required\n");
            return;
        }

        char cwd[BUF_SIZE]; // Buffer for current directory

        if (getcwd(cwd, sizeof(cwd)) == NULL) { // Get current directory
            write_str("Error: PWD error\n");
            return;
        }

        write_str(cwd);     // Print directory path
        write_str("\n");
    }

    ///////////////////////////////////////////////////////////////////////
    // CWD : Change working directory
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "CWD") == 0) {

        if (arg == NULL) { // Argument required
            write_str("Error: argument is required\n");
            return;
        }

        if (stat(arg, &st) == -1) { // Check existence
            write_str("Error: directory not found\n");
            return;
        }

        if (chdir(arg) == -1) { // Change directory
            write_str("Error: cannot access\n");
            return;
        }

        char cwd[BUF_SIZE];
        getcwd(cwd, sizeof(cwd)); // Get updated path
        write_str("CWD\t");
        write_str(arg);
        write_str("\n");
        write_str(cwd);
        write_str("\n");
    }

    ///////////////////////////////////////////////////////////////////////
    // CDUP : Move to parent directory
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "CDUP") == 0) {

        if (chdir("..") == -1) { // Move up
            write(2, "CDUP error\n", 11);
            return;
        }

        char cwd[BUF_SIZE];
        getcwd(cwd, sizeof(cwd)); // Get current path
        write_str("CDUP\n");
        write_str(cwd);
        write_str("\n");
    }

    ///////////////////////////////////////////////////////////////////////
    // MKD : Make directory
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "MKD") == 0) {

        if (arg == NULL) {
            write_str("Error: argument is required\n");
            return;
        }

        while (arg != NULL) { // Handle multiple arguments
            if (mkdir(arg, 0755) == -1) { // Create directory
                write_str("Error: cannot create directory: File exists\n");
            }
            else{
                write_str("MKD\t");
                write_str(arg);
                write_str("\n");
            }

            arg = strtok(NULL, " "); // Next argument
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // DELE : Delete file
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "DELE") == 0) {

        if (arg == NULL) {
            write_str("Error: argument is required\n");
            return;
        }

        while (arg != NULL) {

            if (stat(arg, &st) == -1) { // File existence check
                write_str("Error: file not found ");
                write_str(arg);
                write_str("\n");
            }
            else if (S_ISDIR(st.st_mode)) { // Prevent directory deletion
                write_str("Error: cannot delete directory ");
                write_str(arg);
                write_str("\n");
            }
            else {
                if (remove(arg) == -1) { // Remove file
                    write_str("Error: failed to remove ");
                    write_str(arg);
                    write_str("\n");
                }
                else{
                    write_str("DELE\t");
                    write_str(arg);
                    write_str("\n");
                }
            }

            arg = strtok(NULL, " "); // Next argument
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // RMD : Remove directory
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "RMD") == 0) {

        if (arg == NULL) {
            write_str("Error: argument is required\n");
            return;
        }

        while (arg != NULL) {
            if (stat(arg, &st) == -1) { // Check existence
                write_str("Error: directory not found ");
                write_str(arg);
                write_str("\n");
            }
            else if (!S_ISDIR(st.st_mode)) { // Ensure it's directory
                write_str("Error: not directory\n");
            }
            else {
                if (rmdir(arg) == -1) { // Remove directory
                    write_str("Error: failed to remove ");
                    write_str(arg);
                    write_str("\n");
                }
                else{
                    write_str("RMD\t");
                    write_str(arg);
                    write_str("\n");
                }
            }

            arg = strtok(NULL, " ");
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // RNFR / RNTO : Rename file
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "RNFR") == 0) {

        char *old = arg;                     // Original name
        char *rnto = strtok(NULL, " ");      // RNTO keyword
        char *new = strtok(NULL, " ");       // New name

        if (old == NULL || rnto == NULL || new == NULL || strcmp(rnto, "RNTO") != 0) {
            write_str("Error: two arguments are required\n");
            return;
        }

        if (access(new, F_OK) == 0) { // Check if new file exists
            write_str("Error: name to change already exists\n");
            return;
        }

        if (rename(old, new) == -1) { // Rename operation
            write_str("Error: rename error\n");
        }
        else{
            write_str("RNFR\t");
            write_str(old);
            write_str("\n");
            write_str("RNTO\t");
            write_str(new);
            write_str("\n");
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // RETR : Download file (server -> client)
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "RETR") == 0) {

        if (arg == NULL) {
            write_str("Error: argument is required\n");
            return;
        }

        char src[BUF_SIZE], dst[BUF_SIZE];
        snprintf(src, sizeof(src), "server_root/%s", arg);   // Source path
        snprintf(dst, sizeof(dst), "client_root/%s", arg);   // Destination path

        int fd_src = open(src, O_RDONLY); // Open source file
        if (fd_src < 0) {
            write_str("Error: does not exist in server root\n");
            return;
        }

        int fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Create destination
        if (fd_dst < 0) {
            write_str("Error: cannot create file\n");
            close(fd_src);
            return;
        }

        char buf[BUF_SIZE];
        int n;
        int total=0; // Total bytes copied

        while ((n = read(fd_src, buf, BUF_SIZE)) > 0) { // Read loop
            write(fd_dst, buf, n);  // Write to destination
            total += n;             // Accumulate bytes
        }

        close(fd_src); // Close source
        close(fd_dst); // Close destination

        write_str("RETR\t");
        write_str(arg);
        write_str("\n");

        char msg[BUF_SIZE];
        snprintf(msg, sizeof(msg),
                "OK : %d bytes copied to client_root (expected %d bytes)\n",
                total, total); // Result message
        write_str(msg);
    }

    ///////////////////////////////////////////////////////////////////////
    // STOR : Upload file (client -> server)
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "STOR") == 0) {

        if (arg == NULL) {
            write_str("Error: argument is required\n");
            return;
        }

        char src[BUF_SIZE], dst[BUF_SIZE];
        snprintf(src, sizeof(src), "client_root/%s", arg);   // Source path
        snprintf(dst, sizeof(dst), "server_root/%s", arg);   // Destination path

        int fd_src = open(src, O_RDONLY); // Open source
        if (fd_src < 0) {
            write_str("Error: does not exist in client root\n");
            return;
        }

        int fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Open destination
        if (fd_dst < 0) {
            write(2, "Error: cannot create file\n", 26);
            close(fd_src);
            return;
        }

        char buf[BUF_SIZE];
        int n;
        int total=0; // Total bytes copied

        while ((n = read(fd_src, buf, BUF_SIZE)) > 0) {
            write(fd_dst, buf, n);
            total += n;
        }

        close(fd_src);
        close(fd_dst);

        write_str("STOR\t");
        write_str(arg);
        write_str("\n");

        char msg[BUF_SIZE];
        snprintf(msg, sizeof(msg),
                "OK : %d bytes copied to server_root (expected %d bytes)\n",
                total, total);
        write_str(msg);
    }

    ///////////////////////////////////////////////////////////////////////
    // QUIT : Terminate server
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "QUIT") == 0) {

        if (arg != NULL) {
            write_str("Error: argument is not required\n");
            return;
        }

        write_str("QUIT success\n"); // Print success
        exit(0); // Exit program
    }

    else {
        write_str("Unknown command\n"); // Invalid command
    }
}

///////////////////////////////////////////////////////////////////////////////
// main
// ============================================================================
// Input: none
// Output: int (exit status)
// Purpose: Read user input continuously and execute commands
///////////////////////////////////////////////////////////////////////////////
int main()
{
    char buffer[BUF_SIZE]; // Input buffer

    while (1) { // Infinite loop
        memset(buffer, 0, BUF_SIZE); // Initialize buffer

        int n = read(STDIN_FILENO, buffer, BUF_SIZE); // Read input
        if (n == 0) break;  // EOF → exit loop
        if (n < 0) continue; // Error → retry

        buffer[n] = '\0'; // Null terminate string
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline

        execute_command(buffer); // Execute command
    }

    return 0; // Program end
}