#include "basicc.h"
FILE * tar;
unsigned int stackpos = 0;
AST** statements;
char * tar_path_name;

struct nAST{
    int key;
    AST * ast;
};
//stacks
unsigned int frameArr[50] = {0};
unsigned int frameInt = 0;
unsigned char counterArr[50] = {0};
unsigned char counterInt = 0;
unsigned char counterAddr = 0;
struct nAST whileArr[50] = {0};
unsigned char whileInt = 0;

#define REG_COUNT 16
bool RegIsNotCleared[REG_COUNT] = {false};
bool RegIsOccupied[REG_COUNT] = {false}; //global
bool hasEnd = false;



#define REG_AX  1 << 0
#define REG_BX  1 << 1
#define REG_CX  1 << 2
#define REG_DX  1 << 3
#define REG_SI  1 << 4
#define REG_DI  1 << 5
#define REG_SP  1 << 6
#define REG_BP  1 << 7
#define REG_R8  1 << 8
#define REG_R9  1 << 9
#define REG_R10 1 << 10
#define REG_R11 1 << 11
#define REG_R12 1 << 12
#define REG_R13 1 << 13
#define REG_R14 1 << 14
#define REG_R15 1 << 15

static void put(char * format, ...);
static void put_notab(char * format, ...);
static void pop(char * str);
static void push(char * str);
static void call(char *str);
static void handle_statements(AST *node);
static bool getLastChar(Atom atom, char c);
static void handle_one_arg_op(int* regArr, Atom args[2], char * str[2], char * op);
static void handle_cmp_op(int * regArr, Atom args[2], char* str[2], char * op);
static char *getReg(int ind);
static bool isOccupied(int *regArr, int reg);






static void data_section() {
    int ind = 0;
    put(".lcomm digitspace, 16");
    put(".lcomm bytestorage, 1");
    put(".lcomm stringspace, 32");
    for (int i = 0; i < S_TABLE_SIZE; i++)
    {
        if (S_TABLE->inds[i] == -1) break;   
        ind = S_TABLE->inds[i];
        LL_NODE ** list = &S_TABLE->list[ind];
        
        while ((*list)->id != i) list = &S_TABLE->list[ind]->next;
        if ((*list)->type == type_string) {
            put("str%d: .ascii \"%s\"", (*list)->id, (*list)->data.c);
        }
    }
}


static void multi_mov(int regs, ...) {
    va_list va;
    va_start(va, regs);
    bool pushed[REG_COUNT] = {false};
    for (int i = 0; i < REG_COUNT; i++) {
        int bit = 1 << i;
        char * reg_p;
        if (regs & bit) {
            if (RegIsOccupied[i]) {
                put("push %s", getReg(i));
                pushed[i] = true;
            }
            put("mov %s, %s", va_arg(va, char*), getReg(i));
        }
    }
    put("syscall");
    for (int i = REG_COUNT-1; i >= 0; i--) {
        if (pushed[i]) put("pop %s", getReg(i));
    }
    va_end(va);
}

static void new_stack_frame() {
    push("%rbp");
    put("mov %%rsp, %%rbp");
    stackpos += 8;
    if ((frameInt + 1) > 50) {
        fprintf(stderr, "stack overflow\n");
    }
    frameArr[++frameInt] = stackpos;
}
static void end_stack_frame() {
    put("mov %%rsp, %%rbp");
    pop("%rbp");
    stackpos -= 8;
    frameInt--;
}
unsigned int cur_frame() {
    return frameArr[frameInt];
}

/*
  if RegArr is NULL
  it will occupy globally
 */
static int OccupyReg(int ind, int* regArr) {
    int i;
    if (regArr == NULL) { //global
        for (i = 0; i< REG_COUNT; i++) {
            if (i == 6 || i ==7) continue; // bp and sp regs
            if (!RegIsOccupied[i]) {
                RegIsOccupied[i] = true;
                return i;
            }
        }
    } else {
        for (i = 0; i < REG_COUNT; i++) {
            if (i == 6 || i ==7) continue; 
            if (!isOccupied(regArr, i)) {
                regArr[i] = ind;
                return i;
            }
        }
    }
    return -1;
}
/*
  if regArr is NULL -> ask only global
*/
static bool isOccupied(int *regArr, int reg) {
    if (regArr == NULL) return RegIsOccupied[reg];
    return (regArr[reg] != -1 || RegIsOccupied[reg]);
}

static int requestReg(char * tempVar, int * regArr) {
    int tempind = postfix_GetIndex(tempVar);
    assert(tempind <= 16);
    return OccupyReg(tempind, regArr);
}

