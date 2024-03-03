# TRIANGLE DE PASCAL

## 2 Lancement de l’interprète Postscript par exec
La commande which permet de déterminer dans quel répertoire se trouve le fichier exécutable d’une commande. Dans mon cas, l'absence de reponse m'a indiqué que'il n'était pas installé (il devrait être installé sur /usr/bin et ce chemin est dans la variable PATH). 

Ici, on veut que la commande `$ ./triangle 4 12 - 1> triangle.ps` sauvegarde le triangle dans le fichier `triangle.ps` et puis, l'affiche avec GhostView.
Pour cela, on execute les fonctions principales du *main* (`lire_args`, `compute_cells...`, `compute postcript`, `write file`) et puis, après un temps de pause de 3 secondes, on appele l'executable de gv pour afficher le triangle avec un `execv` (j'ai opté pour `execv` en lieu de `execve` car on n'a pas besoin de transmettre des nouvelles variables d'environnement. Autrement, on aurait pu aussi renvoyer les variables d'environnement reçues dans le main avec `execve(prog_suivant, arguments, envp)`).

```c
char * arguments [] = {"/usr/bin/gv", NULL ,NULL};
char prog_suivant [] = "/usr/bin/gv";

int main (int argc, char *argv[], char *envp[]) {
	// Fonctions principales
    lire_args(argc,argv);
    compute_cells_values ();
    compute_cells_strings ();
    compute_postscript ();
    write_file ();

	// Attendre 3 secondes
    sleep (3);
	
	// Ajouter le nom du fichier cree aux arguments de exec
	arguments[1] = "triangle.ps";
		
	// Appel a gv
	execv(prog_suivant, arguments);

    return 0;
}
```

---
## 3 Lancement de gs/gv par fork et exec
Pour attendre la fin du `execv`, on ne peut pas simplement écrire de code après, car `exec` reécrit tout la suite du programme. Pour attendre la fin du `exec`, on peut créer une duplication de processus avec `fork` et attendre la fin du fils avec `wait`.

En partant du code de la section 2, j'ai ajouté une duplication `fork` et l'appel `exec` est mis dedans le code du fils (pid=0), donc le père connait son pid et peut l'attendre avec le `wait`. Après le message est affiche avec un simple `printf`.

```c
int main (int argc, char *argv[], char *envp[]) {
	...
	arguments[1] = "triangle.ps";

	int pid;
	// Dupliquer processus
	pid = fork();
	// Processus fils
	if (!pid) {
		// appel a gv
		execv(prog_suivant, arguments);
	}
	// Processus père
	else {
		// Attendre fils
		wait(pid);
		// Écrirer message finale sur l'ecran
		printf("Génération et affichage du triangle de Pascal terminés\n");
	}
	
	return 0;
}
```

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
L'idée dans cette section est de paralléliser l'écriture des cellules du triangle. Pour cela, on va creer un thread pour chaque celulle, et puis un RDV a N participants (tous les threads) pour continuer l'execution du programme. On peut resoudre ce problème avec N semaphores, où la resource est la finalisation du thread correspondant et est initialisée à 0 (à la creation du thread, ceci n'est pas fini).  
Étant donnée la simplicité du problème (nos semaphores sont binaires (fini ou pas fini) et on doit réaliser un seul RDV), on peut s'epargner des variables semaphores, et utliser un simple *wait terminaison* du thread.

Pour créer les threads, on sustitue l'appel de la fonction `thread_compute_single` par `pthread_create` avec cette fonction et ses arguments comme paramètres. Pour le RDV à plusieurs, on doit dupliquer les boucles, et puis realiser le *wait terminaison* du thread, qui est donné par la primitive `pthread_join` et qui permet de rassembler tous les fils d'execution avant continuer.


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
		// Creation d'un thread
        pthread_create(&((pascal_cells[l][c]).thread_id),     // thread
					   NULL,                                  // attributes du thread
					   thread_compute_single,                 // fonction a paralleliser
					   &(pascal_cells[l][c]));                // arguments de la fonction a paralléliser
    }
}

for(l=0; l<=number_of_lines; l++) {
    for (c=0; c<=l; c++) {
		// Attente des threads.
        pthread_join((pascal_cells[l][c]).thread_id,          // thread
					 NULL);                                   // attributes du thread
    }
}
```

Étant donnée la legère charge de travail pour chaque thread, le parallélisme ici n'ameliore pas la performance du programme, par contre, on peut observer les appels systèmes executées avec `strace -f`, où on observe les appels `clone` correspondants à les créations des threads.