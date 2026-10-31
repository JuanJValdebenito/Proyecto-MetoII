/*****************************************************
 * timer.h: tiempos separados de los datos del laberinto.
 * Autores: Maximiliano Correa Valdes,
 *          Juan Valdebenito Sanhueza.
 * Santiago de Chile, 31/10/2026.
 ****************************************************/

#ifndef TIMER_H
#define TIMER_H

#include <time.h>

#ifdef _WIN32
#define CLOCK_SEMANTICS "windows_elapsed"
#else
#define CLOCK_SEMANTICS "iso_cpu"
#endif

struct Timings {
   float prep_clock_s, search_clock_s, output_clock_s;
};

void InitTimings(struct Timings *times);
float Elapsed(clock_t start, clock_t end);
float AddTime(float total, float elapsed);

#endif
