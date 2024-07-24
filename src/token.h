#ifndef TOKEN_H_
#define TOKEN_H_

typedef enum wordtype {
    WT_CHAR,
    WT_OPER,
    WT_NUM,
    WT_ETC,
    WT_QUOTES,
    WT_SPACE,
    WT_NEWLINE,
    WT_PARENTHESIS, //left and right are needede
    WT_NULL
} wt;

bool LineToTokens(FILE * in);
void TokensToLinePrint();
void freeTokensArr();
void allocTokensArr();
void get_next_token();
char * cur_token();
char * next_token();
bool isBINEXP(char * str);
bool isSTRING(char * str);
bool isUNARY(char * str);
bool isVAR(char * str);
bool isINT(char * str);
bool isBINEXP(char * str);


#endif