#ifndef HASH_H_
#define HASH_H_
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#define HASH_MOD 65521
#define S_TABLE_SIZE 64


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
    Atom data;
    enum {
        type_int,
        type_float,
        type_string,
        type_variable,
        type_pointer_var,
        type_null
    } type;
    unsigned int id;
    struct ll_node* next;
} LL_NODE;
typedef struct s_table{
    LL_NODE** list;
    int inds[S_TABLE_SIZE];
    struct s_table * next;
} hashmap;

extern hashmap * S_TABLE;
extern int min_available_id;

void sortAST(AST **arr, int left, int right);
void free_s_table();
LL_NODE * MakeLLnode(char * name,AST * data);
void removeLLnode(LL_NODE * head, char * name);
AST * getLLdata(LL_NODE * head, char* name);
LL_NODE * appendLLnode(LL_NODE * head, char * name, AST * data);
void FreeLLIST_all(LL_NODE ** p);
hashmap * create_table();
int getId(char *str, hashmap *table);

/*
    adler-32
*/
unsigned long hash(char * str);
void test_hashes_on_keywords();
int getIndexByHash(char * str);
int getIndexBySymbol(AST * node);
void introduce_s_table();
void add_symbol(AST * data);
int getAddrByAST(AST * arg, hashmap* table);
int getAddrByID(int sym, hashmap * table);
void insert_hashmap_addr(hashmap * table, int data, int id);
void quicksort(int *arr, int left, int right);

#endif
