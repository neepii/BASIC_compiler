
#ifndef TOKEN_H_
#define TOKEN_H_
#include <stdio.h>

typedef enum wordtype {
  WT_LATIN,
  WT_PUNCT,
  WT_OPER,
  WT_NUM,
  WT_ETC,
  WT_QUOTES,
  WT_SPACE,
  WT_NEWLINE,
  WT_PARENTHESIS, // left and right are needede
  WT_NULL
} wt;

typedef struct graph {
    unsigned int v;
    unsigned int e;
    struct successors {
        int n;
        int arrlen;
        int arr[1];
    } * adj[1];
} GRAPH;

struct node {
    int data;
    struct node * next;
};
    
typedef struct stack {
    unsigned int last;
    unsigned int arrlen;
    int arr[1];
} STACK;
typedef struct stack_str {
    unsigned int last; 
    unsigned int arrlen;
    char arr[1][20];
} STACK_STR;

typedef struct nfa {
    GRAPH * g;
    int M;
    char* re;
} NFA;


int LineToTokens(FILE * in);
void TokensToLinePrint();
void freeTokensArr();
void allocTokensArr();
void get_next_token();
char * cur_token();
char * next_token();
bool isBINEXP(char * str);
bool isSTRING(char * str);
bool isUNARY(char * str);
bool isVAR(char * str);
bool isINT(char *str);
bool isFLOAT(char *str);
bool isBINEXP(char *str);
bool isCOMMON_FLOAT(char *str);
bool isDOT_FLOAT(char *str);
bool isSCIENTIFIC_FLOAT(char *str);
bool isNUM(char c);
void init_nfa();
void free_nfa();
bool match(char * str1, char* str2);
void test_regex(char *str);
STACK *init_stack(unsigned int len);
int pop_s(STACK * st);
void push_s(STACK *st, int data);
void clear_stack(STACK *s);
int top_stack(STACK *s);
bool stack_is_empty(STACK *st);
bool stack_str_is_empty(STACK_STR *st);
STACK_STR *init_stack_str(unsigned int len);
void push_str_s(STACK_STR *st, char * str);
void pop_str_s(STACK_STR *st, char addr[20]);
extern bool *marked;
extern NFA *nfaINTEGER;
extern NFA *nfaSTRING;
extern NFA *nfaCOMMON_FLOAT;
extern NFA *nfaDOT_FLOAT;
extern NFA *nfaSCIENTIFIC_FLOAT;
extern NFA *nfaVARIABLE;
extern STACK *goto_s;
extern STACK_STR *float_inits;

#endif
