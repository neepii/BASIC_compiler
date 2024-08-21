#ifndef BASICC_H_
#define BASICC_H_
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>
#include "token.h"
#include "parse.h"
#include "hash.h"
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
#define GOTO_H 313
#define GOSUB_H 384
#define RETURN_H 480
#define DEC_H 204
#define INC_H 218


#define PLUS_H 43
#define MINUS_H 45
#define MUL_H 42
#define DIV_H 47
#define EQUAL_H 61
#define LESS_H 60
#define GREATER_H 62
#define GR_EQ_H 123
#define LS_EQ_H 121
#define NEQUAL_H 122
#define NOT_H 241
#define OR_H 161
#define AND_H 211


#define MAX_TOKENS_IN_LINE 20
#define TOKEN_LEN 20
#define STACKS_SIZE 50

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

extern char ** tokens;
extern unsigned int tokInd;
extern unsigned int tokLen;
extern char * tar_path_name;
extern AST **statements;




#endif
