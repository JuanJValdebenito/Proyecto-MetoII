/*****************************************************
 * solver.c: backtracking D,L,R,U y control de busqueda.
 * Autores: Maximiliano Correa Valdes,
 *          Juan Valdebenito Sanhueza.
 * Santiago de Chile, 31/10/2026.
 ****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "solver.h"

/* Movimientos en orden lexicografico. */
static const int RowStep[4] = {1, 0, 0, -1};
static const int ColumnStep[4] = {0, -1, 1, 0};
static const char Move[4] = {'D', 'L', 'R', 'U'};

void InitOptions(struct SolverOptions *options) {

   options->version = ROWS;
   options->list = 0;
   options->repeat = 1;
   options->memory_limit = 1000000000UL;
   options->depth_limit = 2048UL;
}

void InitResult(struct SolverResult *result) {

   result->paths = 0;
   result->nodes = 0;
   result->max_depth = 0;
   result->repeat_completed = 0;
   result->status = SOLVER_OK;
}

void InitWorkspace(struct SearchWorkspace *work) {

   work->visited = NULL;
   work->path = NULL;
}

const char *VersionName(enum SolverVersion version) {

   switch (version) {
      case ROWS: return "rows";
      case FLAT: return "flat";
      case INPLACE: return "inplace";
   }
   return "unknown";
}

const char *StatusName(enum SolverStatus status) {

   switch (status) {
      case SOLVER_OK: return "ok";
      case INVALID_INPUT: return "invalid_input";
      case INVALID_ARGUMENTS: return "invalid_arguments";
      case MEMORY_LIMIT: return "memory_limit";
      case ALLOCATION_FAILED: return "allocation_failed";
      case DEPTH_LIMIT: return "depth_limit";
      case COUNT_OVERFLOW: return "count_overflow";
      case NODES_OVERFLOW: return "nodes_overflow";
      case OUTPUT_ERROR: return "output_error";
      case COUNT_MISMATCH: return "count_mismatch";
   }
   return "unknown";
}

int SolverExitCode(enum SolverStatus status) {

   switch (status) {
      case SOLVER_OK: return 0;
      case INVALID_INPUT:
      case INVALID_ARGUMENTS: return 2;
      case MEMORY_LIMIT:
      case ALLOCATION_FAILED: return 3;
      case DEPTH_LIMIT: return 4;
      case COUNT_OVERFLOW:
      case NODES_OVERFLOW:
      case COUNT_MISMATCH: return 5;
      case OUTPUT_ERROR: return 6;
   }
   return 2;
}

/* Conserva la primera causa de una ejecucion incompleta. */
static void SetFailure(struct SolverResult *result, enum SolverStatus status) {

   if (result->status == SOLVER_OK)
      result->status = status;
}

/* Incluye estructuras Maze, celdas, visitados y ruta; excluye la pila. */
enum SolverStatus CalculateMemory(unsigned long n,
                                 const struct SolverOptions *options,
                                 unsigned long *bytes) {

   unsigned long cells, factor, pointer_bytes, pointer_size, structure_bytes;

   *bytes = 0;
   if (n == 0 || n > ULONG_MAX / n)
      return INVALID_INPUT;
   cells = n * n;
   factor = 3;
   if (options->version == INPLACE)
      factor = 2;
   if (cells > ULONG_MAX / factor)
      return MEMORY_LIMIT;
   *bytes = cells * factor;
   if (options->version == ROWS) {
      pointer_size = sizeof(unsigned char *);
      if (n > ULONG_MAX / pointer_size / 2UL)
         return MEMORY_LIMIT;
      pointer_bytes = n * pointer_size * 2UL;
      if (*bytes > ULONG_MAX - pointer_bytes)
         return MEMORY_LIMIT;
      *bytes = *bytes + pointer_bytes;
   }
   structure_bytes = sizeof(struct Maze);
   if (options->version != INPLACE)
      structure_bytes = structure_bytes * 2UL;
   if (*bytes > ULONG_MAX - structure_bytes)
      return MEMORY_LIMIT;
   *bytes = *bytes + structure_bytes;
   if (*bytes > options->memory_limit || *bytes > (size_t)-1)
      return MEMORY_LIMIT;
   return SOLVER_OK;
}

void KillWorkspace(struct SearchWorkspace *work) {

   KillMaze(work->visited);
   work->visited = NULL;
   free(work->path);
   work->path = NULL;
}

int AllocateWorkspace(struct SearchWorkspace *work, unsigned long n,
                      enum SolverVersion version) {

   if (n == 0 || n > ULONG_MAX / n || n > (size_t)-1 / n)
      return 0;
   work->path = calloc(n * n,sizeof(char));
   if (work->path == NULL)
      return 0;
   if (version != INPLACE) {
      work->visited = version == ROWS ? InitMaze(n) : InitFlatMaze(n);
      if (work->visited == NULL) {
         KillWorkspace(work);
         return 0;
      }
   }
   return 1;
}

