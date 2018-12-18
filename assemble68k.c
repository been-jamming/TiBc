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

label_entry *create_label_entry(){
	label_entry *output;
	output = malloc(sizeof(label_entry));
	output.label_seen = 0;
	output.label_pointers = (label_pointer *) 0;
	return output;
}

label_pointer *create_label_pointer(unsigned int file_position){
	label_pointer *output;
	output = malloc(sizeof(label_pointer));
	output.file_position = file_position;
	output.next = (label_pointer *) 0;
	return output;
}

void add_label_pointer(label_pointer **head, unsigned int file_position){
	label_pointer *new_label_pointer;
	new_label_pointer = create_label_pointer(file_position);
	new_label_pointer->next = *head;
	*head = new_label_pointer;
}

void write_asm68k(instruction *instructions){
	label_entry *label;
	unsigned int fpos;

	while(instructions->next1){
		instructions = instructions->next1;
		if(instructions->opcode == PUSH){
			if(instructions->type1 == LITERAL){
				if(instructions->const_pointer->type == INTEGER){
					write_bits(0b0010, 4);
					write_bits(0b111100, 6);
					write_bits(0b111100, 6);
					write_long(instructions->const_pointer->int_value);
				}
			} else if(instructions->type1 == LOCAL){
				write_bits(0b00101110, 8);
				write_bits(0b00101111, 8);
				write_word(instructions->address1*4);
				write_bits(0b00101110, 8);
				write_bits(0b00100111, 8);
			} else if(instructions->type1 == GLOBAL){
				write_bits(0b00101111, 8);
				write_bits(0b00111100, 8);
				
				label = (label_entry *) dictionary_read(labels, instructions->name);
				if(!label){
					label = create_label_entry();
					dictionary_write(&labels, instructions->name, label);
				}

				if(label->label_seen){
					write_long(label->address);
				} else {
					fpos = FGetPos(current_file);
					add_label_pointer(&(label->label_pointers), fpos);
					write_long(0);
				}
			} else if(instructions->type1 == GLOBALINDIRECT){
				write_bits(0b00100011, 8);
				write_bits(0b11100111, 8);
				
				label = (label_entry *) dicitonary_read(labels, instructions->name);
				if(!label){
					label = create_label_entry();
					dictionary_write(&labels, instructions->name, label);
				}

				if(label->label_seen){
					write_long(label->address);
				} else {
					fpos = FGetPos(current_file);
					add_label_pointer(&(label->label_pointers), fpos);
					write_long(0);
				}
		}
	}
}
