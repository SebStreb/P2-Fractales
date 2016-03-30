#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
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
	
#define N 100                   // taille des buffer
static pthread_mutex_t mutex1;  // mutex du buffer lecture - calcul
static sem_t empty1;            // si > 0 alors il y a des places libres dans le buffer1
static sem_t full1;             // si > 0 alors il y a des données dans le buffer1
struct fractal buffer[N];
int firstEmpty=0;
int firstFull=-1;
int flagDetail;
int maxThreads;
int nbrArg=1; //Lecture des arguments

void initFirst(){
	int err=pthread_mutex_init( &mutex1, NULL);
	if(err!=0)
		fprintf(stderr,"pthread_mutex_init");

	int ret1 = sem_init(&empty1, 0, N);
	if(ret1!=0){
		fprintf(stderr, "Création de empty1");
	}

	int ret2 = sem_init(&full1, 0, 0);
	if(ret2!=0){
		fprintf(stderr, "Création de full1");
	}
}
struct fractal* compute(char* str){
	const char *delim = " ";
	char* name = strsep(&str, delim);
	int width = atoi(strsep(&str, delim));
	int height = atoi(strsep(&str, delim));
	double *a = (double*) strsep(&str, delim);
	double *b = (double*) str;
	struct fractal *result = fractal_new(name, width, height, *a, *b);
	return result;
	
}

void insert(struct fractal *fract){
	sem_wait(&empty1);
    pthread_mutex_lock(&mutex1);
    buffer[firstEmpty]=*fract;
    firstEmpty++;
    pthread_mutex_unlock(&mutex1);
    sem_post(&full1);
}

void * producer(void *arg){
	char * fichier = (char*) arg;
	printf("Hello ! I'm a producer ! My file is : %s\n", fichier);
	fflush(stdout);
		FILE* toRead=NULL;
		toRead=fopen(fichier, "r");
		if(toRead != NULL){
			printf("fichier ouvert\n");
			int comm=fgetc(toRead);
			if(comm=='#'){//On regarde si ce n'est pas une ligne en commentaire
				char chaine[50]="";
				fgets(chaine, 50, toRead);//Passer à la ligne suivante si c'est le cas
			}
			else{//Sinon, on lit
				fseek(toRead, -1, SEEK_CUR);//Ne pas oublier de reculer pour ne pas perdre le premier caractère
				char chaine[50]="";
				while (fgets(chaine, 50, toRead) != NULL){ // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)
					chaine[strlen(chaine)-1] = '\0';
					struct fractal *toAdd=compute(chaine);
					insert(toAdd);
				}
			}
			fclose(toRead);//Fermer le fichier
			if(strcmp(fichier, "stdin.txt")==0){//Si c'est le fichier tampon où l'utilisateur a entré ses données, le supprimer
				remove(fichier);
			}
		}
		else{
			fprintf(stderr, "Impossible de lire le fichier %s, il a été ignoré\n", fichier);
		}
		return NULL;//pthread_create veut absolument un return
	}


int main(int argc, char const *argv[]) {
	if(argc < 3){//Il faut au moins 3 arguments (le nom de base, un fichier d'entrée et un de sortie)
		fprintf(stderr, "Il n'y a pas assez d'arguments donnés, il faut au moins stipuler un fichier d'où tirer les données des fractales et un fichier de sortie!");
		exit(EXIT_FAILURE);
	}
	char const *arg=argv[nbrArg];

	if(strcmp(arg, "-d")==0){//Si on trouve le -d, mettre le flag à 1
		flagDetail=1;
		printf("Option -d\n");
		nbrArg++;//Un argument de lu, passons au suivant
	}
	else{
		flagDetail=0;
	}
	arg=argv[nbrArg];
	char* code = strstr(arg, "--");//Si on trouve les deux tirets, c'est maxThreads
	if(code!=NULL){
		int nbr=atoi(code+2);//Récupérer le nombre
		maxThreads=nbr;//Le stocker
		nbrArg++;//Passer à l'argument suivant
		arg=argv[nbrArg];//Permet de switcher les arguments mais moche
		if(strcmp(arg, "-d")==0){
			flagDetail=1;
			printf("Option -d\n");
			nbrArg++;
		}
	}
	
	initFirst();

	while(nbrArg<argc-1){//Tant qu'on a des arguments à lire (ici, ce sont des fichiers + s'arrêter un avant la fin pour l'output)
		char* fichier=argv[nbrArg];
		printf("Argument lu : %s\n", fichier);
		fflush(stdout);
		if(strcmp(fichier, "-")==0){
			FILE* std = NULL;
			std = fopen("stdin.txt", "w+");//Créons un fichier tampon où stocker les entrées au clavier (w+ efface l'éventuel contenu, pour s'assurer qu'il est vide au départ)
			int stop=0;
			while(stop != 1) {
				char saisie[50];
				printf("Entrez une fractale (Nom hauteur largeur a b) : \n");
				fflush(stdout);
				fgets (saisie, sizeof(saisie), stdin);//On ne modifie pas car on veut le retour à la ligne
				fputs(saisie, std);
				printf("Voulez-vous entrer une autre fractale ? (o/n)\n");
				char rep[10];
				fgets(rep, sizeof(rep), stdin);
				rep[strlen(rep)-1] = '\0';
				int res = strcmp(rep, "n");
				if(res==0) {
					stop=1;
				}
			}
			fclose(std);
			fichier="stdin.txt";
			nbrArg++;
		}
		pthread_t th;
		printf("Lancement du thread, fichier : %s\n", fichier);
		fflush(stdout);
		int res = pthread_create(&th,NULL, *producer,(void *)fichier);
		if(res!=0){
			printf("Problème de producteur\n");
			fflush(stdout);
		}
		nbrArg++;
		}
    return 0;
}
