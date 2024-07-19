#include "parse.h"

typedef enum wordtype {
    WT_ASCII,
    WT_OPER,
    WT_NUM,
    WT_ETC,
    WT_QUOTES,
    WT_PARENTHESIS,
    WT_NULL
} wt;


typedef struct exp
{
    enum {
        tag_int,
        tag_str,
        tag_call,
        tag_var,
        tag_unary,
        tag_binary,
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
            char operator;
        } binaryExp;
        struct {
            struct exp* operand;
            char operator
        } unaryExp;
        struct {
            char * name;
            EXP_LIST *  argument;
        } callExp;

    }oper;
} AST;

typedef struct exp_list
{
    AST * ast;
    struct exp_list * next;
} EXP_LIST;


void TokensToLinePrint(char tokens[20][20]) {
    int i = 0;
    while(tokens[i][0]) {
        printf("%s\n" , tokens[i++]);
    }
}

static int isPARENTHESIS(char c) {
    return (c==0x28 || c==0x29);
}

static int isQUOTE(char c) {
    return c == 0x22;
}

static int isOPER(char c) {
    return ((c >= 0x2A && c <= 0x2B) || c == 0x2D || c == 0x2F || (c >= 0x3C && c <= 0x3E)) ? 1 : 0;
}

static int isNUM(char c) {
    return (c >= 0x30 && c <= 0x39) ? 1 : 0;
}


static int isASCII(char c) {
    return ((c >=0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A)) ? 1 : 0;
}

FILE * OpenFile(const char *arg) {
    FILE * f = fopen(arg, "r");
    return f;
}

void FillTokenArray(FILE * in, char tokens[20][20]) {
    char line[100];
    fgets(line, 100, in);
    char word[20];
    for (int i = 0; i < 20; i++)
    {
        memset(tokens[i], 0,20);
    }
    
    int c = 0;
    int i = 0;
    int j = 0;
    wt type;
    wt last = WT_NULL;

    while (line[c]) {
        if (line[c] == ' ') {
            c++;
            continue;
        }

        if (isASCII(line[c])) {
            type = WT_ASCII;
        }
        else if (isOPER(line[c])) {
            type = WT_OPER;
        } 
        else if (isNUM(line[c])) {
            type = WT_NUM;
        }
        else if (isQUOTE(line[c])) {
            type = WT_QUOTES;
        }
        else if (isPARENTHESIS(line[c])) {

        }
        else {
            type = WT_ETC;
        }

        if (last != type && last != WT_NULL) {
            strncat(tokens[j], word,i);
            i=0;
            j++;
        }
        word[i] = line[c];
        i++;
        c++;
        last = type;
    }
    
}

AST * MakeBinaryExp(char operator, AST* left, AST* right) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_binary;
    node->oper.binaryExp.left = left;
    node->oper.binaryExp.right = right;
    node->oper.binaryExp.operator = operator;
    return node;
}

AST * MakeUnaryExp(char operator, AST* operand) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_unary;
    node->oper.unaryExp.operand = operand;
    node->oper.unaryExp.operator = operator;
    return node;
}

AST * MakeCallExp(char * name, EXP_LIST * arg) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_call;
    node->oper.callExp.argument = arg;
    node->oper.callExp.name = name;
    return node;
}
AST * MakeIntExp(int value) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_int;
    node->oper.intExp = value;
    return node;
}
AST * MakeStrExp(char* str) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_int;
    node->oper.strExp = str;
    return node;
}