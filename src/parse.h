#ifndef PARSE_H_
#define PARSE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_TOKENS_IN_LINE 20
#define TOKEN_LEN 20

extern char ** tokens;

typedef struct exp AST;
typedef enum wordtype {
    WT_CHAR,
    WT_OPER,
    WT_NUM,
    WT_ETC,
    WT_QUOTES,
    WT_SPACE,
    WT_NEWLINE,
    WT_PARENTHESIS, //left and right are needede
    WT_NULL
} wt;




typedef struct exp
{
    enum {
        tag_int,
        tag_str,
        tag_call, // function
        tag_var,
        tag_unary,
        tag_binary,
        tag_assign, // no keyword 
        tag_statement,
        tag_numline
    } tag;  
    union
    {
        int intExp;
        char* strExp;
        char * varExp;
        struct {
            struct exp* left;
            struct exp* right;
        } assignExp;
        struct {
            int value;
            struct exp * next;
        } numline;
        struct {
            struct exp* left;
            struct exp* right;
            char * operator;
        } binaryExp;
        struct {
            struct exp* operand;
            char * operator;
        } unaryExp;
        struct {
            char * name;
            struct exp* arguments;
        } callExp;
        struct 
        {
            char * name;
            struct exp* identifier;
            struct exp* value;
        } statementExp;
        

    }oper;
} AST;


void FillTokenArray(FILE * in);
FILE * OpenFile(const char* arg);
FILE * CreateFile(const char* arg);
void TokensToLinePrint();
AST * MakeBinaryExp(AST* left, AST* right);
AST * MakeUnaryExp(AST* operand);
AST * MakeIntExp();
AST * MakeStrExp();
AST * MakeCallExp();
AST * MakeAssignExp(AST* left, AST* right);
AST * MakeAST();
void freeTokensArr();
void allocTokensArr();
void get_next_token();
void recursivePrintAST(AST* ast);
char * cur_token();
char * next_token();
AST * parse_expression();

#endif