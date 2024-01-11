#include "html.h"
#include "data.h"

#include <stdio.h>

// generate cell postscript in postscript_string filed of the cell
// call set_font_size before using this function

// fills text with the html triangle
// returns the size of the html

int generate_html (char *text) {
    int l,c;
    char *begin = text;
    
    text += sprintf(text,"<!doctype html>\n" "<html lang=\"fr\">\n\n");

    text += sprintf(text,"<table>\n");

    for (l=0;l<number_of_lines;l++) {
        text += sprintf(text,"  <tr>\n");

        for (c=0;c<=number_of_lines;c++) {
            text += sprintf(text,"   <td> ");
            if (c <= l) { text += sprintf(text,"%d",pascal_cells[l][c].value); }
            text += sprintf(text," </td>\n");
            }

        text += sprintf(text,"  </tr>\n");
    }   
    text += sprintf(text,"</table>\n");
    text += sprintf(text,"<caption> Le triangle de Pascal (%d) </caption>\n",number_of_lines);

    text += sprintf(text,"</html>\n");
    return text-begin; 
}

