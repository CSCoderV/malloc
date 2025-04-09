#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_ITER 100000

int main(){
    printf("Hello, starting benchmark...\n");

    clock_t start = clock();
    for (int i = 0; i < NUM_ITER; i++){
        //  two blocks of 2048 bytes each allocated
        void *a1 = malloc(2048);
        void *a2 = malloc(2048);
        
        free(a1);
        
        // allocate 1024 bytes to new block ad free it
        void *a3 = malloc(1024);
        free(a3);
        free(a2);
        
        void *a4 = malloc(4096);
        free(a4);
    }
    clock_t end = clock();
    
    double time_req = (double)(end - start) / CLOCKS_PER_SEC;
    printf("\n Benchmark completed successfully!!\n");
    printf("Execution time was %f seconds.\n", time_req);
    
    return 0;
}
