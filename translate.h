#include "linked_list.h"
#include "dictionary.h"
#include "parse.h"
#include "compile.h"

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
#define ADD 9
#define SUB 10
#define MUL 11
#define DIV 12
#define REF 13
#define DEREF 14
#define BNZSTACK 15
#define BNZ 16
#define SSP 17
#define JMPSTACK 18
#define JMP 19
#define MOV 20
#define POPOFFSET 21

typedef struct instruction instruction;

struct instruction{
	unsigned int address;
	unsigned char operation;
	union{
		struct{
			unsigned int address1;
			unsigned int address2;
		};
		char *name;
	};
};

unsigned int get_instruction_length(unsigned char instruction);

instruction *create_instruction(unsigned int address, unsigned char operation);

void add_instruction(instruction *inst);

void translate_variable_declaration(variable *var);

void translate_function(variable *var);

void translate_variable(void *void_variable);

void translate_program(linked_list **instructions, dictionary global_space);

