#include "lire_args.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "data.h"

void usage () {
    fprintf (stderr, "usage : \n"
                    "./triangle number_of_lines font_size output_file_name ( - for standard output)\n"
                    " max : %d lines\n\n",MAX_PASCAL);
    exit (EXIT_FAILURE);
}

void lire_args (int argc, char *argv[]) {
 
  if ((sscanf(argv[1],"%d",&number_of_lines) != 1) || (number_of_lines > MAX_PASCAL)) {
      fprintf (stderr, "Illegal number of lines : %s\n",argv[1]);
      usage();
  }

  if (sscanf(argv[2],"%d",&font_size) != 1) {
      fprintf (stderr, "Illegal font size : %s\n",argv[2]);
      usage();
  }
      
  if (argc == 4) { strcpy(output_file_name,argv[3]);}
  else if (argc == 3) { ;}
  else { usage ();}
 
}

