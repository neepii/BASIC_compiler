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

void test_regex(char * str) {
    printf("Argument is integer: %s\n", (recognizes_nfa(nfaINTEGER, str)) ? "True" : "False");
    printf("Argument is string: %s\n", (recognizes_nfa(nfaSTRING, str)) ? "True" : "False");
}

void init_nfa() {
    nfaINTEGER = createNFA("(+|-)?[0-9][0-9]*");
    nfaSTRING = createNFA("\"(.)*\"");
}

void free_nfa() {
    clear_nfa(nfaINTEGER);
    clear_nfa(nfaSTRING);
}

static STACK *init_stack(int len) {
    STACK * stack;
    int arrlen = (len == 0) ? DEFAULT_STACK_SIZE : len;
    stack = (STACK*) malloc(sizeof(STACK) + sizeof(int) * (arrlen - 1));
    stack->last = 0;
    stack->arrlen = arrlen;
    assert(stack->arrlen);
    return stack;
}

static void clear_stack(STACK *s) { s->last = 0; }
static int top_stack(STACK *s) { return s->arr[s->last - 1]; }
static bool stack_is_empty(STACK *s) { return s->last == 0; }


static char *stack_to_str(STACK *s) { //pretty printing
    static char str[50];
    int last = 0;
    for (int i = 0; i < s->last; i++) {
        str[last++] = s->arr[i];
        str[last++] = ' ';
    }
    str[last] = 0;
    return str;
}

static void push(STACK *st, int data) {
    if (st->last > st->arrlen) {
        st->arrlen *= 2;
        st = realloc(st, sizeof(STACK) + sizeof(int) * (st->arrlen - 1));
    }
    st->arr[st->last++] = data;
}

static int pop(STACK *st) {
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
    for (int i = 0 ; i < M; i++) {
        int lp = i;
        if (re[i] == '(' || re[i] == '|' || re[i] == '[') {
            push(ops, i);
        }
        else if(re[i] == ')') {
            int or = pop(ops);
            if (re[or] == '|') {
                lp = pop(ops);
                add_edge_graph(G, lp, or+1);
                add_edge_graph(G, or, i);
            }
            else lp = or;
        }
        else if (re[i] == ']') {
            lp = pop(ops);
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

static STACK* DEPTH_FIRST(GRAPH * g, int v) {
    STACK* s = init_stack(0);
    STACK* eps = init_stack(10);
    if (marked) {
        for (int i = 0; i < markedLen; i++) marked[i] = false;
        if (g->v > markedLen) marked = realloc(marked, sizeof(bool) * g->v);
        markedLen = g->v;
    } else {
        marked = (bool*) malloc(sizeof(bool) * g->v);
        markedLen = g->v;
    }
    
    push(s, v);
    while (s->last) {
        int t = pop(s);
        
        if (!marked[t]) {
            push(eps, t);
            marked[t] = true;
        }
        for (int i = 0; i < g->adj[t]->n; i++) {
            int adj_v = g->adj[t]->arr[i];
            if (!marked[adj_v]) {
                push(s, adj_v);
            }
        }
    }
    free(s);
    return eps;
}
static bool recognizes_nfa(NFA *nfa, char *str) {
    STACK* dfs = DEPTH_FIRST(nfa->g, 0);
    STACK * pc = init_stack(10);
    STACK* aux = init_stack(10);
    int len = strlen(str);
    
    for (int v = 0; v < nfa->g->v; v++) 
        if (marked[v]) push(pc, v);
    
    for (int i = 0; i < len; i++) { 
        for (int j = 0; j < pc->last; j++) { //
            int v = pc->arr[j];
            if (v < nfa->M) {
                
                if (nfa->re[v] == '[') {
                    assert(nfa->re[v+2] == '-');
                    if (str[i] >= nfa->re[v+1] && str[i] <= nfa->re[v+3]) push(aux, v+4);
                }
                else if (nfa->re[v] == str[i] || nfa->re[v] == '.') push(aux, v+1);
                
            }
        }
        
        clear_stack(pc);
        clear_stack(dfs);

        for (int j = 0; j < aux->last; j++) {
            dfs = DEPTH_FIRST(nfa->g, aux->arr[j]);
            for (int v = 0; v < nfa->g->v;v++)
                if (marked[v]) push(pc, v);
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

static bool isCHAR(char c) {
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
    return isCHAR(str[0]);
}

bool isINT(char * str) {
    return recognizes_nfa(nfaINTEGER,str);
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

        if (isQUOTE(cur_char)) {
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
        else if (isCHAR(cur_char)) {
            type = WT_CHAR;
        }
        else if (isOPER(cur_char)) {
            type = WT_OPER;
        } 
        else if (isNUM(cur_char)) {
            type = WT_NUM;
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
