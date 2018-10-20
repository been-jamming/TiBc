typedef struct dictionary dictionary;

struct dictionary{
	unsigned char bits;
	dictionary *next_chars[8];
	void *value;
};
