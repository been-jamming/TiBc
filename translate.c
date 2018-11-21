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
reg_list *global_regs;

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
	output->name = (char *) 0;
	return output;
}

void free_instruction(instruction *i){
	if(i->opcode == CONSTANT || i->opcode == SSP){
		free_constant(i->const_pointer);
	} else if(i->opcode == LABEL){
		free(i->name);
	}

	free(i);
}

void free_instructions(instruction *i){
	instruction *last;

	while(i){
		last = i;
		i = i->next1;
		free_instruction(last);
	}
}

void add_instruction(instruction **instructions, instruction *i){
	(*instructions)->next1 = i;
	*instructions = i;
}

void translate_expression(expression *expr, block *func, instruction **instructions, unsigned int *local_offset, reg_list *regs, unsigned char to_stack){
	instruction *load_a;
	instruction *operation;
	instruction *jmp_instruction;
	linked_list *argument;
	unsigned int call_id;
	char *new_label;
	unsigned int label_length;
	unsigned int num_args;
	unsigned int reg;

	if(expr->to_stack){
		to_stack = 1;
	}

	if(expr->type == LITERAL){
		if(!to_stack){
			reg = allocate_register(regs);
		} else {
			reg = 0;
		}

		if(reg){
			load_a = create_instruction(MOV);
			load_a->type1 = LITERAL;
			load_a->const_pointer = expr->const_pointer;
			load_a->type2 = REGISTER;
			load_a->address2 = reg;
		} else {
			load_a = create_instruction(PUSH);
			load_a->type1 = LITERAL;
			load_a->const_pointer = expr->const_pointer;
			++*local_offset;
		}

		add_instruction(instructions, load_a);
		expr->reg = reg;
	} else if(expr->type == IDENTIFIER){
		if(!to_stack){
			reg = allocate_register(regs);
		} else {
			reg = 0;
		}

		if(reg){
			load_a = create_instruction(MOV);
		} else {
			load_a = create_instruction(PUSH);
		}

		load_a->type1 = expr->var_pointer->type;
		if(load_a->type1 == LOCAL){
			load_a->address1 = *(func->local_size) - expr->var_pointer->offset + *local_offset - 1;
		} else if(load_a->type1 == GLOBAL){
			load_a->name = expr->var_pointer->name;
			if(!expr->var_pointer->is_function){
				load_a->type1 = GLOBALINDIRECT;
			}
		}

		if(reg){
			load_a->type2 = REGISTER;
			load_a->address2 = reg;
		}
		add_instruction(instructions, load_a);
		if(!reg){
			++*local_offset;
		}
		expr->reg = reg;
	} else if(expr->type == OPERATOR){
		if(expr->sub_type != ASSIGN){
			translate_expression(expr->expr1, func, instructions, local_offset, regs, to_stack);
			translate_expression(expr->expr2, func, instructions, local_offset, regs, 0);
			
			if(expr->sub_type == ADD){
				operation = create_instruction(ADDOP);
			} else if(expr->sub_type == MULTIPLY){
				operation = create_instruction(MULOP);
			} else if(expr->sub_type == SUBTRACT){
				operation = create_instruction(SUBOP);
			} else if(expr->sub_type == DIVIDE){
				operation = create_instruction(DIVOP);
			} else if(expr->sub_type == LESSTHAN){
				operation = create_instruction(LTOP);
			} else if(expr->sub_type == GREATERTHAN){
				operation = create_instruction(GTOP);
			} else if(expr->sub_type == EQUALS){
				operation = create_instruction(EQOP);
			} else if(expr->sub_type == NOTEQUALS){
				operation = create_instruction(NEQOP);
			} else if(expr->sub_type == OR){
				operation = create_instruction(OROP);
			} else if(expr->sub_type == AND){
				operation = create_instruction(ANDOP);
			} else if(expr->sub_type == ELEMENT){
				operation = create_instruction(LSLOP);
				if(!expr->expr2->reg){
					operation->type2 = LOCAL;
					operation->address2 = 0;
				} else {
					operation->type2 = REGISTER;
					operation->address2 = expr->expr2->reg;
				}
				operation->type1 = LITERAL;
				operation->const_pointer = create_constant(INTEGER, 0);
				operation->const_pointer->int_value = 2;
				add_instruction(instructions, operation);
				operation = create_instruction(ADDOP);
			} else {
				printf("Operation not implemented: %d\n", (int) expr->sub_type);
				exit(1);
			}

			if(!expr->expr2->reg){
				operation->type1 = LOCAL;
				operation->address1 = 0;
				--*local_offset;
			} else {
				operation->type1 = REGISTER;
				operation->address1 = expr->expr2->reg;
				free_register(regs, expr->expr2->reg);
			}
			if(!expr->expr1->reg){
				operation->type2 = LOCAL;
				if(!expr->expr2->reg){
					operation->address2 = 1;
				} else {
					operation->address2 = 0;
				}
				expr->reg = expr->expr1->reg;
			} else {
				operation->type2 = REGISTER;
				operation->address2 = expr->expr1->reg;
				expr->reg = expr->expr1->reg;
			}

			add_instruction(instructions, operation);
		
			if(!expr->expr2->reg){
				operation = create_instruction(SSP);
				operation->const_pointer = create_constant(INTEGER, 0);
				operation->const_pointer->int_value = -1;
				add_instruction(instructions, operation);
			}

			if(expr->sub_type == ELEMENT){
				operation = create_instruction(DEREF);
				if(!expr->reg){
					operation->type1 = LOCAL;
					operation->address1 = 0;
				} else {
					operation->type1 = REGISTER;
					operation->address1 = expr->reg;
				}
				add_instruction(instructions, operation);
			}
		} else {
			if(expr->expr1->type == IDENTIFIER){
				if(expr->expr1->var_pointer->type == LOCAL){
					translate_expression(expr->expr2, func, instructions, local_offset, regs, to_stack);
					operation = create_instruction(MOV);
					if(!expr->expr2->reg){
						operation->type1 = LOCAL;
						operation->address1 = 0;
						expr->reg = 0;
					} else {
						operation->type1 = REGISTER;
						operation->address1 = expr->expr2->reg;
						expr->reg = expr->expr2->reg;
					}
					operation->type2 = LOCAL;
					operation->address2 = *(func->local_size) - expr->expr1->var_pointer->offset + *local_offset - 1;
					add_instruction(instructions, operation);
				} else if(expr->expr1->var_pointer->type == GLOBAL){
					translate_expression(expr->expr2, func, instructions, local_offset, regs, 0);
					operation = create_instruction(MOV);
					if(!expr->expr2->reg){
						operation->type1 = LOCAL;
						operation->address1 = 0;
						expr->reg = 0;
					} else {
						operation->type1 = REGISTER;
						operation->address1 = expr->expr2->reg;
						expr->reg = expr->expr2->reg;
					}
					operation->type2 = GLOBAL;
					operation->name2 = expr->expr1->var_pointer->name;
					add_instruction(instructions, operation);
				}
			} else if(expr->expr1->type == UNARY){
				if(expr->expr1->sub_type != DEREFERENCE){
					printf("Unexpected unary operator %d\n", expr->expr1->sub_type);
					exit(1);
				}
				translate_expression(expr->expr2, func, instructions, local_offset, regs, to_stack);
				translate_expression(expr->expr1->expr2, func, instructions, local_offset, regs, 0);
				expr->expr1->reg = expr->expr1->expr2->reg;
				operation = create_instruction(MOV);
				if(!expr->expr2->reg){
					operation->type1 = LOCAL;
					if(!expr->expr1->reg){
						operation->address1 = 1;
					} else {
						operation->address1 = 0;
					}
					expr->reg = 0;
				} else {
					operation->type1 = REGISTER;
					operation->address1 = expr->expr2->reg;
					expr->reg = expr->expr2->reg;
				}

				if(!expr->expr1->reg){
					operation->type2 = LOCALINDIRECT;
					operation->address1 = 0;
				} else {
					operation->type2 = REGISTERINDIRECT;
					operation->address1 = expr->expr1->reg;
				}

				add_instruction(instructions, operation);

				if(!expr->expr1->reg){
					operation = create_instruction(SSP);
					operation->type1 = LITERAL;
					operation->const_pointer = create_constant(INTEGER, 0);
					operation->const_pointer->int_value = -1;
					add_instruction(instructions, operation);
					--*local_offset;
				} else {
					free_register(regs, expr->expr1->reg);
				}
			}
		}
	} else if(expr->type == UNARY && expr->sub_type == NEGATE){
		translate_expression(expr->expr2, func, instructions, local_offset, regs, to_stack);
		operation = create_instruction(NEG);
		if(!expr->expr2->reg){
			operation->type1 = LOCAL;
			operation->address1 = 0;
			expr->reg = 0;
		} else {
			operation->type1 = REGISTER;
			operation->address1 = expr->expr2->reg;
			expr->reg = expr->expr2->reg;
		}

		add_instruction(instructions, operation);
	} else if(expr->type == UNARY && expr->sub_type == DEREFERENCE){
		translate_expression(expr->expr2, func, instructions, local_offset, regs, to_stack);
		operation = create_instruction(DEREF);
		if(!expr->expr2->reg){
			operation->type1 = LOCAL;
			operation->address1 = 0;
			expr->reg = 0;
		} else {
			operation->type1 = REGISTER;
			operation->address1 = expr->expr2->reg;
			expr->reg = expr->expr2->reg;
		}

		add_instruction(instructions, operation);
	} else if(expr->type == UNARY && expr->sub_type == NOT){
		translate_expression(expr->expr2, func, instructions, local_offset, regs, to_stack);
		operation = create_instruction(NOTOP);
		if(!expr->expr2->reg){
			operation->type1 = LOCAL;
			operation->address1 = 0;
			expr->reg = 0;
		} else {
			operation->type1 = REGISTER;
			operation->address1 = expr->expr2->reg;
			expr->reg = expr->expr2->reg;
		}

		add_instruction(instructions, operation);
	} else if(expr->type == UNARY && expr->sub_type == REFERENCE){
		if(expr->expr2){
			if(expr->expr2->type == IDENTIFIER){
				if(expr->expr2->var_pointer->type == LOCAL){
					if(!to_stack){
						expr->reg = allocate_register(regs);
					} else {
						expr->reg = 0;
					}

					if(!expr->reg){
						operation = create_instruction(PUSH);
						operation->type1 = STACKRELATIVE;
						operation->address1 = *(func->local_size) - expr->expr2->var_pointer->offset + *local_offset - 1;
						++*local_offset;
					} else {
						operation = create_instruction(MOV);
						operation->type1 = STACKRELATIVE;
						operation->address1 = *(func->local_size) - expr->expr2->var_pointer->offset + *local_offset - 1;
						operation->type2 = REGISTER;
						operation->address2 = expr->reg;
					}

					add_instruction(instructions, operation);
				} else if(expr->expr2->var_pointer->type == GLOBAL){
					if(!to_stack){
						expr->reg = allocate_register(regs);
					} else {
						expr->reg = 0;
					}

					if(!expr->reg){
						operation = create_instruction(PUSH);
						operation->type1 = GLOBAL;
						operation->name = expr->expr2->var_pointer->name;
						++*local_offset;
					} else {
						operation = create_instruction(MOV);
						operation->type1 = GLOBAL;
						operation->name = expr->expr2->var_pointer->name;
						operation->type2 = REGISTER;
						operation->address2 = expr->reg;
					}

					add_instruction(instructions, operation);
				}
			} else {
				printf("Expected identifier after reference operator '&'");
				exit(1);
			}
		}
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
		++*local_offset;

		add_instruction(instructions, operation);
		argument = expr->func_arguments;
		num_args = 0;
		while(argument){
			num_args++;
			translate_expression((expression *) argument->value, func, instructions, local_offset, regs, 1);
			argument = argument->next;
		}
		translate_expression(expr->expr2, func, instructions, local_offset, regs, 0);
		if(!expr->expr2->reg){
			jmp_instruction = create_instruction(JMPSTACK);
		} else {
			jmp_instruction = create_instruction(JMP);
			jmp_instruction->type1 = REGISTER;
			jmp_instruction->address1 = expr->expr2->reg;
			free_register(regs, expr->expr2->reg);
		}

		add_instruction(instructions, jmp_instruction);

		operation = create_instruction(LABEL);
		operation->name = new_label;
		add_instruction(instructions, operation);
		*local_offset -= num_args + 1;
		expr->reg = 0;
	} else {
		printf("unrecognized expression %d\n", (int) expr->type);
	}

}

