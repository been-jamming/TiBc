#include "translate.h"
#include <stdlib.h>
#include <stdio.h>

unsigned int instruction_length = 0;
unsigned int instruction_lengths[] = {};
unsigned int var_length;

unsigned int current_address;
linked_list **current_instructions;
dictionary current_global_space;

unsigned int get_instruction_length(unsigned char instruction){
	if(instruction_length){
		return instruction_length;
	} else {
		return instruction_lengths[instruction];
	}
}

instruction *create_instruction(unsigned int address, unsigned char operation){
	instruction *output;
	output = malloc(sizeof(instruction));
	output->address = address;
	output->operation = operation;

	return output;
}

void add_instruction(intruction *inst){
	add_linked_list(current_instructions, create_linked_list(inst));
}

void translate_variable_declaration(variable *var){
	instruction *inst;
	
	if(var->type == GLOBAL){
		inst = create_instruction(current_address, LABEL);
		inst->name = var->name;
		add_instruction(inst);
		current_address += var_length;
	}
}

void translate_function(variable *var){
	instruction *inst;

	inst = create_instruction(current_address, LABEL);
	inst->name = var->name;
	add_instruction(inst);
	
	inst = create_instruction(current_address, SSP);
	inst->address1 = *(var->function->local_size);
	add_instruction(inst);
	current_address += get_instruction_length(SSP);
	
	translate_block(var->function);
}

void translate_variable(void *void_variable){
	variable *var;
	var = (variable *) void_variable;
	if(var->is_function){
		translate_function(var);
	} else {
		translate_variable_declaration(var);
	}
}

void translate_program(linked_list **instructions, dictionary global_space){
	current_address = 0;
	current_instructions = instructions;
	current_global_space = global_space;
	iterate_dictionary(global_space, translate_variable);
}
