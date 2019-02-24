#include <files.h>
#include "assemble68k.h"

file_bits current_file;
dictionary labels;

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

void write_long(uint32_t l){
	unsigned char byte;

	byte = l>>24;
	write_byte(byte, current_file);
	byte = (l>>16)&0b11111111;
	write_byte(byte, current_file);
	byte = (l>>8)&0b11111111;
	write_byte(byte, current_file);
	byte = l&0b11111111;
	write_byte(byte, current_file);
}

void write_word(uint16_t w){
	unsigned char byte;

	byte = w>>8;
	write_byte(byte, current_file);
	byte = w&0b11111111;
	write_byte(byte, current_file);
}

unsigned int get_file_pointer(){
	return FGetPos(current_file.fsp);
}

label_entry *create_label_entry(unsigned int file_pointer){
	label_entry *output;

	output = malloc(sizeof(label_entry));
	output->found = 0;
	output->file_pointer = file_pointer;
	
	return output;
}

void write_size1(SIZE size){
	switch(size){
		case BYTE:
			write_bits(0, 2);
			break;
		case WORD:
			write_bits(1, 2);
			break;
		case LONG:
			write_bits(2, 2);
	}
}

void write_size2(SIZE size){
	switch(size){
		case WORD:
			write_bits(0, 1);
			break;
		case LONG:
			write_bits(1, 1);
			break;
	}
}

void write_size3(SIZE size){
	switch(size){
		case BYTE:
			write_bits(1, 2);
			break;
		case WORD:
			write_bits(3, 2);
			break;
		case LONG:
			write_bits(2, 2);
	}
}

void write_mode1(ADDRESS_MODE mode, unsigned int reg){	
	switch(mode){
		case PC_DISP:
			write_bits(2, 3);
			break;
		case PC_IDX:
			write_bits(3, 3);
			break;
		case ABS_SHORT:
			write_bits(0, 3);
			break;
		case ABS_LONG:
			write_bits(1, 3);
			break;
		case IMMEDIATE:
			write_bits(4, 3);
			break;
		default:
			write_bits(reg, 3);
	}

	switch(mode){
		case DATA_REG:
			write_bits(0, 3);
			break;
		case ADDRESS_REG:
			write_bits(1, 3);
			break;
		case ADDRESS:
			write_bits(2, 3);
			break;
		case ADDRESS_PI:
			write_bits(3, 3);
			break;
		case ADDRESS_PD:
			write_bits(4, 3);
			break;
		case ADDRESS_DISP:
			write_bits(5, 3);
			break;
		case ADDRESS_IDX:
			write_bits(6, 3);
			break;
		default:
			write_bits(7, 3);
	}
}

void write_mode2(ADDRESS_MODE mode, unsigned int reg){	
	switch(mode){
		case DATA_REG:
			write_bits(0, 3);
			break;
		case ADDRESS_REG:
			write_bits(1, 3);
			break;
		case ADDRESS:
			write_bits(2, 3);
			break;
		case ADDRESS_PI:
			write_bits(3, 3);
			break;
		case ADDRESS_PD:
			write_bits(4, 3);
			break;
		case ADDRESS_DISP:
			write_bits(5, 3);
			break;
		case ADDRESS_IDX:
			write_bits(6, 3);
			break;
		default:
			write_bits(7, 3);
	}

	switch(mode){
		case PC_DISP:
			write_bits(2, 3);
			break;
		case PC_IDX:
			write_bits(3, 3);
			break;
		case ABS_SHORT:
			write_bits(0, 3);
			break;
		case ABS_LONG:
			write_bits(1, 3);
			break;
		case IMMEDIATE:
			write_bits(4, 3);
			break;
		default:
			write_bits(reg, 3);
	}
}

void write_immediate(unsigned long int immediate, SIZE size){
	switch(size){
		case BYTE:
			write_bits(immediate, 8);
			break;
		case WORD:
			write_word(immediate);
			break;
		case LONG:
			write_long(immediate);
	}
}

void write_operation(operation o){
	switch(o.op){
		case ORI:
			write_bits(0, 8);
			write_size1(o.size);
			write_mode2(o.mode1, o.reg1);
			write_immediate(o.immediate, o.size);
			break;
		case ANDI:
			write_bits(2, 8);
			write_size1(o.size);
			write_mode2(o.mode1, o.reg1);
			write_immediate(o.immediate, o.size);
			break;
		case SUBI:
			write_bits(4, 8);
			write_size1(o.size);
			write_mode2(o.mode1, o.reg1);
			write_immediate(o.immediate, o.size);
			break;
		case ADDI:
			write_bits(6, 8);
			write_size1(o.size);
			write_mode2(o.mode1, o.reg1);
			write_immediate(o.immediate, o.size);
			break;
		case EORI:
			write_bits(10, 8);
			write_size1(o.size);
			write_mode2(o.mode1, o.reg1);
			write_immediate(o.immediate, o.size);
			break;
		case CMPI:
			write_bits(12, 8);
			write_size1(o.size);
			write_mode2(o.mode1, o.reg1);
			write_immediate(o.immedaite, o.size);
			break;
		case BTST:
			write_bits(8, 8);
			write_bits(0, 2);
			write_mode2(o.mode1, o.reg1);
			write_immediate(o.immediate, BYTE);
			break;
	}
}

