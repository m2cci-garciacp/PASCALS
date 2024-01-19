#include "write_file.h"
#include "data.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void cell_to_output_file (FILE *foutput, int line, int column) {
    fprintf(foutput,"%s",pascal_cells[line][column].postscript_string);
    fflush(foutput);
}

void write_file () {
    int l,c;
    
    FILE *foutput = stdout;
    
    // Open file
    // Use stdout if filename is -
    if (strcmp(output_file_name, "-")) {
    foutput = fopen (output_file_name,"w");
    if (foutput == NULL) {
        fprintf (stderr, "Error on opening %s for writing \n",output_file_name);
    }
    }
    
    // Write file
    for(l=0;l<number_of_lines;l++) {
        for (c=0; c<=l; c++) {
            cell_to_output_file(foutput,l,c);
        }
    }
    
    // Close file
    if (foutput != stdout) { fclose(foutput); }
}