static int Available(struct Maze *maze, struct SearchWorkspace *work,
                     unsigned long row, unsigned long column) {

   unsigned long index;

   if (maze->rows != NULL)
      return maze->rows[row][column] != 0 && work->visited->rows[row][column] == 0;
   index = row * maze->n + column;
   if (work->visited != NULL)
      return maze->flat[index] != 0 && work->visited->flat[index] == 0;
   return maze->flat[index] != 0;
}


/* Al desmarcar inplace se restaura 1: solamente se visitan celdas abiertas. */
static void MarkCell(struct Maze *maze, struct SearchWorkspace *work,
                     unsigned long row, unsigned long column,
                     unsigned char visited) {

   unsigned long index;

   if (maze->rows != NULL)
      work->visited->rows[row][column] = visited;
   else {
      index = row * maze->n + column;
      if (work->visited != NULL)
         work->visited->flat[index] = visited;
      else
         maze->flat[index] = (1 - visited);
   }
}


static void EmitPath(struct SearchWorkspace *work, struct SolverResult *result,
                     struct Timings *times, unsigned long length) {

   clock_t start, end;

   work->path[length] = '\0';
   start = clock();
   if (fputs(work->path,stdout) == EOF || fputc('\n',stdout) == EOF)
      SetFailure(result,OUTPUT_ERROR);
   end = clock();
   times->output_clock_s = AddTime(times->output_clock_s,Elapsed(start,end));
}


/* depth incluye la celda actual; el camino contiene depth-1 movimientos. */
static void Search(struct Maze *maze, const struct SolverOptions *options,
                   struct SearchWorkspace *work, struct SolverResult *result,
                   struct Timings *times, unsigned long row, unsigned long column,
                   unsigned long depth) {

   long next_row, next_column, n;
   int direction;

   /* n*n fue validado: n cabe en long y admite el vecino -1. */
   n = (long)maze->n;
   if (result->nodes == ULONG_MAX) {
      SetFailure(result,NODES_OVERFLOW);
      return;
   }
   result->nodes = result->nodes + 1;
   if (depth > result->max_depth)
      result->max_depth = depth;
   if (row == maze->n - 1 && column == maze->n - 1) {
      if (result->paths == ULONG_MAX) {
         SetFailure(result,COUNT_OVERFLOW);
         return;
      }
      result->paths = result->paths + 1;
      if (options->list)
         EmitPath(work,result,times,depth - 1);
      return;
   }
   MarkCell(maze,work,row,column,1);
   for (direction = 0; direction < 4 && result->status == SOLVER_OK;
        direction = direction + 1) {
      next_row = (long)row + RowStep[direction];
      next_column = (long)column + ColumnStep[direction];
      if (next_row >= 0 && next_row < n && next_column >= 0 && next_column < n &&
          Available(maze,work,next_row,next_column)) {
         if (depth >= options->depth_limit)
            SetFailure(result,DEPTH_LIMIT);
         else {
            work->path[depth - 1] = Move[direction];
            Search(maze,options,work,result,times,next_row,next_column,depth + 1);
         }
      }
   }
   MarkCell(maze,work,row,column,0);
}


/* Todas las marcas se restauran al retroceder, incluso tras un error. */
void RunSearches(struct Maze *maze, const struct SolverOptions *options,
                 struct SearchWorkspace *work, struct SolverResult *result,
                 struct Timings *times) {

   unsigned long iteration, reference_paths, reference_nodes;
   clock_t start, end, output_start, output_end;

   result->repeat_completed = 0;
   times->output_clock_s = 0.0f;
   reference_paths = 0;
   reference_nodes = 0;
   start = clock();
   for (iteration = 0; iteration < options->repeat && result->status == SOLVER_OK;
        iteration = iteration + 1) {
      result->paths = 0;
      result->nodes = 0;
      result->max_depth = 0;
      if (Available(maze,work,0,0) && Available(maze,work,maze->n - 1,maze->n - 1))
         Search(maze,options,work,result,times,0,0,1);
      if (options->list) {
         output_start = clock();
         if (fflush(stdout) == EOF)
            SetFailure(result,OUTPUT_ERROR);
         output_end = clock();
         times->output_clock_s = AddTime(times->output_clock_s,
                                        Elapsed(output_start,output_end));
      }
      if (result->status == SOLVER_OK) {
         if (iteration == 0) {
            reference_paths = result->paths;
            reference_nodes = result->nodes;
         }
         else if (result->paths != reference_paths || result->nodes != reference_nodes)
            SetFailure(result,COUNT_MISMATCH);
         if (result->status == SOLVER_OK)
            result->repeat_completed = result->repeat_completed + 1;
      }
   }
   end = clock();
   times->search_clock_s = Elapsed(start,end);
}


