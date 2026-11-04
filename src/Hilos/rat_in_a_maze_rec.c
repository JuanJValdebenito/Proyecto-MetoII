/*******************************************************************************
*
* rat_in_a_maze.c: Solucion para Rat in a Maze.
* Autores: Maximiliano Correa, Juan Valdebenito         
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#define MAX_PATH 1500

int IsValid(int r, int c, int n, unsigned char **maze) {
    if (r >= 0 && c >= 0 && r < n && c < n) {
        if (maze[r][c] == 1) {
            return 1;
        }
    }
    return 0;
}

void FindPath(int r, int c, int n, unsigned char **maze, char *path, int pathLen) {
    int i, nr, nc;
    int dr[4] = {1, 0, 0, -1};
    int dc[4] = {0, -1, 1, 0};
    char dir[4] = {'D', 'L', 'R', 'U'};

    if (r == n - 1 && c == n - 1) {
        path[pathLen] = '\0';
        printf("%s\n", path);
        return;
    }

    maze[r][c] = 0;

    for (i = 0; i < 4; i = i + 1) {
        nr = r + dr[i];
        nc = c + dc[i];

        if (IsValid(nr, nc, n, maze) == 1) {
            path[pathLen] = dir[i];
            FindPath(nr, nc, n, maze, path, pathLen + 1);
        }
    }

    maze[r][c] = 1;
}

void main(int argc, char *argv[]) {
    int n, i, j, val;
    unsigned char **maze;
    char path[MAX_PATH];
    FILE *file;

    if (argc < 2) {
        printf("Uso: %s <archivo_laberinto.txt>\n", argv[0]);
        return;
    }

    file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error: No se pudo abrir el archivo %s\n", argv[1]);
        return;
    }

    fscanf(file, "%d", &n);

    maze = (unsigned char **)calloc(n, sizeof(unsigned char *));
    for (i = 0; i < n; i = i + 1) {
        maze[i] = (unsigned char *)calloc(n, sizeof(unsigned char));
    }

    for (i = 0; i < n; i = i + 1) {
        for (j = 0; j < n; j = j + 1) {
            fscanf(file, "%d", &val);
            maze[i][j] = (unsigned char)val;
        }
    }

    fclose(file);

    printf("Caminos encontrados para salir del laberinto (%dx%d):\n\n", n, n);

    if (maze[0][0] == 1 && maze[n - 1][n - 1] == 1) {
        FindPath(0, 0, n, maze, path, 0);
    }

    for (i = 0; i < n; i = i + 1) {
        free(maze[i]);
    }
    free(maze);
}

