#PTAR

Extracteur durable et parallèle d'archives ustar POSIX

Projet de RS TELECOM Nancy 2016-2017

Auteurs

    GARCIA Guillaume
    ZAMBAUX Gauthier

Dépendances

    zlibc zlib1g zlib1g-dev        (La bibliothèque zlib)
    groff-utf8                     (optionnel, pour ptar(1))

Build & execute:

    make
    ./ptar  [opts]  [args]

Utilisation de ptar

	Lister les éléments contenus dans une archive tar

	    ./ptar <chemin/vers/archive>

	Extraire les éléments d'une archive tar (et listing basique)

	    ./ptar -x <chemin/vers/archive>

	Listing détaillé des métadonnées des éléments d'une archive tar

	    ./ptar -l <chemin/vers/archive>

	Parallélisation et durabilisation des écritures avec NBTHREADS threads

	    ./ptar -p <NBTHREADS> <chemin/vers/archive>

	Décompression d'une archive .tar.gz (compressée avec gzip), gère le cas de header désordonnés

	   ./ptar -z <chemin/vers/archive>

	Générer un logfile (pour extraction & décompression) de divers codes retours, pour développeurs

	   ./ptar -e <chemin/vers/archive>


	Exemples :

	    Les options peuvent être combinées.

	    L'exemple testf.tar fourni dans le git ne contient que des fichiers (dont un vide).

	    L'exemple testall.tar fourni dans le git contient des fichiers, des dossiers et un lien symbolique.

	    L'exemple testfalsearch.tar fourni dans le git n'est pas une archive mais un fichier avec l'extension .tar.

	    Les exemples testf.tar.gz et testall.tar.gz sont les mêmes mais compressés au format gzip.

	    ./ptar archives_test/testall.tar
	    ./ptar -x archive_test/testall.tar
	    ./ptar -l archive_test/testall.tar
	    ./ptar -xl archive_test/testall.tar
	    ./ptar -xe archive_test/testall.tar
	    ./ptar -z archive_test/testall.tar
	    ./ptar -xz archive_test/testall.tar.gz
	    ./ptar -zxl archive_test/testall.tar.gz
	    ./ptar -xlp 3 archive_test/testall.tar
	    ./ptar -xzlp 4 archive_test/testall.tar.gz
      ./ptar -lxzep 6 archive_test/testall.tar.gz

	    Exécuter le script rmtest.sh avant chaque test sur les archives pour nettoyer le dossier courant.
	    ./rmtest.sh


    	Il est possible d'également décompresser un fichier.gz (sans archivage), mais les attributs (tels que premissions, gid et uid) ne sont pas pas traités.
    	Les permissions sont forcées : Lecture ecriture et recherche pour l'utilisateur.
    	Cette fonction est purement optionnelle et en bêta.
    	./ptar -z manpage/ptar.1.gz


###Page de manuel ptar(1)

	Lire la page de manuel de ptar sans manipulations/droits super-utilisateurs au préalable

	   man ./manpage/ptar.1.gz

	Lire la page de manuel avec `man ptar`, nécessite d'avoir les droits super-utilisateurs

	   sudo cp ./manpage/man.config /etc
	   sudo mkdir -p /usr/local/man/man1/
	   sudo install -g 0 -o 0 -m 0644 ./manpage/ptar.1.gz /usr/local/man/man1/
	   man ptar

	   (La commande `man ptar` devrait alors afficher la page man du bon programme
	   et pas celle du programme ptar potentiellement préexistant "tar-like program written in perl")

	Lire la page de manuel avec groff-utf8

	   (Si nécessaire, télécharger le paquet groff-utf8, sinon passer à (***) )
 	   wget http://www.haible.de/bruno/gnu/groff-utf8.tar.gz
	   (C'est une archive tar non compressée à l'inverse de ce que laisse penser son extension !)
	   tar xvf groff-utf8.tar.gz
 	   cd groff-utf8
	   make
	   make install PREFIX=/usr/local
	   (Veillez à vérifier que votre chemin $PREFIX/bin est contenu dans $PATH).
	   cd <chemin/dossier/rs2016-Garcia-Zambaux>
	   (***)
	   tar -xvzf ./manpage/ptar.1.gz
	   groff-utf8 -Tutf8 -mandoc ptar.1 | less


###Bibliotheque dynamique : zlib

  Installer la zlib

      sudo apt-get install zlibc zlib1g zlib1g-dev

  Configurer la variable LD_LIBRARY_PATH

      export LD_LIBRARY_PATH=<path_du_raccourci_vers_zlib.so>


