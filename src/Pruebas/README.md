# Compilación y Ejecución

## 1. Compilación del Código Fuente (C)

```bash
# versión recursiva
gcc -O3 rat_in_a_maze_rec.c -o rat_in_a_maze_rec

# versión secuencial
gcc -O3 rat_in_a_maze_sec.c -o rat_in_a_maze_sec
```

## 2. Ejecución del Programa.

Para ejecutar el binario se realizará de las siguientes formas (Desde n=26 el tiempo aumenta demasiado):

### 2.1 Para mostrar y contar caminos

```bash
# versión recursiva
./rat_in_a_maze_rec -a < maze(n).txt

# versión secuencial
./rat_in_a_maze_sec -a < maze(n).txt
```

### 2.2 Para solo contar caminos (para no contar el tiempo de impresión)

```bash
# versión recursiva
./rat_in_a_maze_rec -c < maze(n).txt

# versión secuencial
./rat_in_a_maze_sec -c < maze(n).txt
```