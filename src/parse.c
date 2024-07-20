#include "parse.h"

char** tokens = NULL;
const char* builtins[] = {
    "LET",
    "PRINT"
};
const int builtins_count = 2;

void TokensToLinePrint() {
    int i = 0;
    while(tokens[i][0]) {
        printf("%s\n" , tokens[i++]);
    }
}

bool isCallExp(char * str) {
    for (int i = 0; i < builtins_count; i++)
    {
        if (strcmp(str, builtins[i]) == 0)
        return true;
    }
    return false;
}

static bool isPARENTHESIS(char c) {
    return (c==0x28 || c==0x29);
}

static bool isQUOTE(char c) {
    return c == 0x22;
}

static bool isOPER(char c) {
    return (c >= 0x2A && c <= 0x2D) || c == 0x2F || (c >= 0x3C && c <= 0x3E);
}

static bool isNUM(char c) {
    return c >= 0x30 && c <= 0x39;
}


static bool isCHAR(char c) {
    return (c >=0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A);
}

FILE * OpenFile(const char *arg) {
    FILE * f = fopen(arg, "r");
    return f;
}

FILE * CreateFile(const char *arg) {
    FILE * f = fopen(arg, "w");
    return f;
}

void freeTokensArr() {
    for (int i = 0; i < MAX_TOKENS_IN_LINE; i++)
    {
        free(tokens[i]);
    }
    free(tokens);
}

void allocTokensArr() {
    tokens = (char **) malloc(sizeof(char*) * MAX_TOKENS_IN_LINE);
    for (int i = 0; i < MAX_TOKENS_IN_LINE; i++)
    {
        tokens[i] = (char*) malloc(sizeof(char) * TOKEN_LEN);
        memset(tokens[i], 0,20);
    }
}

int FillTokenArray(FILE * in) {
    if (tokens != NULL) {
        freeTokensArr();    
    }
    allocTokensArr();
    
    char line[100];
    fgets(line, 100, in);
    char word[TOKEN_LEN];
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

        if (isCHAR(line[c])) {
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
    return j;
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
AST * MakeNumLineExp(char* str) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_numline;
    node->oper.intExp = atoi(str);
    return node;
}

AST * MakeAssignExp(AST* left, AST* right) {
    AST * node = (AST*)malloc(sizeof(AST));
    node->tag = tag_assign;
    node->oper.assignExp.left = left;
    node->oper.assignExp.right = right;
    return node;
}
EXP_LIST * MakeExpList(int ind, int end) {
    if (ind == end) {
        return NULL;
    }
    EXP_LIST * node = (EXP_LIST*)malloc(sizeof(EXP_LIST));
    node->ast = MakeAST(ind,end);
    node->next = MakeExpList(ind + 1, end);
    return node;
}

// make get next token

AST * MakeAST(int lvl, int end) { // lvl starts with 0
    if (lvl == end) {
        return NULL;
    }
    AST * node;
    if (lvl == 0) {
        node = MakeNumLineExp(tokens[lvl]);
        node->oper.nulline.next = MakeAST(lvl+ 1, end);
        return node;
    }
    if (isCallExp(tokens[lvl])) {
        EXP_LIST * args = MakeExpList(lvl + 1, end);
        node = MakeCallExp(tokens[lvl], args);
    } else {}
    return NULL;
}