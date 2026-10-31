/*******************************************************************************
 *
 * test_ratmaze.c: pruebas funcionales y de fallos de los modulos C.
 * Autores: Maximiliano Correa Valdes y Juan Valdebenito Sanhueza.
 * Proyecto Metodologia de la Investigacion II, octubre de 2026.
 *
 * Compilar: gcc -std=c99 -Wall -Wextra -Wpedantic -Werror -O2
 *           test_ratmaze.c -o test_ratmaze
 * Ejecutar: ./test_ratmaze > validacion_c.txt
 *
 * Las variables globales se limitan a instrumentacion de pruebas. Los hooks
 * sustituyen llamadas solamente en esta unidad de prueba. Los modulos se
 * incluyen aqui para probar funciones internas sin modificar sus fuentes.
 * El oraculo usa una pila explicita y conjuntos de vertices, no Search.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>


unsigned long TestAssertions, TestBoards, TestAllocationCases;
unsigned long TestDepthCases, TestOutputCases, TestInputCases, TestArgumentCases;
void *TestPointers[512];
size_t TestSizes[512], TestBytes;
unsigned int TestLive, TestCalls, TestFailAt;
char TestCapture[8192];
unsigned long TestCaptureLength;
int TestCaptureEnabled, TestOutputFailure;
char *TestInput;
unsigned long TestInputPosition;
int TestInputFailure;


void Require(int condition, char *message) {

   TestAssertions = TestAssertions + 1;
   if (!condition) {
      fprintf(stderr,"FALLO tras %lu aserciones: %s\n",TestAssertions,message);
      exit(1);
   }
}


void *TestCalloc(size_t count, size_t size) {

   void *pointer;

   TestCalls = TestCalls + 1;
   if (TestCalls == TestFailAt)
      return NULL;
   pointer = calloc(count,size);
   if (pointer != NULL) {
      Require(TestLive < 512,"capacidad del registro de asignaciones");
      TestPointers[TestLive] = pointer;
      TestSizes[TestLive] = count * size;
      TestBytes = TestBytes + TestSizes[TestLive];
      TestLive = TestLive + 1;
   }
   return pointer;
}


void TestFree(void *pointer) {

   unsigned int i;

   if (pointer == NULL)
      return;
   for (i = 0; i < TestLive; i = i + 1) {
      if (TestPointers[i] == pointer) {
         TestBytes = TestBytes - TestSizes[i];
         TestLive = TestLive - 1;
         TestPointers[i] = TestPointers[TestLive];
         TestSizes[i] = TestSizes[TestLive];
         free(pointer);
         return;
      }
   }
   Require(0,"liberacion doble o puntero no registrado");
}


int TestPuts(char *text, FILE *stream) {

   unsigned long i;

   if (stream != stdout || !TestCaptureEnabled)
      return fputs(text,stream);
   if (TestOutputFailure == 1)
      return EOF;
   for (i = 0; text[i] != '\0'; i = i + 1) {
      Require(TestCaptureLength + 1 < sizeof(TestCapture),"capacidad de captura");
      TestCapture[TestCaptureLength] = text[i];
      TestCaptureLength = TestCaptureLength + 1;
   }
   TestCapture[TestCaptureLength] = '\0';
   return 0;
}


int TestPutc(int character, FILE *stream) {

   if (stream != stdout || !TestCaptureEnabled)
      return fputc(character,stream);
   if (TestOutputFailure == 2)
      return EOF;
   Require(TestCaptureLength + 1 < sizeof(TestCapture),"capacidad de captura final");
   TestCapture[TestCaptureLength] = character;
   TestCaptureLength = TestCaptureLength + 1;
   TestCapture[TestCaptureLength] = '\0';
   return character;
}


int TestFlush(FILE *stream) {

   if (stream == stdout && TestCaptureEnabled) {
      if (TestOutputFailure == 3)
         return EOF;
      return 0;
   }
   return fflush(stream);
}


int TestGetc(FILE *stream) {

   int character;

   if (stream != stdin || TestInput == NULL)
      return fgetc(stream);
   if (TestInputFailure || TestInput[TestInputPosition] == '\0')
      return EOF;
   character = (unsigned char)TestInput[TestInputPosition];
   TestInputPosition = TestInputPosition + 1;
   return character;
}


int TestFerror(FILE *stream) {

   if (stream == stdin && TestInput != NULL)
      return TestInputFailure;
   return ferror(stream);
}


#define calloc TestCalloc
#define free TestFree
#define fputs TestPuts
#define fputc TestPutc
#define fflush TestFlush
#define fgetc TestGetc
#ifdef ferror
#undef ferror
#endif
#define ferror TestFerror
#include "maze.c"
#include "timer.c"
#include "solver.c"
#define main RatmazeProgramMain
#include "ratmaze.c"
#undef main
#undef calloc
#undef free
#undef fputs
#undef fputc
#undef fflush
#undef fgetc
#undef ferror


/* Agrupacion exclusiva de pruebas: el programa usa estos objetos por separado. */
struct TestCase {
   struct Maze *maze;
   struct SolverOptions options;
   struct SolverResult result;
   struct SearchWorkspace work;
   struct Timings times;
   unsigned long n, heap_bytes;
};

