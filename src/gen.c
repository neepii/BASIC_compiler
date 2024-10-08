
#include "basicc.h"
FILE * tar;
unsigned int stackpos = 0;
AST** statements;
char * tar_path_name;

struct nAST{
    int key;
    AST * ast;
};
#define NON_HEAP_STACK_SIZE 50
unsigned int cur_pos;
//stacks
unsigned int frameArr[NON_HEAP_STACK_SIZE] = {0};
unsigned int frameInt = 0;
unsigned char counterArr[NON_HEAP_STACK_SIZE] = {0};
unsigned char counterInt = 0;
unsigned char counterAddr = 0;
struct nAST whileArr[NON_HEAP_STACK_SIZE] = {0};
unsigned char whileInt = 0;


#define REG_COUNT 32
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
static void handle_one_arg_op(int *regArr, Atom args[2], char *str[2],char * op, TAC * tac);
static void handle_cmp_op(int * regArr, Atom args[2], char* str[2],char * op,  TAC * tac);
static char *getReg(int ind);
static bool isOccupied(int *regArr, int reg);
static int OccupyReg_SIMD_only(int ind, int *regArr);
static int OccupyReg_all(int ind, int *regArr);



static void check_stack(unsigned int ind) {
    if ((ind + 1) > NON_HEAP_STACK_SIZE) {
        fprintf(stderr, "stack overflow\n");
        exit(1);
    }
}
static void rodata_section() {
    put_notab("clear_escape_seq: .asciz \"\\033[2J\\033[H\"");
    put_notab("ten_f: .double 10.0");
    for (int i = 0; i < S_TABLE_SIZE; i++) //maybe replace it with popstack loop
    {
        if (S_TABLE->inds[i] == -1) break;   
        int ind = S_TABLE->inds[i];
        LL_NODE ** list = &S_TABLE->list[ind];
        
        while ((*list)->id != i) list = &S_TABLE->list[ind]->next;
        if ((*list)->type == type_string) {
            put_notab("str%d: .ascii \"%s\"", i, (*list)->data.c);
        }
    }
    for (int i = 0; i < float_inits->last; i++) {
        char * fl = float_inits->arr[i];
        if (isDOT_FLOAT(fl)) {
            put_notab("float%d: .double 0%s", i , fl);
        } else {
            put_notab("float%d: .double %s", i , fl);
        }
    }
    
}    

static void data_section() {

    put(".lcomm bytestorage, 1");    
    put(".lcomm digitspace, 16");
    put(".lcomm stringspace, 32");
    put(".lcomm floatspace, 64");
}


