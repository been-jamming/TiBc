#include "parse.h"
#include "compile.h"
#include <stdlib.h>

dictionary global_namespace;
unsigned const int operator_precedence = {3, 2, 2, 1, 1};

variable *create_variable(unsigned char type, unsigned int offset){
	variable *output;

	output = malloc(sizeof(variable));
	output->type = type;
	output->offset = offset;
	
	return output;
}

block *create_block(){
	block *output;
	output = malloc(sizeof(block));
	output->expressions = create_linked_list((void *) 0);
	output->variables = create_dictionary();

	return output;
}

expression *create_expression(unsigned char type, unsigned char sub_type){
	expression *output;
	output = malloc(sizeof(expression));
	output->type = type;
	output->sub_type = sub_type;
	return output;
}

expression *compile_expression(token **token_list, unsigned int token_length){
	expression *output;
	expression *current_expression;

	output = create_expression(0, 0);

	while((*token_list)->type != CONTROL || (*token_list)->sub_type != SEMICOLON || (*token_list)->sub_type != CLOSEPARENTHESES){
		
