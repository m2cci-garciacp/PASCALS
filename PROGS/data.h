#ifndef DATA_H
#define DATA_H

#include "dimensions.h"
#include <pthread.h>

typedef struct {
    unsigned int value;
    unsigned delta_x;                      
    unsigned delta_y;
    char string_value[STRING_VALUE_LENGTH];  // value number as a string
    char *postscript_string;                 // postscript sequence to draw the cell
    pthread_t thread_id;                     // for thrad parallelism
} pascal_cell_t;  

extern pascal_cell_t pascal_cells[MAX_PASCAL+1][MAX_PASCAL+1];

extern int number_of_lines;       // number of lines to compute/display
extern int font_size;

extern int max_length;            // max number of digits of a cell
extern char max_format[6];        // string format for the number of digits

extern char output_file_name[MAX_FILENAME_LENGTH+1];
extern char output_errfile_name[MAX_FILENAME_LENGTH+1];
extern char output_html[MAX_HTML];

#endif // DATA_H