static void multi_mov(int regs, ...) {
    va_list va;
    va_start(va, regs);
    bool pushed[REG_COUNT] = {false};
    for (int i = 0; i < REG_COUNT; i++) {
        int bit = 1 << i;
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
    check_stack(frameInt);
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

static int OccupyReg(int ind, int* regArr, int start) {
    int i;
    if (regArr == NULL) { //global
        for (i = start; i< REG_COUNT; i++) {
            if (i == 6 || i ==7) continue; // bp and sp regs
            if (!RegIsOccupied[i]) {
                RegIsOccupied[i] = true;
                return i;
            }
        }
    } else {
        for (i = start; i < REG_COUNT; i++) {
            if (i == 6 || i ==7) continue; 
            if (!isOccupied(regArr, i)) {
                regArr[i] = ind;
                return i;
            }
        }
    }
    return -1;
}

static int OccupyReg_SIMD_only(int ind, int *regArr) {
    return OccupyReg(ind, regArr, 16);
}

static int OccupyReg_all(int ind, int *regArr) {
    return OccupyReg(ind, regArr, 0);
}

/*
  if regArr is NULL -> ask only global
*/
static bool isOccupied(int *regArr, int reg) {
    if (regArr == NULL) return RegIsOccupied[reg];
    return (regArr[reg] != -1 || RegIsOccupied[reg]);
}

static int requestReg(char * tempVar, int * regArr, TAC *tac) {
    int tempind = postfix_GetIndex(tempVar);
    assert(tempind <= REG_COUNT);
    if (tac->is_float) {
        return OccupyReg_SIMD_only(tempind, regArr);
    }
    return OccupyReg_all(tempind, regArr);
}

static void handle_one_arg_op(int* regArr, Atom args[2], char * str[2], char * op, TAC* tac) {
    put("");
    bool pop_b = false;
    int temporary = requestReg(str[1], regArr,tac);
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
        "%r12", "%r13", "%r14", "%r15",
        
        "%xmm0", "%xmm1" ,"%xmm2", "%xmm3", //SIMD registers
        "%xmm4", "%xmm5" ,"%xmm6", "%xmm7",
        "%xmm8", "%xmm9" ,"%xmm10", "%xmm11",
        "$xmm12", "%xmm13" ,"%xmm14", "%xmm15",
    };
    return regs[ind];
}

static void handle_cmp_op(int * regArr, Atom args[2], char* str[2],char * op,  TAC * tac) {
    put("cmp %s, %s", str[0], str[1]);
    if (isOccupied(regArr, 0)) {
        int new_ind = requestReg(args[0].c, regArr, tac);
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
            if (addr == 0) {
                stackpos += 4;
                addr = stackpos - cur_frame();
                insert_hashmap_addr(S_TABLE, addr, id);
            }
            sprintf(temp[i], "-%d(%%rbp)", addr);
            str[i] = temp[i];

        }
        else if (isFloatVar(args[i])) {
            int id = postfix_GetIndex(args[i].c);
            sprintf(temp[i], "(float%d)", id);
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

    if (!(isTempVar(args[0]) || isTempVar(args[1])) ) {
        new_ind = requestReg(args[1].c, regArr, tac);
        char * reg_temp = getReg(new_ind);

	    RegIsNotCleared[new_ind] = true;
        
        if (tac->is_float) put("movsd %s, %s",str[1], reg_temp);
        else put("mov %s, %s",str[1], reg_temp);
        
        strcpy(args[1].c, reg_temp);
        str[1] = reg_temp;
    }
    if (isTempVar(args[0]) && !isTempVar(args[1])){
        char * temp_p = str[1];
        str[1] = str[0];
        str[0] = temp_p;
    }
    char * instr;
    switch (tac->arr[num].operator)
    {
    case op_null:
        break;
    case op_plus:
        instr = (tac->is_float) ? "addsd" : "add";
        put("%s %s, %s",instr,  str[0], str[1]);
        break;    
    case op_minus:
        instr = (tac->is_float) ? "subsd" : "sub";
        put("sub %s, %s",instr, str[0], str[1]);
        break;
    case op_mul:
        instr = (tac->is_float) ? "mulsd" : "mul";
        handle_one_arg_op(regArr, args, str, instr, tac);
        break;
    case op_div:
        instr = (tac->is_float) ? "divsd" : "div";
        handle_one_arg_op(regArr, args, str, instr, tac);
        break;
    case op_equal:
        handle_cmp_op(regArr, args, str, "sete", tac);
        break;
    case op_greater:
        handle_cmp_op(regArr, args, str, "setg", tac);
        break;
    case op_less:
        handle_cmp_op(regArr, args, str, "setl", tac);
        break;
    case op_less_eq:
        handle_cmp_op(regArr, args, str, "setle", tac);
        break;
    case op_greater_eq:
        handle_cmp_op(regArr, args, str, "setge", tac);
        break;
    case op_not_eq:
        handle_cmp_op(regArr, args, str, "setne",tac);
        break;
    case op_or:
        put("or %s, %s",  str[0], str[1]);
        break;
    case op_and:
        put("and %s, %s",  str[0], str[1]);
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

    int * LocalRegOccup = (int*) malloc(sizeof(int) * REG_COUNT); // ???
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
    
    put("sub $%d, %rsp", (nodes[2] != NULL) ? 12 : 8);
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

static void check_addr(int *addr, int sym) {
    if (*addr == 0) {
        stackpos += 4;
        *addr = stackpos - cur_frame();
        insert_hashmap_addr(S_TABLE, *addr, sym);
    }
}
static void handle_var_ascii(char *str, AST *arg) {
    int addr = getAddrByID(arg->oper.symbol, S_TABLE);
    check_addr(&addr, arg->oper.symbol);
    sprintf(str, "-%d(%%rbp)", addr);
    put("mov %s, %%rax", "$digitspace");
    put("movl %s, %%edi", str);
    call("itoa");
    put("mov %%rax, %%rdx");
    multi_mov(REG_AX | REG_SI | REG_DI, "$1", "$digitspace", "$1");
}

void handle_common_statements(AST * node) {
    AST * arg = node->oper.commonExp.arg;
    switch (node->oper.commonExp.stmt) {
    case op_print: {
        put("");
        char str[64] = {0};
        char len[25] = {0};
        if (arg->tag != tag_symbol && arg->tag != tag_assign) { //my god what have i done
            char *reg = eval_arith_exp(arg);
            assert(arg->tag == tag_arith);
            if (arg->oper.arithExp.lowlvl->is_float) {
                put("mov %s, %%rax", "$digitspace");
                put("mov $8, %rcx");
                call("ftoa");
                put("mov %%rax, %rdx");
                multi_mov(REG_AX | REG_SI | REG_DI, "$1", "$digitspace", "$1");                
            } else {
            if (match(reg, "%rax") ) {
                put("mov %%rax, %%rdi");
                put("mov %s, %%rax", "$digitspace"); 
            } else {
                put("mov $digitspace, %%rax");
                put("mov %s, %%rdi", reg);
            }
            call("itoa");
            put("mov %%rax, %%rdx");
            multi_mov(REG_AX | REG_SI | REG_DI, "$1", "$digitspace", "$1");
            }
        } else { //is symbol
            int ind = getIndexBySymbol(arg);
            switch (S_TABLE->list[ind]->type) {
            case type_string:
                sprintf(str, "$str%d", arg->oper.symbol);
                sprintf(len, "$%ld", strlen(S_TABLE->list[ind]->data.c));
                multi_mov(REG_AX | REG_DX | REG_SI | REG_DI, "$1", len, str, "$1");
                break;
            case type_pointer_var: {
                handle_var_ascii(str, arg);
                break; 
            }
            default:
                break;
            }
        }
        call("newline");
        break;
    }
    case op_goto:
        put("jmp goto%d",arg->oper.intExp);
        break;
    case op_gosub:
        put("call goto%d", arg->oper.intExp);
        break;
    case op_input: {
        multi_mov(REG_AX | REG_DX | REG_SI | REG_DI, "$0", "$32", "$stringspace", "$0");
        put("mov $stringspace, %rdi");
        call("atoi");
        int addr = getAddrByAST(arg, S_TABLE);
        check_addr(&addr, arg->oper.symbol);
        put("movl %%eax, -%d(%%rbp)", addr);
        break;
    }
    case op_dec:{
        put("");
        int addr = getAddrByAST(arg, S_TABLE);
        put("sub $1, -%d(%%rbp)", addr);
        break;
    }
    case op_inc: {
        put("");
        int addr = getAddrByAST(arg, S_TABLE);
        put("add $1, -%d(%%rbp)", addr);
        break;
    }
    case op_let: {
        put("");
        AST * identifier = arg->oper.assignExp.identifier;
        int sym = identifier->oper.symbol;
        int addr = getAddrByAST(identifier, S_TABLE);
        check_addr(&addr, sym);
        char * value = eval_arith_exp(arg->oper.assignExp.value);


        if (value[1] != 'x') {
            char x86[40];
            x86_64_to_x86(value,x86);
            put("sub $4, %rsp");
            put("movl %s, -%d(%%rbp)",x86,addr);
        } else {
            put("sub $4, %rsp");
            put("movsd %s, -%d(%%rbp)", value, addr);
        }


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
        counterInt--; //???
        break;
    }
    case op_while: {
        static int id = 0;
        check_stack(whileInt);
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
    case op_cls:
        multi_mov(REG_AX | REG_DX | REG_SI | REG_DI, "$1", "$7", "$clear_escape_seq", "$1");
        break;
    case op_return:
        put("ret");
        break;
    case op_wend: {
        put_notab("while_cmp%d:", whileArr[whileInt].key);
        char * reg = eval_arith_exp(whileArr[whileInt].ast);
        put("cmp $1, %s", reg);
        put("jge while_iter%d", whileArr[whileInt].key);
        whileInt++;
        break;
    }
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
        cur_pos = node->oper.numline.value;
        put_notab("goto%d:", cur_pos);
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


void make_target_src() {

    tar = fopen(tar_path_name, "w"); 
    put(".code64");
    put_notab(".section .data");
    data_section();
    put_notab(".section .rodata");
    rodata_section();    
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