void translate_statement(statement *s, block *func, instruction **instructions, unsigned int *local_offset, reg_list *regs){
	instruction *ssp;
	instruction *operation;
	instruction *branch;
	instruction *start_block;
	unsigned int id;
	unsigned int label_length;
	char *new_label;
	char *new_label2;

	if(!s->type){
		translate_expression(s->expr, func, instructions, local_offset, regs, 0);
		if(!s->expr->reg){
			ssp = create_instruction(SSP);
			ssp->type1 = LITERAL;
			ssp->const_pointer = create_constant(INTEGER, 0);
			ssp->const_pointer->int_value = -1;
			add_instruction(instructions, ssp);
			--*local_offset;
		} else {
			free_register(regs, s->expr->reg);
		}
	} else if(s->type == KEYWORD && s->sub_type == RETURN){
		translate_expression(s->expr, func, instructions, local_offset, regs, 0);
		if(!s->expr->reg){
			operation = create_instruction(POP);
			operation->type1 = LOCAL;
			operation->address1 = *(func->local_size) + 2;
		} else {
			operation = create_instruction(MOV);
			operation->type1 = REGISTER;
			operation->address1 = s->expr->reg;
			operation->type2 = LOCAL;
			operation->address2 = *(func->local_size) + 1;
			free_register(regs, s->expr->reg);
		}

		add_instruction(instructions, operation);

		operation = create_instruction(SSP);
		operation->type1 = LITERAL;
		operation->const_pointer = create_constant(INTEGER, 0);
		operation->const_pointer->int_value = -*(func->local_size);
		add_instruction(instructions, operation);

		operation = create_instruction(JMPSTACK);
		operation->type1 = LOCAL;
		operation->address1 = 0;
		add_instruction(instructions, operation);
	} else if(s->type == KEYWORD && s->sub_type == IF){
		translate_expression(s->expr, func, instructions, local_offset, regs, 1);
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

		translate_block(s->code, func, instructions, local_offset, regs);
		operation = create_instruction(LABEL);
		operation->name = new_label;
		branch->next2 = operation;
		add_instruction(instructions, operation);
	} else if(s->type == KEYWORD && s->sub_type == WHILE){
		translate_expression(s->expr, func, instructions, local_offset, regs, 1);
		branch = create_instruction(BZSTACK);
		branch->type1 = GLOBAL;

		id = while_id;
		while_id++;
		sprintf(var_temp, "__while%dafter", id);
		label_length = strlen(var_temp);
		new_label = malloc(sizeof(char)*(label_length + 1));
		strcpy(new_label, var_temp);
		branch->name = new_label;
		add_instruction(instructions, branch);
		--*local_offset;

		start_block = create_instruction(LABEL);
		sprintf(var_temp, "__while%d", id);
		label_length = strlen(var_temp);
		new_label2 = malloc(sizeof(char)*(label_length + 1));
		strcpy(new_label2, var_temp);
		start_block->name = new_label2;
		add_instruction(instructions, start_block);

		translate_block(s->code, func, instructions, local_offset, regs);

		translate_expression(s->expr, func, instructions, local_offset, regs, 1);
		operation = create_instruction(BNZSTACK);
		operation->type1 = GLOBAL;
		operation->name = new_label2;
		operation->next2 = start_block;
		add_instruction(instructions, operation);
		--*local_offset;

		operation = create_instruction(LABEL);
		operation->name = new_label;
		add_instruction(instructions, operation);

		branch->next2 = operation;
	}
}

