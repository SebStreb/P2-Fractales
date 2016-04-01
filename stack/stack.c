#include "stack.h"

size_t length(node *list) {
	size_t result = 0;
	node *temp = list;
	while(temp != NULL){
		temp = temp->next;
		result++;
	}
	return result;
}

int push(node **list, struct fractal * value) {
	node *toAdd= (node*) malloc(sizeof(node));
	if (toAdd == NULL) {
		free(toAdd);
		return -1;
    }
	toAdd->next = *list;
	toAdd->value = value;
	*list = toAdd;
	return 0;

}

struct fractal * pop(node **list) {
	node *tofree = *list;
	struct fractal *save = tofree->value;
	*list = tofree->next;
	free(tofree);
	return save;
}

void free_list(node *list) {
	while (list != NULL) {
		node *current = list;
		list = list->next;
		free(current);
	}
	free(list);
}
