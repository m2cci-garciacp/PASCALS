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
vos messages de trace (sinon ils iront se mélanger aux ordres postscript dans le fichier triangle.ps).

A TESTER
```c
char * arguments [] = {"/usr/bin/gv", NULL ,NULL};
char prog_suivant [] = "/usr/bin/gv";

int main (int argc, char *argv[], char *envp[]) {
    lire_args(argc,argv);
    compute_cells_values ();
    compute_cells_strings ();

    compute_postscript ();

    write_file ();
    sleep (3);   // wait 3 second to allow gs to display the picture
	
	// Ajouter le nom du fichier cree aux arguments de exec
	arguments[1] = output_file_name;
		
	// appel a gs
	execv(prog_suivant, arguments);

  return 0;
}
```

## 3 Lancement de gs/gv par fork et exec
Pour completer la practique sur le fork et exec, on peut calculer le triangle dans le programme principal, et puis dupliquer le processus avec fork pour lui dessiner sur gs.

```c
int main (int argc, char *argv[], char *envp[]) {
	char * arguments [] = {"/usr/bin/gs", NULL , NULL};
	char prog_suivant [] = "/usr/bin/gs";
	int pid;

	...

	// Ajouter le nom du fichier cree aux arguments de exec
	arguments[1] = output_file_name;

	// dupliquer processus
	pid =fork();
	// pocessus fils
	if (!pid) {
		// appel a gs
		execv(prog_suivant, arguments);
	}
	// pocessus pere
	else {
		// attendre fils
		wait(NULL);
		// ecrirer message finale sur l'ecran
		printf("Génération et affichage du triangle de Pascal terminés\n");
	}
```
Pour cet exercise, j'ai opté pour ``execv`` en lieu de ``execve`` car on n'a pas besoin de transmettre des nouvelles variables d'environnement. Autrement, on aurait pu aussi renvoyer les variables d'environnement reçues dans le main avec ``execve(prog_suivant, arguments, envp)``

---

## 4 Génération du Postscript dans un fichier
Le code source l'application de triangle Pascal ne semble pas marcher correctement quand on ajoute un fichier de sauvegarde (3ème paramètre): le fichier résultant ne contient qu'un seule carreau avec la valeur 1.

En analysant le code, on découvre un erreur dans le code. En appelant la fonction ``write_file`` on itère sur les différent cellules à dessiner, et on appele la fonction ``cell_to_output_file`` pour chaque cellule.
Cette fonction (résumé ci-dessous) ouvre en écriture le fichier destination, écrit la valeur correspondant à la cellule, et puis ferme le fichier.
```c
void cell_to_output_file (int line, int column) {
    FILE * foutput = stdout;

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
    int l, c;
    for(l=0; l<number_of_lines; l++) {
        for (c=0; c<=l; c++) {
            cell_to_output_file(l, c);;
        }
    }
}
```

Ce code efface le fichier de sauvegarde et récrit une cellule a chaque fois, donc la dernière cellule écrite est celle-ci qu'on pouvait observer (valeur ``1``).

