/*   Librairies de base   */
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/*   Librairies des threads   */
#include <pthread.h>
#include <semaphore.h>

/*   Librairies internes   */
#include "libfractal/fractal.h"
#include "stack/stack.h"
#include "threads/threads.h"

/*   Protection du premier buffer   */
pthread_mutex_t mutex1; //mutex du buffer lecture - calcul
sem_t empty1; //si > 0 alors il y a des places libres dans le buffer1
sem_t full1; //si > 0 alors il y a des données dans le buffer1
node * buffer1; //Buffer pour les fractales "vides"

/*   Protection du second buffer   */
pthread_mutex_t mutex2; //Mutex du buffer calcul - moyenne
sem_t empty2; //si > 0 alors il y a des places libres dans le buffer2
sem_t full2; //si > 0 alors il y a des données dans le buffer2
node * buffer2; //Buffer pour les fractales calculées et la moyenne

/*   Gestion de la fin du programme   */
pthread_mutex_t files;
int remainingFiles = 0; //Stock le nombre de fichiers restant à lire
pthread_mutex_t newfract;
int readFract = 0; //Stock le nombre de fractales lues
pthread_mutex_t finished;
int finishedFract = 0; //Stock le nombre de fractales terminées (passées par le thread de moyenne)

int flagDetail = 0; //Option de détail sur toutes les fractales
static int maxThreads = 0; //Nombre maximum de threads de calcul
static int nbrArg = 1; //Lecture des arguments

/*
 * Initialisation des mutex et sémaphores pour le premier buffer
 */
void initFirst() {
	int err = pthread_mutex_init(&mutex1, NULL);
	if (err != 0)
		fprintf(stderr,"ERREUR : pthread_mutex_init (premier buffer)\n");

	int ret1 = sem_init(&empty1, 0, maxThreads);
	if (ret1 != 0)
		fprintf(stderr, "ERREUR : Création de empty1 (premier buffer)\n");

	int ret2 = sem_init(&full1, 0, 0);
	if (ret2 != 0)
		fprintf(stderr, "ERREUR : Création de full1 (premier buffer)\n");
}

/*
 * Initialisation des mutex et sémaphores pour le second buffer
 */
void initSecond() {
	int err = pthread_mutex_init(&mutex2, NULL);
	if (err != 0)
		fprintf(stderr,"ERREUR : pthread_mutex_init (second buffer)\n");

	int ret1 = sem_init(&empty2, 0, maxThreads);
	if (ret1 != 0)
		fprintf(stderr, "ERREUR : Création de empty2 (second buffer)\n");

	int ret2 = sem_init(&full2, 0, 0);
	if (ret2 != 0)
		fprintf(stderr, "ERREUR : Création de full2 (second buffer)\n");
}

/*
 * Initialisation des mutex pour gérer la fin du programme
 */
void initThird() {
	int err1 = pthread_mutex_init(&files, NULL);
	if (err1 != 0)
		fprintf(stderr, "ERREUR pthread_mutex_init (files)\n");

	int err2 = pthread_mutex_init(&newfract, NULL);
	if (err2 != 0)
		fprintf(stderr, "ERREUR pthread_mutex_init (newfract)\n");

	int err3 = pthread_mutex_init(&finished, NULL);
	if (err3 != 0)
		fprintf(stderr, "ERREUR pthread_mutex_init (finished)\n");
}

/*
 * Fonction principale du programme
 */
