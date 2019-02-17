#include <stdlib.h>
#include "main.h"
#include "translate.h"
#include "optimize2.h"

reg_values *create_reg_values(unsigned int num_registers){
	reg_values *output;

	output = malloc(sizeof(reg_values));
	output->num_registers = num_registers;
	output->values = calloc(num_registers, sizeof(reg_value));

	return output;
}

//Determine if a non-branching instruction is useless
unsigned char do_delete_instruction(instruction i){
	if(i.opcode == SSP && i.const_pointer->int_value == 0){
		return 1;
	}

	return 0;
}

void sweep_instructions(instruction **instructions){
	instruction *current_instruction;
	instruction *previous_instruction;

	previous_instruction = *instructions;
	current_instruction = previous_instruction->next1;
	while(current_instruction){
		if(do_delete_instruction(*current_instruction)){
			previous_instruction->next1 = current_instruction->next1;
			free_instruction(current_instruction);
			current_instruction = previous_instruction->next1;
		} else {
			previous_instruction = current_instruction;
			current_instruction = current_instruction->next1;
		}
	}
}