void InitTestCase(struct TestCase *test) {

   test->maze = NULL;
   test->n = 0;
   InitOptions(&test->options);
   InitResult(&test->result);
   InitWorkspace(&test->work);
   InitTimings(&test->times);
   test->heap_bytes = 0;
}

int CalculateTestMemory(struct TestCase *test) {

   test->result.status = CalculateMemory(test->n,&test->options,&test->heap_bytes);
   return test->result.status == SOLVER_OK;
}

int AllocateTestCase(struct TestCase *test) {

   test->maze = test->options.version == ROWS ? InitMaze(test->n) : InitFlatMaze(test->n);
   if (test->maze == NULL || !AllocateWorkspace(&test->work,test->n,test->options.version)) {
      test->result.status = ALLOCATION_FAILED;
      return 0;
   }
   Require(TestBytes == test->heap_bytes,"presupuesto coincide con bytes realmente reservados");
   return 1;
}

void KillTestCase(struct TestCase *test) {

   KillWorkspace(&test->work);
   KillMaze(test->maze);
   test->maze = NULL;
}

struct OracleFrame {

   unsigned long used;
   unsigned int vertex, length;
   char route[17];
};


struct OracleResult {

   unsigned long count;
   char routes[256][17];
};


void ResetHooks(void) {

   Require(TestLive == 0 && TestBytes == 0,"no quedan asignaciones de una prueba anterior");
   TestCalls = 0;
   TestFailAt = 0;
   TestCaptureLength = 0;
   TestCapture[0] = '\0';
   TestCaptureEnabled = 0;
   TestOutputFailure = 0;
   TestInput = NULL;
   TestInputPosition = 0;
   TestInputFailure = 0;
}


/* Referencia independiente: cada estado contiene su conjunto y ruta completos.
 * Se obtiene la adyacencia comparando todos los vertices del grafo pequeno.
 * Las soluciones se ordenan despues: no dependen del orden de expansion.
 */
void Oracle(unsigned char *board, unsigned int n, struct OracleResult *result) {

   struct OracleFrame stack[64], current, next;
   unsigned int top, candidate, row, column, other_row, other_column;
   unsigned long i, j, bit;
   int row_distance, column_distance;
   char temporary[17];

   Require(n >= 1 && n <= 4,"dominio del oraculo n <= 4");
   result->count = 0;
   if (board[0] == 0 || board[n * n - 1] == 0)
      return;
   memset(&current,0,sizeof(current));
   current.used = 1;
   stack[0] = current;
   top = 1;
   while (top != 0) {
      top = top - 1;
      current = stack[top];
      if (current.vertex == n * n - 1) {
         Require(result->count < 256,"capacidad de soluciones del oraculo");
         strcpy(result->routes[result->count],current.route);
         result->count = result->count + 1;
      }
      else {
         row = current.vertex / n;
         column = current.vertex % n;
         for (candidate = 0; candidate < n * n; candidate = candidate + 1) {
            other_row = candidate / n;
            other_column = candidate % n;
            row_distance = (int)row - (int)other_row;
            column_distance = (int)column - (int)other_column;
            bit = 1UL << candidate;
            if (abs(row_distance) + abs(column_distance) == 1 &&
                board[candidate] != 0 && (current.used & bit) == 0) {
               Require(top < 64,"capacidad de la pila del oraculo");
               next = current;
               next.vertex = candidate;
               next.used = current.used | bit;
               if (other_row > row)
                  next.route[current.length] = 'D';
               else if (other_row < row)
                  next.route[current.length] = 'U';
               else if (other_column > column)
                  next.route[current.length] = 'R';
               else
                  next.route[current.length] = 'L';
               next.length = current.length + 1;
               next.route[next.length] = '\0';
               stack[top] = next;
               top = top + 1;
            }
         }
      }
   }
   for (i = 1; i < result->count; i = i + 1) {
      strcpy(temporary,result->routes[i]);
      j = i;
      while (j > 0 && strcmp(result->routes[j - 1],temporary) > 0) {
         strcpy(result->routes[j],result->routes[j - 1]);
         j = j - 1;
      }
      strcpy(result->routes[j],temporary);
   }
}


