///////////////////////////////////////////////////////////////////////////////
// File Name : kw2024402055_opt.c
// Date : 2026/03/30
// OS : Ubuntu 20.04.6 LTS 64bits
// Author : Park Hyun-ji
// Student ID : 2024402055      
// --------------------------------------------------------------------
// Title : System Programming Assignment #1-1 (ftp server)
// Description : This program parses command-line options using getopt()
//               and prints the result based on given options.
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <getopt.h>
#include <unistd.h>

///////////////////////////////////////////////////////////////////////////////////////
// main
// ===================================================================
// Input: int argc -> number of input arguments
//        char **argv -> array of input arguments (options from user)
// Output: int -> returns 0 when program terminates normally
// Purpose: To process options (-a, -b, -c) and print corresponding results
///////////////////////////////////////////////////////////////////////////////////////
int main (int argc, char **argv)
{
    int aflag = 0, bflag = 0;
    char *cvalue = NULL;
    int index, c;

    opterr = 0;  // disable automatic error messages from getopt

    //////////////////////////// Option parsing //////////////////////////////
    while ((c = getopt (argc, argv, "abc:")) != -1)
    {
        switch (c)
        {
            case 'a':
                aflag++;   // increase count when -a option is used
                break;

            case 'b':
                bflag++;   // increase count when -b option is used
                break;

            case 'c':
                cvalue = optarg;   // store argument value of -c option
                break;

            case '?':
                // ignore undefined options
                break;
        }
    }
    //////////////////////////// End of option parsing //////////////////////////////

    // print result
    printf("aflag = %d, bflag = %d, cvalue = %s\n",
           aflag, bflag, cvalue);

    //////////////////////////// Non-option arguments //////////////////////////////
    for (index = optind; index < argc; index++)
    {
        printf("Non-option argument %s\n", argv[index]);  // print remaining arguments
    }
    //////////////////////////// End of non-option arguments //////////////////////////////

    return 0;
}