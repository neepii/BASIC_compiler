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
    WT_PARENTHESIS, //left and right are needede
    WT_NULL
} wt;


// typedef struct exp_list
// {
//     AST * ast;
//     struct exp_list * next;
// } EXP_LIST;




typedef struct exp
{
    enum {
        tag_int,
        tag_str,
        tag_call,
        tag_var,
        tag_unary,
        tag_binary,
        tag_assign,
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
            struct exp* argument;
        } callExp;

    }oper;
} AST;


void FillTokenArray(FILE * in);
FILE * OpenFile(const char* arg);
FILE * CreateFile(const char* arg);
void TokensToLinePrint();
AST * MakeBinaryExp(char * operator, AST* left, AST* right);
AST * MakeUnaryExp(char * operator, AST* operand);
AST * MakeIntExp(char* str);
AST * MakeStrExp(char* str);
AST * MakeCallExp(char * name);
AST * MakeAssignExp(AST* left, AST* right);
// EXP_LIST * MakeExpList();
AST * MakeAST();
void freeTokensArr();
void allocTokensArr();
char * get_next_token();
void recursivePrintAST(AST* ast);

#endif