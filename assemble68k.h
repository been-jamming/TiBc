typedef struct file_bits file_bits;

struct file_bits{
	FILES *fsp;
	unsigned char current_bitnum;
	unsigned char current_byte;
};
