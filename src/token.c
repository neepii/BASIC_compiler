#include "basicc.h"
#include "token.h"

#define T_EXIT_SUCCESS 1
#define T_EXIT_EOF 0
#define T_EXIT_BLANK 2
#define GRAPH_UNIT_SIZE 50
#define DEFAULT_STACK_SIZE 50

char** tokens = NULL;

unsigned int tokInd =0;
unsigned int tokLen = 0;
bool *marked;

unsigned int markedLen;

static void clear_nfa(NFA *nfa);
static NFA *createNFA(char *re);
static void free_graph(GRAPH *g);
static void print_graph(GRAPH *g);
static bool recognizes_nfa(NFA *nfa, char *str);

NFA *nfaINTEGER;
NFA *nfaSTRING;
NFA *nfaCOMMON_FLOAT;
NFA *nfaDOT_FLOAT;
NFA *nfaSCIENTIFIC_FLOAT;
NFA *nfaVARIABLE;


static char *str_bool(bool b) {
    static char * bools[2] = {"True", "False"};
    return (b) ? bools[0] : bools[1];
}
void test_regex(char * str) {
    printf("Argument is integer: %s\n", str_bool(recognizes_nfa(nfaINTEGER, str)));
    printf("Argument is string: %s\n", str_bool(recognizes_nfa(nfaSTRING, str)));
    printf("Argument is float: %s\n", str_bool(recognizes_nfa(nfaCOMMON_FLOAT, str)));
    printf("Argument is float but with only dot: %s\n", str_bool(recognizes_nfa(nfaDOT_FLOAT, str)));
    printf("Argument is float with scientific notation: %s\n", str_bool(recognizes_nfa(nfaSCIENTIFIC_FLOAT, str)));
    printf("Argument is variable: %s\n", str_bool(recognizes_nfa(nfaVARIABLE, str)));
}

void init_nfa() {
    nfaINTEGER = createNFA("(+|-)?[0-9][0-9]*");
    nfaSTRING = createNFA("\"(.)*\"");
    nfaCOMMON_FLOAT = createNFA("(+|-)?[0-9][0-9]*.[0-9][0-9]*");
    nfaDOT_FLOAT = createNFA("(+|-)?.[0-9][0-9]*");
    nfaSCIENTIFIC_FLOAT = createNFA("(+|-)?[0-9][0-9]*.[0-9][0-9]*(e|E)(+|-)?[0-9][0-9]*");
    nfaVARIABLE = createNFA("([A-Z]|[a-z])([A-Z]|[a-z]|[0-9])*");
}

void free_nfa() {
    clear_nfa(nfaINTEGER);
    clear_nfa(nfaSTRING);
    clear_nfa(nfaCOMMON_FLOAT);
    clear_nfa(nfaDOT_FLOAT);
    clear_nfa(nfaSCIENTIFIC_FLOAT);
    clear_nfa(nfaVARIABLE);
}

STACK *init_stack(unsigned int len) {
    STACK * stack;
    int arrlen = (len == 0) ? DEFAULT_STACK_SIZE : len;
    stack = (STACK*) malloc(sizeof(STACK) + sizeof(int) * (arrlen - 1));
    stack->last = 0;
    stack->arrlen = arrlen;
    assert(stack->arrlen);
    return stack;
}
STACK_STR *init_stack_str(unsigned int len) {
    STACK_STR * stack;
    int arrlen = (len == 0) ? DEFAULT_STACK_SIZE : len;
    stack = (STACK_STR*) malloc(sizeof(STACK) + 20 * sizeof(char) * (arrlen - 1));
    stack->last = 0;
    stack->arrlen = arrlen;
    assert(stack->arrlen);
    return stack;
}

void clear_stack(STACK *s) { s->last = 0; }
int top_stack(STACK *s) { return s->arr[s->last - 1]; }
bool stack_is_empty(STACK *s) { return s->last == 0; }
bool stack_str_is_empty(STACK_STR *s) { return s->last == 0; }

void push_str_s(STACK_STR *st, char * str) {
    if (st->last > st->arrlen) {
        st->arrlen *= 2;
        st = realloc(st, sizeof(STACK) + 20 * sizeof(char) * (st->arrlen - 1));
    }
    strncpy(st->arr[st->last++], str,20);
}
void push_s(STACK *st, int data) {
    if (st->last > st->arrlen) {
        st->arrlen *= 2;
        st = realloc(st, sizeof(STACK) + sizeof(int) * (st->arrlen - 1));
    }
    st->arr[st->last++] = data;
}

