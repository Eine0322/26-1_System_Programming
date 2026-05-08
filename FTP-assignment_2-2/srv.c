///////////////////////////////////////////////////////////////////////////////
// File Name : srv.c
// Date : 2026/05/08
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyunji
// Student ID : 2024402055
// -------------------------------------------------------------------------- //
// Title : System Programming Assignment #2-2 (FTP Server)
// Description :
//   - Create server socket and wait for client connection
//   - Create child process by using fork()
//   - Echo received message to client
//   - Terminate child process when QUIT is received
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

void sh_chld(int);
void sh_alrm(int);

///////////////////////////////////////////////////////////////////////////////
// main                                                                      //
// ========================================================================== //
// Input : int argc     -> argument count                                    //
//         char** argv  -> argument vector, PORT                             //
// Output: int        -> 0 success                                           //
// Purpose: Main function of FTP server                                      //
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    char buff[BUFF_SIZE];                 // Message buffer
    int n;                                // Number of bytes

    struct sockaddr_in server_addr;       // Server address structure
    struct sockaddr_in client_addr;       // Client address structure

    int server_fd;                        // Server socket descriptor
    int client_fd;                        // Client socket descriptor

    socklen_t len;                        // Client address length

    ///////////////////////////// Argument Check /////////////////////////////
    if(argc != 2) {
        write(STDERR_FILENO, "Usage : ./srv <PORT>\n", 22);
        exit(1);
    }
    ////////////////////////// End of Argument Check /////////////////////////

    ///////////////////////////// Signal Setting /////////////////////////////
    signal(SIGALRM, sh_alrm); // Register SIGALRM handler
    signal(SIGCHLD, sh_chld); // Register SIGCHLD handler
    ////////////////////////// End of Signal Setting /////////////////////////

    ////////////////////////////// Create Socket /////////////////////////////
    server_fd = socket(PF_INET, SOCK_STREAM, 0);

    if(server_fd < 0) {
        write(STDERR_FILENO, "socket() error!!\n", 17);
        exit(1);
    }
    /////////////////////////// End of Create Socket /////////////////////////

    ////////////////////////// Set Server Address ////////////////////////////
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;             // IPv4 protocol
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Accept all IP
    server_addr.sin_port = htons(atoi(argv[1])); // Convert port number
    /////////////////////// End of Set Server Address ////////////////////////

    ////////////////////////////////// Bind //////////////////////////////////
    if(bind(server_fd,
           (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {

        write(STDERR_FILENO, "bind() error!!\n", 15);

        close(server_fd);
        exit(1);
    }
    //////////////////////////////// End Bind ////////////////////////////////

    ///////////////////////////////// Listen /////////////////////////////////
    if(listen(server_fd, 5) < 0) {

        write(STDERR_FILENO, "listen() error!!\n", 17);

        close(server_fd);
        exit(1);
    }
    ////////////////////////////// End Listen ////////////////////////////////

    ////////////////////////////// Server Loop ///////////////////////////////
    while(1) {

        pid_t pid; // Process ID

        len = sizeof(client_addr);

        ////////////////////////// Accept Client ////////////////////////////
        client_fd = accept(server_fd,
                          (struct sockaddr *)&client_addr,
                          &len);

        if(client_fd < 0) {
            write(STDERR_FILENO, "accept() error!!\n", 17);
            continue;
        }
        //////////////////////// End of Accept Client ///////////////////////

        ////////////////////////////// fork() ///////////////////////////////
        pid = fork();

        ///////////////////////////// fork Fail /////////////////////////////
        if(pid < 0) {

            write(STDERR_FILENO, "fork() error!!\n", 15);

            close(client_fd);
            continue;
        }
        ////////////////////////// End of fork Fail /////////////////////////

        ////////////////////////// Parent Process ///////////////////////////
        if(pid > 0) {

            printf("==========Client info==========\n");
            printf("client IP : %s\n\n",
                   inet_ntoa(client_addr.sin_addr));

            printf("client port : %d\n",
                   ntohs(client_addr.sin_port));

            printf("===============================\n");

            printf("Child Process ID : %d\n", pid);

            close(client_fd); // Parent closes connected socket
        }
        //////////////////////// End of Parent Process //////////////////////

        /////////////////////////// Child Process ///////////////////////////
        else {

            close(server_fd); // Child closes listening socket

            ////////////////////////// Client Loop /////////////////////////
            while(1) {

                memset(buff, 0, BUFF_SIZE); // Initialize buffer

                n = read(client_fd, buff, BUFF_SIZE);

                ///////////////////////// Read Fail ////////////////////////
                if(n <= 0) {
                    break;
                }
                ////////////////////// End of Read Fail ////////////////////

                ////////////////////////// QUIT Check //////////////////////
                if(strncmp(buff, "QUIT", 4) == 0) {

                    close(client_fd); // Close client socket

                    raise(SIGALRM); // Generate SIGALRM signal
                }
                ////////////////////// End of QUIT Check ///////////////////

                ////////////////////////// Echo Message ////////////////////
                if(write(client_fd, buff, BUFF_SIZE) <= 0) {
                    break;
                }
                ////////////////////// End of Echo Message /////////////////
            }
            //////////////////////// End of Client Loop ////////////////////

            close(client_fd); // Close client socket

            exit(0);
        }
        ///////////////////////// End of Child Process /////////////////////
    }
    /////////////////////////// End of Server Loop ///////////////////////////

    close(server_fd); // Close server socket

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// sh_chld                                                                   //
// ========================================================================== //
// Input : int signum -> received signal number                              //
// Output: void                                                              //
// Purpose: Handle terminated child process                                  //
///////////////////////////////////////////////////////////////////////////////
void sh_chld(int signum)
{
    printf("Status of child process was changed.\n");

    wait(NULL); // Remove zombie process
}

///////////////////////////////////////////////////////////////////////////////
// sh_alrm                                                                   //
// ========================================================================== //
// Input : int signum -> received signal number                              //
// Output: void                                                              //
// Purpose: Terminate child process by SIGALRM                               //
///////////////////////////////////////////////////////////////////////////////
void sh_alrm(int signum)
{
    printf("Child Process(PID : %d) will be terminated.\n",
           getpid());

    exit(1);
}