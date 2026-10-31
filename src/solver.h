/*****************************************************
 * solver.h: opciones, espacio de trabajo y resultados.
 * Autores: Maximiliano Correa Valdes,
 *          Juan Valdebenito Sanhueza.
 * Santiago de Chile, 31/10/2026.
 ****************************************************/

#ifndef SOLVER_H
#define SOLVER_H

#include "maze.h"
#include "timer.h"

enum SolverVersion { ROWS, FLAT, INPLACE };

enum SolverStatus {
   SOLVER_OK, INVALID_INPUT, INVALID_ARGUMENTS, MEMORY_LIMIT,
   ALLOCATION_FAILED, DEPTH_LIMIT, COUNT_OVERFLOW, NODES_OVERFLOW,
   OUTPUT_ERROR, COUNT_MISMATCH
};

struct SolverOptions {
   enum SolverVersion version;
   int list;
   unsigned long repeat, memory_limit, depth_limit;
};

struct SolverResult {
   unsigned long paths, nodes, max_depth, repeat_completed;
   enum SolverStatus status;
};

/* Las reservas temporales de busqueda no pertenecen al laberinto. */
struct SearchWorkspace {
   struct Maze *visited;
   char *path;
};

void InitOptions(struct SolverOptions *options);
void InitResult(struct SolverResult *result);
void InitWorkspace(struct SearchWorkspace *work);
const char *VersionName(enum SolverVersion version);
const char *StatusName(enum SolverStatus status);
int SolverExitCode(enum SolverStatus status);
enum SolverStatus CalculateMemory(unsigned long n,
                                 const struct SolverOptions *options,
                                 unsigned long *bytes);
int AllocateWorkspace(struct SearchWorkspace *work, unsigned long n,
                      enum SolverVersion version);
void KillWorkspace(struct SearchWorkspace *work);
void RunSearches(struct Maze *maze, const struct SolverOptions *options,
                 struct SearchWorkspace *work, struct SolverResult *result,
                 struct Timings *times);

#endif
