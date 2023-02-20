#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#define DT 0.05
#define NUM_THREADS 8

typedef struct {
    double x, y;
} vector;

typedef struct Body {
    double mass;
    double x, y;
    double vx, vy;
} Body;

int timeSteps;
Body *bodiesArr;
vector *accelerations;

int bodiesSize;
double GravConstant;
int counter = 0;
pthread_cond_t cv;


pthread_t *threads;
pthread_mutex_t mutex;

void initiateSystem(char *fileName) {
    int i;
    FILE *fp = fopen(fileName, "r");

    fscanf(fp, "%lf%d%d", &GravConstant, &bodiesSize, &timeSteps);

    bodiesArr = (Body *) malloc(bodiesSize * sizeof(Body));
    threads = (pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    accelerations = (vector *) malloc(bodiesSize * sizeof(vector));

    for (i = 0; i < bodiesSize; i++) {
        fscanf(fp, "%lf", &bodiesArr[i].mass);
        fscanf(fp, "%lf%lf", &bodiesArr[i].x, &bodiesArr[i].y);
        fscanf(fp, "%lf%lf", &bodiesArr[i].vx, &bodiesArr[i].vy);
    }

    fclose(fp);
}

void *computeAccelerationsThread(void *arg) {
    int i, j;
    int thread_id = *(int *) arg;
    int bodies_per_thread = bodiesSize / NUM_THREADS;
    int start_index = thread_id * bodies_per_thread;
    int end_index = (thread_id == NUM_THREADS - 1) ? bodiesSize : start_index + bodies_per_thread;

    for (i = start_index; i < end_index; i++) {
        accelerations[i].x = 0;
        accelerations[i].y = 0;
        for (j = 0; j < bodiesSize; j++) {
            if (i != j) {
                double dx = bodiesArr[j].x - bodiesArr[i].x;
                double dy = bodiesArr[j].y - bodiesArr[i].y;
                double d = sqrt(dx * dx + dy * dy);
                double temp = GravConstant * bodiesArr[j].mass / (d * d * d);
                accelerations[i].x = temp * dx;
                accelerations[i].y = temp * dy;
            }
        }
    }
    return NULL;
}

void *computeVelocitiesThread(void *arg) {
    int i;
    int thread_id = *(int *) arg;
    int bodies_per_thread = bodiesSize / NUM_THREADS;
    int start_index = thread_id * bodies_per_thread;
    int end_index = (thread_id == NUM_THREADS - 1) ? bodiesSize : start_index + bodies_per_thread;

    for (i = start_index; i < end_index; i++) {
        bodiesArr[i].vx = bodiesArr[i].vx + DT * accelerations[i].x;
        bodiesArr[i].vy = bodiesArr[i].vy + DT * accelerations[i].y;
    }
    return NULL;
}

void *computePositionsThread(void *arg) {
    int i;
    int thread_id = *(int *) arg;
    int bodies_per_thread = bodiesSize / NUM_THREADS;
    int start_index = thread_id * bodies_per_thread;
    int end_index = (thread_id == NUM_THREADS - 1) ? bodiesSize : start_index + bodies_per_thread;
    for (i = start_index; i < end_index; i++) {
        bodiesArr[i].x = bodiesArr[i].x + DT * bodiesArr[i].vx;
        bodiesArr[i].y = bodiesArr[i].y + DT * bodiesArr[i].vy;
    }
    return NULL;
}

void resolveCollisions() {
    int i, j;

    for (i = 0; i < bodiesSize - 1; i++)
        for (j = i + 1; j < bodiesSize; j++) {
            if (bodiesArr[i].x == bodiesArr[j].x && bodiesArr[i].y == bodiesArr[j].y) {
                double tempX = bodiesArr[i].vx;
                double tempY = bodiesArr[i].vy;
                bodiesArr[i].vx = bodiesArr[j].vx;
                bodiesArr[i].vy = bodiesArr[j].vy;
                bodiesArr[j].vx = tempX;
                bodiesArr[j].vy = tempY;
            }
        }
}

void *threadFunc(void *arg) {
    computeAccelerationsThread(arg);
    pthread_mutex_lock(&mutex);
    counter += 1;
    while (counter != NUM_THREADS) {
        pthread_cond_wait(&cv, &mutex);
    }
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&mutex);
    computePositionsThread(arg);
    computeVelocitiesThread(arg);
}

void simulateSystem() {
    int i;
    FILE *fp = fopen("", "r");
    fp = fopen("output.txt", "w");
    FILE *fpForPython = fopen("", "r");
    fpForPython = fopen("outputForPython.csv", "w");
    int *thread_ids = (int *) malloc(NUM_THREADS * sizeof(int));
    fprintf(fpForPython, "t");
    for (i = 0; i < bodiesSize; i++) {
        fprintf(fpForPython, ",x%d,y%d", i + 1, i + 1);
    }
    pthread_cond_init(&cv, NULL);
    pthread_mutex_init(&mutex, NULL);
    for (int t = 0; t < timeSteps; t++) {
        fprintf(fp, "\nCycle %d\n", t + 1);
        fprintf(fpForPython, ",\n%d",t+1);
        for (i = 0; i < NUM_THREADS; i++) {
            thread_ids[i] = i;
            pthread_create(&threads[i], NULL, threadFunc, &thread_ids[i]);
        }

        for (i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }
        counter = 0;
        resolveCollisions();
        for (i = 0; i < bodiesSize; i++) {
            fprintf(fp, "Body %d : %lf\t%lf\t%lf\t%lf\n", i + 1, bodiesArr[i].x, bodiesArr[i].y, bodiesArr[i].vx,
                    bodiesArr[i].vy);
            fprintf(fpForPython, ",%lf,%lf", bodiesArr[i].x, bodiesArr[i].y);
        }

    }
    fprintf(fpForPython, ",");
    pthread_cond_destroy(&cv);
    pthread_mutex_destroy(&mutex);
    free(thread_ids);
}


int main(int argc, char *argv[]) {
    initiateSystem("c:/Users/evsharun/CLionProjects/untitled4/input.txt");
    simulateSystem();
    free(bodiesArr);
    free(threads);

    return 0;
}