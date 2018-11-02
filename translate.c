#include "translate.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

unsigned int instruction_length = 0;
unsigned int instruction_lengths[] = {};
unsigned int var_length;

char var_temp[256];

unsigned int current_address;
dictionary current_global_space;

instruction **global_instructions;

unsigned int if_id = 0;
unsigned int while_id = 0;
unsigned int function_call_id = 0;

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

void translate_dereference(expression *expr, instruction **instructions, unsigned int *local_offset){
	instruction *operation;
	
	if(expr->type != UNARY){
		translate_expression(expr, instructions, local_offset);
	} else if(expr->sub_type == DEREFERENCE){
		translate_dereference(expr->expr2, instructions, local_offset);
		operation = create_instruction(DEREFSTACK);
		add_instruction(instructions, operation);
	}
}

void translate_expression(expression *expr, instruction **instructions, unsigned int *local_offset){
	instruction *load_a;
	instruction *operation;
	instruction *jmp_instruction;
	linked_list *argument;
	unsigned int call_id;
	char *new_label;
	unsigned int label_length;

	if(expr->type == LITERAL){
		load_a = create_instruction(PUSH);
		load_a->type1 = LITERAL;
		load_a->const_pointer = expr->const_pointer;
		add_instruction(instructions, load_a);
		++*local_offset;
	} else if(expr->type == IDENTIFIER){
		load_a = create_instruction(PUSH);
		load_a->type1 = expr->var_pointer->type;
		if(load_a->type1 == LOCAL){
			load_a->address1 = expr->var_pointer->offset + *local_offset;
		} else if(load_a->type1 == GLOBAL){
			load_a->name = expr->var_pointer->name;
		}
		add_instruction(instructions, load_a);
		++*local_offset;
	} else if(expr->type == OPERATOR){
		if(expr->sub_type != ASSIGN){
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
		} else {
			if(expr->expr1->type == IDENTIFIER){
				if(expr->expr1->var_pointer->type == LOCAL){
					translate_expression(expr->expr2, instructions, local_offset);
					operation = create_instruction(MOV);
					operation->type1 = LOCAL;
					operation->address1 = 0;
					operation->type2 = LOCAL;
					operation->address2 = expr->expr1->var_pointer->offset + *local_offset;
					add_instruction(instructions, operation);
				} else if(expr->expr1->var_pointer->type == GLOBAL){
					translate_expression(expr->expr2, instructions, local_offset);
					operation = create_instruction(MOV);
					operation->type1 = LOCAL;
					operation->address1 = 0;
					operation->type2 = GLOBAL;
					operation->name2 = expr->expr1->var_pointer->name;
				}
			} else if(expr->expr1->type == UNARY){
				if(expr->expr1->sub_type == REFERENCE){
					printf("Cannot assign the address of variable '%s'\n", expr->expr1->expr2->var_pointer->name);
					exit(1);
				}
				translate_expression(expr->expr2, instructions, local_offset);
				translate_dereference(expr->expr1->expr2, instructions, local_offset);
				operation = create_instruction(MOV);
				operation->type1 = LOCAL;
				operation->address1 = 1;
				operation->type2 = LOCALINDIRECT;
				operation->address2 = 0;
				add_instruction(instructions, operation);

				operation = create_instruction(SSP);
				operation->type1 = LITERAL;
				operation->const_pointer = create_constant(INTEGER, 0);
				operation->const_pointer->int_value = -1;
				add_instruction(instructions, operation);
			}
		}
	} else if(expr->type == UNARY && expr->sub_type == DEREFERENCE){
		translate_dereference(expr, instructions, local_offset);
	} else if(expr->type == RUNFUNCTION){
		call_id = function_call_id;
		function_call_id++;
		operation = create_instruction(SSP);
		operation->type1 = LITERAL;
		operation->const_pointer = create_constant(INTEGER, 0);
		operation->const_pointer->int_value = 1;
		add_instruction(instructions, operation);
		++*local_offset;
		
		operation = create_instruction(PUSH);
		operation->type1 = GLOBAL;
		
		sprintf(var_temp, "__function_call%d", call_id);
		label_length = strlen(var_temp);
		new_label = malloc(sizeof(char)*(label_length + 1));
		strcpy(new_label, var_temp);
		operation->name = new_label;

		add_instruction(instructions, operation);
		argument = expr->func_arguments;
		while(argument){
			translate_expression((expression *) argument->value, instructions, local_offset);
			argument = argument->next;
		}
		translate_expression(expr->expr2, instructions, local_offset);
		jmp_instruction = create_instruction(JMPSTACK);
		add_instruction(instructions, jmp_instruction);

		operation = create_instruction(LABEL);
		operation->name = new_label;
		add_instruction(instructions, operation);
	} else {
		printf("unrecognized expression %d\n", (int) expr->type);
	}
}

