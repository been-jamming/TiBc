#include <stdlib.h>
#include <stdio.h>
#include "translate.h"
#include "optimize1.h"
#include "include68k.h"

/*
 * B compiler
 *
 * by Ben Jones
 *
 * Began on
 * 8/1/2018
 *
 * 'main.c' created on
 * 2/14/2019 <3
 */

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

	include68k(&global_space);
	
	compile_program(&global_space, token_list_pointer, &token_length, &const_list, &const_offset);

	free(token_start);

	optimize1(global_space);

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

