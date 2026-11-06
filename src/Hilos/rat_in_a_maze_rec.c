
/***************************************************************
 *
 * rat_in_a_maze.c: Solucion recursiva para Rat in a Maze.
 * Autores: Maximiliano Correa, Juan Valdebenito
 *
 ***************************************************************/

#include <stdio.h>
#include <stdlib.h>

int IsValid(int row, int col, int n, int maze[],
            int visited[])
{
    int pos;

    if (row < 0 || row >= n || col < 0 || col >= n)
    {
        return 0;
    }

    pos = row * n + col;

    if (maze[pos] == 0 || visited[pos] == 1)
    {
        return 0;
    }

    return 1;
}

void FindPaths(int row, int col, int n, int maze[],
               int visited[], char path[], int path_length,
               int *solution_count, int row_change[],
               int col_change[], char moves[])
{
    int direction;
    int next_row;
    int next_col;
    int next_pos;

    if (row == n - 1 && col == n - 1)
    {
        path[path_length] = '\0';

        printf("%s\n", path);

        *solution_count = *solution_count + 1;

        return;
    }

    for (direction = 0; direction < 4;
         direction = direction + 1)
    {
        next_row = row + row_change[direction];
        next_col = col + col_change[direction];

        if (IsValid(next_row, next_col, n, maze, visited))
        {
            next_pos = next_row * n + next_col;

            visited[next_pos] = 1;
            path[path_length] = moves[direction];

            FindPaths(next_row, next_col, n, maze,
                      visited, path, path_length + 1,
                      solution_count, row_change,
                      col_change, moves);

            visited[next_pos] = 0;
        }
    }
}

int main(void)
{
    int n;
    int total;
    int i;
    int solution_count;
    int *maze;
    int *visited;
    char *path;
    int row_change[4] = {1, 0, 0, -1};
    int col_change[4] = {0, -1, 1, 0};
    char moves[4] = {'D', 'L', 'R', 'U'};

    if (scanf("%d", &n) != 1)
    {
        printf("Error: invalid input.\n");
        return 1;
    }

    if (n <= 0 || n > 100)
    {
        printf("Error: invalid maze size.\n");
        return 1;
    }

    total = n * n;

    maze = calloc(total, sizeof(int));
    visited = calloc(total, sizeof(int));
    path = calloc(total + 1, sizeof(char));

    if (maze == NULL || visited == NULL || path == NULL)
    {
        printf("Error: memory allocation failed.\n");

        free(maze);
        free(visited);
        free(path);

        return 1;
    }

    for (i = 0; i < total; i = i + 1)
    {
        if (scanf("%d", &maze[i]) != 1)
        {
            printf("Error: invalid maze data.\n");

            free(maze);
            free(visited);
            free(path);

            return 1;
        }

        if (maze[i] != 0 && maze[i] != 1)
        {
            printf("Error: maze values must be 0 or 1.\n");

            free(maze);
            free(visited);
            free(path);

            return 1;
        }
    }

    solution_count = 0;

    if (maze[0] == 1 && maze[total - 1] == 1)
    {
        visited[0] = 1;

        FindPaths(0, 0, n, maze, visited, path, 0,
                  &solution_count, row_change,
                  col_change, moves);
    }

    printf("Total paths: %d\n", solution_count);

    free(maze);
    free(visited);
    free(path);

    return 0;
}