Pour résoudre ce problème, j'ai ouvert une fois le fichier (en mode écriture), et puis avec le fichier ouvert, on itère sur les cellules pour les sauvegarder, comme montre ci-dessous.
```c
void cell_to_output_file (FILE * fname, int line, int column) {
    fprintf(fname,"%s",pascal_cells[line][column].postscript_string);
    fflush(fname);
}

void write_file () {
    FILE * foutput = stdout;

    // Ouvrir fichier si l'argument n'est pas '-'
    if (strcmp(output_file_name, "-")) {
        foutput = fopen (output_file_name, "w");
        ...
    }
    // Itération sur les cellules et sauvegarde
    int l, c;
    for (l=0; l<number_of_lines; l++) {
        for (c=0; c<=l; c++) {
            cell_to_output_file(foutput, l, c);;
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

La commande ``diff`` nous permet de voir les différences entre les deux fichiers. On peut aussi vérifier visuellement avec GhostView en affichant chaque fichier.

---

## 5 Création d’un tube entre triangle et gs

Pour écrire les redirections du tube, il a fallu utiliser des fonctions ``write`` et ``read`` pour tester la communication entre les fils.
Et puis, avec ``cat`` j'ai imprimé le résultat du ``write_file`` dans le fils droite.

Finalement, la visualisation graphique avec le logiciel GhostScript permet d'afficher le triangle de Pascal, par contre une fois fermé la fenêtre graphique, le processus
continue a tourner sans pouvoir sortir comme d'habitude avec la commande ``quit``. Pour remédier cela, le triangle est montré sur
GhostView, qui n'a pas posé de problèmes à ce niveau-la.


```c
int main (int argc, char *argv[], char *envp[]) {
    lire_args(argc, argv);
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
  			dup2(p[1], 1);
  			// On peut fermer les deux extrémités du tube: p[0] ne sera pas utilisée
			// et p[1] est maintenant sur 1.
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
  				dup2(p[0], 0);
  			    // On peut fermer les deux extrémités du tube: p[1] ne sera pas
				// utilisée et p[0] est maintenant sur 0.
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

## 6 Générateur de triangle en serveur html
On part d'un serveur TCP, qui tourner sur le port 8080. On peut y acceder avec l'adresse ``localhost:8080`` ou ``127.0.0.1:8080``. Le valeur par defaut de étages pour le triangle généré par le serveur est 8, par contre, on peut facilement ajouter un paramètre a la commande, pour customizer notre triangle au serveur.
```c
// Changer
 number_of_lines = 8;  // default size 
 
 // Par
 number_of_lines = analyze_client_request();
 ```
 

On peut aussi faire des requetes avec la ligne de commande avec ``wget`` ou ``curl``. ``wget`` peut telecharger le fichier directe avec
```wget 127.0.0.1:8080/15``` et le fichier téléchargé est nommé ``15``, si c'est sans paramètre, le fichier téléchargé est nommé ``index.html``. Par contre,
```curl 127.0.0.1:8080/15``` montre le triangle a l'ecran, donc il faut le rediriger avec ``>``.

Le serveur fourni, n'accepte qu'une seule connexion. Avec les outils étudiés en cours, on peut facilement modifier le serveur pour qui écoute autant de requetes que necessaire.
Pour cela, on reajoute un boucle while true  apres la primitive listen et puis, une fois acceptée la connexion, on fait un fork pour le fils, qui gere la requte et le père continue a ecouter pour une nouvelle connexion:
```c
	// ecouter pour connexions entrantes
	listen(server_socket, ...)
    while (1) {
		// accepter connexion entrante
        client_socket = accept(server_socket, ...);
		// dupliquer processus
        pid = fork();
		// fils
        if (pid == 0){
			// fermer socket serveur, on a pas besoin de deux
            close(server_socket);
            // gerer la demande du client
			length = read(client_socket, client_message, sizeof(client_message));
			...
            create_answer();
			...
            length = write(client_socket, server_message, strlen(server_message));
			// fermer socket client
            close(client_socket);
			// finir processus
            exit(0);
		}
		// père
        else {
			// fermer socket client pour continuer a ecouter
            close(client_socket);
        }
    }
    close(server_socket);
```

---

## 7 Parallélisation de génération du post-script
Pour parallélisér la création des cellules, on sustitue l'appel de la function ``thread_compute_single`` par la création d'un thread avec cette fonction comme paramètre. Il faut aussi dupliquer les boucles embriquées car on doit d'abord créer les threads et puis les attendre.

Voici l'extrait du code que j'ai remplacé
 ```c
for(l=0; l<=number_of_lines; l++) {
    for (c=0; c<=l; c++) {
        thread_compute_single(&(pascal_cells[l][c]));
    }
}
```
par
```c
for(l=0; l<=number_of_lines; l++) {
    for (c=0; c<=l; c++) {
        pthread_create(&((pascal_cells[l][c]).thread_id), NULL, thread_compute_single, &(pascal_cells[l][c]));
    }
}

for(l=0; l<=number_of_lines; l++) {
    for (c=0; c<=l; c++) {
        pthread_join((pascal_cells[l][c]).thread_id, NULL);
    }
}
```

Etant donné la legère charge de travail pour chaque thread, le parallélisme ici n'ameliore pas la performance du programme, par contre, on peut observer les appels systèmes executées avec ``strace -f``, où on observe les appels ``clone`` correspondants à les créations des threads.