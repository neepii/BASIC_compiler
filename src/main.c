#include "parse.h"

int main(int argc, char const *argv[])
{
    // if (argc == 1) {
    //     perror("No argument");
    //     exit(1);
    // }
    FILE * src = OpenFile("a.bas");
    FILE * tar = CreateFile("output.c");
    bool loop = true;
    while (true)
    {
        loop = LineToTokens(src);
        if (!loop) break;
        AST * ast = MakeAST();
        printParsedLine(ast);
        FreeAST(ast);
    }

    return 0;
}
