#include <stdlib.h>
#include "../libfractal/fractal.h"

typedef struct node {
  struct fractal *value;
  struct node *next;
} node;

size_t length(node *list);

int push(node **list, struct fractal * value);

struct fractal * pop(node **list);

void free_list(node *list);