void PrepareBoard(struct TestCase *maze, unsigned char *board, unsigned long n,
                  enum SolverVersion version) {

   unsigned long row, column, index;

   InitTestCase(maze);
   maze->options.version = version;
   maze->n = n;
   Require(CalculateTestMemory(maze),"calculo de memoria de caso valido");
   Require(AllocateTestCase(maze),"reserva de caso valido");
   for (row = 0; row < n; row = row + 1) {
      for (column = 0; column < n; column = column + 1) {
         index = row * n + column;
         if (maze->maze->rows != NULL)
            maze->maze->rows[row][column] = board[index];
         else
            maze->maze->flat[index] = board[index];
      }
   }
}


void CheckRestored(struct TestCase *maze, unsigned char *board) {

   unsigned long row, column, index;

   for (row = 0; row < maze->n; row = row + 1) {
      for (column = 0; column < maze->n; column = column + 1) {
         index = row * maze->n + column;
         if (maze->maze->rows != NULL) {
            Require(maze->maze->rows[row][column] == board[index],"matriz por filas preservada");
            Require(maze->work.visited->rows[row][column] == 0,"visitados por filas restaurados");
         }
         else {
            Require(maze->maze->flat[index] == board[index],"matriz lineal restaurada");
            if (maze->work.visited != NULL)
               Require(maze->work.visited->flat[index] == 0,"visitados lineales restaurados");
         }
      }
   }
}


/* Reconstruye cada ruta capturada para verificar semantica y orden estricto. */
void CheckCaptured(unsigned char *board, unsigned int n,
                   struct OracleResult *expected) {

   unsigned long position, count, used, bit;
   unsigned int length;
   int row, column, dimension;
   char route[17], previous[17], symbol;

   position = 0;
   count = 0;
   dimension = n;
   previous[0] = '\0';
   while (TestCapture[position] != '\0') {
      row = 0;
      column = 0;
      used = 1;
      length = 0;
      Require(board[0] != 0,"origen libre de ruta capturada");
      while (TestCapture[position] != '\n' && TestCapture[position] != '\0') {
         symbol = TestCapture[position];
         Require(length < n * n - 1,"longitud de camino simple");
         route[length] = symbol;
         length = length + 1;
         if (symbol == 'D')
            row = row + 1;
         else if (symbol == 'L')
            column = column - 1;
         else if (symbol == 'R')
            column = column + 1;
         else if (symbol == 'U')
            row = row - 1;
         else
            Require(0,"simbolo de movimiento valido");
         Require(row >= 0 && column >= 0 && row < dimension && column < dimension,
                 "ruta dentro de matriz");
         bit = 1UL << (row * dimension + column);
         Require(board[row * dimension + column] != 0,"ruta no atraviesa obstaculos");
         Require((used & bit) == 0,"ruta no repite celda");
         used = used | bit;
         position = position + 1;
      }
      Require(TestCapture[position] == '\n',"ruta termina en salto de linea");
      position = position + 1;
      route[length] = '\0';
      Require(row == dimension - 1 && column == dimension - 1,"ruta alcanza destino");
      Require(count < expected->count,"sin soluciones adicionales");
      Require(strcmp(route,expected->routes[count]) == 0,"ruta coincide con oraculo");
      if (count > 0)
         Require(strcmp(previous,route) < 0,"orden lexicografico sin duplicados");
      strcpy(previous,route);
      count = count + 1;
   }
   Require(count == expected->count,"lista completa respecto del oraculo");
}


