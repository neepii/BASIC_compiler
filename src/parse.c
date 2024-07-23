#include "parse.h"
#include "hash.h"
#define TREE_LEFT 0
#define TREE_CENTER 1 
#define TREE_RIGHT 2
#define TREE_NOTHING -1

char** tokens = NULL;
int exit_code = 0;
int ParenthesisLvl = 0;
static AST * iter_parse_exp(int min_prec);
static AST * recursive_parse_exp(AST * left, int min_prec);

const char * UnaryOpers[] = {
    "+",
    "-",

};
const int unary_count = 2;

const char* BinOpers[] = {
    "+",
    "-",
    "*",
    "/",
    "=",
    "#",
    "<>",
    "><",
    ">",
    "<",
    "<=",
    ">="
};
const int bin_opers_count = 12;

int tokInd =0;
int tokLen = 0;


static bool isNUM(char c);
static bool isCHAR(char c);

void printAST(AST * ast) {
    if (ast == NULL) {
        return;
    }
    printf("(");
    switch (ast->tag)
    {
    case tag_assign:
        printAST(ast->oper.assignExp.identifier);
        printf(" = ");
        printAST(ast->oper.assignExp.value);
        break;
    case tag_numline:
        printf("%d", ast->oper.numline.value);
        printAST(ast->oper.numline.next);
        break;
    case tag_var:
        printf("%s",ast->oper.varExp);
        break;
    case tag_int:
        printf("%d", ast->oper.intExp);
        break;
    case tag_str:
        printf("%s", ast->oper.strExp);
        break;
    case tag_common_statement:
        printf("%s", ast->oper.commonExp.name);
        printAST(ast->oper.commonExp.arg);
        break;
    case tag_one_word_statement:
        printf("%s", ast->oper.oneword_statement);
        break;
    case tag_if:
        printf("IF");
        printAST(ast->oper.ifstatementExp.predicate);
        printf("THEN");
        printAST(ast->oper.ifstatementExp.thenExp);
        if (ast->oper.ifstatementExp.elseExp) {
            printf("ELSE");
            printAST(ast->oper.ifstatementExp.elseExp);
        }
        break;
    case tag_binary:
        printAST(ast->oper.binaryExp.left);
        printf("%s", ast->oper.binaryExp.operator);
        printAST(ast->oper.binaryExp.right);
        break;
    case tag_for:
        printf("FOR");
        printAST(ast->oper.forstatementExp.initial);
        printf("TO");
        printAST(ast->oper.forstatementExp.final);
        printf("STEP");
        printAST(ast->oper.forstatementExp.step);
        break;
    // case tag_call:
    //     printf("%s", ast->oper.callExp.name);
    case tag_unary:
        printf("%s", ast->oper.unaryExp.operator);
        printAST(ast->oper.unaryExp.operand);
        break;
    default:
        break;
    }   
    
    

    printf(")");

}


void printParsedLine(AST * tree) {
    printAST(tree);
    printf("\n");
}

void TokensToLinePrint() {
    int i = 0;
    while(tokens[i][0]) {
        printf("%s\n" , tokens[i++]);
    }
}


bool isSTRING(char * str) {
    return (str[0] == '"') || (str[0] == '`');
}

bool isUNARY(char * str) {
    for (int i = 0; i < unary_count; i++)
    {
        if (match(str, UnaryOpers[i])) return true;
    }
    return false;
}


bool isVAR(char * str) {
    return isCHAR(str[0]);
}

bool isINT(char * str) {
    return isNUM(str[0]);
}

