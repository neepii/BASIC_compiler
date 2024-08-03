#ifndef GEN_H_
#define GEN_H_

void make_target_src();
void compile(char * output_name);
void include(char * path);
char* handle_arith_exp(AST * node);

#endif