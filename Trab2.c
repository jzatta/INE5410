#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>
#include <time.h>

#include <mpi.h>

#define PRINTVECTOR 1

typedef struct {
    int size, minValue, maxValue;
    int *bucket;
} bucket_t;

bucket_t *arrayBuckets;

void executeMaster(int, int);
void receiveBucket(int, int);
void sendSlave(int, int);
void terminateSlave(int);
void executeSlave(void);
void bubbleSort(int *, int);


int main(int argc, char **argv) {
    int i, *vector, vectSize, numProcess, myRank, numBuckets;
    
    MPI_Init(&argc,&argv);
   //Pega o rank e o numero de processos 
    MPI_Comm_size(MPI_COMM_WORLD,&numProcess);
    MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
    // verificar o numero de processos
    if (numProcess < 2) {
        printf("Number of process must be grater than 1, its necessary at least one slave!\n");
        exit(0);
    }
    // se o rank for igual a 0 executa o master
    if (myRank == 0) { // execute Master
        
        if (argc == 3) {
            vectSize = atoi(argv[1]);
            numBuckets = atoi(argv[2]);
            if (numBuckets > vectSize) {
                printf("Number of buckets must be smaller or equal to vector's size!\n");
                exit(0);
            }
        } else {
            printf("Too few arguments: use $exec x y, x = size of vector, y = number of buckets.\n");
            exit(0);
        }
        
        vector = (int *)malloc(sizeof(int) * vectSize); // allocate vector
        
        srand(time(NULL));
        // Initialize Vector
        for (i = 0; i < vectSize; i++) {
            vector[i] = rand() % vectSize;
            printf("%d ", vector[i]);
        }
        printf("\n");
        arrayBuckets = (bucket_t *)calloc(sizeof(bucket_t), numBuckets);
        // Define bucket range
        int quantMod = vectSize % numBuckets;
        int quantDiv = vectSize / numBuckets;
        arrayBuckets[0].minValue = 0;
        arrayBuckets[0].maxValue = quantDiv + ((quantMod-- > 0)?1:0) - 1;
        for (i = 1; i < numBuckets; i++) {
            arrayBuckets[i].minValue = arrayBuckets[i - 1].maxValue + 1;
            arrayBuckets[i].maxValue = arrayBuckets[i].minValue + quantDiv + ((quantMod-- > 0)?1:0) - 1;
        }
        // Check each bucket size
        for (i = 0; i < vectSize; i++) {
            int j;
            for (j = 0; j < numBuckets; j++) {
                if ((vector[i] >= arrayBuckets[j].minValue) && (vector[i] <= arrayBuckets[j].maxValue)) {
                    arrayBuckets[j].size++;
                    break;
                }
            }
        }
        // Allocate buckets
        for (i = 0; i < numBuckets; i++) {
            arrayBuckets[i].bucket = (int *)malloc(sizeof(int) * arrayBuckets[i].size);
            arrayBuckets[i].size = 0;
        }
        // "distribui" elements to buckets
        for (i = 0; i < vectSize; i++) {
            int j;
            for (j = 0; j < numBuckets; j++) {
                if ((vector[i] >= arrayBuckets[j].minValue) && (vector[i] <= arrayBuckets[j].maxValue)) {
                    arrayBuckets[j].bucket[arrayBuckets[j].size++] = vector[i];
                    break;
                }
            }
        }
        /*
         *    for (i = 0; i < numBuckets; i++) {
         *        int j;
         *        printf("Bucket %d (min= %d, max=%d): ",i, arrayBuckets[i].minValue, arrayBuckets[i].maxValue);
         *        for (j = 0; j < arrayBuckets[i].size; j++)
         *            printf("%d ",arrayBuckets[i].bucket[j]);
         *        printf("\n");
    }
    printf("\n");*/
        executeMaster(numBuckets, numProcess);
	//imprime os valores ordenados
        for (i = 0; i < numBuckets; i++) {
            int j;
            for (j = 0; j < arrayBuckets[i].size; j++)
                printf("%d ",arrayBuckets[i].bucket[j]);
        }
        printf("\n");
	// libera memoria
        for (i = 0; i < numBuckets; i++) {
            free(arrayBuckets[i].bucket);
        }
        free(arrayBuckets);
        free(vector);
    } else { // execute slave
        executeSlave();
    }
    MPI_Finalize();
}

void executeMaster(int numBuckets, const int totalProcess) {
    int usedBucket = 0;
    int numProcess = totalProcess;
    int i;
    for (i = 1; i < totalProcess; i++) {
        if (usedBucket < numBuckets) {
            sendSlave(usedBucket, i);
            usedBucket++;
        } else {
            terminateSlave(i);
            numProcess--;
        }
    }
    while (numProcess > 1) {
        MPI_Status st;
        int ordenedBucket;
        int processWaiting;
        MPI_Recv(&ordenedBucket, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
        processWaiting = st.MPI_SOURCE;
        receiveBucket(ordenedBucket, processWaiting);
        if (usedBucket < numBuckets) {
            sendSlave(usedBucket,processWaiting);
            usedBucket++;
        } else {
            terminateSlave(processWaiting);
            numProcess--;
        }
    }
}

void receiveBucket(int bucket, int target) {
    MPI_Status st;
    MPI_Recv(arrayBuckets[bucket].bucket, arrayBuckets[bucket].size, MPI_INT, target, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
}

void sendSlave(int bucket, int target) {
    int bucketNum_Size[2];
    bucketNum_Size[0] = bucket;
    bucketNum_Size[1] = arrayBuckets[bucket].size;
    MPI_Send(bucketNum_Size, 2, MPI_INT, target, 0, MPI_COMM_WORLD);
    MPI_Send(arrayBuckets[bucket].bucket, arrayBuckets[bucket].size, MPI_INT, target, 0, MPI_COMM_WORLD);
    printf("Mestre ENVIOU bucket %d para Escravo %d\n", bucket, target);
}

void terminateSlave(int target) {
    int bucketNum_Size[2];
    bucketNum_Size[0] = -1;
    bucketNum_Size[1] = -1;
    MPI_Send(bucketNum_Size, 2, MPI_INT, target, 0, MPI_COMM_WORLD);
}

void executeSlave(void) {
    int bucketNum_Size[2];
    int *bucket;
    MPI_Status st;
    while ("The sun still hot!") {
        MPI_Recv(bucketNum_Size, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
        if (bucketNum_Size[0] == -1 && bucketNum_Size[1] == -1) {
            break;
        }
        bucket = (int *)malloc(sizeof(int) * bucketNum_Size[1]);
        MPI_Recv(bucket, bucketNum_Size[1], MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
        bubbleSort(bucket, bucketNum_Size[1]);
        MPI_Send(&bucketNum_Size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(bucket, bucketNum_Size[1], MPI_INT, 0, 0, MPI_COMM_WORLD);
        free(bucket);
    }
    return;
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

