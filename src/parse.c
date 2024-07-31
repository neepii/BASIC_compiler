#include "basicc.h"



int exit_code = 0;
int ParenthesisLvl = 0;
static AST * iter_parse_exp(int min_prec);
static AST * recursive_parse_exp(AST * left, int min_prec);

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
    case tag_symbol:
        printf("<id: %d>", ast->oper.symbol);
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




void FreeAST(AST * ast) {
    if (ast == NULL || ast->inSymbol==true) return;
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
        break;
    }
    free(ast);
}

AST * parse_OneWordStatementExp(char * name) {
    AST * node = AllocNode();
    node->tag = tag_one_word_statement;
    strcpy(node->oper.oneword_statement,name);
    return node;
}

AST * parse_CommonExp(AST* (*f)(void),char * name) {
    AST * node = AllocNode();
    node->tag = tag_common_statement;
    get_next_token();
    strcpy(node->oper.commonExp.name, name);
    node->oper.commonExp.arg = f();
    return node;

}




AST * parse_IfStatementExp() {
    AST * node = AllocNode();
    node->tag = tag_if;
    get_next_token();
    node->oper.ifstatementExp.predicate = parse_arith_expression();
    if (!match(cur_token(), "THEN")) {
        parse_syntax_error("no \"THEN\" after \"IF\" statement");
        return NULL;
    } else {
        get_next_token();
        node->oper.ifstatementExp.thenExp = parse_AST();
    }
    if (match(cur_token(), "ELSE")) {
        get_next_token();
        node->oper.ifstatementExp.elseExp = parse_AST();
    }
    return node;
    
}
AST * parse_EndStatementExp() {
    return parse_CommonExp(parse_AST, "END");
}
AST * parse_ClsStatementExp() {
    return parse_OneWordStatementExp("CLS");
}
AST * parse_PrintStatementExp() {
    char * str = next_token();
    AST * node = parse_CommonExp(parse_leaf, "PRINT");
    if (isSTRING(str)) {
        add_symbol(node->oper.commonExp.arg);
    } else if (isVAR(str)) {
        int h = (int) hash(str) % S_TABLE_SIZE;
        int id = S_TABLE->list[h]->id;
        node->oper.commonExp.arg->oper.symbol = id;
        node->oper.commonExp.arg->tag = tag_symbol;
    }
    
    return node;
}
AST * parse_LetStatementExp() {
    return parse_CommonExp(parse_AssignExp, "LET");
}
AST * parse_InputStatementExp() {
    return parse_CommonExp(parse_VarExp, "INPUT");
}
AST * parse_WhileStatementExp() {
    return parse_CommonExp(parse_arith_expression, "WHILE");
}
AST * parse_WendStatementExp() {
    return parse_OneWordStatementExp("WEND");
}
AST * parse_NextStatementExp() {
    return parse_CommonExp(parse_VarExp, "NEXT");
}
AST * parse_GotoStatementExp() {
    AST * node = parse_CommonExp(parse_IntExp, "GOTO");
    int num = node->oper.commonExp.arg->oper.intExp;
    for (int i = 0;; i++)
    {
        if (statements[i]->tag == tag_numline && statements[i]->oper.numline.value == num){ // what if theres no numline?
            statements[i]->oper.numline.isGotoLabel=true;
            break;
        }
    }
    return node;
}


AST * AllocNode() {
    AST * node = (AST *) malloc(sizeof(AST));
    node->inSymbol = false;
    return node;
}


AST * parse_ForStatementExp() {
    AST* node = AllocNode();
    node->tag = tag_for;
    get_next_token();
    node->oper.forstatementExp.initial = parse_AssignExp();
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
// AST* parse_FunctionExp() {
//     AST * node = parse_CommonExp();
//     node->oper.commonExp.name = cur_token();
//     node->oper.com

// }


AST * parse_BinaryExp(AST* left, char * operator, AST* right) {
    AST * node = AllocNode();
    node->tag = tag_binary;
    strcpy(node->oper.binaryExp.operator,operator);
    node->oper.binaryExp.left = left;
    node->oper.binaryExp.right = right;
    return node;
}

AST * parse_UnaryExp(char * operator) {
    AST * node = AllocNode();
    node->tag = tag_unary;
    strcpy(node->oper.unaryExp.operator,operator);
    node->oper.unaryExp.operand = parse_leaf();
    return node;
}

// AST * parse_CallExp() {
//     AST * node = AllocNode();
//     node->tag = tag_call;s
//     node->oper.callExp.name = cur_token();
//     return node;
// }
AST * parse_IntExp(){
    if (!isINT(cur_token())) {
        parse_syntax_error("type error with int");
    }
    int value = atoi(cur_token());
    AST * node = AllocNode();
    node->tag = tag_int;
    node->oper.intExp = value;
    return node;
}
AST * parse_StrExp() {
    if (!isSTRING(cur_token())) {
        parse_syntax_error("type error with string");
    }
    AST * node = AllocNode();
    strcpy(node->oper.strExp, cur_token());
    node->tag = tag_str;
    return node;
}
AST * parse_NumLineExp() {
    if (!isINT(cur_token())) {
        parse_syntax_error("typer error with int");
    }
    AST * node = AllocNode();
    node->tag = tag_numline;
    node->oper.intExp = atoi(cur_token());
    get_next_token();
    node->oper.numline.next = parse_AST();
    node->oper.numline.isGotoLabel = false;
    return node;
}

AST * parse_AssignExp() {
    AST * node = AllocNode();
    node->tag = tag_assign;
    AST * identifier = parse_VarExp();
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
    add_symbol(node);

    return node;
}
AST * parse_VarExp() {
    if (isINT(cur_token())) {
        parse_syntax_error("numbers found in variable name");
    }
    AST * node = AllocNode();
    strcpy(node->oper.varExp, cur_token());
    node->tag = tag_var;
    return node;
}


AST * parse_leaf() {
    char * token = cur_token();
    if (isUNARY(token)) {
        get_next_token();
        return parse_UnaryExp(token);
    }

    if (isINT(token)) return parse_IntExp();
    if (isSTRING(token)) return parse_StrExp();
    if (isVAR(token)) return parse_VarExp();
    

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
        return parse_BinaryExp(left, op, right);
    }
}

AST * parse_arith_expression() {
    if (ParenthesisLvl) parse_syntax_error("non-closed parethesis");
    return iter_parse_exp(-1);
}







AST * parse_AST() { // lvl starts with 0
    if (tokInd == tokLen) {
        return NULL;
    }

    if (tokInd == 0 && isINT(cur_token())) return parse_NumLineExp();
    
    unsigned long t = hash(cur_token());
    if (match(cur_token(), "")) return NULL;
    switch (t)
    {
    case LET_H: return parse_LetStatementExp();
    case PRINT_H: return parse_PrintStatementExp();
    case INPUT_H: return parse_InputStatementExp();
    case END_H: return parse_EndStatementExp();
    case IF_H: return parse_IfStatementExp();
    case FOR_H: return parse_ForStatementExp();
    case NEXT_H: return parse_NextStatementExp();
    case WHILE_H: return parse_WhileStatementExp();
    case WEND_H: return parse_WendStatementExp();
    case REM_H: return parse_OneWordStatementExp("REM");
    case GOTO_H: return parse_GotoStatementExp();
    default: return parse_AssignExp();
    }
    
    return NULL;

}