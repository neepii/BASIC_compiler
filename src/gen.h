#ifndef GEN_H_
#define GEN_H_

void make_target_src();
void compile(char * output_name);
void include(char * path);
char * eval_arith_exp(AST *node);
void handle_common_statements(AST *node);

#endif
