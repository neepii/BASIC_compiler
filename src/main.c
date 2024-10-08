#include "basicc.h"

#define STATEMENTS_SIZE 50
TAC_Entry * TacArr;
unsigned int TacInd = 0;
STACK *goto_s;
STACK_STR *float_inits;

int main(int argc, char  *argv[])
{

    char output_name[50] = "a.out";
    char source_name[50] = {0};
    tar_path_name = (char *) malloc(sizeof(char) * 50);
    strcpy(tar_path_name,"/tmp/XXX08080.s");
    init_nfa();
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
        else if(match(argv[i], "--test-regex") && (i + 1 <= argc)) {
            test_regex(argv[i+1]);
            free_nfa();
            return 0;
        }
        else if (match(argv[i], "-o") && argv[i+1]) {
            strcpy(output_name, argv[i+1]);
            i++;
        }
        else if (match(argv[i], "--debug")) {
            strcpy(tar_path_name, "XXX08080.s");
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
    goto_s = init_stack(0);
    float_inits = init_stack_str(0);
    int loop = true;
    statements = (AST**)malloc(sizeof(AST*) * STATEMENTS_SIZE);
    for (int  i = 0; i < STATEMENTS_SIZE; i++)
    {
        statements[i] = NULL;
    }
    int i = 0;
    while (true)
    {
        loop = LineToTokens(src);
        if (loop == 2) continue;
        else if (loop == 0) break;
        statements[i] = parse_AST();
        i++;
    }
    sortAST(statements,0,i-1);
    while (!stack_is_empty(goto_s)) {
        int num = pop_s(goto_s);
        AST * goto_stmt = bsearch_statements(statements, i, num);
        goto_stmt->oper.numline.isGotoLabel = true;
    }
    for (int j = 0; j < i; j++)
    {
        printParsedLine(statements[j]);
    }
    make_target_src();
    compile(output_name);

    for (int j = 0; j < i; j++)
    {    
        if (statements[j] != NULL) map_ast(statements[j], free);
    }
    free(statements);
    free(tar_path_name);
    free(marked);
    free(goto_s);
    free(float_inits);
    freeTokensArr();
    free_s_table();
    free_nfa();
    fclose(src);

    return 0;
}
