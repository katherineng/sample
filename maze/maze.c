#include "maze.h"

/**
 * calculates the offset in a given direction along a given axis
 */
int calculate_offset(int direction, char type) {
  int x_offset = 0;
  int y_offset = 0;

  switch(direction) {
  case EAST:
    x_offset++;
    break;
  case WEST:
    x_offset--;
    break;
  case SOUTH:
    y_offset++;
    break;
  case NORTH:
    y_offset--;
    break;
  }
  if (type == 'x')
    return x_offset;
  else return y_offset;
}

/**
 * given x- and y-coordinates, returns 1 if (x, y) is out of 
 * bounds, 0 otherwise
 */
int out_of_bounds(int x, int y) {
  if ((x < 0 || x > NUM_COLS - 1) || (y < 0 || y > NUM_ROWS - 1))
    return 1;
  else return 0;
}
