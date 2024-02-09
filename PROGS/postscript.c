#include "postscript.h"
#include "data.h"

#include <stdio.h>
#include <pthread.h>

// generate cell postscript in postscript_string field of the cell
// call set_font_size before using this function

void compute_single_postcript (int line, int column) {
    int x,y,delta_x, delta_y,val_x,val_y;
    char *text;
        
    delta_x = (max_length+1) * RATIO_DELTA_X * font_size;
    delta_y = RATIO_DELTA_Y * font_size;

    y = ORIG_Y + delta_y * (number_of_lines-(line+1));
    x = ORIG_X + delta_x * (column+1);

    val_x = x + (int) ((float) delta_x / 12.0);
    val_y = y + (int) ((float) delta_y / 4.0);

    text=pascal_cells[line][column].postscript_string;

    text += sprintf(text,"newpath %u %u moveto ",x,y);
    text += sprintf(text,"%u %u lineto ",x+delta_x,y);
    text += sprintf(text,"%u %u lineto stroke ",x+delta_x,y+delta_y);
    text += sprintf(text,"stroke ");
    text += sprintf(text,"newpath %u %u moveto ", x,y);
    text += sprintf(text,"%u %u lineto ",x,y+delta_y);
    text += sprintf(text,"%u %u lineto stroke ",x+delta_x,y+delta_y);
    text += sprintf(text,"stroke ");
    text += sprintf(text,"/Courrier findfont %u scalefont setfont ",font_size);
    text += sprintf(text,"newpath %u %u moveto ",val_x, val_y);
    text += sprintf(text,"(%s) show \n",pascal_cells[line][column].string_value);
}

// compute (l,c) form cell address and call compute_single_postcript
void *thread_compute_single (void *p) {
    int n,l,c;
    pascal_cell_t *ptr=p;

    n = ptr-(pascal_cell_t *)pascal_cells;
    c = n%(MAX_PASCAL+1);
    l = n/(MAX_PASCAL+1);
    compute_single_postcript (l,c);
    // dummy return value
    return ptr;
}

// call set_font_size before using this function

void compute_postscript () {
    int l,c;

    // for(l=0;l<=number_of_lines;l++) {
    //     for (c=0;c<=l;c++) {
    //         thread_compute_single(&(pascal_cells[l][c]));
    //     }
    // }

    for(l=0;l<=number_of_lines;l++) {
     for (c=0;c<=l;c++) {
             pthread_create(&((pascal_cells[l][c]).thread_id), NULL, thread_compute_single, (&(pascal_cells[l][c])));
        }
    }
    
 for(l=0;l<=number_of_lines;l++) {
         for (c=0;c<=l;c++) {
             pthread_join((pascal_cells[l][c]).thread_id, NULL);
         }
     }
    }

