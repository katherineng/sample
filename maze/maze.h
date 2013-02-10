#ifndef MAZE_H
#define MAZE_H

#define NUM_ROWS 10
#define NUM_COLS 25

/**
 * struct representing a room in the maze
 * visited - 0 if the room has not been visited; 1 if the room has been visited
 * connections - each index represents the type of connection in one of the cardinal directions (in the order east, west, south, north)
 *             - 0 if there is a door; 1 if there is a wall
 */
typedef struct {
  int visited;
  int connections[4];
} Room;

/**
 * enumerated type for the cardinal directions
 * sets each direction to the corresponding index of the connections array in a Room structure
 */
enum Direction {EAST, WEST, SOUTH, NORTH};

/**
 * calculates the offset in a given direction along a given axis
 */
int calculate_offset(int direction, char type);

/**
 * given x- and y-coordinates, returns 1 if (x, y) is out of 
 * bounds, 0 otherwise
 */
int out_of_bounds(int x, int y);

#endif /* MAZE_H */
