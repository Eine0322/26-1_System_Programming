///////////////////////////////////////////////////////////////////////////////
// File Name : cli.c
// Date : 2026/05/21
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyunji
// Student ID : 2024402055
// -------------------------------------------------------------------------- //
// Title : System Programming Assignment #3-1 (FTP Client)
// Description :
//   - Connect to FTP server
//   - Check client authentication
//   - Input username and password
//   - Receive login result from server
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUF 20

///////////////////////////////////////////////////////////////////////////////
// log_in                                                                    //
// ========================================================================== //
// Input : int sockfd -> connected socket descriptor                         //
// Output: void                                                              //
// Purpose: Perform user login process                                       //
///////////////////////////////////////////////////////////////////////////////
void log_in(int sockfd)
{
    int n;
    char user[MAX_BUF];
    char passwd[MAX_BUF];
    char buf[MAX_BUF];

    memset(buf, 0, sizeof(buf));

    //////////////////////// Check Connection Status /////////////////////////
    n = read(sockfd, buf, MAX_BUF);

    if (n <= 0) {
        return;
    }

    buf[n] = '\0';

    /////////////////////////// Rejected Client //////////////////////////////
    if (!strcmp(buf, "REJECTION")) {
        printf("** Connection refused **\n");
        return;
    }
    //////////////////////// End of Rejected Client //////////////////////////

    printf("** It is connected to Server **\n");
    ////////////////////////// End of Connection /////////////////////////////

    ////////////////////////////// Login Loop ////////////////////////////////
    while (1) {

        memset(user, 0, sizeof(user));
        memset(passwd, 0, sizeof(passwd));
        memset(buf, 0, sizeof(buf));

        ////////////////////////// Input Username ////////////////////////////
        printf("Input ID : ");
        scanf("%s", user);

        write(sockfd, user, MAX_BUF);
        /////////////////////// End of Input Username ////////////////////////

        ////////////////////////// Input Password ////////////////////////////
        printf("Input Password : ");
        scanf("%s", passwd);

        write(sockfd, passwd, MAX_BUF);
        /////////////////////// End of Input Password ////////////////////////

        ////////////////////////// Receive Result ////////////////////////////
        n = read(sockfd, buf, MAX_BUF);

        if (n <= 0) {
            break;
        }

        buf[n] = '\0';
        /////////////////////// End of Receive Result ////////////////////////

        ///////////////////////////// Success ////////////////////////////////
        if (!strcmp(buf, "OK")) {
            printf("** User '%s' logged in **\n", user);
            break;
        }
        ////////////////////////// End of Success ////////////////////////////

        ////////////////////////////// Fail /////////////////////////////////
        else if (!strcmp(buf, "FAIL")) {
            printf("** Log-in failed **\n");
        }
        /////////////////////////// End of Fail //////////////////////////////

        ///////////////////////// Disconnection //////////////////////////////
        else if (!strcmp(buf, "DISCONNECTION")) {
            printf("** Connection closed **\n");
            break;
        }
        ////////////////////// End of Disconnection //////////////////////////
    }
    ////////////////////////// End of Login Loop /////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////
// main                                                                      //
// ========================================================================== //
// Input : int argc     -> argument count                                    //
//         char** argv  -> server IP and port                                //
// Output: int        -> 0 success                                           //
// Purpose: Main function of client                                          //
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    int sockfd;

    struct sockaddr_in servaddr;

    ///////////////////////////// Argument Check /////////////////////////////
    if (argc != 3) {
        printf("Usage : ./cli <IP> <PORT>\n");
        exit(1);
    }
    ////////////////////////// End of Argument Check /////////////////////////

    ////////////////////////////// Create Socket /////////////////////////////
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }
    /////////////////////////// End of Create Socket /////////////////////////

    ////////////////////////// Set Server Address ////////////////////////////
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    /////////////////////// End of Set Server Address ////////////////////////

    /////////////////////////// Connect to Server ////////////////////////////
    if (connect(sockfd,
                (struct sockaddr *)&servaddr,
                sizeof(servaddr)) < 0) {

        perror("connect");
        close(sockfd);
        exit(1);
    }
    //////////////////////// End of Connect to Server ////////////////////////

    log_in(sockfd);

    close(sockfd);

    return 0;
}