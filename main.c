#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "libfractal/fractal.h"
#include "stack/stack.h"

#define N 100 //taille max des buffers (plus forcément utile avec les stacks

static pthread_mutex_t mutex1; //mutex du buffer lecture - calcul
static sem_t empty1; //si > 0 alors il y a des places libres dans le buffer1
static sem_t full1; //si > 0 alors il y a des données dans le buffer1
node * buffer1; //Buffer pour les fractales "vides"

static pthread_mutex_t mutex2; //Mutex du buffer calcul - moyenne
static sem_t empty2;
static sem_t full2;
node * buffer2; //Buffer pour les fractales calculées et la moyenne

struct fractal *bestAv; //Variable où stocker la meilleure fractale
int flagDetail = 0;
int maxThreads = 0;
int nbrProducer;//Condition d'arrêt des consommateurs
int nbrConsumer;//Condition d'arrêt du thread de moyenne
int nbrArg = 1; //Lecture des arguments


void initFirst() {
	int err = pthread_mutex_init(&mutex1, NULL);
	if (err != 0)
		fprintf(stderr,"ERREUR : pthread_mutex_init (premier buffer)\n");

	int ret1 = sem_init(&empty1, 0, N);
	if (ret1 != 0)
		fprintf(stderr, "ERREUR : Création de empty1 (premier buffer)\n");

	int ret2 = sem_init(&full1, 0, 0);
	if (ret2 != 0)
		fprintf(stderr, "ERREUR : Création de full1 (premier buffer)\n");
}

void initSecond() {
	int err = pthread_mutex_init(&mutex2, NULL);
	if (err != 0)
		fprintf(stderr,"ERREUR : pthread_mutex_init (second buffer)\n");

	int ret1 = sem_init(&empty2, 0, N);
	if (ret1 != 0)
		fprintf(stderr, "ERREUR : Création de empty2 (second buffer)\n");

	int ret2 = sem_init(&full2, 0, 0);
	if (ret2 != 0)
		fprintf(stderr, "ERREUR : Création de full2 (second buffer)\n");

	bestAv = fractal_new("empty", 1, 1, 0.0, 0.0);
}

struct fractal* compute(char* str){
	const char *delim = " ";
	char* name = strsep(&str, delim);
	if (str == NULL) { //Si à ce point str vaut NULL, c'est que la chaine ne contenait que le nom
		fprintf(stderr, "Erreur, la fractale : %s n'est pas formatée correctement. Elle a été ignorée", str);
		return NULL;
	}

	int width = atoi(strsep(&str, delim));
	if (str == NULL || width <= 0) { //Il manque des arguments ou la longueur est invalide
		fprintf(stderr, "Erreur, la fractale : %s n'est pas formatée correctement. Elle a été ignorée", str);
		return NULL;
	}

	int height = atoi(strsep(&str, delim));
	if (str == NULL || height <= 0) { //Il manque des arguments ou la hauteur est invalide
		fprintf(stderr, "Erreur, la fractale : %s n'est pas formatée correctement. Elle a été ignorée", str);
		return NULL;
	}

	double a = strtod(strsep(&str, delim), NULL);
	if (str == NULL || a > 1.0 || a < -1.0) { //Il manque un argument ou a n'est pas dans les bonnes bornes
		fprintf(stderr, "Erreur, la fractale : %s n'est pas formatée correctement. Elle a été ignorée", str);
		return NULL;
	}

	double b = strtod(str, NULL);
	if (b > 1.0 || b <- 1.0) { //b n'est pas dans les bonnes bornes
		fprintf(stderr, "Erreur, la fractale : %s n'est pas formatée correctement. Elle a été ignorée", str);
		return NULL;
	}
	struct fractal *result = fractal_new(name, width, height, a, b); //name+1 pour éviter la parenthèse
	return result;
}