static void handle_one_arg_op(int* regArr, Atom args[2], char * str[2], char * op) {
    put("");
    bool pop_b = false;
    int temporary = requestReg(str[1], regArr);
    char * reg = getReg(temporary);
    
    if (isOccupied(regArr, 0) && !match(str[1], "%rax")) {
        assert(0);
        // put("xchg %%rax, %s",reg);    

        // if (isOccupied(regArr,3)) { //rdx
        //     push("%rdx");
        //     pop_b = true;
        // }

        // put("mov %s, %%rax", str[0]);
        // put("%s %s",op, str[1]);
        // if (pop_b)  pop("%rdx");
        // put("mov %%rax, %s", str[1]);

        // put("mov %s, %%rax", getReg(temporary));
    } else {
        assert(isOccupied(regArr, 0));
        
        if (isOccupied(regArr,3)) { //rdx
            push("%rdx");
            pop_b = true;
        }
        put("mov %s, %s", str[0], reg);
        put("xchg %%rax, %s",reg);
        put("%s %s",op, reg);

        if (!match(str[1], "%rax")) put("mov %%rax, %s", str[1]);
        if (pop_b) pop("%rdx");
    }
    regArr[temporary] = -1;
}

static char * getReg(int ind) {
    static char * regs[REG_COUNT] = {
        "%rax", "%rbx", "%rcx", "%rdx",
        "%rsi", "%rdi",

        "%rsp", "%rbp", //cant use this

        "%r8", "%r9", "%r10", "%r11",
        "%r12", "%r13", "%r14", "%r15"
    };
    return regs[ind];
}

static void handle_cmp_op(int * regArr, Atom args[2], char* str[2], char * op) {
    put("cmp %s, %s", str[0], str[1]);
    if (isOccupied(regArr, 0)) {
        int new_ind = requestReg(args[0].c, regArr);
        char *reg = getReg(new_ind);
        if (RegIsNotCleared[new_ind])
            put("xor %s, %s", reg, reg);
        RegIsNotCleared[new_ind] = true;
        put("xchg %%rax, %s", reg);
        put("%s %%al", op);
        put("movzbl %%al, %%eax");
        put("xchg %%rax, %s", reg);
        str[1] = reg;
    } else {
        put("%s %%al", op);
        put("movzbl %%al, %%eax");
        put("mov %%rax, %s", str[1]);
    }
}

char * put_tac(int num, TAC* tac, int *regArr) {
    char * str[2];
    Atom args[2] = {tac->arr[num].arg1, tac->arr[num].arg2};
    char temp[2][20] = {0};
    int new_ind;

    for (int i = 0; i < 2; i++)
    {
        if (isTempVar(args[i])) {
            for (int j = num;j >= 0; j--) {
                if (match(args[i].c, tac->arr[j].result.c)) {
                    str[i] =put_tac(j,tac, regArr);
                    break;
                }
            }
        }
        else if (isSymbolVar(args[i])) {
            int id = postfix_GetIndex(args[i].c);
            int addr = getAddrByID(id, S_TABLE);
            sprintf(temp[i], "-%d(%%rbp)", addr);
            str[i] = temp[i];

        } else {
            sprintf(temp[i], "$%lld", args[i].i);
            str[i] = temp[i];
        }   
    }

    //free registers
    for (int i = 0; i < tac->len; i++) {
        if (tac->LiveInterval[i][1] < num) { //not being used

            for (int j = 0; j < REG_COUNT; j++) { // linear search
                if (regArr[j] == i) {
                    regArr[i] = -1;
                    RegIsNotCleared[i] = false;
                }
                break;
            }
        }
    }

    if (! (isTempVar(args[0]) || isTempVar(args[1])) ) {
        new_ind = requestReg(args[1].c, regArr);
        char * reg_temp = getReg(new_ind);
	    if (RegIsNotCleared[new_ind]) put("xor %s, %s", reg_temp, reg_temp);
	    RegIsNotCleared[new_ind] = true;    
        put("mov %s, %s",str[1], reg_temp);
        strcpy(args[1].c, reg_temp);
        str[1] = reg_temp;
    }
    if (isTempVar(args[0]) && !isTempVar(args[1])){
        char * temp_p = str[1];
        str[1] = str[0];
        str[0] = temp_p;
    }

    switch (tac->arr[num].operator)
    {
    case op_null:
        break;
    case op_plus:
        put("add %s, %s", str[0], str[1]);
        break;
    case op_minus:
        put("sub %s, %s", str[0], str[1]);
        break;
    case op_mul:
        handle_one_arg_op(regArr, args, str, "mul");
        break;
    case op_div:
        handle_one_arg_op(regArr, args, str, "div");
        break;
    case op_equal:
        handle_cmp_op(regArr, args, str, "sete");
        break;
    case op_greater:
        handle_cmp_op(regArr, args, str, "setng");
        break;
    case op_less:
        handle_cmp_op(regArr, args, str, "setnl");
        break;
    case op_less_eq:
        handle_cmp_op(regArr, args, str, "setnle");
        break;
    case op_greater_eq:
        handle_cmp_op(regArr, args, str, "setnge");
        break;
    case op_not_eq:
        handle_cmp_op(regArr, args, str, "setne");
        break;
    default:
        break;
    }


    return str[1];  // return register char*
}

