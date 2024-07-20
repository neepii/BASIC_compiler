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
    WT_ASCII,
    WT_OPER,
    WT_NUM,
    WT_ETC,
    WT_QUOTES,
    WT_PARENTHESIS,
    WT_NULL
} wt;


typedef struct exp_list
{
    AST * ast;
    struct exp_list * next;
} EXP_LIST;




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
        } nulline;
        struct {
            struct exp* left;
            struct exp* right;
            char operator;
        } binaryExp;
        struct {
            struct exp* operand;
            char operator;
        } unaryExp;
        struct {
            char * name;
            EXP_LIST *  argument;
        } callExp;

    }oper;
} AST;

int FillTokenArray(FILE * in);
FILE * OpenFile(const char* arg);
FILE * CreateFile(const char* arg);
void TokensToLinePrint();
AST * MakeBinaryExp(char operator, AST* left, AST* right);
AST * MakeUnaryExp(char operator, AST* operand);
AST * MakeIntExp(int value);
AST * MakeStrExp(char* str);
AST * MakeCallExp(char * name, EXP_LIST* arg);
AST * MakeAssignExp(AST* left, AST* right);
EXP_LIST * MakeExpList( int ind, int end);
AST * MakeAST( int lvl, int end);
void freeTokensArr();
void allocTokensArr();


#endif