void insert(struct fractal *fract) {
	sem_wait(&empty1);//On attend une place sur le buffer
	pthread_mutex_lock(&mutex1);//On lock
	int res = stack_push(&buffer1, fract);//On ajoute la fractale
	if (res != 0)//S'il y a un problème, le signaler
		fprintf(stderr, "Impossible d'ajouter la fractale %s au buffer1. Elle a été ignorée\n", fractal_get_name(fract));
	pthread_mutex_unlock(&mutex1);
	sem_post(&full1);
}

void * producer(void *arg) {
	char * fichier = (char*) arg;
	FILE* toRead = NULL;//Créer le FILE pour la lecture
	if (strcmp(fichier, "-") != 0 )//Ouvrir le fichier si ce n'est pas "-"
		toRead = fopen(fichier, "r");
	else//Si c'est "-", on a utilisé l'entrée standard, ouvir le fichier temporaire stdin.txt
		toRead = fopen("stdin.txt", "r");
	if (toRead != NULL) {//Si on a su ouvir
		char chaine[50] = "";
		while (fgets(chaine, 50, toRead) != NULL) { // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)
			chaine[strlen(chaine)-1] = '\0';
			char comm = *chaine;
			if (comm != '#') { //On regarde si ce n'est pas une ligne en commentaire
				struct fractal *toAdd = compute(chaine);
				if (toAdd != NULL){
					insert(toAdd);
				}
			}
		}
		fclose(toRead);//Fermer le fichier
		if (strcmp(fichier, "-") == 0) //Si c'est le fichier tampon où l'utilisateur a entré ses données, le supprimer
			remove("stdin.txt");
	} 
	else{
		fprintf(stderr, "Impossible de lire le fichier %s. Il a été ignoré\n", fichier);
	}
	pthread_exit(NULL);
}

void * consumer() {
	while (nbrProducer != 0 || stack_length(buffer1) != 0) { //TODO
		sem_wait(&full1); //attente d'un slot rempli
		pthread_mutex_lock(&mutex1);
		struct fractal *toFill = stack_pop(&buffer1);//Récupérer la fractale
		toFill = fractal_fill(toFill);//Remplir la fractale
		if (flagDetail){//S'il faut faire les détails, créer les images
				write_bitmap_sdl(toFill, strcat(fractal_get_name(toFill), ".bmp"));//bouger dans le thread de moyenne ?
			}
		pthread_mutex_unlock(&mutex1);
		sem_post(&empty1); //il y a un slot libre en plus
		//Partie producer
		sem_wait(&empty2); //On attend une place sur le buffer2
		pthread_mutex_lock(&mutex2); //Quand on l'a, on lock
		int res = stack_push(&buffer2, toFill); //On met la fractale dans le buffer2
		if (res != 0)
			fprintf(stderr, "Impossible d'ajouter la fractale %s au buffer2. Elle a été ignorée\n", fractal_get_name(toFill));
		pthread_mutex_unlock(&mutex2); //On delock
		sem_post(&full2); //On signale qu'une valeur est présente
	 }
	 pthread_exit(NULL);
 }

void * average() {
	 while (nbrConsumer != 0 || stack_length(buffer2)!=0) {
		 if(nbrConsumer != 0){//Evite un deathlock à la toute dernière itéraltion
			sem_wait(&full2); //On attend qu'il y ait quelque chose dans le buffer
		}
		else{
			break;
		}
		pthread_mutex_lock(&mutex2); //On lock
		struct fractal *test = stack_pop(&buffer2); //On prend la fractale;
		if (fractal_get_av(test) > fractal_get_av(bestAv)) { //Si la fractale est meilleure que celle précédement en mémoire
			fractal_free(bestAv);
			bestAv = test;
		}
		else{
			fractal_free(test);
		}
		pthread_mutex_unlock(&mutex2);
		sem_post(&empty2);
	 }
	 pthread_exit(bestAv);
 }

