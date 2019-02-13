#ifndef OPTIMIZE_INCLUDED
#define OPTIMIZE_INCLUDED

unsigned char OPTIMIZE_CONTINUE;

void propogate_constants(expression *expr);

void optimize1_expression(expression *expr);

void optimize1_statement(statement *s);

void optimize1_block(block *b);

void optimize1(dictionary global_space);

#endif

