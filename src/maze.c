/*****************************************************
 * maze.c: memoria y lectura de matrices binarias.
 * Autores: Maximiliano Correa Valdes,
 *          Juan Valdebenito Sanhueza.
 * Santiago de Chile, 31/10/2026.
 ****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include "maze.h"

/* Admite NULL y reservas parciales. Libera tambien la estructura. */
void KillMaze(struct Maze *maze) {

   unsigned long row;

   if (maze != NULL) {
      if (maze->rows != NULL) {
         for (row = 0; row < maze->n; row = row + 1)
            free(maze->rows[row]);
      }
      free(maze->rows);
      free(maze->flat);
      free(maze);
   }
}

/* Crea la estructura y sus filas; ante un fallo no deja reservas vivas. */
struct Maze *InitMaze(unsigned long n) {

   unsigned long row;
   struct Maze *maze;

   if (n == 0 || n > ULONG_MAX / n || n > (size_t)-1 / n ||
       n > (size_t)-1 / sizeof(unsigned char *))
      return NULL;
   maze = calloc(1,sizeof(struct Maze));
   if (maze == NULL)
      return NULL;
   maze->n = n;
   maze->rows = calloc(n,sizeof(unsigned char *));
   if (maze->rows == NULL) {
      KillMaze(maze);
      return NULL;
   }
   for (row = 0; row < n; row = row + 1) {
      maze->rows[row] = calloc(n,sizeof(unsigned char));
      if (maze->rows[row] == NULL) {
         KillMaze(maze);
         return NULL;
      }
   }
   return maze;
}

/* Constructor de la representacion contigua para flat e inplace. (EN ESTUDIO)*/
struct Maze *InitFlatMaze(unsigned long n) {

   struct Maze *maze;

   if (n == 0 || n > ULONG_MAX / n || n > (size_t)-1 / n)
      return NULL;
   maze = calloc(1,sizeof(struct Maze));
   if (maze == NULL)
      return NULL;
   maze->n = n;
   maze->flat = calloc(n * n,sizeof(unsigned char));
   if (maze->flat == NULL) {
      KillMaze(maze);
      return NULL;
   }
   return maze;
}

/* Lectura estricta: dimension positiva y celdas literales 0 o 1. */
int ReadDimension(unsigned long *value) {

   int character;
   unsigned long result, digit;

   result = 0;
   character = fgetc(stdin);
   while (character != EOF && isspace(character))
      character = fgetc(stdin);
   if (character == EOF)
      return 0;
   while (character != EOF && !isspace(character)) {
      if (character < '0' || character > '9')
         return 0;
      digit = (character - '0');
      if (result > (ULONG_MAX - digit) / 10UL)
         return 0;
      result = result * 10UL + digit;
      character = fgetc(stdin);
   }
   if (ferror(stdin) || result == 0)
      return 0;
   *value = result;
   return 1;
}


/* Las celdas son tokens literales 0 o 1: 01 y 1x no son validos. */
static int ReadCell(unsigned char *value) {

   int character, next;

   character = fgetc(stdin);
   while (character != EOF && isspace(character))
      character = fgetc(stdin);
   if (character != '0' && character != '1')
      return 0;
   next = fgetc(stdin);
   if (next != EOF && !isspace(next))
      return 0;
   if (ferror(stdin))
      return 0;
   *value = (character - '0');
   return 1;
}


static int EndOfInput(void) {

   int character;

   character = fgetc(stdin);
   while (character != EOF && isspace(character))
      character = fgetc(stdin);
   return character == EOF && !ferror(stdin);
}


int ReadMaze(struct Maze *maze) {

   unsigned long row, column;
   unsigned char value;

   for (row = 0; row < maze->n; row = row + 1) {
      for (column = 0; column < maze->n; column = column + 1) {
         if (!ReadCell(&value))
            return 0;
         if (maze->rows != NULL)
            maze->rows[row][column] = value;
         else
            maze->flat[row * maze->n + column] = value;
      }
   }
   return EndOfInput();
}


