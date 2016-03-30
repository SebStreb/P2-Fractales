#include <stdlib.h>
#include <string.h>
#include "fractal.h"

struct fractal *fractal_new(const char *name, int width, int height, double a, double b)
{
    struct fractal * new = malloc(sizeof(struct fractal));
    char * n = malloc(strlen(name)+1);
    strcpy(n, name);
    new->name = n;
    new->width = width;
    new->height = height;
    new->a = a;
    new->b = b;
    new->val = calloc(width*height, sizeof(int));//Calloc pour être sûr d'avoir 0
    return new;
}

struct fractal fractal_fill(struct fractal* f){
    int x, y;
    for (x = 0; x < fractal_get_width(f); x++) {
        for (y = 0; y < fractal_get_height(f); y++) {
            fractal_compute_value(f, x, y);
        }
    }
    return *f;
}

void fractal_free(struct fractal *f)
{
    free(f->name);
    free(f->val);
    free(f);
}

int fractal_get_value(const struct fractal *f, int x, int y)
{
    int index = fractal_get_width(f)*x + y;
    return *((f->val)+index);
}

void fractal_set_value(struct fractal *f, int x, int y, int val)
{
    int index = fractal_get_width(f)*x + y;
    *((f->val)+index) = val;
}

int fractal_get_width(const struct fractal *f)
{
    return f->width;
}

int fractal_get_height(const struct fractal *f)
{
    return f->height;
}

double fractal_get_a(const struct fractal *f)
{
    return f->a;
}

double fractal_get_b(const struct fractal *f)
{
    return f->b;
}
