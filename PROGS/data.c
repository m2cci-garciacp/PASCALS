#include "data.h"

pascal_cell_t pascal_cells[MAX_PASCAL+1][MAX_PASCAL+1];
int number_of_lines;
int font_size;

int max_length;
char max_format[6];

char output_file_name[MAX_FILENAME_LENGTH+1] = "";
char output_errfile_name[MAX_FILENAME_LENGTH+1];
char output_html[MAX_HTML];
