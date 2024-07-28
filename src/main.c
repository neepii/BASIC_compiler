#include "basicc.h"

#define STATEMENTS_SIZE 50


int main(int argc, char  *argv[])
{
    char output_name[50] = "a.out";
    char source_name[50] = {0};
    temp_name = (char *) malloc(sizeof(char) * 50);
    strcpy(temp_name,"/tmp/XXX08080.s");
    if (argc == 1) {
        printf("No arguments\n");
        exit(1);
    }
    for (int i = 1; i < argc; i++)
    {
        if (match(argv[i], "--test-hash")) {
            test_hashes_on_keywords();
            return 0;
        }
        else if (match(argv[i], "-o") && argv[i+1]) {
            strcpy(output_name, argv[i+1]);
            i++;
        }
        else if (match(argv[i], "--debug")) {
            strcpy(temp_name, "XXX08080.s");
        }
        else {
            strcpy(source_name, argv[i]);
        }
    }
    
    if (!source_name[0]) {
        parse_error("no source file");
        exit(1);
    }
    FILE * src = fopen(source_name, "r");
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
    compile(output_name);

    for (int j = 0; j < i; j++)
    {
        if (statements[j] != NULL) FreeAST(statements[j]);
    }
        

    free(statements);
    free(temp_name);
    freeTokensArr();
    free_s_table();
    fclose(src);

    return 0;
}
