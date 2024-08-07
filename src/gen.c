
#include "basicc.h"
FILE * tar;
unsigned int stackpos = 0;
AST** statements;
char * tar_path_name;
unsigned int frameArr[50] = {0};
unsigned int frameInt = 0;

#define REG_COUNT 16
bool RegIsNotCleared[REG_COUNT] = {false};
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
static void pop(char * str);
static void push(char * str);
static void call(char *str);
static void handle_statements(AST *node);




static void data_section() {
    int ind = 0;
    put(".lcomm digitspace, 8");
    put(".lcomm bytestorage, 1");
    put(".lcomm stringspace, 32");
    for (int i = 0; i < S_TABLE_SIZE; i++)
    {
        if (S_TABLE->inds[i] == -1) break;   
        ind = S_TABLE->inds[i];
        LL_NODE * list = S_TABLE->list[ind];
        
        while (S_TABLE->list[ind]->id != i) list = list->next;
        if (list->type == type_string) {
            put("str%d: .ascii \"%s\"", list->id, list->data.c);
        }
    }
}


static void multi_mov(int regs, ...) {
    va_list va;
    va_start(va, regs);
    if (regs & REG_AX) put("mov %s, %%rax", va_arg(va, char*));
    if (regs & REG_BX) put("mov %s, %%rbx", va_arg(va, char*));
    if (regs & REG_CX) put("mov %s, %%rcx", va_arg(va, char*));
    if (regs & REG_DX) put("mov %s, %%rdx", va_arg(va, char*));
    if (regs & REG_SI) put("mov %s, %%rsi", va_arg(va, char*));
    if (regs & REG_DI) put("mov %s, %%rdi", va_arg(va, char*));
    if (regs & REG_SP) put("mov %s, %%rsp", va_arg(va, char*));
    if (regs & REG_BP) put("mov %s, %%rbp", va_arg(va, char*));
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

static int OccupyReg(int ind, int* regArr) {
    int i;
    for (i = 0; i < REG_COUNT; i++) {
        if (i == 6 || i ==7) continue; // bp and sp regs
        if (regArr[i] == -1) {
            regArr[i] = ind;
            break;
        }
    }
    return i;
}

static int requestReg(char * tempVar, int * regArr) {
    int tempind = GetTempIndex(tempVar);
    assert(tempind <= 16);
    return OccupyReg(tempind, regArr);
}

static void handle_one_arg_op(int* regArr, char * regs[16], Atom args[2], char * str[2], char * op) {
    bool pop_b = false;
    int temporary = requestReg(str[1], regArr);
    if (regArr[0] != -1 && !match(str[1], "%rax")) {
        put("xchg %%rax, %s", regs[temporary]);

        if (regArr[3] != -1) {
            push("%rdx");
            pop_b = true;
        }

        put("mov %s, %%rax", str[0]);
        put("%s %s",op, str[1]);
        if (pop_b)  pop("%rdx");
        put("mov %%rax, %s", str[1]);

        put("mov %s, %%rax", regs[temporary]);
    } else {
        assert(regArr[0] != -1);
        
        if (regArr[3] != -1) { //rdx is occupied
            push("%rdx");
        }
        put("mov %s, %s", str[0],regs[temporary]);
        put("%s %s",op, regs[temporary]);

        if (!match(str[1], "%rax")) put("mov %%rax, %s", str[1]);
        if (pop_b) pop("%rdx");
    }
    regArr[temporary] = -1;
}

char * put_tac(int num, TAC* tac, int *regArr) {
    static char * regs[REG_COUNT] = {
        "%rax", "%rbx", "%rcx", "%rdx",
        "%rsi", "%rdi",

        "%rsp", "%rbp", //cant use this

        "%r8", "%r9", "%r10", "%r11",
        "%r12", "%r13", "%r14", "%r15"
    };
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
        char * reg_temp = regs[new_ind];
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
        handle_one_arg_op(regArr, regs, args, str, "mul");
        break;
    case op_div:
        handle_one_arg_op(regArr, regs, args, str, "div");
        break;
    case op_equal:
	put("cmp %s, %s", str[0], str[1]);
	if (regArr[0] != -1) {
	    new_ind = requestReg(args[0].c, regArr);
	    if (RegIsNotCleared[new_ind]) put("xor %s, %s", regs[new_ind],regs[new_ind]);
	    RegIsNotCleared[new_ind] = true;    
	    put("xchg %%rax, %s", regs[new_ind]);
	    call("bool_equal");
	    put("xchg %%rax, %s", regs[new_ind]);
	    str[1] = regs[new_ind]; 
	} else {
	    call("bool_equal");
	    put("mov %%rax, %s", str[1]);
	}
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

    int * isRegOccup = (int*) malloc(sizeof(int) * REG_COUNT);
    for (int i = 0; i < REG_COUNT; i++) isRegOccup[i] = -1; // -1 == register is free
    
    return put_tac(tac->len-1, tac, isRegOccup);
}

void handle_if_statement(AST * node) {
    static int if_ind = 0;
    char * predicate_reg = eval_arith_exp(node->oper.ifstatementExp.predicate);
    put("cmp $1, %s", predicate_reg);
    put("jge .false%d",if_ind);
    handle_statements(node->oper.ifstatementExp.thenExp);
    
    fprintf(tar, ".false%d:", if_ind);
    if (node->oper.ifstatementExp.elseExp) handle_statements(node->oper.ifstatementExp.elseExp);
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
                multi_mov(REG_AX | REG_DI, "$digitspace", reg);
            }
            call("uitoa");
            put("mov %%rax, %%rdx");
            multi_mov(REG_AX | REG_SI | REG_DI, "$1", "$digitspace", "$1");
        } else { //is symbol
            int ind = getIndexBySymbol(arg);
            if (S_TABLE->list[ind]->type == type_string) {
                sprintf(str, "$str%d", arg->oper.symbol);
                sprintf(len, "$%ld", strlen(S_TABLE->list[ind]->data.c)+1);
                multi_mov(REG_AX | REG_DX | REG_SI | REG_DI, "$1", len, str, "$1");
            } else if (S_TABLE->list[ind]->type == type_int) {
                sprintf(str, "-%d(%%rbp)", S_TABLE->list[ind]->data.addr);
                put("mov %s, %%rax", "$digitspace");
                put("movl %s, %%edi", str);
                call("uitoa");
                put("mov %%rax, %%rdx");
                multi_mov(REG_AX | REG_SI | REG_DI, "$1", "$digitspace", "$1");
            } else if (S_TABLE->list[ind]->type == type_pointer_var) {
                put("mov $stringspace, %%rax");
                call("strlen");
                put("mov %%rax, %%rdx");
                sprintf(str, "%s", S_TABLE->list[ind]->data.c);
                multi_mov(REG_AX | REG_SI | REG_DI, "$1", str, "$1");
            }
        }
        
        put("syscall");
        call("newline");
        break;
    }
    case op_goto:
        put("jmp goto%d",arg->oper.intExp);
        break;
    case op_input: {
        int ind = getIndexBySymbol(arg);
        multi_mov(REG_AX | REG_DX | REG_SI | REG_DI, "$0", "$32", "$stringspace", "$0");
        put("syscall");
        put("mov $stringspace, %rdi");
        call("atoi");
        stackpos += 4;
        put("movl %%eax, -%d(%%rbp)", stackpos-cur_frame());
        strncpy(S_TABLE->list[ind]->data.c, "$stringspace", 13);
        S_TABLE->list[ind]->type = type_int;
        S_TABLE->list[ind]->data.addr = stackpos - cur_frame();
        break;
    }
    case op_let: {
        put("");
        int ind = getIndexBySymbol(arg->oper.assignExp.identifier);
        char * value = eval_arith_exp(arg->oper.assignExp.value);
        char x86[40];
        stackpos += 4;
        x86_64_to_x86(value,x86);
        put("movl %s, -%d(%%rbp)",x86,stackpos-cur_frame());
        S_TABLE->list[ind]->data.addr = stackpos - cur_frame();
        break;
    }
    case op_end:
        put("");
        end_stack_frame();
        multi_mov(REG_AX | REG_DI, "$60", "$0");
        put("syscall");
        hasEnd = true;
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
    default:
        break;
    }

}

static void start() {
    int i= 0;
    fprintf(tar, "_start:\n");
    new_stack_frame();
    while (statements[i]) {
    AST * node = statements[i];
    if (node->tag == tag_numline && node->oper.numline.isGotoLabel) {
        fprintf(tar, "goto%d:", node->oper.numline.value);
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
    put("call %s", str);
}

static void put(char * format, ...) {
    va_list args;
    
    va_start(args, format);
    fprintf(tar,"\t");
    vfprintf(tar,format, args);
    fprintf(tar, "\n");
    va_end(args);
}

void make_target_src() { //my brain hurts

    tar = fopen(tar_path_name, "w"); 
    put(".code64");
    fprintf(tar,".section .data\n"); 
    data_section();
    fprintf(tar,".global _start\n"); 
    fprintf(tar, ".section .text\n");
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
