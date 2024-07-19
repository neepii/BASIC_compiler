#include "parse.h"


typedef struct ast AST;

struct branches
{
    AST * left;
    AST * right;
};

typedef enum wordtype {
    WT_ASCII,
    WT_OPER,
    WT_NUM,
    WT_ETC,
    WT_QUOTES,
    WT_NULL
} wt;



struct ast
{
    unsigned int tag;
    union
    {
        struct branches assign;

    }oper;
};

int isQUOTE(char c) {
    return c == 0x22;
}

int isOPER(char c) {
    return ((c >= 0x28 && c <= 0x2B) || c == 0x2D || c == 0x2F || (c >= 0x3C && c <= 0x3E)) ? 1 : 0;
}

int isNUM(char c) {
    return (c >= 0x30 && c <= 0x39) ? 1 : 0;
}


int isASCII(char c) {
    return ((c >=0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A)) ? 1 : 0;
}

FILE * OpenFile(const char *arg) {
    FILE * f = fopen(arg, "r");
    return f;
}

void FillTokenArray(FILE * in, char tokens[20][20]) {
    char line[100];
    fgets(line, 100, in);
    char word[20];
    for (int i = 0; i < 20; i++)
    {
        memset(tokens[i], 0,20);
    }
    
    int c = 0;
    int i = 0;
    int j = 0;
    wt type;
    wt last = WT_NULL;

    while (line[c]) {
        if (isASCII(line[c])) {
            type = WT_ASCII;
        }
        else if (isOPER(line[c])) {
            type = WT_OPER;
        } 
        else if (isNUM(line[c])) {
            type = WT_NUM;
        }
        else if (isQUOTE(line[c])) {
            type = WT_QUOTES;
        }
        else {
            type = WT_ETC;
        }

        if (last != type && last != WT_NULL) {
            strncat(tokens[j], word,i);
            i=0;
            j++;
        }
        word[i] = line[c];
        i++;
        c++;
        last = type;
    }
    
}



