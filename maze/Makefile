CC = gcc
GEN = generator
SOL = solver
SOL_FULL = solver_full
CFLAGS = -g -Wall -Wextra -std=c99

GEN_OBJS = generator.c maze.c
SOL_OBJS = solver.c maze.c

all: solver generator solver_full

generator: $(GEN_OBJS)
	$(CC) $(CFLAGS) -o $(GEN) $(GEN_OBJS)  -lm

solver: $(SOL_OBJS)
	$(CC) $(CFLAGS) -o $(SOL) $(SOL_OBJS)

solver_full: $(SOL_OBJS)
	$(CC) $(CFLAGS) -o $(SOL_FULL) -DFULL $(SOL_OBJS)

clean:
	rm -f $(GEN) $(SOL) $(SOL_FULL)
