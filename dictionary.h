typedef struct dictionary dictionary;

struct dictionary{
	unsigned char bits;
	dictionary *next_chars[8];
	void *value;
};

dictionary create_dictionary(void *value);

void *read_dictionary(dictionary dict, unsigned char *string, unsigned char offset);

void write_dictionary(dictionary *dict, unsigned char *string, void *value, unsigned char offset);

