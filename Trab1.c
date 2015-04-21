#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>

#define VECT_SIZE   100
#define BUCKETS     5
#define THREADS     10

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
    
    srand(666);
    for (i = 0; i < VECT_SIZE; i++) {
        vector[i] = rand() % VECT_SIZE;
        printf("%d, ", vector[i]);
    }
    
    arrayBuckets = (bucket_t *)calloc(sizeof(bucket_t), BUCKETS);
    int quantMod = VECT_SIZE % BUCKETS - 1;
    int quantDiv = VECT_SIZE / BUCKETS;
    arrayBuckets[0].minValue = 0;
    arrayBuckets[0].maxValue = quantDiv + (quantMod-- > 0)?1:0;
    arrayBuckets[0].bucket = (int *)malloc(sizeof(int) * VECT_SIZE);
    for (i = 1; i < BUCKETS; i++) {
        arrayBuckets[i].minValue = arrayBuckets[i - 1].maxValue + 1;
        arrayBuckets[i].maxValue = arrayBuckets[i].minValue + quantDiv + (quantMod-- > 0)?1:0;
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
    
    //realloc arrayBuckets.bucket and delete empty buckets
    
    
    pthread_mutex_init(&usedBucketMutex, NULL);
    myThreads = (pthread_t *)malloc(sizeof(pthread_t) * THREADS);
    for (i = 0; i < THREADS; i++)	
        pthread_create(&myThreads[i], NULL, sortIt, NULL);
    for (i = 0; i < THREADS; i++) {
        pthread_join(myThreads[i],NULL);
    }
    free(myThreads);
    pthread_mutex_destroy(&usedBucketMutex);
    
    for (i = 0; i < BUCKETS; i++) {
        free(arrayBuckets[i].bucket);
    }
    free(arrayBuckets);
    free(vector);
}

void *sortIt(void *arg) {
    int bucketIndex;
    while ("Todo o sempre!") {
        pthread_mutex_lock(&usedBucketMutex);
        bucketIndex = usedBucket++;
        pthread_mutex_unlock(&usedBucketMutex);
        if (bucketIndex >= BUCKETS) {
            break;
        }
        else if (arrayBuckets[bucketIndex].size == 0) {
            continue;
        }
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
