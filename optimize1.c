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

	if(expr->type == OPERATOR && (expr->sub_type == ADD || expr->sub_type == SUBTRACT || expr->sub_type == MULTIPLY || expr->sub_type == DIVIDE || expr->sub_type == AND || expr->sub_type == OR || expr->sub_type == XOR)){
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
					OPTIMIZE_CONTINUE = 1;
					break;
				case SUBTRACT:
					expr->const_pointer->int_value = value_a - value_b;
					OPTIMIZE_CONTINUE = 1;
					break;
				case MULTIPLY:
					expr->const_pointer->int_value = value_a*value_b;
					OPTIMIZE_CONTINUE = 1;
					break;
				case DIVIDE:
					expr->const_pointer->int_value = value_a/value_b;
					OPTIMIZE_CONTINUE = 1;
					break;
				case AND:
					expr->const_pointer->int_value = value_a&value_b;
					OPTIMIZE_CONTINUE = 1;
					break;
				case OR:
					expr->const_pointer->int_value = value_a|value_b;
					OPTIMIZE_CONTINUE = 1;
					break;
				case XOR:
					expr->const_pointer->int_value = value_a^value_b;
					OPTIMIZE_CONTINUE = 1;
					break;
			}
			expr->sub_type = INTEGER;
		}
	}
}

void optimize1_expression(expression *expr){
	propogate_constants(expr);
}

void optimize1_statement(statement *s){
	if(!s->type){
		optimize1_expression(s->expr);
	} else if(s->type == KEYWORD){
		if(s->sub_type == IF || s->sub_type == WHILE){
			optimize1_block(s->code);
		}
		if(s->sub_type == RETURN || s->sub_type == IF || s->sub_type == WHILE){
			optimize1_expression(s->expr);
		}
	}
}

void optimize1_block(block *b){
	unsigned char output;
	linked_list *statements;

	statements = b->statements;
	while(statements->next){
		statements = statements->next;
		optimize1_statement((statement *) statements->value);
	}
}

void _optimize1(void *void_var){
	variable *var;

	var = (variable *) void_var;
	if(var->is_function && !var->is_data){
		optimize1_block(var->function);
	}
}

void optimize1(dictionary global_space){
	OPTIMIZE_CONTINUE = 1;
	
	while(OPTIMIZE_CONTINUE){
		OPTIMIZE_CONTINUE = 0;
		iterate_dictionary(global_space, _optimize1);
	}
}

