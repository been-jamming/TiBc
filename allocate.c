#include "allocate.h"

reg_list *create_reg_list(unsigned int num_regs){
	reg_list *output;
	unsigned int i;

	output = malloc(sizeof(reg_list));
	reg_list->num_regs = num_regs;
	reg_list->regs = malloc(sizeof(unsigned int)*num_regs);
	reg_list->positions = malloc(sizeof(unsigned int)*num_regs);

	for(i = 0; i < num_regs; i++){
		reg_list->regs[i] = i;
		reg_list->positions[i] = i;
	}

	reg_list->current_reg = 0;
}

void free_reg_list(reg_list *r){
	free(r->regs);
	free(r);
}

unsigned int use_register(reg_list *r){
	unsigned int output;

	if(r->current_reg >= r->num_regs){
		return 0;
	}

	output = r->regs[r->current_reg];
	r->current_reg++;
	return output;
}

void free_register(reg_list *r, unsigned int reg){
	r->current_reg--;
	r->positions[r->regs[r->current_reg]] = r->positions[reg];
	r->positions[reg] = current_reg;
	r->regs[r->positions[r->regs[r->current_reg]]] = r->regs[current_reg];
	r->regs[current_reg] = reg;
}

unsigned int pop_register(reg_list *r){
	r->current_reg--;
	return r->regs[r->current_reg];
}

