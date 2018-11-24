#include "parse.h"
#include "compile.h"
#include "allocate.h"
#include <stdlib.h>
#include <stdio.h>

dictionary global_namespace;
unsigned const int operator_precedence[] = {0, 127, 3, 3, 2, 2, 127, 127, 127, 127, 5, 4, 4, 4, 4, 2, 6, 6, 6, 6, 6, 6, 6, 1, 5};

static void _empty_callback(void *v){}

static void _free_variable(void *v);

variable *create_variable(unsigned char type, unsigned int offset, char *name){
	variable *output;

	output = malloc(sizeof(variable));
	output->type = type;
	output->offset = offset;
	output->name = name;
	output->is_function = 0;
	
	return output;
}

void free_variable(variable *var){
	free(var->name);
	
	if(var->is_function){
		free(var->function->local_size);
		free_dictionary(*(var->function->variables), _free_variable);
		free(var->function->variables);
		free_block(var->function);
	}
	free(var);
}

constant *create_constant(unsigned char type, unsigned int offset){
	constant *output;
	output = malloc(sizeof(constant));
	output->type = type;
	output->offset = offset;

	return output;
}

void free_constant(constant *c){
	if(!c){
		return;
	}

	if(c->type == STRING){
		free(c->string_value);
	}

	free(c);
}

block *create_block(dictionary *variables, unsigned int *local_size){
	block *output;
	output = malloc(sizeof(block));
	output->statements = create_linked_list((void *) 0);
	output->variables = variables;
	output->local_size = local_size;

	return output;
}

void free_block(block *b){
	linked_list *last;
	
	while(b->statements){
		last = b->statements;
		b->statements = b->statements->next;
		free_statement((statement *) last->value);
		free(last);
	}

	free(b);
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
	output->reg = 0;
	output->to_stack = 0;
	return output;
}

void free_expression(expression *e){
	linked_list *last;

	if(e->type == OPERATOR){
		free_expression(e->expr1);
		free_expression(e->expr2);
	} else if(e->type == UNARY){
		free_expression(e->expr2);
	} else if(e->type == LITERAL){
		free_constant(e->const_pointer);
	} else if(e->type == RUNFUNCTION){
		free_expression(e->expr2);
		while(e->func_arguments){
			last = e->func_arguments;
			e->func_arguments = e->func_arguments->next;
			free_expression((expression *) last->value);
			free(last);
		}
	} else if(e->type == IDENTIFIER){
		//free_variable(e->var_pointer);
	} else {
		printf("unknown expression free type %d\n", (int) e->type);
	}

	free(e);
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

void free_statement(statement *s){
	if(!s){
		return;
	}

	if(!s->type){
		free_expression(s->expr);
	} else if(s->type == KEYWORD && (s->sub_type == IF || s->sub_type == WHILE)){
		free_expression(s->expr);
		free_block(s->code);
	} else if(s->type == KEYWORD && s->sub_type == RETURN){
		free_expression(s->expr);
	}

	free(s);
}

static void _free_variable(void *v){
	free_variable((variable *) v);
}

void free_space(dictionary global_space){
	iterate_dictionary(global_space, _free_variable);
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
	expression *unary;
	linked_list *arguments;
	linked_list *new_argument;

	root = create_expression(0, 0);
	current_expression = root;

	while(((*token_list)->type != CONTROL || (*token_list)->sub_type == OPENPARENTHESES) && *token_length != 0){
		if((*token_list)->type == IDENTIFIER){
			child = current_expression;
			while(child->expr2){
				child = child->expr2;
			}
			child->expr2 = variable_expression(global_space, local_space, (*token_list)->string_value);
			free((*token_list)->string_value);
			child->expr2->parent = child;
		} else if((*token_list)->type == LITERAL){
			child = current_expression;
			while(child->expr2){
				child = child->expr2;
			}
			child->expr2 = literal_expression(**token_list, const_list, const_offset);
			child->expr2->parent = child;
		} else if((*token_list)->type == OPERATOR){
			current_expression->parent = create_expression(OPERATOR, (*token_list)->sub_type);
			current_expression->parent->expr1 = current_expression;
			if((*token_list)->sub_type == ELEMENT){
				++*token_list;
				current_expression->parent->expr2 = compile_expression(global_space, local_space, token_list, token_length, const_list, const_offset);
				current_expression->parent->expr2->do_order = 0;
			}
			current_expression = current_expression->parent;
		} else if((*token_list)->type == UNARY){
			child = current_expression;
			while(child->expr2){
				child = child->expr2;
			}

			unary = create_expression(UNARY, (*token_list)->sub_type);
			child->expr2 = unary;
			unary->parent = child;
		} else if((*token_list)->type == CONTROL && (*token_list)->sub_type == OPENPARENTHESES){
			child = current_expression;
			while(child->expr2){
				if(child->expr2->type != UNARY){
					break;
				}
				child = child->expr2;
			}

			++*token_list;
			--*token_length;
			if(!child->expr2){
				child->expr2 = compile_expression(global_space, local_space, token_list, token_length, const_list, const_offset);
				child->expr2->do_order = 0;
			} else {
				/*spooky*/
				child = create_expression(RUNFUNCTION, 0);
				child->to_stack = 1;
				current_expression->expr2->parent = child;
				child->expr2 = current_expression->expr2;
				current_expression->expr2 = child;
				child->parent = current_expression;

				if((*token_list)->type == CONTROL && (*token_list)->sub_type == CLOSEPARENTHESES){
					arguments = (linked_list *) 0;
				} else {
					arguments = create_linked_list(compile_expression(global_space, local_space, token_list, token_length, const_list, const_offset));
					new_argument = arguments;
					while((*token_list)->type != CONTROL || (*token_list)->sub_type != CLOSEPARENTHESES){
						++*token_list;
						--*token_length;
						add_linked_list(&new_argument, create_linked_list(compile_expression(global_space, local_space, token_list, token_length, const_list, const_offset)));
					}
				}
				child->func_arguments = arguments;
			}
		}

		++*token_list;
		--*token_length;
	}
	if(root->parent){
		root->parent->expr1 = root->expr2;
	} else {
		current_expression = current_expression->expr2;
	}
	free(root);
	order_expression(&current_expression);
	to_stack_expression(current_expression);

	return current_expression;
}

void to_stack_expression(expression *expr){
	if(!expr){
		return;
	}

	if(!expr->do_order){
		return;
	}

	to_stack_expression(expr->expr1);
	to_stack_expression(expr->expr2);

	if(expr->type == RUNFUNCTION){
		expr->to_stack = 1;
	}
	
	if(expr->expr1){
		if(expr->expr1->to_stack){
			expr->to_stack = 1;
		}
	}
	if(expr->expr2){
		if(expr->expr2->to_stack){
			expr->to_stack = 1;
		}
	}
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
		order_expression(&(child->expr2));
	}
}

