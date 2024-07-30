#include "basicc.h"
#include <stdarg.h>
#include "parse.h"
FILE * tar;
int stackpos = 0;
AST** statements;
char * temp_name;


#define REG_AX 1 << 0
#define REG_BX 1 << 1
#define REG_CX 1 << 2
#define REG_DX 1 << 3
#define REG_SI 1 << 4
#define REG_DI 1 << 5
#define REG_SP 1 << 6
#define REG_BP 1 << 7
#define REG_R8 1 << 8
#define REG_R9 1 << 9
#define REG_R10 1 << 10
#define REG_R11 1 << 11
#define REG_R12 1 << 12
#define REG_R13 1 << 13
#define REG_R14 1 << 14
#define REG_R15 1 << 15


static void put(char * format, ...);
static void pop(char * str);
static void push(char * str);




static void data_section() {
    int ind = 0;
    for (int i = 0; i < S_TABLE_SIZE; i++)
    {
        if (S_TABLE->inds[i] == -1) break;   
        ind = S_TABLE->inds[i];
        LL_NODE * list = S_TABLE->list[ind];
        
        while (S_TABLE->list[ind]->id != i) list = list->next;
        if (list->type == type_string) {
            put("str%d: .ascii \"%s\\n\"", list->id, list->data.c);
        }
    }
}


static void multi_mov(int regs, ...) {
    va_list va;
    va_start(va, regs);
    if (regs & REG_AX) put("mov $%s, %%rax", va_arg(va, char*));
    if (regs & REG_BX) put("mov $%s, %%rbx", va_arg(va, char*));
    if (regs & REG_CX) put("mov $%s, %%rcx", va_arg(va, char*));
    if (regs & REG_DX) put("mov $%s, %%rdx", va_arg(va, char*));
    if (regs & REG_SI) put("mov $%s, %%rsi", va_arg(va, char*));
    if (regs & REG_DI) put("mov $%s, %%rdi", va_arg(va, char*));
    if (regs & REG_SP) put("mov $%s, %%rsp", va_arg(va, char*));
    if (regs & REG_BP) put("mov $%s, %%rbp", va_arg(va, char*));
    va_end(va);
}

static void new_stack_frame() {
    push("%rbp");
    put("mov %%rsp, %%rbp");    
}
static void end_stack_frame() {
    put("mov %%rsp, %%rbp");
    pop("%rbp");
}

static void start() {
    int i= 0;
    fprintf(tar, "_start:\n");
    new_stack_frame();
    while (statements[i]) {
        AST * node = statements[i];
        if (node->tag == tag_numline) node = node->oper.numline.next;
        switch (node->tag)
        {
        case tag_common_statement:
            int switch_h = hash(node->oper.commonExp.name);
            switch (switch_h)
            {
            case PRINT_H:

                put("");
                int id = node->oper.commonExp.arg->oper.symbol;
                int ind = S_TABLE->inds[id];
                char str[50] = {0};
                char len[25] = {0};
                sprintf(str, "str%d", id);
                sprintf(len, "%ld", strlen(S_TABLE->list[ind]->data.c)+1);
                multi_mov(REG_AX | REG_DX | REG_SI | REG_DI, "1", len, str, "1");
                put("syscall");
                put("");
                break;
            
            default:
                break;
            }
        
        default:
            break;
        }
        i++;
    }
    end_stack_frame();
    put("");
    multi_mov(REG_AX | REG_DI, "60", "0");
    put("syscall");
}

static void push(char * str) {
    put("push %s", str);
}

static void pop(char* str) {
    put("pop %s", str);
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

    tar = fopen(temp_name, "w"); 
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

    sprintf(as, "as %s -o %s.o", temp_name, output_name);
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