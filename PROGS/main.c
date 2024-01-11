#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "init_cells.h"
#include "postscript.h"
#include "html.h"
#include "write_file.h"
#include "data.h"
#include "lire_args.h"


char * arguments [] = {"/usr/bin/gv", NULL ,NULL};
char prog_suivant [] = "/usr/bin/gv";

int main (int argc, char *argv[], char *envp[]) {
    lire_args(argc,argv);
    compute_cells_values ();
    compute_cells_strings ();

    compute_postscript ();
    
  	if (argc==3) {
  		int p[2];
  		pipe(p);
		int pid =fork();
	  
	  	if (!pid) {
			// fils gauche
			dup2(p[1],1);
			close(p[0]);
			close(p[1]);
			write_file ();
			exit(0);
	  	}
		else {	    
			int pid2 =fork();
		
			if (!pid2) {
				// fils droite
				dup2(p[0],0);
				close(p[1]);
				close(p[0]);
				char * arguments2 [] = {"gv", "-", NULL};
				execv("/usr/bin/gv", arguments2);
				exit(0);
			} 
			else {
				// pere
				wait(NULL);
				wait(NULL);
				printf("GV fini...\n");
				return 0;
			}
		}
    
	}
	
	write_file ();
	sleep (3);   // wait 3 second to allow gs to display the picture
	
	if (argc==4) {
		arguments[1] = output_file_name;
		
		int pid =fork();
		if (!pid) {
			execv(prog_suivant, arguments);
		}
		else {
			wait(NULL);
			printf("Génération et affichage du triangle de Pascal terminés\n");
		}
	} 

	return 0;
}