unsigned long CheckBoard(unsigned char *board, unsigned int n) {

   struct TestCase maze;
   struct OracleResult expected;
   enum SolverVersion versions[3] = {ROWS,FLAT,INPLACE};
   unsigned int version;
   unsigned long reference_nodes, reference_depth;

   Oracle(board,n,&expected);
   reference_nodes = 0;
   reference_depth = 0;
   for (version = 0; version < 3; version = version + 1) {
      ResetHooks();
      PrepareBoard(&maze,board,n,versions[version]);
      maze.options.list = 1;
      TestCaptureEnabled = 1;
      RunSearches(maze.maze,&maze.options,&maze.work,&maze.result,&maze.times);
      Require(SolverExitCode(maze.result.status) == 0,"listado completo");
      Require(maze.result.paths == expected.count,"conteo coincide con oraculo");
      CheckCaptured(board,n,&expected);
      CheckRestored(&maze,board);
      if (version == 0) {
         reference_nodes = maze.result.nodes;
         reference_depth = maze.result.max_depth;
      }
      else {
         Require(maze.result.nodes == reference_nodes,"mismos estados entre versiones");
         Require(maze.result.max_depth == reference_depth,"misma profundidad entre versiones");
      }
      TestCaptureLength = 0;
      TestCapture[0] = '\0';
      maze.options.list = 0;
      maze.options.repeat = 5;
      maze.result.repeat_completed = 0;
      RunSearches(maze.maze,&maze.options,&maze.work,&maze.result,&maze.times);
      Require(SolverExitCode(maze.result.status) == 0 && maze.result.repeat_completed == 5,"cinco repeticiones completas");
      Require(maze.result.paths == expected.count && maze.result.nodes == reference_nodes,
              "repeticiones conservan rutas y estados");
      Require(TestCaptureLength == 0,"modo count no imprime rutas");
      CheckRestored(&maze,board);
      KillTestCase(&maze);
      Require(TestLive == 0,"liberacion completa del caso");
   }
   TestBoards = TestBoards + 1;
   return expected.count;
}


void TestAllSmallBoards(void) {

   unsigned char board[16];
   unsigned char sample[16] = {1,0,0,0,1,1,0,1,1,1,0,0,0,1,1,1};
   unsigned int n, cell;
   unsigned long mask, combinations;
   struct OracleResult expected;

   for (n = 1; n <= 3; n = n + 1) {
      combinations = 1UL << (n * n);
      for (mask = 0; mask < combinations; mask = mask + 1) {
         for (cell = 0; cell < n * n; cell = cell + 1)
            board[cell] = (mask >> cell) & 1UL;
         CheckBoard(board,n);
      }
   }
   Require(TestBoards == 530,"530 matrices binarias exhaustivas");
   Require(CheckBoard(sample,4) == 2,"ejemplo publicado tiene dos rutas");
   Oracle(sample,4,&expected);
   Require(strcmp(expected.routes[0],"DDRDRR") == 0 &&
           strcmp(expected.routes[1],"DRDDRR") == 0,"rutas conocidas del ejemplo");
   for (cell = 0; cell < 16; cell = cell + 1)
      board[cell] = 1;
   Require(CheckBoard(board,4) == 184,"cuadrado abierto 4x4 tiene 184 rutas");
}


