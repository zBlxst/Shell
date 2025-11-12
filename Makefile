TMPFILES = lex.c parse.c
MODULES = main parse builtins
OBJECTS = $(MODULES:=.o)
CC = gcc  
CC_OPTIONS = -g -Wall -Wno-stringop-overflow
LINK = $(CC)
LIBS = -lreadline

shell: $(OBJECTS)
	$(LINK) $(OBJECTS) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CC_OPTIONS) -c $<

parse.o: parse.c lex.c
parse.c: parse.y global_parser.h
	bison parse.y -o $@
lex.c: lex.l
	flex -o$@ lex.l

clean: 
	rm -f shell $(OBJECTS) $(TMPFILES) *.o
