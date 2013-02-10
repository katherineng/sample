#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "maze.h"

Room maze[NUM_ROWS][NUM_COLS];

/**
 * extracts bits from integer
 */
void integer_to_bits(int hex, int *connections) {
  connections[EAST] = hex / 8;
  hex = hex % 8;
  connections[WEST] = hex / 4;
  hex = hex % 4;
  connections[SOUTH] = hex / 2;
  hex = hex % 2;
  connections[NORTH] = hex;
}

/**
 * reconstructs maze from hexadecimal data in the input file
 */
void reconstruct(FILE *file) {
  int i, j;
  for (i = 0; i < NUM_ROWS; ++i) {
    for(j = 0; j < NUM_COLS; ++j) {
      Room *r = &maze[i][j];
      r->visited = 0;
      int hex;
      fscanf(file, "%1x", &hex);
      integer_to_bits(hex, r->connections);
    }
  }
}

/**
 * depth-first search begins at (x, y) and explores adjacent,
 * accessible rooms recursively until (goal_x, goal_y) is found.
 * if program is compiled with macro FULL, prints out the entire 
 * path traversed, including backtracking.
 * otherwise, prints visited rooms that are part of the final 
 * route in reverse.
 */
int dfs(int x, int y, int goal_x, int goal_y, FILE *file) {
  if (x == goal_x && y == goal_y) {
    fprintf(file, "%d, %d\n", x, y);
    return 1; // if current (x, y) is goal, return true
  }

  Room *r = &maze[y][x]; // pointer to room at (x, y) in maze
  #ifdef FULL
  fprintf(file, "%d, %d\n", x, y);
  #endif
  r->visited = 1; // set visited to true

  int dir;
  for(dir = 0; dir < 4; ++dir) { // for each direction
    int neighbor_x = x + calculate_offset(dir, 'x');
    int neighbor_y = y + calculate_offset(dir, 'y');
    Room *neighbor = &maze[neighbor_y][neighbor_x];
    
    if (!r->connections[dir] && !neighbor->visited) {
      if (dfs(neighbor_x, neighbor_y, goal_x, goal_y, file)) {
	#ifndef FULL
	fprintf(file, "%d, %d\n", x, y);
	#endif
	return 1; // if recursive call returns true, return true
      } else {
	#ifdef FULL
	fprintf(file, "%d, %d\n", x, y);
	#endif
      }
    }
  }
  return 0; // dead end, return false
}

/**
 * given an a pointer to a string in an array and the length of the
 * array, determines if the strings in the array are integers
 */
int parseable(char **input, int length) {
  int i;
  for (i = 0; i < length; ++i) {
    if ((strcmp(input[i], "0") != 0) && !atoi(input[i])) {
      return 0;
    }
  }
  return 1;
}

int main(int argc, char **argv) {
  if (argc != 7) {
    printf("Usage: %s <input> <output> <start_x> <start_y> <end_x> <end_y>\n", argv[0]);
  } else {
    FILE *in = fopen(argv[1], "r"); // open input file
    FILE *out = fopen(argv[2], "w"); // open output file

    int start_x = atoi(argv[3]);
    int start_y = atoi(argv[4]);
    int end_x = atoi(argv[5]);
    int end_y = atoi(argv[6]); 
 
    if (in == NULL) {
      printf("Could not open input file: No such file or directory\n");
    } else if (out == NULL) {
      printf("Could not open output file\n");
    } else if (!parseable(&argv[3], 4)) {
      printf("Could not parse coordinates\n");
    } else if (out_of_bounds(start_x, start_y)) {
      printf("Start location out of bounds: (%d, %d)\n", start_x, start_y);
    } else if (out_of_bounds(end_x, end_y)) {
      printf("End location out of bounds: (%d, %d)\n", end_x, end_y);
    } else {
      reconstruct(in); // reconstruct maze array from input file
      
#ifdef FULL
      fprintf(out, "FULL\n");
#else
      fprintf(out, "PRUNED\n");
#endif
	  
      // depth-first search for solution path
      dfs(start_x, start_y, end_x, end_y, out);
      
      fclose(in);
      fclose(out);
    }
  }
  return 0;
}
