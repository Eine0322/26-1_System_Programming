///////////////////////////////////////////////////////////////////////////////
// File Name : cli.c
// Date : 2026/05/14
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyunji
// Student ID : 2024402055
// -------------------------------------------------------------------------- //
// Title : System Programming Assignment #2-3 (FTP Client)
// Description :
//   - Connect to FTP server by using socket() and connect()
//   - Convert user commands to FTP commands
//   - Send converted commands to the server
//   - Receive and display command results from the server
//   - Send QUIT command when quit or Ctrl+C is entered
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>

#define MAX_BUFF 1024
#define RCV_BUFF 8192

int g_sockfd = -1;                       // Global socket descriptor for SIGINT

///////////////////////////////////////////////////////////////////////////////
// send_all                                                                  //
// ========================================================================== //
// Input : int fd        -> socket file descriptor                            //
//         const char* b -> buffer to send                                     //
//         size_t len    -> buffer length                                      //
// Output: int        -> 0 success, -1 fail                                    //
// Purpose: Send all bytes in the buffer                                      //
///////////////////////////////////////////////////////////////////////////////
int send_all(int fd, const char *b, size_t len)
{
    size_t sent = 0;                     // Number of bytes already sent
    ssize_t n;                           // Number of bytes sent at one write

    while (sent < len) {
        n = write(fd, b + sent, len - sent); // Send remaining bytes
        if (n <= 0) {
            return -1;                   // Return error on write failure
        }
        sent += (size_t)n;               // Update sent byte count
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// sigint_handler                                                            //
// ========================================================================== //
// Input : int signo -> signal number                                         //
// Output: void                                                               //
// Purpose: Send QUIT command and terminate client when Ctrl+C is pressed      //
///////////////////////////////////////////////////////////////////////////////
void sigint_handler(int signo)
{
    char quit_msg[] = "QUIT";            // FTP quit command

    if (g_sockfd >= 0) {
        send_all(g_sockfd, quit_msg, strlen(quit_msg)); // Notify server
        close(g_sockfd);                 // Close connected socket
    }

    write(STDOUT_FILENO, "\nProgram quit!!\n", 16); // Print quit message
    exit(0);
}

///////////////////////////////////////////////////////////////////////////////
// append_remain_tokens                                                      //
// ========================================================================== //
// Input : char* cmd_buff -> FTP command buffer                               //
//         char* token    -> first remaining token                             //
// Output: void                                                               //
// Purpose: Append remaining arguments to converted command                    //
///////////////////////////////////////////////////////////////////////////////
void append_remain_tokens(char *cmd_buff, char *token)
{
    while (token != NULL) {
        strcat(cmd_buff, " ");           // Add blank before argument
        strcat(cmd_buff, token);         // Add argument token
        token = strtok(NULL, " \t\n");  // Get next token
    }
}

///////////////////////////////////////////////////////////////////////////////
// conv_cmd                                                                  //
// ========================================================================== //
// Input : char* buff     -> user input command                               //
//         char* cmd_buff -> buffer to store converted FTP command             //
// Output: int        -> 0 success, -1 fail                                    //
// Purpose: Convert user command to FTP command                               //
///////////////////////////////////////////////////////////////////////////////
int conv_cmd(char *buff, char *cmd_buff)
{
    char temp[MAX_BUFF];                 // Temporary buffer for tokenizing
    char *token;                         // Current token pointer
    char *arg1;                          // First argument pointer
    char *arg2;                          // Second argument pointer
    char *extra;                         // Extra argument pointer

    memset(temp, 0, sizeof(temp));        // Initialize temporary buffer
    strcpy(temp, buff);                  // Copy user input

    token = strtok(temp, " \t\n");       // Get command token
    if (token == NULL) {
        return -1;                       // Empty command is invalid
    }

    ////////////////////////////// ls command //////////////////////////////
    if (strcmp(token, "ls") == 0) {
        strcpy(cmd_buff, "NLST");        // Convert ls to NLST
        append_remain_tokens(cmd_buff, strtok(NULL, " \t\n")); // Append options/path
    }
    ////////////////////////////// pwd command /////////////////////////////
    else if (strcmp(token, "pwd") == 0) {
        if (strtok(NULL, " \t\n") != NULL) {
            return -1;                   // pwd does not need arguments
        }
        strcpy(cmd_buff, "PWD");         // Convert pwd to PWD
    }
    ////////////////////////////// dir command /////////////////////////////
    else if (strcmp(token, "dir") == 0) {
        strcpy(cmd_buff, "LIST");        // Convert dir to LIST
        append_remain_tokens(cmd_buff, strtok(NULL, " \t\n")); // Append path if exists
    }
    ////////////////////////////// cd command //////////////////////////////
    else if (strcmp(token, "cd") == 0) {
        arg1 = strtok(NULL, " \t\n");    // Get target path
        extra = strtok(NULL, " \t\n");   // Check extra argument
        if (arg1 == NULL || extra != NULL) {
            return -1;                   // cd needs exactly one argument
        }
        if (strcmp(arg1, "..") == 0) {
            strcpy(cmd_buff, "CDUP");    // Convert cd .. to CDUP
        }
        else {
            sprintf(cmd_buff, "CWD %s", arg1); // Convert cd path to CWD
        }
    }
    ////////////////////////////// mkdir command ///////////////////////////
    else if (strcmp(token, "mkdir") == 0) {
        strcpy(cmd_buff, "MKD");         // Convert mkdir to MKD
        arg1 = strtok(NULL, " \t\n");    // Get first directory name
        if (arg1 == NULL) {
            return -1;                   // mkdir needs at least one argument
        }
        append_remain_tokens(cmd_buff, arg1); // Append directory names
    }
    ////////////////////////////// delete command //////////////////////////
    else if (strcmp(token, "delete") == 0) {
        strcpy(cmd_buff, "DELE");        // Convert delete to DELE
        arg1 = strtok(NULL, " \t\n");    // Get first file name
        if (arg1 == NULL) {
            return -1;                   // delete needs at least one argument
        }
        append_remain_tokens(cmd_buff, arg1); // Append file names
    }
    ////////////////////////////// rmdir command ///////////////////////////
    else if (strcmp(token, "rmdir") == 0) {
        strcpy(cmd_buff, "RMD");         // Convert rmdir to RMD
        arg1 = strtok(NULL, " \t\n");    // Get first directory name
        if (arg1 == NULL) {
            return -1;                   // rmdir needs at least one argument
        }
        append_remain_tokens(cmd_buff, arg1); // Append directory names
    }
    ////////////////////////////// rename command //////////////////////////
    else if (strcmp(token, "rename") == 0) {
        arg1 = strtok(NULL, " \t\n");    // Get original name
        arg2 = strtok(NULL, " \t\n");    // Get new name
        extra = strtok(NULL, " \t\n");   // Check extra argument
        if (arg1 == NULL || arg2 == NULL || extra != NULL) {
            return -1;                   // rename needs exactly two arguments
        }
        sprintf(cmd_buff, "RNFR %s RNTO %s", arg1, arg2); // Convert rename
    }
    ////////////////////////////// quit command ////////////////////////////
    else if (strcmp(token, "quit") == 0) {
        if (strtok(NULL, " \t\n") != NULL) {
            return -1;                   // quit does not need arguments
        }
        strcpy(cmd_buff, "QUIT");        // Convert quit to QUIT
    }
    ////////////////////////////// invalid command /////////////////////////
    else {
        return -1;                       // Unknown command
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// process_result                                                            //
// ========================================================================== //
// Input : char* rcv_buff -> received result buffer                           //
// Output: void                                                               //
// Purpose: Display server response                                           //
///////////////////////////////////////////////////////////////////////////////
void process_result(char *rcv_buff)
{
    write(STDOUT_FILENO, rcv_buff, strlen(rcv_buff)); // Print received result
}

///////////////////////////////////////////////////////////////////////////////
// main                                                                      //
// ========================================================================== //
// Input : int argc    -> argument count                                      //
//         char** argv -> server IP and port                                  //
// Output: int        -> 0 success                                            //
// Purpose: Main routine of FTP client                                        //
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    int sockfd;                           // Socket file descriptor
    int n;                                // Number of bytes read
    char buff[MAX_BUFF];                  // User input buffer
    char cmd_buff[MAX_BUFF];              // Converted command buffer
    char rcv_buff[RCV_BUFF];              // Receive buffer
    struct sockaddr_in servaddr;          // Server address structure

    if (argc != 3) {
        write(STDERR_FILENO, "Usage: ./cli <IP> <PORT>\n", 25); // Print usage
        exit(1);
    }

    signal(SIGINT, sigint_handler);        // Register Ctrl+C handler

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (sockfd < 0) {
        perror("socket");                 // Print socket error
        exit(1);
    }
    g_sockfd = sockfd;                     // Save socket for signal handler

    memset(&servaddr, 0, sizeof(servaddr)); // Initialize server address
    servaddr.sin_family = AF_INET;          // Set IPv4 family
    servaddr.sin_port = htons(atoi(argv[2])); // Set server port
    servaddr.sin_addr.s_addr = inet_addr(argv[1]); // Set server IP

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect");                // Print connect error
        close(sockfd);                     // Close socket
        exit(1);
    }

    while (1) {
        memset(buff, 0, sizeof(buff));      // Clear input buffer
        memset(cmd_buff, 0, sizeof(cmd_buff)); // Clear command buffer
        memset(rcv_buff, 0, sizeof(rcv_buff)); // Clear receive buffer

        write(STDOUT_FILENO, "> ", 2);     // Print prompt
        n = read(STDIN_FILENO, buff, MAX_BUFF - 1); // Read user input
        if (n <= 0) {
            break;                         // Stop on input error or EOF
        }
        buff[n] = '\0';                    // Terminate input string

        if (conv_cmd(buff, cmd_buff) < 0) {
            write(STDERR_FILENO, "conv_cmd() error!!\n", 19); // Print conversion error
            continue;
        }

        if (send_all(sockfd, cmd_buff, strlen(cmd_buff)) < 0) {
            perror("write");              // Print write error
            break;
        }

        n = read(sockfd, rcv_buff, RCV_BUFF - 1); // Receive server response
        if (n <= 0) {
            write(STDERR_FILENO, "Server closed connection\n", 25); // Print closed state
            break;
        }
        rcv_buff[n] = '\0';                // Terminate response string

        if (strcmp(rcv_buff, "QUIT") == 0) {
            write(STDOUT_FILENO, "Program quit!!\n", 15); // Print quit message
            break;
        }

        process_result(rcv_buff);          // Print command result
    }

    close(sockfd);                         // Close connected socket
    g_sockfd = -1;                         // Reset global socket descriptor

    return 0;
}
