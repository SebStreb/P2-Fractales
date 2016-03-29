#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libfractal/fractal.h"

/**TODO Architecture :
Autant de thread "producteurs" que de fichiers d'entrées, qui lisent les entrée, créent des struct fractales et les stockent dans un buffer protégé par un sémaphore.
Autant de thread de calcul que l'option --nthread, qui prennent dans le buffer les struct fract et les calculent via julia
Les struct fract "remplies" vont dans un autre buffer (aussi protégé par un sémaphore), auquel accède un thread de moyenne qui dédage les fractales moins
bonnes au fur et à mesure, afin de sortir le bon fichier bmp (si l'option -d est là, juste sortit le fichier puis dégager la fractale sans regarder la moyenne)
Dans la structure fractale, rajouter un int moyenne, où sera directement stockée la moyenne (elle est donc calculée dans lee thread qui rempli la fractale)
Faire un bon makefile !
*/

int main(int argc, char const *argv[]) {
    struct fractal * f = fractal_new("test1", 100, 30, 0.5, 0.4);
    printf("%i\n", fractal_get_value(f, 50, 10));
    return 0;
}
