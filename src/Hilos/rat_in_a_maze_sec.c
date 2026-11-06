
/***************************************************************
 *
 * rat_in_a_maze_iterative.c: Solucion iterativa para Rat in a Maze.
 * Autores: Maximiliano Correa, Juan Valdebenito
 *
 ***************************************************************/

#include <stdio.h>
#include <stdlib.h>

struct Frame
{
    int pos;
    int next_direction;
};

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

int main(void)
{
    int n;
    int total;
    int i;
    int top;
    int current_pos;
    int current_row;
    int current_col;
    int direction;
    int next_row;
    int next_col;
    int next_pos;
    int path_length;
    int solution_count;
    int *maze;
    int *visited;
    char *path;
    struct Frame *stack;
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
    stack = calloc(total, sizeof(struct Frame));

    if (maze == NULL || visited == NULL ||
        path == NULL || stack == NULL)
    {
        printf("Error: memory allocation failed.\n");

        free(maze);
        free(visited);
        free(path);
        free(stack);

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
            free(stack);

            return 1;
        }

        if (maze[i] != 0 && maze[i] != 1)
        {
            printf("Error: maze values must be 0 or 1.\n");

            free(maze);
            free(visited);
            free(path);
            free(stack);

            return 1;
        }
    }

    solution_count = 0;
    top = -1;
    path_length = 0;

    if (maze[0] == 1 && maze[total - 1] == 1)
    {
        top = 0;

        stack[top].pos = 0;
        stack[top].next_direction = 0;

        visited[0] = 1;

        while (top >= 0)
        {
            current_pos = stack[top].pos;

            current_row = current_pos / n;
            current_col = current_pos % n;

            if (current_pos == total - 1)
            {
                path[path_length] = '\0';

                printf("%s\n", path);

                solution_count = solution_count + 1;

                visited[current_pos] = 0;
                top = top - 1;

                if (top >= 0)
                {
                    path_length = path_length - 1;
                }
            }
            else if (stack[top].next_direction >= 4)
            {
                visited[current_pos] = 0;
                top = top - 1;

                if (top >= 0)
                {
                    path_length = path_length - 1;
                }
            }
            else
            {
                direction = stack[top].next_direction;

                stack[top].next_direction =
                    stack[top].next_direction + 1;

                next_row = current_row +
                           row_change[direction];

                next_col = current_col +
                           col_change[direction];

                if (IsValid(next_row, next_col, n,
                            maze, visited))
                {
                    next_pos = next_row * n + next_col;

                    path[path_length] = moves[direction];
                    path_length = path_length + 1;

                    visited[next_pos] = 1;

                    top = top + 1;

                    stack[top].pos = next_pos;
                    stack[top].next_direction = 0;
                }
            }
        }
    }

    printf("Total paths: %d\n", solution_count);

    free(maze);
    free(visited);
    free(path);
    free(stack);

    return 0;
}
