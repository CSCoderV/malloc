#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_ITER 1000000

int main() {
    printf("Starting Benchmark 2....\n");
    //I have used rand function to randomly allocate and deallocate memory blocks
    //this way it tests real world usage
    // Array to hold pointers for simulated block allocations
    void *ptrs[100] = {0};
    size_t ptr_sizes[100] = {0};
    size_t alloc_count = 0, free_count = 0, current_alloc = 0, peak_alloc = 0;
    
    // using clock_gettime for higher resolution
    clock_t start = clock();
    
    // looping for larger time so that time difference is larger - basically for easier measurement
    for (int i = 0; i < NUM_ITER; i++) {
        // Using deterministic index instead of random: replace rand() % 7 with i % 7
        int idx = i % 7;
        
        // free ptr if being used
        if (ptrs[idx]) {
            free(ptrs[idx]);
            free_count++; current_alloc -= ptr_sizes[idx]; ptr_sizes[idx] = 0;
            ptrs[idx] = NULL;
        }
        
        // Deterministically picks block size (kindof real world)
        // Instead of 512 * (1 + rand() % 7), we use 512 * (1 + (i % 7))
        size_t size = 512 * (1 + (i % 7));
        ptrs[idx] = malloc(size);
        if (ptrs[idx]) {
            memset(ptrs[idx], 0, size);
            alloc_count++; current_alloc += size; ptr_sizes[idx] = size;
            if(current_alloc > peak_alloc)
                peak_alloc = current_alloc;
        }
    }
    
    // free all blocks
    for (int i = 0; i < 100; i++) {
        if (ptrs[i]) {
            free(ptrs[i]);
            free_count++; current_alloc -= ptr_sizes[i]; ptr_sizes[i] = 0;
        } else {
            free(ptrs[i]);
        }
    }
    
    clock_t end = clock();
    // Calculate elapsed time in seconds
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Benchmark 2 completed...Success!!!\n");
    printf("Execution time: %f seconds\n", time_spent);
    printf("Total malloc calls: %zu\n", alloc_count);
    printf("Total free calls: %zu\n", free_count);
    printf("Peak allocated memory: %zu bytes\n", peak_alloc);
    
    return 0;
}
