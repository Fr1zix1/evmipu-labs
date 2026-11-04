#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "func.h"

int main(void) {
    srand((unsigned int)time(NULL));
    
    size_t min_size = 256;         // 1 KB 
    size_t max_size = 8388608;     // 32 MB 
  
    printf("Size_KB,Forward,Backward,Random\n");
    
    // Увеличиваем размер массива ровно в 2 раза на каждой итерации
    for (size_t n = min_size; n <= max_size; n *= 2){ 
        size_t k = (50000000 / n) + 1; 
        
        int *arr = (int *)malloc(n * sizeof(int));
        
        if (arr != NULL) {
            init_forward(arr, n);
            uint64_t time_fwd = measure_time(arr, n, k);
            
            init_backward(arr, n);
            uint64_t time_bwd = measure_time(arr, n, k);
            
            init_random(arr, n);
            uint64_t time_rnd = measure_time(arr, n, k);
            
            printf("%zu,%llu,%llu,%llu\n", (n * sizeof(int)) / 1024, 
                   (unsigned long long)time_fwd, 
                   (unsigned long long)time_bwd, 
                   (unsigned long long)time_rnd);
            
            free(arr);
        }
    }
    return 0;
}
