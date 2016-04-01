#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "libfractal/fractal.h"
#include "stack.c"

/**TODO Architecture :
Autant de thread "producteurs" que de fichiers d'entrées, qui lisent les entrée, créent des struct fractales et les stockent dans un buffer protégé par un sémaphore.
Autant de thread de calcul que l'option --nthread, qui prennent dans le buffer les struct fract et les calculent via julia
Les struct fract "remplies" vont dans un autre buffer (aussi protégé par un sémaphore), auquel accède un thread de moyenne qui dédage les fractales moins
bonnes au fur et à mesure, afin de sortir le bon fichier bmp (si l'option -d est là, juste sortit le fichier puis dégager la fractale sans regarder la moyenne)
Dans la structure fractale, rajouter un int moyenne, où sera directement stockée la moyenne (elle est donc calculée dans lee thread qui rempli la fractale)
Faire un bon makefile !
*/

#define N 100// taille max des buffers (plus forcément utile avec les stacks
static pthread_mutex_t mutex1;// mutex du buffer lecture - calcul
static sem_t empty1;// si > 0 alors il y a des places libres dans le buffer1
static sem_t full1;// si > 0 alors il y a des données dans le buffer1
node * buffer1;//Buffer pour les fractales "vides"
static pthread_mutex_t mutex2;//Mutex du buffer calcul - moyenne
static sem_t empty2;
static sem_t full2;
node * buffer2;//Buffer pour les fractales calculées et la moyenne
struct fractal *bestAv;//Variable où stocker la meilleure fractale
int flagDetail;
int maxThreads;
int nbrArg=1; //Lecture des arguments

void initFirst(){
	int err=pthread_mutex_init( &mutex1, NULL);
	if(err!=0)
		fprintf(stderr,"ERREUR : pthread_mutex_init (premier buffer)\n");

	int ret1 = sem_init(&empty1, 0, N);
	if(ret1!=0){
		fprintf(stderr, "ERREUR : Création de empty1 (premier buffer)\n");
	}

	int ret2 = sem_init(&full1, 0, 0);
	if(ret2!=0){
		fprintf(stderr, "ERREUR : Création de full1 (premier buffer)\n");
	}
}

void initSecond(){
	int err=pthread_mutex_init(&mutex2, NULL);
	if(err!=0)
		fprintf(stderr,"ERREUR : pthread_mutex_init (second buffer)\n");

	int ret1 = sem_init(&empty2, 0, N);
	if(ret1!=0){
		fprintf(stderr, "ERREUR : Création de empty2 (second buffer)\n");
	}

	int ret2 = sem_init(&full2, 0, 0);
	if(ret2!=0){
		fprintf(stderr, "ERREUR : Création de full2 (second buffer)\n");
	}
	bestAv=fractal_new("empty", 1, 1, 0.0, 0.0);
}

struct fractal* compute(char* str){
	const char *delim = " ";
	char* name = strsep(&str, delim);
	if(str==NULL){//Si à ce point str vaut NULL, c'est que la chaine ne contenait que le nom
		fprintf(stderr, "Erreur, la fractale : %s n'est pas formatée correctement. Elle a été ignorée", str);
		return NULL;
	}
	int width = atoi(strsep(&str, delim));
	if(str==NULL || width<=0){//Il manque des arguments ou la longueur est invalide
		fprintf(stderr, "Erreur, la fractale : %s n'est pas formatée correctement. Elle a été ignorée", str);
		return NULL;
	}
	int height = atoi(strsep(&str, delim));
	if(str==NULL || height<=0){//Il manque des arguments ou la hauteur est invalide
		fprintf(stderr, "Erreur, la fractale : %s n'est pas formatée correctement. Elle a été ignorée", str);
		return NULL;
	}
	char * temp =strsep(&str, delim);
	double a = strtod(temp, NULL);
	if(str==NULL || a>1.0 || a<-1.0){//Il manque un argument ou a n'est pas dans les bonnes bornes
		fprintf(stderr, "Erreur, la fractale : %s n'est pas formatée correctement. Elle a été ignorée", str);
		return NULL;
	}
	double b = strtod(str, NULL);
	if(b>1.0 || b<-1.0){//b n'est pas dans les bonnes bornes
		fprintf(stderr, "Erreur, la fractale : %s n'est pas formatée correctement. Elle a été ignorée", str);
		return NULL;
	}
	printf("Envoyé : name = %s, height = %i, width = %i, a = %f, b = %f\n", name, height, width, a, b);
	struct fractal *result = fractal_new(name, width, height, a, b);
	return result;
}

void insert(struct fractal *fract){
	printf("Reçu : %s\n", fractal_get_name(fract));
	sem_wait(&empty1);
    pthread_mutex_lock(&mutex1);
    int res = push(&buffer1, *fract);
    if(res != 0)
		printf("Impossible d'ajouter la fractale au buffer1\n");
    printf("Fractale ajoutée (%i))\n", res);
    pthread_mutex_unlock(&mutex1);
    sem_post(&full1);
}

