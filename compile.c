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

constant *create_constant(unsigned char type, unsigned int offset){
	constant *output;
	output = malloc(sizeof(constant));
	output->type = type;
	output->offset = offset;

	return output;
}

block *create_block(dictionary *variables){
	block *output;
	output = malloc(sizeof(block));
	output->statements = create_linked_list((void *) 0);
	output->variables = variables;

	return output;
}

expression *create_expression(unsigned char type, unsigned char sub_type){
	expression *output;
	output = malloc(sizeof(expression));
	output->type = type;
	output->sub_type = sub_type;
	output->expr1 = (expression *) 0;
	output->expr2 = (expression *) 0;
	output->do_order = 1;
	output->parent = (expression *) 0;
	return output;
}

statement *create_statement(unsigned char type, unsigned char sub_type){
	statement *output;
	output = malloc(sizeof(statement));
	output->type = type;
	output->sub_type = sub_type;
	output->expr = (expression *) 0;
	output->code = (block *) 0;
	
	return output;
}

void add_constant(linked_list **list, constant *c){
	add_linked_list(list, create_linked_list(c));
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

expression *literal_expression(token t, linked_list **list, unsigned int *constant_offset){
	expression *output;
	constant *c;

	output = create_expression(LITERAL, t.sub_type);
	c = create_constant(t.sub_type, *constant_offset);
	output->const_pointer = c;
	++*constant_offset;

	add_constant(list, c);
	
	if(t.sub_type == INTEGER){
		output->const_pointer->int_value = t.int_value;
	} else if(t.sub_type == CHARACTER){
		output->const_pointer->char_value = t.char_value;
	} else if(t.sub_type == STRING){
		output->const_pointer->string_value = t.string_value;
	}

	return output;
}

expression *compile_expression(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset){
	expression *root;
	expression *current_expression;
	expression *child;

	root = create_expression(0, 0);
	current_expression = root;

	while(((*token_list)->type != CONTROL || (*token_list)->sub_type == OPENPARENTHESES) && *token_length != 0){
		if((*token_list)->type == IDENTIFIER){
			current_expression->expr2 = variable_expression(global_space, local_space, (*token_list)->string_value);
			current_expression->expr2->parent = current_expression;
		} else if((*token_list)->type == LITERAL){
			current_expression->expr2 = literal_expression(**token_list, const_list, const_offset);
			current_expression->expr2->parent = current_expression;
		} else if((*token_list)->type == OPERATOR){
			current_expression->parent = create_expression(OPERATOR, (*token_list)->sub_type);
			current_expression->parent->expr1 = current_expression;
			current_expression = current_expression->parent;
		} else if((*token_list)->type == CONTROL && (*token_list)->sub_type == OPENPARENTHESES && !current_expression->expr2){
			++*token_list;
			current_expression->expr2 = compile_expression(global_space, local_space, token_list, token_length, const_list, const_offset);
			current_expression->expr2->do_order = 0;
		}
		
		++*token_list;
		--*token_length;
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
	if((*expr)->expr1->type != OPERATOR || !(*expr)->do_order || !(*expr)->expr1->do_order){
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

statement *compile_statement(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset, unsigned int *local_offset){
	statement *output;
	output = create_statement(0, 0);
	
	if((*token_list)->type == KEYWORD){
		output->type = (*token_list)->type;
		output->sub_type = (*token_list)->sub_type;
		if((*token_list)->sub_type == VAR){
			++*token_list;
			--*token_length;
			if(read_dictionary(*local_space, (*token_list)->string_value, 0) || read_dictionary(*global_space, (*token_list)->string_value, 0)){
				printf("Redefinition of variable: %s\n", (*token_list)->string_value);
				exit(1);
			} else {
				output->var_pointer = create_variable(LOCAL, *local_offset);
				++*local_offset;
				write_dictionary(local_space, (*token_list)->string_value, output->var_pointer, 0);
				++*token_list;
				--*token_length;
				if((*token_list)->type != CONTROL || (*token_list)->sub_type != SEMICOLON){
					printf("Expected ';' token\n");
					exit(1);
				}
				++*token_list;
				--*token_length;
				return output;
			}
		} else if((*token_list)->sub_type == IF || (*token_list)->sub_type == WHILE){
			++*token_list;
			--*token_length;
			if((*token_list)->type != CONTROL || (*token_list)->sub_type != OPENPARENTHESES){
				printf("Expected '(' token\n");
				exit(1);
			} else {
				++*token_list;
				--*token_length;
				output->expr = compile_expression(global_space, local_space, token_list, token_length, const_list, const_offset);
				++*token_list;
				--*token_length;
				if((*token_list)->type != CONTROL || (*token_list)->sub_type != OPENBRACES){
					printf("Expected '{' token\n");
					exit(1);
				} else {
					++*token_list;
					--*token_length;
					output->code = compile_block(global_space, local_space, token_list, token_length, const_list, const_offset, local_offset);
					++*token_list;
					--*token_length;
					return output;
				}
			}
		} else if((*token_list)->sub_type == ELSE){
			++*token_list;
			--*token_length;
			if((*token_list)->type != CONTROL || (*token_list)->sub_type != OPENBRACES){
				printf("Expected '{' token\n");
				exit(1);
			} else {
				++*token_list;
				--*token_length;
				output->code = compile_block(global_space, local_space, token_list, token_length, const_list, const_offset, local_offset);
				++*token_list;
				--*token_length;
				return output;
			}
		} else {
			printf("Uknown keyword: %d\n", (int) (*token_list)->sub_type);
		}
	} else {
		output->expr = compile_expression(global_space, local_space, token_list, token_length, const_list, const_offset);
		++*token_list;
		--*token_length;
		return output;
	}
}

block *compile_block(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset, unsigned int *local_offset){
	block *output;
	output = create_block(local_space);
	++*token_list;
	--*token_length;
	
	while((*token_list)->type != CONTROL || (*token_list)->sub_type != CLOSEBRACES){
		add_linked_list(&(output->statements), create_linked_list(compile_statement(global_space, local_space, token_list, token_length, const_list, const_offset, local_offset)));
	}

	return output;
}

void compile_program(dictionary *global_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset){
	variable *var_pointer;
	variable *local_var_pointer;
	dictionary *new_local_space;
	unsigned int local_offset;

	while((*token_list)->type != END){
		if((*token_list)->type == KEYWORD && (*token_list)->sub_type == VAR){
			++*token_list;
			--*token_length;
			if((*token_list)->type == IDENTIFIER){
				if(read_dictionary(*global_space, (*token_list)->string_value, 0)){
					var_pointer = read_dictionary(*global_space, (*token_list)->string_value, 0);
				} else {
					var_pointer = create_variable(GLOBAL, 0);
					write_dictionary(global_space, (*token_list)->string_value, var_pointer, 0);
				}
				++*token_list;
				--*token_length;
				if((*token_list)->type == CONTROL && (*token_list)->sub_type == OPENPARENTHESES){
					var_pointer->is_function = 1;
					new_local_space = malloc(sizeof(dictionary));
					*new_local_space = create_dictionary((void *) 0);
					++*token_list;
					--*token_length;
					local_offset = 0;
					while((*token_list)->type != CONTROL || (*token_list)->sub_type != CLOSEPARENTHESES){
						if((*token_list)->type != IDENTIFIER){
							printf("Expected identifier\n", (int) (*token_list)->type, (int) (*token_list)->sub_type);
							exit(1);
						} else {
							local_var_pointer = create_variable(LOCAL, local_offset);
							write_dictionary(new_local_space, (*token_list)->string_value, local_var_pointer, 0);
							++local_offset;
							++*token_list;
							--*token_length;
							if((*token_list)->type != CONTROL || ((*token_list)->sub_type != COMMA && (*token_list)->sub_type != CLOSEPARENTHESES)){
								printf("Expected ',' token\n");
								exit(1);
							}
						}
						++*token_list;
						--*token_length;
					}
					++*token_list;
					--*token_length;
					if((*token_list)->type != CONTROL || (*token_list)->sub_type != OPENBRACES){
						printf("Expected '{' token\n", (int) (*token_list)->type, (int) (*token_list)->sub_type);
						exit(1);
					} else {
						var_pointer->function = compile_block(global_space, new_local_space, token_list, token_length, const_list, const_offset, &local_offset);
					}
				} else if((*token_list)->type == CONTROL && (*token_list)->sub_type == SEMICOLON){
					var_pointer->is_function = 0;
					var_pointer->offset = *const_offset;
					++*const_offset;
				} else {
					printf("Expected ';' or '(' token\n");
					exit(1);
				}
				++*token_list;
				--*token_length;
			}
		} else {
			printf("Expected keyword 'var'\n");
			exit(1);
		}
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
			printf("%d", expr->const_pointer->int_value);
		}
	} else if(expr->type == IDENTIFIER){
		printf("var{%d}", expr->var_pointer->offset);
	} else {
		printf("Unrecognized type: %d ", (int) expr->type);
	}
}

int main(){
	char *test_program = "var ben; var mi(){var printf; printf + 1;}";
	char **test_program_pointer;
	token *token_list;
	token **token_list_pointer;
	linked_list *const_list;
	unsigned int token_length;
	unsigned int token_index;
	unsigned int const_offset = 0;
	unsigned int local_offset;
	
	const_list = create_linked_list((void *) 0);

	block *hi;

	token_list = calloc(10, sizeof(token));
	token_list_pointer = &token_list;
	token_length = 10;
	token_index = 0;
	local_offset = 0;

	test_program_pointer = &test_program;
	
	parse_program(test_program_pointer, token_list_pointer, &token_index, &token_length);

	expression *expr;
	dictionary global_space;
	dictionary local_space;
	variable *var_pointer;

	local_space = create_dictionary((void *) 0);
	global_space = create_dictionary((void *) 0);

	var_pointer = malloc(sizeof(variable));
	var_pointer->type = 0;
	var_pointer->offset = 221;

	write_dictionary(&local_space, "ben", var_pointer, 0);

	compile_program(&global_space, token_list_pointer, &token_length, &const_list, &const_offset);
	printf("hello world!\n");
	return 0;
}