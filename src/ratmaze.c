/*****************************************************
 * ratmaze.c: consola y coordinacion del programa.
 * Autores: Maximiliano Correa Valdes,
 *          Juan Valdebenito Sanhueza.
 * Santiago de Chile, 31/10/2026.
 * Compilar con maze.c, solver.c y timer.c.
 ****************************************************/

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "solver.h"

static void Usage(char *program) {

   printf("Uso: %s [--version rows|flat|inplace] [--mode list|count]\n",program);
   printf("       [--repeat N] [--max-depth N] [--memory-limit BYTES] < laberinto.txt\n");
   printf("Entrada: n positivo, seguido de exactamente n*n tokens 0 o 1.\n");
   printf("Predeterminados: rows, count, repeat=1, max-depth=2048, memory-limit=1000000000.\n");
   printf("list requiere repeat=1; una ruta vacia (n=1) es una linea vacia.\n");
   printf("Profundidad: numero de celdas del camino, incluyendo el origen.\n");
   printf("Presupuesto: bytes solicitados con calloc; excluye pila y biblioteca C.\n");
   printf("search_clock_s acumula repeticiones; en list incluye la salida.\n");
   printf("Semantica de clock(): %s; -1 indica medicion no disponible.\n",CLOCK_SEMANTICS);
   printf("paths, nodes y max_depth corresponden al ultimo intento de busqueda.\n");
   printf("Salidas: 0 exito; 2 entrada/argumentos; 3 memoria; 4 profundidad;\n");
   printf("         5 contadores; 6 error de salida.\n");
}


/* Solo admite enteros decimales positivos, sin signo ni espacios. */
static int ParsePositive(char *text, unsigned long *value) {

   unsigned long result, digit;
   unsigned int i;

   result = 0;
   i = 0;
   if (text[0] == '\0')
      return 0;
   while (text[i] != '\0') {
      if (text[i] < '0' || text[i] > '9')
         return 0;
      digit = (text[i] - '0');
      if (result > (ULONG_MAX - digit) / 10UL)
         return 0;
      result = result * 10UL + digit;
      i = i + 1;
   }
   if (result == 0)
      return 0;
   *value = result;
   return 1;
}


static int ParseArguments(struct SolverOptions *options, int argc, char *argv[]) {

   int i;

   for (i = 1; i < argc; i = i + 2) {
      if (i == argc - 1)
         return 0;
      if (strcmp(argv[i],"--version") == 0) {
         if (strcmp(argv[i + 1],"rows") == 0)
            options->version = ROWS;
         else if (strcmp(argv[i + 1],"flat") == 0)
            options->version = FLAT;
         else if (strcmp(argv[i + 1],"inplace") == 0)
            options->version = INPLACE;
         else
            return 0;
      }
      else if (strcmp(argv[i],"--mode") == 0) {
         if (strcmp(argv[i + 1],"list") == 0)
            options->list = 1;
         else if (strcmp(argv[i + 1],"count") == 0)
            options->list = 0;
         else
            return 0;
      }
      else if (strcmp(argv[i],"--repeat") == 0) {
         if (!ParsePositive(argv[i + 1],&options->repeat))
            return 0;
      }
      else if (strcmp(argv[i],"--max-depth") == 0) {
         if (!ParsePositive(argv[i + 1],&options->depth_limit))
            return 0;
      }
      else if (strcmp(argv[i],"--memory-limit") == 0) {
         if (!ParsePositive(argv[i + 1],&options->memory_limit))
            return 0;
      }
      else
         return 0;
   }
   return !options->list || options->repeat == 1;
}

static int Report(unsigned long n, const struct SolverOptions *options,
                  const struct SolverResult *result, struct Timings *times,
                  unsigned long heap_bytes) {

   int includes_output, written;

   includes_output = options->list;
   written = fprintf(stderr,
                     "status=%s\tversion=%s\tn=%lu\tpaths=%lu\tnodes=%lu"
                     "\tmax_depth=%lu\trepeat=%lu\tprep_clock_s=%.9f"
                     "\tsearch_clock_s=%.9f\toutput_clock_s=%.9f"
                     "\theap_bytes=%lu\tclock_ticks_per_s=%lu"
                     "\trepeat_completed=%lu\tmode=%s\tsearch_includes_output=%d"
                     "\tcounters_scope=last_search\theap_scope=requested_bytes"
                     "\tclock_semantics=%s\n",
                     StatusName(result->status),VersionName(options->version),
                     n,result->paths,result->nodes,
                     result->max_depth,options->repeat,times->prep_clock_s,
                     times->search_clock_s,times->output_clock_s,heap_bytes,
                     (unsigned long)CLOCKS_PER_SEC,result->repeat_completed,
                     (options->list ? "list" : "count"),includes_output,CLOCK_SEMANTICS);
   if (written < 0 || fflush(stderr) == EOF)
      return 6;
   return SolverExitCode(result->status);
}


int main(int argc, char *argv[]) {

   struct Maze *maze;
   struct SolverOptions options;
   struct SolverResult result;
   struct SearchWorkspace work;
   struct Timings times;
   unsigned long n, heap_bytes;
   clock_t start, end;

   maze = NULL;
   n = 0;
   InitOptions(&options);
   InitResult(&result);
   InitWorkspace(&work);
   InitTimings(&times);
   heap_bytes = 0;
   if (argc == 2 && strcmp(argv[1],"--help") == 0) {
      Usage(argv[0]);
      if (fflush(stdout) == EOF)
         return 6;
      return 0;
   }
   start = clock();
   if (!ParseArguments(&options,argc,argv))
      result.status = INVALID_ARGUMENTS;
   else if (!ReadDimension(&n))
      result.status = INVALID_INPUT;
   else {
      result.status = CalculateMemory(n,&options,&heap_bytes);
      if (result.status == SOLVER_OK) {
         maze = options.version == ROWS ? InitMaze(n) : InitFlatMaze(n);
         if (maze == NULL || !AllocateWorkspace(&work,n,options.version))
            result.status = ALLOCATION_FAILED;
         else if (!ReadMaze(maze))
            result.status = INVALID_INPUT;
      }
   }
   end = clock();
   times.prep_clock_s = Elapsed(start,end);
   if (result.status == SOLVER_OK)
      RunSearches(maze,&options,&work,&result,&times);
   KillWorkspace(&work);
   KillMaze(maze);
   return Report(n,&options,&result,&times,heap_bytes);
}
