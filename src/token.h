#ifndef TOKEN_H_
#define TOKEN_H_
#include <stdio.h>

typedef enum wordtype {
  WT_CHAR,
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
    int last; //available
    int arrlen;
    int arr[1];
} STACK;

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
bool isBINEXP(char *str);
bool isNUM(char c);
void init_nfa();
void free_nfa();
bool match(char * str1, char* str2);
void test_regex(char *str);

extern bool *marked;

extern NFA *nfaINTEGER;
extern NFA *nfaSTRING;

#endif
