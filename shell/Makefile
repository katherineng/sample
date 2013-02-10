EXEC =		sh
SRC = 		sh.c jobs.c
CFLAGS =    -g3 -Wall -Wextra -Wconversion -Wcast-qual -Wcast-align
CFLAGS +=   -Winline -Wfloat-equal -Wnested-externs
CFLAGS +=   -pedantic -std=c99 -Werror -D_GNU_SOURCE
NOPROMPT =	-D NO_PROMPT=1
CC =		/usr/bin/gcc

.PHONY: default
default: $(EXEC)

.PHONY: all
all: $(EXEC) noprompt

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@

noprompt: $(SRC)
	$(CC) $(CFLAGS) $(NOPROMPT) $(SRC) -o $@

.PHONY: clean
clean:
	rm -f *.o $(EXEC) noprompt
