# Makefile for SYSV

CC      = gcc
PROF    = -O -ggdb3
NOCRYPT =
C_FLAGS =  -Wall $(PROF) $(DEBUG) $(NOCRYPT) -IINC
L_FLAGS =  $(PROF) $(DEBUG)

C_FILES = $(wildcard *.c)
H_FILES = $(wildcard INC/*.h)
O_FILES = $(patsubst %.c,OBJ/%.o,$(C_FILES))

rom: $(O_FILES)
	rm -f rom
	$(CC) $(L_FLAGS) -o rom $(O_FILES)

OBJ/%.o: %.c $(H_FILES)
	$(CC) -c $(C_FLAGS) -o $@ $<

cleaner:
	-rm -f $(O_FILES)

clean:	cleaner rom

checkin:
	
