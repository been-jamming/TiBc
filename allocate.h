#ifndef INCLUDE_ALLOCATE
#define INCLUDE_ALLOCATE
typedef struct reg_list reg_list;

struct reg_list{
	unsigned int num_regs;
	unsigned int *regs;
	unsigned int *positions;
	unsigned int current_reg;
};

reg_list *create_reg_list(unsigned int num_regs);

void free_reg_list(reg_list *r);

unsigned int allocate_register(reg_list *r);

void free_register(reg_list *r, unsigned int reg);

void allocate_expression(expression *e, reg_list *regs, dictionary *local_space);

void allocate_statement(statement *s, reg_list *r, dictionary *local_space);
#endif

