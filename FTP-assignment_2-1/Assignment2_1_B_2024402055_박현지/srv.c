///////////////////////////////////////////////////////////////////////////////
// File Name : srv.c
// Date : 2026/05/02
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyunji
// Student ID : 2024402055
// -------------------------------------------------------------------------- //
// Title : System Programming Assignment #2-1 (FTP Server)
// Description :
//   - Open server socket by using socket(), bind(), listen()
//   - Accept client connection
//   - Process FTP command NLST and QUIT
//   - Send processed result to client
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define MAX_BUFF 1024
#define SEND_BUFF 8192
#define SA struct sockaddr

///////////////////////////////////////////////////////////////////////////////
// client_info                                                               //
// ========================================================================== //
// Input : struct sockaddr_in* cliaddr -> client address information          //
// Output: int        -> 0 success                                           //
// Purpose: Display client IP and port                                       //
///////////////////////////////////////////////////////////////////////////////
int client_info(struct sockaddr_in *cliaddr)
{
    char info[MAX_BUFF];                // Buffer for client information

    memset(info, 0, sizeof(info));       // Initialize buffer

    sprintf(info,
            "==========Client info===========\n"
            "client IP: %s\n"
            "client port: %d\n"
            "================================\n",
            inet_ntoa(cliaddr->sin_addr),
            ntohs(cliaddr->sin_port));

    write(STDOUT_FILENO, info, strlen(info)); // Print client information

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// make_perm                                                                 //
// ========================================================================== //
// Input : mode_t mode -> file mode                                          //
//         char* perm  -> permission string buffer                           //
// Output: void                                                              //
// Purpose: Make permission string like -rwxr-xr-x                           //
///////////////////////////////////////////////////////////////////////////////
void make_perm(mode_t mode, char *perm)
{
    perm[0] = S_ISDIR(mode) ? 'd' : '-';     // File type

    perm[1] = (mode & S_IRUSR) ? 'r' : '-';  // User read permission
    perm[2] = (mode & S_IWUSR) ? 'w' : '-';  // User write permission
    perm[3] = (mode & S_IXUSR) ? 'x' : '-';  // User execute permission

    perm[4] = (mode & S_IRGRP) ? 'r' : '-';  // Group read permission
    perm[5] = (mode & S_IWGRP) ? 'w' : '-';  // Group write permission
    perm[6] = (mode & S_IXGRP) ? 'x' : '-';  // Group execute permission

    perm[7] = (mode & S_IROTH) ? 'r' : '-';  // Other read permission
    perm[8] = (mode & S_IWOTH) ? 'w' : '-';  // Other write permission
    perm[9] = (mode & S_IXOTH) ? 'x' : '-';  // Other execute permission

    perm[10] = '\0';                         // End of string
}

///////////////////////////////////////////////////////////////////////////////
// append_file_info                                                          //
// ========================================================================== //
// Input : char* result_buff -> buffer to store result                       //
//         char* filename    -> file name                                    //
// Output: void                                                              //
// Purpose: Append long format file information to result buffer             //
///////////////////////////////////////////////////////////////////////////////
void append_file_info(char *result_buff, char *filename)
{
    struct stat st;                     // Structure for file status
    struct passwd *pw;                  // Structure for user information
    struct group *gr;                   // Structure for group information
    char perm[11];                      // Permission string
    char time_buff[64];                 // Modified time string
    char line[MAX_BUFF];                // One line of ls -l result

    memset(line, 0, sizeof(line));       // Initialize line buffer

    if (lstat(filename, &st) < 0) {      // Get file information
        return;
    }

    make_perm(st.st_mode, perm);         // Make permission string

    pw = getpwuid(st.st_uid);            // Get owner name
    gr = getgrgid(st.st_gid);            // Get group name

    strftime(time_buff,
             sizeof(time_buff),
             "%b %d %H:%M",
             localtime(&st.st_mtime));   // Convert modified time to string

    sprintf(line,
            "%s %ld %s %s %ld %s %s\n",
            perm,
            (long)st.st_nlink,
            pw ? pw->pw_name : "unknown",
            gr ? gr->gr_name : "unknown",
            (long)st.st_size,
            time_buff,
            filename);

    strcat(result_buff, line);           // Append line to result buffer
}

///////////////////////////////////////////////////////////////////////////////
// cmd_process                                                               //
// ========================================================================== //
// Input : char* buff        -> received FTP command                         //
//         char* result_buff -> buffer to store command result               //
// Output: int        -> 0 success, -1 fail                                  //
// Purpose: Process NLST and QUIT commands                                  //
///////////////////////////////////////////////////////////////////////////////
int cmd_process(char *buff, char *result_buff)
{
    char temp[MAX_BUFF];                 // Temporary command buffer
    char *token;                         // Token pointer
    int option_a = 0;                    // Flag for -a option
    int option_l = 0;                    // Flag for -l option

    DIR *dp;                             // Directory pointer
    struct dirent *dirp;                 // Directory entry pointer

    memset(temp, 0, sizeof(temp));        // Initialize temporary buffer
    strcpy(temp, buff);                  // Copy command for display

    token = strtok(buff, " \t\n");       // Get first command token

    if (token == NULL) {
        return -1;
    }

    ////////////////////////////// QUIT Command //////////////////////////////
    if (strcmp(token, "QUIT") == 0) {
        strcpy(result_buff, "QUIT");     // Store QUIT result
        return 0;
    }
    /////////////////////////// End of QUIT Command //////////////////////////

    ////////////////////////////// NLST Check ////////////////////////////////
    if (strcmp(token, "NLST") != 0) {
        return -1;
    }
    /////////////////////////// End of NLST Check ////////////////////////////

    ////////////////////////// Display Converted Command /////////////////////
    write(STDOUT_FILENO, temp, strlen(temp)); // Print converted command
    write(STDOUT_FILENO, "\n", 1);            // Print newline
    /////////////////////// End of Display Converted Command /////////////////

    ////////////////////////////// Option Check //////////////////////////////
    while ((token = strtok(NULL, " \t\n")) != NULL) {
        if (strcmp(token, "-a") == 0) {
            option_a = 1;                 // Enable hidden file output
        }
        else if (strcmp(token, "-l") == 0) {
            option_l = 1;                 // Enable long format output
        }
        else if (strcmp(token, "-al") == 0 || strcmp(token, "-la") == 0) {
            option_a = 1;                 // Enable hidden file output
            option_l = 1;                 // Enable long format output
        }
        else {
            return -1;                    // Invalid option
        }
    }
    /////////////////////////// End of Option Check //////////////////////////

    ///////////////////////////// Open Directory /////////////////////////////
    dp = opendir(".");                    // Open current directory

    if (dp == NULL) {
        strcpy(result_buff, "opendir() error!!\n");
        return 0;
    }
    ////////////////////////// End of Open Directory /////////////////////////

    ///////////////////////////// Read Directory /////////////////////////////
    while ((dirp = readdir(dp)) != NULL) {

        if (!option_a && dirp->d_name[0] == '.') {
            continue;                     // Skip hidden file without -a
        }

        if (option_l) {
            append_file_info(result_buff, dirp->d_name); // Append long format
        }
        else {
            strcat(result_buff, dirp->d_name);           // Append file name
            strcat(result_buff, "\n");                   // Append newline
        }
    }
    ////////////////////////// End of Read Directory /////////////////////////

    closedir(dp);                          // Close directory

    return 0;
}

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
    int listenfd;                         // Listening socket descriptor
    int connfd;                           // Connected socket descriptor
    int n;                                // Number of bytes read
    int opt = 1;                          // Option value for setsockopt

    char buff[MAX_BUFF];                  // Receive command buffer
    char result_buff[SEND_BUFF];          // Result buffer

    struct sockaddr_in servaddr;          // Server address structure
    struct sockaddr_in cliaddr;           // Client address structure
    socklen_t clilen;                     // Client address length

    ///////////////////////////// Argument Check /////////////////////////////
    if (argc != 2) {
        write(STDERR_FILENO, "Usage: ./srv <PORT>\n", 20);
        exit(1);
    }
    ////////////////////////// End of Argument Check /////////////////////////

    ////////////////////////////// Create Socket /////////////////////////////
    listenfd = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket

    if (listenfd < 0) {
        write(STDERR_FILENO, "socket() error!!\n", 17);
        exit(1);
    }
    /////////////////////////// End of Create Socket /////////////////////////

    ///////////////////////////// Set Socket Option //////////////////////////
    setsockopt(listenfd,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));              // Reuse address
    ////////////////////////// End of Set Socket Option //////////////////////

    ////////////////////////// Set Server Address ////////////////////////////
    memset(&servaddr, 0, sizeof(servaddr));       // Initialize server address
    servaddr.sin_family = AF_INET;                // IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // Accept any client address
    servaddr.sin_port = htons(atoi(argv[1]));     // Convert port
    /////////////////////// End of Set Server Address ////////////////////////

    ///////////////////////////////// Bind ///////////////////////////////////
    if (bind(listenfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        write(STDERR_FILENO, "bind() error!!\n", 15);
        close(listenfd);
        exit(1);
    }
    ////////////////////////////// End of Bind ///////////////////////////////

    //////////////////////////////// Listen //////////////////////////////////
    if (listen(listenfd, 5) < 0) {
        write(STDERR_FILENO, "listen() error!!\n", 17);
        close(listenfd);
        exit(1);
    }
    ///////////////////////////// End of Listen //////////////////////////////

    ////////////////////////////// Server Loop ///////////////////////////////
    while (1) {
        clilen = sizeof(cliaddr);          // Set client address length

        connfd = accept(listenfd, (SA *)&cliaddr, &clilen); // Accept client

        if (connfd < 0) {
            write(STDERR_FILENO, "accept() error!!\n", 17);
            continue;
        }

        if (client_info(&cliaddr) < 0) {
            write(STDERR_FILENO, "client_info() err!!\n", 21);
        }

        ////////////////////////// Client Loop ///////////////////////////////
        while (1) {
            memset(buff, 0, MAX_BUFF);         // Initialize receive buffer
            memset(result_buff, 0, SEND_BUFF); // Initialize result buffer

            n = read(connfd, buff, MAX_BUFF - 1); // Read FTP command

            if (n <= 0) {
                close(connfd);                 // Close client socket
                break;
            }

            buff[n] = '\0';                    // Add null character

            ///////////////////////// Process Command ///////////////////////
            if (cmd_process(buff, result_buff) < 0) {
                write(STDERR_FILENO, "cmd_process() err!!\n", 20);
                strcpy(result_buff, "command error\n");
            }
            ////////////////////// End of Process Command ///////////////////

            write(connfd, result_buff, strlen(result_buff)); // Send result

            ///////////////////////////// Quit Check ////////////////////////
            if (strcmp(result_buff, "QUIT") == 0) {
                write(STDOUT_FILENO, "QUIT\n", 5);
                close(connfd);
                break;
            }
            ////////////////////////// End of Quit Check ////////////////////
        }
        //////////////////////// End of Client Loop //////////////////////////
    }
    /////////////////////////// End of Server Loop ///////////////////////////

    close(listenfd);                         // Close listening socket

    return 0;
}