###Liste des corrections

  Dernière Màj

  	13/12/2016    20:36     version 1.7.3.0 : Version stable de ptar multithreadé distribuable

  Liste des corrections 1.7.0.0:

      - Les fonctions checkpath() et recoverpath() sont optimisées et améliorées, et devraient être
        stables à long terme. La fonction recoverpath() a été largement corrigée et optimisée.
      - Le programme entier a été remanié pour fonctionner sur plusieurs threads (-p). Le main lance
        un certain nombre de threads (passé en paramètre avec la fonction -p), qui vont lancer la procédure
        traitement() modifiée pour l'occasion. Deux mutex ont été utilisés pour protéger en lecture l'archive source
        et en écriture les éléments extraits.
      - La décompression se fait donc avant la création des threads et les données sont stockées dans un pipe
        nommé, cependant, la taille maximale d'un pipe est de 64Ko, et sachant qu'il est nécessaire de tout écrire
        dans le pipe avant de lire dedans (ici), on ne peut pas décompresser des tar.gz de taille supérieur à 64Ko, sinon
        le write en entrée du pipe est bloquant ! Il faudra voir une version améliorée du programme (threads+décompression)
        dans une version supérieur (prévue pour 1.7.2.0).

  Liste des corrections 1.7.0.1:

      - Les threads sont tous lancés et dans le bon nombre. En effet, une mauvaise utilisation de pthread_create conduisait
        à l'echec de la création mais le traitement se lançait toute de même (dans le thread principal). Aussi, un thread en
        trop était créé à cause d'une incrémentation inutile du nombre de threads.
      - Si l'option -p n'est pas spécifiée, ne crée aucun thread et appelle traitement() normalement. En effet, ptar
        ne créait aucun thread, mais n'appelait pas non plus traitement dans ce cas, et terminait normalement sans rien faire.
      - Optimisation de la compatibilité : on considère maintenant que si le champ typeflag contient un NUL (byte nul),
        il s'agit d'un fichier (typeflag='0') : voir tar(5).

  Liste des corrections 1.7.1.0:

      - Les champs char du header ont une terminaison forcée par "\0", en effet il arrivait que certains champs fusionnent.
        C'était le cas du typeflag et du linkname dans certains cas (type=2 et link=5 => typeflag=25 apres strtol).
      - Les fichiers/dossiers intermédiaires créés pour les symlink() sont immédiatemment supprimés après création du lien,
        seulement si le fichier n'existait pas au préalable. Cela permet de créer des liens dans le cas éventuel où leur fichier
        pointé dans l'archive n'existe pas ou a été supprimé de l'archive (cf tests blancs : include/curses.h).
      - Le checkfile et l'open (et son cas d'erreur) sont désormais bien placés dans le main et non dans traitement()
        qui est utilisé par les threads (redondance des appels aux fonctions précédents donc).
      - Les symlink vers des dossiers fonctionnent désormais correctement, à condition que les linkname des dossiers
        pointés se terminent bien par le caractère '/', autrement on ne peut pas le différencier d'un fichier/symlink pointé.
      - Optimisation des fonctions existeFile() et existeDir() et de leur implémentation dans extraction(). Optimisation
        générale de extraction(). Stabilité accrue pour l'attribution des permissions.
      - Une archive "vicieuse" a été construite sur la base des tests blancs pour vérifier la stabilité du programme.
        Il s'agit de inc.tar.

  Liste des corrections 1.7.2.0:

      - Une archive corrompue a été ajoutée aux archives test pour vérifier la fiabilité du calcul du checksum.
      - La décompression n'est désormais plus indépendante de l'extraction : tout se fait en même temps.
        Une petite fonction loadzlib() s'occupe de précharger dynamiquement avec dlopen les fonctions nécessaires
        au traitementdes archives compressées. La taille d'un tar.gz n'est donc plus limité à 64Ko (mais par le système
        lui-même).
      - Grâce, entres autres, au point précédent, le programme a été largement optimisé en mémoire, les malloc/realloc et
        et les free sont désormais moins nombreux. Le programme se rapproche alors des exigences du logiciel embarqué.
      - La durabilité a été accure : des appels à fflush() avant les printf dans la sortie standard permettent d'assurer
        correctement l'ecriture dans stdout par un threads avant que celui ci ne termine. La durabilité à l'écriture des
        éléments a déjà été ajoutée en 1.6.
      - Les ressources sont désormais bien mutexées, tous les cas de figures utilisant des threads (et par complémentarité,
        pas de threads) devrait fonctionner correctement. Des flags d'EOF et de corruption ont été déplacés en variables
        globales pour être utilisés par les threads afin d'être terminés correctement. La parallélisation crée bien tous
        les éléments, amélioration de l'algorithme de traitement. Correction du main.
      - A voir pour des linkname en path absolu. Cela fonctionne étrangement meme pour des archives étrangères...

  Liste des corrections 1.7.3.:

      - Cette version a pour vocation d'être distribuée (au client par exemple).
      - Le dépôt sur la branche master a été nettoyé et rangé pour un release final propre. Les options de débug comme
        l'option -g de gcc ont également été enlevées. Le code est également allégé de tout commentaires de débug. Le
        code a également été espacé et nettoyé.
      - Un début de rapport de projet est également déposé.
      - Il est à noter que PTAR gère une archive d'au plus 2048 dossiers différents. Autrement l'attribution de leur date
        de modification n'est pas toujours assurée. Cette valeur est purement arbitraire et peut être modifiée dans utils.h
        dans la constante MAXDIR.




###Debug

	Pour observer le code brute d'une fichier et son affichage

		hexdump -C testfalsearch.tar
		hexdump -C testf.tar
		hexdump -C testall.tar

  Pour observer le contenu d'un .o (table des symboles)

      nm main.o
      nm utils.o
      nm checkfile.o

  Pour voir les includes à la compilation, ajouter aux CFLAGS du Makefile

      -I<path>

  Pour déboguer avec GDB, il est préférable d'ajouter aux CFLAGS du Makefile (génère une table des symboles pour débug)

      -g

  Le fichier logfile.txt généré lors de l'extraction si l'option -e est spécifiée contient les codes de retours des fonctions utilisées pendant l'extraction.
  Générer un logfile lors de l'extraction/décompression d'une archive

      ./ptar -xzep 3 archive_test/testall.tar

  Pour compter les threads utilisés dans le programme, lancer cette commande dans un terminal parallèle

      ps -T -C ptar

  Pour bien charger la bibliotheque dynamique, il faut parfois bien set la variable d'environnement LD_LIBRARY_PATH

      export LD_LIBRARY_PATH=path_du_raccourci.so

  ptar vérifie la somme de contrôle (checksum) des fichiers de l'archive, si l'un des éléments est corrompu, ptar termine et renvoie une erreur.
