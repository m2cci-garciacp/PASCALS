# TRIANGLE DE PASCAL

## 2 Lancement de l’interprète Postscript par exec
La commande which permet de déterminer dans quel répertoire 1 se trouve le fichier exécutable d’une commande. Vérifier dans quel(s) répertoire(s) se trouvent les interprètes Postscript gv et gs. Dans la suite, nous supposerons qu’ils sont dans /usr/bin.
1. du chemin d’accès aux commandes défini par la variable d’environnement PATH
3mandelbrot> which gv
mandelbrot> which gs
Avec la redirection, on peut afficher le triangle des deux manières suivantes (il faut saisir la commande quit dans la fenêtre terminal pour terminer l’exécution de gs) :
mandelbrot> ./triangle 4 12 - > triangle.ps
mandelbrot> /usr/bin/gv triangle.ps
mandelbrot> /usr/bin/gs -q -sDEVICE=x11 triangle.ps
GS> quit
On veut maintenant que la première commande exécute automatiquement gv ou
gs sur le fichier triangle.ps pour afficher le résultat.
mandelbrot> ./triangle 4 12 - > triangle.ps
Pour celà, il suffit d’utiliser la primitive exec à la fin de la procédure main pour lancer gv. Inspirez-vous de l’exemple d’utilisation de exec du tp sur les processus (notament dans forkecho, forkps).
Modifier le code, recompiler, tester (ne pas oublier de taper q pour terminer l’exécution de gv).
Attention : Pensez à la sortie standard d’erreur (qui n’est pas redirigée) pour
vos messages de trace (sinon ils iront se mélanger aux ordres postscript dans le fichier triangle.ps

---

## 4 Génération du Postscript dans un fichier
Le code source l'application de triangle Pascal ne semble pas marcher correctement quand on ajoute un fichier de sauvegarde (3ème paramètre): le fichier résultant ne contient qu'un seule carreau avec la valeur *1*.

En analysant le code, on découvre un erreur dans le code. En appelant la fonction *write_file* on itère sur les différent cellules à dessiner, et on appele la fonction *cell_to_output_file* pour chaque cellule.
Cette fonction (résumé ci-dessous) ouvre en écriture le fichier destination, écrit la valeur correspondant à la cellule, et puis ferme le fichier.
```c
void cell_to_output_file (int line, int column) {
  FILE *foutput = stdout;

  // Use stdout if filename is -
  if (strcmp(output_file_name,"-")) {
     foutput = fopen (output_file_name,"w");
     ...
  }
  fprintf(foutput,"%s",pascal_cells[line][column].postscript_string);
  fflush(foutput);

  if (foutput != stdout) { fclose(foutput); }
}

void write_file () {
  int l,c;
  for(l=0;l<number_of_lines;l++) {
     for (c=0;c<=l;c++) {
        cell_to_output_file(l,c);;
     }
  }
}
```

Ce code efface le fichier de sauvegarde et récrit une cellule a chaque fois, donc la dernière cellule écrite est celle-ci qu'on pouvait observer (valeur *1*).

Pour résoudre ce problème, j'ai ouvert une fois le fichier (en mode écriture), et puis avec le fichier ouvert, on itère sur les cellules pour les sauvegarder, comme montre ci-dessous.
```c
void cell_to_output_file (FILE * fname, int line, int column) {
  }
  fprintf(fname,"%s",pascal_cells[line][column].postscript_string);
  fflush(fname);
}

void write_file () {
  FILE *foutput = stdout;

  // Ouvrir fichier si l'argument n'est pas '-'
  if (strcmp(output_file_name,"-")) {
     foutput = fopen (output_file_name,"w");
       ...
  }
  // Itération sur les cellules et sauvegarde
  int l,c;
  for(l=0;l<number_of_lines;l++) {
     for (c=0;c<=l;c++) {
        cell_to_output_file(foutput, l,c);;
     }
  }

  // Fermer fichier
  if (foutput != stdout) { fclose(foutput); }
}
```

Pour vérifier que ceci marche correctement, on peut exécuter la suite de commandes suivante:
```bash
$ ./triangle 4 12 - > triangle_redigire.ps
$ ./triangle 4 12 triangle_par_argument.ps
$ diff triangle_redigire.ps triangle_redigire.ps
```

La commande *diff* nous permet de voir les différences entre les deux fichiers. On peut aussi vérifier visuellement avec GhostView en affichant chaque fichier.

---

## 5 Création d’un tube entre triangle et gs

Pour écrire les redirections du tube, il a fallu utiliser des fonctions *write* et *read* pour tester la communication entre les fils.
Et puis, avec *cat* j'ai imprimé le résultat du *write_file* dans le fils droite.

Finalement, la visualisation graphique avec le logiciel GhostScript permet d'afficher le triangle de Pascal, par contre une fois fermé la fenêtre graphique, le processus
continue a tourner sans pouvoir sortir comme d'habitude avec la commande *quit*. Pour remédier cela, le triangle est montré sur
GhostView, qui n'a pas posé de problèmes à ce niveau-la.


```c
int main (int argc, char *argv[], char *envp[]) {
    lire_args(argc,argv);
    compute_cells_values ();
    compute_cells_strings ();
    compute_postscript ();

  	int p[2];
  	int pid1, pid2;

  	// Créer une tube avant le premier fork
  	pipe(p);
  	pid1 = fork();

  	if (!pid1) {
  			// Fils gauche : écrit
  			// Redirige la sortie std vers l'entrée du tube : on écrit dans le tube
  			dup2(p[1],1);
  			// On peut fermer les deux extrémités du tube: p[0] ne sera pas utilisée et p[1] est maintenant sur 1.
  			close(p[0]);
  			close(p[1]);
  			// Appel de la fonction qui écrit le ps.
  			write_file ();
  			exit(0);
  	}
  	else {
  			// Père  : créer un deuxième fils  
  			pid2 = fork();
  			if (!pid2) {
  					// Fils droit : reçoit
  					// Diriger la sortie du tube vers l'entrée std : on lit du tube.
  					dup2(p[0],0);
  			    // On peut fermer les deux extrémités du tube: p[1] ne sera pas utilisée et p[0] est maintenant sur 0.
  					close(p[1]);
  					close(p[0]);
  			    // Exécution de gv avec paramètres entrée par la entrée std (0).
  					char * arguments [] = {"gv", "-", NULL};
  					execv("/usr/bin/gv", arguments);
  					exit(0);
    		}
    		else {
    					// Pere
    					// Attente de finalisation de deux enfants.
    					wait(NULL);
    					wait(NULL);
    					printf("Triangle montré sur GV.\n");
    					return 0;
    		}
  	}
    return 0;
}
```

Voici un extrait du code utilisé avec des commentaires dans les parties importantes. En plus, ceci a été implémenté dans le programme du point 4,
 où le troisième argument de l’exécutable est le fichier de sauvegarde du triangle.

---

##  Générateur de triangle en serveur html
6.1 Découverte (observation uniquement)

L’exécutable serveur attend une requête HTTP (GET) sur le port local 8080, affiche le message de requête du client, répond en envoyant le html d’un triangle de Pascal et se termine.

Dans votre fenêtre terminal, lancer le serveur de triangle : ./triangle puis dans votre navigateur firefox saisir l’URL localhost:8080 un triangle de Pascal doit s’afficher. Dans la fenêtre terminal, le serveur doit afficher le message de requête reçu du client. Noter le / entre GET et HTTP.

Relancer le serveur dans votre fenêtre terminal. Ouvrir une autre fenêtre terminal et dans celle-ci saisir la commande suivante :
wget localhost:8080

Wget stocke la réponse du serveur dans le fichier index.html, dont vous pouvez observer le contenu avec votre éditeur de fichiers préféré (par exemple nedit ou gedit).

Vous pouvez aussi l’afficher avec la commande suivante :
firefox file:index.html dans le terminal dans lequel vous avez lancé wget.

6.2 Requête paramétrée

La syntaxe d’une URL est généralement la suivante :

nom_hote:port/param

le port étant optionel et param un paramètre de nature quelconque, par exemple un entier (dans ce tp la taille du triangle à tracer) ou un nom de fichier.

Relancer le serveur et s’y connecter via l’URL localhost:8080/12
et observer que le dans le début du message du client, entre GET et HTTP, / est devenu /12.

Modifier le code de create_answer dans le fichier server.c pour dessiner un triangle de x lignes en réponse à l’URL de la forme localhost:8080/x

Recompiler et tester.
