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
	#define ELEMENT 6

//Control

#define CONTROL 3

//sub_types
	#define OPENBRACES 1
	#define CLOSEBRACES 2
	#define OPENPARENTHESES 3
	#define CLOSEPARENTHESES 4
	#define SEMICOLON 5
	#define COMMA 6

//Key words

#define KEYWORD 4

//sub_types

	#define IF 1
	#define ELSE 2
	#define WHILE 3
	#define VAR 4
	#define RETURN 5

//Unary operator

#define UNARY 5

//sub_types

	#define DEREFERENCE 1
	#define REFERENCE 2

//Identifier

#define IDENTIFIER 6

//End of file

#define END 7

#define RUNFUNCTION 8

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

unsigned char is_alpha(unsigned char c);

unsigned char is_digit(unsigned char c);

void skip_whitespace(char **c);

int get_integer(unsigned char **c);

char *get_identifier(char **c);

void print_token(token t);

token get_token(char **c);

void add_token(token **token_list, token t, unsigned int *token_index, unsigned int *token_length);

void parse_expression(char **c, token **token_list, unsigned int *token_index, unsigned int *token_length, token closing_token);

void parse_statement(char **c, token **token_list, unsigned int *token_index, unsigned int *token_length);

void parse_block(char **c, token **token_list, unsigned int *token_index, unsigned int *token_length);

void parse_program(char **c, token **token_list, unsigned int *token_index, unsigned int *token_length);

