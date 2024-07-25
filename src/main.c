#include "basicc.h"
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
    FILE * src = fopen(argv[1], "r");
    if (!src) {
        perror("ERROR");
        exit(1);
    }
    introduce_s_table();
    bool loop = true;
    AST* statements[50] = {NULL};
    
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
    for (int j = 0; j < i; j++)
    {
        if (statements[j] != NULL) FreeAST(statements[j]);
    }
    
    // make_target_src();

    freeTokensArr();
    free_s_table();
    fclose(src);

    return 0;
}
