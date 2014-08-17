
# Ziyan M. Based on the following sources:
# http://babbage.cs.qc.edu/courses/cs701/Handouts/using_make.html
# http://www.cs.toronto.edu/~penny/teaching/csc444-05f/maketutorial.html
#Changelog
#2013-03-06: Initial version 
#2014-02-10: Rewrite rules for testing
#2014-03-08: Variables for pre/post processing

#Compiler to use 
CC	:=	cc
#Compiler options
CFLAGS	+=	-pthread -g -pedantic -W -Wall -Wundef -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations
#Preprocessor flags e.g. include paths
CPPFLAGS+=	-I.
#Linker flags e.g. -lm for the math library
#LDFLAGS	:=	-lmcheck
#List of source files
SRCS	:=	$(wildcard *.c)
#List of test input files
#Name of compiled binary
BINARY	:=	chatroom

.INTERMEDIATES : .exp .out .result
.PHONY : clean test 

$(BINARY) : $(SRCS:.c=.o)
	@$(LINK.c) $^  -o $@

%.d : %.c
	@$(CC) -MM $(CPPFLAGS) $< | sed 's/$*.o/& $@/g' > $@

clean:
	-rm -f *.result *.exp *.out *.o *.d core $(BINARY)

#header file dependencies
-include $(SRCS:.c=.d)

