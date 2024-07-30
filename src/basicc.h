#ifndef BASICC_H_
#define BASICC_H_
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include "parse.h"
#include "hash.h"
#include "token.h"
#include "gen.h"

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
#define REM_H 228

#define MAX_TOKENS_IN_LINE 20
#define TOKEN_LEN 20
extern char ** tokens;
extern int tokInd;
extern int tokLen;
extern char * temp_name;
extern AST** statements;

#endif