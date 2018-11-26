#include <files.h>
#include "assemble68k.h"

file_bits current_file;

void file_error(){}

void write_byte(unsigned char byte, FILES *fsp){
	unsigned short status;

	status = FPutC(byte, fsp);
	if(status != FS_OK){
		file_error();
	}
}

void write_bit(unsigned char bit){
	current_file.current_byte = current_file.current_byte>>1 | bit;
	if(current_file.current_bitnum == 7){
		write_byte(current_file.current_byte, current_file.fsp);
		current_file.current_bitnum = 0;
	} else {
		current_file.current_bitnum++;
	}
}

void write_bits(unsigned char byte, unsigned char num_bits){
	unsigned char i;
	
	byte <<= (8 - num_bits);
	for(i = 0; i < num_bits; i++){
		write_bit(byte&0b10000000);
		byte <<= 1;
	}
}

void write_asm68k(instruction *instructions){
	while(instructions->next1){
		instructions = instructions->next1;
		if(instructions->opcode == PUSH){
			write_bits(0b0010, 4);
		}
	}
}
