#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "libfractal/fractal.h"
#include "stack/stack.h"
#include "threads/threads.h"

#define N 100 //taille max des buffers (plus forcément utile avec les stacks

pthread_mutex_t mutex1; //mutex du buffer lecture - calcul
sem_t empty1; //si > 0 alors il y a des places libres dans le buffer1
sem_t full1; //si > 0 alors il y a des données dans le buffer1
node * buffer1; //Buffer pour les fractales "vides"

pthread_mutex_t mutex2; //Mutex du buffer calcul - moyenne
sem_t empty2;
sem_t full2;
node * buffer2; //Buffer pour les fractales calculées et la moyenne

struct fractal *bestAv; //Variable où stocker la meilleure fractale
int flagDetail = 0;
pthread_mutex_t prod;
int nbrProducer;//Condition d'arrêt des consommateurs
pthread_mutex_t cons;
int nbrConsumer;//Condition d'arrêt du thread de moyenne

static int maxThreads = 0;
static int nbrArg = 1; //Lecture des arguments

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

	int err2 = pthread_mutex_init(&prod, NULL);
	if (err2)
		fprintf(stderr, "ERREUR : pthread_mutex_init (prod)\n");
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

	int err2 = pthread_mutex_init(&cons, NULL);
	if (err2)
		fprintf(stderr, "ERREUR : pthread_mutex_init (cons)\n");

	bestAv = fractal_new("empty", 1, 1, 0.0, 0.0);
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
	pthread_mutex_lock(&prod);
	nbrProducer=argc-nbrArg-1;
	pthread_t threads[nbrProducer];
	pthread_mutex_unlock(&prod);
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
	pthread_mutex_lock(&cons);
	nbrConsumer=maxThreads;
	pthread_t threadsC[nbrConsumer];
	pthread_mutex_unlock(&cons);
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
		pthread_mutex_lock(&prod);
		nbrProducer--;//Un producer en moins
		pthread_mutex_unlock(&prod);
	}
	for (int i = 0; i < maxThreads; i++){
		pthread_join(threadsC[i], NULL);
		pthread_mutex_lock(&cons);
		nbrConsumer--;//Un consommateur de moins
		pthread_mutex_unlock(&cons);
	}

	pthread_join(moyenne, NULL);

	free_list(buffer1);
	free_list(buffer2);

  write_bitmap_sdl(bestAv, argv[nbrArg]);
  printf("Fichier out écrit avec : %s\n", fractal_get_name(bestAv));
	fractal_free(bestAv);
    return 0;
}
