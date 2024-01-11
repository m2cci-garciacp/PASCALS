# TRIANGLE DE PASCAL



## 5 Création d’un tube entre triangle et gs

Pour écrire les rédirections du tube, il a fallu utiliser des fonctions *write* et *read* pour tester la communication entre les fils. 
Et puis, avec *cat* j'ai imprimé le résultat du *write_file* dans le fils droite.

Finalement, la visualisation graphique avec le logicile GhostScript permet d'afficher le triangle de Pascal, par contre une fois fermé la fenêtre graphique, le processus 
continue a tourner sans pouvoir sortir comme d'habitude avec la commande *quit*. Pour rémedier cela, le triangle est montré sur 
GhostView, qui n'a pas possé de problèmes à ce niveau-la.


```c
int main (int argc, char *argv[], char *envp[]) {
    lire_args(argc,argv);
    compute_cells_values ();
    compute_cells_strings ();
    compute_postscript ();
  	
	int p[2];
	int pid1, pid2;
	
	// Creer une tube avant le premier fork
	pipe(p);
	pid1 = fork();
	
	if (!pid1) {
		// Fils gauche : ecrit 
		// Redigire la sortie std vers l'entrée du tube : on ecrit dans le tube
		dup2(p[1],1);
		// On peut fermer les deux extremites du tube: p[0] ne sera pas utilisée et p[1] est maintenant sur 1.
		close(p[0]);
		close(p[1]);
		// Appel de la fonction qui écrit le ps.
		write_file ();
		exit(0);
	}
	else {	
		// Pere  : creer un deuxième fils  
		pid2 = fork();	
		if (!pid2) {
			// Fils droit : recoit 
			// Diriger la sortie du tube vers l'entrée std : on lit du tube.
			dup2(p[0],0);
		    // On peut fermer les deux extremites du tube: p[1] ne sera pas utilisée et p[0] est maintenant sur 0.
			close(p[1]);
			close(p[0]);
		    // Execution de gv avec parametres entrée par la entrée std (0).
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

Voici un extract du code utilisé avec des commentaires dans les parties importantes. En plus, ceci a été implementé dans le programme du point 4,
 où le troisième argument de l'executable est le fichier de sauvegarde du triangle.
