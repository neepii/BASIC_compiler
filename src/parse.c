#include "basicc.h"

int exit_code = 0;
int ParenthesisLvl = 0;
static void map_tac(TAC *tac, void (*f)(void *));
static AST * parse_ReturnStatementExp();
static AST * parse_GoSubStatementExp();
static AST * parse_OneWordStatementExp(int stmt);
static AST * parse_CommonExp(AST* (*f)(void),int stmt);
static AST * parse_IfStatementExp();
static AST * parse_EndStatementExp();
static AST * parse_ClsStatementExp();
static AST * parse_PrintStatementExp();
static AST * parse_LetStatementExp();
static AST * parse_InputStatementExp();
static AST * parse_WhileStatementExp();
static AST * parse_RemStatementExp();
static AST * parse_WendStatementExp();
static AST * parse_NextStatementExp();
static AST * parse_GotoStatementExp();
static AST * parse_ForStatementExp();
static AST * parse_BinaryExp(AST *left, char *operator, AST * right);
static AST * parse_UnaryExp(char *operator);
static AST * parse_IntExp();
static AST * parse_StrExp();
static AST * parse_NumLineExp();
static AST * parse_AssignExp();
static AST * parse_VarExp();
static AST * parse_arith_expression();
static AST * parse_leaf();
static int get_predecense(char * op);
static bool compare_prec(int new_prec, int prec);
static AST * iter_parse_exp(int min_prec);
static AST * recursive_parse_exp(AST * left, int min_prec);
static bool getLastChar(Atom atom, char c);

bool float_flag;

static void print_op(int op) {
    switch (op)
    {
    case op_plus:
        printf("+");
        break;
    case op_minus:
        printf("-");
        break;
    case op_mul:
        printf("*");
        break;
    case op_div:
        printf("/");
        break;
    case op_equal:
        printf("=");
        break;
    case op_greater:
        printf(">");
        break;
    case op_less:
        printf("<");
        break;
    case op_less_eq:
        printf("<=");
        break;
    case op_greater_eq:
        printf(">=");
        break;
    case op_not_eq:
        printf("><");
        break;
    case op_not:
        printf("NOT");
        break;
    case op_and:
        printf("AND");
        break;
    case op_or:
        printf("OR");
        break;
    default:
        break;
    }
}

static void print_stmt(int stmt) {
    switch (stmt)
        {
        case op_print:
            printf("PRINT");
            break;
        case op_input:
            printf("INPUT");
            break;
        case op_end:
            printf("END");
            break;
        case op_let:
            printf("LET");
            break;
        case op_while:
            printf("WHILE");
            break;
        case op_rem:
            printf("REM");
            break;
        case op_dim:
            printf("DIM");
            break;
        case op_next:
            printf("NEXT");
            break;
        case op_goto:
            printf("GOTO");
            break;
        case op_gosub:
            printf("GOSUB");
            break;
        case op_return:
            printf("RETURN");
            break;
        case op_inc:
            printf("INC");
            break;
        case op_dec:
            printf("DEC");
            break;
        default:
            break;
    }
}

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
    case tag_float:
        printf("{float_id: %d}", ast->oper.floatExp);
        break;
    case tag_common_statement:
        print_stmt(ast->oper.commonExp.stmt);
        printAST(ast->oper.commonExp.arg);
        break;
    case tag_one_word_statement:
        switch (ast->oper.one_word_stmt)
        {
        case op_cls:    
            printf("CLS");
            break;     
        case op_wend:
            printf("WEND");
            break;
        default:
            break;
        }
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
    case tag_arith:
        printAST(ast->oper.arithExp.highlvl);
        break;
    case tag_binary:
        printAST(ast->oper.binaryExp.left);
        print_op(ast->oper.binaryExp.operator);
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
        print_op(ast->oper.unaryExp.operator);
        printAST(ast->oper.unaryExp.operand);
        break;
    default:
        break;
    }   
    
    

    printf(")");

}

