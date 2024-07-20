#include "parse.h"

int main(int argc, char const *argv[])
{
    // if (argc == 1) {
    //     perror("No argument");
    //     exit(1);
    // }
    FILE * src = OpenFile("a.bas");
    FILE * tar = CreateFile("output.c");
    FillTokenArray(src);
    AST * ast = MakeAST();
    recursivePrintAST(ast);


    return 0;
}