statement *compile_statement(dictionary *global_space, dictionary *local_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset, unsigned int *local_offset){
	statement *output;
	output = create_statement(0, 0);
	
	if((*token_list)->type == KEYWORD){
		output->type = KEYWORD;
		output->sub_type = (*token_list)->sub_type;
		if((*token_list)->sub_type == VAR){
			++*token_list;
			--*token_length;
			if(read_dictionary(*local_space, (*token_list)->string_value, 0) || read_dictionary(*global_space, (*token_list)->string_value, 0)){
				printf("Redefinition of variable: %s\n", (*token_list)->string_value);
				exit(1);
			} else {
				output->var_pointer = create_variable(LOCAL, *local_offset, (*token_list)->string_value);
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
		} else if((*token_list)->sub_type == RETURN){
			++*token_list;
			--*token_length;
			output->expr = compile_expression(global_space, local_space, token_list, token_length, const_list, const_offset);
			++*token_list;
			--*token_length;
			return output;
		} else {
			printf("Uknown keyword: %d\n", (int) (*token_list)->sub_type);
			exit(1);
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
	linked_list *original_statements;
	
	output = create_block(local_space, local_offset);
	original_statements = output->statements;
	++*token_list;
	--*token_length;
	
	while((*token_list)->type != CONTROL || (*token_list)->sub_type != CLOSEBRACES){
		add_linked_list(&(output->statements), create_linked_list(compile_statement(global_space, local_space, token_list, token_length, const_list, const_offset, local_offset)));
	}

	output->statements = original_statements;

	return output;
}

void compile_program(dictionary *global_space, token **token_list, unsigned int *token_length, linked_list **const_list, unsigned int *const_offset){
	variable *var_pointer;
	variable *local_var_pointer;
	dictionary *new_local_space;
	unsigned int *local_offset;
	unsigned int num_args;

	while((*token_list)->type != END){
		if((*token_list)->type == KEYWORD && (*token_list)->sub_type == VAR){
			++*token_list;
			--*token_length;
			if((*token_list)->type == IDENTIFIER){
				var_pointer = read_dictionary(*global_space, (*token_list)->string_value, 0);
				if(!var_pointer){
					var_pointer = create_variable(GLOBAL, 0, (*token_list)->string_value);
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
					local_offset = malloc(sizeof(unsigned int));
					num_args = 0;
					*local_offset = 0;
					while((*token_list)->type != CONTROL || (*token_list)->sub_type != CLOSEPARENTHESES){
						num_args++;
						if((*token_list)->type != IDENTIFIER){
							printf("Expected identifier\n");
							exit(1);
						} else {
							local_var_pointer = create_variable(LOCAL, *local_offset, (*token_list)->string_value);
							write_dictionary(new_local_space, (*token_list)->string_value, local_var_pointer, 0);
							++*local_offset;
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
						printf("Expected '{' token\n");
						exit(1);
					} else {
						var_pointer->function = compile_block(global_space, new_local_space, token_list, token_length, const_list, const_offset, local_offset);
						var_pointer->function->num_args = num_args;
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

/*int main(){
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
	while(1){}
	return 0;
}*/
