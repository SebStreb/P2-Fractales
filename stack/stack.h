#include <stdlib.h>
#include "../libfractal/fractal.h"

typedef struct node {
  struct fractal *value;
  struct node *next;
} node;

size_t stack_length(node *list);

int stack_push(node **list, struct fractal * value);

struct fractal * stack_pop(node **list);

void free_list(node *list);
