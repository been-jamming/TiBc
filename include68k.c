#include "dictionary.h"
#include "compile.h"

void include(dictionary *global_space, char *var_name, unsigned char *data, unsigned int data_size){
	variable *var_pointer;

	var_pointer = create_variable(GLOBAL, 0, var_name);
	var_pointer->is_function = 1;
	var_pointer->is_data = 1;
	var_pointer->data = data;
	var_pointer->data_size = data_size;
	write_dictionary(global_space, var_name, var_pointer, 0);
}

void include68k(dictionary *global_space){
	include(global_space, "putchar",
			"	move.w #0,D0\n"
			"	move.w #1,D1\n"
			"	move.l A7,A1\n"
			"	adda.l #3,A1\n"
			"	trap #15\n"
			"	adda.l #4,A7\n"
			"	move.l (A7)+,A0\n"
			"	jmp (A0)", 0);
}
