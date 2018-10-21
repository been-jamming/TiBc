#include "linked_list.h"
#include "dictionary.h"

#define BLOCK 7

typedef struct variable variable;

struct variable{
	unsigned char type;
	unsigned int offset;
};

typedef struct block block;

struct block{
	linked_list *expressions;
	dictionary variables;
};

typedef struct expression expression;

struct expression{
	unsigned char type;
	unsigned char sub_type;
	union{
		struct{
			expression *expr1;
			expression *expr2;
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
		unsigned int int_value;
		char char_value;
		char *string_value;
	};
	expression *parent;
};

variable *create_variable(unsigned char type, unsigned int offset);

block *create_block();

expression *create_expression(unsigned char type, unsigned char sub_type);

expression *variable_expression(dictionary *global_space, dictionary *local_space, char *var_string);

expression *literal_expression(token t);

void order_expression(expression **expr);

