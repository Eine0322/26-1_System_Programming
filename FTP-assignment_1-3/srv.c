///////////////////////////////////////////////////////////////////////////////
// File Name : srv.c
// Date : 2026/04/10
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyun-ji
// Student ID : 2024402055
// --------------------------------------------------------------------
// Title : System Programming Assignment #1-3 (FTP server)
// Description : This program implements a simple FTP server that handles FTP protocol commands
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#define BUF_SIZE 1024

void execute_command(char *buffer)
{
    char *cmd;
    char *arg;

    struct stat st;

    cmd = strtok(buffer, " ");
    arg = strtok(NULL, " ");

    // =========================
    // PWD
    // =========================
    if (strcmp(cmd, "PWD") == 0) {
        char cwd[BUF_SIZE];

        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("PWD error");
            return;
        }

        write(STDOUT_FILENO, cwd, strlen(cwd));
        write(STDOUT_FILENO, "\n", 1);
    }

    // =========================
    // CWD
    // =========================
    else if (strcmp(cmd, "CWD") == 0) {
        if (arg == NULL) { // check if directory name is provided
            write(2, "Error: No directory name\n", 25);
            return;
        }

        if (chdir(arg) == -1) { // check if directory name is provided and change directory
            perror("CWD error");
            return;
        }
    }

    // =========================
    // CDUP
    // =========================
    else if (strcmp(cmd, "CDUP") == 0) {
        if (chdir("..") == -1) {
            perror("CDUP error");
            return;
        }
    }

    // =========================
    // MKD
    // =========================
    else if (strcmp(cmd, "MKD") == 0) {
         if (arg == NULL) {
            write(2, "Error: missing argument\n", 23);
            return;
        }

        while (arg != NULL) {
            if (mkdir(arg, 0755) == -1) {
                perror("MKD error");
            }
            arg = strtok(NULL, " ");
        }
    }

    // =========================
    // DELE
    // =========================
    else if (strcmp(cmd, "DELE") == 0) {
        if (arg == NULL) {
            write(2, "Error: missing argument\n", 23);
            return;
        }

        while (arg != NULL) {
            if (stat(arg, &st) == -1) {
                perror("stat error");
            }
            else if (S_ISDIR(st.st_mode)) {
                write(2, "Error: cannot delete a directory\n", 32);
            }
            else {
                if (remove(arg) == -1) {
                    perror("DELE error");
                }
            }
            arg = strtok(NULL, " ");
        }
    }

    // =========================
    // RMD
    // =========================
    else if (strcmp(cmd, "RMD") == 0) {
        if (arg == NULL) {
            write(2, "Error: missing argument\n", 23);
            return;
        }

        while (arg != NULL) {
            if (stat(arg, &st) == -1) {
                perror("stat error");
            }
            else if(!S_ISDIR(st.st_mode)) {
                write(2, "Error: cannot remove a file\n", 27);
            }
            else {
                if (rmdir(arg)== -1) {
                    perror("RMD error");
                }
            }
            arg = strtok(NULL, " ");
        }
    }

    // =========================
    // RNFR + RNTO
    // =========================
    else if (strcmp(cmd, "RNFR") == 0) {
        if (arg == NULL) { // none A
            write(2, "Error: missing old name argument\n", 31);
            return;
        }
        else {
            char* old = arg;
            strtok(NULL, " "); // skip "RNTO"
            char* new = strtok(NULL, " ");
            if (new == NULL) { // none B
                write(2, "Error: missing new name argument\n", 31);
                return;
            }
            else if (rename(old, new) == -1) {
                perror("RNFR/RNTO error");
                return;
            }
        }   
    }

    // =========================
    // NLST / LIST
    // =========================
    else if (strcmp(cmd, "NLST") == 0 || strcmp(cmd, "LIST") == 0) {

        DIR *dp;
        struct dirent *entry;

        // TODO
        // opendir(arg1 or ".")
        // readdir 반복 출력
        // LIST는 자세히, NLST는 이름만 (간단히 이름만 해도 됨)
    }

    // =========================
    // RETR (get)
    // =========================
    else if (strcmp(cmd, "RETR") == 0) {

        // TODO (간단 버전)
        // 파일 존재 확인만 해도 충분한 과제 많음
    }

    // =========================
    // STOR (put)
    // =========================
    else if (strcmp(cmd, "STOR") == 0) {

        // TODO (간단 버전)
        // 파일 생성 정도만
    }

    // =========================
    // QUIT
    // =========================
    else if (strcmp(cmd, "QUIT") == 0) {
        exit(0);
    }

    else {
        write(2, "Unknown command\n", 16);
    }
}

int main()
{
    char buffer[BUF_SIZE];

    while (1) {
        memset(buffer, 0, BUF_SIZE);

        // 👉 cli에서 넘어온 명령 입력받는다고 가정
        read(STDIN_FILENO, buffer, BUF_SIZE);

        // 개행 제거
        buffer[strcspn(buffer, "\n")] = 0;

        execute_command(buffer);
    }

    return 0;
}