/*
Parser for B compiler
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "parse.h"

token last_token = (token) {.type = 0, .sub_type = 0};

unsigned char is_alpha(unsigned char c){
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

unsigned char is_digit(unsigned char c){
	return c >= '0' && c <= '9';
}

void skip_whitespace(char **c){
	while(**c == ' ' || **c == '	' || **c == '\n' || **c == '\r'){
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
	} else if(**c == '-'){
		++*c;
		if(last_token.type != IDENTIFIER && last_token.type != LITERAL && (last_token.type != CONTROL || last_token.sub_type != CLOSEPARENTHESES) && (last_token.type != CONTROL || last_token.sub_type != CLOSEBRACKET)){
			if(is_digit(**c)){
				output.type = LITERAL;
				output.sub_type = INTEGER;
				output.int_value = -get_integer((unsigned char **) c);
			} else {
				output.type = UNARY;
				output.sub_type = NEGATE;
			}
		} else {
			output.type = OPERATOR;
			output.sub_type = SUBTRACT;
		}
	} else if(**c == '+'){
		++*c;
		output.type = OPERATOR;
		output.sub_type = ADD;
	} else if(**c == '*'){
		++*c;
		output.type = OPERATOR;
		output.sub_type = MULTIPLY;
	} else if(**c == '/'){
		++*c;
		output.type = OPERATOR;
		output.sub_type = DIVIDE;
	} else if(**c == '&'){
		++*c;
		output.type = OPERATOR;
		output.sub_type = AND;
	} else if(**c == '|'){
		++*c;
		output.type = OPERATOR;
		output.sub_type = OR;
	} else if(**c == '!'){
		++*c;
		if(**c == '='){
			++*c;
			output.type = OPERATOR;
			output.sub_type = NOTEQUALS;
		} else {
			output.type = UNARY;
			output.sub_type = NOT;
		}
	} else if(**c == '<'){
		++*c;
		output.type = OPERATOR;
		output.sub_type = LESSTHAN;
	} else if(**c == '>'){
		++*c;
		output.type = OPERATOR;
		output.sub_type = GREATERTHAN;
	} else if(**c == '='){
		++*c;
		if(**c == '='){
			++*c;
			output.type = OPERATOR;
			output.sub_type = EQUALS;
		} else if(**c == '+'){
			++*c;
			output.type = OPERATOR;
			output.sub_type = EQUADD;
		} else if(**c == '-'){
			++*c;
			output.type = OPERATOR;
			output.sub_type = EQUSUB;
		} else if(**c == '*'){
			++*c;
			output.type = OPERATOR;
			output.sub_type = EQUMUL;
		} else if(**c == '/'){
			++*c;
			output.type = OPERATOR;
			output.sub_type = EQUDIV;
		} else if(**c == ' ' || **c == '	' || **c == '\n'){
			++*c;
			output.type = OPERATOR;
			output.sub_type = ASSIGN;
		} else {
			printf("Unknown operator: =%c\n", **c);
			getchar();
			exit(1);
		}
	} else if(**c == '('){
		++*c;
		output.type = CONTROL;
		output.sub_type = OPENPARENTHESES;
	} else if(**c == ')'){
		++*c;
		output.type = CONTROL;
		output.sub_type = CLOSEPARENTHESES;
	} else if(**c == '{'){
		++*c;
		output.type = CONTROL;
		output.sub_type = OPENBRACES;
	} else if(**c == '}'){
		++*c;
		output.type = CONTROL;
		output.sub_type = CLOSEBRACES;
	} else if(**c == ','){
		++*c;
		output.type = CONTROL;
		output.sub_type = COMMA;
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
	} else if(**c == ']'){
		++*c;
		output.type = CONTROL;
		output.sub_type = CLOSEBRACKET;
	} else if(is_alpha(**c)){
		if(**c == 'i'){
			++*c;
			if(**c == 'f'){
				++*c;
				if(!is_alpha(**c) && **c != '_' && !is_digit(**c)){
					output.type = KEYWORD;
					output.sub_type = IF;
				} else {
					*c -= 2;
					output.type = IDENTIFIER;
					output.string_value = get_identifier(c);
				}
			} else {
				--*c;
				output.type = IDENTIFIER;
				output.string_value = get_identifier(c);
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
						} else {
							*c -= 4;
							output.type = IDENTIFIER;
							output.string_value = get_identifier(c);
						}
					} else {
						*c -= 3;
						output.type = IDENTIFIER;
						output.string_value = get_identifier(c);
					}
				} else {
					*c -= 2;
					output.type = IDENTIFIER;
					output.string_value = get_identifier(c);
				}
			} else {
				--*c;
				output.type = IDENTIFIER;
				output.string_value = get_identifier(c);
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
					} else {
						*c -= 3;
						output.type = IDENTIFIER;
						output.string_value = get_identifier(c);
					}
				} else {
					*c -= 2;
					output.type = IDENTIFIER;
					output.string_value = get_identifier(c);
				}
			} else {
				--*c;
				output.type = IDENTIFIER;
				output.string_value = get_identifier(c);
			}
		} else if(**c == 'r'){
			++*c;
			if(**c == 'e'){
				++*c;
				if(**c == 't'){
					++*c;
					if(**c == 'u'){
						++*c;
						if(**c == 'r'){
							++*c;
							if(**c == 'n'){
								++*c;
								if(!is_alpha(**c) && **c != '_' && !is_digit(**c)){
									output.type = KEYWORD;
									output.sub_type = RETURN;
								} else {
									*c -= 6;
									output.type = IDENTIFIER;
									output.string_value = get_identifier(c);
								}
							} else {
								*c -= 5;
								output.type = IDENTIFIER;
								output.string_value = get_identifier(c);
							}
						} else {
							*c -= 4;
							output.type = IDENTIFIER;
							output.string_value = get_identifier(c);
						}
					} else {
						*c -= 3;
						output.type = IDENTIFIER;
						output.string_value = get_identifier(c);
					}
				} else {
					*c -= 2;
					output.type = IDENTIFIER;
					output.string_value = get_identifier(c);
				}
			} else {
				--*c;
				output.type = IDENTIFIER;
				output.string_value = get_identifier(c);
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
							} else {
								*c -= 5;
								output.type = IDENTIFIER;
								output.string_value = get_identifier(c);
							}
						} else {
							*c -= 4;
							output.type = IDENTIFIER;
							output.string_value = get_identifier(c);
						}
					} else {
						*c -= 3;
						output.type = IDENTIFIER;
						output.string_value = get_identifier(c);
					}
				} else {
					*c -= 2;
					output.type = IDENTIFIER;
					output.string_value = get_identifier(c);
				}
			} else {
				--*c;
				output.type = IDENTIFIER;
				output.string_value = get_identifier(c);
			}
		} else {
			output.type = IDENTIFIER;
			output.string_value = get_identifier(c);
		}
	} else if(**c == ';'){
		++*c;
		output.type = CONTROL;
		output.sub_type = SEMICOLON;
	} else {
		printf("Unrecognized token starting at: %c", **c);
		exit(1);
	}
	
	last_token = output;
	return output;
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
	token current_token;
	token next_token;
	unsigned char do_next_token = 1;

	last_token.type = 0;
	last_token.sub_type = 0;
	orig_c = c;
	
	skip_whitespace(c);
	temp_c = *c;
	current_token = get_token(c);
	while((current_token.type != closing_token.type || current_token.sub_type != closing_token.sub_type) && (current_token.type != CONTROL || current_token.sub_type != CLOSEPARENTHESES)){
		if(current_token.type == LITERAL || current_token.type == IDENTIFIER){
			add_token(token_list, current_token, token_index, token_length);
			
			temp_c2 = *c;
			next_token = get_token(c);
			while((next_token.type == CONTROL && next_token.sub_type == OPENPARENTHESES) || (next_token.type == OPERATOR && next_token.sub_type == ELEMENT)){
				if(next_token.type == CONTROL && next_token.sub_type == OPENPARENTHESES){
					add_token(token_list, next_token, token_index, token_length);
					skip_whitespace(c);
					if(**c == ')'){
						next_token = get_token(c);
						add_token(token_list, next_token, token_index, token_length);
					} else {
						while(*(*c - 1) != ')'){
							parse_expression(c, token_list, token_index, token_length, (token) {.type = CONTROL, .sub_type = COMMA});
						}
					}
					skip_whitespace(c);
					temp_c2 = *c;
					next_token = get_token(c);
				} else if(next_token.type == OPERATOR && next_token.sub_type == ELEMENT){
					add_token(token_list, next_token, token_index, token_length);
					skip_whitespace(c);
					parse_expression(c, token_list, token_index, token_length, (token) {.type = CONTROL, .sub_type = CLOSEBRACKET});
					skip_whitespace(c);
					temp_c2 = *c;
					next_token = get_token(c);
				}
			}

			if(next_token.type != OPERATOR && current_token.type != LITERAL && (next_token.type != closing_token.type || next_token.sub_type != closing_token.sub_type) && (next_token.type != CONTROL || next_token.sub_type != CLOSEPARENTHESES)){
				printf("Expected operator instead of: %c\n", *temp_c2);
				exit(1);
			} else if(next_token.type == OPERATOR){
				add_token(token_list, next_token, token_index, token_length);
			} else if((next_token.type == closing_token.type && next_token.sub_type == closing_token.sub_type) || (next_token.type == CONTROL && next_token.sub_type == CLOSEPARENTHESES)){
				add_token(token_list, next_token, token_index, token_length);
				return;
			} else {
				printf("Unexpected token: %c\n", *temp_c2);
				exit(1);
			}
		} else if(current_token.type == CONTROL && current_token.sub_type == OPENPARENTHESES){
			add_token(token_list, current_token, token_index, token_length);
			parse_expression(c, token_list, token_index, token_length, (token) {.type = CONTROL, .sub_type = CLOSEPARENTHESES});
			temp_c2 = *c;
			next_token = get_token(c);

			while((next_token.type == CONTROL && next_token.sub_type == OPENPARENTHESES) || (next_token.type == OPERATOR && next_token.sub_type == ELEMENT)){
				if(next_token.type == CONTROL && next_token.sub_type == OPENPARENTHESES){
					add_token(token_list, next_token, token_index, token_length);
					skip_whitespace(c);
					if(**c == ')'){
						next_token = get_token(c);
						add_token(token_list, next_token, token_index, token_length);
					} else {
						while(*(*c - 1) != ')'){
							parse_expression(c, token_list, token_index, token_length, (token) {.type = CONTROL, .sub_type = COMMA});
						}
					}
					skip_whitespace(c);
					temp_c2 = *c;
					next_token = get_token(c);
				} else if(next_token.type == OPERATOR && next_token.sub_type == ELEMENT){
					add_token(token_list, next_token, token_index, token_length);
					skip_whitespace(c);
					parse_expression(c, token_list, token_index, token_length, (token) {.type = CONTROL, .sub_type = CLOSEBRACKET});
					skip_whitespace(c);
					temp_c2 = *c;
					next_token = get_token(c);
				}
			}
			
			if(next_token.type != OPERATOR && current_token.type != LITERAL && (next_token.type != closing_token.type || next_token.sub_type != closing_token.sub_type)){
				printf("Expected operator instead of: %c\n", *temp_c2);
				exit(1);
			} else if(next_token.type == OPERATOR){
				add_token(token_list, next_token, token_index, token_length);
			} else if(next_token.type == closing_token.type && next_token.sub_type == closing_token.sub_type){
				add_token(token_list, next_token, token_index, token_length);
				return;
			} else {
				printf("Unexpected token: %c\n", *temp_c2);
				exit(1);
			}
		} else if(current_token.type == OPERATOR && current_token.sub_type == MULTIPLY){
			current_token.type = UNARY;
			current_token.sub_type = DEREFERENCE;
			add_token(token_list, current_token, token_index, token_length);
		} else if(current_token.type == OPERATOR && current_token.sub_type == AND){
			current_token.type = UNARY;
			current_token.sub_type = REFERENCE;
			add_token(token_list, current_token, token_index, token_length);
		} else if(current_token.type == UNARY){
			add_token(token_list, current_token, token_index, token_length);
		} else {
			printf("Unexpected token: %c\n", *(*c - 1));
			exit(1);
		}
		skip_whitespace(c);
		if(!**c){
			printf("Expected closing token: ");
			print_token(closing_token);
			printf("\n");
			exit(1);
		}
		if(do_next_token){
			temp_c = *c;
			current_token = get_token(c);
		} else {
			do_next_token = 1;
		}
	}
}

void parse_statement(char **c, token **token_list, unsigned int *token_index, unsigned int *token_length){
	char *old_c;
	unsigned int old_token_index;
	token current_token;
	
	skip_whitespace(c);
	
	old_c = *c;
	old_token_index = *token_index;
	
	current_token = get_token(c);

	if(current_token.type == KEYWORD){
		if(current_token.sub_type == IF || current_token.sub_type == WHILE){
			add_token(token_list, current_token, token_index, token_length);
			skip_whitespace(c);
			current_token = get_token(c);
			if(current_token.type != CONTROL || current_token.sub_type != OPENPARENTHESES){
				printf("Expected opening token '('\n");
				exit(1);
			}
			add_token(token_list, current_token, token_index, token_length);
			parse_expression(c, token_list, token_index, token_length, (token) {.type = CONTROL, .sub_type = CLOSEPARENTHESES});
			skip_whitespace(c);
			parse_block(c, token_list, token_index, token_length);
		} else if(current_token.sub_type == ELSE){
			add_token(token_list, current_token, token_index, token_length);
			skip_whitespace(c);
			current_token = get_token(c);
			if(current_token.type != CONTROL || current_token.sub_type != OPENBRACES){
				printf("Expected opening token '{'\n");
				exit(1);
			}
			add_token(token_list, current_token, token_index, token_length);
			parse_block(c, token_list, token_index, token_length);
		} else if(current_token.sub_type == VAR){
			add_token(token_list, current_token, token_index, token_length);
			skip_whitespace(c);
			current_token = get_token(c);
			if(current_token.type != IDENTIFIER){
				printf("Expected identifier\n");
				exit(1);
			}
			add_token(token_list, current_token, token_index, token_length);
			skip_whitespace(c);
			current_token = get_token(c);
			if(current_token.type == OPERATOR && current_token.sub_type == ELEMENT){
				add_token(token_list, current_token, token_index, token_length);
				skip_whitespace(c);
				current_token = get_token(c);
				if(current_token.type != LITERAL || current_token.sub_type != INTEGER){
					printf("Expected integer literal\n");
					exit(1);
				}
				add_token(token_list, current_token, token_index, token_length);
				skip_whitespace(c);
				current_token = get_token(c);
				if(current_token.type != CONTROL || current_token.sub_type != CLOSEBRACKET){
					printf("Expected ']' token\n");
					exit(1);
				}
				add_token(token_list, current_token, token_index, token_length);
				skip_whitespace(c);
				current_token = get_token(c);
			}
			if(current_token.type != CONTROL || current_token.sub_type != SEMICOLON){
				printf("Expected ';' token\n");
				exit(1);
			}
			add_token(token_list, current_token, token_index, token_length);
		} else if(current_token.sub_type == RETURN){
			add_token(token_list, current_token, token_index, token_length);
			skip_whitespace(c);
			parse_expression(c, token_list, token_index, token_length, (token) {.type = CONTROL, .sub_type = SEMICOLON});
		}
	} else {
		if(current_token.type == IDENTIFIER){
			free(current_token.string_value);
		}

		*token_index = old_token_index;
		*c = old_c;
		parse_expression(c, token_list, token_index, token_length, (token) {.type = CONTROL, .sub_type = SEMICOLON});
	}
}

void parse_block(char **c, token **token_list, unsigned int *token_index, unsigned int *token_length){
	token current_token;
	
	skip_whitespace(c);
	current_token = get_token(c);
	if(current_token.type != CONTROL || current_token.sub_type != OPENBRACES){
		printf("Expected '{' token\n");
		exit(1);
	}
	
	add_token(token_list, current_token, token_index, token_length);
	skip_whitespace(c);

	while(**c != '}'){
		parse_statement(c, token_list, token_index, token_length);
		skip_whitespace(c);
		if(**c == (char) 0){
			printf("Expected '}' token\n");
			exit(1);
		}
	}

	current_token = get_token(c);
	add_token(token_list, current_token, token_index, token_length);
}

void parse_program(char **c, token **token_list, unsigned int *token_index, unsigned int *token_length){
	token current_token;
	skip_whitespace(c);

	while(**c){
		current_token = get_token(c);
		if(current_token.type != KEYWORD || current_token.sub_type != VAR){
			printf("Expected 'var' keyword\n");
			exit(1);
		}

		add_token(token_list, current_token, token_index, token_length);
		skip_whitespace(c);
		current_token = get_token(c);
		if(current_token.type != IDENTIFIER){
			printf("Expected identifier\n");
			exit(1);
		}
		add_token(token_list, current_token, token_index, token_length);
		skip_whitespace(c);
		current_token = get_token(c);
		if(current_token.type == OPERATOR && current_token.sub_type == ELEMENT){
			add_token(token_list, current_token, token_index, token_length);
			skip_whitespace(c);
			current_token = get_token(c);
			if(current_token.type != LITERAL || current_token.sub_type != INTEGER){
				printf("Expected integer literal\n");
				exit(1);
			}
			add_token(token_list, current_token, token_index, token_length);
			skip_whitespace(c);
			current_token = get_token(c);
			if(current_token.type != CONTROL || current_token.sub_type != CLOSEBRACKET){
				printf("Expected ']' token\n");
				exit(1);
			}
			add_token(token_list, current_token, token_index, token_length);
			skip_whitespace(c);
			current_token = get_token(c);
			if(current_token.type != CONTROL || current_token.sub_type != SEMICOLON){
				printf("Expected ';' token\n");
				exit(1);
			}
			add_token(token_list, current_token, token_index, token_length);
		} else if(current_token.type == CONTROL && current_token.sub_type == OPENPARENTHESES){
			add_token(token_list, current_token, token_index, token_length);
			skip_whitespace(c);
			current_token = get_token(c);
			while(**c && (current_token.type != CONTROL || current_token.sub_type != CLOSEPARENTHESES)){
				if(current_token.type != IDENTIFIER){
					printf("Expected identifier\n");
					exit(1);
				}
				add_token(token_list, current_token, token_index, token_length);
				skip_whitespace(c);
				current_token = get_token(c);
				if(current_token.type == CONTROL && current_token.sub_type == CLOSEPARENTHESES){
					add_token(token_list, current_token, token_index, token_length);
					break;
				} else if(current_token.type == CONTROL && current_token.sub_type == COMMA){
					add_token(token_list, current_token, token_index, token_length);
					skip_whitespace(c);
					current_token = get_token(c);
				} else {
					printf("Expected ',' or ')' tokens\n");
					exit(1);
				}
			}
			add_token(token_list, current_token, token_index, token_length);
			skip_whitespace(c);
			parse_block(c, token_list, token_index, token_length);
		} else if(current_token.type == CONTROL && current_token.sub_type == SEMICOLON){
			add_token(token_list, current_token, token_index, token_length);
		}
		skip_whitespace(c);
	}
	add_token(token_list, (token) {.type = END}, token_index, token_length);
}

