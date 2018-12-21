#ifndef INCLUDE_COMPILE
#define INCLUDE_COMPILE
#include "linked_list.h"
#include "dictionary.h"
#include "parse.h"

#define LOCAL 8
#define GLOBAL 9
#define LOCALLIST 10
#define GLOBALLIST 11
#define LOCALINDIRECT 12
#define GLOBALINDIRECT 13
#define REGISTER 14
#define REGISTERINDIRECT 15
#define STACKRELATIVE 16

typedef struct block block;

struct block{
	linked_list *statements;
	dictionary *variables;
	unsigned int *local_size;
	unsigned int num_args;
};

typedef struct variable variable;

struct variable{
	unsigned char type;
	unsigned char is_function;
	unsigned char is_data;
	unsigned int offset;
	unsigned int size;
	unsigned char referenced;
	char *name;
	union{
		block *function;
		struct{
			unsigned char *data;
			unsigned int data_size;
		};
	};
	unsigned int reg;
};

typedef struct constant constant;

struct constant{
	unsigned char type;
	union{
		int int_value;
		char char_value;
		char *string_value;
	};
	unsigned char offset;
	unsigned int size;
};

typedef struct expression expression;

struct expression{
	unsigned char type;
	unsigned char sub_type;
	expression *expr1;
	expression *expr2;
	unsigned char do_order;
	expression *condition;
	block *code_block;
	variable *var_pointer;
	constant *const_pointer;
	linked_list *func_arguments;
	expression *parent;
	unsigned int reg;
	unsigned char to_stack;
};

typedef struct statement statement;

struct statement{
	unsigned char type;
	unsigned char sub_type;
	union{
		struct{
			expression *expr;
			block *code;
		};
		variable *var_pointer;
	};
};

constant *create_constant(unsigned char type, unsigned int offset);

void free_constant(constant *c);

variable *create_variable(unsigned char type, unsigned int offset, char *name);

void free_variable(variable *var);

block *create_block(dictionary *variables, unsigned int *local_size);

void free_block(block *b);

expression *create_expression(unsigned char type, unsigned char sub_type);

void free_expression(expression *e);

statement *create_statement(unsigned char type, unsigned char sub_type);

void free_statement(statement *s);

void free_space(dictionary global_space);

expression *variable_expression(dictionary *global_space, dictionary *local_space, char *var_string);

expression *literal_expression(token t, linked_list **const_list, unsigned int *const_offset);

void to_stack_expression(expression *expr);

void order_expression(expression **expr);

expression *compile_expression(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset);

statement *compile_statement(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset, unsigned int *local_offset);

block *compile_block(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset, unsigned int *local_offset);

void compile_program(dictionary *global_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset);
#endif

