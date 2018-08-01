#include <stdint.h>

/*
Extremely simple cpu emulator created for my B compiler

4 registers:

Program counter (pc): Contains the address of the current instruction. Incremented after instruction executes (except for when JMP is called).
Stack pointer (sp): Contains the address of the stack. Incremented after push, decremented before pop.
Accumulator (a): Register upon which arithmetic operations operate.
Data (d): Register from which arithmetic operations operate.

** instructions:
NOP: No operation.
	Instruction: 0x00
LOD addr: Load from memory address pointed to by the memory address addr into d.
	Instruction: 0x02
LODI addr: Load from memory address addr into d.
	Instruction: 0x03
ADD: Add the values in a and d and store the result in a.
	Instruction: 0x04
SUB: Subtract the value in d from the value in a and store the result in a.
	Instruction: 0x06
MUL: Multiply the values in a and d and store the result in a.
	Instruction: 0x08
DIV: Divide the value in a by the value in d and store the result in a.
	Instruction: 0x0A
MULU: Multiply the values in a and d assuming they are unsigned and store the result in a.
	Instruction: 0x0C
DIVU: Divide the value in a by the value in d assuming they are unsigned and store the result in a.
	Instruction: 0x0E
AND: And the value in a with the value in d and store the result in a.
	Instruction: 0x10
OR: Or the value in a with the value in d and store the result in a.
	Instruction: 0x12
NOT: Not the value in a and store the result in a.
	Instruction: 0x14
XOR: XOR the value in a with the value in d and store the result in a.
	Instruction: 0x16
RSH: Right shift the value in a by the value in d and store the result in a.
	Instruction: 0x18
LSH: Left shift the value in a by the value in d and store the result in a.
	Instruction: 0x1A
SWP: Swap the value in a with the value in d.
	Instruction: 0x1C
LSP addr: Load the value from memory address addr into sp.
	Instruction: 0x1E
SSP addr: Store the value of sp into memory address addr.
	Instruction: 0x20
POP: Decrement sp by 2, then store the value pointed to by sp into d.
	Instruction: 0x22
PUSH: Store the value in a into the value pointed to by sp, then increment sp by 2.
	Instruction: 0x24
JMP: Store the value of d into pc before the next instruction is executed (jump to the address stored int d).
	Instruction: 0x26
STA addr: Store the value of a into the address pointed to by addr and set a to 0.
	Instruction: 0x28
STAI addr: Store the value of a directly into the address addr and set a to 0.
	Instruction: 0x29
STD addr: Store the value of d into the address pointed to by addr.
	Instruction: 0x2A
STDI addr: Store the value of d directly into the address addr.
	Instruction: 0x2B
*/

typedef struct cpu cpu;

struct cpu{
	unsigned int clock_cycle;
	uint16_t pc;
	uint16_t sp;
	uint16_t a;
	uint16_t d;
	uint8_t memory[1<<16];
};

void cycle(cpu *c){
	uint8_t op_code;
	uint16_t mem_address;
	uint16_t indirect;
	op_code = c->memory[c->pc]
	mem_address = ((uint16_t) c->memory[c->pc + 1])<<8 + c->memory[c->pc + 2];
	indirect = ((uint16_t) c->memory[mem_address])<<8 + c->memory[mem_address + 1];
	select(op_code){
		case 0x00:
			c->pc++;
			break;
			
		case 0x02:
			c->d = ((uint16_t) c->memory[mem_address])<<8 + c->memory[mem_address + 1];
			c->pc += 3;
			break;
		
		case 0x03:
			c->d = ((uint16_t) c->memory[indirect])<<8 + c->memory[indirect + 1];
			c->pc += 3;
			break;
		
		case 0x04:
			c->a += c->d;
			c->pc += 1;
			break;
		
		case 0x06:
			c->a -= c->d;
			c->pc += 1;
			break;
		
		case 0x08:
			c->a = ((int16_t) c->a)*((int16_t) c->d);
			c->pc += 1;
			break;
		
		case 0x0A:
			c->a = ((int16_t) c->a)/((int16_t) c->d);
			c->pc += 1;
			break;
		
		case 0x0C:
			c->a *= c->d;
			c->pc += 1;
			break;
		
		case 0x0E:
			c->a /= c->d;
			c->pc += 1;
			break;
			
	}
	
	c->clock_cycle++;
}