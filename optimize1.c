#include <stdlib.h>
#include "parse.h"
#include "compile.h"
#include "optimize1.h"

/*
 * Intermediate AST Optimizations
 *
 * for TiBc (Ti B compiler)
 *
 * by Ben Jones
 * 2/12/2019
 */

void propogate_constants(expression *expr){
	long int value_a;
	long int value_b;

	if(expr->type == OPERATOR && (expr->sub_type == ADD || expr->sub_type == SUBTRACT || expr->sub_type == MULTIPLY || expr->sub_type == DIVIDE || expr->sub_type == AND || expr->sub_type == OR || expr->sub_type == XOR || expr->sub_type == ELEMENT)){
		propogate_constants(expr->expr1);
		propogate_constants(expr->expr2);
		if(expr->expr1->type == LITERAL && expr->expr2->type == LITERAL && expr->expr1->sub_type == INTEGER && expr->expr2->sub_type == INTEGER){
			value_a = expr->expr1->const_pointer->int_value;
			value_b = expr->expr2->const_pointer->int_value;
			free_expression(expr->expr1);
			free_expression(expr->expr2);
			expr->type = LITERAL;
			expr->const_pointer = create_constant(INTEGER, 0);
			switch(expr->sub_type){
				case ADD:
					expr->const_pointer->int_value = value_a + value_b;
					OPTIMIZE1_CONTINUE = 1;
					break;
				case SUBTRACT:
					expr->const_pointer->int_value = value_a - value_b;
					OPTIMIZE1_CONTINUE = 1;
					break;
				case MULTIPLY:
					expr->const_pointer->int_value = value_a*value_b;
					OPTIMIZE1_CONTINUE = 1;
					break;
				case DIVIDE:
					expr->const_pointer->int_value = value_a/value_b;
					OPTIMIZE1_CONTINUE = 1;
					break;
				case AND:
					expr->const_pointer->int_value = value_a&value_b;
					OPTIMIZE1_CONTINUE = 1;
					break;
				case OR:
					expr->const_pointer->int_value = value_a|value_b;
					OPTIMIZE1_CONTINUE = 1;
					break;
				case XOR:
					expr->const_pointer->int_value = value_a^value_b;
					OPTIMIZE1_CONTINUE = 1;
					break;
			}
			expr->sub_type = INTEGER;
		}
	}
}

void const_index_expression(expression *expr, block *func){
	unsigned int calculated_index;
	variable *var_pointer;

	if(expr->type == UNARY || (expr->type == OPERATOR && expr->sub_type == ASSIGN)){
		const_index_expression(expr->expr2, func);
	} else if(expr->type == OPERATOR){
		const_index_expression(expr->expr1, func);
		const_index_expression(expr->expr2, func);
	}

	if(expr->type == OPERATOR && expr->sub_type == ELEMENT && expr->expr1->type == IDENTIFIER && expr->expr1->var_pointer->type == LOCALLIST && expr->expr2->type == LITERAL){
		calculated_index = *(func->local_size) - expr->expr2->const_pointer->int_value + 1;
		var_pointer = expr->expr1->var_pointer;
		expr->reg = expr->expr1->reg;
		free_expression(expr->expr1);
		free_expression(expr->expr2);
		expr->type = LOADSTACK;
		expr->const_pointer = create_constant(INTEGER, 0);
		expr->const_pointer->int_value = calculated_index;
		OPTIMIZE1_CONTINUE = 1;
	}
}

void optimize1_expression(expression *expr, block *func){
	propogate_constants(expr);
	const_index_expression(expr, func);
}

void optimize1_statement(statement *s, block *func){
	if(!s->type){
		optimize1_expression(s->expr, func);
	} else if(s->type == KEYWORD){
		if(s->sub_type == IF || s->sub_type == WHILE){
			optimize1_block(s->code, func);
		}
		if(s->sub_type == RETURN || s->sub_type == IF || s->sub_type == WHILE){
			optimize1_expression(s->expr, func);
		}
	}
}

void optimize1_block(block *b, block *func){
	unsigned char output;
	linked_list *statements;

	statements = b->statements;
	while(statements->next){
		statements = statements->next;
		optimize1_statement((statement *) statements->value, func);
	}
}

void _optimize1(void *void_var){
	variable *var;

	var = (variable *) void_var;
	if(var->is_function && !var->is_data){
		optimize1_block(var->function, var->function);
	}
}

void optimize1(dictionary global_space){
	OPTIMIZE1_CONTINUE = 1;
	
	while(OPTIMIZE1_CONTINUE){
		OPTIMIZE1_CONTINUE = 0;
		iterate_dictionary(global_space, _optimize1);
	}
}

