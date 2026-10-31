/*****************************************************
 * generate_maze.c: generacion reproducible de entradas.
 * Autores: Maximiliano Correa Valdes,
 *          Juan Valdebenito Sanhueza.
 * Santiago de Chile, 31/10/2026.
 ****************************************************/

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "maze.h"

static int ReadNumber(const char *text, unsigned long limit,
                      unsigned long *value)
{
   char *end;

   if (text[0] < '0' || text[0] > '9')
      return 0;
   errno = 0;
   *value = strtoul(text, &end, 10);
   return errno == 0 && *end == '\0' && *value <= limit;
}

/* xorshift32: operaciones de 32 bits, iguales en Windows y Linux. */
static uint32_t NextRandom(uint32_t *state)
{
   *state ^= *state << 13;
   *state ^= *state >> 17;
   *state ^= *state << 5;
   return *state;
}

static void FillMaze(struct Maze *maze, const char *family,
                     unsigned long percent, uint32_t seed)
{
   unsigned long r, c, n, connector;
   uint32_t state;
   uint64_t threshold;

   n = maze->n;
   state = seed == 0 ? UINT32_C(2463534242) : seed;
   threshold = UINT64_C(4294967296) * percent / 100;
   if (strcmp(family, "corridor") == 0 ||
       (strcmp(family, "snake") == 0 && n <= 2)) {
      for (c = 0; c < n; c = c + 1)
         maze->flat[c] = 1;
      for (r = 0; r < n; r = r + 1)
         maze->flat[r * n + n - 1] = 1;
   } else if (strcmp(family, "snake") == 0) {
      for (r = 0; r < n; r = r + 2) {
         for (c = 0; c < n; c = c + 1)
            maze->flat[r * n + c] = 1;
         if (r + 1 < n) {
            connector = (r / 2) % 2 == 0 ? n - 1 : 0;
            maze->flat[(r + 1) * n + connector] = 1;
         }
      }
      if (n % 2 == 0) {
         connector = ((n - 4) / 2) % 2 == 0 ? n - 1 : 0;
         for (c = 0; c < n; c = c + 1) {
            maze->flat[(n - 2) * n + c] = c == connector;
            maze->flat[(n - 1) * n + c] = 1;
         }
      }
   } else {
      for (r = 0; r < n; r = r + 1)
         for (c = 0; c < n; c = c + 1) {
            maze->flat[r * n + c] = strcmp(family, "random") != 0 ||
                                    NextRandom(&state) >= threshold;
            if (strcmp(family, "disconnected") == 0 && r == n / 2)
               maze->flat[r * n + c] = 0;
         }
   }
   /* Se liberan los extremos; no se fuerza una ruta en los casos aleatorios. */
   maze->flat[0] = 1;
   maze->flat[n * n - 1] = 1;
}

int main(int argc, char **argv)
{
   struct Maze *maze;
   unsigned long n, percent, seed, r, c;
   int valid_family, failed;

   if (argc != 5) {
      fprintf(stderr, "Uso: %s open|random|corridor|snake|disconnected N PORCENTAJE SEMILLA\n", argv[0]);
      return 2;
   }
   valid_family = strcmp(argv[1], "open") == 0 ||
                  strcmp(argv[1], "random") == 0 ||
                  strcmp(argv[1], "corridor") == 0 ||
                  strcmp(argv[1], "snake") == 0 ||
                  strcmp(argv[1], "disconnected") == 0;
   if (!valid_family || !ReadNumber(argv[2], 10000, &n) || n == 0 ||
       !ReadNumber(argv[3], 100, &percent) ||
       !ReadNumber(argv[4], UINT32_MAX, &seed) ||
       (strcmp(argv[1], "disconnected") == 0 && n < 3) ||
       (strcmp(argv[1], "random") != 0 && percent != 0)) {
      fprintf(stderr, "Argumentos invalidos: N 1..10000, porcentaje 0..100, semilla 0..4294967295.\n");
      return 2;
   }
   maze = InitFlatMaze(n);
   if (maze == NULL) {
      fprintf(stderr, "No se pudo reservar el laberinto.\n");
      return 3;
   }
   FillMaze(maze, argv[1], percent, (uint32_t) seed);
   printf("%lu\n", n);
   for (r = 0; r < n; r = r + 1) {
      for (c = 0; c < n; c = c + 1)
         printf("%u%c", (unsigned int) maze->flat[r * n + c], c + 1 == n ? '\n' : ' ');
   }
   failed = fflush(stdout) != 0 || ferror(stdout);
   KillMaze(maze);
   return failed ? 6 : 0;
}