void pop_str_s(STACK_STR *st, char addr[20]) {
    if (st->arrlen / st->last == 3) {
        st->arrlen /= 2;
        st = realloc(st, sizeof(STACK) + sizeof(int) * (st->arrlen - 1));
    }
    strncpy(addr, st->arr[st->last--], 20);
}
int pop_s(STACK *st) {
    if (st->arrlen / st->last == 3) {
        st->arrlen /= 2;
        st = realloc(st, sizeof(STACK) + sizeof(int) * (st->arrlen - 1));
    }
    return st->arr[--st->last];
}

static void print_graph(GRAPH *g) {
    for (int i = 0; i < g->v; i++) {
        for (int j = 0; j < g->adj[i]->arrlen; j++) {
            printf("%d ", g->adj[i]->arr[j]);
        }
        printf("\n");
    }
}
static void print_stack(STACK *s) {
    for (int i = 0; i < s->last; i++)
        printf("%d ", s->arr[i]);
    printf("\n");
}


static GRAPH *  init_graph(int num) {
    GRAPH * g = (GRAPH*) malloc(sizeof(GRAPH) + sizeof(struct successors*) * (num -1) );
    g->v = num;
    g->e = 0;
    assert(g);
    for (int i  = 0; i < num; i++) {
        g->adj[i] = (struct successors*) malloc(sizeof(struct successors));
        g->adj[i]->n = 0;
        g->adj[i]->arrlen = 1;
     }
    return g;
}

static void free_graph(GRAPH *g) {
    for (int i  = 0; i < g->v; i++) free(g->adj[i]);
    free(g);
}

static void add_edge_graph(GRAPH *g, int a , int b) {
    assert(a >= 0);
    assert(a < g->v);
    assert(b >= 0);
    assert(b < g->v);
    while(g->adj[a]->n >= g->adj[a]->arrlen) { //cool, it will be dynamic
        g->adj[a]->arrlen *= 2;
        g->adj[a] = realloc(g->adj[a], sizeof(struct successors) + sizeof(int) * (g->adj[a]->arrlen - 1));
    }

    g->adj[a]->arr[g->adj[a]->n++] = b;
    g->e++;
}

static void clear_nfa(NFA *nfa) {
    free_graph(nfa->g);
    free(nfa->re);
    free(nfa);
}

static NFA * createNFA(char * re) {
    NFA * nfa = (NFA*) malloc(sizeof(NFA));
    int M = strlen(re);
    char * regex = (char *) malloc(sizeof(char) * M);
    strncpy(regex, re, M);
    GRAPH * G = init_graph(M+1);
    STACK * ops = init_stack(0);
    STACK * ors = init_stack(0);
    for (int i = 0 ; i < M; i++) {
        int lp = i;
        if (re[i] == '(' || re[i] == '|' || re[i] == '[') {
            push_s(ops, i);
        }
        else if(re[i] == ')') {
            int or = top_stack(ops);
            bool OrPresence = false;
            while (re[or] == '|') {
                push_s(ors, pop_s(ops));     
                or = top_stack(ops);     
                OrPresence = true;
            }
            if (OrPresence) {
                lp = pop_s(ops);
                assert(re[lp] == '(');
                do {
                    or = pop_s(ors);
                    add_edge_graph(G, lp, or +1);
                    add_edge_graph(G, or, i);
                } while (!stack_is_empty(ors));
            }
            
            else lp = or;
        }
        else if (re[i] == ']') {
            lp = pop_s(ops);
            assert(re[lp] == '[');
        }
        if (i < M - 1 && re[i+1] == '*') {
            add_edge_graph(G,lp, i+1);
            add_edge_graph(G,i+1, lp);
        }
        else if (i < M - 1 && re[i+1] == '?') {
            add_edge_graph(G, lp, i+1);
        }
        if (re[i] == '(' || re[i] == '*' || re[i] == ')' || re[i] == ']' || re[i] == '?') {
            add_edge_graph(G, i, i+1);
        }
    }
    free(ops);
    nfa->g = G;
    nfa->M = M;
    nfa->re = regex;
    return nfa;
}

