#include <stdlib.h>
#include <string.h>
#include <stdio.h>//Pour le dev
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
    new->average=-1.0;
    return new;
}

struct fractal *fractal_fill(struct fractal* f){
	fflush(stdout);
    int x, y;
    long sum=0;
    double count = 0.0;
    fflush(stdout);
    for (x = 0; x < fractal_get_width(f); x++) {
        for (y = 0; y < fractal_get_height(f); y++) {
			fflush(stdout);
            fractal_compute_value(f, x, y);
            sum=sum+fractal_get_value(f, x, y);
            count = count+1.0;
        }
    }
    double av = sum/count;
    printf("Moyenne de %s = %f\n", fractal_get_name(f), av);
    fractal_set_av(f, av);
    return f;
}

void fractal_free(struct fractal *f)
{
    free(f->name);
    free(f->val);
    free(f);
}

int fractal_get_value(const struct fractal *f, int x, int y)
{
    int index = fractal_get_height(f)*x + y;
    return *((f->val)+index);
}

char* fractal_get_name(const struct fractal *f){
	return f->name;
}

void fractal_set_value(struct fractal *f, int x, int y, int val)
{
	int index;
	if(x==0)
		index=y;
	else
		index = fractal_get_height(f)*(x-1) + y;
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

double fractal_get_av(const struct fractal *f){
	return f->average;
}

void fractal_set_av(struct fractal *f, double value){
	f->average=value;
}
