#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>
#include <time.h>

#define VECT_SIZE   100000
#define BUCKETS     5
#define THREADS     5 

typedef struct {
    int size, minValue, maxValue;
    int *bucket;
} bucket_t;

int usedBucket;
bucket_t *arrayBuckets;

pthread_mutex_t usedBucketMutex;

void bubbleSort(int *, int);
void *sortIt(void *);

int main(int argc, char **argv) {
    pthread_t *myThreads;
    int i, *vector;

    vector = (int *)malloc(sizeof(int) * VECT_SIZE);

    srand(time(NULL));
    for (i = 0; i < VECT_SIZE; i++) {
        vector[i] = rand() % VECT_SIZE;
        printf("%d ", vector[i]);
    }
    printf("\n");

    arrayBuckets = (bucket_t *)calloc(sizeof(bucket_t), BUCKETS);
    int quantMod = VECT_SIZE % BUCKETS;
    int quantDiv = VECT_SIZE / BUCKETS;
    arrayBuckets[0].minValue = 0;
    arrayBuckets[0].maxValue = quantDiv + ((quantMod-- > 0)?1:0) - 1;
    for (i = 1; i < BUCKETS; i++) {
        arrayBuckets[i].minValue = arrayBuckets[i - 1].maxValue + 1;
        arrayBuckets[i].maxValue = arrayBuckets[i].minValue + quantDiv + ((quantMod-- > 0)?1:0) - 1;
    }

    for (i = 0; i < VECT_SIZE; i++) {
        int j;
        for (j = 0; j < BUCKETS; j++) {
            if ((vector[i] >= arrayBuckets[j].minValue) && (vector[i] <= arrayBuckets[j].maxValue)) {
                arrayBuckets[j].size++;
                break;
            }
        }
    }

    for (i = 0; i < BUCKETS; i++) {
        arrayBuckets[i].bucket = (int *)malloc(sizeof(int) * arrayBuckets[i].size);
        arrayBuckets[i].size = 0;
    }

    for (i = 0; i < VECT_SIZE; i++) {
        int j;
        for (j = 0; j < BUCKETS; j++) {
            if ((vector[i] >= arrayBuckets[j].minValue) && (vector[i] <= arrayBuckets[j].maxValue)) {
                arrayBuckets[j].bucket[arrayBuckets[j].size++] = vector[i];
                break;
            }
        }
    }

    for (i = 0; i < BUCKETS; i++) {
        int j;
        printf("Bucket %d (min= %d, max=%d): ",i, arrayBuckets[i].minValue, arrayBuckets[i].maxValue);
        for (j = 0; j < arrayBuckets[i].size; j++)
            printf("%d ",arrayBuckets[i].bucket[j]);
        printf("\n");
    }
    printf("\n");

    usedBucket = 0;
    pthread_mutex_init(&usedBucketMutex, NULL);
    myThreads = (pthread_t *)malloc(sizeof(pthread_t) * THREADS);
    for (i = 0; i < THREADS; i++)
        pthread_create(&myThreads[i], NULL, sortIt, (void *)i);
    for (i = 0; i < THREADS; i++) {
        pthread_join(myThreads[i],NULL);
    }

    for (i = 0; i < BUCKETS; i++) {
        int j;
        for (j = 0; j < arrayBuckets[i].size; j++)
            printf("%d ",arrayBuckets[i].bucket[j]);
    }
    printf("\n");

    free(myThreads);
    pthread_mutex_destroy(&usedBucketMutex);

    for (i = 0; i < BUCKETS; i++) {
        free(arrayBuckets[i].bucket);
    }
    free(arrayBuckets);
    free(vector);
}

void *sortIt(void *arg) {
    int threadIndex = (int)arg;
    int bucketIndex;
    while ("The sun still hot!") {
        pthread_mutex_lock(&usedBucketMutex);
        bucketIndex = usedBucket++;
        pthread_mutex_unlock(&usedBucketMutex);
        if (bucketIndex >= BUCKETS) {
            break;
        }
        else if (arrayBuckets[bucketIndex].size == 0) {
            continue;
        }
        printf("Thread %d processando bucket %d\n", threadIndex, bucketIndex);
        bubbleSort(arrayBuckets[bucketIndex].bucket, arrayBuckets[bucketIndex].size);
    }
    return NULL;
}

void bubbleSort(int *buf, int size) {
    int aux, i, j;
    for(i=size-1; i >= 1; i--)
        for(j=0; j < i ; j++)
            if(buf[j]>buf[j+1]) {
                aux = buf[j];
                buf[j] = buf[j+1];
                buf[j+1] = aux;
            }
}

