#include "parse.h"

int main(int argc, char const *argv[])
{
    if (argc == 1) {
        perror("No argument");
        exit(1);
    }
    FILE * src = OpenFile(argv[1]);
    FILE * tar = CreateFile("output.c");
    char tokens[20][20];
    int count = FillTokenArray(src);
    TokensToLinePrint();
    printf("%d\n", count);

    return 0;
}
