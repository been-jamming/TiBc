#include "parse.h"
#include "compile.h"
#include <stdlib.h>
#include <stdio.h>

dictionary global_namespace;
unsigned const int operator_precedence[] = {0, 0, 2, 2, 1, 1, 0};

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
	output->variables = create_dictionary((void *) 0);

	return output;
}

expression *create_expression(unsigned char type, unsigned char sub_type){
	expression *output;
	output = malloc(sizeof(expression));
	output->type = type;
	output->sub_type = sub_type;
	output->expr1 = (expression *) 0;
	output->expr2 = (expression *) 0;
	output->parent = (expression *) 0;
	return output;
}

expression *variable_expression(dictionary *global_space, dictionary *local_space, char *var_string){
	expression *output;
	output = create_expression(IDENTIFIER, 0);
	output->var_pointer = (variable *) read_dictionary(*local_space, var_string, 0);
	if(!output->var_pointer){
		output->var_pointer = (variable *) read_dictionary(*global_space, var_string, 0);
		if(!output->var_pointer){
			printf("Undefined variable: %s\n", var_string);
			exit(1);
		}
	}

	return output;
}

expression *literal_expression(token t){
	expression *output;
	output = create_expression(1, t.sub_type);
	if(t.sub_type == INTEGER){
		output->int_value = t.int_value;
	} else if(t.sub_type == CHARACTER){
		output->char_value = t.char_value;
	} else if(t.sub_type == STRING){
		output->string_value = t.string_value;
	}

	return output;
}

expression *compile_expression(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int token_length){
	expression *root;
	expression *current_expression;
	expression *child;

	root = create_expression(0, 0);
	current_expression = root;

	while(((*token_list)->type != CONTROL || (*token_list)->sub_type != SEMICOLON || (*token_list)->sub_type != CLOSEPARENTHESES) && token_length != 0){
		if((*token_list)->type == IDENTIFIER){
			current_expression->expr2 = variable_expression(global_space, local_space, (*token_list)->string_value);
			current_expression->expr2->parent = current_expression;
		} else if((*token_list)->type == LITERAL){
			current_expression->expr2 = literal_expression(**token_list);
			current_expression->expr2->parent = current_expression;
		} else if((*token_list)->type == OPERATOR){
			current_expression->parent = create_expression(OPERATOR, (*token_list)->sub_type);
			current_expression->parent->expr1 = current_expression;
			current_expression = current_expression->parent;
		}
		++*token_list;
		token_length--;
	}
	
	root->parent->expr1 = root->expr2;
	free(root);

	order_expression(&current_expression);

	return current_expression;
}

void order_expression(expression **expr){
	expression *child;
	expression *parent;

	if(!*expr){
		return;
	}
	if((*expr)->type != OPERATOR){
		return;
	}
	if((*expr)->expr1->type != OPERATOR){
		return;
	}
	order_expression(&((*expr)->expr1));
	if(operator_precedence[(*expr)->sub_type] < operator_precedence[(*expr)->expr1->sub_type]){
		child = (*expr)->expr1;
		parent = *expr;
		*expr = child;
		parent->expr1 = child->expr2;
		child->expr2->parent = parent;
		child->expr2 = parent;
		child->parent = parent->parent;
		parent->parent = child;
	}
}

void print_expression(expression *expr){
	if(expr->type == OPERATOR){
		printf("(");
		print_expression(expr->expr1);
		if(expr->sub_type == SUBTRACT){
			printf("-");
		} else if(expr->sub_type == ADD){
			printf("+");
		} else if(expr->sub_type == MULTIPLY){
			printf("*");
		} else if(expr->sub_type == DIVIDE){
			printf("/");
		} else if(expr->sub_type == ELEMENT){
			printf("[");
		} else {
			printf("Unrecognized operation: %d ", (int) expr->sub_type);
		}
		print_expression(expr->expr2);
		if(expr->sub_type == ELEMENT){
			printf("]");
		}
		printf(")");
	} else if(expr->type == LITERAL){
		if(expr->sub_type == INTEGER){
			printf("%d", expr->int_value);
		}
	} else {
		printf("Unrecognized type: %d ", (int) expr->type);
	}
}

int main(){
	char *test_program = "2*3+4[0]*5;";
	char **test_program_pointer;
	token *token_list;
	token **token_list_pointer;
	unsigned int token_length;
	unsigned int token_index;
	
	token_list = calloc(10, sizeof(token));
	token_list_pointer = &token_list;
	token_length = 10;
	token_index = 0;

	test_program_pointer = &test_program;
	
	parse_expression(test_program_pointer, token_list_pointer, &token_index, &token_length, (token) {.type = CONTROL, .sub_type = SEMICOLON});

	expression *expr;
	expr = compile_expression((dictionary *) 0, (dictionary *) 0, token_list_pointer, token_index);
	print_expression(expr);
	return 0;
}
