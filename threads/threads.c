#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "../libfractal/fractal.h"
#include "../stack/stack.h"
#include "threads.h"

/*   Récupérations des variables nécésessaires depuis main.c   */
extern pthread_mutex_t mutex1;
extern sem_t empty1;
extern sem_t full1;
extern node * buffer1;

extern pthread_mutex_t mutex2;
extern sem_t empty2;
extern sem_t full2;
extern node * buffer2;

extern int flagDetail;

extern pthread_mutex_t files;
extern int remainingFiles;

extern pthread_mutex_t newfract;
extern int readFract;

extern pthread_mutex_t finished;
extern int finishedFract;

/*
 * Fonction auxilière à producer
 * Crée une fractale depuis un string
 *
 * @str: description d'une fractale
 * @return: la fractale créée
 */
struct fractal* compute(char* str) {
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
	struct fractal *result = fractal_new(name, width, height, a, b); //crée la fractale
	pthread_mutex_lock(&newfract);
	readFract++; //Une fractale supplémentaire a été créée
	pthread_mutex_unlock(&newfract);
	return result; //Renvoie la fracale
}

/*
 * Fonction auxilière à producer
 * Met une fractale dans le premier buffer
 *
 * @fract: la fractale à insérer
 */
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
	printf("Lecture du fichier %s\n", arg);
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
	while (1) {
		sem_wait(&full1); //attente d'un slot rempli
		pthread_mutex_lock(&mutex1);
		struct fractal *toFill = stack_pop(&buffer1);//Récupérer la fractale
		toFill = fractal_fill(toFill);//Remplir la fractale
		if (flagDetail) {//S'il faut faire les détails, créer les images
				write_bitmap_sdl(toFill, strcat(fractal_get_name(toFill), ".bmp"));
				printf("Fichier %s.bmp écrit\n", fractal_get_name(toFill));
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
	/*   Récupération des variables pour la condition de fin   */
	pthread_mutex_lock(&files);
	pthread_mutex_lock(&newfract);
	pthread_mutex_lock(&finished);
	int nfile = remainingFiles;
	int nread = readFract;
	int nfinished = finishedFract;
	pthread_mutex_unlock(&files);
	pthread_mutex_unlock(&newfract);
	pthread_mutex_unlock(&finished);

	struct fractal *bestAv = fractal_new("empty", 1, 1, 0.0, 0.0); //Variable où stocker la meilleure fractale
	while (nfile != 0 || nread != nfinished) { //Tant qu'il y a des fichiers à lire ou que le nombre de fractales créées est différent du nombre de fracales terminées
		sem_wait(&full2); //On attend qu'il y ait quelque chose dans le buffer
		pthread_mutex_lock(&mutex2); //On lock
		struct fractal *test = stack_pop(&buffer2); //On prend la fractale;
		if (fractal_get_av(test) > fractal_get_av(bestAv)) { //Si la fractale est meilleure que celle précédement en mémoire
			fractal_free(bestAv);
			bestAv = test;
		} else {
			fractal_free(test);
		}
		pthread_mutex_unlock(&mutex2);
		sem_post(&empty2);

		/*   Update des variables pour la condition de fin   */
		pthread_mutex_lock(&files);
		pthread_mutex_lock(&newfract);
		pthread_mutex_lock(&finished);
		finishedFract++; //Une fracatle supplémentaire est terminée
		nfile = remainingFiles;
		nread = readFract;
		nfinished = finishedFract;
		pthread_mutex_unlock(&files);
		pthread_mutex_unlock(&newfract);
		pthread_mutex_unlock(&finished);
	}
	pthread_exit((void *) bestAv); //Renvoie la meilleure fractale
}
