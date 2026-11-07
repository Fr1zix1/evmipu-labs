#include <stdio.h>
#include <stdlib.h>
#include "func.h"

// Смещение, гарантирующее попадание в один банк (16 МБ)
#define OFFSET_BYTES (16 * 1024 * 1024)

int main(void) {
    size_t max_fragments = 32;
    
    size_t cache_sizes[] = {32 * 1024, 256 * 1024, 12 * 1024 * 1024}; 
    const char *cache_names[] = {"L1", "L2", "L3"};
    size_t num_caches = sizeof(cache_sizes) / sizeof(cache_sizes[0]);

    for (size_t c = 0; c < num_caches; c++) {
        size_t cache_size = cache_sizes[c];
        
        printf("\n--- Test %s Cache (Size: %zu Bytes) ---\n", cache_names[c], cache_size);
        printf("N,Ticks\n");

        size_t array_size_bytes = (max_fragments * OFFSET_BYTES) + cache_size;
        
        uint32_t *arr = (uint32_t *)aligned_alloc(4096, array_size_bytes);
        if (arr == NULL) {
            printf("Memory allocation failed for %s\n", cache_names[c]);
            return 1;
        }

        // Динамический подбор итераций для ускорения теста больших объемов памяти
        size_t iterations = (cache_size <= 32 * 1024) ? 100 : (cache_size <= 256 * 1024) ? 10 : 2;

        for (size_t n = 1; n <= max_fragments; n++) {
            size_t frag_size_bytes = (cache_size / n) & ~63UL;
            
            // Защита от нулевого размера на больших n
            if (frag_size_bytes == 0) continue; 
            
            init_array(arr, n, frag_size_bytes, OFFSET_BYTES);
            
            size_t total_elements = (frag_size_bytes / sizeof(uint32_t)) * n;
            
            double min_ticks_per_element = measure_time(arr, total_elements, iterations);
            
            printf("%zu,%.2f\n", n, min_ticks_per_element);
        }

        free(arr);
    }

    return 0;
}