void TestMazeConstructors(void) {

   struct Maze *maze;
   unsigned int contiguous, calls, failure;
   unsigned long row, column;

   for (contiguous = 0; contiguous < 2; contiguous = contiguous + 1) {
      ResetHooks();
      maze = contiguous ? InitFlatMaze(3) : InitMaze(3);
      Require(maze != NULL && maze->n == 3,"constructor devuelve laberinto de dimension solicitada");
      Require(contiguous ? maze->rows == NULL && maze->flat != NULL :
              maze->rows != NULL && maze->flat == NULL,"solo una representacion reservada");
      for (row = 0; row < 3; row = row + 1)
         for (column = 0; column < 3; column = column + 1)
            Require((contiguous ? maze->flat[row * 3 + column] : maze->rows[row][column]) == 0,
                    "celdas del constructor inicializadas en cero");
      calls = TestCalls;
      KillMaze(maze);
      Require(TestLive == 0 && TestBytes == 0,"KillMaze libera matriz y estructura");
      for (failure = 1; failure <= calls; failure = failure + 1) {
         ResetHooks();
         TestFailAt = failure;
         maze = contiguous ? InitFlatMaze(3) : InitMaze(3);
         Require(maze == NULL,"constructor devuelve NULL si falla cualquier calloc");
         Require(TestLive == 0 && TestBytes == 0,"constructor libera reservas parciales antes de devolver NULL");
         KillMaze(maze);
         TestAllocationCases = TestAllocationCases + 1;
      }
   }
   ResetHooks();
   Require(InitMaze(0) == NULL && InitFlatMaze(0) == NULL,"constructores rechazan dimension cero");
   Require(InitMaze(ULONG_MAX) == NULL && InitFlatMaze(ULONG_MAX) == NULL,"constructores rechazan dimensiones desbordadas");
   Require(TestCalls == 0,"dimensiones invalidas no llaman calloc");
   KillMaze(NULL);
}


void TestAllocations(void) {

   struct TestCase maze;
   enum SolverVersion versions[3] = {ROWS,FLAT,INPLACE};
   unsigned int version, failure;
   int allocated;

   for (version = 0; version < 3; version = version + 1) {
      for (failure = 1; failure <= 10; failure = failure + 1) {
         ResetHooks();
         InitTestCase(&maze);
         maze.options.version = versions[version];
         maze.n = 2;
         TestFailAt = failure;
         Require(CalculateTestMemory(&maze),"calculo antes de inyectar fallo");
         allocated = AllocateTestCase(&maze);
         if (allocated)
            Require(TestCalls < failure,"fallo programado posterior a ultima reserva");
         else
            Require(SolverExitCode(maze.result.status) == 3 && strcmp(StatusName(maze.result.status),"allocation_failed") == 0,
                    "fallo de reserva informado");
         KillTestCase(&maze);
         Require(TestLive == 0,"liberacion tras construccion parcial");
         TestAllocationCases = TestAllocationCases + 1;
      }
   }
}


void TestMemoryAndCounters(void) {

   struct TestCase maze;
   enum SolverVersion versions[3] = {ROWS,FLAT,INPLACE};
   unsigned char board[4] = {1,1,1,1};
   unsigned int version;
   unsigned long expected;

   for (version = 0; version < 3; version = version + 1) {
      ResetHooks();
      InitTestCase(&maze);
      maze.options.version = versions[version];
      maze.n = 2;
      expected = 12;
      if (version == 0)
         expected = expected + 4 * sizeof(unsigned char *);
      if (version == 2)
         expected = 8 + sizeof(struct Maze);
      else
         expected = expected + 2 * sizeof(struct Maze);
      Require(CalculateTestMemory(&maze) && maze.heap_bytes == expected,"formula de memoria");
      maze.options.memory_limit = expected - 1;
      Require(!CalculateTestMemory(&maze) && SolverExitCode(maze.result.status) == 3,"limite de memoria estricto");
      Require(TestCalls == 0,"rechazo de presupuesto antes de calloc");
      ResetHooks();
      PrepareBoard(&maze,board,2,versions[version]);
      maze.result.nodes = ULONG_MAX - 2;
      Search(maze.maze,&maze.options,&maze.work,&maze.result,&maze.times,0,0,1);
      Require(SolverExitCode(maze.result.status) == 5 && strcmp(StatusName(maze.result.status),"nodes_overflow") == 0,
              "desbordamiento de estados detectado");
      CheckRestored(&maze,board);
      KillTestCase(&maze);
   }
   ResetHooks();
   PrepareBoard(&maze,board,1,ROWS);
   maze.result.paths = ULONG_MAX;
   Search(maze.maze,&maze.options,&maze.work,&maze.result,&maze.times,0,0,1);
   Require(SolverExitCode(maze.result.status) == 5 && strcmp(StatusName(maze.result.status),"count_overflow") == 0,
           "desbordamiento de rutas detectado");
   KillTestCase(&maze);
   InitTestCase(&maze);
   Require(!CalculateTestMemory(&maze) && maze.result.status == INVALID_INPUT,
           "dimension cero rechazada antes de reservar");
   maze.n = ULONG_MAX;
   Require(!CalculateTestMemory(&maze) && SolverExitCode(maze.result.status) == 2,"producto n*n desbordado");
}


