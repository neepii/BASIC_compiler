#ifndef HASH_H_
#define HASH_H_
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#define HASH_MOD 65521
#define S_TABLE_SIZE 512


//output of test-hash
#define LET_H 229
#define PRINT_H 397
#define INPUT_H 400
#define IF_H 143
#define WHILE_H 377
#define WEND_H 302
#define FOR_H 231
#define NEXT_H 319
#define CLS_H 226
#define END_H 215




typedef struct ll_node {
    char name[AST_STR_LEN];
    AST * data;
    unsigned int id;
    struct ll_node* next;
} LL_NODE;
typedef struct s_table{
    LL_NODE** list;
    int ids[S_TABLE_SIZE];   
} hashmap;

void sortAST(AST **arr, int left, int right);
void free_s_table();
LL_NODE * MakeLLnode(char * name,AST * data);
void removeLLnode(LL_NODE * head, char * name);
AST * getLLdata(LL_NODE * head, char* name);
LL_NODE * appendLLnode(LL_NODE * head, char * name, AST * data);
void FreeLLIST_one(LL_NODE * l);
void FreeLLIST_all(LL_NODE * l);
unsigned long hash(char * str);
void test_hashes_on_keywords();
void introduce_s_table();
void add_symbol(char * name, AST * data);

#endif