enum OPCODE{
	ORI,
	ANDI,
	SUBI,
	ADDI,
	EORI,
	CMPI,
	BTST,
	BCHG,
	BCLR,
	BSET,
	MOVEP,
	MOVEA,
	MOVE,
	NEGX,
	CLR,
	NEG,
	NOT,
	EXT,
	NBCD,
	SWAP,
	PEA,
	ILLEGAL,
	TAS,
	TST,
	TRAP,
	LINK,
	UNLK,
	RESET,
	NOP,
	STOP,
	RTE,
	RTS,
	TRAPV,
	RTR,
	JSR,
	JMP,
	MOVEM,
	LEA,
	CHK,
	ADDQ,
	SUBQ,
	Scc,
	DBcc,
	BRA,
	BSR,
	Bcc,
	MOVEQ,
	DIVU,
	DIVS,
	SBCD,
	OR,
	SUB,
	SUBX,
	SUBA,
	EOR,
	CMPM,
	CMP,
	CMPA,
	MULU,
	MULS,
	ABCD,
	EXG,
	AND,
	ADD,
	ADDX,
	ADDA,
	ASd,
	LSd,
	ROXd,
	ROd
};

enum SIZE{
	BYTE,
	WORD,
	LONG
};

enum ADDRESS_MODE{
	DATA_REG,
	ADDRESS_REG,
	ADDRESS,
	ADDRESS_PI,
	ADDRESS_PD,
	ADDRESS_DISP,
	ADDRESS_IDX,
	PC_DISP,
	PC_IDX,
	ABS_SHORT,
	ABS_LONG,
	IMMEDIATE
};

typedef struct operation operation;

struct operation{
	OPCODE op;
	SIZE size;
	ADDRESS_MODE mode1;
	ADDRESS_MODE mode2;
	unsigned char reg1;
	unsigned int displacement1;
	unsigned char data_reg1;
	unsigned char reg2;
	unsigned int displacement2;
	unsigned char data_reg2;
	unsigned long int immediate;
};

typedef struct file_bits file_bits;

struct file_bits{
	FILES *fsp;
	unsigned char current_bitnum;
	unsigned char current_byte;
};

typedef struct label_entry label_entry;

struct label_entry{
	unsigned char found;
	union{
		unsigned int address;
		unsigned int file_pointer;
	};
	label_entry *next;
};

