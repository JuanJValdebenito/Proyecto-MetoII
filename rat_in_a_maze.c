/*******************************************************************************
*
* rat_in_a_maze.c: Solucion al problema Rat in a Maze con matriz unsigned char.
* Autores: Maximiliano Correa, Juan Valdebenito         
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#define MAX_PATH 100

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

void main() {
    int n, i, j;
    unsigned char **maze;
    char path[MAX_PATH];

    n = 4;

    maze = (unsigned char **)calloc(n, sizeof(unsigned char *));
    for (i = 0; i < n; i = i + 1) {
        maze[i] = (unsigned char *)calloc(n, sizeof(unsigned char));
    }

    maze[0][0] = 1; maze[0][1] = 0; maze[0][2] = 0; maze[0][3] = 0;
    maze[1][0] = 1; maze[1][1] = 1; maze[1][2] = 0; maze[1][3] = 1;
    maze[2][0] = 1; maze[2][1] = 1; maze[2][2] = 0; maze[2][3] = 0;
    maze[3][0] = 0; maze[3][1] = 1; maze[3][2] = 1; maze[3][3] = 1;

    printf("Caminos encontrados para salir del laberinto:\n\n");

    if (maze[0][0] == 1 && maze[n - 1][n - 1] == 1) {
        FindPath(0, 0, n, maze, path, 0);
    }

    for (i = 0; i < n; i = i + 1) {
        free(maze[i]);
    }
    free(maze);
}