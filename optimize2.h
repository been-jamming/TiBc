/*
 * B compiler pseudo-assembly optimizations
 *
 * by Ben Jones
 * 
 * 2/16/2019
 */

unsigned char OPTIMIZE2_CONTINUE;

#define UNDEFINED_VALUE 0
#define EQUALS_VALUE 1
#define BETWEEN_VALUE 2
#define NOTEQUALS_VALUE 3

/*
 * Stores the possible values of a register
 *
 * Handle when the register either:
 * 	1. Has a determined value
 * 	2. Is between two values
 * 	3. Is not a determined value
 * 	4. Has no determinable value
 */

typedef struct reg_value reg_value;

struct reg_value{
	unsigned char value_type;
	unsigned long int register_state;
	union{
		unsigned int value;
		struct{
			unsigned int min_value;
			unsigned int max_value;
		};
		unsigned int not_value;
	};
};

/*
 * Store the current possible values of all of the registers
 */

typedef struct reg_values reg_values;

struct reg_values{
	reg_value *values;
	unsigned int num_registers;
};

void sweep_instructions(instruction **instructions);

