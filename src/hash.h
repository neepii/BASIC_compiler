#ifndef HASH_H_
#define HASH_H_
#include <string.h>
#include <stdio.h>

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


unsigned long hash(char * str);
void test_hashes_on_keywords();

#endif