void translate_statement(statement *s, block *func, instruction **instructions, unsigned int *local_offset){
	instruction *ssp;
	instruction *operation;
	instruction *branch;
	unsigned int id;
	unsigned int label_length;
	char *new_label;

	if(!s->type){
		translate_expression(s->expr, instructions, local_offset);
		ssp = create_instruction(SSP);
		ssp->type1 = LITERAL;
		ssp->const_pointer = create_constant(INTEGER, 0);
		ssp->const_pointer->int_value = -1;
		add_instruction(instructions, ssp);
		--*local_offset;
	} else if(s->type == KEYWORD && s->sub_type == RETURN){
		translate_expression(s->expr, instructions, local_offset);
		operation = create_instruction(POP);
		operation->type1 = LOCAL;
		operation->address1 = *(func->local_size) + 2;
		add_instruction(instructions, operation);

		operation = create_instruction(SSP);
		operation->type1 = LITERAL;
		operation->const_pointer = create_constant(INTEGER, 0);
		operation->const_pointer->int_value = -*(func->local_size);
		add_instruction(instructions, operation);

		operation = create_instruction(JMPSTACK);
		add_instruction(instructions, operation);
	} else if(s->type == KEYWORD && s->sub_type == IF){
		translate_expression(s->expr, instructions, local_offset);
		branch = create_instruction(BZSTACK);
		branch->type1 = GLOBAL;

		sprintf(var_temp, "__if%d", if_id);
		if_id++;
		label_length = strlen(var_temp);
		new_label = malloc(sizeof(char)*(label_length + 1));
		strcpy(new_label, var_temp);
		branch->name = new_label;
		add_instruction(instructions, branch);
		--*local_offset;

		translate_block(s->code, func, instructions, local_offset);
		operation = create_instruction(LABEL);
		operation->name = new_label;
		branch->next2 = operation;
		add_instruction(instructions, operation);
	}
}

void translate_block(block *b, block *func, instruction **instructions, unsigned int *local_offset){
	linked_list *statements;
	statements = b->statements;
	while(statements->next){
		statements = statements->next;
		translate_statement((statement *) statements->value, func, instructions, local_offset);
	}
}

void translate_function(variable *var, instruction **instructions){
	unsigned int local_offset;
	instruction *ssp;
	instruction *jmp_return;

	if(var->is_function){
		local_offset = 0;
		ssp = create_instruction(SSP);
		ssp->type1 = LITERAL;
		ssp->const_pointer = create_constant(INTEGER, 0);
		ssp->const_pointer->int_value = *(var->function->local_size) - var->function->num_args;
		add_instruction(instructions, ssp);
		translate_block(var->function, var->function, instructions, &local_offset);
		ssp = create_instruction(SSP);
		ssp->type1 = LITERAL;
		ssp->const_pointer = create_constant(INTEGER, 0);
		ssp->const_pointer->int_value = -*(var->function->local_size);
		add_instruction(instructions, ssp);

		jmp_return = create_instruction(JMPSTACK);
		add_instruction(instructions, jmp_return);
	}
}

void _translate_program(void *void_var){
	instruction *operation;
	variable *var;
	var = (variable *) void_var;
	
	operation = create_instruction(LABEL);
	operation->name = var->name;
	add_instruction(global_instructions, operation);
	if(var->is_function){
		translate_function(var, global_instructions);
	} else {
		operation = create_instruction(CONSTANT);
		operation->const_pointer = create_constant(INTEGER, 0);
		operation->const_pointer->int_value = 0;
		add_instruction(global_instructions, operation);
	}
}

void translate_program(dictionary global_space, instruction **instructions){
	global_instructions = instructions;
	iterate_dictionary(global_space, _translate_program);
}

void print_instructions(instruction *instructions){
	while(instructions->next1){
		printf(">");
		instructions = instructions->next1;
		if(instructions->opcode == PUSH){
			printf("PUSH ");
			if(instructions->type1 == LITERAL){
				if(instructions->const_pointer->type == INTEGER){
					printf("%d", instructions->const_pointer->int_value);
				}
			} else if(instructions->type1 == LOCAL){
				printf("from stack %d", instructions->address1);
			} else if(instructions->type1 == GLOBAL){
				printf("label: %s", instructions->name);
			}
		} else if(instructions->opcode == POP){
			printf("POP ");
			if(instructions->type1 == GLOBAL){
				printf("to label: %s", instructions->name);
			} else if(instructions->type1 == LOCAL){
				printf("to stack %d", instructions->address1);
			} else if(instructions->type1 == LOCALINDIRECT){
				printf("to address on stack %d", instructions->address1);
			}
		} else if(instructions->opcode == MOV){
			printf("MOV ");
			if(instructions->type1 == LOCAL){
				printf("from stack %d ", instructions->address1);
			} else if(instructions->type1 == GLOBAL){
				printf("from label %s ", instructions->name);
			}
			if(instructions->type2 == LOCAL){
				printf("to stack %d", instructions->address2);
			} else if(instructions->type2 == GLOBAL){
				printf("to label %s", instructions->name2);
			} else if(instructions->type2 == LOCALINDIRECT){
				printf("to address on stack %d", instructions->address2);
			}
		} else if(instructions->opcode == BZSTACK){
			printf("BZSTACK ");
			if(instructions->type1 == GLOBAL){
				printf("to label %s", instructions->name);
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
		} else if(instructions->opcode == JMPSTACK){
			printf("JMPSTACK");
		} else if(instructions->opcode == DEREFSTACK){
			printf("DEREFSTACK");
		} else if(instructions->opcode == LABEL){
			printf("\n>%s:", instructions->name);
		} else if(instructions->opcode == CONSTANT){
			printf("CONSTANT %d", instructions->const_pointer->int_value);
		}
		printf("\n");
	}
}

int main(){
	char *test_program = "var factorial(n){if(n){return n*factorial(-1 + n);} return 1;}";
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
	translate_program(global_space, &instructions);
	print_instructions(original_instructions);
	return 0;
}