void map_ast(AST * ast, void (*f)(void*)) {
    if (ast == NULL) {
        return;
    }
    switch (ast->tag)
    {
    case tag_assign:
        map_ast(ast->oper.assignExp.identifier,f);
        map_ast(ast->oper.assignExp.value,f);
        break;
    case tag_numline:
        map_ast(ast->oper.numline.next,f);
        break;
    case tag_symbol:
    case tag_var:
    case tag_int:
    case tag_str:
    case tag_float:
        break;
    case tag_common_statement:
        map_ast(ast->oper.commonExp.arg,f);
        break;
    case tag_one_word_statement:
        switch (ast->oper.one_word_stmt)
        {
        case op_cls:    
            break;     
        case op_wend:
            break;
        default:
            break;
        }
        break;
    case tag_if:
        map_ast(ast->oper.ifstatementExp.predicate,f);
        map_ast(ast->oper.ifstatementExp.thenExp,f);
        if (ast->oper.ifstatementExp.elseExp) {
            map_ast(ast->oper.ifstatementExp.elseExp,f);
        }
        break;
    case tag_binary:
        map_ast(ast->oper.binaryExp.left,f);
        map_ast(ast->oper.binaryExp.right,f);
        break;
    case tag_for:
        map_ast(ast->oper.forstatementExp.initial,f);
        map_ast(ast->oper.forstatementExp.final,f);
        map_ast(ast->oper.forstatementExp.step,f);
        break;
    case tag_unary:
        map_ast(ast->oper.unaryExp.operand,f);
        break;
    case tag_arith:
        map_ast(ast->oper.arithExp.highlvl,f);
        map_tac(ast->oper.arithExp.lowlvl,f);
        break;
    default:
        break;
    }   
    f(ast);
}

static void map_tac(TAC *tac, void (*f)(void *)) {
    f(tac->arr);
    f(tac);
}


void printParsedLine(AST * tree) {
    printAST(tree);
    printf("\n");
}

void FreeAST(AST * ast) {
    map_ast(ast, free);
}

AST * bsearch_statements(AST ** stmts, int len, int tar_numline) {
    int i = 0, j = len - 1;
    int mid;
    while(i <= j) {
        mid = (i + j) / 2;
        assert(stmts[mid]->tag == tag_numline);
        if (tar_numline > stmts[mid]->oper.numline.value) i = mid + 1; 
        if (tar_numline < stmts[mid]->oper.numline.value) j = mid - 1;
        if (tar_numline == stmts[mid]->oper.numline.value) return stmts[mid];
    }
    return NULL;
}

static AST * parse_OneWordStatementExp(int stmt) {
    AST * node = AllocNode();
    node->tag = tag_one_word_statement;
    node->oper.commonExp.stmt = stmt;
    return node;
}

static AST * parse_CommonExp(AST* (*f)(void),int stmt) {
    AST * node = AllocNode();
    node->tag = tag_common_statement;
    get_next_token();
    node->oper.commonExp.stmt = stmt;
    node->oper.commonExp.arg = f();
    if (node->oper.commonExp.arg->inTable) node->inTable =true;
    return node;

}

