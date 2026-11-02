#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>
#include "func.h"

// Функция для чтения счетчика тактов процессора
static inline unsigned long long get_ticks(void) {
    return __rdtsc();
}

unsigned long long measure_time(int *arr, size_t n, int passes) {
    volatile size_t k = 0; 
    
    // Прогрев кэша: выгружаем посторонние данные из памяти L1/L2
    for (size_t i = 0; i < n; i++) {
        k = arr[k]; 
    }
    
    k = 0;
    // Замеряем начальный такт таймера
    unsigned long long start = get_ticks();
    
    // Основной обход массива заданное количество раз
    for (size_t i = 0; i < n * passes; i++) {
        k = arr[k]; // Переход по цепочке индексов
    }
    
    // Замеряем конечный такт таймера
    unsigned long long end = get_ticks();
    
    // Возвращаем среднее время доступа к одному элементу
    return (end - start) / (n * passes);
}

int main(void) {
    // Границы тестирования: от 1 КБ (меньше L1 кэша) до 32 МБ (больше L3 кэша)
    size_t min_size = 256;       
    size_t max_size = 8388608;   
    int passes = 10; // Количество полных обходов для точности замера

    printf("Size(KB),Forward,Reverse,Random\n");
    
    // Цикл с переменным шагом (размер массива увеличивается на 20% каждую итерацию)
    for (size_t n = min_size; n <= max_size; n = (size_t)(n * 1.2)) {
        int *arr = (int*)malloc(n * sizeof(int));
        if (arr == NULL) {
            return 1;
        }

        // Тестирование прямого обхода
        init_forward(arr, n);
        unsigned long long t_fwd = measure_time(arr, n, passes);

        // Тестирование обратного обхода
        init_reverse(arr, n);
        unsigned long long t_rev = measure_time(arr, n, passes);

        // Тестирование случайного обхода
        init_random(arr, n);
        unsigned long long t_rnd = measure_time(arr, n, passes);

        // Вывод результатов
        printf("%zu,%llu,%llu,%llu\n", (n * sizeof(int)) / 1024, t_fwd, t_rev, t_rnd);
        free(arr);
    }
    return 0;
}
