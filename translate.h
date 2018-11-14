#ifndef INCLUDE_TRANSLATE
#define INCLUDE_TRANSLATE
#include "parse.h"
#include "compile.h"
#include "allocate.h"

//Instructions

#define LABEL 0
#define PUSH 1
#define POP 2
#define ADDSTACK 3
#define SUBSTACK 4
#define MULSTACK 5
#define DIVSTACK 6
#define REFSTACK 7
#define DEREFSTACK 8
#define REF 13
#define DEREF 14
#define BZSTACK 15
#define BZ 16
#define SSP 17
#define JMPSTACK 18
#define JMP 19
#define MOV 20
#define CONSTANT 21
#define ORSTACK 22
#define ANDSTACK 23
#define LTSTACK 24
#define GTSTACK 25
#define EQSTACK 26
#define BNZSTACK 27
#define NEQSTACK 28
#define NOTSTACK 29
#define ADDOP 30
#define MULOP 31
#define SUBOP 32
#define DIVOP 33
#define LTOP 34
#define GTOP 35
#define EQOP 36
#define NEQOP 37
#define OROP 38
#define ANDOP 39
#define NEG 40
#define BZOP 41
#define BNZOP 42
#define NOTOP 43

typedef struct instruction instruction;

struct instruction{
	unsigned char opcode;
	instruction *next1;
	instruction *next2;
	unsigned char type1;
	unsigned char type2;
	union{
		int address1;
		constant *const_pointer;
		char *name;
	};
	union{
		unsigned int address2;
		char *name2;
	};
};

instruction *create_instruction(unsigned char opcode);

void free_instruction(instruction *i);

void free_instructions(instruction *i);

void add_instruction(instruction **instructions, instruction *i);

void translate_dereference(expression *expr, block *func, instruction **instructions, unsigned int *local_offset);

void translate_expression(expression *expr, block *func, instruction **instructions, unsigned int *local_offset, reg_list *regs, unsigned char to_stack);

void translate_statement(statement *s, block *func, instruction **instructions, unsigned int *local_offset, reg_list *regs);

void translate_block(block *b, block *func, instruction **instructions, unsigned int *local_offset, reg_list *regs);

void translate_function(variable *var, instruction **instructions, reg_list *regs);

void translate_program(dictionary global_space, instruction **instructions, reg_list *regs);
#endif

