#include "allocate.h"
#include "compile.h"
#include <stdlib.h>

reg_list *create_reg_list(unsigned int num_regs){
	reg_list *output;
	unsigned int i;

	output = malloc(sizeof(reg_list));
	output->num_regs = num_regs;
	output->regs = malloc(sizeof(unsigned int)*num_regs);
	output->positions = malloc(sizeof(unsigned int)*num_regs);

	for(i = 0; i < num_regs; i++){
		output->regs[i] = i;
		output->positions[i] = i;
	}

	output->current_reg = 0;
	return output;
}

void free_reg_list(reg_list *r){
	free(r->regs);
	free(r);
}

unsigned int allocate_register(reg_list *r){
	unsigned int output;

	if(r->current_reg >= r->num_regs){
		return 0;
	}

	output = r->regs[r->current_reg] + 1;
	r->current_reg++;
	return output;
}

void free_register(reg_list *r, unsigned int reg){
	if(!reg){
		return;
	} else {
		reg--;
	}
	
	r->current_reg--;
	r->positions[r->regs[r->current_reg]] = r->positions[reg];
	r->positions[reg] = r->current_reg;
	r->regs[r->positions[r->regs[r->current_reg]]] = r->regs[r->current_reg];
	r->regs[r->current_reg] = reg;
}

unsigned int pop_register(reg_list *r){
	r->current_reg--;
	return r->regs[r->current_reg];
}

