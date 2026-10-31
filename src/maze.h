/*****************************************************
 * maze.h: representacion y operaciones del laberinto.
 * Autores: Maximiliano Correa Valdes,
 *          Juan Valdebenito Sanhueza.
 * Santiago de Chile, 31/10/2026.
 ****************************************************/

#ifndef MAZE_H
#define MAZE_H

struct Maze {
   unsigned long n;
   unsigned char **rows;
   unsigned char *flat;
};

/* Devuelven la estructura y sus celdas en cero, o NULL sin reservas pendientes. */
struct Maze *InitMaze(unsigned long n);
struct Maze *InitFlatMaze(unsigned long n);
/* Libera celdas y estructura; acepta NULL. No reutilizar el puntero liberado. */
void KillMaze(struct Maze *maze);
int ReadDimension(unsigned long *value);
int ReadMaze(struct Maze *maze);

#endif
