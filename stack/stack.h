#include <stdlib.h>
#include "../libfractal/fractal.h"

/*   Interface pour représenter un stack   */

/*
 * Noeud dd'une liste
 */
typedef struct node {
  struct fractal *value;
  struct node *next;
} node;

/*
 * Donne la taille d'une liste
 *
 * @list: une liste
 * @return: renvoie la taille de la liste
 */
size_t stack_length(node *list);

/*
 * Met une valeur dans une liste
 *
 * @list: une liste
 * @value: valeur à mettre dans la liste
 * @return: 0 si pas d'erreur, -1 sinon
 */
int stack_push(node **list, struct fractal * value);

/*
 * Récupère la dernière valeur d'une liste
 *
 * @list: une liste
 * @return: la dernière valeur mise dans la liste
 */
struct fractal * stack_pop(node **list);

/*
 * Libère une liste
 *
 * @list: une liste
 */
void free_list(node *list);
