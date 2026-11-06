
/***************************************************************
 *
 * rat_in_a_maze_iterative.c: Solucion iterativa para Rat in a Maze.
 * Autores: Maximiliano Correa, Juan Valdebenito
 *
 ***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Frame {
    int row;
    int col;
    int next_direction;
};

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

int main(int argc, char *argv[])
{
    int n;
    int i;
    int show_all;
    int top;
    int direction;
    int next_row;
    int next_col;
    int row_change[4] = {1, 0, 0, -1};
    int col_change[4] = {0, -1, 1, 0};
    char moves[4] = {'D', 'L', 'R', 'U'};
    int *maze;
    char *visited;
    char *path;
    struct Frame *stack;
    unsigned long long solution_count;
    size_t total;
    size_t position;

    maze = NULL;
    visited = NULL;
    path = NULL;
    stack = NULL;
    solution_count = 0;
    top = -1;

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
    stack = calloc(total, sizeof(struct Frame));

    if (show_all != 0) {
        path = calloc(total + 1, sizeof(char));
    }

    if (maze == NULL || visited == NULL || stack == NULL ||
        (show_all != 0 && path == NULL)) {
        fprintf(stderr, "Error: no se pudo reservar memoria.\n");
        free(maze);
        free(visited);
        free(stack);
        free(path);
        return EXIT_FAILURE;
    }

    for (i = 0; i < (int)total; i = i + 1) {
        if (scanf("%d", &maze[i]) != 1 ||
            (maze[i] != 0 && maze[i] != 1)) {
            fprintf(stderr, "Error: matriz inválida.\n");
            free(maze);
            free(visited);
            free(stack);
            free(path);
            return EXIT_FAILURE;
        }
    }

    if (maze[0] == 1 && maze[total - 1] == 1) {
        top = 0;
        stack[top].row = 0;
        stack[top].col = 0;
        stack[top].next_direction = 0;
        visited[0] = 1;

        while (top >= 0) {
            position = (size_t)stack[top].row * (size_t)n
                       + (size_t)stack[top].col;

            if (stack[top].row == n - 1 &&
                stack[top].col == n - 1) {
                solution_count = solution_count + 1;

                if (show_all != 0) {
                    path[top] = '\0';
                    printf("%s\n", path);
                }

                visited[position] = 0;
                top = top - 1;
                continue;
            }

            if (stack[top].next_direction < 4) {
                direction = stack[top].next_direction;
                stack[top].next_direction =
                    stack[top].next_direction + 1;

                next_row = stack[top].row + row_change[direction];
                next_col = stack[top].col + col_change[direction];

                if (IsValid(next_row, next_col, n, maze, visited)) {
                    if (show_all != 0) {
                        path[top] = moves[direction];
                    }

                    top = top + 1;
                    stack[top].row = next_row;
                    stack[top].col = next_col;
                    stack[top].next_direction = 0;

                    position = (size_t)next_row * (size_t)n
                               + (size_t)next_col;
                    visited[position] = 1;
                }
            } else {
                visited[position] = 0;
                top = top - 1;
            }
        }
    }

    printf("Total paths: %llu\n", solution_count);

    free(maze);
    free(visited);
    free(path);
    free(stack);

    return EXIT_SUCCESS;
}