/*   Interface pour les différents threads du programme   */

/*
 * Fonction d'un thread de producteur
 * Lis un fichier, récupère les fractales décries
 * Envoie les fractales dans un buffer pour les consommateurs
 *
 * @arg: fichier à lire par le producteur
 * @return: NULL quand le fichier est terminé
 */
void * producer(void *arg);

/*
 * Fonction d'un thread de consommateur
 * Récupère une fracale créée par un producteur
 * Remplis le tableau de valeur et calcule la moyenne
 * Envoie les fractales remplies dans un buffer pour le thread de moyenne
 *
 * @return: Rien, ne s'arrête jamais
 */
void * consumer();

/*
 * Fonction du thread de moyenne
 * Récupère une fractale remplie par un consommateur
 * Stock la meilleure fractales
 *
 * @return: la meilleure fractale
 */
void * average();
