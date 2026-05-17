///////////////////////////////////////////////////////////////////////////////
// File Name : srv.c
// Date : 2026/05/14
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyunji
// Student ID : 2024402055
// -------------------------------------------------------------------------- //
// Title : System Programming Assignment #2-3 (FTP Server)
// Description :
//   - Open server socket by using socket(), bind(), listen(), accept()
//   - Support concurrent clients by using fork()
//   - Process FTP commands from clients and send results
//   - Print child process information every 10 seconds
//   - Handle SIGCHLD, SIGALRM, and SIGINT signals
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_BUFF 1024
#define SEND_BUFF 8192
#define MAX_CLIENT 128
#define LISTENQ 10
#define SA struct sockaddr

typedef struct {
    pid_t pid;                            // Child process ID
    int port;                             // Client port number
    time_t start_time;                    // Client service start time
    int active;                           // Active state flag
} ClientInfo;

ClientInfo g_clients[MAX_CLIENT];         // Client process table
int g_listenfd = -1;                      // Listening socket descriptor

///////////////////////////////////////////////////////////////////////////////
// safe_append                                                               //
// ========================================================================== //
// Input : char* dst      -> destination buffer                               //
//         size_t dstsize -> destination buffer size                           //
//         const char* src -> source string                                    //
// Output: void                                                               //
// Purpose: Append string safely to result buffer                             //
///////////////////////////////////////////////////////////////////////////////
void safe_append(char *dst, size_t dstsize, const char *src)
{
    size_t cur = strlen(dst);             // Current string length
    size_t remain;                        // Remaining buffer size

    if (cur >= dstsize - 1) {
        return;                           // Do not append if buffer is full
    }

    remain = dstsize - cur - 1;           // Calculate remaining space
    strncat(dst, src, remain);            // Append source string safely
}

