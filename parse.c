/*
Parser for B compiler
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

//Tokens

//Literals

#define LITERAL 1

//sub_types

	#define INTEGER 1
	#define CHARACTER 2
	#define STRING 3

//Operators

#define OPERATOR 2

//sub_types

	#define ASSIGN 1
	#define ADD 2
	#define SUBTRACT 3
	#define MULTIPLY 4
	#define DIVIDE 5
	#define EQUADD 6
	#define EQUSUB 7
	#define EQUMUL 8
	#define EQUDIV 9
	#define EQUALS 10
	#define LESSTHAN 11
	#define GREATERTHAN 12
	#define LESSTHANEQU 13
	#define GREATERTHANEQU 14
	#define MODULO 15
	#define AND 16
	#define OR 17
	#define XOR 18
	#define NOT 19
	#define BITAND 20
	#define BITOR 21
	#define BITXOR 22
	#define BITNOT 23
	#define ELEMENT 24

//Control

#define CONTROL 3

//sub_types
	#define OPENBRACES 1
	#define CLOSEBRACES 2
	#define OPENPARENTHESES 3
	#define CLOSEPARENTHESES 4
	#define SEMICOLON 5

//Key words

#define KEYWORD 4

//sub_types

	#define IF 1
	#define ELSE 2
	#define WHILE 3
	#define VAR 4

//Unary operator

#define UNARY 5

//sub_types

	#define DEREFERENCE 1
	#define REFERENCE 2

//Identifier

#define IDENTIFIER 6

typedef struct token token;

struct token{
	unsigned char type;
	unsigned char sub_type;
	union{
		int int_value;
		char char_value;
		char *string_value;
	};
};

unsigned char is_alpha(unsigned char c){
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

unsigned char is_digit(unsigned char c){
	return c >= '0' && c <= '9';
}

void skip_whitespace(char **c){
	while(**c == ' ' || **c == '	' || **c == '\n'){
		++*c;
	}
}

int get_integer(unsigned char **c){
	int output;
	output = 0;
	
	while(is_digit(**c)){
		output = (output*10) + (**c - '0');
		++*c;
	}
	
	return output;
}

char *get_identifier(char **c){
	char *output;
	char *orig_c;
	unsigned int string_length;
	
	orig_c = *c;
	string_length = 0;
	
	while(is_alpha(**c) || is_digit(**c) || **c == '_'){
		++*c;
		string_length += 1;
	}
	
	output = malloc(sizeof(char)*(string_length + 1));
	output[string_length] = (char) 0;
	memcpy(output, orig_c, string_length);
	
	return output;
}

void print_token(token t){
	if(t.type == LITERAL){
		if(t.sub_type == INTEGER){
			printf("%d", t.int_value);
		} else if(t.sub_type == CHARACTER){
			printf("%d", t.char_value);
		} else if(t.sub_type == STRING){
			printf("%s", t.string_value);
		}
	} else if(t.type == OPERATOR){
		if(t.sub_type == ASSIGN){
			printf("=");
		} else if(t.sub_type == ADD){
			printf("+");
		} else if(t.sub_type == SUBTRACT){
			printf("-");
		} else if(t.sub_type == MULTIPLY){
			printf("*");
		} else if(t.sub_type == DIVIDE){
			printf("/");
		}
	}
}

token get_token(char **c){
	token output;
	
	skip_whitespace(c);
	if(is_digit(**c)){
		output.type = LITERAL;
		output.sub_type = INTEGER;
		output.int_value = get_integer((unsigned char **) c);
		return output;
	} else if(**c == '-'){
		++*c;
		if(is_digit(**c)){
			output.type = LITERAL;
			output.sub_type = INTEGER;
			output.int_value = -get_integer((unsigned char **) c);
			return output;
		} else {
			output.type = OPERATOR;
			output.sub_type = SUBTRACT;
			return output;
		}
	} else if(**c == '+'){
		++*c;
		output.type = OPERATOR;
		output.sub_type = ADD;
		return output;
	} else if(**c == '*'){
		++*c;
		output.type = OPERATOR;
		output.sub_type = MULTIPLY;
		return output;
	} else if(**c == '/'){
		++*c;
		output.type = OPERATOR;
		output.sub_type = DIVIDE;
		return output;
	} else if(**c == '='){
		++*c;
		if(**c == '='){
			++*c;
			output.type = OPERATOR;
			output.sub_type = EQUALS;
			return output;
		} else if(**c == '+'){
			++*c;
			output.type = OPERATOR;
			output.sub_type = EQUADD;
			return output;
		} else if(**c == '-'){
			++*c;
			output.type = OPERATOR;
			output.sub_type = EQUSUB;
			return output;
		} else if(**c == '*'){
			++*c;
			output.type = OPERATOR;
			output.sub_type = EQUMUL;
			return output;
		} else if(**c == '/'){
			++*c;
			output.type = OPERATOR;
			output.sub_type = EQUDIV;
			return output;
		} else if(**c == ' ' || **c == '	' || **c == '\n'){
			++*c;
			output.type = OPERATOR;
			output.sub_type = ASSIGN;
			return output;
		} else {
			printf("Unknown operator: =%c\n", **c);
			getchar();
			exit(1);
		}
	} else if(**c == '('){
		++*c;
		output.type = CONTROL;
		output.sub_type = OPENPARENTHESES;
		return output;
	} else if(**c == ')'){
		++*c;
		output.type = CONTROL;
		output.sub_type = CLOSEPARENTHESES;
		return output;
	} else if(**c == '{'){
		++*c;
		output.type = CONTROL;
		output.sub_type = OPENBRACES;
		return output;
	} else if(**c == '}'){
		++*c;
		output.type = CONTROL;
		output.sub_type = CLOSEBRACES;
		return output;
	} else if(**c == '\''){
		++*c;
		output.type = LITERAL;
		output.sub_type = CHARACTER;
		output.char_value = **c;
		++*c;
		if(**c != '\''){
			printf("Expected closing token ' instead of %c\n", **c);
		}
		++*c;
	} else if(**c == '['){
		++*c;
		output.type = OPERATOR;
		output.sub_type = ELEMENT;
		return output;
	} else if(**c == ']'){
		++*c;
		return get_token(c);
	} else if(is_alpha(**c)){
		if(**c == 'i'){
			++*c;
			if(**c == 'f'){
				++*c;
				if(!is_alpha(**c) && **c != '_' && !is_digit(**c)){
					output.type = KEYWORD;
					output.sub_type = IF;
					return output;
				} else {
					*c -= 2;
					output.type = IDENTIFIER;
					output.string_value = get_identifier(c);
					return output;
				}
			} else {
				--*c;
				output.type = IDENTIFIER;
				output.string_value = get_identifier(c);
				return output;
			}
		} else if(**c == 'e'){
			++*c;
			if(**c == 'l'){
				++*c;
				if(**c == 's'){
					++*c;
					if(**c == 'e'){
						++*c;
						if(!is_alpha(**c) && **c != '_' && !is_digit(**c)){
							output.type = KEYWORD;
							output.sub_type = ELSE;
							return output;
						} else {
							*c -= 4;
							output.type = IDENTIFIER;
							output.string_value = get_identifier(c);
							return output;
						}
					} else {
						*c -= 3;
						output.type = IDENTIFIER;
						output.string_value = get_identifier(c);
						return output;
					}
				} else {
					*c -= 2;
					output.type = IDENTIFIER;
					output.string_value = get_identifier(c);
					return output;
				}
			} else {
				--*c;
				output.type = IDENTIFIER;
				output.string_value = get_identifier(c);
				return output;
			}
		} else if(**c == 'v'){
			++*c;
			if(**c == 'a'){
				++*c;
				if(**c == 'r'){
					++*c;
					if(!is_alpha(**c) && **c != '_' && !is_digit(**c)){
						output.type = KEYWORD;
						output.sub_type = VAR;
						return output;
					} else {
						*c -= 3;
						output.type = IDENTIFIER;
						output.string_value = get_identifier(c);
						return output;
					}
				} else {
					*c -= 2;
					output.type = IDENTIFIER;
					output.string_value = get_identifier(c);
					return output;
				}
			} else {
				--*c;
				output.type = IDENTIFIER;
				output.string_value = get_identifier(c);
				return output;
			}
		} else if(**c == 'w'){
			++*c;
			if(**c == 'h'){
				++*c;
				if(**c == 'i'){
					++*c;
					if(**c == 'l'){
						++*c;
						if(**c == 'e'){
							++*c;
							if(!is_alpha(**c) && **c != '_' && !is_digit(**c)){
								output.type = KEYWORD;
								output.sub_type = WHILE;
								return output;
							} else {
								*c -= 5;
								output.type = IDENTIFIER;
								output.string_value = get_identifier(c);
								return output;
							}
						} else {
							*c -= 4;
							output.type = IDENTIFIER;
							output.string_value = get_identifier(c);
							return output;
						}
					} else {
						*c -= 3;
						output.type = IDENTIFIER;
						output.string_value = get_identifier(c);
						return output;
					}
				} else {
					*c -= 2;
					output.type = IDENTIFIER;
					output.string_value = get_identifier(c);
					return output;
				}
			} else {
				--*c;
				output.type = IDENTIFIER;
				output.string_value = get_identifier(c);
				return output;
			}
		} else {
			output.type = IDENTIFIER;
			output.string_value = get_identifier(c);
			return output;
		}
	} else if(**c == ';'){
		output.type = CONTROL;
		output.sub_type = SEMICOLON;
		return output;
	} else {
		printf("Unrecognized token starting at: %c", **c);
		exit(1);
	}
}

void add_token(token **token_list, token t, unsigned int *token_index, unsigned int *token_length){
	token *new_tokens;
	
	while(*token_index >= *token_length){
		*token_length += 10;
		new_tokens = calloc(*token_length, sizeof(token));
		memcpy(new_tokens, *token_list, sizeof(token)*(*token_length - 10));
		free(*token_list);
		*token_list = new_tokens;
	}
	
	(*token_list)[*token_index] = t;
	*token_index += 1;
}

void parse_expression(char **c, token **token_list, unsigned int *token_index, unsigned int *token_length, token closing_token){
	char **orig_c;
	char *temp_c;
	char *temp_c2;
	token last_token;
	token current_token;
	token next_token;
	last_token.type = 0;
	last_token.sub_type = 0;
	orig_c = c;
	
	skip_whitespace(c);
	temp_c = *c;
	current_token = get_token(c);
	while(current_token.type != closing_token.type || current_token.sub_type != closing_token.sub_type){
		if(current_token.type == LITERAL || current_token.type == IDENTIFIER){
			temp_c2 = *c;
			next_token = get_token(c);
			
			if(next_token.type != OPERATOR && current_token.type == LITERAL){
				printf("Expected operartor instead of: %c\n", *temp_c2);
				exit(1);
			} else if(next_token.type == OPERATOR || (current_token.type == IDENTIFIER && next_token.type == CONTROL && next_token.sub_type == OPENPARENTHESES)){
				add_token(token_list, current_token, token_index, token_length);
				add_token(token_list, next_token, token_index, token_length);
			} else if(next_token.type == closing_token.type && next_token.sub_type == closing_token.sub_type){
				add_token(token_list, current_token, token_index, token_length);
				add_token(token_list, next_token, token_index, token_length);
				return;
			} else {
				printf("Unexpected token: %c\n", *temp_c2);
				exit(1);
			}
		} else if(current_token.type == CONTROL && current_token.sub_type == OPENPARENTHESES){
			add_token(token_list, current_token, token_index, token_length);
			parse_expression(c, token_list, token_index, token_length, (token) {.type = CONTROL, .sub_type = CLOSEPARENTHESES});
		}
		
		skip_whitespace(c);
		if(!**c){
			printf("Expected closing token: ");
			print_token(closing_token);
			printf("\n");
			exit(1);
		}
		temp_c = *c;
		current_token = get_token(c);
	}
}

int main(){
	char test_program_const[] = "hello + (this + is + a) + test;";
	char *test_program;
	token **token_list;
	unsigned int token_length;
	unsigned int token_index;
	
	test_program = malloc(sizeof(test_program_const)*sizeof(char));
	strcpy(test_program, test_program_const);
	token_list = malloc(sizeof(token *));
	*token_list = calloc(10, sizeof(token));
	token_length = 10;
	token_index = 0;
	
	parse_expression(&test_program, token_list, &token_index, &token_length, (token) {.type = CONTROL, .sub_type = SEMICOLON});
	
	unsigned int i;
	for(i = 0; i < token_index; i++){
		printf("%d %d\n", (int) ((*token_list)[i].type), (int) ((*token_list)[i].sub_type));
	}
	
	return 0;
}