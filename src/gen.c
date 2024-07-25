#include "basicc.h"
#include "stdarg.h"
FILE * tar;
int stackpos = 0;

static char * regs = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void ASTtoASM(AST * line) {
    switch (line->tag)
    {
    case tag_numline:
        line = line->oper.numline.next;
        break;
    case tag_common_statement:
        break;
    
    default:
        break;
    }
}


static void push(char * reg) {
    fprintf("push %s\n", reg);
}

static void pop(char* reg) {
    fprintf("pop %s\n", reg);
}

static void put(char * str) {
    fprintf(tar, str);
    fprintf(tar, "\n");
}

void make_target_src() { //my brain hurts
    tar = fopen("XXX0808.s", "w"); 
    put(".code64");
    put(".section .data"); 
    put("");
    put(".global _start"); 
    put(".section .text");
    put("_start:");
}