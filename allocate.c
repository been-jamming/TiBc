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

	output = r->regs[r->current_reg];
	r->current_reg++;
	return output;
}

void free_register(reg_list *r, unsigned int reg){
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

void allocate_expression(expression *e, reg_list *regs, dictionary *local_space){
	if(e->type == OPERATOR){
		allocate_expression(e->expr1, regs, local_space);
		allocate_expression(e->expr2, regs, local_space);
		if(e->expr1->reg){
			e->reg = e->expr1->reg;
			free_register(regs, e->expr2->reg);
		} else if(e->expr2->reg){
			e->reg = e->expr2->reg;
			free_register(regs, e->expr1->reg);
		} else {
			e->reg = allocate_register(regs);
		}
	} else if(e->type == UNARY){
		allocate_expression(e->expr2, regs, local_space);
		if(e->expr2->reg){
			e->reg = e->expr2->reg;
		} else {
			e->reg = allocate_register(regs);
		}
	} else if(e->type == IDENTIFIER)
		e->reg = 0;
	} else if(e->type == LITERAL){
		e->reg = 0;
	} else if(e->type == RUNFUNCTION){

	}
}

void allocate_statement(statement *s, reg_list *r, dictionary *local_space){
	while(s){
		if(!s->type){
			allocate_expression(s->expr, reg_list *r, dictionary *local_space);
		}

		s = s->previous;
	}
}
