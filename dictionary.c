#include "dictionary.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

dictionary create_dictionary(void *value){
	dictionary output = (dictionary) {.value = value};
	memset(output.next_chars, 0, sizeof(dictionary *)*8);
	return output;
}

void *read_dictionary(dictionary dict, char *string, unsigned char offset){
	unsigned char zeros = 0;
	unsigned char c;

	while(*string){
		c = (*string)>>offset | (*(string + 1))<<(8 - offset);
		zeros = 0;

		if(!(c&15)){
			zeros += 4;
			c>>=4;
		}
		if(!(c&3)){
			zeros += 2;
			c>>=2;
		}
		if(!(c&1)){
			zeros++;
		}

		offset += zeros + 1;
		string += (offset&0b00001000)>>3;
		offset = offset&0b00000111;
		if(dict.next_chars[zeros]){
			dict = *(dict.next_chars[zeros]);
		} else {
			return (void *) 0;
		}
	}
	
	return dict.value;
}


void write_dictionary(dictionary *dict, char *string, void *value, unsigned char offset){
	unsigned char zeros = 0;
	unsigned char c;

	while(*string){
		c = (*string)>>offset | (*(string + 1))<<(8 - offset);
		zeros = 0;

		if(!(c&15)){
			zeros += 4;
			c>>=4;
		}
		if(!(c&3)){
			zeros += 2;
			c>>=2;
		}
		if(!(c&1)){
			zeros++;
		}

		offset += zeros + 1;
		string += (offset&0b00001000)>>3;
		offset = offset&0b00000111;
		if(!dict->next_chars[zeros]){
			dict->next_chars[zeros] = malloc(sizeof(dictionary));
			*(dict->next_chars[zeros]) = create_dictionary((void *) 0);
		}
		dict = dict->next_chars[zeros];
	}
	
	dict->value = value;
}

void iterate_dictionary(dictionary dict, void (*func)(void *)){
	unsigned char i;
	
	if(dict.value){
		func(dict.value);
	}

	for(i = 0; i < 8; i++){
		if(dict.next_chars[i]){
			iterate_dictionary(*dict.next_chars[i], func);
		}
	}
}

/*
int main(){
	dictionary test_dictionary;
	test_dictionary = create_dictionary((void *) 0);
	printf("%d\n", (int) read_dictionary(test_dictionary, "ben", 0));
	write_dictionary(&test_dictionary, "ben", (void *) 221, 0);
	write_dictionary(&test_dictionary, "bey", (void *) 222, 0);
	printf("%d ", (int) read_dictionary(test_dictionary, "ben", 0));

	printf("%d\n", (int) read_dictionary(test_dictionary, "bey", 0));
	return 0;
}*/
