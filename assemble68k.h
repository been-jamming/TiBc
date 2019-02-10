enum OPCODE{
	MOVE,
	NEG,
	NOT,
	JSR,
	JMP,
	MOVEM,
	ADD,
	SUB,
	CMP,
	EOR,
	AND,
	OR
};

//The code for the address mode is represented as the symbol's value
//This is done for convenience in the write_instructions68k routine
#define DATAREG 0
#define ADDRREG 1
#define ADDR 2
#define ADDRPI 3
#define ADDRPD 4
#define ADDRDISP 5
#define PCDISP 7

//The following three addressing modes actually have a code of 7
#define ABSSHORT 8
#define ABSLONG 9
#define IMMEDIATE 10

typedef unsigned char address_mode;

typedef struct instruction68k instruction68k;

struct instruction68k{
	OPCODE op;
	address_mode address1;
	address_mode address2;
	union{
		uint32_t immediate1;
		unsigned char reg1;
	};
	union{
		uint32_t immediate2;
		unsigned char reg2;
	};
	uint32_t displacement1;
	uint32_t displacement2;
};

typedef struct file_bits file_bits;

struct file_bits{
	FILES *fsp;
	unsigned char current_bitnum;
	unsigned char current_byte;
};

typedef struct label_entry label_entry;

struct label_entry{
	unsigned char label_seen;
	union{
		label_pointer *label_pointers;
		unsigned long int address;
	};
};

typedef struct label_pointer label_pointer;

struct label_pointer{
	unsigned int file_position;
	label_pointer *next;
};

