#ifndef INCLUDE_INCLUDE68K
#define INCLUDE_INCLUDE68K
variable *include(dictionary *global_space, char *var_name, unsigned char *data, unsigned int data_size);

void include68k(dictionary *global_space);

variable *included_putchar68k;

variable *included_getchar68k;

variable *included_setcolor68k;

variable *included_setpix68k;

variable *included_mul68k;

variable *included_div68k;
#endif