bool isBINEXP(char * str) {
    for (int i = 0; i < bin_opers_count; i++)
    {
        if (strcmp(str, BinOpers[i]) == 0) return true;
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

bool match(char * str1, const char* str2) {
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

bool LineToTokens(FILE * in) {
    if (tokens != NULL) {
        freeTokensArr();    
    }
    allocTokensArr();
    char word[TOKEN_LEN];
    int i = 0;
    int j = 0;
    wt type;
    wt last = WT_NULL;
    bool inQuotes = false;
    char cur_char = fgetc(in);
    if (feof(in)) {
        return false;
    }

    while(cur_char && type != WT_ETC) {
        cur_char = (char)toupper(cur_char);

        if (cur_char == ' ') {
            type = WT_SPACE;
        }
        else if (cur_char == '\n') {
            type = WT_NEWLINE;
        }
        else if (isCHAR(cur_char)) {
            type = (inQuotes) ? WT_QUOTES : WT_CHAR;
        }
        else if (isOPER(cur_char)) {
            type = WT_OPER;
        } 
        else if (isNUM(cur_char)) {
            type = WT_NUM;
        }
        else if (isQUOTE(cur_char)) {
            type = WT_QUOTES;
            if (!inQuotes) inQuotes = true;
        }
        else if (isPARENTHESIS(cur_char)) {
            type = WT_PARENTHESIS;
        }
        else {
            type = WT_ETC;
        }

        if ((last != type && last != WT_NULL) || type == WT_PARENTHESIS) {
            
            if (last != WT_SPACE) {
                strncat(tokens[j], word,i);
                j++;
            }
            i=0;
        }

        if (type == WT_NEWLINE) break;

        word[i] = cur_char;
        i++;
        last = type;
        cur_char = fgetc(in);
    }
    tokLen = j;
    return true;
}

void FreeAST(AST * ast) {
    if (ast == NULL) return;
    switch (ast->tag) {
    case tag_numline:
        FreeAST(ast->oper.numline.next);
        break;
    case tag_assign:
        FreeAST(ast->oper.assignExp.identifier);
        FreeAST(ast->oper.assignExp.value);
        break;
    case tag_common_statement:
        FreeAST(ast->oper.commonExp.arg);
        break;
    case tag_binary:
        FreeAST(ast->oper.binaryExp.left);
        FreeAST(ast->oper.binaryExp.right);
        break;
    case tag_unary:
        FreeAST(ast->oper.unaryExp.operand);
        break;
    case tag_for:
        FreeAST(ast->oper.forstatementExp.initial);
        FreeAST(ast->oper.forstatementExp.final);
        break;
    case tag_if:
        FreeAST(ast->oper.ifstatementExp.predicate);
        FreeAST(ast->oper.ifstatementExp.thenExp);
        FreeAST(ast->oper.ifstatementExp.elseExp);
    }
    free(ast);
}

AST * MakeOneWordStatementExp(char * name) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_one_word_statement;
    node->oper.oneword_statement = name;
    return node;
}

AST * MakeCommonExp(AST* (*f)(void),char * name) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_common_statement;
    get_next_token();
    node->oper.commonExp.next = NULL;
    node->oper.commonExp.arg = f();
    node->oper.commonExp.name = name;
    return node;

}




AST * MakeIfStatementExp() {
    AST * node = (AST *) malloc(sizeof(AST));
    node->tag = tag_if;
    get_next_token();
    node->oper.ifstatementExp.predicate = parse_arith_expression();
    if (!match(cur_token(), "THEN")) {
        parse_syntax_error("no \"THEN\" after \"IF\" statement");
        return NULL;
    } else {
        get_next_token();
        node->oper.ifstatementExp.thenExp = MakeAST();
    }
    if (match(cur_token(), "ELSE")) {
        get_next_token();
        node->oper.ifstatementExp.elseExp = MakeAST();
    }
    return node;
    
}
AST * MakeEndStatementExp() {
    return MakeCommonExp(MakeAST, "END");
}
AST * MakeClsStatementExp() {
    return MakeOneWordStatementExp("CLS");
}
AST * MakePrintStatementExp() {
    return MakeCommonExp(MakeStrExp, "PRINT");
}
AST * MakeLetStatementExp() {
    return MakeCommonExp(MakeAssignExp, "LET");
}
AST * MakeInputStatementExp() {
    return MakeCommonExp(MakeVarExp, "INPUT");
}
AST * MakeWhileStatementExp() {
    return MakeCommonExp(parse_arith_expression, "WHILE");
}
AST * MakeWendStatementExp() {
    return MakeOneWordStatementExp("WEND");
}
AST * MakeNextStatementExp() {
    return MakeCommonExp(MakeVarExp, "NEXT");
}




AST * MakeForStatementExp() {
    AST * node = (AST *) malloc(sizeof(AST));
    node->tag = tag_for;
    get_next_token();
    node->oper.forstatementExp.initial = MakeAssignExp();
    if (!match(cur_token(), "TO")) {
        parse_syntax_error("no \"TO\" after \"FOR\" statement");
        return NULL;
    } else {
        get_next_token();
        node->oper.forstatementExp.final = parse_arith_expression();
    }
    if (match(cur_token(), "STEP")) {
        get_next_token();
        node->oper.forstatementExp.step = parse_arith_expression();
    }
    return node;
}
// AST* MakeFunctionExp() {
//     AST * node = MakeCommonExp();
//     node->oper.commonExp.name = cur_token();
//     node->oper.com

// }


AST * MakeBinaryExp(AST* left, char * operator, AST* right) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_binary;
    node->oper.binaryExp.left = left;
    node->oper.binaryExp.right = right;
    node->oper.binaryExp.operator = operator;
    return node;
}

AST * MakeUnaryExp(char * operator) {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_unary;
    node->oper.unaryExp.operand = parse_leaf();
    node->oper.unaryExp.operator = operator;
    return node;
}

AST * MakeCallExp() {
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_call;
    node->oper.callExp.name = cur_token();
    return node;
}
AST * MakeIntExp(){
    if (!isINT(cur_token())) {
        parse_syntax_error("type error with int");
    }
    int value = atoi(cur_token());
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_int;
    node->oper.intExp = value;
    return node;
}
AST * MakeStrExp() {
    if (!isSTRING(cur_token())) {
        parse_syntax_error("type error with string");
    }
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_str;
    node->oper.strExp = cur_token();
    return node;
}
AST * MakeNumLineExp() {
    if (!isINT(cur_token())) {
        parse_syntax_error("typer error with int");
    }
    AST * node = (AST*) malloc(sizeof(AST));
    node->tag = tag_numline;
    node->oper.intExp = atoi(cur_token());
    return node;
}

