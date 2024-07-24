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
    AST *arr[50];
    for (int i = 0; i < 50; i++)
    {
        arr[50] = NULL;
    }
    
    int i = 0;
    while (true)
    {
        loop = LineToTokens(src);
        if (!loop) break;
        arr[i] = MakeAST();
        i++;
    }
    sortAST(arr,0,i-1);
    for (int j = 0; j < i; j++)
    {
        printParsedLine(arr[j]);
    }
    


    freeTokensArr();
    free_s_table();
    fclose(src);
    fclose(tar);

    return 0;
}
