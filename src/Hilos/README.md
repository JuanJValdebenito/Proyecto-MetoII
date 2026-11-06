## Compilación y Ejecución

### 1. Compilación del Código Fuente (C)

```bash
# versión recursiva
gcc -O3 rat_in_a_maze_rec.c -o rat_in_a_maze_rec

# versión secuencial
gcc -O3 rat_in_a_maze_sec.c -o rat_in_a_maze_sec

# versión con hilos
gcc -O3 -pthread rat_in_a_maze_threads.c -o rat_in_a_maze_threads
```

### 2. Ejecución del Programa.

Para ejecutar el binario.

```bash
# versión recursiva
./rat_in_a_maze_rec < maze(n).txt

# versión secuencial
./rat_in_a_maze_sec < maze(n).txt

# versión con hilos
./rat_in_a_maze_threads < maze(n).txt
```
