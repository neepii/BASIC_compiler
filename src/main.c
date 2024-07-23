#include "parse.h"
#include "hash.h"

int main(int argc, char  *argv[])
{
    if (argc == 1) {
        printf("No arguments\n");
        exit(1);
    }
    if (match(argv[1], "--test-hash")) {
        test_hashes_on_keywords();
        return 0;
    }
    FILE * src = OpenFile(argv[1]);
    if (!src) {
        perror("ERROR");
        exit(1);
    }
    FILE * tar = CreateFile("output.c");
    introduce_s_table();
    bool loop = true;
    while (true)
    {
        loop = LineToTokens(src);
        if (!loop) break;
        TokensToLinePrint();
        AST * ast = MakeAST();
        printParsedLine(ast);
        FreeAST(ast);
    }

    return 0;
}
