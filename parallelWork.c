#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sysinfo.h>

typedef struct{
    int sizeMat;
    int numCores;
    int divider;
    float **matX;
    float **matY;
    float **matEnd;
    pthread_mutex_t lock;
}EstMatThreads;

float **allocates_matrix(int sizeMat){
    float **matrix;
    matrix = (float**) malloc((sizeMat) * sizeof(float *));
    for(int i = 0; i < sizeMat; i++){
        matrix[i] = malloc((sizeMat) * sizeof(float));
    }
    return matrix;
}

float **read(int *sizeMat, char file[]){
    char char_removed;
    FILE *fp = fopen(file, "r");
    if(fp == NULL){
        printf("\n Error! File %s not reachable or not found.\n", file);
        exit(1);
    }
    fscanf(fp,"%d\n", sizeMat);
    float **matrix = (float**) malloc( (*sizeMat) * sizeof(float *));
    for(int i = 0; i < *sizeMat; i ++){
        matrix[i] = malloc((*sizeMat) * sizeof(float));
    }
    for(int i = 0; i < *sizeMat; i ++){
        for(int j = 0; j < *sizeMat; j ++){
            fscanf(fp, "%f%c", &matrix[i][j], &char_removed);
        }
    }
    fclose(fp);
    return matrix;
}

void save_matrix_end(float **matrix, int sizeMat, char file[]){
    FILE *fp = fopen(file, "w");
    fprintf(fp, "%d\n", sizeMat);
    for(int i = 0; i < sizeMat; i ++){
        for(int j = 0; j < sizeMat; j ++){
            if ( j != sizeMat - 1 ) fprintf(fp, "%.1f:", matrix[i][j]);
            else fprintf(fp, "%.1f\n", matrix[i][j]);
        }
    }
    fclose(fp);
}

void multiply_matrices(float **matX, float **matY, float **matEnd , int sizeMat, int inicio, int fim){
    for(int i = inicio; i < fim; i ++){
        for(int j = 0; j < sizeMat; j ++){
            matEnd[i][j] = 0;
            for(int k = 0; k < sizeMat; k ++){
                matEnd[i][j] +=  matX[i][k] * matY[k][j];
            }
        }
    }

}

void *multiply_parallel(void *arg){
    EstMatThreads *mat = (EstMatThreads*) (arg);
    int division;
    pthread_mutex_lock(&mat -> lock);
    division = mat -> divider;
    mat -> divider ++;
    pthread_mutex_unlock(&mat -> lock);
    printf(" Thread %d inicialized.\n", division + 1);
    int part = mat -> sizeMat / mat -> numCores;
    int start = division * part;
    int last = (division + 1) * part;
    if (division == (mat -> numCores - 1)){
        last += (mat -> sizeMat) % (mat -> numCores);
    }
    multiply_matrices( mat -> matX, mat -> matY, mat -> matEnd, mat -> sizeMat, start, last);
    return 0;
}

int main(int argc, char *argv[]){
    system("clear || cls");
    if(argc != 4){
        printf("\n Error! Try as in the example: time %s <matrix_1.txt> <matrix_2txt> <matrix _ target.txt>\n", argv[0]);
        exit(1);
    }
    EstMatThreads mat;
    mat.numCores = get_nprocs();
    mat.matX = read(&mat.sizeMat, argv[1]);
    mat.matY = read(&mat.sizeMat, argv[2]);
    mat.matEnd = allocates_matrix(mat.sizeMat);
    printf("\n Calculating the product ...\n\n");
    mat.divider = 0;
    pthread_t vetThreads[mat.numCores];
    for(int i = 0; i < mat.numCores; i ++){
        pthread_create( &vetThreads[i], NULL, multiply_parallel, (void *)(&mat));
    }
    for(int i = 0; i < mat.numCores; i ++){
        pthread_join(vetThreads[i], NULL);
        printf(" Thread %d finalized.\n", i + 1);
    }
    save_matrix_end(mat.matEnd, mat.sizeMat, argv[3]);
    puts("\n\n Parallel multiplication successfully completed!!!");
    printf(" Matrix product saved in %s\n\n", argv[3]);
    puts(" END.");
    return 0;
}
