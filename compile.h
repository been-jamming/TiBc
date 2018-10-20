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
		block *code_block;
	};
};

variable *create_variable(unsigned char type, unsigned int offset);

block *create_block();

expression *create_expression(unsigned char type, unsigned char sub_type);
