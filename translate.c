#include "translate.h"
#include <stdlib.h>
#include <stdio.h>

unsigned int instruction_length = 0;
unsigned int instruction_lengths[] = {};
unsigned int var_length;

unsigned int current_address;
dictionary current_global_space;

instruction *create_instruction(unsigned char opcode){
	instruction *output;
	output = malloc(sizeof(instruction));
	output->opcode = opcode;
	output->next1 = (instruction *) 0;
	output->next2 = (instruction *) 0;
	return output;
}

void add_instruction(instruction **instructions, instruction *i){
	(*instructions)->next1 = i;
	*instructions = i;
}

void translate_expression(expression *expr, instruction **instructions, unsigned int *local_offset){
	instruction *load_a;
	instruction *operation;

	if(expr->type == LITERAL){
		load_a = create_instruction(PUSH);
		load_a->type1 = LITERAL;
		load_a->const_pointer = expr->const_pointer;
		add_instruction(instructions, load_a);
		++*local_offset;
	} else if(expr->type == IDENTIFIER){
		load_a = create_instruction(PUSH);
		load_a->type1 = expr->var_pointer->type;
		load_a->address1 = expr->var_pointer->offset + *local_offset;
		add_instruction(instructions, load_a);
		++*local_offset;
	} else if(expr->type == OPERATOR){
		translate_expression(expr->expr1, instructions, local_offset);
		translate_expression(expr->expr2, instructions, local_offset);
		if(expr->sub_type == ADD){
			operation = create_instruction(ADDSTACK);
			--*local_offset;
		} else if(expr->sub_type == MULTIPLY){
			operation = create_instruction(MULSTACK);
			--*local_offset;
		}
		add_instruction(instructions, operation);
	}
}

void translate_statement(statement *s, instruction **instructions, unsigned int *local_offset){
	instruction *ssp;

	if(!s->type){
		translate_expression(s->expr, instructions, local_offset);
		ssp = create_instruction(SSP);
		ssp->type1 = LITERAL;
		ssp->const_pointer = create_constant(INTEGER, 0);
		ssp->const_pointer->int_value = -1;
		add_instruction(instructions, ssp);
		--*local_offset;
	}
}

void translate_block(block *b, instruction **instructions, unsigned int *local_offset){
	linked_list *statements;
	statements = b->statements;
	while(statements->next){
		statements = statements->next;
		translate_statement((statement *) statements->value, instructions, local_offset);
	}
}

void translate_function(variable *var, instruction **instructions){
	unsigned int *local_offset;
	instruction *ssp;

	if(var->is_function){
		local_offset = malloc(sizeof(unsigned int));
		*local_offset = 0;
		ssp = create_instruction(SSP);
		ssp->type1 = LITERAL;
		ssp->const_pointer = create_constant(INTEGER, 0);
		ssp->const_pointer->int_value = *(var->function->local_size);
		add_instruction(instructions, ssp);
		translate_block(var->function, instructions, local_offset);
		ssp = create_instruction(SSP);
		ssp->type1 = LITERAL;
		ssp->const_pointer = create_constant(INTEGER, 0);
		ssp->const_pointer->int_value = -*(var->function->local_size);
		add_instruction(instructions, ssp);
	}
}

void print_instructions(instruction *instructions){
	while(instructions->next1){
		printf(":");
		instructions = instructions->next1;
		if(instructions->opcode == PUSH){
			printf("PUSH ");
			if(instructions->type1 == LITERAL){
				if(instructions->const_pointer->type == INTEGER){
					printf("%d", instructions->const_pointer->int_value);
				}
			} else if(instructions->type1 == LOCAL){
				printf("from stack %d", instructions->address1);
			}
		} else if(instructions->opcode == SSP){
			printf("SSP ");
			if(instructions->type1 = LITERAL){
				if(instructions->const_pointer->type == INTEGER){
					printf("%d", instructions->const_pointer->int_value);
				}
			} else if(instructions->type1 == LOCAL){
				printf("from stack %d", instructions->address1);
			}
		} else if(instructions->opcode == ADDSTACK){
			printf("ADDSTACK");
		} else if(instructions->opcode == MULSTACK){
			printf("MULSTACK");
		}
		printf("\n");
	}
}

int main(){
	char *test_program = "var main(){var a; var b; 2*a+3*b;a + b;}";
	char **test_program_pointer;
	token *token_list;
	token **token_list_pointer;
	linked_list *const_list;
	unsigned int token_length;
	unsigned int token_index;
	unsigned int const_offset = 0;
	unsigned int local_offset;

	instruction *instructions;
	instruction *original_instructions;
	instructions = create_instruction(0);
	original_instructions = instructions;

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
	translate_function((variable *) read_dictionary(global_space, "main", 0), &instructions);
	print_instructions(original_instructions);
	printf("Done!!!\n");
	return 0;
}
