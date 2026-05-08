///////////////////////////////////////////////////////////////////////////////
// File Name : cli.c
// Date : 2026/05/08
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyunji
// Student ID : 2024402055
// -------------------------------------------------------------------------- //
// Title : System Programming Assignment #2-2 (FTP Client)
// Description :
//   - Connect to server by using socket() and connect()
//   - Send user input message to server
//   - Receive echoed message from server
//   - Terminate when connection is closed
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>

#define BUFF_SIZE 256

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
    char buff[BUFF_SIZE];             // Message buffer
    int n;                            // Number of bytes
    int sockfd;                       // Socket descriptor

    struct sockaddr_in serv_addr;     // Server address structure

    ///////////////////////////// Argument Check /////////////////////////////
    if(argc != 3) {
        write(STDERR_FILENO, "Usage : ./cli <IP> <PORT>\n", 27);
        exit(1);
    }
    ////////////////////////// End of Argument Check /////////////////////////

    ////////////////////////////// Create Socket /////////////////////////////
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd < 0) {
        write(STDERR_FILENO, "socket() error!!\n", 17);
        exit(1);
    }
    /////////////////////////// End of Create Socket /////////////////////////

    ////////////////////////// Set Server Address ////////////////////////////
    memset(&serv_addr, 0, sizeof(serv_addr));     // Initialize structure

    serv_addr.sin_family = AF_INET;               // IPv4 protocol
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); // Convert IP address
    serv_addr.sin_port = htons(atoi(argv[2]));    // Convert port number
    /////////////////////// End of Set Server Address ////////////////////////

    /////////////////////////// Connect to Server ////////////////////////////
    if(connect(sockfd,
              (struct sockaddr*)&serv_addr,
              sizeof(serv_addr)) < 0) {

        write(STDERR_FILENO, "connect() error!!\n", 18);

        close(sockfd);
        exit(1);
    }
    //////////////////////// End of Connect to Server ////////////////////////

    ////////////////////////////// Main Loop /////////////////////////////////
    while(1) {

        memset(buff, 0, BUFF_SIZE);   // Initialize buffer

        write(STDOUT_FILENO, "> ", 2); // Print prompt

        n = read(STDIN_FILENO, buff, BUFF_SIZE);

        ////////////////////////// Read Fail Check //////////////////////////
        if(n <= 0) {
            break;
        }
        /////////////////////// End of Read Fail Check //////////////////////

        ////////////////////////// Send Message /////////////////////////////
        if(write(sockfd, buff, BUFF_SIZE) > 0) {

            memset(buff, 0, BUFF_SIZE); // Initialize receive buffer

            ////////////////////// Receive Message /////////////////////////
            if(read(sockfd, buff, BUFF_SIZE) > 0) {

                printf("from server: %s", buff); // Print echoed message
            }
            //////////////////// End of Receive Message ////////////////////

            else {
                break;
            }
        }
        /////////////////////// End of Send Message /////////////////////////

        else {
            break;
        }
    }
    /////////////////////////// End of Main Loop /////////////////////////////

    close(sockfd); // Close socket

    return 0;
}