int main(int argc, char const *argv[]) {
	printf("Bonjour et bienvenue dans ce programme de création de fracatles\n");//Bonjour
	/*   Lecture des arguments   */
	if (argc < 3) { //Il faut au moins 3 arguments (le nom de base, un fichier d'entrée et un de sortie)
		fprintf(stderr, "Il n'y a pas assez d'arguments donnés, vous devez au moins donner un fichier contenant les fractales et un fichier de sortie!\n");
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
	if (strcmp(arg, "--maxthreads") == 0) {//Chercher maxthreads
		nbrArg++; //On va un argument plus loin pour trouver le nombre
		int nbr = atoi(argv[nbrArg]); //Récupérer le nombre
		maxThreads = nbr; //Le stocker
		nbrArg++; //Passer à l'argument suivant
		printf("Vous avez demandé %i threads de calcul\n", maxThreads);
		arg = argv[nbrArg]; //Permet de switcher les arguments mais moche
		if (strcmp(arg, "-d") == 0) { //Si on trouve le -d, mettre le flag à 1
			flagDetail = 1;
			nbrArg++;
			printf("Vous avez choisi l'option -d\n");
		}
	} else {
		printf("Vous n'avez pas précisé de nombre de threads de calcul, nombre par défaut : 1\n");
		maxThreads = 1;
	}

	/*   Initialisation des buffers, mutex et semaphores   */
	initFirst();
	initSecond();
	initThird();

	/*   Lancement des producteurs   */
	pthread_t threadP[argc-nbrArg-1]; //Stockage des threads de production
	int nthreadP = 0; //Nombre de threads de production
	while (nbrArg < argc-1) { //Tant qu'on a des arguments à lire (ici, ce sont des fichiers + s'arrêter un avant la fin pour l'output)
		pthread_mutex_lock(&files);
		remainingFiles++; //Un fichier de plus à lire
		pthread_mutex_unlock(&files);
		char const *fichier = argv[nbrArg];
		if (strcmp(fichier, "-") == 0) { //S'il faut lire l'entrée standart
			FILE* std = NULL;
			std = fopen("stdin.txt", "w"); //Créons un fichier tampon où stocker les entrées au clavier (w efface l'éventuel contenu, pour s'assurer qu'il est vide au départ)
			int stop = 0;
			while (stop != 1) {
				char saisie[50];
				printf("Entrez une fractale (Nom hauteur largeur a b) : \n");
				fgets (saisie, sizeof(saisie), stdin);
				fputs(saisie, std);
				printf("Voulez-vous entrer une autre fractale ? (o/n)\n");
				char rep[10];
				fgets(rep, sizeof(rep), stdin);
				rep[strlen(rep)-1] = '\0'; //Enlève le retour à la ligne
				int res = strcmp(rep, "n");
				if (res == 0)
					stop = 1; //Fin de lecture
			}
			fclose(std);
		}
		pthread_t th = NULL;
		threadP[nthreadP] = th;
		int res = pthread_create(&threadP[nthreadP], NULL, *producer, (void *) fichier); //Création d'un producteur
		nthreadP++;
		if (res != 0)
			fprintf(stderr, "Problème à la création d'un producteur pour le fichier %s\n", fichier);
		nbrArg++;
	}

	/*   Attendre pour que les fichiers commencent à se lire   */
	sleep(1);

	/*   Lancement des consommateurs   */
	for(int i = 0; i < maxThreads; i++){
		pthread_t th = NULL;
		int res = pthread_create(&th, NULL, *consumer, NULL); //Création d'un consommateur
		if (res != 0)
			fprintf(stderr, "Problème à la création du consumer n°%d\n", i);
	}

	/*   Thread de moyenne   */
	pthread_t moyenne = NULL;
	int res = pthread_create(&moyenne, NULL, *average, NULL); //Création du thread de moyenne
	if (res != 0)
		fprintf(stderr, "Problème à la création du thread de moyenne\n");

	/*   Jointure des threads de production, fichier lu entièrement   */
	for (int i = 0; i < nthreadP; i++){
		pthread_join(threadP[i], NULL);
		pthread_mutex_lock(&files);
		remainingFiles--; //Un fichier est terminé
		pthread_mutex_unlock(&files);
	}

	/*   Jointure du thread de moyenne   */
	void * ret = NULL;
	pthread_join(moyenne, &ret);
	struct fractal * bestAv = (struct fractal *) ret; //Récupère la meilleure fractale

	/*   Libération des buffers   */
	free_list(buffer1);
	free_list(buffer2);

	write_bitmap_sdl(bestAv, argv[nbrArg]); //Écriture du fichier
	printf("Fichier %s écrit avec : %s\n", argv[nbrArg], fractal_get_name(bestAv));
	remove(fractal_get_name(bestAv)); //Supression du fichier doublon
	fractal_free(bestAv);
	return 0;
}