void translate_block(block *b, block *func, instruction **instructions, unsigned int *local_offset, reg_list *regs){
	linked_list *statements;
	statements = b->statements;
	while(statements->next){
		statements = statements->next;
		translate_statement((statement *) statements->value, func, instructions, local_offset, regs);
	}
}

void translate_function(variable *var, instruction **instructions, reg_list *regs){
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
		translate_block(var->function, var->function, instructions, &local_offset, regs);
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
	operation->name = strdup(var->name);
	add_instruction(global_instructions, operation);
	if(var->is_function){
		translate_function(var, global_instructions, global_regs);
	} else {
		operation = create_instruction(CONSTANT);
		operation->const_pointer = create_constant(INTEGER, 0);
		operation->const_pointer->int_value = 0;
		add_instruction(global_instructions, operation);
	}
}

void translate_program(dictionary global_space, instruction **instructions, reg_list *regs){
	global_instructions = instructions;
	global_regs = regs;
	iterate_dictionary(global_space, _translate_program);
}

void print_instructions_68k(instruction *instructions, FILE *foutput){
	while(instructions->next1){
		instructions = instructions->next1;
		if(instructions->opcode == PUSH){
			fprintf(foutput, "	move.l ");
			if(instructions->type1 == LITERAL){
				if(instructions->const_pointer->type == INTEGER){
					fprintf(foutput, "#%d,-(A7)", instructions->const_pointer->int_value);
				}
			} else if(instructions->type1 == LOCAL){
				fprintf(foutput, "%d(A7),D7\n", instructions->address1*4);
				fprintf(foutput, "	move.l D7,-(A7)");
			} else if(instructions->type1 == GLOBAL){
				fprintf(foutput, "#%s,-(A7)", instructions->name);
			} else if(instructions->type1 == GLOBALINDIRECT){
				fprintf(foutput, "(%s),-(A7)", instructions->name);
			} else if(instructions->type1 == STACKRELATIVE){
				fprintf(foutput, "A7,D7\n");
				fprintf(foutput, "	move.l D7,-(A7)\n");
				fprintf(foutput, "	add.l #%d,(A7)\n", instructions->address1);
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "D%d,-(A7)", instructions->address1 - 1);
			}
		} else if(instructions->opcode == POP){
			fprintf(foutput, "	move.l ");
			if(instructions->type1 == GLOBAL){
				fprintf(foutput, "(A7)+,(%s)", instructions->name);
			} else if(instructions->type1 == LOCAL){
				fprintf(foutput, "(A7)+,D7\n");
				fprintf(foutput, "	move.l D7,%d(A7)", (instructions->address1 - 1)*4);
			} else if(instructions->type1 == LOCALINDIRECT){
				fprintf(foutput, "%d(A7),A0\n", instructions->address1*4);
				fprintf(foutput, "	move.l (A7)+,(A0)", instructions->address1);
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "(A7)+,D%d", instructions->address1 - 1);
			}
		} else if(instructions->opcode == MOV){
			fprintf(foutput, "	move.l ");
			if(instructions->type1 == LOCAL){
				fprintf(foutput, "%d(A7),", instructions->address1*4);
			} else if(instructions->type1 == GLOBAL){
				fprintf(foutput, "#%s,", instructions->name);
			} else if(instructions->type1 == GLOBALINDIRECT){
				fprintf(foutput, "(%s),", instructions->name);
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "D%d,", instructions->address1 - 1);
			} else if(instructions->type1 == LITERAL){
				fprintf(foutput, "#%d,", instructions->const_pointer->int_value);
			} else if(instructions->type1 == STACKRELATIVE){
				fprintf(foutput, "A7,");
			}

			if(instructions->type2 == LOCAL){
				fprintf(foutput, "%d(A7)", instructions->address2*4);
				if(instructions->type1 == STACKRELATIVE){
					fprintf(foutput, "\n	addi.l #%d,%d(A7)", instructions->address1*4, instructions->address2*4);
				}
			} else if(instructions->type2 == GLOBAL){
				fprintf(foutput, "(%s)", instructions->name2);
				if(instructions->type1 == STACKRELATIVE){
					fprintf(foutput, "\n	addi.l #%d,(%s)", instructions->address1*4, instructions->name2);
				}
			} else if(instructions->type2 == LOCALINDIRECT){
				fprintf(foutput, "D7\n	move.l %d(A7),A0\n	move.l D7,(A0)", instructions->address2*4);
				if(instructions->type1 == STACKRELATIVE){
					fprintf(foutput, "\n	addi.l #%d,(A0)", instructions->address1*4);
				}
			} else if(instructions->type2 == REGISTER){
				fprintf(foutput, "D%d", instructions->address2 - 1);
				if(instructions->type1 == STACKRELATIVE){
					fprintf(foutput, "\n	addi.l #%d,D%d", instructions->address1*4, instructions->address2 - 1);
				}
			}
		} else if(instructions->opcode == BZSTACK){
			fprintf(foutput, "	move.l (A7)+,D7\n");
			fprintf(foutput, "	cmpi.l #0,D7\n");
			if(instructions->type1 == GLOBAL){
				fprintf(foutput, "	beq.w %s", instructions->name);
			}
		} else if(instructions->opcode == BNZSTACK){
			fprintf(foutput, "	move.l (A7)+,D7\n");
			fprintf(foutput, "	cmpi.l #0,D7\n");
			if(instructions->type1 == GLOBAL){
				fprintf(foutput, "	bne.w %s", instructions->name);
			}
		} else if(instructions->opcode == SSP){
			fprintf(foutput, "	adda.l ");
			if(instructions->type1 = LITERAL){
				if(instructions->const_pointer->type == INTEGER){
					fprintf(foutput, "#%d,A7", -instructions->const_pointer->int_value*4);
				}
			}
		} else if(instructions->opcode == LSLOP){
			if(instructions->type2 == LOCAL){
				fprintf(foutput, "	move.l %d(A7),D7\n", instructions->address2*4);
			}
			fprintf(foutput, "	lsl.l ");
			if(instructions->type1 == LITERAL){
				fprintf(foutput, "#%d,", instructions->const_pointer->int_value);
			}
			if(instructions->type2 == LOCAL){
				fprintf(foutput, "D7", instructions->address2*4);
			} else if(instructions->type2 == REGISTER){
				fprintf(foutput, "D%d", instructions->address2 - 1);
			}
			if(instructions->type2 == LOCAL){
				fprintf(foutput, "\n	move.l D7,%d(A7)", instructions->address2*4);
			}
		} else if(
				instructions->opcode == ADDOP ||
				instructions->opcode == SUBOP ||
				instructions->opcode == MULOP ||
				instructions->opcode == DIVOP ||
				instructions->opcode == OROP ||
				instructions->opcode == ANDOP ||
				instructions->opcode == LTOP ||
				instructions->opcode == GTOP ||
				instructions->opcode == EQOP ||
				instructions->opcode == NEQOP){
			if(instructions->type1 == LOCAL && instructions->type2 == LOCAL){
				fprintf(foutput, "	move.l %d(A7),D7\n", instructions->address1*4);
			}

			switch(instructions->opcode){
				case ADDOP:
					fprintf(foutput, "	add.l ");
					break;
				case SUBOP:
					fprintf(foutput, "	sub.l ");
					break;
				case MULOP:
					fprintf(foutput, "	MULL");
					break;
				case DIVOP:
					fprintf(foutput, "	DIVL");
					break;
				case OROP:
					fprintf(foutput, "	or.l");
					break;
				case ANDOP:
					fprintf(foutput, "	and.l");
					break;
				case LTOP:
					fprintf(foutput, "	sub.l");
					break;
				case GTOP:
					fprintf(foutput, "	sub.l");
					break;
				case EQOP:
					fprintf(foutput, "	sub.l");
					break;
				case NEQOP:
					fprintf(foutput, "	sub.l");
					break;
			}

			if(instructions->type1 == LOCAL && instructions->type2 == LOCAL){
				fprintf(foutput, "D7,");
			} else if(instructions->type1 == LOCAL){
				fprintf(foutput, "%d(A7),", instructions->address1*4);
			} else if(instructions->type1 == GLOBAL){
				fprintf(foutput, "(%s),", instructions->name);
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "D%d,", instructions->address1 - 1);
			}

			if(instructions->type2 == LOCAL){
				fprintf(foutput, "%d(A7)", instructions->address2*4);
			} else if(instructions->type2 == GLOBAL){
				fprintf(foutput, "(%s)", instructions->name2);
			} else if(instructions->type2 == REGISTER){
				fprintf(foutput, "D%d", instructions->address2 - 1);
			}

			if(instructions->opcode == LTOP){
				if(instructions->type2 == LOCAL){
					fprintf(foutput, "	slt %d(A7)\n", instructions->address2*4);
					fprintf(foutput, "	andi.l #$000000FF,%d(A7)", instructions->address2*4);
				} else if(instructions->type2 == GLOBAL){
					fprintf(foutput, "	slt (%s)\n", instructions->name2);
					fprintf(foutput, "	andi.l #$000000FF,(%s)", instructions->name2);
				} else if(instructions->type2 == REGISTER){
					fprintf(foutput, "	slt D%d\n", instructions->address2 - 1);
					fprintf(foutput, "	andi.l #$000000FF,D%d\n", instructions->address2 - 1);
				}
			} else if(instructions->opcode == GTOP){
				if(instructions->type2 == LOCAL){
					fprintf(foutput, "	sgt %d(A7)\n", instructions->address2*4);
					fprintf(foutput, "	andi.l #$000000FF,%d(A7)", instructions->address2*4);
				} else if(instructions->type2 == GLOBAL){
					fprintf(foutput, "	sgt (%s)\n", instructions->name2);
					fprintf(foutput, "	andi.l #$000000FF,(%s)", instructions->name2);
				} else if(instructions->type2 == REGISTER){
					fprintf(foutput, "	sgt D%d\n", instructions->address2 - 1);
					fprintf(foutput, "	andi.l #$000000FF,D%d\n", instructions->address2 - 1);
				}
			} else if(instructions->opcode == EQOP){
				if(instructions->type2 == LOCAL){
					fprintf(foutput, "	seq %d(A7)\n", instructions->address2*4);
					fprintf(foutput, "	andi.l #$000000FF,%d(A7)", instructions->address2*4);
				} else if(instructions->type2 == GLOBAL){
					fprintf(foutput, "	seq (%s)\n", instructions->name2);
					fprintf(foutput, "	andi.l #$000000FF,(%s)", instructions->name2);
				} else if(instructions->type2 == REGISTER){
					fprintf(foutput, "	seq D%d\n", instructions->address2 - 1);
					fprintf(foutput, "	andi.l #$000000FF,D%d\n", instructions->address2 - 1);
				}
			} else if(instructions->opcode == NEQOP){
				if(instructions->type2 == LOCAL){
					fprintf(foutput, "	sne %d(A7)\n", instructions->address2*4);
					fprintf(foutput, "	andi.l #$000000FF,%d(A7)", instructions->address2*4);
				} else if(instructions->type2 == GLOBAL){
					fprintf(foutput, "	sne (%s)\n", instructions->name2);
					fprintf(foutput, "	andi.l #$000000FF,(%s)", instructions->name2);
				} else if(instructions->type2 == REGISTER){
					fprintf(foutput, "	sne D%d\n", instructions->address2 - 1);
					fprintf(foutput, "	andi.l #$000000FF,D%d\n", instructions->address2 - 1);
				}
			}
		} else if(instructions->opcode == NOTOP){
			fprintf(foutput, "	not.l ");
			if(instructions->type1 == LOCAL){
				fprintf(foutput, "%d(A7)", instructions->address1);
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "D%d", instructions->address1 - 1);
			}
		} else if(instructions->opcode == JMPSTACK){
			fprintf(foutput, "	movea.l (A7)+,A0\n");
			fprintf(foutput, "	jmp (A0)");
		} else if(instructions->opcode == DEREF){
			fprintf(foutput, "	move.l ");
			if(instructions->type1 == LOCAL){
				fprintf(foutput, "%d(A7),A0\n", instructions->address1);
				fprintf(foutput, "	move.l (A0),%d(A7)", instructions->address1*4);
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "D%d,A0\n", instructions->address1 - 1);
				fprintf(foutput, "	move.l (A0),D%d", instructions->address1 - 1);
			}
		} else if(instructions->opcode == JMP){
			if(instructions->type1 == LOCAL){
				fprintf(foutput, "	move.l %d(A7),A0\n", instructions->address1*4);
				fprintf(foutput, "	jmp (A0)");
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "	move.l D%d,A0\n", instructions->address1 - 1);
				fprintf(foutput, "	jmp (A0)");
			}
		} else if(instructions->opcode == LABEL){
			fprintf(foutput, "\n%s:", instructions->name);
		} else if(instructions->opcode == CONSTANT){
			fprintf(foutput, "DATA DC.L %08x", instructions->const_pointer->int_value);
		}
		fprintf(foutput, "\n");
	}
}

