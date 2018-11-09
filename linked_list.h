#ifndef INCLUDE_LINKED_LIST
#define INCLUDE_LINKED_LIST
typedef struct linked_list linked_list;

struct linked_list{
	void *value;
	linked_list *next;
	linked_list *previous;
};

linked_list *create_linked_list(void *value);

void add_linked_list(linked_list **list, linked_list *element);
#endif