int main(int argc, char const *argv[]) {
	/*   Lecture des arguments   */
	if (argc < 3) { //Il faut au moins 3 arguments (le nom de base, un fichier d'entrée et un de sortie)
		fprintf(stderr, "Il n'y a pas assez d'arguments donnés, il faut au moins stipuler un fichier d'où tirer les données des fractales et un fichier de sortie!");
		exit(EXIT_FAILURE);
	}
	char const *arg = argv[nbrArg];
	if (strcmp(arg, "-d") == 0) { //Si on trouve le -d, mettre le flag à 1
		flagDetail = 1;
		nbrArg++; //Un argument de lu, passons au suivant
		printf("Vous avez choisi l'option -d\n");
	} else {
		flagDetail = 0;
	}
	arg = argv[nbrArg];
	char *code = strstr(arg, "--"); //Si on trouve les deux tirets, c'est maxThreads
	if (code != NULL) {
		int nbr = atoi(code+2); //Récupérer le nombre
		maxThreads = nbr; //Le stocker
		nbrArg++; //Passer à l'argument suivant
		printf("Vous avez demandé %i threads de calcul\n", maxThreads);
		arg = argv[nbrArg]; //Permet de switcher les arguments mais moche
		if (strcmp(arg, "-d") == 0) {
			flagDetail = 1;
			nbrArg++;
		}
	} else {
		printf("Vous n'avez pas précisé de nombre de threads de calcul, nombre par défaut : 1\n");
		maxThreads = 1;
	}
	
	/*   Initialisation des buffers, mutex et semaphores   */
	initFirst();
	initSecond();
	
	/*   Lancementdes producteurs   */
	nbrProducer=argc-nbrArg-1;
	pthread_t threads[nbrProducer];
	int nthread = 0;
	while (nbrArg < argc-1) { //Tant qu'on a des arguments à lire (ici, ce sont des fichiers + s'arrêter un avant la fin pour l'output)
		char const *fichier = argv[nbrArg];
		if (strcmp(fichier, "-") == 0) {
			FILE* std = NULL;
			std = fopen("stdin.txt", "w"); //Créons un fichier tampon où stocker les entrées au clavier (w efface l'éventuel contenu, pour s'assurer qu'il est vide au départ)
			int stop = 0;
			while (stop != 1) {
				char saisie[50];
				printf("Entrez une fractale (Nom hauteur largeur a b) : \n");
				fgets (saisie, sizeof(saisie), stdin); //On ne modifie pas car on veut le retour à la ligne
				fputs(saisie, std);
				printf("Voulez-vous entrer une autre fractale ? (o/n)\n");
				char rep[10];
				fgets(rep, sizeof(rep), stdin);
				rep[strlen(rep)-1] = '\0';
				int res = strcmp(rep, "n");
				if (res == 0)
					stop = 1;
			}
			fclose(std);
		}
		pthread_t th = NULL;
		threads[nthread] = th;
		int res = pthread_create(&threads[nthread], NULL, *producer, (void *) fichier);
		nthread++;
		if (res != 0)
			fprintf(stderr, "Problème à la création d'un producteur pour le fichier %s\n", fichier);
		nbrArg++;
	}

	/*   Lancement des consommateurs   */
	nbrConsumer=maxThreads;
	pthread_t threadsC[maxThreads];
	for(int i = 0; i < maxThreads; i++){
		pthread_t th = NULL;
		threadsC[i] = th;
		int res = pthread_create(&threadsC[i], NULL, *consumer, NULL);
		if (res != 0)
			fprintf(stderr, "Problème à la création du consumer n°%d\n", i);
	}

	/*   Thread de moyenne   */
	pthread_t moyenne = NULL;
	int res = pthread_create(&moyenne, NULL, *average, NULL);
	if (res != 0)
		fprintf(stderr, "Problème à la création du thread de moyenne\n");

	for (int i = 0; i < nthread; i++){
		pthread_join(threads[i], NULL);
		nbrProducer--;//Un producer en moins
	}
	for (int i = 0; i < maxThreads; i++){
		pthread_join(threadsC[i], NULL);
		nbrConsumer--;//Un consommateur de moins
	}
	free_list(buffer1);
	free_list(buffer2);

	pthread_join(moyenne, NULL);
    write_bitmap_sdl(bestAv, argv[nbrArg]);
    printf("Fichier out écrit avec : %s\n", fractal_get_name(bestAv));
    return 0;
}