void x86_64_to_x86(char * str, char new[40]) {
    strncpy(new, str, 5);
    new[1] = 'e';
}

char * eval_arith_exp(AST * node) {
    assert(node->tag == tag_arith);
    TAC * tac = node->oper.arithExp.lowlvl;

    int * LocalRegOccup = (int*) malloc(sizeof(int) * REG_COUNT);
    for (int i = 0; i < REG_COUNT; i++) LocalRegOccup[i] = -1; // -1 == register is free
    
    char * str = put_tac(tac->len-1, tac, LocalRegOccup);
    free(LocalRegOccup);
    return str;
}

static void handle_for_statement(AST * node) {
    put("");
    AST * assignment = node->oper.forstatementExp.initial;
    int sym = assignment->oper.assignExp.identifier->oper.symbol;
    char * values[3];
    int address[3];
    char x86[3][40];
    AST * nodes[3] = {
        assignment->oper.assignExp.value,
        node->oper.forstatementExp.final,
        (node->oper.forstatementExp.step) ? node->oper.forstatementExp.step : NULL
    };
    
    put("sub $%d, %rsp", (nodes[3] != NULL) ? 12 : 8);
    for (int i = 0; i < 3; i++) {
        if (nodes[i] == NULL) continue;
        values[i] = eval_arith_exp(nodes[i]);
        x86_64_to_x86(values[i],x86[i]);
        stackpos += 4;
        address[i]= stackpos - cur_frame();
        put("movl %s, -%d(%%rbp)",x86[i],address[i]);        
    }

    put("jmp for_cmp%d", sym);
    put_notab("for_iter%d:", sym);
    counterAddr = address[0];
}


void handle_if_statement(AST * node) {
    put("");
    static int if_ind = 0;
    char * predicate_reg = eval_arith_exp(node->oper.ifstatementExp.predicate);
    put("cmp $1, %s", predicate_reg);
    put("jl .false%d",if_ind);
    handle_statements(node->oper.ifstatementExp.thenExp);
    put("jmp .end%d", if_ind);
    put_notab(".false%d:", if_ind);
    if (node->oper.ifstatementExp.elseExp) handle_statements(node->oper.ifstatementExp.elseExp);
    put_notab(".end%d:", if_ind);
    put("");
    if_ind++;

}

void handle_common_statements(AST * node) {
    AST * arg = node->oper.commonExp.arg;
    switch (node->oper.commonExp.stmt) {
    case op_print: {
        put("");
        char str[64] = {0};
        char len[25] = {0};

        if (arg->tag != tag_symbol && arg->tag != tag_assign) { //my god what have i done
            char * reg = eval_arith_exp(arg);
            if (match(reg, "%rax") ) {
                put("mov %%rax, %%rdi");
                put("mov %s, %%rax", "$digitspace");
            } else {
                put("mov $digitspace, %%rax");
                put("mov %s, %%rdi", reg);
            }
            call("uitoa");
            put("mov %%rax, %%rdx");
            multi_mov(REG_AX | REG_SI | REG_DI, "$1", "$digitspace", "$1");
        } else { //is symbol
            int ind = getIndexBySymbol(arg);
            switch (S_TABLE->list[ind]->type) {
            case type_string:
                sprintf(str, "$str%d", arg->oper.symbol);
                sprintf(len, "$%ld", strlen(S_TABLE->list[ind]->data.c));
                multi_mov(REG_AX | REG_DX | REG_SI | REG_DI, "$1", len, str, "$1");
                break;
            case type_pointer_var: {
                int addr = getAddrByAST(arg, S_TABLE);
                sprintf(str, "-%d(%%rbp)", addr);
                put("mov %s, %%rax", "$digitspace");
                put("movl %s, %%edi", str);
                call("uitoa");
                put("mov %%rax, %%rdx");
                multi_mov(REG_AX | REG_SI | REG_DI, "$1", "$digitspace", "$1");
                break; 
                }
            }
        }
        call("newline");
        break;
    }
    case op_goto:
        put("jmp goto%d",arg->oper.intExp);
        break;
    case op_input: {
        int ind = getIndexBySymbol(arg);
        multi_mov(REG_AX | REG_DX | REG_SI | REG_DI, "$0", "$32", "$stringspace", "$0");
        put("mov $stringspace, %rdi");
        call("atoi");
        stackpos += 4;
        put("movl %%eax, -%d(%%rbp)", stackpos-cur_frame());
        strncpy(S_TABLE->list[ind]->data.c, "$stringspace", 13);
        S_TABLE->list[ind]->data.addr = stackpos - cur_frame();
        break;
    }
    case op_let: {
        put("");
        AST * identifier = arg->oper.assignExp.identifier;
        char * value = eval_arith_exp(arg->oper.assignExp.value);
        char x86[40];
        stackpos += 4;
        x86_64_to_x86(value,x86);
        put("sub $4, %rsp");
        put("movl %s, -%d(%%rbp)",x86,stackpos-cur_frame());
        insert_hashmap_addr(S_TABLE,stackpos- cur_frame(), identifier->oper.symbol);
        break;
    }
    case op_next: {
        int sym = node->oper.commonExp.arg->oper.symbol;
        char * reg = getReg(counterArr[counterInt]);
        char x86[40];
        x86_64_to_x86(reg,x86);
        put("addl $1, -%d(%%rbp)", counterAddr);
        put_notab("for_cmp%d:", sym);
        put("mov -%d(%%rbp), %s",counterAddr, x86);
        put("cmp -%d(%%rbp), %s",counterAddr + 4, x86);
        put("jle for_iter%d", sym);
        RegIsOccupied[counterArr[counterInt]] = false;
        counterInt--;
        break;
    }
    case op_while: {
        static int id = 0;
        whileArr[whileInt].key = id++;
        whileArr[whileInt].ast = arg;
        put("jmp while_cmp%d", whileArr[whileInt].key);
        put_notab("while_iter%d:", whileArr[whileInt].key);
        break;
    }
    case op_end:
        put("");
        end_stack_frame();
        multi_mov(REG_AX | REG_DI, "$60", "$0");
        hasEnd = true;
        break;
    default:
        break;
    }
    
}
static void handle_one_word_statements(AST *node) {
    switch (node->oper.one_word_stmt)
    {
    case op_rem:
        break;
    case op_wend:
        put_notab("while_cmp%d:", whileArr[whileInt].key);
        char * reg = eval_arith_exp(whileArr[whileInt].ast);
        put("cmp $1, %s", reg);
        put("jge while_iter%d", whileArr[whileInt].key);
        whileInt++;
        break;
    default:
        break;
    }
}

