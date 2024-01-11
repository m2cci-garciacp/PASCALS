#ifndef DIMENSION_H
#define DIMENSION_H

#ifndef MAX_PASCAL
#define MAX_PASCAL 18    // max number of lines 
#endif // MAX_PASCAL

#define STRING_VALUE_LENGTH 10  // max string size for a cell value
#define POSTSCRIPT_CELL_LENGTH 1000  // postscript size per cell

                         // (x,y) start of triangle figure
#define ORIG_X 10
#ifndef ORIG_Y
#define ORIG_Y 200		   
#endif

#define RATIO_DELTA_X 0.6	   // font size to x width
#define RATIO_DELTA_Y (1.0 + .25)  // vertical font size to cell size

#define MAX_FILENAME_LENGTH 10000
#define MAX_HTML 100000            // triangle in  html 

#endif // DIMENSIONS_H
