#include "func.h"
#include <x86intrin.h>

// Глобальная переменная для защиты измеряемого цикла от удаления оптимизатором GCC (-O1)
volatile uint32_t last_index = 0; 

void init_array(uint32_t *arr, size_t n_fragments, size_t frag_size_bytes, size_t offset_bytes) {
    size_t frag_elems = frag_size_bytes / sizeof(uint32_t);
    size_t offset_elems = offset_bytes / sizeof(uint32_t);

    for (size_t i = 0; i < frag_elems; i++) {
        for (size_t j = 0; j < n_fragments; j++) {
            size_t current_idx = j * offset_elems + i;
            size_t next_idx;

            if (j == (n_fragments - 1)) {
                // Переход к следующему элементу в первом фрагменте
                next_idx = 0 * offset_elems + i + 1;
            } else {
                // Переход к текущему элементу в следующем фрагменте
                next_idx = (j + 1) * offset_elems + i;
            }

            if ((i == (frag_elems - 1)) && (j == (n_fragments - 1))) {
                // Замыкание списка на первый элемент
                next_idx = 0;
            }

            arr[current_idx] = (uint32_t)next_idx;
        }
    }
}

double measure_time(uint32_t *arr, size_t total_elements, size_t iterations) {
    uint32_t idx = 0;
    
    // Прогрев кэша
    for (size_t i = 0; i < total_elements; i++) {
        idx = arr[idx];
    }

    uint64_t min_time = UINT64_MAX;
    const int runs = 5; // Выполняем 5 прогонов для отсеивания случайных помех ОС

    for (int r = 0; r < runs; r++) {
        idx = 0;
        uint64_t start = __rdtsc();
        
        // Основной многократный обход массива
        for (size_t i = 0; i < total_elements * iterations; i++) {
            idx = arr[idx];
        }
        
        uint64_t end = __rdtsc();
        uint64_t current_time = end - start;
        
        if (current_time < min_time) {
            min_time = current_time;
        }
    }
    
    last_index = idx; 
    
    // Возвращаем время в double для сохранения дробных долей такта
    return (double)min_time / (double)(total_elements * iterations);
}