///////////////////////////////////////////////////////////////////////////////
// send_all                                                                  //
// ========================================================================== //
// Input : int fd        -> socket file descriptor                            //
//         const char* b -> send buffer                                        //
//         size_t len    -> send length                                        //
// Output: int        -> 0 success, -1 fail                                    //
// Purpose: Send all bytes to connected client                                //
///////////////////////////////////////////////////////////////////////////////
int send_all(int fd, const char *b, size_t len)
{
    size_t sent = 0;                      // Number of bytes already sent
    ssize_t n;                            // Number of bytes sent at once

    while (sent < len) {
        n = write(fd, b + sent, len - sent); // Send remaining bytes
        if (n <= 0) {
            return -1;                    // Return error on write failure
        }
        sent += (size_t)n;                // Update sent byte count
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// count_clients                                                             //
// ========================================================================== //
// Input : void                                                               //
// Output: int -> number of active clients                                    //
// Purpose: Count active child processes                                      //
///////////////////////////////////////////////////////////////////////////////
int count_clients(void)
{
    int i;                                // Loop index
    int count = 0;                        // Active client count

    for (i = 0; i < MAX_CLIENT; i++) {
        if (g_clients[i].active) {
            count++;                      // Count active client
        }
    }

    return count;
}

///////////////////////////////////////////////////////////////////////////////
// print_process_info                                                        //
// ========================================================================== //
// Input : void                                                               //
// Output: void                                                               //
// Purpose: Print current child process information                           //
///////////////////////////////////////////////////////////////////////////////
void print_process_info(void)
{
    int i;                                // Loop index
    time_t now = time(NULL);              // Current time

    printf("Current Number of Client : %d\n", count_clients());
    printf("   PID   PORT   TIME\n");

    for (i = 0; i < MAX_CLIENT; i++) {
        if (g_clients[i].active) {
            printf("%6d %6d %6ld\n",
                   g_clients[i].pid,
                   g_clients[i].port,
                   (long)(now - g_clients[i].start_time)); // Print service time
        }
    }

    fflush(stdout);                       // Flush output immediately
}

///////////////////////////////////////////////////////////////////////////////
// add_client                                                                //
// ========================================================================== //
// Input : pid_t pid -> child process ID                                      //
//         int port  -> client port number                                    //
// Output: void                                                               //
// Purpose: Add child process information to process table                    //
///////////////////////////////////////////////////////////////////////////////
void add_client(pid_t pid, int port)
{
    int i;                                // Loop index

    for (i = 0; i < MAX_CLIENT; i++) {
        if (!g_clients[i].active) {
            g_clients[i].pid = pid;       // Store child PID
            g_clients[i].port = port;     // Store client port
            g_clients[i].start_time = time(NULL); // Store service start time
            g_clients[i].active = 1;      // Mark client as active
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// remove_client                                                             //
// ========================================================================== //
// Input : pid_t pid -> child process ID                                      //
// Output: void                                                               //
// Purpose: Remove terminated child process from process table                //
///////////////////////////////////////////////////////////////////////////////
void remove_client(pid_t pid)
{
    int i;                                // Loop index

    for (i = 0; i < MAX_CLIENT; i++) {
        if (g_clients[i].active && g_clients[i].pid == pid) {
            printf("Client(%d)'s Release\n", pid); // Print release message
            g_clients[i].active = 0;      // Remove client information
            fflush(stdout);               // Flush output immediately
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// sigchld_handler                                                           //
// ========================================================================== //
// Input : int signo -> signal number                                         //
// Output: void                                                               //
// Purpose: Reap terminated child processes                                   //
///////////////////////////////////////////////////////////////////////////////
void sigchld_handler(int signo)
{
    pid_t pid;                            // Terminated child PID
    int status;                           // Child status

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        remove_client(pid);               // Remove child from table
    }
}

///////////////////////////////////////////////////////////////////////////////
// sigalrm_handler                                                           //
// ========================================================================== //
// Input : int signo -> signal number                                         //
// Output: void                                                               //
// Purpose: Print process table every 10 seconds                              //
///////////////////////////////////////////////////////////////////////////////
void sigalrm_handler(int signo)
{
    print_process_info();                 // Print current child process table
    alarm(10);                            // Restart 10 second interval
}

///////////////////////////////////////////////////////////////////////////////
// sigint_handler                                                            //
// ========================================================================== //
// Input : int signo -> signal number                                         //
// Output: void                                                               //
// Purpose: Terminate server and all child processes on Ctrl+C                //
///////////////////////////////////////////////////////////////////////////////
void sigint_handler(int signo)
{
    int i;                                // Loop index

    printf("\nServer terminated by SIGINT\n");

    for (i = 0; i < MAX_CLIENT; i++) {
        if (g_clients[i].active) {
            kill(g_clients[i].pid, SIGTERM); // Terminate child process
        }
    }

    if (g_listenfd >= 0) {
        close(g_listenfd);                // Close listening socket
    }

    exit(0);                              // Terminate server process
}

///////////////////////////////////////////////////////////////////////////////
// client_info                                                               //
// ========================================================================== //
// Input : struct sockaddr_in* cliaddr -> client address information           //
//         pid_t pid                  -> child process ID                      //
// Output: void                                                               //
// Purpose: Display client IP, port, and child PID                            //
///////////////////////////////////////////////////////////////////////////////
void client_info(struct sockaddr_in *cliaddr, pid_t pid)
{
    printf("==========Client info===========\n");
    printf("client IP: %s\n", inet_ntoa(cliaddr->sin_addr));
    printf("client port: %d\n", ntohs(cliaddr->sin_port));
    printf("Child Process ID : %d\n", pid);
    printf("================================\n");
    fflush(stdout);                       // Flush output immediately
}

///////////////////////////////////////////////////////////////////////////////
// make_perm                                                                 //
// ========================================================================== //
// Input : mode_t mode -> file mode                                           //
//         char* perm  -> permission string buffer                            //
// Output: void                                                               //
// Purpose: Make permission string like -rwxr-xr-x                            //
///////////////////////////////////////////////////////////////////////////////
void make_perm(mode_t mode, char *perm)
{
    perm[0] = S_ISDIR(mode) ? 'd' : '-';  // Set file type character
    perm[1] = (mode & S_IRUSR) ? 'r' : '-'; // Set user read permission
    perm[2] = (mode & S_IWUSR) ? 'w' : '-'; // Set user write permission
    perm[3] = (mode & S_IXUSR) ? 'x' : '-'; // Set user execute permission
    perm[4] = (mode & S_IRGRP) ? 'r' : '-'; // Set group read permission
    perm[5] = (mode & S_IWGRP) ? 'w' : '-'; // Set group write permission
    perm[6] = (mode & S_IXGRP) ? 'x' : '-'; // Set group execute permission
    perm[7] = (mode & S_IROTH) ? 'r' : '-'; // Set other read permission
    perm[8] = (mode & S_IWOTH) ? 'w' : '-'; // Set other write permission
    perm[9] = (mode & S_IXOTH) ? 'x' : '-'; // Set other execute permission
    perm[10] = '\0';                      // Terminate permission string
}

///////////////////////////////////////////////////////////////////////////////
// append_file_info                                                          //
// ========================================================================== //
// Input : char* result -> result buffer                                      //
//         size_t size  -> result buffer size                                 //
//         char* path   -> target file path                                   //
//         char* name   -> display file name                                  //
// Output: void                                                               //
// Purpose: Append long format file information to result buffer              //
///////////////////////////////////////////////////////////////////////////////
void append_file_info(char *result, size_t size, char *path, char *name)
{
    struct stat st;                       // File status structure
    struct passwd *pw;                    // Owner information
    struct group *gr;                     // Group information
    char perm[11];                        // Permission string
    char time_buff[64];                   // Modified time string
    char line[MAX_BUFF];                  // One result line

    if (lstat(path, &st) < 0) {
        return;                           // Skip if status cannot be read
    }

    make_perm(st.st_mode, perm);          // Make permission string
    pw = getpwuid(st.st_uid);             // Get owner name
    gr = getgrgid(st.st_gid);             // Get group name
    strftime(time_buff, sizeof(time_buff), "%b %d %H:%M", localtime(&st.st_mtime));

    snprintf(line, sizeof(line), "%s %ld %s %s %ld %s %s\n",
             perm,
             (long)st.st_nlink,
             pw ? pw->pw_name : "unknown",
             gr ? gr->gr_name : "unknown",
             (long)st.st_size,
             time_buff,
             name);                       // Make long format line

    safe_append(result, size, line);       // Append line to result
}

///////////////////////////////////////////////////////////////////////////////
// process_list                                                              //
// ========================================================================== //
// Input : char* args   -> command arguments                                  //
//         char* result -> result buffer                                      //
//         int force_al -> force -a and -l options                            //
// Output: int        -> 0 success                                            //
// Purpose: Process NLST and LIST commands                                    //
///////////////////////////////////////////////////////////////////////////////
int process_list(char *args, char *result, int force_al)
{
    char local[MAX_BUFF];                 // Local argument buffer
    char *token;                          // Token pointer
    char target[MAX_BUFF] = ".";          // Target directory path
    char fullpath[MAX_BUFF];              // Full file path
    int option_a = force_al;              // Hidden file option
    int option_l = force_al;              // Long format option
    DIR *dp;                              // Directory pointer
    struct dirent *dirp;                  // Directory entry pointer

    memset(local, 0, sizeof(local));       // Initialize local buffer
    if (args != NULL) {
        strcpy(local, args);              // Copy arguments
    }

    token = strtok(local, " \t\n");       // Get first argument token
    while (token != NULL) {
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
        else if (token[0] == '-') {
            safe_append(result, SEND_BUFF, "Error: invalid option\n");
            return 0;                     // Invalid option
        }
        else {
            strcpy(target, token);        // Set target path
        }
        token = strtok(NULL, " \t\n");   // Get next token
    }

    dp = opendir(target);                 // Open target directory
    if (dp == NULL) {
        safe_append(result, SEND_BUFF, "Error: cannot open directory\n");
        return 0;
    }

    while ((dirp = readdir(dp)) != NULL) {
        if (!option_a && dirp->d_name[0] == '.') {
            continue;                     // Skip hidden files without -a
        }

        snprintf(fullpath, sizeof(fullpath), "%s/%s", target, dirp->d_name); // Make full path

        if (option_l) {
            append_file_info(result, SEND_BUFF, fullpath, dirp->d_name); // Append long info
        }
        else {
            safe_append(result, SEND_BUFF, dirp->d_name); // Append file name
            safe_append(result, SEND_BUFF, "\n");        // Append newline
        }
    }

    closedir(dp);                         // Close directory stream
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// cmd_process                                                               //
// ========================================================================== //
// Input : char* buff   -> received FTP command                               //
//         char* result -> command result buffer                              //
// Output: int        -> 1 quit, 0 normal                                     //
// Purpose: Execute FTP command and make response                             //
///////////////////////////////////////////////////////////////////////////////
int cmd_process(char *buff, char *result)
{
    char origin[MAX_BUFF];                // Original command for display
    char *cmd;                            // Command token
    char *arg;                            // Argument pointer
    char *arg2;                           // Second argument pointer
    char cwd[MAX_BUFF];                   // Current working directory

    memset(origin, 0, sizeof(origin));     // Initialize original command buffer
    strcpy(origin, buff);                 // Save original command

    cmd = strtok(buff, " \t\n");          // Get FTP command
    if (cmd == NULL) {
        safe_append(result, SEND_BUFF, "command error\n");
        return 0;
    }

    printf("%s [%d]\n", origin, getpid()); // Display command with child PID
    fflush(stdout);                       // Flush output immediately

    if (strcmp(cmd, "QUIT") == 0) {
        strcpy(result, "QUIT");          // Return QUIT response
        return 1;
    }
    else if (strcmp(cmd, "PWD") == 0) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            snprintf(result, SEND_BUFF, "%s\n", cwd); // Store current path
        }
        else {
            safe_append(result, SEND_BUFF, "Error: getcwd failed\n");
        }
    }
    else if (strcmp(cmd, "NLST") == 0) {
        arg = strtok(NULL, "\n");         // Get all remaining arguments
        process_list(arg, result, 0);      // Process NLST command
    }
    else if (strcmp(cmd, "LIST") == 0) {
        arg = strtok(NULL, "\n");         // Get all remaining arguments
        process_list(arg, result, 1);      // Process LIST command as -al
    }
    else if (strcmp(cmd, "CWD") == 0) {
        arg = strtok(NULL, " \t\n");      // Get target path
        if (arg == NULL || strtok(NULL, " \t\n") != NULL) {
            safe_append(result, SEND_BUFF, "Error: CWD needs one argument\n");
        }
        else if (chdir(arg) < 0) {
            safe_append(result, SEND_BUFF, "Error: directory not found\n");
        }
        else if (getcwd(cwd, sizeof(cwd)) != NULL) {
            snprintf(result, SEND_BUFF, "%s\n", cwd); // Return changed path
        }
    }
    else if (strcmp(cmd, "CDUP") == 0) {
        if (chdir("..") < 0) {
            safe_append(result, SEND_BUFF, "Error: CDUP failed\n");
        }
        else if (getcwd(cwd, sizeof(cwd)) != NULL) {
            snprintf(result, SEND_BUFF, "%s\n", cwd); // Return changed path
        }
    }
    else if (strcmp(cmd, "MKD") == 0) {
        arg = strtok(NULL, " \t\n");      // Get first directory name
        if (arg == NULL) {
            safe_append(result, SEND_BUFF, "Error: MKD needs arguments\n");
        }
        while (arg != NULL) {
            if (mkdir(arg, 0755) < 0) {
                snprintf(cwd, sizeof(cwd), "Error: cannot create %s\n", arg);
                safe_append(result, SEND_BUFF, cwd); // Append creation error
            }
            else {
                snprintf(cwd, sizeof(cwd), "MKD %s success\n", arg);
                safe_append(result, SEND_BUFF, cwd); // Append success message
            }
            arg = strtok(NULL, " \t\n"); // Get next directory name
        }
    }
    else if (strcmp(cmd, "DELE") == 0) {
        arg = strtok(NULL, " \t\n");      // Get first file name
        if (arg == NULL) {
            safe_append(result, SEND_BUFF, "Error: DELE needs arguments\n");
        }
        while (arg != NULL) {
            if (remove(arg) < 0) {
                snprintf(cwd, sizeof(cwd), "Error: cannot delete %s\n", arg);
                safe_append(result, SEND_BUFF, cwd); // Append deletion error
            }
            else {
                snprintf(cwd, sizeof(cwd), "DELE %s success\n", arg);
                safe_append(result, SEND_BUFF, cwd); // Append success message
            }
            arg = strtok(NULL, " \t\n"); // Get next file name
        }
    }
    else if (strcmp(cmd, "RMD") == 0) {
        arg = strtok(NULL, " \t\n");      // Get first directory name
        if (arg == NULL) {
            safe_append(result, SEND_BUFF, "Error: RMD needs arguments\n");
        }
        while (arg != NULL) {
            if (rmdir(arg) < 0) {
                snprintf(cwd, sizeof(cwd), "Error: cannot remove %s\n", arg);
                safe_append(result, SEND_BUFF, cwd); // Append remove error
            }
            else {
                snprintf(cwd, sizeof(cwd), "RMD %s success\n", arg);
                safe_append(result, SEND_BUFF, cwd); // Append success message
            }
            arg = strtok(NULL, " \t\n"); // Get next directory name
        }
    }
    else if (strcmp(cmd, "RNFR") == 0) {
        arg = strtok(NULL, " \t\n");      // Get old name
        arg2 = strtok(NULL, " \t\n");     // Get RNTO token
        if (arg == NULL || arg2 == NULL || strcmp(arg2, "RNTO") != 0) {
            safe_append(result, SEND_BUFF, "Error: RNFR/RNTO usage\n");
        }
        else {
            arg2 = strtok(NULL, " \t\n"); // Get new name
            if (arg2 == NULL || strtok(NULL, " \t\n") != NULL) {
                safe_append(result, SEND_BUFF, "Error: RNFR/RNTO usage\n");
            }
            else if (rename(arg, arg2) < 0) {
                safe_append(result, SEND_BUFF, "Error: rename failed\n");
            }
            else {
                safe_append(result, SEND_BUFF, "RNFR RNTO success\n");
            }
        }
    }
    else {
        safe_append(result, SEND_BUFF, "Unknown command\n"); // Unknown FTP command
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// handle_client                                                             //
// ========================================================================== //
// Input : int connfd -> connected socket descriptor                          //
// Output: void                                                               //
// Purpose: Receive commands from one client and send results                 //
///////////////////////////////////////////////////////////////////////////////
void handle_client(int connfd)
{
    int n;                                // Number of received bytes
    char buff[MAX_BUFF];                  // Receive buffer
    char result[SEND_BUFF];               // Response buffer

    while (1) {
        memset(buff, 0, sizeof(buff));     // Clear receive buffer
        memset(result, 0, sizeof(result)); // Clear response buffer

        n = read(connfd, buff, MAX_BUFF - 1); // Receive FTP command
        if (n <= 0) {
            break;                        // Stop if client disconnected
        }
        buff[n] = '\0';                   // Terminate command string

        if (cmd_process(buff, result) == 1) {
            send_all(connfd, result, strlen(result)); // Send QUIT response
            break;                        // Terminate child loop
        }

        if (strlen(result) == 0) {
            strcpy(result, "OK\n");       // Set default response
        }

        if (send_all(connfd, result, strlen(result)) < 0) {
            break;                        // Stop on send failure
        }
    }

    close(connfd);                        // Close connected socket
}

///////////////////////////////////////////////////////////////////////////////
// main                                                                      //
// ========================================================================== //
// Input : int argc    -> argument count                                      //
//         char** argv -> server port                                         //
// Output: int        -> 0 success                                            //
// Purpose: Main routine of concurrent FTP server                             //
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    int listenfd;                          // Listening socket descriptor
    int connfd;                            // Connected socket descriptor
    int opt = 1;                           // Socket option value
    pid_t pid;                             // Child process ID
    struct sockaddr_in servaddr;           // Server address structure
    struct sockaddr_in cliaddr;            // Client address structure
    socklen_t clilen;                      // Client address length

    if (argc != 2) {
        write(STDERR_FILENO, "Usage: ./srv <PORT>\n", 20); // Print usage
        exit(1);
    }

    signal(SIGCHLD, sigchld_handler);      // Register SIGCHLD handler
    signal(SIGALRM, sigalrm_handler);      // Register SIGALRM handler
    signal(SIGINT, sigint_handler);        // Register SIGINT handler

    listenfd = socket(AF_INET, SOCK_STREAM, 0); // Create listening socket
    if (listenfd < 0) {
        perror("socket");                  // Print socket error
        exit(1);
    }
    g_listenfd = listenfd;                  // Save listening socket globally

    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // Reuse port

    memset(&servaddr, 0, sizeof(servaddr)); // Initialize server address
    servaddr.sin_family = AF_INET;          // Set IPv4 family
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // Accept all interfaces
    servaddr.sin_port = htons(atoi(argv[1])); // Set server port

    if (bind(listenfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");                    // Print bind error
        close(listenfd);                    // Close listening socket
        exit(1);
    }

    if (listen(listenfd, LISTENQ) < 0) {
        perror("listen");                  // Print listen error
        close(listenfd);                    // Close listening socket
        exit(1);
    }

    alarm(10);                              // Start process information timer

    while (1) {
        clilen = sizeof(cliaddr);           // Set client address length
        connfd = accept(listenfd, (SA *)&cliaddr, &clilen); // Accept client
        if (connfd < 0) {
            if (errno == EINTR) {
                continue;                   // Retry accept after signal
            }
            perror("accept");              // Print accept error
            continue;
        }

        pid = fork();                       // Create child process
        if (pid < 0) {
            perror("fork");                // Print fork error
            close(connfd);                  // Close connected socket
            continue;
        }
        else if (pid == 0) {
            close(listenfd);                // Child does not need listen socket
            handle_client(connfd);          // Process one client
            exit(0);                        // Terminate child process
        }
        else {
            close(connfd);                  // Parent does not need connected socket
            add_client(pid, ntohs(cliaddr.sin_port)); // Add child process info
            client_info(&cliaddr, pid);     // Print connected client info
            print_process_info();           // Print current process table
            alarm(10);                      // Reset 10 second interval
        }
    }

    close(listenfd);                        // Close listening socket
    return 0;
}