void TestDepth(void) {

   struct TestCase maze;
   unsigned char *board;
   enum SolverVersion versions[3] = {ROWS,FLAT,INPLACE};
   unsigned long row, column;
   unsigned int version, attempt;

   board = calloc(65UL * 65UL,sizeof(unsigned char));
   Require(board != NULL,"reserva de corredor de prueba");
   for (row = 0; row < 65; row = row + 1) {
      if (row % 2 == 0) {
         for (column = 0; column < 65; column = column + 1)
            board[row * 65 + column] = 1;
      }
      else {
         column = 0;
         if (((row - 1) / 2) % 2 == 0)
            column = 64;
         board[row * 65 + column] = 1;
      }
   }
   for (version = 0; version < 3; version = version + 1) {
      for (attempt = 0; attempt < 2; attempt = attempt + 1) {
         ResetHooks();
         PrepareBoard(&maze,board,65,versions[version]);
         if (attempt == 1) {
            maze.options.depth_limit = 2177;
            maze.options.repeat = 3;
         }
         RunSearches(maze.maze,&maze.options,&maze.work,&maze.result,&maze.times);
         if (attempt == 0)
            Require(SolverExitCode(maze.result.status) == 4 && maze.result.max_depth == 2048 &&
                    strcmp(StatusName(maze.result.status),"depth_limit") == 0,"profundidad detenida en 2048");
         else
            Require(SolverExitCode(maze.result.status) == 0 && maze.result.max_depth == 2177 &&
                    maze.result.paths == 1 && maze.result.repeat_completed == 3,"profundidad exacta 2177");
         CheckRestored(&maze,board);
         KillTestCase(&maze);
         Require(TestLive == 0,"liberacion de corredor");
         TestDepthCases = TestDepthCases + 1;
      }
   }
   free(board);
}


void TestOutputErrors(void) {

   struct TestCase maze;
   unsigned char board[4] = {1,1,1,1};
   enum SolverVersion versions[3] = {ROWS,FLAT,INPLACE};
   unsigned int version;
   int failure;

   for (version = 0; version < 3; version = version + 1) {
      for (failure = 1; failure <= 3; failure = failure + 1) {
         ResetHooks();
         PrepareBoard(&maze,board,2,versions[version]);
         maze.options.list = 1;
         TestCaptureEnabled = 1;
         TestOutputFailure = failure;
         RunSearches(maze.maze,&maze.options,&maze.work,&maze.result,&maze.times);
         Require(SolverExitCode(maze.result.status) == 6 && strcmp(StatusName(maze.result.status),"output_error") == 0,
                 "error inyectado fputs/fputc/fflush informado");
         CheckRestored(&maze,board);
         KillTestCase(&maze);
         Require(TestLive == 0,"liberacion tras error de salida");
         TestOutputCases = TestOutputCases + 1;
      }
   }
}


void TestInputs(void) {

   struct TestCase maze;
   enum SolverVersion versions[3] = {ROWS,FLAT,INPLACE};
   char *invalid[] = {"", "0\n", "-1\n", "x\n", "2\n1 1 1", "1\n2",
                      "1\n-1", "1\nx", "1\n1 0", "1\n1 tail", "1.0\n1",
                      "2\n1 1 1 1x", "1\n1.0", "1\n01",
                      "99999999999999999999999999999999999999\n", NULL};
   unsigned int version, sample;
   int valid;

   for (version = 0; version < 3; version = version + 1) {
      for (sample = 0; invalid[sample] != NULL; sample = sample + 1) {
         ResetHooks();
         InitTestCase(&maze);
         maze.options.version = versions[version];
         TestInput = invalid[sample];
         valid = ReadDimension(&maze.n);
         if (valid)
            valid = CalculateTestMemory(&maze) && AllocateTestCase(&maze) && ReadMaze(maze.maze);
         Require(!valid,"entrada invalida rechazada");
         KillTestCase(&maze);
         Require(TestLive == 0,"liberacion tras entrada invalida");
         TestInputCases = TestInputCases + 1;
      }
      ResetHooks();
      InitTestCase(&maze);
      maze.options.version = versions[version];
      TestInput = " 2\t1 1\r\n1 1  \n";
      Require(ReadDimension(&maze.n) && CalculateTestMemory(&maze) &&
              AllocateTestCase(&maze) && ReadMaze(maze.maze),"lectura valida con espacios variados");
      RunSearches(maze.maze,&maze.options,&maze.work,&maze.result,&maze.times);
      Require(SolverExitCode(maze.result.status) == 0 && maze.result.paths == 2,"resultado tras lectura valida");
      KillTestCase(&maze);
      TestInputCases = TestInputCases + 1;
      ResetHooks();
      TestInput = "1 1";
      TestInputFailure = 1;
      Require(!ReadDimension(&maze.n),"error de flujo de entrada detectado");
      TestInputCases = TestInputCases + 1;
   }
   ResetHooks();
}


