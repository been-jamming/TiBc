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

