CC=gcc
LD=gcc

# To change vertical position, un comment this line ;
#DEFS= -DORIG_Y 300

CFLAGS= -c -g -Wall ${DEFS}
LDFLAGS= -static -lpthread

TRIANGLE_OBJECTS = lire_args.o data.o postscript.o write_file.o init_cells.o html.o


all: triangle server

clean:
	/bin/rm -f ${TRIANGLE_OBJECTS} main.o server.o triangle.ps triangle.html


main.o:	main.c init_cells.h postscript.h write_file.h data.h lire_args.h dimensions.h html.h

server.o:	server.c init_cells.h write_file.h data.h lire_args.h dimensions.h html.h

lire_args.o:	lire_args.c lire_args.h data.h dimensions.h

data.o:		data.c data.h dimensions.h

init_cells.o:	init_cells.c init_cells.h data.h dimensions.h postscript.h

postscript.o:	postscript.c postscript.h data.h dimensions.h

html.o:		html.c html.h data.h dimensions.h

write_file:	write_file.c write_file.h data.h dimensions.h

triangle:	${TRIANGLE_OBJECTS} main.o
		${LD} $^ ${LDFLAGS}  -o triangle

server:		${TRIANGLE_OBJECTS} server.o
		${LD} $^ ${LDFLAGS}  -o server

# Demos
GV= /usr/bin/gv
GS= /usr/bin/gs

demo_gv:	triangle
		echo "demo_gs"; ./triangle 6 12 - | ${GV} -

demo_gs: 	triangle
		./triangle 6 12 - | ${GS} -sDEVICE=x11 -q

