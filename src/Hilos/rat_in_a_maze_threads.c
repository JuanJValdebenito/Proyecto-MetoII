/*******************************************************************************
*
* rat_in_a_maze_threadpool.c: Solucion para Rat in a Maze utilizando hilos.
* Autores: Maximiliano Correa, Juan Valdebenito         
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_PATH 1500
#define QUEUE_SIZE 5000

struct Task {
    int r;
    int c;
    int n;
    unsigned char **maze;
    char path[MAX_PATH];
    int pathLen;
};

struct TaskQueue {
    struct Task tasks[QUEUE_SIZE];
    int head;
    int tail;
    int count;
    int active_workers;
    int shutdown;
    pthread_mutex_t lock;
    pthread_mutex_t print_lock;
    pthread_cond_t not_empty;
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

int IsValid(int r, int c, int n, unsigned char **maze) {
    if (r >= 0 && c >= 0 && r < n && c < n) {
        if (maze[r][c] == 1) {
            return 1;
        }
    }
    return 0;
}

void FindPathLocal(int r, int c, int n, unsigned char **maze, char *path, int pathLen, pthread_mutex_t *print_lock) {
    int i, nr, nc;
    int dr[4] = {1, 0, 0, -1};
    int dc[4] = {0, -1, 1, 0};
    char dir[4] = {'D', 'L', 'R', 'U'};

    if (r == n - 1 && c == n - 1) {
        path[pathLen] = '\0';
        pthread_mutex_lock(print_lock);
        printf("Camino encontrado: %s\n", path);
        pthread_mutex_unlock(print_lock);
        return;
    }

    maze[r][c] = 0;

    for (i = 0; i < 4; i = i + 1) {
        nr = r + dr[i];
        nc = c + dc[i];

        if (IsValid(nr, nc, n, maze) == 1) {
            path[pathLen] = dir[i];
            FindPathLocal(nr, nc, n, maze, path, pathLen + 1, print_lock);
        }
    }

    maze[r][c] = 1;
}

void QueueInit(struct TaskQueue *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->active_workers = 0;
    q->shutdown = 0;
    pthread_mutex_init(&(q->lock), NULL);
    pthread_mutex_init(&(q->print_lock), NULL);
    pthread_cond_init(&(q->not_empty), NULL);
}

void QueuePush(struct TaskQueue *q, struct Task t) {
    pthread_mutex_lock(&(q->lock));
    q->tasks[q->tail] = t;
    q->tail = (q->tail + 1) % QUEUE_SIZE;
    q->count = q->count + 1;
    pthread_cond_signal(&(q->not_empty));
    pthread_mutex_unlock(&(q->lock));
}

int QueuePop(struct TaskQueue *q, struct Task *t) {
    pthread_mutex_lock(&(q->lock));
    
    q->active_workers = q->active_workers - 1;
    
    while (q->count == 0 && q->shutdown == 0) {
        pthread_cond_wait(&(q->not_empty), &(q->lock));
    }
    
    if (q->count == 0 && q->shutdown == 1) {
        pthread_mutex_unlock(&(q->lock));
        return 0;
    }
    
    *t = q->tasks[q->head];
    q->head = (q->head + 1) % QUEUE_SIZE;
    q->count = q->count - 1;
    
    q->active_workers = q->active_workers + 1;
    
    pthread_mutex_unlock(&(q->lock));
    return 1;
}

void* WorkerRoutine(void *arg) {
    struct TaskQueue *q;
    struct Task t;
    q = (struct TaskQueue *)arg;

    while (1) {
        if (QueuePop(q, &t) == 0) {
            break;
        }

        FindPathLocal(t.r, t.c, t.n, t.maze, t.path, t.pathLen, &(q->print_lock));
        FreeMaze(t.n, t.maze);
    }
    return NULL;
}

void main(int argc, char *argv[]) {
    int n, i, j, val, tIdx, num_threads;
    unsigned char **maze;
    struct TaskQueue queue;
    pthread_t *workers;
    FILE *file;
    long long l;

    if (argc < 3) {
        printf("Uso: %s <archivo_laberinto.txt> <num_hilos>\n", argv[0]);
        return;
    }

    file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error: No se pudo abrir el archivo %s\n", argv[1]);
        return;
    }

    num_threads = atoi(argv[2]);
    if (num_threads <= 0) {
        printf("Error: El número de hilos debe ser mayor a 0\n");
        fclose(file);
        return;
    }

    fscanf(file, "%d", &n);
    QueueInit(&queue);

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

    workers = (pthread_t *)calloc(num_threads, sizeof(pthread_t));

    if (maze[0][0] == 1 && maze[n - 1][n - 1] == 1) {
        struct Task rootTask;
        rootTask.r = 0;
        rootTask.c = 0;
        rootTask.n = n;
        rootTask.maze = CopyMaze(n, maze);
        rootTask.maze[0][0] = 0;
        rootTask.pathLen = 0;
        rootTask.path[0] = '\0';

        QueuePush(&queue, rootTask);
    }

    queue.active_workers = num_threads;
    for (tIdx = 0; tIdx < num_threads; tIdx = tIdx + 1) {
        pthread_create(&workers[tIdx], NULL, WorkerRoutine, (void *)&queue);
    }

    while (1) {
        pthread_mutex_lock(&(queue.lock));
        if (queue.count == 0 && queue.active_workers == 0) {
            queue.shutdown = 1;
            pthread_cond_broadcast(&(queue.not_empty));
            pthread_mutex_unlock(&(queue.lock));
            break;
        }
        pthread_mutex_unlock(&(queue.lock));
        for (l = 0; l < 100000; l = l + 1);
    }

    for (tIdx = 0; tIdx < num_threads; tIdx = tIdx + 1) {
        pthread_join(workers[tIdx], NULL);
    }

    pthread_mutex_destroy(&(queue.lock));
    pthread_mutex_destroy(&(queue.print_lock));
    pthread_cond_destroy(&(queue.not_empty));

    for (i = 0; i < n; i = i + 1) {
        free(maze[i]);
    }
    free(maze);
    free(workers);
}