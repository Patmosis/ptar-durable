//////////////////////////////////////////////////////////   ptar - Extracteur d'archives durable et parallèle  ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////   v 1.7.5.0        15/12/2016   ///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#include <zlib.h>
#include <dlfcn.h>

#include "utils.h"
#include "checkfile.h"

int main(int argc, char *argv[]) {

	/*
	Déclarations et initialisation des variables
	*/

	int j;							//Indice de la boucle for pour la création des threads;
	int k;							//Indice de la boucle for de destruction de threads éventuelle.
	int status;					//Retour des pthread_create
	int opt;						//Valeur de retour de getopt().
	pthread_t *tabthrd;	//Tableau des idf des threads;
	void *ret;					//Structure pour le join final des threads.
	void *arg;					//Argument pour la procédure traitement().

	//Initialisation des flags des options de ligne de commande. Ce sont des variables globales dans utils.h
	thrd=0;
	nthreads=0;
	extract=0;
	listingd=0;
	decomp=0;
	logflag=0;
	isEOF=false;
	isCorrupted=false;
	corrupted=false;
	arg=NULL;

	//Faire taire ce warning unused
	MutexRead=(pthread_mutex_t) MutexRead;

	/*
	Traitement des options de ligne de commande. Set des flags (var globales) d'options.
	*/

	while ((opt=getopt(argc, argv, "xelzp:")) != -1) {
		switch (opt) {
			//Logfile (utile seulement pour l'extraction pour l'instant)
			case 'e' :
				logflag=1;
				break;
			//Extraction : -x
			case 'x' :
				extract=1;
				break;
			//Listing détaillé : -l
			case 'l' :
				listingd=1;
				break;
			//Décompression : -z
			case 'z' :
				decomp=1;
				break;
			//Durabilité et parallélisation : -p NBTHREADS
			case 'p' :
				nthreads=atoi(optarg); // nthreads <-- NBTHREADS
				//Si le p est mal placé (il optarg sera la lettre suivante), atoi renverra 0, c'est-à-dire que optarg n'est pas un nombre.
				//(Et on considère qu'on ne tolère pas 0 threads ...)
				if (nthreads==0) {
					printf("Utilisation: %s [-x] [-l] [-z] [-p NBTHREADS] emplacement_archive\n", argv[0]);
					return 1;
				}
				thrd=1;
				break;
			default:
				printf("Utilisation: %s [-x] [-l] [-z] [-p NBTHREADS] emplacement_archive\n", argv[0]);
				return 1;
		}
	}

	//Pour l'option -p, attente d'un argument pour p (et ce sera forcément un int à cause du test dans le case 'p')
	if (optind >= argc) {
		printf("Arguments attendus après options\n");
		return 1;
	}

	/*
	Tester l'argument (existence et nom bien formé).
	*/

	if (checkfile(argv[optind])==false) {
		//Les messages d'erreurs sont gérés dans checkfile.c.
		//Si l'archive est seulement compressée, tente une décompression directe: ptar n'est pas prévu pour cela, ce module est donc en bêta (développement inutil)
		return 1;
	}

	/*
	Génération du logfile si besoin -e. Option pour développeurs. (utile pour extraction et décompression)
	*/

	if (logflag==1) {
		logfile = genlogfile("logfile.txt", "a", argv[optind]);
	}

	/*
	Ouverture du fichier et mise en variable globale du descripteur de fichier.
	*/

	//On ouvre l'archive tar avec open() et le flag O_RDONLY (read-only). Si option -z, on décompresse, récupère les données dans un tube et on ouvre ce tube nommé.
	if (decomp==1) {

		//On charge dynamiquement la zlib par une mini fonction utilisant dlopen.
		loadzlib();

		//On ouvre le fichier tar.gz avec la zlib.
		filez=(*gzOpen)(argv[optind], "rb");

		//Cas d'erreur du gzopen.
		if (filez==NULL) {
			printf("Erreur d'ouverture du fichier, gzopen() retourne : %p. Le fichier n'existe pas ou alors est corrompu.\n", filez);
		}

		//On rembobine la tête de lecture au cas où
		(*gzRewind)(file);
	}

	//Cas où pas compressée.
	else {
		file=open(argv[optind], O_RDONLY);

		//Cas d'erreur -1 du open().
		if (file<0) {
			printf("Erreur d'ouverture du fichier, open() retourne : %d. Le fichier n'existe pas ou alors est corrompu.\n", file);
			if (logflag==1) fclose(logfile); //Aussi le logfile.
			return 1;
		}
	}

	/*
	Création des threads et Lancement de la procédure.
	*/

	if (thrd==1) {
		//Tableau des pthread_t
		tabthrd=malloc(nthreads*sizeof(pthread_t));

		//Lancement de chaque threads.
		for (j=0; j<nthreads; j++) {
			status=pthread_create(&tabthrd[j], NULL, (void *)traitement, arg);

			//Si une erreur est provoquée lors de la création, on arrete les autres threads puis le programme.
			if (status<0) {
				printf("Erreur lors de la création du thread numéro %d.", j);
				for (k=0; k<j; k++) {
						pthread_cancel(tabthrd[k]);
				}
				return 1;
			}
		}

		//Join des threads
		for (j=0; j<nthreads; j++) {
			(void)pthread_join(tabthrd[j], &ret);
		}

		//Libération du pointeur sur les threads.
		free(tabthrd);
	}

	//Si on ne veut pas utiliser de threads, on appelle tous simplement la procédure.
	else {
		traitement(arg);
	}

	//Fermeture de l'archive.
	if (decomp==0) {
		close(file);
	}
	else {
		(*gzClose)(filez);
	}

	//Fermeture de zlib dynaique.
	if (decomp==1 && handle != NULL) dlclose(handle);

	//Fermeture propre du logfile.
	if (logflag==1) {
		fclose(logfile);
		fputs("Les sommes de contrôle (checksum) sont toutes valides.\n", logfile);
		fputs("Fin de decompression/extraction.\n\n", logfile);
	}

	//Si l'archive est corrompu, ptar doit renvoyer 1
	if (corrupted==true) {
		printf("ptar : arrêt avec somme de contrôle invalide d'un des élément de l'archive.\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}
