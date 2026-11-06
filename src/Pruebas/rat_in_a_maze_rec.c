
/***************************************************************
 *
 * rat_in_a_maze.c: Solucion recursiva para Rat in a Maze.
 * Autores: Maximiliano Correa, Juan Valdebenito
 *
 ***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int IsValid(int row, int col, int n,
            const int maze[], const char visited[])
{
    size_t position;

    if (row < 0 || row >= n || col < 0 || col >= n) {
        return 0;
    }

    position = (size_t)row * (size_t)n + (size_t)col;

    return maze[position] == 1 && visited[position] == 0;
}

void FindPaths(int row, int col, int n,
               const int maze[], char visited[],
               char path[], int path_length,
               unsigned long long *solution_count,
               const int row_change[], const int col_change[],
               const char moves[], int show_all)
{
    int direction;
    int next_row;
    int next_col;
    size_t position;

    position = (size_t)row * (size_t)n + (size_t)col;

    if (row == n - 1 && col == n - 1) {
        *solution_count = *solution_count + 1;

        if (show_all != 0) {
            path[path_length] = '\0';
            printf("%s\n", path);
        }

        return;
    }

    visited[position] = 1;

    for (direction = 0; direction < 4; direction = direction + 1) {
        next_row = row + row_change[direction];
        next_col = col + col_change[direction];

        if (IsValid(next_row, next_col, n, maze, visited)) {
            if (show_all != 0) {
                path[path_length] = moves[direction];
            }

            FindPaths(next_row, next_col, n,
                      maze, visited,
                      path, path_length + 1,
                      solution_count,
                      row_change, col_change, moves,
                      show_all);
        }
    }

    visited[position] = 0;
}

int main(int argc, char *argv[])
{
    int n;
    int i;
    int show_all;
    int *maze;
    char *visited;
    char *path;
    unsigned long long solution_count;
    size_t total;
    int row_change[4] = {1, 0, 0, -1};
    int col_change[4] = {0, -1, 1, 0};
    char moves[4] = {'D', 'L', 'R', 'U'};

    maze = NULL;
    visited = NULL;
    path = NULL;
    solution_count = 0;

    if (argc != 2 ||
        (strcmp(argv[1], "-c") != 0 &&
         strcmp(argv[1], "-a") != 0)) {
        fprintf(stderr, "Uso: %s -c | -a < maze.txt\n", argv[0]);
        return EXIT_FAILURE;
    }

    show_all = (strcmp(argv[1], "-a") == 0);

    if (scanf("%d", &n) != 1 || n < 1 || n > 100) {
        fprintf(stderr, "Error: tamaño de matriz inválido (1 a 100).\n");
        return EXIT_FAILURE;
    }

    total = (size_t)n * (size_t)n;

    maze = calloc(total, sizeof(int));
    visited = calloc(total, sizeof(char));

    if (show_all != 0) {
        path = calloc(total + 1, sizeof(char));
    }

    if (maze == NULL || visited == NULL ||
        (show_all != 0 && path == NULL)) {
        fprintf(stderr, "Error: no se pudo reservar memoria.\n");
        free(maze);
        free(visited);
        free(path);
        return EXIT_FAILURE;
    }

    for (i = 0; i < (int)total; i = i + 1) {
        if (scanf("%d", &maze[i]) != 1 ||
            (maze[i] != 0 && maze[i] != 1)) {
            fprintf(stderr, "Error: matriz inválida.\n");
            free(maze);
            free(visited);
            free(path);
            return EXIT_FAILURE;
        }
    }

    if (maze[0] == 1 && maze[total - 1] == 1) {
        FindPaths(0, 0, n, maze, visited,
                  path, 0, &solution_count,
                  row_change, col_change, moves, show_all);
    }

    if (show_all != 0) {
        printf("Total paths: %llu\n", solution_count);
    } else {
        printf("Total paths: %llu\n", solution_count);
    }

    free(maze);
    free(visited);
    free(path);

    return EXIT_SUCCESS;
}