void TestArguments(void) {

   struct TestCase maze;
   char *invalid[][6] = {{"ratmaze","--version","unknown",NULL},
                         {"ratmaze","--mode","unknown",NULL},
                         {"ratmaze","--repeat","0",NULL},
                         {"ratmaze","--max-depth","0",NULL},
                         {"ratmaze","--memory-limit","-1",NULL},
                         {"ratmaze","--unknown","1",NULL},
                         {"ratmaze","--repeat",NULL},
                         {"ratmaze","--mode","list","--repeat","2",NULL}};
   char *valid[] = {"ratmaze","--version","inplace","--mode","count",
                    "--repeat","5","--max-depth","100","--memory-limit","1000"};
   char overflow[64];
   unsigned long value;
   unsigned int sample;
   int argc;

   for (sample = 0; sample < 8; sample = sample + 1) {
      InitTestCase(&maze);
      argc = 0;
      while (invalid[sample][argc] != NULL)
         argc = argc + 1;
      Require(!ParseArguments(&maze.options,argc,invalid[sample]),"argumentos invalidos rechazados");
      TestArgumentCases = TestArgumentCases + 1;
   }
   InitTestCase(&maze);
   Require(ParseArguments(&maze.options,11,valid) && maze.options.repeat == 5 && maze.options.depth_limit == 100 &&
           maze.options.memory_limit == 1000 && maze.options.version == INPLACE,
           "argumentos validos aplicados");
   TestArgumentCases = TestArgumentCases + 1;
   sprintf(overflow,"%lu0",ULONG_MAX);
   Require(!ParsePositive(overflow,&value),"numero de argumento desbordado rechazado");
   Require(!ParsePositive("+1",&value) && !ParsePositive(" 1",&value),"entero decimal estricto");
   Require(ParsePositive("0001",&value) && value == 1,"ceros iniciales admitidos en entero");
   TestArgumentCases = TestArgumentCases + 3;
}


int main(void) {

   TestMazeConstructors();
   TestAllSmallBoards();
   TestAllocations();
   TestMemoryAndCounters();
   TestDepth();
   TestOutputErrors();
   TestInputs();
   TestArguments();
   Require(TestLive == 0,"registro final de memoria vacio");
   printf("Estado: APROBADO\n");
   printf("Aserciones verificadas: %lu\n",TestAssertions);
   printf("Matrices binarias exhaustivas n=1,2,3: 530\n");
   printf("Matrices funcionales totales: %lu (incluye ejemplo y abierta 4x4)\n",TestBoards);
   printf("Versiones: rows, flat, inplace; conteo repetido 5 veces por caso\n");
   printf("Rutas conocidas: DDRDRR, DRDDRR; abierta 4x4: 184\n");
   printf("Intentos de asignacion con inyeccion de fallos: %lu\n",TestAllocationCases);
   printf("Escenarios de profundidad 2048/2177: %lu\n",TestDepthCases);
   printf("Errores de salida inyectados fputs/fputc/fflush: %lu\n",TestOutputCases);
   printf("Casos de lectura: %lu; casos de argumentos: %lu\n",TestInputCases,TestArgumentCases);
   printf("Verificados: rutas validas, orden, unicidad, equivalencia, restauracion,\n");
   printf("limites de memoria, overflow de contadores y liberacion de reservas parciales.\n");
   printf("Alcance: prueba directa de funciones con hooks; no prueba procesos externos,\n");
   printf("energia, rendimiento ni el transporte real del sistema operativo.\n");
   return 0;
}
