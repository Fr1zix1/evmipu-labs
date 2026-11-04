#include "func.h"
#include <stdlib.h>
#include <x86intrin.h>

// Глобальная переменная для защиты измеряемого цикла от удаления оптимизатором GCC (-O1)
volatile int last_index = 0; 

// Прямой обход: каждый элемент хранит индекс следующего
void init_forward(int *arr, size_t n) {
    for (size_t i = 0; i < n - 1; i++) {
        arr[i] = (int)(i + 1);
    }
    arr[n - 1] = 0;
}

// Обратный обход: каждый элемент хранит индекс предыдущего
void init_backward(int *arr, size_t n) {
    for (size_t i = n - 1; i > 0; i--) {
        arr[i] = (int)(i - 1);
    }
    arr[0] = (int)(n - 1);
}

// Случайный обход
void init_random(int *arr, size_t n) {
    int *indices = (int *)malloc(n * sizeof(int));
    if (indices != NULL) {
        // Создаем последовательный массив индексов
        for (size_t i = 0; i < n; i++) {
            indices[i] = (int)i;
        }
        
        // Перемешиваем индексы алгоритмом Фишера-Йетса
        for (size_t i = n - 1; i > 0; i--) {
            size_t j = (size_t)(rand() % (int)(i + 1));
            int temp = indices[i];
            indices[i] = indices[j];
            indices[j] = temp;
        }
        
        for (size_t i = 0; i < n - 1; i++) {
            arr[indices[i]] = indices[i + 1];
        }
        arr[indices[n - 1]] = indices[0];
        
        free(indices);
    }
}

// Функция замера времени (возвращает такты на один элемент)
uint64_t measure_time(int *arr, size_t n, size_t k) {
    int idx = 0;
    
    // Прогрев кэша
    for (size_t i = 0; i < n; i++) {
        idx = arr[idx];
    }

    uint64_t min_time = UINT64_MAX;
    const int runs = 5; // Выполняем 5 прогонов для отсеивания случайных помех ОС

    for (int r = 0; r < runs; r++) {
        idx = 0;
        uint64_t start = __rdtsc();
        
        // Основной многократный обход массива
        for (size_t i = 0; i < n * k; i++) {
            idx = arr[idx];
        }
        
        uint64_t end = __rdtsc();
        uint64_t current_time = end - start;
        
        // Фиксируем самое быстрое (чистое) время из всех прогонов
        if (current_time < min_time) {
            min_time = current_time;
        }
    }
    last_index = idx; 
    
    return min_time / (n * k);
}
