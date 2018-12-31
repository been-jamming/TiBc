#ifndef INCLUDE_PARSE
#define INCLUDE_PARSE
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
	#define BITAND 19
	#define BITOR 20
	#define BITXOR 21
	#define BITNOT 22
	#define ELEMENT 23
	#define NOTEQUALS 24

//Control

#define CONTROL 3

//sub_types
	#define OPENBRACES 1
	#define CLOSEBRACES 2
	#define OPENPARENTHESES 3
	#define CLOSEPARENTHESES 4
	#define SEMICOLON 5
	#define COMMA 6
	#define CLOSEBRACKET 7

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
	#define NEGATE 3
	#define NOT 4

//Identifier

#define IDENTIFIER 6

//End of file

#define END 7

#define RUNFUNCTION 8

typedef struct token token;

//Token definition.
//Tokens encompass LITERALS, OPERATORS, CONTROL CHARACTERS, KEYWORDS, UNARY OPERATORS, and IDENTIFIERS
//
//Each token has a type and a sub_type defined as one of the values at the beginning of this file.
//The type alone gives enough information to handle the following tokens.
//The sub_type gives information required to compile the tokens into an abstract syntax tree.
struct token{
	unsigned char type;
	unsigned char sub_type;
	union{
		int int_value;
		char char_value;
		char *string_value;
	};
};

//Determines if a character is a letter.
unsigned char is_alpha(unsigned char c);

//Determines if a character is a digit.
unsigned char is_digit(unsigned char c);

//Increments *c until **c is not a whitespace character.
void skip_whitespace(char **c);

//Interprets an integer string starting at *c and returns the integer value.
//Modifies *c such that it points to the next character after the integer.
int get_integer(unsigned char **c);

//Returns a new string containing the identifier name pointed to by *c.
//Modifies *c such that it points to the next character after the identifier.
char *get_identifier(char **c);

//Prints a token for debugging purposes.
void print_token(token t);

//Gets the first token pointed to by *c.
//Modifies *c such that it points to the next character after that token.
token get_token(char **c);

//Adds a token to the current list of tokens
void add_token(token **token_list, token t, unsigned int *token_index, unsigned int *token_length);

//Validates and tokenizes an expression pointed to by *c.
//Modifies *c such that it points to the next character after that expression.
void parse_expression(char **c, token **token_list, unsigned int *token_index, unsigned int *token_length, token closing_token);

//Validates and tokenizes a statement pointed to by *c.
//Modifies *c such that it points to the next character after that statement.
void parse_statement(char **c, token **token_list, unsigned int *token_index, unsigned int *token_length);

//Validates and tokenizes a block of statements pointed to by *c.
//Modifies *c such that it points to the next character after that block.
void parse_block(char **c, token **token_list, unsigned int *token_index, unsigned int *token_length);

//Validates and tokenizes a full program pointed to by *c.
void parse_program(char **c, token **token_list, unsigned int *token_index, unsigned int *token_length);
#endif

