#include "parse.h"

int main(int argc, char const *argv[])
{
    if (argc == 1) {
        perror("No argument");
        exit(1);
    }
    FILE * src = OpenFile(argv[1]);
    FILE * tar = CreateFile("output.c");
    FillTokenArray(src);
    TokensToLinePrint();
    AST * ast = MakeAST();
    recursivePrintAST(ast);


    return 0;
}
