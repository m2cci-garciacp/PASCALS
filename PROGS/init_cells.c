#include "init_cells.h"
#include "data.h"
#include "postscript.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

// fills the value, max_value and format for value fields.

void compute_cells_values () {
    int l,c,value,max_value;
    char str[STRING_VALUE_LENGTH];

    max_value = 0;
    for (l=0;l<=number_of_lines;l++) {
        pascal_cells[l][l].value = 1;
        pascal_cells[l][0].value = 1;
        for (c=1;c<l;c++) {
        value =  pascal_cells[l-1][c-1].value + pascal_cells[l-1][c].value;
        pascal_cells[l][c].value = value;
        if (value > max_value) { max_value = value;}
        }
    }
    // define the "%xxd" format for string_value
    sprintf(str,"%d",max_value);
    max_length = strlen(str);
    sprintf(max_format,"%%""%dd ",max_length);
}

// malloc and fill the string_value field
void compute_cells_strings () {
    int l,c;
    void *m;
    for(l=0;l<=MAX_PASCAL;l++) {
        for (c=0;c<=l;c++) {
            sprintf ( pascal_cells[l][c].string_value, max_format, pascal_cells[l][c].value);
            m=malloc(POSTSCRIPT_CELL_LENGTH); 
            if (m == NULL) {
                fprintf(stderr,"compute_cells_strings : malloc error");
                exit(EXIT_FAILURE);
                }
            pascal_cells[l][c].postscript_string=m;
        }
    }
}

void set_font_size(int size) {
    font_size=size;
}

// display string_value field (debugging only)
void display_cells_values () { 
    int l,c;
    printf ("max = %d\n",max_length);
    for(l=0;l<=number_of_lines;l++) {
        for (c=0;c<=l;c++) {
        printf ("%s",pascal_cells[l][c].string_value);
        }
        printf ("\n");
    }
    printf ("format : %s\n",max_format);
    }

