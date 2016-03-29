#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "libfractal/fractal.h"

/**TODO Architecture :
Autant de thread "producteurs" que de fichiers d'entrées, qui lisent les entrée, créent des struct fractales et les stockent dans un buffer protégé par un sémaphore.
Autant de thread de calcul que l'option --nthread, qui prennent dans le buffer les struct fract et les calculent via julia
Les struct fract "remplies" vont dans un autre buffer (aussi protégé par un sémaphore), auquel accède un thread de moyenne qui dédage les fractales moins
bonnes au fur et à mesure, afin de sortir le bon fichier bmp (si l'option -d est là, juste sortit le fichier puis dégager la fractale sans regarder la moyenne)
Dans la structure fractale, rajouter un int moyenne, où sera directement stockée la moyenne (elle est donc calculée dans lee thread qui rempli la fractale)
Faire un bon makefile !
*/
int flagDetail;

int main(int argc, char const *argv[]) {
	if(argc <2){
		fprintf(stderr, "Il n'y a pas assez d'arguments donnés, il faut au moins stipuler un fichier d'où tirer les données des fractales !");
		exit(EXIT_FAILURE);
	}
	int nbrArg=1;
	char const *arg=argv[nbrArg];
	
	if(strcmp(arg, "-d")==0){
		flagDetail=1;
		printf("Option -d\n");
		nbrArg++;
	}
	else{
		flagDetail=0;
	}
	arg=argv[nbrArg];
	char* code = strstr(arg, "--");
	if(code!=NULL){
		int nbr=atoi(code+2);
		printf("Nombre : %i\n", nbr);
		nbrArg++;
	}

	while(nbrArg<argc-1){//Tant qu'on a des arguments à lire (ici, ce sont des fichiers + s'arrêter un avant la fin pour l'outpput)
		char const* fichier=argv[nbrArg];
		//TODO : le cas où c'est un "-" pour dire qu'on veut la sortie au clavier
		FILE* toRead=NULL;
		toRead=fopen(fichier, "r");
		if(toRead != NULL){
			printf("fichier ouvert\n");
			int comm =fgetc(toRead);
			if(comm=='#'){//On regarde si ce n'est pas une ligne en commentaire
				//Passer à la ligne suivante si c'est le cas
			}
			else{//Sinon, on lit
				fseek(toRead, -1, SEEK_CUR);
				char chaine[30]="";
				printf("Avant le while\n");
				while (fgets(chaine, 30, toRead) != NULL){ // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)
					printf("%s\n", chaine);
				}
			}
			fclose(toRead);//Fermer le fichier
		}
		else{
			fprintf(stderr, "Impossible de lire le fichier %s, il a été ignoré\n", fichier);
		}
		nbrArg++;
	}
    return 0;
}
