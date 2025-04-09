#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_ITER 100000

int main(){
    printf("Hello, starting benchmark...\n");
    size_t alloc_count = 0, free_count = 0, current_alloc = 0, peak_alloc = 0;
    clock_t start = clock();
    for (int i = 0; i < NUM_ITER; i++){
        //  two blocks of 2048 bytes each allocated
        void *a1 = malloc(2048);
        if(a1){
            alloc_count++; 
            current_alloc += 2048;
            if(current_alloc> peak_alloc)
                peak_alloc = current_alloc;
        }
        void *a2 = malloc(2048);
        if(a2){
            alloc_count++; 
            current_alloc += 2048;
            if(current_alloc> peak_alloc)
                peak_alloc = current_alloc;
        }
        free(a1);
        free_count++;
        current_alloc -= 2048;
        // allocate 1024 bytes to new block ad free it
        void *a3 = malloc(1024);
        if(a3){
            alloc_count++;
            current_alloc += 1024; 
            if(current_alloc > peak_alloc)
                peak_alloc = current_alloc; 
        }
        free(a3);
        free_count++;
        current_alloc -= 1024;

        free(a2);
        free_count++;
        current_alloc -= 2048;
        void *a4 = malloc(4096);
        if(a4){
            alloc_count++;
            current_alloc += 4096; 
            if(current_alloc > peak_alloc)
                peak_alloc = current_alloc; 
        }
        free(a4);
        free_count++;
        current_alloc -= 4096;

    }
    clock_t end = clock();
    
    double time_req = (double)(end - start) / CLOCKS_PER_SEC;
    printf("\n Benchmark completed successfully!!\n");
    printf("Execution time was %f seconds.\n", time_req);
    printf("Total malloc calls: %zu\n", alloc_count);
    printf("Total free calls: %zu\n", free_count);
    printf("Peak allocated memory: %zu bytes\n", peak_alloc);
    return 0;
}
