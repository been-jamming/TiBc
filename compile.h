#ifndef INCLUDE_COMPILE
#define INCLUDE_COMPILE
#include "linked_list.h"
#include "dictionary.h"
#include "parse.h"

#define LOCAL 8
#define GLOBAL 9
#define LOCALINDIRECT 10
#define GLOBALINDIRECT 11
#define REGISTER 12
#define REGISTERINDIRECT 13
#define STACKRELATIVE 14

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
	unsigned int offset;
	char *name;
	block *function;
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

variable *create_variable(unsigned char type, unsigned int offset, char *name);

block *create_block(dictionary *variables, unsigned int *local_size);

expression *create_expression(unsigned char type, unsigned char sub_type);

expression *variable_expression(dictionary *global_space, dictionary *local_space, char *var_string);

expression *literal_expression(token t, linked_list **const_list, unsigned int *const_offset);

void to_stack_expression(expression *expr);

void order_expression(expression **expr);

expression *compile_expression(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset);

statement *compile_statement(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset, unsigned int *local_offset);

block *compile_block(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset, unsigned int *local_offset);

void compile_program(dictionary *global_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset);
#endif

