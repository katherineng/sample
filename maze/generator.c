#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "maze.h"

Room maze[NUM_ROWS][NUM_COLS];

/**
 * given an array, performs an in-place shuffle using the Fisher-
 * Yates algorithm
 */
void shuffle_array(int arr[]) {
  int i;
  for (i = 3; i > 0; --i) {
    int r = rand() % (i + 1);
    int temp = arr[i];
    arr[i] = arr[r];
    arr[r] = temp;
  }
}

/**
 * given a direction, returns the opposite direction
 */
int opposite(int direction) {
  switch(direction) {
  case EAST:
    return WEST;
  case WEST:
    return EAST;
  case SOUTH:
    return NORTH;
  case NORTH:
    return SOUTH;
  }
  return 0;
}


/**
 * given starting x- and y-coordinates, recursively constructs a 
 * maze by visiting each room and randomly choosing connections 
 * for that room by visiting adjacent rooms
 */
void drunken_walk(int x, int y) {
  Room *r = &maze[y][x]; // pointer to room at (x, y) in maze
  r->visited = 1; // set visited to true
  
  // set connections to -1 to indicate that they do not exist yet
  int i;
  for(i = 0; i < 4; ++i) {
    r->connections[i] = -1;
  }

  // randomize the order of the directions
  int directions[4] = {EAST, WEST, SOUTH, NORTH};
  shuffle_array(directions);

  for (i = 0; i < 4; ++i) { // for each direction in random order
    int dir = directions[i];
    int neighbor_x = x + calculate_offset(dir, 'x');
    int neighbor_y = y + calculate_offset(dir, 'y');
    
    if (out_of_bounds(neighbor_x, neighbor_y)) {
      r->connections[dir] = 1;
    } else {
      Room *neighbor = &maze[neighbor_y][neighbor_x];
      if (neighbor->visited != 1) {
	r->connections[dir] = 0;
	drunken_walk(neighbor_x, neighbor_y);
      } else {
	int opposite_dir = opposite(dir);
	if (neighbor->connections[opposite_dir] != -1) {
	  r->connections[dir] = neighbor->connections[opposite_dir];
	}
	else {
	  r->connections[dir] = 1;
	}
      }
    }
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
      printf("Usage: %s <output>\n", argv[0]);
  } else {
    FILE *file = fopen(argv[1], "w"); // open output file
    
    if (file == NULL) {
      printf("Could not write to file %s\n", argv[1]);
    } else {
      
      srand(time(NULL)); // change seed value to ensure randomness

      drunken_walk(0, 0); // generate random maze

      int i, j, k;
      for(i = 0; i < NUM_ROWS; ++i) {
	for(j = 0; j < NUM_COLS; ++j) {
	  int decimal = 0;
	  for(k = 0; k < 4; ++k) {
	    // convert bits to integer
	    decimal += maze[i][j].connections[k] * pow(2, (3 - k));
	  }
	  fprintf(file, "%x", decimal); // convert integer to hexadecimal
	}
	fprintf(file, "\n");
      }
      fclose(file);
    }
  }
  return 0;
}

