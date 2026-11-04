/*******************************************************************************
*
* rat_in_a_maze_iterative.c: Solucion secuencial para rat in a maze.
* Autores: Maximiliano Correa, Juan Valdebenito         
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#define MAX_PATH 1500
#define STACK_SIZE 20000

struct Node {
    int r;
    int c;
    int n;
    unsigned char **maze;
    int pathLen;
    char path[MAX_PATH];
};

unsigned char** CopyMaze(int n, unsigned char **src) {
    int i, j;
    unsigned char **dest;
    
    dest = (unsigned char **)calloc(n, sizeof(unsigned char *));
    for (i = 0; i < n; i = i + 1) {
        dest[i] = (unsigned char *)calloc(n, sizeof(unsigned char));
        for (j = 0; j < n; j = j + 1) {
            dest[i][j] = src[i][j];
        }
    }
    return dest;
}

void FreeMaze(int n, unsigned char **maze) {
    int i;
    for (i = 0; i < n; i = i + 1) {
        free(maze[i]);
    }
    free(maze);
}

void main(int argc, char *argv[]) {
    int n, i, j, val, top, idx, nr, nc, k;
    unsigned char **initial_maze;
    FILE *file;
    struct Node *stack;
    struct Node curr;
    struct Node nextNode;
    int dr[4];
    int dc[4];
    char dir[4];

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

    initial_maze = (unsigned char **)calloc(n, sizeof(unsigned char *));
    for (i = 0; i < n; i = i + 1) {
        initial_maze[i] = (unsigned char *)calloc(n, sizeof(unsigned char));
    }

    for (i = 0; i < n; i = i + 1) {
        for (j = 0; j < n; j = j + 1) {
            fscanf(file, "%d", &val);
            initial_maze[i][j] = (unsigned char)val;
        }
    }

    fclose(file);

    dr[0] = 1; dr[1] = 0; dr[2] = 0; dr[3] = -1;
    dc[0] = 0; dc[1] = -1; dc[2] = 1; dc[3] = 0;
    dir[0] = 'D'; dir[1] = 'L'; dir[2] = 'R'; dir[3] = 'U';

    stack = (struct Node *)calloc(STACK_SIZE, sizeof(struct Node));
    top = 0;

    printf("Caminos encontrados para salir del laberinto (%dx%d):\n\n", n, n);

    if (initial_maze[0][0] == 1 && initial_maze[n - 1][n - 1] == 1) {
        stack[top].r = 0;
        stack[top].c = 0;
        stack[top].n = n;
        stack[top].maze = CopyMaze(n, initial_maze);
        stack[top].maze[0][0] = 0; // Marcar origen como visitado
        stack[top].pathLen = 0;
        stack[top].path[0] = '\0';
        top = top + 1;
    }

    while (top > 0) {
        top = top - 1;
        curr = stack[top];

        if (curr.r == n - 1 && curr.c == n - 1) {
            printf("%s\n", curr.path);
            FreeMaze(curr.n, curr.maze);
            continue;
        }

        for (idx = 0; idx < 4; idx = idx + 1) {
            nr = curr.r + dr[idx];
            nc = curr.c + dc[idx];

            if (nr >= 0 && nc >= 0 && nr < n && nc < n) {
                if (curr.maze[nr][nc] == 1) {
                    nextNode.r = nr;
                    nextNode.c = nc;
                    nextNode.n = n;
                    nextNode.maze = CopyMaze(n, curr.maze);
                    nextNode.maze[nr][nc] = 0; // Marcar vecino visitado
                    nextNode.pathLen = curr.pathLen + 1;
                    
                    for (k = 0; k < curr.pathLen; k = k + 1) {
                        nextNode.path[k] = curr.path[k];
                    }
                    nextNode.path[curr.pathLen] = dir[idx];
                    nextNode.path[curr.pathLen + 1] = '\0';

                    stack[top] = nextNode;
                    top = top + 1;
                }
            }
        }
        FreeMaze(curr.n, curr.maze);
    }

    free(stack);
    for (i = 0; i < n; i = i + 1) {
        free(initial_maze[i]);
    }
    free(initial_maze);
}