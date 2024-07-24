#ifndef PARSE_H_
#define PARSE_H_
#include "basicc.h"



typedef struct exp AST;

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
        tag_common_statement,
        tag_one_word_statement,
        tag_function,
        tag_if,
        tag_for,
        tag_numline
    } tag;  
    union
    {   
        int intExp;
        char* strExp;
        char * varExp;
        struct {
            struct exp* identifier;
            struct exp* value;
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
        struct {
            struct exp* predicate;
            struct exp* thenExp;
            struct exp* elseExp;
        } ifstatementExp;
        struct {
            struct exp* initial;
            struct exp* final;
            struct exp* step;
        } forstatementExp;
        struct {
            char* name;
            struct exp* arg;
            struct exp* next;
        } commonExp;
        char * oneword_statement;
    }oper;
    bool inSymbol;
} AST;

AST * AllocNode();
AST * MakeOneWordStatementExp(char * name);
AST * MakeEndStatementExp();
AST * MakeClsStatementExp();
AST * MakeBinaryExp(AST* left, char* operator, AST* right);
AST * MakeUnaryExp(char * operator);
AST * MakeIntExp();
AST * MakeStrExp();
AST * MakeCallExp();
AST * MakeVarExp();
AST * MakeAssignExp();
AST * MakeAST();
void freeTokensArr();
void allocTokensArr();
void get_next_token();
char * cur_token();
char * next_token();
AST * parse_arith_expression();
void printParsedLine(AST * ast);
void parse_error(char * str);
void parse_syntax_error(char* str);
bool match(char*, const char*);
void FreeAST(AST * ast);
AST * parse_leaf();



#endif