static void DEPTH_FIRST(STACK* eps, GRAPH * g, int v) {
    clear_stack(eps);
    STACK* s = init_stack(5);
    if (marked) {
        for (int i = 0; i < markedLen; i++) marked[i] = false;
        if (g->v > markedLen)
            marked = realloc(marked, sizeof(bool) * g->v);
        markedLen = g->v;
    } else {
        marked = (bool*) malloc(sizeof(bool) * g->v);
        markedLen = g->v;
        for (int i = 0; i < markedLen; i++) marked[i] = false;
    }
    
    push_s(s, v);
    while (s->last) {
        int t = pop_s(s);
        if (!marked[t]) {
            push_s(eps, t);
            marked[t] = true;
        }
        for (int i = 0; i < g->adj[t]->n; i++) {
            int adj_v = g->adj[t]->arr[i];
            if (!marked[adj_v]) {
                push_s(s, adj_v);
            }
        }
    }
    free(s);
}
static bool recognizes_nfa(NFA *nfa, char *str) {
    STACK* dfs = init_stack(0);
    STACK * pc = init_stack(0);
    STACK* aux = init_stack(0);
    DEPTH_FIRST(dfs,nfa->g, 0);
    int len = strlen(str);
    
    for (int v = 0; v < nfa->g->v; v++) 
        if (marked[v]) push_s(pc, v);
    
    for (int i = 0; i < len; i++) { 
        for (int j = 0; j < pc->last; j++) { //
            int v = pc->arr[j];
            if (v < nfa->M) {
                
                if (nfa->re[v] == '[') {
                    assert(nfa->re[v+2] == '-');
                    if (str[i] >= nfa->re[v+1] && str[i] <= nfa->re[v+3]) push_s(aux, v+4); 
                }
                else if (nfa->re[v] == str[i] || nfa->re[v] == '.') push_s(aux, v+1);
                
            }
        }
        
        clear_stack(pc);

        for (int j = 0; j < aux->last; j++) {
            DEPTH_FIRST(dfs,nfa->g, aux->arr[j]);
            for (int v = 0; v < nfa->g->v;v++)
                if (marked[v]) push_s(pc, v);
        }
        clear_stack(aux);
    }
    free(aux);
    free(dfs);
    for (int j = 0; j < pc->last; j++) {
        if (pc->arr[j] == nfa->M) {
            free(pc);
            return true;
        }
    }
    free(pc);
    return false;
}

void freeTokensArr() {
    if (tokens[0] == NULL) return;
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

bool isNUM(char c) {
    return c >= 0x30 && c <= 0x39;
}

static bool isPUNCT(char c) {
    return (c == ',' || c == '?' || c == '!' || c == ';' || c == ':');
}

static bool isLATIN(char c) {
    return (c >=0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A);
}

bool isSTRING(char * str) {
    return recognizes_nfa(nfaSTRING, str);
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
    return recognizes_nfa(nfaVARIABLE, str);
}

bool isINT(char * str) {
    return recognizes_nfa(nfaINTEGER, str);
}
bool isCOMMON_FLOAT(char *str) { return recognizes_nfa(nfaCOMMON_FLOAT, str); }
bool isDOT_FLOAT(char *str) { return recognizes_nfa(nfaDOT_FLOAT, str); }
bool isSCIENTIFIC_FLOAT(char *str) { return recognizes_nfa(nfaSCIENTIFIC_FLOAT, str); }

bool isFLOAT(char *str) {
    return recognizes_nfa(nfaCOMMON_FLOAT, str) ||
           recognizes_nfa(nfaDOT_FLOAT, str) ||
           recognizes_nfa(nfaSCIENTIFIC_FLOAT, str);
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
        ">=",
        "AND",
        "OR"
    };
    int len = sizeof(BinOpers) / sizeof(*BinOpers);
    for (int i = 0; i < len; i++)
    {
        if (strcmp(str, BinOpers[i]) == 0) return true;
    }
    return false;
}


bool match(char * str1, char* str2) {
    return (strcmp(str1, str2) == 0);
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
        if (!inQuotes) cur_char = (char)toupper(cur_char);

        if (isQUOTE(cur_char)) { // what if i could use array instead of if else ladder?
            type = WT_QUOTES;
            inQuotes = !inQuotes;
        }
        else if (inQuotes) {
            type = WT_QUOTES;
        }
        else if (cur_char == ' ') {
            type = WT_SPACE;
        }
        else if (cur_char == '\n') {
            type = WT_NEWLINE;
        }
        else if (isLATIN(cur_char)) {
            type = WT_LATIN;
        }
        else if (isOPER(cur_char)) {
            type = WT_OPER;
        } 
        else if (isNUM(cur_char) || cur_char == '.') {
            type = WT_NUM;
        }
        else if (isPARENTHESIS(cur_char)) {
            type = WT_PARENTHESIS;
        }
        else if (isPUNCT(cur_char)) {
            type = WT_PUNCT;
        }
        else {
            type = WT_ETC;
        }

        if (type != WT_SPACE && type != WT_NEWLINE) isBlank = false;

        if ((last != type && last != WT_NULL) || type == WT_PARENTHESIS) {
            
            if (last != WT_SPACE) {
                strncat(tokens[j], word,i);
                inQuotes = false;
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
