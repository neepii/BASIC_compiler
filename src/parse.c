#include "parse.h"

char** tokens = NULL;

const char* builtins[] = {
    "LET",
    "PRINT"
};
const char* binOperator[] = {
    "+",
    "-",
    "*",
    "/"
};
const int builtins_count = 2;
const int bin_exp_count = 4;

int tokInd =0;
int tokLen = 0;


static bool isNUM(char c);
static bool isCHAR(char c);

static void recursionPrintAST(AST* ast, int spaces);
void recursivePrintAST(AST* ast) {
    int spaces = 40;
    recursionPrintAST(ast, spaces);
    printf("\n");
}

static void  recursionPrintAST(AST* ast, int spaces) {
    printf("\n");
    printf("\x1b[%dC", spaces);
    switch (ast->tag)
    {
    case tag_numline:
        printf("%d", ast->oper.numline.value);
        recursionPrintAST(ast->oper.numline.next,spaces);
        break;
    case tag_var:
        printf("%s",ast->oper.varExp);
        break;
    case tag_int:
        printf("%d", ast->oper.intExp);
        break;
    case tag_call:
        printf("%s", ast->oper.callExp.name);
        recursionPrintAST(ast->oper.callExp.argument, spaces);
        break;
    case tag_str:
        printf("%s", ast->oper.strExp);
        break;
    default:
        break;
    }
}


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

bool isSTRING(char * str) {
    return str[0] == '"';
}

bool isVAR(char * str) {
    return isCHAR(str[0]);
}

bool isINT(char * str) {
    return isNUM(str[0]);
}

bool isBINEXP(char * str) {
    for (int i = 0; i < bin_exp_count; i++)
    {
        if (strcmp(str, binOperator[i]) == 0) return true;
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

bool match(char * str1, char* str2) {
    if (strcmp(str1, str2) == 0) {
        return true;
    }
    return false;
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

    tokInd = 0;
    for (int i = 0; i < MAX_TOKENS_IN_LINE; i++)
    {
        tokens[i] = (char*) malloc(sizeof(char) * (TOKEN_LEN + 1)); 
        memset(tokens[i], 0,20);
    }
}

void FillTokenArray(FILE * in) {
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
    bool inQuotes = false;

    while (line[c]) {
        if (line[c] == ' ') {
            c++;
            continue;
        }
        if (isCHAR(line[c])) {
            type = (inQuotes) ? WT_QUOTES : WT_CHAR;
        }
        else if (isOPER(line[c])) {
            type = WT_OPER;
        } 
        else if (isNUM(line[c])) {
            type = WT_NUM;
        }
        else if (isQUOTE(line[c])) {
            type = WT_QUOTES;
            if (!inQuotes) inQuotes = true;
        }
        else if (isPARENTHESIS(line[c])) {
            type = WT_PARENTHESIS;
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
    tokLen = j;
}

AST * MakeBinaryExp(char *operator, AST* left, AST* right) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_binary;
    node->oper.binaryExp.left = left;
    node->oper.binaryExp.right = right;
    node->oper.binaryExp.operator = operator;
    return node;
}

AST * MakeUnaryExp(char *operator, AST* operand) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_unary;
    node->oper.unaryExp.operand = operand;
    node->oper.unaryExp.operator = operator;
    return node;
}

AST * MakeCallExp(char * name) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_call;
    node->oper.callExp.name = name;
    return node;
}
AST * MakeIntExp(char * str) {
    int value = atoi(str);
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_int;
    node->oper.intExp = value;
    return node;
}
AST * MakeStrExp(char* str) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_str;
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
AST * MakeVarExp(char* str) {
    AST * node = (AST*)malloc(sizeof(AST));
    node->tag = tag_var;
    node->oper.varExp = str;
    return node;
}

// EXP_LIST * MakeExpList() { //linked list
//     if (tokInd == tokLen) {
//         return NULL;
//     }
//     EXP_LIST * node = (EXP_LIST*)malloc(sizeof(EXP_LIST));
//     node->ast = MakeAST();
//     node->next = MakeExpList();
//     return node;
// }


char * get_next_token() {
    if (tokInd != tokLen) {
        return tokens[tokInd++];
    } else {
        fprintf(stderr, "ERROR: Token index is out of bounds");
        return NULL;
    }

}   

AST *  parse_leaf(){
    char * next = get_next_token();
    AST * node; 

    if (isSTRING(next)) node = MakeStrExp(next);
    else if (isINT(next)) node = MakeIntExp(next);
    else if (isVAR(next)) node = MakeVarExp(next);

    return node;
}

AST * parse_single_assign() {
    AST * left = parse_leaf();
    char * oper = get_next_token();
    AST * right = parse_leaf();
    if (match(oper, "=")) {
        return MakeAssignExp(left, right);
    }
    return NULL;
}



AST * MakeAST() { // lvl starts with 0
    if (tokInd == tokLen) {
        return NULL;
    }
    AST * node;
    if (tokInd == 0) {
        char * next = get_next_token();
        node = MakeNumLineExp(next);
        node->oper.numline.next = MakeAST();
        return node;
    }
    
    
    if (isSTRING(tokens[tokInd])) {
        char * next = get_next_token();
        node = MakeStrExp(next);
        return node;
    }
    else if (isINT(tokens[tokInd])) {
        char * next = get_next_token();
        node = MakeIntExp(next);
        return node;
    }
    else if (match(tokens[tokInd+1], "=")) {
        AST * left = parse_leaf();
        get_next_token();
        AST * right = parse_leaf();
        node = MakeAssignExp(left, right);
        return node;
        
    }
    if (isCallExp(tokens[tokInd])) {
        char * next = get_next_token();
        node = MakeCallExp(next);
        node->oper.callExp.argument = MakeAST();
        return node;
    }
    parse_leaf();
    return NULL;
}