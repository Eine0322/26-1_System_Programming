///////////////////////////////////////////////////////////////////////////////
// File Name : srv.c
// Date : 2026/04/17
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyun-ji
// Student ID : 2024402055
// --------------------------------------------------------------------
// Title : System Programming Assignment #1-3 (FTP server)
// Description : This program implements a simple FTP server
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 1024

///////////////////////////////////////////////////////////////////////////////////////
// write_str
// ===================================================================
// Input: const char *str -> string to print
// Output: void
// Purpose: print string using write system call
///////////////////////////////////////////////////////////////////////////////////////
void write_str(const char *str) {
    write(STDOUT_FILENO, str, strlen(str));
}

///////////////////////////////////////////////////////////////////////////////////////
// execute_command
// ===================================================================
// Input: char *buffer -> command string from client
// Output: void
// Purpose: parse and execute FTP commands
///////////////////////////////////////////////////////////////////////////////////////
void execute_command(char *buffer)
{
    char *cmd;
    char *arg;
    struct stat st;

    // ===================== command parsing =====================
    cmd = strtok(buffer, " ");
    if (cmd == NULL) return;         // extract command
    arg = strtok(NULL, " ");           // extract argument

    ///////////////////////////////////////////////////////////////////////
    // PWD : print working directory
    ///////////////////////////////////////////////////////////////////////
    if (strcmp(cmd, "PWD") == 0) {

        char cwd[BUF_SIZE];

        if (getcwd(cwd, sizeof(cwd)) == NULL) { // get current directory
            write(2, "PWD error\n", 10);
            return;
        }

        write_str(cwd);   // print directory
        write_str("\n");
    }

    ///////////////////////////////////////////////////////////////////////
    // CWD : change directory
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "CWD") == 0) {

        if (arg == NULL) { // check argument
            write(2, "Error: No directory name\n", 25);
            return;
        }

        if (chdir(arg) == -1) { // change directory
            write(2, "CWD error\n", 10);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // CDUP : move to parent directory
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "CDUP") == 0) {

        if (chdir("..") == -1) {
            write(2, "CDUP error\n", 11);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // MKD : create directory
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "MKD") == 0) {

        if (arg == NULL) {
            write(2, "Error: missing argument\n", 23);
            return;
        }

        while (arg != NULL) {
            if (mkdir(arg, 0755) == -1) { // create directory
                write(2, "MKD error\n", 10);
            }
            arg = strtok(NULL, " ");
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // DELE : delete file
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "DELE") == 0) {

        if (arg == NULL) {
            write(2, "Error: missing argument\n", 23);
            return;
        }

        while (arg != NULL) {

            if (stat(arg, &st) == -1) { // check file
                write(2, "stat error\n", 11);
            }
            else if (S_ISDIR(st.st_mode)) { // if directory → error
                write(2, "Error: cannot delete directory\n", 31);
            }
            else {
                if (remove(arg) == -1) { // remove file
                    write(2, "DELE error\n", 11);
                }
            }
            arg = strtok(NULL, " ");
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // RMD : remove directory
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "RMD") == 0) {

        if (arg == NULL) {
            write(2, "Error: missing argument\n", 23);
            return;
        }

        while (arg != NULL) {

            if (stat(arg, &st) == -1) {
                write(2, "stat error\n", 11);
            }
            else if (!S_ISDIR(st.st_mode)) { // not directory
                write(2, "Error: not directory\n", 21);
            }
            else {
                if (rmdir(arg) == -1) { // remove directory
                    write(2, "RMD error\n", 10);
                }
            }
            arg = strtok(NULL, " ");
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // RNFR / RNTO : rename file
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "RNFR") == 0) {

        char *old = arg;                     // old name
        char *rnto = strtok(NULL, " ");      // should be RNTO
        char *new = strtok(NULL, " ");       // new name

        if (old == NULL || rnto == NULL || new == NULL || strcmp(rnto, "RNTO") != 0) {
            write(2, "Error: RNFR syntax\n", 19);
            return;
        }

        if (rename(old, new) == -1) {
            write(2, "rename error\n", 13);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // NLST / LIST : list directory
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "NLST") == 0 || strcmp(cmd, "LIST") == 0) {

        DIR *dp;
        struct dirent *entry;

        char *files[BUF_SIZE];
        int count = 0;

        int opt_a = 0, opt_l = 0;
        char *path = ".";

        // -------- option parsing --------
        while (arg != NULL) {

            if (arg[0] == '-') {
                if (strcmp(arg, "-a") == 0) opt_a = 1;
                else if (strcmp(arg, "-l") == 0) opt_l = 1;
                else if (strcmp(arg, "-al") == 0 || strcmp(arg, "-la") == 0) {
                    opt_a = 1; opt_l = 1;
                }
                else {
                    write(2, "Error: invalid option\n", 22);
                    return;
                }
            } else {
                path = arg; // directory path
            }
            arg = strtok(NULL, " ");
        }

        if (strcmp(cmd, "LIST") == 0) { // LIST = -al
            opt_a = 1; opt_l = 1;
        }


        // -------- path validation --------
        if (stat(path, &st) == -1) {
            write(2, "Error: invalid path\n", 20);
            return;
        }

        // -------- file case --------
        if (!S_ISDIR(st.st_mode)) {
            write_str(path);
            write_str("\n");
            return;
        }

        // -------- open directory --------
        dp = opendir(path);
        if (dp == NULL) {
            write(2, "Error: cannot open directory\n", 29);
            return;
        }

        // -------- read entries --------
        while ((entry = readdir(dp)) != NULL) {
            if (!opt_a && entry->d_name[0] == '.') continue;
            if (count >= BUF_SIZE) break; // prevent overflow
            files[count++] = strdup(entry->d_name);
        }

        closedir(dp);

        // -------- sort (ASCII) --------
        for (int i = 0; i < count - 1; i++) {
            for (int j = i + 1; j < count; j++) {
                if (strcmp(files[i], files[j]) > 0) {
                    char *tmp = files[i];
                    files[i] = files[j];
                    files[j] = tmp;
                }
            }
        }

        // -------- print --------
        int printed = 0;

        for (int i = 0; i < count; i++) {

            char fullpath[BUF_SIZE];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, files[i]);

            if (stat(fullpath, &st) == -1) continue;

            char name[BUF_SIZE];
            strcpy(name, files[i]);

            if (S_ISDIR(st.st_mode)) strcat(name, "/");

            if (opt_l) {
                write_str(name);
                write_str("\n");
            }
            else {
                char buf[BUF_SIZE];
                snprintf(buf, sizeof(buf), "%-20s", name);
                write_str(buf);

                printed++;
                if (printed % 5 == 0) write_str("\n");
            }
        }

        if (!opt_l && printed % 5 != 0) write_str("\n");

        for (int i = 0; i < count; i++) free(files[i]);
    }

    ///////////////////////////////////////////////////////////////////////
    // RETR : get file (server → client)
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "RETR") == 0) {

        if (arg == NULL) {
            write(2, "Error: No file name\n", 20);
            return;
        }

        char src[BUF_SIZE], dst[BUF_SIZE];
        snprintf(src, sizeof(src), "server_root/%s", arg);
        snprintf(dst, sizeof(dst), "client_root/%s", arg);

        int fd_src = open(src, O_RDONLY);           // open source file
        if (fd_src < 0) {
            write(2, "Error: file not found\n", 22);
            return;
        }

        int fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_dst < 0) {
            write(2, "Error: cannot create file\n", 26);
            close(fd_src);
            return;
        }

        char buf[BUF_SIZE];
        int n;

        while ((n = read(fd_src, buf, BUF_SIZE)) > 0) {
            write(fd_dst, buf, n);   // copy data
        }

        close(fd_src);
        close(fd_dst);

        write_str("RETR success\n");
    }

    ///////////////////////////////////////////////////////////////////////
    // STOR : put file (client → server)
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "STOR") == 0) {

        if (arg == NULL) {
            write(2, "Error: No file name\n", 20);
            return;
        }

        char src[BUF_SIZE], dst[BUF_SIZE];
        snprintf(src, sizeof(src), "client_root/%s", arg);
        snprintf(dst, sizeof(dst), "server_root/%s", arg);

        int fd_src = open(src, O_RDONLY);
        if (fd_src < 0) {
            write(2, "Error: file not found\n", 22);
            return;
        }

        int fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_dst < 0) {
            write(2, "Error: cannot create file\n", 26);
            close(fd_src);
            return;
        }

        char buf[BUF_SIZE];
        int n;

        while ((n = read(fd_src, buf, BUF_SIZE)) > 0) {
            write(fd_dst, buf, n);
        }

        close(fd_src);
        close(fd_dst);

        write_str("STOR success\n");
    }

    ///////////////////////////////////////////////////////////////////////
    // QUIT : terminate server
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "QUIT") == 0) {
        write_str("QUIT\n");
        exit(0);
    }

    else {
        write(2, "Unknown command\n", 16);
    }
}

///////////////////////////////////////////////////////////////////////////////////////
// main
// ===================================================================
// Input: none
// Output: int
// Purpose: receive command and execute repeatedly
///////////////////////////////////////////////////////////////////////////////////////
int main()
{
    char buffer[BUF_SIZE];

    while (1) {
        memset(buffer, 0, BUF_SIZE);

        int n = read(STDIN_FILENO, buffer, BUF_SIZE); // read command
        if (n == 0) break; // EOF
        if (n < 0) continue;
        
        buffer[n] = '\0';                      // null-terminate
        buffer[strcspn(buffer, "\n")] = 0;    // remove newline

        execute_command(buffer);              // execute command
    }

    return 0;
}