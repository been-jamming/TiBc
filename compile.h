#include "linked_list.h"
#include "dictionary.h"

#define LOCAL 0
#define GLOBAL 1

typedef struct block block;

struct block{
	linked_list *statements;
	dictionary *variables;
};

typedef struct variable variable;

struct variable{
	unsigned char type;
	unsigned char is_function;
	unsigned int offset;
	block *function;
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
	union{
		struct{
			expression *expr1;
			expression *expr2;
			unsigned char do_order;
		};
		struct{
			variable *arguments;
			unsigned int num_arguments;
		};
		struct{
			expression *condition;
			block *code_block;
		};
		variable *var_pointer;
		constant *const_pointer;
	};
	expression *parent;
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

variable *create_variable(unsigned char type, unsigned int offset);

block *create_block(dictionary *variables);

expression *create_expression(unsigned char type, unsigned char sub_type);

expression *variable_expression(dictionary *global_space, dictionary *local_space, char *var_string);

expression *literal_expression(token t, linked_list **const_list, unsigned int *const_offset);

void order_expression(expression **expr);

expression *compile_expression(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset);

statement *compile_statement(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset, unsigned int *local_offset);

block *compile_block(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset, unsigned int *local_offset);

void compile_program(dictionary *global_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset);
