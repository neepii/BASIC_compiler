#include "basicc.h"
#include "token.h"

#define T_EXIT_SUCCESS 1
#define T_EXIT_EOF 0
#define T_EXIT_BLANK 2
char** tokens = NULL;

unsigned int tokInd =0;
unsigned int tokLen = 0;

void freeTokensArr() {
    for (int i = 0; i < MAX_TOKENS_IN_LINE; i++)
    {
        free(tokens[i]);
    }
    free(tokens);
}

void allocTokensArr() {
    tokens = (char **) malloc(sizeof(char*) * MAX_TOKENS_IN_LINE);
    tokInd = 0;
    for (int i = 0; i < MAX_TOKENS_IN_LINE; i++)
    {
        tokens[i] = (char*) malloc(sizeof(char) * (TOKEN_LEN + 1)); 
        memset(tokens[i], 0,20);
    }
}

void TokensToLinePrint() {
    int i = 0;
    while(tokens[i][0]) {
        printf("%s\n" , tokens[i++]);
    }
}

static bool isPARENTHESIS(char c) {
    return (c==0x28 || c==0x29);
}

static bool isQUOTE(char c) {
    return c == 0x22;
}

static bool isOPER(char c) {
    return (c >= 0x2A && c <= 0x2D) || c == 0x2F || (c >= 0x3C && c <= 0x3E);
}

static bool isNUM(char c) {
    return c >= 0x30 && c <= 0x39;
}


static bool isCHAR(char c) {
    return (c >=0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A);
}



bool isSTRING(char * str) {
    return (str[0] == '"') || (str[0] == '`');
}

bool isUNARY(char * str) {
    static char * UnaryOpers[] = {
    "+",
    "-",
    };
    int len = sizeof(UnaryOpers) / sizeof(*UnaryOpers);
    for (int i = 0; i < len; i++)
    {
        if (match(str, UnaryOpers[i])) return true;
    }
    return false;
}


bool isVAR(char * str) {
    return isCHAR(str[0]);
}

bool isINT(char * str) {
    return isNUM(str[0]);
}

bool isBINEXP(char * str) {
    static char* BinOpers[] = {
        "+",
        "-",
        "*",
        "/",
        "=",
        "#",
        "<>",
        "><",
        ">",
        "<",
        "<=",
        ">="
    };
    int len = sizeof(BinOpers) / sizeof(*BinOpers);
    for (int i = 0; i < len; i++)
    {
        if (strcmp(str, BinOpers[i]) == 0) return true;
    }
    return false;
}


bool match(char * str1, const char* str2) {
    if (strcmp(str1, str2) == 0) {
        return true;
    }
    return false;
}
char * next_token() {
    return tokens[tokInd+1];
}

char * cur_token() {
    return tokens[tokInd];
}

void get_next_token() {
    if (tokInd < tokLen) {
        tokInd++;
    } else {
        parse_error("index out of bounds");
    }

}


int LineToTokens(FILE * in) {
    if (tokens != NULL) {
        freeTokensArr();    
    }
    allocTokensArr();
    char word[TOKEN_LEN];
    int i = 0;
    int j = 0;
    wt type = WT_NULL;
    wt last = WT_NULL;
    bool inQuotes = false;
    bool isBlank = true;
    char cur_char;
    cur_char = fgetc(in);
    if (feof(in)) {
        return T_EXIT_EOF;
    }

    while(cur_char && type != WT_ETC) {
        cur_char = (char)toupper(cur_char);

        if (cur_char == ' ') {
            type = WT_SPACE;
        }
        else if (cur_char == '\n') {
            type = WT_NEWLINE;
        }
        else if (isCHAR(cur_char)) {
            type = (inQuotes) ? WT_QUOTES : WT_CHAR;
        }
        else if (isOPER(cur_char)) {
            type = WT_OPER;
        } 
        else if (isNUM(cur_char)) {
            type = WT_NUM;
        }
        else if (isQUOTE(cur_char)) {
            type = WT_QUOTES;
            if (!inQuotes) inQuotes = true;
        }
        else if (isPARENTHESIS(cur_char)) {
            type = WT_PARENTHESIS;
        }
        else {
            type = WT_ETC;
        }

        if (type != WT_SPACE && type != WT_NEWLINE) isBlank = false;

        if ((last != type && last != WT_NULL) || type == WT_PARENTHESIS) {
            
            if (last != WT_SPACE) {
                strncat(tokens[j], word,i);
                j++;
            }
            i=0;
        }

        if (type == WT_NEWLINE) break;


        word[i] = cur_char;
        i++;
        last = type;
        cur_char = fgetc(in);
    }
    if (isBlank) return T_EXIT_BLANK;

    tokLen = j;
    return T_EXIT_SUCCESS;
}