void * producer(void *arg){
	char * fichier = (char*) arg;
	printf("Hello ! I'm a producer !\n");
	FILE* toRead=NULL;
	fflush(stdout);
	toRead=fopen(fichier, "r");
	if(toRead != NULL){
		printf("fichier ouvert\n");
		char chaine[50]="";
		while(fgets(chaine, 50, toRead) != NULL){ // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)
			chaine[strlen(chaine)-1] = '\0';
			char comm = *chaine;
			if(comm!='#') { //On regarde si ce n'est pas une ligne en commentaire
				printf("Chaine : %s\n", chaine);
				struct fractal *toAdd=compute(chaine);
				if(toAdd!=NULL)
					insert(toAdd);
			}
		}
		fclose(toRead);//Fermer le fichier
		printf("fichier fermé\n");
		if(strcmp(fichier, "stdin.txt")==0){//Si c'est le fichier tampon où l'utilisateur a entré ses données, le supprimer
			printf("Suppression de stdin.txt\n");
			remove(fichier);
		}
	}
	else{
		fprintf(stderr, "Impossible de lire le fichier %s, il a été ignoré\n", fichier);
	}
	printf("Fin d'un producer\n");
	fflush(stdout);
	pthread_exit(NULL);//pthread_create veut absolument un return
}

void * consumer(){
	while(1){
	 sem_wait(&full1); // attente d'un slot rempli
	 pthread_mutex_lock(&mutex1);
	 printf("Consommateur consomme !\n");
	 struct fractal toFill=pop(&buffer1);
	 toFill=fractal_fill(&toFill);
	 printf("Fractale remplie\n");
	 pthread_mutex_unlock(&mutex1);
	 sem_post(&empty1); // il y a un slot libre en plus
	 if(flagDetail){
		write_bitmap_sdl(&toFill, strcat(fractal_get_name(&toFill), ".bmp"));
	}
	else{
		sem_wait(&empty2);//On attend une place sur le buffer2
		pthread_mutex_lock(&mutex2);//Quand on l'a, on lock
		int res=push(&buffer2, toFill);//On met la fractale dans le buffer2
		if(res != 0)
			printf("Impossible d'ajouter la fractale au buffer2\n");
		pthread_mutex_unlock(&mutex2);//On delock
		sem_post(&full2);//On signale qu'une valeur est présente
	}
 }
	 pthread_exit(NULL);//pthread_create veut absolument un return
 }

 void * average(){
	 while(1){
		 sem_wait(&full2);//On attend qu'il y ait quelque chose dans le buffer
		 pthread_mutex_lock(&mutex2);//On lock
		 struct fractal test = pop(&buffer2);//On prend la fractale;
		 printf("Test de moyenne ! Fractale précédente (%s) : %f, fractale actuelle (%s) : %f\n", fractal_get_name(bestAv), fractal_get_av(bestAv), fractal_get_name(&test), fractal_get_av(&test));
		 if(fractal_get_av(&test)>fractal_get_av(bestAv)){//Si la fractale est meilleure que celle précédement en mémoire
			 printf("Fractale update !\n");
			 fractal_free(bestAv);
			 bestAv=&test;//Free bestAv avant ?
		 }
		 pthread_mutex_unlock(&mutex2);
		 sem_post(&empty2);
	 }
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
	} else {
		maxThreads = 10;
	}
	initFirst();
	initSecond();
	printf("Nombre de producer : %i\n", argc-nbrArg-1);
	pthread_t threads[argc-nbrArg-1];
	int nthread = 0;
	while(nbrArg<argc-1){//Tant qu'on a des arguments à lire (ici, ce sont des fichiers + s'arrêter un avant la fin pour l'output)
		char* fichier=argv[nbrArg];
		printf("Argument lu : %s\n", fichier);
		if(strcmp(fichier, "-")==0){
			FILE* std = NULL;
			std = fopen("stdin.txt", "w");//Créons un fichier tampon où stocker les entrées au clavier (w efface l'éventuel contenu, pour s'assurer qu'il est vide au départ)
			int stop=0;
			while(stop != 1) {
				char saisie[50];
				printf("Entrez une fractale (Nom hauteur largeur a b) : \n");
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
		}
		pthread_t th = NULL;
		threads[nthread] = th;
		nthread++;
		int res = pthread_create(&th,NULL, *producer,(void *)fichier);
		if(res!=0)
			printf("Problème de producteur\n");
		nbrArg++;
	}

	pthread_t threadsC[maxThreads];
	int i;
	for(i = 0; i < maxThreads; i++){
		pthread_t th = NULL;
		threadsC[i] = th;
		int res = pthread_create(&th, NULL, *consumer, NULL);
		if (res != 0)
			printf("Problème de consommateur\n");
	}
	if(flagDetail!=1){
		printf("Lancement du thread de moyenne\n");
		pthread_t th = NULL;
		int res = pthread_create(&th,NULL, *average,NULL);
		if(res != 0)
			printf("Problème de thread de moyenne\n");
	}
	while (1) {
		//DEBUG
	}
	void * ret;
	for (int i = 0; i < nthread; i++) {
		pthread_join(threads[i], &ret);
	}
    return 0;
}
