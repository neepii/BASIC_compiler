#ifndef PARSE_H_
#define PARSE_H_
#include "basicc.h"
#define AST_STR_LEN 20
#define TAC_ENTRIES 512 
#define LIVE_INTER_LEN 15
typedef union atom {
    double f;
    long long i;
    char c[64];
    char *cp;
    unsigned int addr;
} Atom;

enum statement {
    op_print,
    op_input,
    op_let,
    op_rem,
    op_end,
    op_if,
    op_for,
    op_while,
    op_dim,
    op_wend,
    op_next,
    op_cls,
    op_goto,
    stmt_null
};

enum operator {
    op_plus,        // "+"
    op_minus,       // "-"
    op_mul,         // "*"
    op_div,         // "/"
    op_less,        // "<"
    op_greater,     // ">"
    op_less_eq,     // "<="
    op_greater_eq,  // ">="
    op_equal,       // "="  
    op_not_eq,      // "<>" "><"
    op_null
};

typedef struct tacentry{
    enum operator operator;
    Atom arg1;
    Atom arg2;
    Atom result;
} TAC_Entry;

/*
    three-address code
*/
typedef struct tac{
    unsigned int len;
    TAC_Entry * arr;
    short LiveInterval[LIVE_INTER_LEN][2];
}TAC;

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
        tag_numline,
        tag_arith
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
            TAC * lowlvl;
            struct exp*highlvl;
        } arithExp;
        struct {
            int value;
            bool isGotoLabel;
            struct exp * next;
        } numline;
        struct {
            struct exp* left;
	    struct exp* right;
            enum operator operator;
        } binaryExp;
        struct {
            struct exp* operand;
            enum operator operator;
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
            enum statement stmt;
            struct exp* arg;
        } commonExp;
        enum statement one_word_stmt;
    }oper;
    bool inTable;
} AST;

void printAST(AST* ast);
AST * AllocNode();
void map_ast(AST * ast, void* (*f)(void*));
AST * parse_AST();
void freeTokensArr();
void allocTokensArr();
void get_next_token();
char * cur_token();
char * next_token();
void printParsedLine(AST * ast);
void parse_error(char * str);
void parse_syntax_error(char* str);
bool match(char*, const char*);
void FreeAST(AST * ast);
TAC * ASTtoTAC(AST * node);
int postfix_GetIndex(char * str);
bool isSymbolVar(Atom atom);
bool isTempVar(Atom atom);



#endif
