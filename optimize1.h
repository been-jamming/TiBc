#ifndef OPTIMIZE_INCLUDED
#define OPTIMIZE_INCLUDED

unsigned char OPTIMIZE1_CONTINUE;

void propogate_constants(expression *expr);

void optimize1_expression(expression *expr, block *func);

void optimize1_statement(statement *s, block *func);

void optimize1_block(block *b, block *func);

void optimize1(dictionary global_space);

#endif

