#include "basicc.h"
#include "parse.h"
#include "hash.h"

#define STATEMENTS_SIZE 50

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
    FILE * src = fopen(argv[1], "r");
    if (!src) {
        perror("ERROR");
        exit(1);
    }
    introduce_s_table();
    bool loop = true;
    statements = (AST**)malloc(sizeof(AST*) * STATEMENTS_SIZE);
    for (int  i = 0; i < STATEMENTS_SIZE; i++)
    {
        statements[i] = NULL;
    }

    int i = 0;
    while (true)
    {
        loop = LineToTokens(src);
        if (!loop) break;
        statements[i] = parse_AST();
        i++;
    }
    sortAST(statements,0,i-1);
    for (int j = 0; j < i; j++)
    {
        printParsedLine(statements[j]);
    }

    make_target_src();

    for (int j = 0; j < i; j++)
    {
        if (statements[j] != NULL) FreeAST(statements[j]);
    }
        

    free(statements);
    freeTokensArr();
    free_s_table();
    fclose(src);

    return 0;
}
