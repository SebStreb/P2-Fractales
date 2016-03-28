#include <stdlib.h>
#include "fractal.h"

struct fractal *fractal_new(int width, int height, double a, double b)
{
    struct fractal * new = malloc(sizeof(struct fractal));
    new->width = width;
    new->height = height;
    new->a = a;
    new->b = b;
    new->val = malloc(width*height*sizeof(int));
    for (size_t i = 0; i < (size_t)width*height; i++) {
        fractal_compute_value(new, width, height);
    }
    return new;
}

void fractal_free(struct fractal *f)
{
    free(f->val);
    free(f);
}

int fractal_get_value(const struct fractal *f, int x, int y)
{
    int index = fractal_get_width(f)*x + y;
    return *(f->val)+index;
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
