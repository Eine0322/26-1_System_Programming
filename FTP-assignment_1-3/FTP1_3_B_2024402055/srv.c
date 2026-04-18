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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 1024

///////////////////////////////////////////////////////////////////////////////
// write_str                                                                 //
// ========================================================================= //
// Input: const char* -> String to be printed                                //
// Output: void                                                              //
// Purpose: Helper function to write a string to standard output             //
///////////////////////////////////////////////////////////////////////////////
void write_str(const char *str) {
    write(STDOUT_FILENO, str, strlen(str)); // Write string to STDOUT
}

///////////////////////////////////////////////////////////////////////////////
// execute_command                                                           //
// ========================================================================= //
// Input: char* -> Buffer containing the user command and arguments          //
// Output: void                                                              //
// Purpose: Parse the input buffer and execute the corresponding FTP command //
///////////////////////////////////////////////////////////////////////////////
void execute_command(char *buffer)
{
    char *cmd;
    char *arg;
    struct stat st;

    ///////////////////////////////// Parsing /////////////////////////////////
    cmd = strtok(buffer, " "); // Tokenize command
    if (cmd == NULL) return; // End if empty
    arg = strtok(NULL, " "); // Tokenize first argument
    
    if (arg != NULL){
        if (arg[0]=='-'){
            if (arg[1]!='a' && arg[1]!='l'){
                write_str("Error: invalid option\n"); // Validate options
                return;
            }
        }
    }
    ////////////////////////////// End of Parsing /////////////////////////////

    ///////////////////////////////////////////////////////////////////////
    // PWD
    ///////////////////////////////////////////////////////////////////////
    if (strcmp(cmd, "PWD") == 0) {

        if (arg != NULL) {
            write_str("Error: argument is not required\n"); // No args for PWD
            return;
        }

        char cwd[BUF_SIZE];

        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            write_str("Error: PWD error\n"); // Handle getcwd failure
            return;
        }

        write_str(cwd); // Output current directory
        write_str("\n");
    }

    ///////////////////////////////////////////////////////////////////////
    // CWD
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "CWD") == 0) {

        if (arg == NULL) {
            write_str("Error: argument is required\n"); // Arg required for CWD
            return;
        }

        if (stat(arg, &st) == -1) {
            write_str("Error: directory not found\n"); // Check path existence
            return;
        }

        if (chdir(arg) == -1) {
            write_str("Error: cannot access\n"); // Change directory
            return;
        }

        char cwd[BUF_SIZE];
        getcwd(cwd, sizeof(cwd)); // Get new path
        write_str("CWD\t");
        write_str(arg);
        write_str("\n");
        write_str(cwd);
        write_str("\n");
    }

    ///////////////////////////////////////////////////////////////////////
    // CDUP
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "CDUP") == 0) {

        if (chdir("..") == -1) {
            write(2, "CDUP error\n", 11); // Move to parent directory
            return;
        }

        char cwd[BUF_SIZE];
        getcwd(cwd, sizeof(cwd));
        write_str("CDUP\n");
        write_str(cwd);
        write_str("\n");
    }

    ///////////////////////////////////////////////////////////////////////
    // MKD
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "MKD") == 0) {

        if (arg == NULL) {
            write_str("Error: argument is required\n");
            return;
        }

        ///////////////////////////// Directory Loop //////////////////////////
        while (arg != NULL) {
            if (mkdir(arg, 0755) == -1) {
                write_str("Error: cannot create directory: File exists\n");
            }
            else{
                write_str("MKD\t");
                write_str(arg);
                write_str("\n");
            }

            arg = strtok(NULL, " "); // Handle multiple MKD arguments
        }
        ////////////////////////// End of Directory Loop //////////////////////
    }

    ///////////////////////////////////////////////////////////////////////
    // DELE
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "DELE") == 0) {

        if (arg == NULL) {
            write_str("Error: argument is required\n");
            return;
        }

        while (arg != NULL) {

            if (stat(arg, &st) == -1) {
                write_str("Error: file not found ");
                write_str(arg);
                write_str("\n");
            }
            else if (S_ISDIR(st.st_mode)) {
                write_str("Error: cannot delete directory "); // DELE is for files
                write_str(arg);
                write_str("\n");
            }
            else {
                if (remove(arg) == -1) {
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

            arg = strtok(NULL, " ");
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // RMD
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "RMD") == 0) {

        if (arg == NULL) {
            write_str("Error: argument is required\n");
            return;
        }

        while (arg != NULL) {
            if (stat(arg, &st) == -1) {
                write_str("Error: directory not found ");
                write_str(arg);
                write_str("\n");
            }
            else if (!S_ISDIR(st.st_mode)) {
                write_str("Error: not directory\n"); // RMD is for directories
            }
            else {
                if (rmdir(arg) == -1) {
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
    // RNFR / RNTO
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "RNFR") == 0) {

        char *old = arg; // Original filename
        char *rnto = strtok(NULL, " ");
        char *new = strtok(NULL, " "); // Target filename

        if (old == NULL || rnto == NULL || new == NULL || strcmp(rnto, "RNTO") != 0) {
            write_str("Error: two arguments are required\n");
            return;
        }

        if (access(new, F_OK) == 0) {
            write_str("Error: name to change already exists\n"); // Check conflict
            return;
        }

        if (rename(old, new) == -1) {
            write_str("Error: rename error\n"); // Execute rename
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
    // NLST / LIST
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "NLST") == 0 || strcmp(cmd, "LIST") == 0) {

        DIR *dp;
        struct dirent *entry;

        char *files[BUF_SIZE];
        int count = 0;

        int opt_a = 0, opt_l = 0;
        char *path = ".";

        ////////////////////////// Option Parsing /////////////////////////////
        while (arg != NULL) {

            if (arg[0] == '-') {
                if (strcmp(arg, "-a") == 0){
                    opt_a = 1; // Hidden files option
                }
                else if (strcmp(arg, "-l") == 0) {
                    opt_l = 1; // Long format option
                }
                else if (strcmp(arg, "-al") == 0 || strcmp(arg, "-la") == 0) {
                    opt_a = 1; opt_l = 1;
                }
            }
            else {
                path = arg; // Set target path
            }
            arg = strtok(NULL, " ");
        }
        /////////////////////// End of Option Parsing /////////////////////////

        if (strcmp(cmd, "LIST") == 0) {
            opt_a = 1;
            opt_l = 1;
            write_str("LIST\n");
        }

        if (strcmp(cmd,"NLST ")==0){
            if(opt_a&&opt_l){
                write_str("-al\n");
            }
            else if(opt_a){
                write_str("-a\n");
            }
            else if(opt_l){
                write_str("-l\n");
            }
        }

        if (stat(path, &st) == -1) {
            write_str("Error: No such file or directory\n");
            return;
        }

        dp = opendir(path); // Open directory stream
        if (dp == NULL) {
            write_str("Error: cannot access\n");
            return;
        }

        while ((entry = readdir(dp)) != NULL) {
            if (!opt_a && entry->d_name[0] == '.') continue; // Skip hidden
            files[count++] = strdup(entry->d_name); // Store filenames
        }

        closedir(dp);

        //////////////////////////// Sort Files ///////////////////////////////
        for (int i = 0; i < count - 1; i++) {
            for (int j = i + 1; j < count; j++) {
                if (strcmp(files[i], files[j]) > 0) {
                    char *tmp = files[i];
                    files[i] = files[j];
                    files[j] = tmp;
                }
            }
        }
        ///////////////////////// End of Sort Files ///////////////////////////

        int printed = 0;

        for (int i = 0; i < count; i++) {

            char fullpath[BUF_SIZE];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, files[i]);

            if (stat(fullpath, &st) == -1) continue;

            char name[BUF_SIZE];
            strcpy(name, files[i]);

            if (S_ISDIR(st.st_mode)) strcat(name, "/"); // Mark directory

            if (opt_l) {
                write_str(name);
                write_str("\n");
            }
            else {
                char buf[BUF_SIZE];
                snprintf(buf, sizeof(buf), "%-20s", name);
                write_str(buf);

                printed++;
                if (printed % 5 == 0) write_str("\n"); // Format columns
            }
        }

        if (!opt_l && printed % 5 != 0) write_str("\n");

        for (int i = 0; i < count; i++) free(files[i]); // Clean memory
    }

    ///////////////////////////////////////////////////////////////////////
    // RETR
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "RETR") == 0) {

        if (arg == NULL) {
            write_str("Error: argument is required\n");
            return;
        }

        char src[BUF_SIZE], dst[BUF_SIZE];
        snprintf(src, sizeof(src), "server_root/%s", arg);
        snprintf(dst, sizeof(dst), "client_root/%s", arg);

        int fd_src = open(src, O_RDONLY); // Open source
        if (fd_src < 0) {
            write_str("Error: does not exist in server root\n");
            return;
        }

        int fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Open dest
        if (fd_dst < 0) {
            write_str("Error: cannot create file\n");
            close(fd_src);
            return;
        }

        char buf[BUF_SIZE];
        int n;
        int total=0;

        /////////////////////////// File Transfer /////////////////////////////
        while ((n = read(fd_src, buf, BUF_SIZE)) > 0) {
            write(fd_dst, buf, n); // Read from server, write to client
            total += n;
        }
        //////////////////////// End of File Transfer /////////////////////////

        close(fd_src);
        close(fd_dst);

        write_str("RETR\t");
        write_str(arg);
        write_str("\n");

        char msg[BUF_SIZE];
        snprintf(msg, sizeof(msg),
                "OK : %d bytes copied to client_root (expected %d bytes)\n",
                total, total);
        write_str(msg);
    }

    ///////////////////////////////////////////////////////////////////////
    // STOR
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "STOR") == 0) {

        if (arg == NULL) {
            write_str("Error: argument is required\n");
            return;
        }

        char src[BUF_SIZE], dst[BUF_SIZE];
        snprintf(src, sizeof(src), "client_root/%s", arg);
        snprintf(dst, sizeof(dst), "server_root/%s", arg);

        int fd_src = open(src, O_RDONLY);
        if (fd_src < 0) {
            write_str("Error: does not exist in client root\n");
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
        int total=0;

        /////////////////////////// File Upload ///////////////////////////////
        while ((n = read(fd_src, buf, BUF_SIZE)) > 0) {
            write(fd_dst, buf, n); // Read from client, write to server
            total += n;
        }
        //////////////////////// End of File Upload ///////////////////////////

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
    // QUIT
    ///////////////////////////////////////////////////////////////////////
    else if (strcmp(cmd, "QUIT") == 0) {

        if (arg != NULL) {
            write_str("Error: argument is not required\n");
            return;
        }

        write_str("QUIT success\n");
        exit(0); // Exit process
    }

    else {
        write_str("Unknown command\n"); // Command not supported
    }
}

///////////////////////////////////////////////////////////////////////////////
// main                                                                      //
// ========================================================================= //
// Input: void                                                               //
// Output: int -> Return 0 on success                                        //
// Purpose: Entry point for the server, handles continuous user input        //
///////////////////////////////////////////////////////////////////////////////
int main()
{
    char buffer[BUF_SIZE];

    //////////////////////////// Server Loop //////////////////////////////////
    while (1) {
        memset(buffer, 0, BUF_SIZE); // Reset buffer

        int n = read(STDIN_FILENO, buffer, BUF_SIZE); // Get user input
        if (n == 0) break; // EOF
        if (n < 0) continue; // Read error

        buffer[n] = '\0';
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline

        execute_command(buffer); // Process command
    }
    ///////////////////////// End of Server Loop //////////////////////////////

    return 0;
}