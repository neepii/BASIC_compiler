#ifndef PARSE_H_
#define PARSE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void FillTokenArray(FILE * in, char tokens[20][20]);
FILE * OpenFile(const char* arg);
#endif