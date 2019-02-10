#ifndef INCLUDE_COMPILE
#define INCLUDE_COMPILE
#include "linked_list.h"
#include "dictionary.h"
#include "parse.h"

//Addressing modes
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

//Block structure definition
//
//Contains an ordered list of statements, a reference to the local variables that block can access, the current state of the stack pointer, and the number of arguments for the parent function.
//Functions are defined as a variable with the value of a block.
//All blocks within the same function share the same 'variables' pointer.
//A new dictionary for 'variables' is created when a function is created.
struct block{
	linked_list *statements;
	dictionary *variables;
	unsigned int *local_size;
	unsigned int num_args;
};

typedef struct variable variable;

//Variable structure definition
//
//Contains the type of the variable, as well as its position on the stack if it is a local variable.
//All local variables are located on the stack.
//Global variables and functions are also referenced with variables. For a function, the 'is_function' value is set to 1.
//External data is also imported as a variable. An example is the '__mul' and '__div' subroutines.
//For external data, the 'is_data' value is set to 1.
//When the 'is_data' value is set, the job of translating the data is platform specific and the data is passed onto a single 'DATA' pseudo-assembly instruction
//The value 'referenced' is set to 1 when the variable is used somewhere else in the program.
//The value 'name' contains a copy of the name of the variable in the original program. No name mangling occurs.
//The variable can either point to a block via the 'function' pointer data and its size via the 'data' pointer and 'data_size' integer.
//The value 'reg' contains the register the variable is stored in. It is currently unused since all variables are stored on the stack.
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

//Constant structure definition
//
//Constants currently define only integer literals used within the program. Thus the type for now is always INTEGER.
//Later, other types will refer to string and character constants.
//The value 'offset' is planned to store the address of string constants in the output asm.
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

//Expression structure definition
//
//This structure refers to all of the individual expressions passed as arguments, return values, or used directly in programs.
//The value 'type' contains the type of the expression. Valid types include particular token types, particularly LITERAL, OPERATOR, and UNARY
//The value 'sub_type' contains either the type of operation or the type or literal. The sub_type of expression uses the same macros and values as the sub_type of token.
//The pointer 'expr1' points to a left expression which should be evaluated first before this expression.
//The output of this expression along with the right expression is used to calculate the value of the parent expression.
//If the expression has no child expressions, then 'expr' will be NULL
//Similarly, 'expr2' points to a right expression which should be evaluated first and is NULL when it doesn't exist.
//The value 'do_order' is set to 1 if the expression should reorder itself and its children based on the order of operations. If the child is a part of an expression surrounded by parentheses, this value will be set to 0 to respect the parentheses.
//The value 'reg' is set to the register which the output of the expression will be stored to. If set to 0, then the output of the expression is stored to the top of the stack.
//The value 'to_stack' tells the compiler whether to force that expression's output onto the stack. If set, the expression may use the registers for calculation, but the result will alwasy end up on the stack. This is useful when passing arguments to a function, which are always passed on the stack.
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

//Statement structure definition
//
//The value 'type' is set to either 0 or KEYWORD.
//0 is an invalid token type, but used in this structure to represent a statement which is just an expression to evaluate.
//If 'type' is 0, then executing the statement is the same as evaluating the expression.
//If 'type' is KEYWORD, then the statement is either an IF, WHILE, VAR, or RETURN statement.
//The value 'sub_type' is either IF, WHILE, VAR, or RETURN as defined by token macros. 'sub_type' determines what type of KEYWORD the statement is.
//The pointer 'expr' represents the expression to evaluate in the case of WHILE, IF, and RETURN statements as well as when 'type' is 0.
//The pointer 'code' points to the block which an IF or WHILE statement should execute.
//The pointer 'var_pointer' points to the variable declared by a VAR statement.
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