static AST * parse_IfStatementExp() {
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
    get_next_token();
    if (match(cur_token(), "ELSE")) {
        get_next_token();
        node->oper.ifstatementExp.elseExp = parse_AST();
    }
    return node;
}
static AST * parse_ReturnStatementExp() {
    return parse_OneWordStatementExp(op_return);
}
static AST * parse_EndStatementExp() {
    return parse_CommonExp(parse_OneWordStatementExp, op_end);
}
static AST * parse_ClsStatementExp() {
    return parse_OneWordStatementExp(op_cls);
}
static AST * parse_PrintStatementExp() {
    char * str = next_token();
    AST * node = AllocNode();
    node->tag = tag_common_statement;
    get_next_token();
    node->oper.commonExp.stmt = op_print;

    if (isSTRING(str)) {
        node->oper.commonExp.arg = parse_StrExp();
        add_symbol(node->oper.commonExp.arg);
    }
    else if (isINT(str) || isUNARY(str) || next_token()) {
        node->oper.commonExp.arg = parse_arith_expression();
    }
    else if (isVAR(str)) {
        node->oper.commonExp.arg = AllocNode();
        node->oper.commonExp.arg->oper.symbol = getId(str, S_TABLE);
        node->oper.commonExp.arg->tag = tag_symbol;
    }
    
    return node;
}
static AST * parse_LetStatementExp() {
    return parse_CommonExp(parse_AssignExp, op_let);
}
static AST * parse_InputStatementExp() {
    return parse_CommonExp(parse_VarExp, op_input);
}
static AST * parse_IncStatementExp() {
    return parse_CommonExp(parse_VarExp, op_inc); 
}
static AST * parse_DecStatementExp() {
    return parse_CommonExp(parse_VarExp, op_dec); 
}
static AST * parse_WhileStatementExp() {
    return parse_CommonExp(parse_arith_expression, op_while);
}
static AST * parse_RemStatementExp() {
    return parse_OneWordStatementExp(op_rem);
}
static AST * parse_WendStatementExp() {
    return parse_OneWordStatementExp(op_wend);
}
static AST * parse_NextStatementExp() {
    return parse_CommonExp(parse_VarExp, op_next);
}
static AST * parse_GotoStatementExp() {
    AST * node = parse_CommonExp(parse_IntExp, op_goto);
    int num = node->oper.commonExp.arg->oper.intExp;
    push_s(goto_s, num);
    return node;
}
static AST *parse_GoSubStatementExp() {
    AST * node = parse_GotoStatementExp();
    node->oper.commonExp.stmt = op_gosub;
    return node;
}

AST * AllocNode() {
    AST * node = (AST *) malloc(sizeof(AST));
    node->inTable = false;
    return node;
}


