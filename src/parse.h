#ifndef PARSE_H_
#define PARSE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

int FillTokenArray(FILE * in, char tokens[20][20]);
FILE * OpenFile(const char* arg);
FILE * CreateFile(const char* arg);
void TokensToLinePrint(char tokens[20][20]);
AST * MakeBinaryExp(char operator, AST* left, AST* right);
AST * MakeUnaryExp(char operator, AST* operand);
AST * MakeIntExp(int value);
AST * MakeStrExp(char* str);
AST * MakeCallExp(char * name, EXP_LIST* arg);
AST * MakeAssignExp(AST* left, AST* right);
EXP_LIST * MakeExpList(char token[20][20], int ind, int end);
AST * MakeAST(char token[20][20], int lvl, int end);

#endif