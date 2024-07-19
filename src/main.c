#include "parse.h"
#include "keywords.h"

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
    TokensToLinePrint(tokens);

    return 0;
}
