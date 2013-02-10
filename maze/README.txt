 __  __    _     __________ 
|  \/  |  / \   |__  / ____|
| |\/| | / _ \    / /|  _|  
| |  | |/ ___ \  / /_| |___ 
|_|  |_/_/   \_\/____|_____| Katherine Ng (kwng)


The header file maze.h contains a struct Room with two fields: visited and connections. The visited field is an int that indicates whether the room has been visited (1) or not (0). The connection field is an array of four ints for each of the four cardinal directions. The value at each index indicates whether or not the connection in the corresponding direction is a wall (1) or a door (0). The x- and y-coordinates were not included in the Room struct because they correspond to the array indices of the Room in the maze, so including them would be redundant.

In order to facilitate the use of an array to store the connections, the header file also includes the enum Direction, which enumerates the cardinal directions such that each direction is associated with the appropriate index in the connections array. This enum also facilitates the use of switch cases when finding the opposite of a given direction or calculating the coordinates of a neighbor in a given direction.

Additionally, there are two methods calculate_offset and out_of_bounds declared in the header file, as they are used in both the solver and generator programs. These two methods are implemented in a third source file maze.c rather than either the solver or generator files because it allows the solver and generator files to be compiled separately using the Makefile targets.

The Makefile was altered to include maze.c. The generator target was also changed to include the -rl option to link the math library, since math.pow was used in generator.c in the conversion of bits to integers.

NUM_ROWS and NUM_COLS are defined in the header file to avoid hard coding the maze dimensions. Maze dimensions can easily be changed simply by changing the value of NUM_ROWS and NUM_COLS once.

In both generator.c and solver.c, the maze is represented internally as a two-dimensional array of rooms (i.e. an array of rows of rooms). The maze is declared as a global variable to avoid having to constantly pass the maze around from one function to another, and since there is only ever one maze being generated or solved when running either program, declaring it as a global variable will not result in conflicts.
