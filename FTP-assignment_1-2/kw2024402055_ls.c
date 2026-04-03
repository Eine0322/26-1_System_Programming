///////////////////////////////////////////////////////////////////////////////
// File Name : kw2024402055_ls.c
// Date : 2026/04/03
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyun-ji
// Student ID : 2024402055
// --------------------------------------------------------------------
// Title : System Programming Assignment #1-2 (ls)
// Description : This program implements a simple version of the 'ls' command.
//               It displays file names in a given directory using opendir(),
//               readdir(), and closedir(). It also handles various error cases.
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

///////////////////////////////////////////////////////////////////////////////////////
// main
// ===================================================================
// Input: int argc -> number of input arguments
//        char *argv[] -> array of input arguments (directory path)
// Output: int -> returns 0 when program terminates normally
// Purpose: To display file names in a directory and handle error cases
///////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    DIR *dp;                     // directory pointer
    struct dirent *dirp;         // directory entry structure
    char *path = ".";            // default path (current directory)

    //////////////////////////// Argument check //////////////////////////////
    if (argc > 2) {
        printf("only one directory path can be processed\n");
        return 1;
    }

    if (argc == 2) {
        path = argv[1];          // set user input path
    }
    //////////////////////////// End of argument check //////////////////////////////

    //////////////////////////// Path validation //////////////////////////////
    struct stat st;

    if (stat(path, &st) == -1) {
        printf("testls: cannot access '%s' : No such directory\n", path);
        return 1;
    }

    if (!S_ISDIR(st.st_mode)) {
        printf("testls: cannot access '%s' : No such directory\n", path);
        return 1;
    }
    //////////////////////////// End of path validation //////////////////////////////

    //////////////////////////// Open directory //////////////////////////////
    dp = opendir(path);

    if (dp == NULL) {
        if (errno == EACCES) {
            printf("testls: cannot access '%s' : Access denied\n", path);
        } else {
            printf("testls: cannot access '%s'\n", path);
        }
        return 1;
    }
    //////////////////////////// End of open directory //////////////////////////////

    //////////////////////////// Read directory //////////////////////////////
    while ((dirp = readdir(dp)) != NULL) {
        printf("%s\n", dirp->d_name);   // print file name
    }
    //////////////////////////// End of read directory //////////////////////////////

    closedir(dp);   // close directory stream

    return 0;
}