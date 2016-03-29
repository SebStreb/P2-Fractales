#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libfractal/fractal.h"

int main(int argc, char const *argv[]) {
    struct fractal * f = fractal_new(200, 100, 0.1, 0.2);
    printf("%i\n", fractal_get_value(f, 50, 50));
    return 0;
}
