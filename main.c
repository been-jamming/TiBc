#include <stdlib.h>
#include <stdio.h>
#include "translate.h"
#include "optimize1.h"
#include "optimize2.h"
#include "include68k.h"
#include "compile.h"
#include "main.h"

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

token *token_list = (token *) 0;
token *token_start = (token *) 0;
token **token_list_pointer = (token **) 0;
linked_list *const_list = (linked_list *) 0;
linked_list *const_list_start = (linked_list *) 0;
reg_list *regs = (reg_list *) 0;
instruction *instructions = (instruction *) 0;
instruction *original_instructions = (instruction *) 0;
dictionary global_space;
char *program;
char *program_start;
unsigned int token_length;
unsigned char DONE_PARSING;

void safe_exit(int exit_code){
	unsigned int i;
	
	printf("Line %d:\n%s", current_line, error_message);

	if(!DONE_PARSING && token_list){
		for(i = 0; i < token_length; i++){
			if(token_list[i].type == IDENTIFIER){
				free(token_list[i].string_value);
			}
		}
		free(token_list);
	}
	if(program_start){
		free(program_start);
	}	
	if(token_start){
		free(token_start);
	}
	free_space(global_space);
	if(original_instructions){
		free_instructions(original_instructions);
	}
	free_dictionary(global_space, _empty_callback);
	if(regs){
		free_reg_list(regs);
	}
	linked_list *last;

	while(const_list_start){
		last = const_list_start;
		const_list_start = const_list_start->next;
		free(last);
	}

	exit(exit_code);
}

int main(int argc, char **argv){
	char *input_name;
	char *output_name;
	unsigned int num_tokens;
	unsigned int token_index;
	unsigned int const_offset = 0;
	unsigned int local_offset;
	unsigned int i;
	unsigned long int program_length;

	FILE *finput;
	FILE *foutput;

	global_space = create_dictionary((void *) 0);
	current_line = 1;
	DONE_PARSING = 0;
	
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
	DONE_PARSING = 1;
	free(program_start);
	program_start = (char *) 0;

	token_start = token_list;
	num_tokens = token_length;

	include68k(&global_space);
	
	compile_program(&global_space, token_list_pointer, &token_length, &const_list, &const_offset);

	free(token_start);

	optimize1(global_space);

	regs = create_reg_list(7);

	translate_program(global_space, &instructions, regs);

	sweep_instructions(&original_instructions);
	
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