AST * MakeAssignExp() {
    AST * node = (AST*)malloc(sizeof(AST));
    node->tag = tag_assign;
    AST * identifier = MakeVarExp();
    get_next_token();
    AST * value;
    if (!match(cur_token(), "=")) {
        parse_syntax_error("no equal sign in assignment");
        value = NULL;
    } else {
        get_next_token();
        value = parse_arith_expression();
    }
    node->oper.assignExp.identifier = identifier;
    node->oper.assignExp.value = value;

    add_symbol(identifier->oper.varExp, value);

    return node;
}
AST * MakeVarExp() {
    if (isINT(cur_token())) {
        parse_syntax_error("numbers found in variable name");
    }
    AST * node = (AST*)malloc(sizeof(AST));
    node->tag = tag_var;
    node->oper.varExp = cur_token();
    return node;
}


AST * parse_leaf() {
    char * token = cur_token();

    if (isUNARY(token)) {
        get_next_token();
        return MakeUnaryExp(token);
    }

    if (isINT(token)) return MakeIntExp();
    if (isSTRING(token)) return MakeStrExp();
    if (isVAR(token)) return MakeVarExp();
    

    parse_error("input not found for parsing lead");
    return NULL;
}

int get_predecense(char * Operator) {
    if (match(Operator, ">") || //TODO: generalize by creating a function
        match(Operator, "<")  ||
        match(Operator, "=") ||
        match(Operator, "")){
        return 1;
    }

    else if (match(Operator, "+") || match (Operator, "-"))
    {
        return 2;
    }
    else if(match(Operator,"*") || match(Operator, "/")) {
        return 3;
    }
    else return 0;
}



void parse_error(char * str) {
    fprintf(stderr, "error: %s\n", str);
    exit_code = 1;
}
void parse_syntax_error(char *str) {
    fprintf(stderr, "syntax error: %s\n", str);
    exit_code = 1;   
}

char * next_token() {
    return tokens[tokInd+1];
}

char * cur_token() {
    return tokens[tokInd];
}

void get_next_token() {
    if (tokInd < tokLen) {
        tokInd++;
    } else {
        parse_error("index out of bounds");
    }

}
bool compare_prec(int new_prec, int prec) { // is the new prec smaller then the old one?
    if (new_prec == -1 || prec == -1) return false;
    else if (new_prec <= prec) return true;
    else return false;
}



static AST * iter_parse_exp(int min_prec) {
    AST * left;
    AST * node;
    if (match(cur_token(), "(")) {
        ParenthesisLvl++;
        get_next_token();
        left = iter_parse_exp(0);
    } else {
        left = parse_leaf();
        get_next_token();
    }
    while (1) {
        node = recursive_parse_exp(left, min_prec);
        if (left == node) return node;
        left = node;
    }
}

static AST * recursive_parse_exp(AST * left, int min_prec) {
    char * op = cur_token();
    if (match(op, ")")) {
        if (!ParenthesisLvl) {
            parse_syntax_error("missing parenthesis");
        }
        ParenthesisLvl--;
        get_next_token();  
        return left;
    }
    if (!isBINEXP(op)) return left;

    int next_prec = get_predecense(op);
    if (compare_prec(next_prec, min_prec)) return left;

    else {
        get_next_token();
        AST * right = iter_parse_exp(next_prec);
        return MakeBinaryExp(left, op, right);
    }
}

AST * parse_arith_expression() {
    if (ParenthesisLvl) parse_syntax_error("non-closed parethesis");
    return iter_parse_exp(-1);
}


AST * parse_numline() {
    AST * node = MakeNumLineExp(cur_token());
    get_next_token();
    node->oper.numline.next = MakeAST();
    return node;
}



AST * MakeAST() { // lvl starts with 0
    if (tokInd == tokLen) {
        return NULL;
    }

    if (tokInd == 0 && isINT(cur_token())) return parse_numline();
    
    unsigned long t = hash(cur_token());
    if (match(cur_token(), "")) return NULL;
    switch (t)
    {
    case LET_H:
        return MakeLetStatementExp();
    case PRINT_H:
        return MakePrintStatementExp();
    case INPUT_H:
        return MakeInputStatementExp();
    case END_H:
        return MakeEndStatementExp();
    case IF_H:
        return MakeIfStatementExp();
    case FOR_H:
        return MakeForStatementExp();
    case NEXT_H:
        return MakeNextStatementExp();
    case WHILE_H:
        return MakeWhileStatementExp();
    case WEND_H:
        return MakeWendStatementExp();
    default:
        return MakeAssignExp();
    }
    
    return NULL;

}