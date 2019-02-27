//How big are ints/pointers? This is implementation dependent, so you can set the size of data using the macro VARSIZE.
//
//If VARSIZE is n, then each var is 2^n bytes.
#define VARSIZE 2

char error_message[512];

unsigned int current_line;

void safe_exit();
