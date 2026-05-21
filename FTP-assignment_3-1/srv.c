///////////////////////////////////////////////////////////////////////////////
// File Name : srv.c
// Date : 2026/05/21
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyunji
// Student ID : 2024402055
// -------------------------------------------------------------------------- //
// Title : System Programming Assignment #3-1 (FTP Server)
// Description :
//   - Accept client connection
//   - Authenticate client IP by using access.txt
//   - Authenticate user by using passwd file
//   - Disconnect after 3 login failures
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUF 20

///////////////////////////////////////////////////////////////////////////////
// check_ip                                                                  //
// ========================================================================== //
// Input : char* ip -> client IP address                                     //
// Output: int        -> 1 success, 0 fail                                   //
// Purpose: Check whether client IP exists in access.txt                     //
///////////////////////////////////////////////////////////////////////////////
int check_ip(char *ip)
{
    FILE *fp;

    char buf[MAX_BUF];

    fp = fopen("access.txt", "r");

    if (fp == NULL) {
        return 0;
    }

    ///////////////////////////// Read access.txt ////////////////////////////
    while (fgets(buf, sizeof(buf), fp) != NULL) {

        buf[strcspn(buf, "\n")] = '\0';

        if (!strcmp(ip, buf)) {
            fclose(fp);
            return 1;
        }
    }
    ////////////////////////// End of Read access.txt ////////////////////////

    fclose(fp);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// user_match                                                                //
// ========================================================================== //
// Input : char* user   -> username                                          //
//         char* passwd -> password                                          //
// Output: int        -> 1 success, 0 fail                                   //
// Purpose: Check username and password in passwd file                       //
///////////////////////////////////////////////////////////////////////////////
int user_match(char *user, char *passwd)
{
    FILE *fp;

    char buf[100];
    char *token;

    fp = fopen("passwd", "r");

    if (fp == NULL) {
        return 0;
    }

    ////////////////////////////// Read passwd ///////////////////////////////
    while (fgets(buf, sizeof(buf), fp) != NULL) {

        token = strtok(buf, ":");

        if (token == NULL) {
            continue;
        }

        ////////////////////////// Username Check ////////////////////////////
        if (!strcmp(token, user)) {

            token = strtok(NULL, ":");

            if (token == NULL) {
                continue;
            }

            token[strcspn(token, "\n")] = '\0';

            ////////////////////// Password Check ///////////////////////////
            if (!strcmp(token, passwd)) {
                fclose(fp);
                return 1;
            }
            /////////////////// End of Password Check ///////////////////////
        }
        /////////////////////// End of Username Check ////////////////////////
    }
    ////////////////////////// End of Read passwd ////////////////////////////

    fclose(fp);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// log_auth                                                                  //
// ========================================================================== //
// Input : int connfd -> connected socket descriptor                         //
// Output: int        -> 1 success, 0 fail                                   //
// Purpose: Authenticate user login                                          //
///////////////////////////////////////////////////////////////////////////////
int log_auth(int connfd)
{
    char user[MAX_BUF];
    char passwd[MAX_BUF];

    int n;
    int count = 1;

    ////////////////////////////// Login Loop ////////////////////////////////
    while (1) {

        memset(user, 0, sizeof(user));
        memset(passwd, 0, sizeof(passwd));

        printf("** User is trying to log-in (%d/3) **\n", count);

        ////////////////////////// Receive Username //////////////////////////
        n = read(connfd, user, MAX_BUF);

        if (n <= 0) {
            return 0;
        }

        user[n] = '\0';
        /////////////////////// End of Receive Username //////////////////////

        ////////////////////////// Receive Password //////////////////////////
        n = read(connfd, passwd, MAX_BUF);

        if (n <= 0) {
            return 0;
        }

        passwd[n] = '\0';
        /////////////////////// End of Receive Password //////////////////////

        ////////////////////////// Authentication ////////////////////////////
        if (user_match(user, passwd) == 1) {

            write(connfd, "OK", MAX_BUF);

            printf("** Success to log-in **\n");

            return 1;
        }
        /////////////////////// End of Authentication ////////////////////////

        ////////////////////////////// Failure ///////////////////////////////
        else {

            printf("** Log-in failed **\n");

            /////////////////////////// 3 Times Fail ////////////////////////
            if (count >= 3) {

                write(connfd, "DISCONNECTION", MAX_BUF);

                return 0;
            }
            //////////////////////// End of 3 Times Fail ////////////////////

            write(connfd, "FAIL", MAX_BUF);

            count++;
        }
        ////////////////////////// End of Failure ////////////////////////////
    }
    ////////////////////////// End of Login Loop /////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////
// main                                                                      //
// ========================================================================== //
// Input : int argc     -> argument count                                    //
//         char** argv  -> server port                                       //
// Output: int        -> 0 success                                           //
// Purpose: Main function of server                                          //
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    int listenfd;
    int connfd;

    socklen_t clilen;

    char ip[INET_ADDRSTRLEN];

    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;

    ///////////////////////////// Argument Check /////////////////////////////
    if (argc != 2) {
        printf("Usage : ./srv <PORT>\n");
        exit(1);
    }
    ////////////////////////// End of Argument Check /////////////////////////

    ////////////////////////////// Create Socket /////////////////////////////
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (listenfd < 0) {
        perror("socket");
        exit(1);
    }
    /////////////////////////// End of Create Socket /////////////////////////

    ////////////////////////// Set Server Address ////////////////////////////
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));
    /////////////////////// End of Set Server Address ////////////////////////

    ///////////////////////////////// Bind ///////////////////////////////////
    if (bind(listenfd,
             (struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0) {

        perror("bind");
        close(listenfd);
        exit(1);
    }
    ////////////////////////////// End of Bind ///////////////////////////////

    //////////////////////////////// Listen //////////////////////////////////
    listen(listenfd, 5);
    ///////////////////////////// End of Listen //////////////////////////////

    ////////////////////////////// Server Loop ///////////////////////////////
    while (1) {

        clilen = sizeof(cliaddr);

        connfd = accept(listenfd,
                        (struct sockaddr *)&cliaddr,
                        &clilen);

        if (connfd < 0) {
            continue;
        }

        ////////////////////////// Client Information ////////////////////////
        printf("** Client is trying to connect **\n");

        inet_ntop(AF_INET,
                  &cliaddr.sin_addr,
                  ip,
                  sizeof(ip));

        printf("- IP: %s\n", ip);
        printf("- Port: %d\n", ntohs(cliaddr.sin_port));
        /////////////////////// End of Client Information ////////////////////

        //////////////////////////// IP Check ////////////////////////////////
        if (check_ip(ip) == 0) {

            write(connfd, "REJECTION", MAX_BUF);

            printf("** It is NOT authenticated client **\n");

            close(connfd);

            continue;
        }
        ///////////////////////// End of IP Check ////////////////////////////

        write(connfd, "ACCEPTED", MAX_BUF);

        printf("** Client is connected **\n");

        ////////////////////////// Login Authentication //////////////////////
        if (log_auth(connfd) == 0) {

            printf("** Fail to log-in **\n");

            close(connfd);

            continue;
        }
        ///////////////////// End of Login Authentication ////////////////////

        close(connfd);
    }
    /////////////////////////// End of Server Loop ///////////////////////////

    close(listenfd);

    return 0;
}