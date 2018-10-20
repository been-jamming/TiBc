#include "linked_list.h"
#include <stdlib.h>

linked_list *create_linked_list(void *value){
	linked_list *output;
	output = malloc(sizeof(linked_list));
	output->value = value;
	output->next = (linked_list *) 0;
	output->previous = (linked_list *) 0;
	return output;
}

void add_linked_list(linked_list **list, linked_list *element){
	(*list)->next = element;
	element->previous = *list;
	*list = element;
}

