#include "parse.h"

int main(int argc, char const *argv[])
{
    if (argc == 1) {
        perror("No argument");
        exit(1);
    }
    FILE * src = OpenFile(argv[1]);
    FILE * tar = OpenFile("output.c");
    char tokens[20][20];
    FillTokenArray(src, tokens);

    int i = 0;
    while (tokens[i][0]) {
        printf("%s\n", tokens[i]);
        i++;
    }
    return 0;
}