static AST * parse_ForStatementExp() {
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

static int get_op(char * str) {
    int h = (int) hash(str);
    
    switch (h)
    {
    case PLUS_H: return op_plus;
    case MINUS_H: return op_minus;
    case MUL_H: return op_mul;
    case DIV_H: return op_div; 
    case LESS_H: return op_less; 
    case GREATER_H: return op_greater;
    case LS_EQ_H: return op_less_eq;
    case GR_EQ_H: return op_greater_eq;
    case EQUAL_H: return op_equal;
    case NEQUAL_H: return op_not_eq;
    case NOT_H: return op_not;
    case AND_H: return op_and;
    case OR_H: return op_or;
    default: return -1;
    }
}

static AST * parse_BinaryExp(AST* left, char * operator, AST* right) {
    AST * node = AllocNode();
    node->tag = tag_binary;
    node->oper.binaryExp.operator = get_op(operator);
    node->oper.binaryExp.left = left;
    node->oper.binaryExp.right = right;
    return node;
}

static AST * parse_UnaryExp(char * operator) {
    AST * node = AllocNode();
    node->tag = tag_unary;
    node->oper.unaryExp.operator = get_op(operator);
    node->oper.unaryExp.operand = parse_leaf();
    return node;
}

// AST * parse_CallExp() {
//     AST * node = AllocNode();
//     node->tag = tag_call;s
//     node->oper.callExp.name = cur_token();
//     return node;
// }
static AST * parse_IntExp(){
    if (!isINT(cur_token())) {
        parse_syntax_error("type error with int");
    }
    int value = atoi(cur_token());
    AST * node = AllocNode();
    node->tag = tag_int;
    node->oper.intExp = value;
    return node;
}
static AST * parse_StrExp() {
    if (!isSTRING(cur_token())) {
        parse_syntax_error("type error with string");
    }
    AST * node = AllocNode();
    strcpy(node->oper.strExp, cur_token());
    node->tag = tag_str;
    return node;
}
static AST * parse_NumLineExp() {
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

static AST * parse_AssignExp() {
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
        char *t = cur_token();
        assert(isINT(t) | isFLOAT(t));
        value = parse_arith_expression();       
    }
    node->oper.assignExp.identifier = identifier;
    node->oper.assignExp.value = value;
    if (identifier->tag != tag_symbol)
        add_symbol(node);

    return node;
}
static AST * parse_VarExp() {
    if (isINT(cur_token())) {
        parse_syntax_error("numbers found in variable name");
    }
    hashmap * table = S_TABLE;
    int id = getId(cur_token(), table);
    AST * node = AllocNode();
    if (id != -1) {
        node->tag = tag_symbol;
        node->oper.symbol = id;
    } else {
	    strcpy(node->oper.varExp, cur_token());
	    node->tag = tag_var;
        add_symbol(node);
    }
    return node;
}
static AST * parse_FloatExp() {
    static int id = 0;
    AST * node = AllocNode();
    node->tag = tag_float;
    node->oper.floatExp = id++;
    push_str_s(float_inits, cur_token());
    return node;
}

static AST * parse_leaf() {
    char * token = cur_token();
    if (isUNARY(token)) {
        get_next_token();
        return parse_UnaryExp(token);
    }

    if (isINT(token)) return parse_IntExp();
    if (isSTRING(token)) return parse_StrExp();
    if (isVAR(token)) return parse_VarExp();
    if (isFLOAT(token)) return parse_FloatExp();
    

    parse_error("input not found for parsing lead");
    return NULL;
}

static int get_predecense(char * s_op) {
    int op = get_op(s_op);
    if (op <= op_or) return 1;                                       // "OR"
    else if (op <= op_and) return 2;                                 // "AND"
    else if (op >= op_less && op <= op_not_eq)  return 3;            // "<" ">" "<=" ">=" "=" "<>"
    else if (op == op_plus || op == op_plus) return 4;               // "+" "-"
    else if (op == op_mul || op == op_div) return 5;                 // "*" "/" 
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


static bool compare_prec(int new_prec, int prec) { // is the new prec smaller then the old one?
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

static unsigned int ParentCount_AST(AST * node) {
    if (node->tag != tag_binary) return 0;
    else {
        return 1 + ParentCount_AST(node->oper.binaryExp.left) + ParentCount_AST(node->oper.binaryExp.right);
    }
}
int p_atoi(char *str, int count) {
    int decimal = 1, num = 0, digit;
    for (int i = count-1; i >= 0; i--) {
        digit = str[i] - '0';
        num += digit * decimal;
        decimal *= 10;
    }
    return num;
}

static bool getLastChar(Atom atom, char c) {
    if (atom.i == 0) return false;
    int len = strlen(atom.c);
    return ((atom.c[len-1] == c) && (isNUM(atom.c[0]))); // [0-9]t is reserved for temp vars in TAC
}
bool isSymbolVar(Atom atom) {
    return getLastChar(atom, 's');
}
bool isTempVar(Atom atom) {
    return getLastChar(atom, 't');
}
bool isFloatVar(Atom atom) {
    return getLastChar(atom, 'f');
}


int postfix_GetIndex(char * str) { 
    return p_atoi(str, strlen(str)-1);
}

static void FillLiveArr(TAC * tac) {
    int tempind = 0;
    for (int i = 0; i < tac->len; i++)
    {
        if (isTempVar(tac->arr[i].result)) {
            tempind = postfix_GetIndex(tac->arr[i].result.c);                                   // index
            if (tac->LiveInterval[tempind][0] == -1) tac->LiveInterval[tempind][0] = i;     // update start interval if it has default value
            tac->LiveInterval[tempind][1] = i;                                              // update end interval //also wtf is this triple bs
        }
        if (isTempVar(tac->arr[i].arg1)) {
            tempind = postfix_GetIndex(tac->arr[i].arg1.c); 
            if (tac->LiveInterval[tempind][0] == -1) tac->LiveInterval[tempind][0] = i;
            tac->LiveInterval[tempind][1] = i; 
        }
        if (isTempVar(tac->arr[i].arg2)) {
            tempind = postfix_GetIndex(tac->arr[i].arg2.c); 
            if (tac->LiveInterval[tempind][0] == -1) tac->LiveInterval[tempind][0] = i;
            tac->LiveInterval[tempind][1] = i; 
        }
    }
}

static AST * parse_arith_expression() {
    if (ParenthesisLvl) parse_syntax_error("non-closed parethesis");
    float_flag = false;
    AST * ast = iter_parse_exp(-1);
    TAC * tac = ASTtoTAC(ast);
    

    AST* exp = (AST*) malloc(sizeof(AST));
    exp->tag = tag_arith;
    exp->oper.arithExp.highlvl = ast;
    exp->oper.arithExp.lowlvl = tac;

    FillLiveArr(tac);

    return exp;
}

static char * FillTac(AST * ast, TAC * tac, int * ind) {
    char * temp[2] = {NULL, NULL};
    AST * asts[2] = {ast->oper.binaryExp.left, ast->oper.binaryExp.right};
    Atom args[2], res;

    for (int i = 0; i < 2; i++) {
        if (asts[i]->tag == tag_binary) temp[i] = FillTac(asts[i], tac, ind);
      
        if (temp[i]) strcpy(args[i].c, temp[i]);
        else if (asts[i]->tag == tag_float) {
            tac->is_float = true;
            sprintf(args[i].c, "%df", asts[i]->oper.symbol); // f for float
        }
        else if (asts[i]->tag == tag_symbol) 
            sprintf(args[i].c, "%ds", asts[i]->oper.symbol); // s for symbol     
        else args[i].i = asts[i]->oper.intExp;
    }
    sprintf(res.c, "%dt", *ind); // t for temp
    tac->arr[*ind].operator = ast->oper.binaryExp.operator;
    tac->arr[*ind].arg1.i = args[1].i;
    tac->arr[*ind].arg2.i = args[0].i;
    strcpy(tac->arr[*ind].result.c, res.c);
    return tac->arr[(*ind)++].result.c;
}

TAC * ASTtoTAC(AST * node) {
    TAC * tac = (TAC*) malloc(sizeof(TAC));
    tac->is_float = false;
    for (int i = 0; i < LIVE_INTER_LEN; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            tac->LiveInterval[i][j] = -1;
        }
            
    }
    /*
      the following is just for one number....
     */
    if (node->tag != tag_binary) {
        char temp[10];
        TAC_Entry * arr = (TAC_Entry *) malloc(sizeof(TAC_Entry));
        
        arr->operator = op_null; // if null then use only arg2
        
        if (node->tag == tag_symbol) {
            sprintf(temp, "%ds", node->oper.symbol);
            strcpy(arr->arg2.c, temp);
        }
        else if (node->tag == tag_float) {
            sprintf(temp, "%df", node->oper.symbol);
            strcpy(arr->arg2.c, temp);
            tac->is_float = true;
        }
        else if (node->tag == tag_unary && node->oper.unaryExp.operator == op_minus) {
            arr->arg2.i = UINT_MAX - node->oper.unaryExp.operand->oper.intExp;
        }
        else {
            arr->arg2.i = node->oper.intExp;
        }
        arr->arg1.i = 0;
        arr->result.i = 0;
        tac->arr = arr;
        tac->len = 1;
        return tac;
    }
    unsigned int count = ParentCount_AST(node);
    
    TAC_Entry * arr = (TAC_Entry *) malloc(sizeof(TAC_Entry) * count);
    tac->arr = arr;
    tac->len = count;
    int * ind = (int*) malloc(sizeof(int));
    *ind = 0;

    FillTac(node, tac, ind);
    free(ind);
    return tac;
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
    case REM_H: return parse_RemStatementExp();
    case GOSUB_H: return parse_GoSubStatementExp();
    case GOTO_H: return parse_GotoStatementExp();
    case RETURN_H: return parse_ReturnStatementExp();
    case CLS_H: return parse_ClsStatementExp();
    case DEC_H: return parse_DecStatementExp();
    case INC_H: return parse_IncStatementExp();

    default: {
        AST * let = AllocNode();
        let->tag = tag_common_statement;
        let->oper.commonExp.stmt = op_let;
        let->oper.commonExp.arg = parse_AssignExp();
        return let;
    }
    }
    
    return NULL;

}
