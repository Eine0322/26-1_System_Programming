///////////////////////////////////////////////////////////////////////////////
// File Name : cli.c
// Date : 2026/05/02
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyunji
// Student ID : 2024402055
// -------------------------------------------------------------------------- //
// Title : System Programming Assignment #2-1 (FTP Client)
// Description :
//   - Connect to FTP server by using socket() and connect()
//   - Convert user commands to FTP commands
//   - Send FTP command to server by using write()
//   - Receive result from server by using read()
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFF 1024
#define RCV_BUFF 8192

///////////////////////////////////////////////////////////////////////////////
// conv_cmd                                                                  //
// ========================================================================== //
// Input : char* buff     -> user input command                               //
//         char* cmd_buff -> buffer to store converted FTP command            //
// Output: int        -> 0 success, -1 fail                                   //
// Purpose: Convert user command to FTP command                               //
///////////////////////////////////////////////////////////////////////////////
int conv_cmd(char *buff, char *cmd_buff)
{
    char temp[MAX_BUFF];               // Temporary buffer for tokenizing input
    char *token;                       // Pointer for each token

    memset(temp, 0, sizeof(temp));      // Initialize temporary buffer
    strcpy(temp, buff);                // Copy input command to temporary buffer

    token = strtok(temp, " \t\n");     // Get first token from user input

    if (token == NULL) {               // If input is empty
        return -1;
    }

    ////////////////////////////// ls command //////////////////////////////
    if (strcmp(token, "ls") == 0) {
        strcpy(cmd_buff, "NLST");      // Convert ls to NLST

        while ((token = strtok(NULL, " \t\n")) != NULL) {

            if (strcmp(token, "-a") == 0 ||
                strcmp(token, "-l") == 0 ||
                strcmp(token, "-al") == 0 ||
                strcmp(token, "-la") == 0) {

                strcat(cmd_buff, " ");     // Add blank before option
                strcat(cmd_buff, token);   // Add option to FTP command
            }
            else {
                return -1;                 // Invalid option
            }
        }
    }
    ////////////////////////////// quit command ////////////////////////////
    else if (strcmp(token, "quit") == 0) {
        token = strtok(NULL, " \t\n");     // Check extra argument

        if (token != NULL) {
            return -1;                     // quit must not have argument
        }

        strcpy(cmd_buff, "QUIT");          // Convert quit to QUIT
    }
    ////////////////////////////// invalid command /////////////////////////
    else {
        return -1;                         // Unknown command
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// process_result                                                            //
// ========================================================================== //
// Input : char* rcv_buff -> received result from server                      //
// Output: void                                                              //
// Purpose: Display command result                                           //
///////////////////////////////////////////////////////////////////////////////
void process_result(char *rcv_buff)
{
    write(STDOUT_FILENO, rcv_buff, strlen(rcv_buff)); // Print server result
}

///////////////////////////////////////////////////////////////////////////////
// main                                                                      //
// ========================================================================== //
// Input : int argc     -> argument count                                    //
//         char** argv  -> argument vector, IP and PORT                      //
// Output: int        -> 0 success                                           //
// Purpose: Main function of FTP client                                      //
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    int sockfd;                         // Socket file descriptor
    int n;                              // Number of bytes read or written
    char buff[MAX_BUFF];                // User input buffer
    char cmd_buff[MAX_BUFF];            // Converted FTP command buffer
    char rcv_buff[RCV_BUFF];            // Receive buffer

    struct sockaddr_in servaddr;        // Server address structure

    ///////////////////////////// Argument Check /////////////////////////////
    if (argc != 3) {
        write(STDERR_FILENO, "Usage: ./cli <IP> <PORT>\n", 25);
        exit(1);
    }
    ////////////////////////// End of Argument Check /////////////////////////

    ////////////////////////////// Create Socket /////////////////////////////
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket

    if (sockfd < 0) {
        write(STDERR_FILENO, "socket() error!!\n", 17);
        exit(1);
    }
    /////////////////////////// End of Create Socket /////////////////////////

    ////////////////////////// Set Server Address ////////////////////////////
    memset(&servaddr, 0, sizeof(servaddr));       // Initialize server address
    servaddr.sin_family = AF_INET;                // IPv4
    servaddr.sin_port = htons(atoi(argv[2]));     // Convert port to network byte order
    servaddr.sin_addr.s_addr = inet_addr(argv[1]); // Convert IP string to address
    /////////////////////// End of Set Server Address ////////////////////////

    /////////////////////////// Connect to Server ////////////////////////////
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        write(STDERR_FILENO, "connect() error!!\n", 18);
        close(sockfd);
        exit(1);
    }
    //////////////////////// End of Connect to Server ////////////////////////

    ////////////////////////////// Main Loop /////////////////////////////////
    while (1) {
        memset(buff, 0, MAX_BUFF);          // Initialize user input buffer
        memset(cmd_buff, 0, MAX_BUFF);      // Initialize command buffer
        memset(rcv_buff, 0, RCV_BUFF);      // Initialize receive buffer

        write(STDOUT_FILENO, "> ", 2);      // Print prompt

        n = read(STDIN_FILENO, buff, MAX_BUFF - 1); // Read user command
        if (n <= 0) {
            break;
        }

        buff[n] = '\0';                     // Add null character

        ////////////////////////// Convert Command //////////////////////////
        if (conv_cmd(buff, cmd_buff) < 0) {
            write(STDERR_FILENO, "conv_cmd() error!!\n", 19);
            continue;
        }
        /////////////////////// End of Convert Command //////////////////////

        ////////////////////////// Send Command /////////////////////////////
        n = strlen(cmd_buff);               // Get command length

        if (write(sockfd, cmd_buff, n) != n) {
            write(STDERR_FILENO, "write() error!!\n", 16);
            close(sockfd);
            exit(1);
        }
        /////////////////////// End of Send Command /////////////////////////

        ///////////////////////// Receive Result ////////////////////////////
        n = read(sockfd, rcv_buff, RCV_BUFF - 1);

        if (n < 0) {
            write(STDERR_FILENO, "read() error!!\n", 15);
            close(sockfd);
            exit(1);
        }

        rcv_buff[n] = '\0';                 // Add null character
        ////////////////////// End of Receive Result ////////////////////////

        ///////////////////////////// Quit Check ////////////////////////////
        if (strcmp(rcv_buff, "QUIT") == 0) {
            write(STDOUT_FILENO, "Program quit!!\n", 15);
            break;
        }
        ////////////////////////// End of Quit Check ////////////////////////

        process_result(rcv_buff);           // Display command result
    }
    /////////////////////////// End of Main Loop /////////////////////////////

    close(sockfd);                          // Close socket

    return 0;
}