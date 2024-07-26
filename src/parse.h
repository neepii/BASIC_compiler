#ifndef PARSE_H_
#define PARSE_H_
#include "basicc.h"
#define AST_STR_LEN 20



typedef struct exp AST;

typedef struct exp
{
    enum {
        tag_int,
        tag_str,
        tag_call, // function
        tag_var,
        tag_symbol,
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
        int symbol;
        int intExp;
        char strExp[AST_STR_LEN];
        char varExp[AST_STR_LEN];
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
            char operator[AST_STR_LEN];
        } binaryExp;
        struct {
            struct exp* operand;
            char operator[AST_STR_LEN];
        } unaryExp;
        struct {
            char name[AST_STR_LEN];
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
            char name[AST_STR_LEN];
            struct exp* arg;
        } commonExp;
        char oneword_statement[AST_STR_LEN];
    }oper;
    bool inSymbol;
} AST;

void printAST(AST* ast);
AST * AllocNode();
AST * parse_OneWordStatementExp(char * name);
AST * parse_EndStatementExp();
AST * parse_ClsStatementExp();
AST * parse_BinaryExp(AST* left, char* operator, AST* right);
AST * parse_UnaryExp(char * operator);
AST * parse_IntExp();
AST * parse_StrExp();
AST * parse_CallExp();
AST * parse_VarExp();
AST * parse_AssignExp();
AST * parse_AST();
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
