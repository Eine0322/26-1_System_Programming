#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int aflag = 0, bflag = 0;
    char *cvalue = NULL;
    int c;

    opterr = 0;

    // 옵션 처리
    while ((c = getopt(argc, argv, "abc:")) != -1)
    {
        switch (c)
        {
            case 'a':
                aflag++;
                break;
            case 'b':
                bflag++;
                break;
            case 'c':
                cvalue = optarg;
                break;
            case '?':
                printf("Unknown option: %c\n", optopt);
                break;
        }
    }

    printf("aflag = %d, bflag = %d, cvalue = %s\n",
           aflag, bflag, cvalue);

    // 남은 인자 처리
    for (int index = optind; index < argc; index++)
    {
        printf("Non-option argument %s\n", argv[index]);
    }

    return 0;
}