void print_instructions(instruction *instructions, FILE *foutput){
	while(instructions->next1){
		instructions = instructions->next1;
		if(instructions->opcode == PUSH){
			fprintf(foutput, "PUSH ");
			if(instructions->type1 == LITERAL){
				if(instructions->const_pointer->type == INTEGER){
					fprintf(foutput, "%d", instructions->const_pointer->int_value);
				}
			} else if(instructions->type1 == LOCAL){
				fprintf(foutput, "from stack %d", instructions->address1);
			} else if(instructions->type1 == GLOBAL){
				fprintf(foutput, "label %s", instructions->name);
			} else if(instructions->type1 == STACKRELATIVE){
				fprintf(foutput, "stack pointer %d", instructions->address1);
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "register %d", instructions->address1);
			}
		} else if(instructions->opcode == POP){
			fprintf(foutput, "POP ");
			if(instructions->type1 == GLOBAL){
				fprintf(foutput, "to label: %s", instructions->name);
			} else if(instructions->type1 == LOCAL){
				fprintf(foutput, "to stack %d", instructions->address1);
			} else if(instructions->type1 == LOCALINDIRECT){
				fprintf(foutput, "to address on stack %d", instructions->address1);
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "to register %d", instructions->address1);
			}
		} else if(instructions->opcode == MOV){
			fprintf(foutput, "MOV ");
			if(instructions->type1 == LOCAL){
				fprintf(foutput, "from stack %d ", instructions->address1);
			} else if(instructions->type1 == GLOBAL){
				fprintf(foutput, "label %s ", instructions->name);
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "from register %d ", instructions->address1);
			} else if(instructions->type1 == LITERAL){
				fprintf(foutput, "%d ", instructions->const_pointer->int_value);
			} else if(instructions->type1 == STACKRELATIVE){
				fprintf(foutput, "stack pointer %d ", instructions->address1);
			}

			if(instructions->type2 == LOCAL){
				fprintf(foutput, "to stack %d", instructions->address2);
			} else if(instructions->type2 == GLOBAL){
				fprintf(foutput, "to label %s", instructions->name2);
			} else if(instructions->type2 == LOCALINDIRECT){
				fprintf(foutput, "to address on stack %d", instructions->address2);
			} else if(instructions->type2 == REGISTER){
				fprintf(foutput, "to register %d", instructions->address2);
			}
		} else if(instructions->opcode == BZSTACK){
			fprintf(foutput, "BZSTACK ");
			if(instructions->type1 == GLOBAL){
				fprintf(foutput, "to label %s", instructions->name);
			}
		} else if(instructions->opcode == BNZSTACK){
			fprintf(foutput, "BNZSTACK ");
			if(instructions->type1 == GLOBAL){
				fprintf(foutput, "to label %s", instructions->name);
			}
		} else if(instructions->opcode == SSP){
			fprintf(foutput, "SSP ");
			if(instructions->type1 = LITERAL){
				if(instructions->const_pointer->type == INTEGER){
					fprintf(foutput, "%d", instructions->const_pointer->int_value);
				}
			} else if(instructions->type1 == LOCAL){
				fprintf(foutput, "from stack %d", instructions->address1);
			}
		} else if(
				instructions->opcode == ADDOP ||
				instructions->opcode == SUBOP ||
				instructions->opcode == MULOP ||
				instructions->opcode == DIVOP ||
				instructions->opcode == OROP ||
				instructions->opcode == ANDOP ||
				instructions->opcode == LTOP ||
				instructions->opcode == GTOP ||
				instructions->opcode == EQOP ||
				instructions->opcode == NEQOP){
			switch(instructions->opcode){
				case ADDOP:
					fprintf(foutput, "ADD ");
					break;
				case SUBOP:
					fprintf(foutput, "SUB ");
					break;
				case MULOP:
					fprintf(foutput, "MUL ");
					break;
				case DIVOP:
					fprintf(foutput, "DIV ");
					break;
				case OROP:
					fprintf(foutput, "OR ");
					break;
				case ANDOP:
					fprintf(foutput, "AND ");
					break;
				case LTOP:
					fprintf(foutput, "LT ");
					break;
				case GTOP:
					fprintf(foutput, "GT ");
					break;
				case EQOP:
					fprintf(foutput, "EQ ");
					break;
				case NEQOP:
					fprintf(foutput, "NEQ ");
					break;
			}

			if(instructions->type1 == LOCAL){
				fprintf(foutput, "stack %d ", instructions->address1);
			} else if(instructions->type1 == GLOBAL){
				fprintf(foutput, "label %s ", instructions->name);
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "register %d ", instructions->address1);
			}

			if(instructions->type2 == LOCAL){
				fprintf(foutput, "to stack %d", instructions->address2);
			} else if(instructions->type2 == GLOBAL){
				fprintf(foutput, "to label %s", instructions->name2);
			} else if(instructions->type2 == REGISTER){
				fprintf(foutput, "to register %d", instructions->address2);
			}
		} else if(instructions->opcode == NOTOP){
			fprintf(foutput, "NOT ");
			if(instructions->type1 == LOCAL){
				fprintf(foutput, "stack %d", instructions->address1);
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "register %d", instructions->address1);
			}
		} else if(instructions->opcode == JMPSTACK){
			fprintf(foutput, "JMPSTACK");
		} else if(instructions->opcode == DEREF){
			fprintf(foutput, "DEREF ");
			if(instructions->type1 == LOCAL){
				fprintf(foutput, "stack %d", instructions->address1);
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "register %d", instructions->address1);
			}
		} else if(instructions->opcode == JMP){
			fprintf(foutput, "JMP ");
			if(instructions->type1 == LOCAL){
				fprintf(foutput, "to address on stack %d", instructions->address1);
			} else if(instructions->type1 == REGISTER){
				fprintf(foutput, "to address on register %d", instructions->address1);
			}
		} else if(instructions->opcode == LABEL){
			fprintf(foutput, "\n%s:", instructions->name);
		} else if(instructions->opcode == CONSTANT){
			fprintf(foutput, "CONSTANT %d", instructions->const_pointer->int_value);
		}
		fprintf(foutput, "\n");
	}
}