static void handle_statements(AST *node) { 
    switch (node->tag)
    {
    case tag_common_statement:
        handle_common_statements(node);
        break;
    case tag_one_word_statement:
        handle_one_word_statements(node);
        break;
    case tag_if:
        handle_if_statement(node);
        break;
    case tag_for:
        handle_for_statement(node);
        break;
    default:
        break;
    }

}

static void start() {
    int i= 0;
    put_notab("_start:");
    new_stack_frame();
    while (statements[i]) {
    AST * node = statements[i];
    if (node->tag == tag_numline && node->oper.numline.isGotoLabel) {
        put_notab("goto%d:", node->oper.numline.value);
        node = node->oper.numline.next;
    }
    else if (node->tag == tag_numline) node = node->oper.numline.next;
    handle_statements(node);
    i++;
    }
    if (!hasEnd) fprintf(stderr, "ERROR: no end statement\n");
    
    put("");

}

static void push(char * str) {
    put("push %s", str);
}

static void pop(char* str) {
    put("pop %s", str);
}

static void call(char* str) {
    bool pushed[REG_COUNT] = {false};
    for (int i = 0; i < REG_COUNT; i++) {
        if (RegIsOccupied[i]) {
            put("push %s", getReg(i));
            pushed[i] = true;
        }
    }
    put("call %s", str);
    for (int i = REG_COUNT-1; i >= 0; i--) {
        if (pushed[i]) put("pop %s", getReg(i));
    }
}

static void put(char * format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(tar,"\t");
    vfprintf(tar,format, args);
    fprintf(tar, "\n");
    va_end(args);
}
static void put_notab(char * format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(tar,format, args);
    fprintf(tar, "\n");
    va_end(args);
}


void make_target_src() { //my brain hurts

    tar = fopen(tar_path_name, "w"); 
    put(".code64");
    put_notab(".section .data"); 
    data_section();
    put_notab(".global _start"); 
    put_notab(".section .text");
    include("src/funcs.inc");
    start();
    fclose(tar);
}

void compile(char * output_name)  {

    char as[100];
    char ld[100];
    char rm[100];

    sprintf(as, "as %s -o %s.o", tar_path_name, output_name);
    sprintf(ld, "ld %s.o -o %s", output_name, output_name);
    sprintf(rm, "rm -rf %s.o", output_name);
    system(as);
    system(ld);
    system(rm);
}
void include(char * path) {
    FILE * fp = fopen(path, "r");
    char line[50];
    while(1) {
        fgets(line,50,fp);
        if (feof(fp)) break;
        fprintf(tar, "%s",line);
    }

    fclose(fp);
}
