/*****************************************************
 * timer.c: medicion temporal con el reloj de C.
 * Autores: Maximiliano Correa Valdes,
 *          Juan Valdebenito Sanhueza.
 * Santiago de Chile, 31/10/2026.
 ****************************************************/

#include "timer.h"

void InitTimings(struct Timings *times) {

   times->prep_clock_s = 0.0f;
   times->search_clock_s = 0.0f;
   times->output_clock_s = 0.0f;
}

/* -1 indica reloj no disponible o desbordado. */
float Elapsed(clock_t start, clock_t end) {

   if (start < 0 || end < 0 || end < start)
      return -1.0f;
   return (float)(end - start) / (float)CLOCKS_PER_SEC;
}

float AddTime(float total, float elapsed) {

   if (total < 0.0f || elapsed < 0.0f)
      return -1.0f;
   return total + elapsed;
}