static void _empty_callback(void *v){}

int main(int argc, char **argv){
	char *program;
	char *program_start;
	token *token_list;
	token *token_start;
	token **token_list_pointer;
	char *input_name;
	char *output_name;
	linked_list *const_list;
	linked_list *const_list_start;
	unsigned int token_length;
	unsigned int num_tokens;
	unsigned int token_index;
	unsigned int const_offset = 0;
	unsigned int local_offset;
	unsigned int i;
	unsigned long int program_length;
	reg_list *regs;

	FILE *finput;
	FILE *foutput;

	if(argc <= 1){
		printf("Error: no input files\n");
		exit(1);
	} else if(argc == 2){
		input_name = argv[1];
		output_name = "a.out";
	} else if(argc == 3){
		input_name = argv[1];
		output_name = argv[2];
	} else {
		printf("Error: too many arguments\n");
		exit(1);
	}

	finput = fopen(input_name, "rb");
	fseek(finput, 0, SEEK_END);
	program_length = ftell(finput);
	fseek(finput, 0, SEEK_SET);
	program = malloc(sizeof(char)*(program_length + 1));
	fread(program, program_length, 1, finput);
	fclose(finput);
	program[program_length] = (char) 0;

	instruction *instructions;
	instruction *original_instructions;
	instructions = create_instruction(0);
	original_instructions = instructions;

	const_list = create_linked_list((void *) 0);
	const_list_start = const_list;

	token_list = calloc(10, sizeof(token));
	token_list_pointer = &token_list;
	token_length = 10;
	token_index = 0;
	local_offset = 0;

	program_start = program;
	parse_program(&program, token_list_pointer, &token_index, &token_length);
	free(program_start);

	token_start = token_list;
	num_tokens = token_length;

	dictionary global_space;
	dictionary local_space;
	variable *var_pointer;

	global_space = create_dictionary((void *) 0);

	compile_program(&global_space, token_list_pointer, &token_length, &const_list, &const_offset);

	free(token_start);

	regs = create_reg_list(7);
	
	translate_program(global_space, &instructions, regs);
	
	foutput = fopen(output_name, "w");
	print_instructions_68k(original_instructions, foutput);
	fclose(foutput);

	free_space(global_space);
	free_instructions(original_instructions);
	free_dictionary(global_space, _empty_callback);
	free_reg_list(regs);

	linked_list *last;

	while(const_list_start){
		last = const_list_start;
		const_list_start = const_list_start->next;
		free(last);
	}

	return 0;
}

